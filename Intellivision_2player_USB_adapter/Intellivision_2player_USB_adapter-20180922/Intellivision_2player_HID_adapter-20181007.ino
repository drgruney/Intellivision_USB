#include <Joystick.h>

#include "Adafruit_MCP23017.h"
Adafruit_MCP23017 mcp;

// Create Joysticks
#define JOYSTICK_COUNT 1
Joystick_ Joystick[JOYSTICK_COUNT] = { 
  Joystick_(0x03, JOYSTICK_TYPE_GAMEPAD,
    32, 0,                  // Button Count, Hat Switch Count
    false, false, false,     // X and Y, but no Z Axis
    false, false, false,   // No Rx, Ry, or Rz
    false, false,          // No rudder or throttle
    false, false, false),  // No accelerator, brake, or steering
/*  Joystick_(0x04, JOYSTICK_TYPE_GAMEPAD,
    32, 0,                  // Button Count, Hat Switch Count
    false, false, false,     // X and Y, but no Z Axis
    false, false, false,   // No Rx, Ry, or Rz
    false, false,          // No rudder or throttle
    false, false, false)  // No accelerator, brake, or steering
*/
};
// controller 1 is A
// controller 2 is B

#define DIRMASK B11110001
#define ACTIONMASK B00001110
#define KEYMASK B11111110

// assigns controller1 keystrokes to byteA against KEYMASK
int numPadA[12] = {
  B00011000, // 1
  B00010100, // 2
  B00010010, // 3
  B00101000, // 4
  B00100100, // 5
  B00100010, // 6
  B01001000, // 7
  B01000100, // 8
  B01000010, // 9
  B10001000, // Clear
  B10000100, // 0
  B10000010  // Enter
};
int numKey[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 9, 11};

int actPadA[3] = {
  B00001010, // top
  B00000110, // left
  B00001100  // right
};
int actKeyA[3] = {12, 13, 14};

int dirPadA[16] = {
  B01000000, // N
  B01000001, // NNE
  B01100001, // NE
  B01100000, // ENE
  B00100000, // E
  B00100001, // ESE
  B00110001, // SE
  B00110000, // SSE
  B00010000, // S
  B00010001, // SSW
  B10010001, // SW
  B10010000, // WSW
  B10000000, // W
  B10000001, // WNW
  B11000001, // NW
  B11000000  // NNW
};
int dirKeyA[16] = {15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30};

int numPadB[12] = {
  B00011000, // 1
  B00010100, // 2
  B00010010, // 3
  B00101000, // 4
  B00100100, // 5
  B00100010, // 6
  B01001000, // 7
  B01000100, // 8
  B01000010, // 9
  B10001000, // Clear
  B10000100, // 0
  B10000010  // Enter
};
int numKeyB[12] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 9, 11};

int i=0;       
int pinMapA[8]={4,5,6,7,0,1,2,3}; // map MCP pins to proper order for Controller 1
int pinMapB[8]={11,10,9,8,15,14,13,12}; // map MCP pins to proper order for Controller 2

// create input bytes for controllers 1 and 2
byte inputA = B00000000; // variable to store input A of MCP23017
byte inputB = B00000000; // variable to store input B of MCP23017

byte oldinputA = B00000000; // variable to store old input value A
byte oldinputB = B00000000; // variable to store old input value B

byte stateA = B00000000; // variable to store current state of A
byte stateB = B00000000; // variable to store current state of A

int b = 0;
int t = 0;
int numA = 32;
int numB = 32;
int oldnumA = 0;
int oldnumB = 0;
int buttonStateA = 0;
int buttonStateB = 0;

void setup() {
  for (int index = 0; index < JOYSTICK_COUNT; index++)
  
Joystick[index].begin();

Serial.begin(9600); // wake serial connection
mcp.begin(); // wake wire and set MCP to default address 0

// set all MCP pins to input with pullup resistor enabled
for(int t=0; t<16; t++) {
   mcp.pinMode(t, INPUT);
   mcp.pullUp(t, HIGH);
}
}

void readPads() {
delay(25); // debounce

inputA = 0; // clear byte
inputB = 0; // clear byte
// read pins assigned to controller 1
for(int t=0; t<8; t++){
  inputA |= ((mcp.digitalRead(pinMapA[t])==LOW ? 1:0)<<(7-t));
  }

// read pins assigned to controller 2
for(int t=0; t<8; t++){
  inputB |= ((mcp.digitalRead(pinMapB[t])==LOW ? 1:0)<<(7-t));
  }
}


void loop() {

readPads();  

 if (inputA != oldinputA) {
    Joystick[0].setButton(0,0);
    Joystick[0].setButton(1,0);
    Joystick[0].setButton(2,0);
    Joystick[0].setButton(3,0);
    Joystick[0].setButton(4,0);
    Joystick[0].setButton(5,0);
    Joystick[0].setButton(6,0);
    Joystick[0].setButton(7,0);
    Joystick[0].setButton(8,0);
    Joystick[0].setButton(9,0);
    Joystick[0].setButton(10,0);
    Joystick[0].setButton(11,0);
    Joystick[0].setButton(12,0);
    Joystick[0].setButton(13,0);
    Joystick[0].setButton(14,0);
    Joystick[0].setButton(15,0);
    Joystick[0].setButton(16,0);
    Joystick[0].setButton(17,0);
    Joystick[0].setButton(18,0);
    Joystick[0].setButton(19,0);
    Joystick[0].setButton(20,0);
    Joystick[0].setButton(21,0);
    Joystick[0].setButton(22,0);
    Joystick[0].setButton(23,0);
    Joystick[0].setButton(24,0);
    Joystick[0].setButton(25,0);
    Joystick[0].setButton(26,0);
    Joystick[0].setButton(27,0);
    Joystick[0].setButton(28,0);
    Joystick[0].setButton(29,0);
    Joystick[0].setButton(30,0);
    Joystick[0].setButton(31,0);
  readPads();
  oldinputA = inputA;
}


buttonStateA = inputA;
buttonStateB = inputB;

switch (buttonStateA & ACTIONMASK) { // side buttons
  case B00001010: Joystick[0].setButton(12,1); break;
  case B00000110: Joystick[0].setButton(13,1); break;
  case B00001100: Joystick[0].setButton(14,1); break;
}

// controller 1 (telephone) buttons
  bool pressed = 0;
  for (int t = 0; t < 12; t++) {
    if ((buttonStateA & numPadA[t]) == numPadA[t]) {
      Joystick[0].setButton(t,1);  
    pressed = 1;
    }
  }
  if (pressed == 0) {
    switch (buttonStateA & DIRMASK) { // disc directions
      case B01000000: Joystick[0].setButton(15,1); break;
      case B01000001: Joystick[0].setButton(16,1); break;
      case B01100001: Joystick[0].setButton(17,1); break;
      case B01100000: Joystick[0].setButton(18,1); break;
      case B00100000: Joystick[0].setButton(19,1); break;
      case B00100001: Joystick[0].setButton(20,1); break;
      case B00110001: Joystick[0].setButton(21,1); break;
      case B00110000: Joystick[0].setButton(22,1); break;
      case B00010000: Joystick[0].setButton(23,1); break;
      case B00010001: Joystick[0].setButton(24,1); break;
      case B10010001: Joystick[0].setButton(25,1); break;
      case B10010000: Joystick[0].setButton(26,1); break;
      case B10000000: Joystick[0].setButton(27,1); break;
      case B10000001: Joystick[0].setButton(28,1); break;
      case B11000001: Joystick[0].setButton(29,1); break;
      case B11000000: Joystick[0].setButton(30,1); break;
    }
  }
}

