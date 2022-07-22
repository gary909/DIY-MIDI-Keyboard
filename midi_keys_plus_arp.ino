/*
          _____  ___ _____                       
  /\/\    \_   \/   \\_   \   /\ /\___ _   _ ___ 
 /    \    / /\/ /\ / / /\/  / //_/ _ \ | | / __|
/ /\/\ \/\/ /_/ /_//\/ /_   / __ \  __/ |_| \__ \
\/    \/\____/___,'\____/   \/  \/\___|\__, |___/
                                       |___/     
           _      __    ___                      
   _      /_\    /__\  / _ \                     
 _| |_   //_\\  / \// / /_)/                     
|_   _| /  _  \/ _  \/ ___/                      
  |_|   \_/ \_/\/ \_/\/                          
                                                                    
*/                                                                            
                        
/**********************************************************************************************************   
 *
 *   A MIDI keyboard with Arpeggiator.
 *
 *   Features;
 *
 *   Pot for tempo
 *   Pot for velocity
 *   Button with 5 Arp-modes
 *   Arp stop button
 *   Keyboard Octave Buttons with responsive LED brightness
 *   Switch to chose between keyboard and ARP mode 
 *
 *
 *   Note: to upload code to the board, you'll need the switch set to the left (aka keyboard mode)
 *
 *
 *   v1 22/07/2022
 *   -------------
 *   Mostly finised...  a few known bugs;
 *
 *   Using the octave buttons causes the ARP a 1/4 second delay...  this is because the buttons use 
 *   'delay(250);' (see line 517 onwards).  This might be able to be removed, or at least reduced. I
 *   should probably get around to doing that...
 *
 *   Holding notes and presssing octave/arp buttons can cause bugs.  Avoid this and it'll
 *   be fine.  Sometimes it can lead to fun results though.
 *     
 *   Holding a note and changing octaves causes double notes (it's not a bug its a feature!)...
 *   ..press arp stop button to clear this.
 *   
 *   Holding notes while changing from arp mode on/off can cause the arp to stop working (definitely a bug).
 *   This could probably be cleared by adding 6n137 circuitry inbetween the Arduinos RX/TX pins,
 *   but for the sake of simplicity I've left this out.
 *    
 ***********************************************************************************************************

    Much of the inspiration of the MIDI Keys comes from this article;
    https://create.arduino.cc/projecthub/gleberruyer/midi-wood-keyboard-88053e
    Also this code for the ARP;
    https://github.com/wbajzek/arduino-arpeggiator
    Button setup from this article:
    https://www.arduino.cc/en/Tutorial/BuiltInExamples/Button
    Ascii text from: http://patorjk.com/ using 'Ogre' font
    
    MIDI Note numbers:
    octave  C   C#  D   D#  E   F   F#  G   G#  A   A#  B
        1   36  37  38  39  40  41  42  43  44  45  46  47
        2   48  49  50  51  52  53  54  55  56  57  58  59
        3   60  61  62  63  64  65  66  67  68  69  70  71
        4   72  73  74  75  76  77  78  79  80  81  82  83
        5   84  85  86  87  88  89  90  91  92  93  94  95
*/


/********************************************************************_KEYBOARD_CODE_DECLARATIONS_**************************/
#include <MIDI.h>
MIDI_CREATE_DEFAULT_INSTANCE();

int button1_LowC_pin = 2;    // Low C digital pin 2
int button2_Csharp_pin = 3;  // C#  - digital pin 3
int button3_D_pin = 4;       // D   - digital pin 4
int button4_Dsharp_pin = A1; // D# -     *** ANALOG PIN 1 ***
int button5_E_pin = 6;       // E   - digital pin 6
int button6_F_pin = 7;       // F   - digital pin 7
int button7_Fsharp_pin = 8;  // F#  - digital pin 8
int button8_G_pin = 9;       // G   - digital pin 9
int button9_Gsharp_pin = 10; // G#  - digital pin 10
int button10_A_pin = 11;     // A   - digital pin 11
int button11_Asharp_pin = 12; // A# - digital pin 12
int button12_B_pin = 13;      // B  - digital pin 12
int button13_C_pin = A0;      // C  - analogue pin A0
int midiChannel = 1;
/********************************************************************_END_OF_KEYBOARD CODE_DECLARATIONS_*******************/

/********************************************************************_OCTAVE_BUTTONS_CODE_DECLARATIONS_********************/
const int  octUpButton = A3;      // Octave Up pushbutton
const int  octDwnButton = A4;     // Octave Down pushbutton
const int octLED = 5;             // LED on pin 5 (~pwm pin)
const int octLED_Brightness[] = {0, 10, 30, 50, 120, 255}; // brightness for 0,1,2,3,4,5 octaves
int octButtonPushCounter = 2;     // counter for the number of button presses, 2 to start in middle octave
int octUpButtonState = 0;         // current state Up button
int octDwnButtonState = 0;        // current state of Down button
int octLastButtonState = 0;       // previous state of the button
const int octaveSelected[][12] = {
  {24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35},   // Octave 0
  {36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47},   // Octave 1
  {48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59},   // Octave 2
  {60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71},   // Octave 3
  {72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83},   // Octave 4
  {84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95},   // Octave 5
  {96}                                                // Octave 6 'c' note
};
/********************************************************************END_OF_OCTAVE_BUTTONS_CODE_DECLARATIONS_*************/



/*************************************************START_OF_ARP_DECLARATIONS_*************/
                    // FYI A6 & A7 are analog only on Nano :-)
#define velPot    A6 //7 Velocity **remember, also change instances analogRead(0) to analogRead(6)
#define arpTempPot    A7 //6 Tempo **remember, change instances analogRead(1) to analogRead(7)
#define BUTTON1  A2 //2 Hold mode on/off
#define BUTTON2  A5 //3 Arp Modes
//#define BUTTON3  4 //4 Not enough pins free on the Arduino :-(

const int CHANNEL      = 1;
const int UP           = 0;
const int DOWN         = 1;
const int BOUNCE       = 2;
const int UPDOWN       = 3;
const int ONETHREE     = 4;
const int ONETHREEEVEN = 5;
const int MODES        = 6;

byte notes[10];
unsigned long tempo;
unsigned long lastTime;
unsigned long blinkTime;
unsigned long tick;
unsigned long buttonOneHeldTime;
unsigned long buttonTwoHeldTime;
//unsigned long buttonThreeHeldTime;
unsigned long debounceTime;
int playBeat;
int notesHeld;
int mode;
int clockTick;
boolean blinkOn;
boolean hold;
boolean buttonOneDown;
boolean buttonTwoDown;
//boolean buttonThreeDown;
boolean bypass;
boolean midiThruOn;
boolean arpUp;
boolean clockSync;

/***************************************************END_OF_ARP_DECLARATIONS_*************/




/************************************************************************************************************_Void_SETUP_**/
void setup() {
/********************************************************************_KEYBOARD_CODE_SETUP_*********************************/
  MIDI.begin ();
  pinMode(button1_LowC_pin, INPUT);
  pinMode(button2_Csharp_pin, INPUT);
  pinMode(button3_D_pin, INPUT);
  pinMode(button4_Dsharp_pin, INPUT);
  pinMode(button5_E_pin, INPUT);
  pinMode(button6_F_pin, INPUT);
  pinMode(button7_Fsharp_pin, INPUT);
  pinMode(button8_G_pin, INPUT);
  pinMode(button9_Gsharp_pin, INPUT);
  pinMode(button10_A_pin, INPUT);
  pinMode(button11_Asharp_pin, INPUT);
  pinMode(button12_B_pin, INPUT);
  pinMode(button13_C_pin, INPUT);
  //Serial.begin(31250);
/********************************************************************_END_OF_KEYBOARD_CODE_SETUP_*************************/  



/********************************************************************_OCTAVE_BUTTON_CODE_SETUP_***************************/
  pinMode(octUpButton, INPUT);   // initialize the button pin as a input:
  pinMode(octDwnButton, INPUT);
  pinMode(octLED, OUTPUT);       // Set the LED pin as an output
  analogWrite(octLED, octLED_Brightness[1]);    // Start LED brightness in middle
/********************************************************************_END_OF_OCTAVE_BUTTON_CODE_SETUP_********************/



/*************************************************START_OF_ARP_CODE_SETUP_*************/
  blinkTime = lastTime = millis();
  buttonOneHeldTime = buttonTwoHeldTime = 0;
  notesHeld = 0;
  playBeat=0;
  blinkOn = false;
  hold=true;
  arpUp = true; // used to determine which way arp is going when in updown mode
  buttonOneDown = buttonTwoDown = false;
  mode=0;
  bypass = midiThruOn = false;
  tempo = 400;
  debounceTime = 50;
  clockSync = false;
  clockTick = 1;

  pinMode(velPot,OUTPUT);   
  pinMode(arpTempPot,OUTPUT);
  pinMode(BUTTON1,INPUT);
  pinMode(BUTTON2,INPUT);
//  pinMode(BUTTON3,INPUT);

  digitalWrite(BUTTON1,HIGH);
  digitalWrite(BUTTON2,HIGH);
//  digitalWrite(BUTTON3,HIGH);

  // Initiate MIDI communications, listen to all channels
  MIDI.begin(MIDI_CHANNEL_OMNI);    

  MIDI.setHandleNoteOn(HandleNoteOn); 
  MIDI.setHandleControlChange (HandleControlChange);
  MIDI.setHandleClock (HandleClock);
  MIDI.setHandleStart (HandleStart);
  MIDI.setHandleStop (HandleStop);
  MIDI.turnThruOff();

  digitalWrite(velPot,HIGH);
  digitalWrite(arpTempPot,HIGH);

/***************************************************END_OF_ARP_CODE_SETUP_*************/
}

/**************************************************START_OF_ARP_FUNCTIONS_*************/

// This function will be automatically called when a NoteOn is received.
// see documentation here: 
// http://arduinomidilib.sourceforge.net/

void HandleNoteOn(byte channel, byte pitch, byte velocity) { 


  if (velocity == 0) // note released
    notesHeld--;
  else {
    // If it's in hold mode and you are not holding any notes down,
    // it continues to play the previous arpeggio. Once you press
    // a new note, it resets the arpeggio and starts a new one.
    if (notesHeld==0 && hold) 
      resetNotes();

    notesHeld++;
  }


  // Turn on an LED when any notes are held and off when all are released.
  if (notesHeld > 0)
    digitalWrite(arpTempPot,LOW); // stupid midi shield has high/low backwards for the LEDs
  else
    digitalWrite(arpTempPot,HIGH); // stupid midi shield has high/low backwards for the LEDs



  // find the right place to insert the note in the notes array
  for (int i = 0; i < sizeof(notes); i++) {

    if (velocity == 0) { // note released
      if (!hold && notes[i] >= pitch) { 

        // shift all notes in the array beyond or equal to the
        // note in question, thereby removing it and keeping
        // the array compact.
        if (i < sizeof(notes))
          notes[i] = notes[i+1];
      }
    }
    else {

      if (notes[i] == pitch)
        return;   // already in arpeggio
      else if (notes[i] != '\0' && notes[i] < pitch)
        continue; // ignore the notes below it
      else {
        // once we reach the first note in the arpeggio that's higher
        // than the new one, scoot the rest of the arpeggio array over 
        // to the right
        for (int j = sizeof(notes); j > i; j--)
          notes[j] = notes[j-1];

        // and insert the note
        notes[i] = pitch;
        return;        
      }
    }
  }
}

// just pass midi CC through
void HandleControlChange (byte channel, byte number, byte value) {
  MIDI.sendControlChange(number,value,channel); 
}

// This is a midi clock sync "start" message. In this case, switch to clock
// sync mode and trigger the first note.
void HandleStart () {
  clockSync = true;  
  clockTick = 1;
  playBeat = 0;
  cli();
  tick = millis();
  sei();

  handleTick(tick);
}

// Turn clock sync off when "stop" is received.
void HandleStop () {
  clockSync = false;
}

// on each tick of the midi clock, determine whether or note to play
// a note
void HandleClock () {

  cli();
  tick = millis();
  sei();

  // keep a counter of every clock tick we receive. if the count
  // corresponds with the subdivision selected with the tempo knob,
  // play the note and reset the clock.
  //
  // clock is 1 based instead of 0 for ease of calculation 

  if (clockTick >= (int)map(analogRead(arpTempPot),0,1023,1,4)*6 + 1) { // changed analogRead(1) to analogRead(7) to analogRead(arpTempPot)
    handleTick(tick);
    clockTick = 1;
  }


  clockTick++;
}
/***********************************************END_OF_ARP_FUNCTIONS_*************/





/**********************************************************************************************************_Void_Loop()_**/

void loop() {
  /********************************************************************_KEYBOARD_CODE_LOOP_*********************************/                                 
  static bool button1_LowC_valueOld = LOW;
  static bool button2_Csharp_valueOld = LOW;
  static bool button3_D_valueOld = LOW;
  static bool button4_Dsharp_valueOld = LOW;
  static bool button5_E_valueOld = LOW;
  static bool button6_F_valueOld = LOW;
  static bool button7_Fsharp_valueOld = LOW;
  static bool button8_G_valueOld = LOW;
  static bool button9_Gsharp_valueOld = LOW;
  static bool button10_A_valueOld = LOW;
  static bool button11_Asharp_valueOld = LOW;
  static bool button12_B_valueOld = LOW;
  static bool button13_C_valueOld = LOW;
  
  bool button1_LowC_valueNew = digitalRead(button1_LowC_pin);
  bool button2_Csharp_valueNew = digitalRead(button2_Csharp_pin);
  bool button3_D_valueNew = digitalRead(button3_D_pin);
  bool button4_Dsharp_valueNew = digitalRead(button4_Dsharp_pin);
  bool button5_E_valueNew = digitalRead(button5_E_pin);
  bool button6_F_valueNew = digitalRead(button6_F_pin);
  bool button7_Fsharp_valueNew = digitalRead(button7_Fsharp_pin);
  bool button8_G_valueNew = digitalRead(button8_G_pin);
  bool button9_Gsharp_valueNew = digitalRead(button9_Gsharp_pin);
  bool button10_A_valueNew = digitalRead(button10_A_pin);
  bool button11_Asharp_valueNew = digitalRead(button11_Asharp_pin);
  bool button12_B_valueNew = digitalRead(button12_B_pin);
  bool button13_C_valueNew = digitalRead(button13_C_pin);
  
      ////////////////// Low C //////////////    
  if (button1_LowC_valueNew != button1_LowC_valueOld){
    if (button1_LowC_valueNew == HIGH){
    //MIDI.sendNoteOn(48, 127, midiChannel); // "Note low C On"
    MIDI.sendNoteOn(octaveSelected[octButtonPushCounter][0], 127, midiChannel); // "Note low C On"
    delay(60);
    } else {
    //MIDI.sendNoteOff(48, 0, midiChannel); // "Note low C off"
    MIDI.sendNoteOff(octaveSelected[octButtonPushCounter][0], 0, midiChannel); // "Note low C off"
    delay(60);
    }  
    button1_LowC_valueOld = button1_LowC_valueNew;
  }
  
    ////////////////// C Sharp //////////////    
  if (button2_Csharp_valueNew != button2_Csharp_valueOld){ 
    if (button2_Csharp_valueNew == HIGH){  
    //MIDI.sendNoteOn(49, 127, 1); //  "Note C sharp On"
    MIDI.sendNoteOn(octaveSelected[octButtonPushCounter][1], 127, 1); //  "Note C sharp On"
    } else {
    MIDI.sendNoteOff(octaveSelected[octButtonPushCounter][1], 0, 1); //"Note C Sharp Off"
    }
    button2_Csharp_valueOld = button2_Csharp_valueNew;
  }

    ////////////////// D //////////////    
  if (button3_D_valueNew != button3_D_valueOld){ 
    if (button3_D_valueNew == HIGH){  
    //MIDI.sendNoteOn(50, 127, 1); //  "Note D On"
    MIDI.sendNoteOn(octaveSelected[octButtonPushCounter][2], 127, 1); //  "Note D On"
    } else {
    MIDI.sendNoteOff(octaveSelected[octButtonPushCounter][2], 0, 1); //"Note D Off"
    }
    button3_D_valueOld = button3_D_valueNew;
  }

      ////////////////// D sharp //////////////    
  if (button4_Dsharp_valueNew != button4_Dsharp_valueOld){ 
    if (button4_Dsharp_valueNew == HIGH){  
    //MIDI.sendNoteOn(51, 127, 1); //  "Note D# On"
    MIDI.sendNoteOn(octaveSelected[octButtonPushCounter][3], 127, 1); //  "Note D# On"
    } else {
    MIDI.sendNoteOff(octaveSelected[octButtonPushCounter][3], 0, 1); //"Note D# Off"
    }
    button4_Dsharp_valueOld = button4_Dsharp_valueNew;
  }

      ////////////////// E //////////////    
  if (button5_E_valueNew != button5_E_valueOld){ 
    if (button5_E_valueNew == HIGH){  
    //MIDI.sendNoteOn(52, 127, 1); //  "Note E On"
    MIDI.sendNoteOn(octaveSelected[octButtonPushCounter][4], 127, 1); //  "Note E On"
    } else {
    MIDI.sendNoteOff(octaveSelected[octButtonPushCounter][4], 0, 1); //"Note E Off"
    }
    button5_E_valueOld = button5_E_valueNew;
  }

      ////////////////// F //////////////    
  if (button6_F_valueNew != button6_F_valueOld){ 
    if (button6_F_valueNew == HIGH){  
    //MIDI.sendNoteOn(53, 127, 1); //  "Note F On"
    MIDI.sendNoteOn(octaveSelected[octButtonPushCounter][5], 127, 1); //  "Note F On"
    } else {
    MIDI.sendNoteOff(octaveSelected[octButtonPushCounter][5], 0, 1); //"Note F Off"
    }
    button6_F_valueOld = button6_F_valueNew;
  }

    ////////////////// F# //////////////    
  if (button7_Fsharp_valueNew != button7_Fsharp_valueOld){ 
    if (button7_Fsharp_valueNew == HIGH){  
    //MIDI.sendNoteOn(54, 127, 1); //  "Note F# On"
    MIDI.sendNoteOn(octaveSelected[octButtonPushCounter][6], 127, 1); //  "Note F# On"
    } else {
    MIDI.sendNoteOff(octaveSelected[octButtonPushCounter][6], 0, 1); //"Note F# Off"
    }
    button7_Fsharp_valueOld = button7_Fsharp_valueNew;
  }

      ////////////////// G //////////////    
  if (button8_G_valueNew != button8_G_valueOld){ 
    if (button8_G_valueNew == HIGH){  
    //MIDI.sendNoteOn(55, 127, 1); //  "Note G On"
    MIDI.sendNoteOn(octaveSelected[octButtonPushCounter][7], 127, 1); //  "Note G On"
    } else {
    MIDI.sendNoteOff(octaveSelected[octButtonPushCounter][7], 0, 1); //"Note G Off"
    }
    button8_G_valueOld = button8_G_valueNew;
  }

    ////////////////// G# //////////////    
  if (button9_Gsharp_valueNew != button9_Gsharp_valueOld){ 
    if (button9_Gsharp_valueNew == HIGH){  
    //MIDI.sendNoteOn(56, 127, 1); //  "Note G# On"
    MIDI.sendNoteOn(octaveSelected[octButtonPushCounter][8], 127, 1); //  "Note G# On"
    } else {
    MIDI.sendNoteOff(octaveSelected[octButtonPushCounter][8], 0, 1); //"Note G# Off"
    }
    button9_Gsharp_valueOld = button9_Gsharp_valueNew;
  }

    ////////////////// A //////////////    
  if (button10_A_valueNew != button10_A_valueOld){ 
    if (button10_A_valueNew == HIGH){  
    //MIDI.sendNoteOn(57, 127, 1); //  "Note A On"
    MIDI.sendNoteOn(octaveSelected[octButtonPushCounter][9], 127, 1); //  "Note A On"
    } else {
    MIDI.sendNoteOff(octaveSelected[octButtonPushCounter][9], 0, 1); //"Note A Off"
    }
    button10_A_valueOld = button10_A_valueNew;
  }

    ////////////////// A# //////////////    
  if (button11_Asharp_valueNew != button11_Asharp_valueOld){ 
    if (button11_Asharp_valueNew == HIGH){  
    //MIDI.sendNoteOn(58, 127, 1); //  "Note A# On"
    MIDI.sendNoteOn(octaveSelected[octButtonPushCounter][10], 127, 1); //  "Note A# On"
    } else {
    MIDI.sendNoteOff(octaveSelected[octButtonPushCounter][10], 0, 1); //"Note A# Off"
    }
    button11_Asharp_valueOld = button11_Asharp_valueNew;
  }

    ////////////////// B //////////////    
  if (button12_B_valueNew != button12_B_valueOld){ 
    if (button12_B_valueNew == HIGH){  
    //MIDI.sendNoteOn(59, 127, 1); //  "Note B On"
    MIDI.sendNoteOn(octaveSelected[octButtonPushCounter][11], 127, 1); //  "Note B On"
    } else {
    MIDI.sendNoteOff(octaveSelected[octButtonPushCounter][11], 0, 1); //"Note B Off"
    }
    button12_B_valueOld = button12_B_valueNew;
  }

    ////////////////// C //////////////    
  if (button13_C_valueNew != button13_C_valueOld){ 
    if (button13_C_valueNew == HIGH){  
    //MIDI.sendNoteOn(60, 127, 1); //  "Note C On"
    MIDI.sendNoteOn(octaveSelected[octButtonPushCounter + 1][0], 127, 1); //  "Note C On" *One octave higher than low C, 
                                                                          //  so add + 1 to octButtonPushCounter
    } else {
    MIDI.sendNoteOff(octaveSelected[octButtonPushCounter + 1][0], 0, 1); //"Note F Off"
    }
    button13_C_valueOld = button13_C_valueNew;
  }
    /********************************************************************END_OF_KEYBOARD_CODE_LOOP_*****************************/    

    /********************************************************************_OCTAVE_BUTTONS_CODE_LOOP_*****************************/ 
  octUpButtonState = digitalRead(octUpButton);       // read the pushbutton up input pin:  
  if (octUpButtonState != octLastButtonState) {      // compare the buttonState to its previous state
    if (octUpButtonState == HIGH)                    // if the state has changed, increment the counter  
    {
      if (octButtonPushCounter == 0){                 // ********  Octave 1  *******
        octButtonPushCounter++;
        octaveSelected[1][0];
        delay(250);
        analogWrite(octLED, octLED_Brightness[1]);    // change LED brightness UP
      } else if (octButtonPushCounter == 1){          // ********  Octave 2  *******
          octButtonPushCounter++;
          octaveSelected[2][0];
          delay(250);
          analogWrite(octLED, octLED_Brightness[2]);  // change LED brightness UP
      } else if (octButtonPushCounter == 2){          // ********  Octave 3  *******
          octButtonPushCounter++;
          octaveSelected[3][0];
          delay(250);
          analogWrite(octLED, octLED_Brightness[3]);  // change LED brightness UP
      } else if (octButtonPushCounter == 3){          // ********  Octave 4  *******
          octButtonPushCounter++;
          octaveSelected[4][0];
          delay(250);
          analogWrite(octLED, octLED_Brightness[4]);  // change LED brightness UP
      } else if (octButtonPushCounter == 4){          // ********  Octave 5  *******
          octButtonPushCounter++;
          octaveSelected[5][0];
          delay(250);
          analogWrite(octLED, octLED_Brightness[5]);  // change LED brightness UP
      }    
    }
  }  
  octLastButtonState = octUpButtonState;             // save the current state as the last state, for next time through the loop  
  
  octDwnButtonState = digitalRead(octDwnButton);     // read the pushbutton down input pin:
  if (octDwnButtonState != octLastButtonState) {     // compare the buttonState to its previous state
    if (octDwnButtonState == HIGH)                   // if the state has changed, decrement the counter
    {
      if (octButtonPushCounter == 5){                 // ********  Octave 4  *******
      octButtonPushCounter--;
      octaveSelected[4][0];
      delay(250);
      analogWrite(octLED, octLED_Brightness[4]);      // change LED brightness DOWN     
      } else if (octButtonPushCounter == 4){          // ********  Octave 3  *******
          octButtonPushCounter--;
          octaveSelected[3][0];
          delay(250);
          analogWrite(octLED, octLED_Brightness[3]);  // change LED brightness DOWN 
      } else if (octButtonPushCounter == 3){          // ********  Octave 2  *******
          octButtonPushCounter--;
          octaveSelected[2][0];
          delay(250);
          analogWrite(octLED, octLED_Brightness[2]);  // change LED brightness DOWN 
      } else if (octButtonPushCounter == 2){          // ********  Octave 1  *******
          octButtonPushCounter--;
          octaveSelected[1][0];
          delay(250);
          analogWrite(octLED, octLED_Brightness[1]);  // change LED brightness DOWN
      } else if (octButtonPushCounter == 1){          // ********  Octave 0  *******
          octButtonPushCounter--;
          octaveSelected[0][0];
          delay(250);
          analogWrite(octLED, octLED_Brightness[0]);  // change LED brightness DOWN
      }
    }           
  }
  octLastButtonState = octDwnButtonState;    
   /********************************************************************END_OF_OCTAVE_BUTTONS_CODE_LOOP_**********************/ 


   /***********************************************START_OF_ARP_CODE_LOOP_*************/

  MIDI.read();


  cli();
  tick = millis();
  sei();

  handleButtonOne();
  handleButtonTwo();
  //  handleButtonThree();

  // if not in  clock synch mode, we just read the tempo from the
  // tempo knob. 
  if (!clockSync) {  
    // There's no need to be precise about it here. This simple 
    // calculation is done quickly and gives a very wide range.
    tempo = 6000 / ((127-analogRead(arpTempPot)/8) + 2);   // changed analogRead(1) to analogRead(7)to analogRead(arpTempPot)
    handleTick(tick);
  }

}

  /*************************************************END_OF_ARP_CODE_LOOP_*************/

  void handleTick(unsigned long tick) {

    // leave the LED long enough to be brightish but not so long
    // that it ends up being solid instead of blinking
    if (blinkOn && tick - blinkTime > 10) {
      blinkOn=false;
      digitalWrite(velPot,HIGH); // stupid midi shield has high/low backwards for the LEDs
    }
    if (clockSync || tick - lastTime > tempo) {
      blinkTime = lastTime = tick;
      digitalWrite(velPot,LOW); // stupid midi shield has high/low backwards for the LEDs
      blinkOn = true;


      if ((hold || notesHeld > 0) && notes[0] != '\0') { 


        // stop the previous note
        // MIDI.sendNoteOff(notes[playBeat],0,CHANNEL);

        // fixes a bug where a random note would sometimes get played when switching chords
        if (notes[playBeat] == '\0')
          playBeat = 0;

        // play the current note
        MIDI.sendNoteOn(notes[playBeat],velocity(),CHANNEL);

        // decide what the next note is based on the mode.
        if (mode == UP) 
          up();
        else if (mode == DOWN) 
          down();
        else if (mode == BOUNCE) 
          bounce();
        else if (mode == UPDOWN) 
          upDown();
        else if (mode == ONETHREE)
          oneThree();
        else if (mode == ONETHREEEVEN)
          oneThreeEven();

      }
    }
  }


  int velocity() {
    int velocity = 127 - analogRead(velPot)/8; // changed analogRead(0) to analogRead(6) to analogRead(velPot)

    // don't let it totally zero out.
    if (velocity == 0)
      velocity++;

    return velocity;

  }

  void up() {
    playBeat++;
    if (notes[playBeat] == '\0')
      playBeat=0;     
  }

  void down() {
    if (playBeat == 0) {
      playBeat = sizeof(notes)-1;
      while (notes[playBeat] == '\0') {
        playBeat--;
      }
    }        
    else       
      playBeat--;
  }


  void bounce() {
    if (sizeof(notes) == 1)
      playBeat=0;
    else
      if (arpUp) {
        if (notes[playBeat+1] == '\0') {
          arpUp = false;           
          playBeat--;
        }    
        else
          playBeat++;   
      }
      else {
        if (playBeat == 0) {
          arpUp = true;
          playBeat++;
        }
        else
          playBeat--;
      }
  }


  void upDown() {
    if (sizeof(notes) == 1)
      playBeat=0;
    else
      if (arpUp) {
        if (notes[playBeat+1] == '\0') {
          arpUp = false;           
        }    
        else
          playBeat++;   
      }
      else {
        if (playBeat == 0) {
          arpUp = true;
        }
        else
          playBeat--;
      }
  }

  void oneThree() {
    if (arpUp)
      playBeat += 2;
    else
      playBeat--;

    arpUp = !arpUp;

    if (notes[playBeat] == '\0') {
      playBeat = 0;
      arpUp = true;
    }
  }

  void oneThreeEven() {

    if (notes[playBeat+1] == '\0') {
      playBeat = 0;
      arpUp = true;
      return;
    }


    if (arpUp)
      playBeat += 2;
    else
      playBeat--;

    arpUp = !arpUp;


  }

  // empties out the arpeggio. used when switching modes, when in hold mode and
  // a new arpeggio is started, or when the reset button is pushed.
  void resetNotes() {
    for (int i = 0; i < sizeof(notes); i++)
      notes[i] = '\0';
  }


  // Basic code to read the buttons.
  char button(char button_num)
  {
    return (!(digitalRead(button_num)));
  }


  // Button one is the the Hold button.
  void handleButtonOne() {
    boolean buttonOnePressed = button(BUTTON1);
    if (buttonOnePressed) {
      if (buttonOneHeldTime == 0)
        buttonOneHeldTime = tick;

      if (!buttonOneDown && (tick - buttonOneHeldTime > debounceTime)) {
        buttonOneDown = true;
        hold = !hold;
        resetNotes();
        //delay(250);
        //MIDI.sendNoteOff(notes[playBeat],0,CHANNEL);
        MIDI.sendControlChange(123,0,1); // Added this to clear the hanging note
      }
    }
    else {
      buttonOneDown = false;
      buttonOneHeldTime = 0;
    }
  }

  // Button two is the mode button.
  void handleButtonTwo() {
    boolean buttonTwoPressed = button(BUTTON2);
    if (buttonTwoPressed) {
      if (buttonTwoHeldTime == 0)
        buttonTwoHeldTime = tick;

      if (!buttonTwoDown && (tick - buttonTwoHeldTime > debounceTime)) {
        buttonTwoDown = true;
        playBeat=0;
        mode++;
        if (mode == MODES) {
          mode=0;
        }
        arpUp = true;

      }
    }
    else {
      buttonTwoDown = false;
      buttonTwoHeldTime = 0;
    }
  }

  // button three is the reset button
  //void handleButtonThree() {
  //  boolean buttonThreePressed = button(BUTTON3);
  //  if (buttonThreePressed) {
  //    if (buttonThreeHeldTime == 0)
  //      buttonThreeHeldTime = tick;
  //
  //    if (!buttonThreeDown && (tick - buttonThreeHeldTime > debounceTime)) {
  //
  //      buttonThreeDown = true;
  //      resetNotes();
  //    }
  //  }
  //  else {
  //    buttonThreeDown = false;
  //    buttonThreeHeldTime = 0;
  //  }
  //}


//}