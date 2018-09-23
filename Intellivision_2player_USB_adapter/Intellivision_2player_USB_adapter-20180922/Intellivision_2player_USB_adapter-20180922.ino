/* This program will allow an Arduino Micro
along with an MCP23017 connect to two Intellivision 
controllers for use with the jzint emulator.

It is built off of spinal's Intellivision Controller Adapter 
on socoder.net (https://socoder.net/?Blogs=55773)

The biggest change over my previous Intellivision_USB_adapter
project is that this allows for two controllers to operate simultaneously.

Each bank of the MCP23017 corresponds to one controller. The MCP23017 
is configured to operate as GPIO pins with the Adafruit's MCP23017 library.
Each bank creates it's own byte value for its respective controller.
Input is read and executed sequentially and not simultaneously.
So technically Player 2 is at a constant disadvantage, but I do not believe
it to be serious.

Use the following in the keyboard hack file for jzint

; -----------------------------------------------------------------------------------------
MAP 0      ; keymap 0 (default keymap)
; -----------------------------------------------------------------------------------------

DELETE QUIT
BACKSPACE RESET

; ***** Pad #0 - Left Controller *****
; ** Side Buttons and Numeric Keypad **
f PD0L_A_T
s PD0L_A_L
d PD0L_A_R
! PD0L_KP1
" PD0L_KP2
# PD0L_KP3
$ PD0L_KP4
& PD0L_KP5
( PD0L_KP6
) PD0L_KP7
* PD0L_KP8
+ PD0L_KP9
: PD0L_KPC
< PD0L_KP0
> PD0L_KPE

; ** Direction Disc **
e    PD0L_D_N
r    PD0L_D_NNE
t    PD0L_D_ENE
g    PD0L_D_E
b    PD0L_D_ESE
v    PD0L_D_SSE
c    PD0L_D_S
x    PD0L_D_SSW
z    PD0L_D_WSW
a    PD0L_D_W
q    PD0L_D_WNW
w    PD0L_D_NNW

; ***** Pad #1 - Right Controller *****
; ** Side Buttons and Numeric Keypad **
l PD0R_A_T
j PD0R_A_L
k PD0R_A_R
1 PD0R_KP1
2 PD0R_KP2
3 PD0R_KP3
4 PD0R_KP4
5 PD0R_KP5
6 PD0R_KP6
7 PD0R_KP7
8 PD0R_KP8
9 PD0R_KP9
- PD0R_KPC
0 PD0R_KP0
= PD0R_KPE


; ** Direction Disc **
i    PD0R_D_N
o    PD0R_D_NNE
p    PD0R_D_ENE
;    PD0R_D_E
/    PD0R_D_ESE
.    PD0R_D_SSE
,    PD0R_D_S
m    PD0R_D_SSW
n    PD0R_D_WSW
h    PD0R_D_W
y    PD0R_D_WNW
u    PD0R_D_NNW
*/


// library used for keyboard output over USB
#include <Keyboard.h>
#include "Adafruit_MCP23017.h"
Adafruit_MCP23017 mcp;

/*   Controller 1             Controller 2
    +---+---+---+            +---+---+---+       
  F | ! | " | # | F        L | 1 | 2 | 3 | L
    +---+---+---+            +---+---+---+
  S | $ | & | ( | D        J | 4 | 5 | 6 | K
    +---+---+---+            +---+---+---+
    | ) | * | + |            | 7 | 8 | 9 |
    +---+---+---+            +---+---+---+
    | : | < | > |            | - | 0 | = |
    +---+---+---+            +---+---+---+
        w e r                    u i o
      q   |   t               y    |    p
      a---+---g               h ---+--- ;
      z   |   b               n    |    /
        x c v                    m , .
  No 45 degree angles, the intellivision only pretended to have 16 directions, it was really 12.
*/
// masking and button-direction byte values from spinal at socoder.net
#define DIRMASK B11110001
#define ACTIONMASK B00001110
#define KEYMASK B1111110

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
//int numKeyA[12] = {225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 224};
int numKeyA[12] = {33, 34, 35, 36, 38, 40, 41, 42, 43, 58, 60, 62};
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
int numKeyB[12] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '0', '='};

// controller 1 is A
// controller 2 is B
int i=0;       
int pinMapA[8]={4,5,6,7,0,1,2,3}; // map MCP pins to proper order for Controller 1
int pinMapB[8]={11,10,9,8,15,14,13,12}; // map MCP pins to proper order for Controller 2


// create input bytes for controllers 1 and 2
byte inputA = B00000000; // variable to store input A of MCP23017
byte inputB = B00000000; // variable to store input B of MCP23017

void setup() {
Serial.begin(9600); // wake serial connection
mcp.begin(); // wake wire and set MCP to default address 0

// set all MCP pins to input with pullup resistor enabled
for(int t=0; t<16; t++) {
   mcp.pinMode(t, INPUT);
   mcp.pullUp(t, HIGH);
}

//configure input pin to close game and go back to menu.
pinMode(2, INPUT_PULLUP);
}

void loop() {
delay(20); // debounce 

Keyboard.releaseAll();

//allows a single button to perform reset and exit emulator.
//tap and wait for reset, hold and wait for exit.
if (digitalRead(2)==LOW){
  delay(750);
  if (digitalRead(2)==LOW){
    Keyboard.press(8);
  }
  else {
  Keyboard.press(127);
  }  
}

inputA = 0; //clear byte
inputB = 0; //clear byte

for(int t=0; t<8; t++){
  inputA |= ((mcp.digitalRead(pinMapA[t])==LOW ? 1:0)<<(7-t));
}

for(int t=0; t<8; t++){
  inputB |= ((mcp.digitalRead(pinMapB[t])==LOW ? 1:0)<<(7-t));
}

// read side buttons for controller 1
switch (inputA & ACTIONMASK) {
  case B00001010: Keyboard.press('f'); break;
  case B00000110: Keyboard.press('s'); break;
  case B00001100: Keyboard.press('d'); break;
}
// read side buttons for controller 2
switch (inputB & ACTIONMASK) {
  case B00001010: Keyboard.press('l'); break;
  case B00000110: Keyboard.press('j'); break;
  case B00001100: Keyboard.press('k'); break;  
} 

// controller 1 (telephone) buttons
  bool pressedA = 0;
  for (int t = 0; t < 12; t++) {
  if ((inputA & numPadA[t]) == numPadA[t]) {
      Keyboard.press(numKeyA[t]);
      pressedA = 1;
    }
  }

// controller 1 direction
  if (pressedA == 0) {
    switch (inputA & DIRMASK) { // disc directions
      case B01000000: Keyboard.press('e'); break;
      case B01000001: Keyboard.press('r'); break;
      case B01100001: Keyboard.press('r');
        Keyboard.press('t'); break;
      case B01100000: Keyboard.press('t'); break;
      case B00100000: Keyboard.press('g'); break;
      case B00100001: Keyboard.press('b'); break;
      case B00110001: Keyboard.press('b');
        Keyboard.press('v'); break;
      case B00110000: Keyboard.press('v'); break;
      case B00010000: Keyboard.press('c'); break;
      case B00010001: Keyboard.press('x'); break;
      case B10010001: Keyboard.press('x');
        Keyboard.press('z'); break;
      case B10010000: Keyboard.press('z'); break;
      case B10000000: Keyboard.press('a'); break;
      case B10000001: Keyboard.press('q'); break;
      case B11000001: Keyboard.press('q');
        Keyboard.press('w'); break;
      case B11000000: Keyboard.press('w'); break;
    }
  }
// controller 2 (telephone) buttons
  bool pressedB = 0;
  for (int t = 0; t < 12; t++) {
    if ((inputB & numPadB[t]) == numPadB[t]) {
      Keyboard.press(numKeyB[t]);
      pressedB = 1;
    }
  }

// controller 2 direction
  if (pressedB == 0) {
    switch (inputB & DIRMASK) { // disc directions
      case B01000000: Keyboard.press('i'); break;
      case B01000001: Keyboard.press('o'); break;
      case B01100001: Keyboard.press('p');
        Keyboard.press('p'); break;
      case B01100000: Keyboard.press('p'); break;
      case B00100000: Keyboard.press('g'); break;
      case B00100001: Keyboard.press('/'); break;
      case B00110001: Keyboard.press('/');
        Keyboard.press('.'); break;
      case B00110000: Keyboard.press('.'); break;
      case B00010000: Keyboard.press(','); break;
      case B00010001: Keyboard.press('m'); break;
      case B10010001: Keyboard.press('m');
        Keyboard.press('n'); break;
      case B10010000: Keyboard.press('n'); break;
      case B10000000: Keyboard.press('h'); break;
      case B10000001: Keyboard.press('y'); break;
      case B11000001: Keyboard.press('y');
        Keyboard.press('u'); break;
      case B11000000: Keyboard.press('u'); break;
    }
  }

}
