/*
 LS-30 Rotary Joystick to MAME Interface
 using ATMega32u4 with a Pro Micro bootloader for USB keyboard emulation

  Reads the 12 rotary switches on 1 or 2 joysticks and detects when the position has changed.
  Generates a usb keyboard key press to indicate a clockwise or
  counter-clockwise movement has occurred.

  On a blank ATMega32u4 chip, first use the Arduino IDE to burn the Sparkfun Pro Micro bootloader 
  into the chip using another Arduino board as an ISP Programmer (SPI/reset pin connections between boards)

  Then to upload the sketch over USB like a regular Pro Micro board,
  In the Tools menu, choose the Sparkfun Pro Micro board (install the board support if needed)
  Choose the Port that appears when this joystick board is plugged into USB with a bootloader 
  (some Windows versions may need USB drivers installed) 
  Processor:  ATMega32u4 5V 16MHz version
  Programmer: AVRISP mkII

Tested with Arduino IDE 2.3.4
Uses library Bounce2 - tested with v2.71

Gadget Reboot
https://www.youtube.com/@gadgetreboot
*/

#include "Keyboard.h"
#include <Bounce2.h>

// keyboard characters sent for rotation
const char cw1_char = 'r';
const char ccw1_char = 'l';
const char cw2_char = 'x';
const char ccw2_char = 'z';

const byte debounceTime = 30;  // switch debounce in ms

const byte sw1a = 15;  // rotary switch pins on ATMega32u4 (Arduino digital pin numbers)
const byte sw1b = 16;
const byte sw1c = 14;
const byte sw1d = 8;
const byte sw1e = 9;
const byte sw1f = 10;
const byte sw2a = 5;
const byte sw2b = 3;
const byte sw2c = 2;
const byte sw2d = 4;
const byte sw2e = 6;
const byte sw2f = 7;

byte lastState1 = 0;  // previous stored joystick reading
byte curState1 = 0;   // current joystick reading to evaluate against last reading
byte lastState2 = 0;
byte curState2 = 0;

// create switch debouncer objects for inputs
Bounce debounced1a = Bounce();
Bounce debounced1b = Bounce();
Bounce debounced1c = Bounce();
Bounce debounced1d = Bounce();
Bounce debounced1e = Bounce();
Bounce debounced1f = Bounce();
Bounce debounced2a = Bounce();
Bounce debounced2b = Bounce();
Bounce debounced2c = Bounce();
Bounce debounced2d = Bounce();
Bounce debounced2e = Bounce();
Bounce debounced2f = Bounce();

void setup() {

  pinMode(sw1a, INPUT_PULLUP);
  debounced1a.attach(sw1a);            // setup the bounce instance
  debounced1a.interval(debounceTime);  // debounce interval in ms

  pinMode(sw1b, INPUT_PULLUP);
  debounced1b.attach(sw1b);            // setup the bounce instance
  debounced1b.interval(debounceTime);  // debounce interval in ms

  pinMode(sw1c, INPUT_PULLUP);
  debounced1c.attach(sw1c);            // setup the bounce instance
  debounced1c.interval(debounceTime);  // debounce interval in ms

  pinMode(sw1d, INPUT_PULLUP);
  debounced1d.attach(sw1d);            // setup the bounce instance
  debounced1d.interval(debounceTime);  // debounce interval in ms

  pinMode(sw1e, INPUT_PULLUP);
  debounced1e.attach(sw1e);            // setup the bounce instance
  debounced1e.interval(debounceTime);  // debounce interval in ms

  pinMode(sw1f, INPUT_PULLUP);
  debounced1f.attach(sw1f);            // setup the bounce instance
  debounced1f.interval(debounceTime);  // debounce interval in ms

  pinMode(sw2a, INPUT_PULLUP);
  debounced2a.attach(sw2a);            // setup the bounce instance
  debounced2a.interval(debounceTime);  // debounce interval in ms

  pinMode(sw2b, INPUT_PULLUP);
  debounced2b.attach(sw2b);            // setup the bounce instance
  debounced2b.interval(debounceTime);  // debounce interval in ms

  pinMode(sw2c, INPUT_PULLUP);
  debounced2c.attach(sw2c);            // setup the bounce instance
  debounced2c.interval(debounceTime);  // debounce interval in ms

  pinMode(sw2d, INPUT_PULLUP);
  debounced2d.attach(sw2d);            // setup the bounce instance
  debounced2d.interval(debounceTime);  // debounce interval in ms

  pinMode(sw2e, INPUT_PULLUP);
  debounced2e.attach(sw2e);            // setup the bounce instance
  debounced2e.interval(debounceTime);  // debounce interval in ms

  pinMode(sw2f, INPUT_PULLUP);
  debounced2f.attach(sw2f);            // setup the bounce instance
  debounced2f.interval(debounceTime);  // debounce interval in ms

  // take an initial reading of the rotary switches

  debounced1a.update();           // update the switch status
  if (debounced1a.read() == LOW)  // if a switch was grounded, update the status register
    bitSet(curState1, 0);

  debounced1b.update();           // update the switch status
  if (debounced1b.read() == LOW)  // if a switch was grounded, update the status register
    bitSet(curState1, 1);

  debounced1c.update();           // update the switch status
  if (debounced1c.read() == LOW)  // if a switch was grounded, update the status register
    bitSet(curState1, 2);

  debounced1d.update();           // update the switch status
  if (debounced1d.read() == LOW)  // if a switch was grounded, update the status register
    bitSet(curState1, 3);

  debounced1e.update();           // update the switch status
  if (debounced1e.read() == LOW)  // if a switch was grounded, update the status register
    bitSet(curState1, 4);

  debounced1f.update();           // update the switch status
  if (debounced1f.read() == LOW)  // if a switch was grounded, update the status register
    bitSet(curState1, 5);

  debounced2a.update();           // update the switch status
  if (debounced2a.read() == LOW)  // if a switch was grounded, update the status register
    bitSet(curState2, 0);

  debounced2b.update();           // update the switch status
  if (debounced2b.read() == LOW)  // if a switch was grounded, update the status register
    bitSet(curState2, 1);

  debounced2c.update();           // update the switch status
  if (debounced2c.read() == LOW)  // if a switch was grounded, update the status register
    bitSet(curState2, 2);

  debounced2d.update();           // update the switch status
  if (debounced2d.read() == LOW)  // if a switch was grounded, update the status register
    bitSet(curState2, 3);

  debounced2e.update();           // update the switch status
  if (debounced2e.read() == LOW)  // if a switch was grounded, update the status register
    bitSet(curState2, 4);

  debounced2f.update();           // update the switch status
  if (debounced2f.read() == LOW)  // if a switch was grounded, update the status register
    bitSet(curState2, 5);

  lastState1 = curState1;         // make last and current readings identical to avoid a false output trigger on power up
  lastState2 = curState2;

}  // end setup()

void loop() {
  readJoysticks();     // read the current rotary switch states into curState registers
  processJoysticks();  // determine if a rotation has occurred and send keystrokes if so
}

// read the joystick rotary switches into the curState register
void readJoysticks() {

  debounced1a.update();      // update the debouncer status
  if (debounced1a.fell()) {  // if a switch was grounded, update the status register
    curState1 = 0;           // clear the register and re-build it from current switch reading
    bitSet(curState1, 0);
  }

  debounced1b.update();      // update the debouncer status
  if (debounced1b.fell()) {  // if a switch was grounded, update the status register
    curState1 = 0;           // clear the register and re-build it from current switch reading
    bitSet(curState1, 1);
  }

  debounced1c.update();      // update the debouncer status
  if (debounced1c.fell()) {  // if a switch was grounded, update the status register
    curState1 = 0;           // clear the register and re-build it from current switch reading
    bitSet(curState1, 2);
  }

  debounced1d.update();      // update the debouncer status
  if (debounced1d.fell()) {  // if a switch was grounded, update the status register
    curState1 = 0;           // clear the register and re-build it from current switch reading
    bitSet(curState1, 3);
  }

  debounced1e.update();      // update the debouncer status
  if (debounced1e.fell()) {  // if a switch was grounded, update the status register
    curState1 = 0;           // clear the register and re-build it from current switch reading
    bitSet(curState1, 4);
  }

  debounced1f.update();      // update the debouncer status
  if (debounced1f.fell()) {  // if a switch was grounded, update the status register
    curState1 = 0;           // clear the register and re-build it from current switch reading
    bitSet(curState1, 5);
  }

  debounced2a.update();      // update the debouncer status
  if (debounced2a.fell()) {  // if a switch was grounded, update the status register
    curState2 = 0;           // clear the register and re-build it from current switch reading
    bitSet(curState2, 0);
  }

  debounced2b.update();      // update the debouncer status
  if (debounced2b.fell()) {  // if a switch was grounded, update the status register
    curState2 = 0;           // clear the register and re-build it from current switch reading
    bitSet(curState2, 1);
  }

  debounced2c.update();      // update the debouncer status
  if (debounced2c.fell()) {  // if a switch was grounded, update the status register
    curState2 = 0;           // clear the register and re-build it from current switch reading
    bitSet(curState2, 2);
  }

  debounced2d.update();      // update the debouncer status
  if (debounced2d.fell()) {  // if a switch was grounded, update the status register
    curState2 = 0;           // clear the register and re-build it from current switch reading
    bitSet(curState2, 3);
  }

  debounced2e.update();      // update the debouncer status
  if (debounced2e.fell()) {  // if a switch was grounded, update the status register
    curState2 = 0;           // clear the register and re-build it from current switch reading
    bitSet(curState2, 4);
  }

  debounced2f.update();      // update the debouncer status
  if (debounced2f.fell()) {  // if a switch was grounded, update the status register
    curState2 = 0;           // clear the register and re-build it from current switch reading
    bitSet(curState2, 5);
  }
}  // end readJoysticks()


// check if rotary switches are different from last saved reading
// and generate keystrokes as required
void processJoysticks() {

  int diff = (lastState1 - curState1);  // check for a difference in joystick readings

  // clockwise rotation has occurred
  if (((diff < 0) && !((lastState1 == B000001) && (curState1 == B100000))) | ((diff > 0) && (lastState1 == B100000) && (curState1 == B000001))) {
    Keyboard.write(cw1_char);
  }
  // counter-clockwise rotation has occurred
  else if (((diff > 0) && !((lastState1 == B100000) && (curState1 == B000001))) | ((diff < 0) && (lastState1 == B000001) && (curState1 == B100000))) {
    Keyboard.write(ccw1_char);
  }
  lastState1 = curState1;

  diff = (lastState2 - curState2);  // check for a difference in joystick readings

  // clockwise rotation has occurred
  if (((diff < 0) && !((lastState2 == B000001) && (curState2 == B100000))) | ((diff > 0) && (lastState2 == B100000) && (curState2 == B000001))) {
    Keyboard.write(cw2_char);
  }
  // counter-clockwise rotation has occurred
  else if (((diff > 0) && !((lastState2 == B100000) && (curState2 == B000001))) | ((diff < 0) && (lastState2 == B000001) && (curState2 == B100000))) {
    Keyboard.write(ccw2_char);
  }
  lastState2 = curState2;
}  // end processJoysticks()
