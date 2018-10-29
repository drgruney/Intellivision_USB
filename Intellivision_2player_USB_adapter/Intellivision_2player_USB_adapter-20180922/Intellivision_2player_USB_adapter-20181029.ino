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
Each input is read separately from the other and n-key rollover in the library means input 
from one controller will not interfere with the other.

Use the following in the keyboard hack file for jzint

No modifier keys are used to keep input from one controller causing unwanted/unexpected 
input on the other controller.
Many thanks go to user 'johnwasser' at the Ardiuno.cc forums for figuring out KP key values
http://forum.arduino.cc/index.php?topic=179548.0

; -----------------------------------------------------------------------------------------
MAP 0      ; keymap 0 (default keymap)
; -----------------------------------------------------------------------------------------

DELETE QUIT
BACKSPACE RESET

; ***** Pad #0 - Left Controller *****
; ** Side Buttons and Numeric Keypad **
f             PD0L_A_T
s             PD0L_A_L
d             PD0L_A_R
1             PD0L_KP1
2             PD0L_KP2
3             PD0L_KP3
4             PD0L_KP4
5             PD0L_KP5
6             PD0L_KP6
7             PD0L_KP7
8             PD0L_KP8
9             PD0L_KP9
-             PD0L_KPC
0             PD0L_KP0
=             PD0L_KPE

; ** Direction Disc **
e             PD0L_D_N
r             PD0L_D_NNE
]             PD0L_D_NE
t             PD0L_D_ENE
g             PD0L_D_E
b             PD0L_D_ESE
\             PD0L_D_SE
v             PD0L_D_SSE
c             PD0L_D_S
x             PD0L_D_SSW
`             PD0L_D_SW
z             PD0L_D_WSW
a             PD0L_D_W
q             PD0L_D_WNW
[             PD0L_D_NW
w             PD0L_D_NNW

; ***** Pad #1 - Right Controller *****
; ** Side Buttons and Numeric Keypad **
l             PD0R_A_T
j             PD0R_A_L
k             PD0R_A_R
KP7           PD0R_KP1
KP8           PD0R_KP2
KP9           PD0R_KP3
KP4           PD0R_KP4
KP5           PD0R_KP5
KP6           PD0R_KP6
KP1           PD0R_KP7
KP2           PD0R_KP8
KP3           PD0R_KP9
KP0           PD0R_KPC
KP_PERIOD     PD0R_KP0
KP_ENTER      PD0R_KPE


; ** Direction Disc **
i             PD0R_D_N
o             PD0R_D_NNE
KP_DIVIDE     PD0R_D_NE
p             PD0R_D_ENE
'             PD0R_D_E
/             PD0R_D_ESE
KP_MULTIPLY   PD0R_D_SE
.             PD0R_D_SSE
,             PD0R_D_S
m             PD0R_D_SSW
KP_MINUS      PD0R_D_SW
n             PD0R_D_WSW
h             PD0R_D_W
y             PD0R_D_WNW
KP_PLUS       PD0R_D_NW
u             PD0R_D_NNW
*/


// library used for keyboard output over USB
#include <Keyboard.h>
#include "Adafruit_MCP23017.h"
Adafruit_MCP23017 mcp;

/*   Controller 1             Controller 2
    +---+---+---+            +---+---+---+       
  f | 1 | 2 | 3 | f        l |kp7|kp8|kp9| l
    +---+---+---+            +---+---+---+
  s | 4 | 5 | 6 | d        j |kp4|kp5|kp6| k
    +---+---+---+            +---+---+---+
    | 7 | 8 | 9 |            |kp1|kp2|kp3|
    +---+---+---+            +---+---+---+
    | - | 0 | = |            |kp0|kp.|kpE|
    +---+---+---+            +---+---+---+
     [  w e r  ]            kp+  u i o  kp/
      q   |   t               y    |    p
      a---+---g               h ---+--- '
      z   |   b               n    |    /
     `  x c v  \            kp-  m , .  kp*
*/
// masking and button-direction byte values from spinal at socoder.net
#define DIRMASK B11110001
#define ACTIONMASK B00001110
#define KEYMASK B11111100

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
int numKeyA[12] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '0', '='};
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
int numKeyB[12] = {231, 232, 233, 228, 229, 230, 225, 226, 227, 234, 235, 224};

// controller 1 is A
// controller 2 is B
int i=0;       
int pinMapA[8]={4,5,6,7,0,1,2,3}; // map MCP pins to proper order for Controller 1
int pinMapB[8]={11,10,9,8,15,14,13,12}; // map MCP pins to proper order for Controller 2


// create input bytes for controllers 1 and 2
byte inputA = B00000000; // variable to store input A of MCP23017
byte inputB = B00000000; // variable to store input B of MCP23017

byte oldinputA = B00000000; // variable to store old input value A
byte oldinputB = B00000000; // variable to store old input value B

byte buttonStateA = B00000000; // variable to store current state of A
byte buttonStateB = B00000000; // variable to store current state of A

void setup() {
Serial.begin(9600); // wake serial connection
mcp.begin(); // wake wire and set MCP to default address 0

// set all MCP pins to input with pullup resistor enabled
for(int t=0; t<16; t++) {
   mcp.pinMode(t, INPUT);
   mcp.pullUp(t, HIGH);
}

//configure input pin to close game and go back to menu.
pinMode(4, INPUT_PULLUP);
}

void readPadA() {

inputA = 0; //clear byte

for(int t=0; t<8; t++){
inputA |= ((mcp.digitalRead(pinMapA[t])==LOW ? 1:0)<<(7-t));
}
}

void readPadB() {

inputB = 0; //clear byte

for(int t=0; t<8; t++){
inputB |= ((mcp.digitalRead(pinMapB[t])==LOW ? 1:0)<<(7-t));
}
}

void loop() {
  delay(20); // debounce
  readPadA();
  if (inputA != oldinputA) {
    Keyboard.release('s');
    Keyboard.release('d');
    Keyboard.release('f');
    Keyboard.release('e');
    Keyboard.release('r');
    Keyboard.release('t');
    Keyboard.release('g');
    Keyboard.release('b');
    Keyboard.release('v');
    Keyboard.release('c');
    Keyboard.release('x');
    Keyboard.release('z');
    Keyboard.release('a');
    Keyboard.release('q');
    Keyboard.release('w');
    Keyboard.release(92);
    Keyboard.release('`');
    Keyboard.release('[');
    Keyboard.release(']');
    Keyboard.release('1');
    Keyboard.release('2');
    Keyboard.release('3');
    Keyboard.release('4');
    Keyboard.release('5');
    Keyboard.release('6');
    Keyboard.release('7');
    Keyboard.release('8');
    Keyboard.release('9');
    Keyboard.release('0');
    Keyboard.release('-');
    Keyboard.release('=');
    readPadA();
    oldinputA = inputA;
    }

  readPadB();
  if (inputB != oldinputB) {
    Keyboard.release('i');
    Keyboard.release('o');
    Keyboard.release('p');
    Keyboard.release(39);
    Keyboard.release('/');
    Keyboard.release('.');
    Keyboard.release(',');
    Keyboard.release('m');
    Keyboard.release('n');
    Keyboard.release('h');
    Keyboard.release('y');
    Keyboard.release('u');
    Keyboard.release('j');
    Keyboard.release('k');
    Keyboard.release('l');
    Keyboard.release(220);
    Keyboard.release(221);
    Keyboard.release(222);
    Keyboard.release(223);
    Keyboard.release(224);
    Keyboard.release(225);
    Keyboard.release(226);
    Keyboard.release(227);
    Keyboard.release(228);
    Keyboard.release(229);
    Keyboard.release(230);
    Keyboard.release(231);
    Keyboard.release(232);
    Keyboard.release(233);
    Keyboard.release(234);
    Keyboard.release(235);
    readPadB();
    oldinputB = inputB;
    }
  
  buttonStateA = inputA;
  buttonStateB = inputB;

//allows a single button to perform reset and exit emulator.
//tap and wait for reset, hold and wait for exit.
if (digitalRead(4)==LOW){
  delay(750);
  if (digitalRead(4)==LOW){
    Keyboard.press(8);
  }
  else {
  Keyboard.press(127);
  }  
}

// read side buttons for controller 1
switch (buttonStateA & ACTIONMASK) { // side buttons
  case B00001010: Keyboard.press('f'); break;
  case B00000110: Keyboard.press('s'); break;
  case B00001100: Keyboard.press('d'); break;
}
// read side buttons for controller 2
switch (buttonStateB & ACTIONMASK) { // side buttons
  case B00001010: Keyboard.press('l'); break;
  case B00000110: Keyboard.press('j'); break;
  case B00001100: Keyboard.press('k'); break;  
} 

// controller 1 (telephone) buttons
  bool pressedA = 0;
  for (int t = 0; t < 12; t++) {
  if ((buttonStateA & numPadA[t]) == numPadA[t]) {
      Keyboard.press(numKeyA[t]);
      pressedA = 1;
    }
  }

// controller 1 direction
  if (pressedA == 0) {
    switch (buttonStateA & DIRMASK) { // disc directions
      case B01000000: Keyboard.press('e'); break;
      case B01000001: Keyboard.press('r'); break;
      case B01100001: Keyboard.press(']'); break;
      case B01100000: Keyboard.press('t'); break;
      case B00100000: Keyboard.press('g'); break;
      case B00100001: Keyboard.press('b'); break;
      case B00110001: Keyboard.press(92); break;
      case B00110000: Keyboard.press('v'); break;
      case B00010000: Keyboard.press('c'); break;
      case B00010001: Keyboard.press('x'); break;
      case B10010001: Keyboard.press('`'); break;
      case B10010000: Keyboard.press('z'); break;
      case B10000000: Keyboard.press('a'); break;
      case B10000001: Keyboard.press('q'); break;
      case B11000001: Keyboard.press('['); break;
      case B11000000: Keyboard.press('w'); break;
    }
  }
// controller 2 (telephone) buttons
  bool pressedB = 0;
  for (int t = 0; t < 12; t++) {
    if ((buttonStateB & numPadB[t]) == numPadB[t]) {
      Keyboard.press(numKeyB[t]);
      pressedB = 1;
    }
  }

// controller 2 direction
  if (pressedB == 0) {
    switch (buttonStateB & DIRMASK) { // disc directions
      case B01000000: Keyboard.press('i'); break;
      case B01000001: Keyboard.press('o'); break;
      case B01100001: Keyboard.press('p'); break;
      case B01100000: Keyboard.press(220); break;
      case B00100000: Keyboard.press(39); break;
      case B00100001: Keyboard.press('/'); break;
      case B00110001: Keyboard.press(221); break;
      case B00110000: Keyboard.press('.'); break;
      case B00010000: Keyboard.press(','); break;
      case B00010001: Keyboard.press('m'); break;
      case B10010001: Keyboard.press(222); break;
      case B10010000: Keyboard.press('n'); break;
      case B10000000: Keyboard.press('h'); break;
      case B10000001: Keyboard.press('y'); break;
      case B11000001: Keyboard.press(223); break;
      case B11000000: Keyboard.press('u'); break;
    }
  }

}
