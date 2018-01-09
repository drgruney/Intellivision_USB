#include <Keyboard.h>

#define DIRMASK B11110001
#define ACTIONMASK B00001110
#define KEYMASK B1111110

int numPad[12] = {
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
int numKey[12] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', KEY_BACKSPACE, '0', KEY_RETURN};

/*
  +---+---+---+
  X | 1 | 2 | 3 | X
  +---+---+---+
  Y | 4 | 5 | 6 | Z
  +---+---+---+
  | 7 | 8 | 9 |
  +---+---+---+
  |BS | 0 |ENT|
  +---+---+---+
      p a b
    n   |   d
    m---+---e
    l   |   f
      j i h
  No 45 degree angles, the intellivision only pretended to have 16 directions, it was really 12.

*/

// Arduino Micro digital pins
int myPin[8]={5,4,3,2,9,8,7,6};


unsigned char wire;
unsigned char oldWire = B11111111;
unsigned char buttonState;

int counter = 0;

void setup() {

  // begin serial com
  Serial.begin(9600);

  // Set pins 2 to 9 as input and pull high
  for (int t = 0; t < 8; t++) {
    pinMode( myPin[t] , INPUT_PULLUP );
  }

// (Dan's Addition) Set pins 10-12 as input and pull high

pinMode(10, INPUT_PULLUP);
pinMode(11, INPUT_PULLUP);
pinMode(12, INPUT_PULLUP);

  // initialize control over thekeyboard:
  Keyboard.begin();
}

void readPad() {
delay(25); // debounce

  wire = 0; // clear byte
  for(int t=0; t<8; t++){
    wire |= ((digitalRead(myPin[t])==LOW ? 1:0)<<(7-t));
  }
  printBits(wire);
}

void loop() {

  readPad();
  if (wire != oldWire) {
    Keyboard.releaseAll();
    readPad();
    oldWire = wire;
  }
  buttonState = wire;

  switch (buttonState & ACTIONMASK) { // side buttons
    case B00001010: Keyboard.press('x'); break;
    case B00000110: Keyboard.press('y'); break;
    case B00001100: Keyboard.press('z'); break;
  }

  // keypad (telephone) buttons
  bool pressed = 0;
  for (int t = 0; t < 12; t++) {
    if ((buttonState & numPad[t]) == numPad[t]) {
      Keyboard.press(numKey[t]);
      pressed = 1;
    }
  }

  if (pressed == 0) {
    switch (buttonState & DIRMASK) { // disc directions
      case B01000000: Keyboard.press('a'); break;
      case B01000001: Keyboard.press('b'); break;
      case B01100001: Keyboard.press('b');
        Keyboard.press('d'); break;
      case B01100000: Keyboard.press('d'); break;
      case B00100000: Keyboard.press('e'); break;
      case B00100001: Keyboard.press('f'); break;
      case B00110001: Keyboard.press('f');
        Keyboard.press('h'); break;
      case B00110000: Keyboard.press('h'); break;
      case B00010000: Keyboard.press('i'); break;
      case B00010001: Keyboard.press('j'); break;
      case B10010001: Keyboard.press('j');
        Keyboard.press('l'); break;
      case B10010000: Keyboard.press('l'); break;
      case B10000000: Keyboard.press('m'); break;
      case B10000001: Keyboard.press('n'); break;
      case B11000001: Keyboard.press('n');
        Keyboard.press('p'); break;
      case B11000000: Keyboard.press('p'); break;
    }
  }

  if (digitalRead(10) == LOW) {Keyboard.press(KEY_F11);
  }
  if (digitalRead(11) == LOW) {Keyboard.press(KEY_F1);
  }
  if (digitalRead(12) == LOW) {Keyboard.press(KEY_F12);
  }

  if (digitalRead(10) == HIGH) {Keyboard.release(KEY_F11);
  }
  if (digitalRead(11) == HIGH) {Keyboard.release(KEY_F1);
  }
  if (digitalRead(12) == HIGH) {Keyboard.release(KEY_F12);
  }
}

void printBits(byte myByte) {
  for (byte mask = 0x80; mask; mask >>= 1) {
    if (mask  & myByte)
      Serial.print('1');
    else
      Serial.print('0');
  }
  Serial.println('.');
}
