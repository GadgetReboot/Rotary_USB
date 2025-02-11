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

#include <Keyboard.h>  // part of the Sparkfun Pro Micro board support
#include <Bounce2.h>   // install library from library manager
#include "RingBuf.h"   // ring buffer library file included in this sketch project, taken from https://github.com/Locoduino/RingBuffer

// track whether pending rotation keystrokes stored in a ring buffer
// should be clockwise or counter-clockwise when the time comes to send the keystrokes out.
// each joystick has a ring buffer to store pending keystrokes for cw or ccw rotation
// the buffer is used when the joystick rotates faster than the USB keystrokes can be sent
enum rotateDirection {
  CW,  // clockwise
  CCW  // counter-clockwise
};

const byte keystrokeBufSize = 20;                    // number of pending keystrokes to buffer for each joystick if needed
RingBuf<rotateDirection, keystrokeBufSize> joy1Buf;  // ring buffer to store pending keystrokes if joystick 1 moves faster than USB keypresses can keep up with
RingBuf<rotateDirection, keystrokeBufSize> joy2Buf;  // ring buffer to store pending keystrokes if joystick 2 moves faster than USB keypresses can keep up with

// keyboard characters sent for rotations
const char cw1_char = 'r';   // joystick 1 clockwise character
const char ccw1_char = 'l';  // joystick 1 counter-clockwise character
const char cw2_char = 'x';   // joystick 2 clockwise character
const char ccw2_char = 'z';  // joystick 2 counter-clockwise character

// timing control for switch reading and keystroke sending
const byte keypressDuration = 110;  // how long a keypress is held, in mS
const byte debounceTime = 10;        // joystick switch contact debounce time in mS

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

  Keyboard.begin();

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

  lastState1 = curState1;  // make last and current readings identical to avoid a false output trigger on power up
  lastState2 = curState2;

}  // end setup()

void loop() {
  readJoysticks();      // read the current rotary switch states into curState registers
  processJoysticks();   // determine if a rotation has occurred
  processKeystrokes();  // send keystrokes for joystick rotations that have accumulated
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
void processJoysticks() {

  int diff = (lastState1 - curState1);  // check for a difference in joystick 1 readings

  // joystick 1 clockwise rotation has occurred
  if (((diff < 0) && !((lastState1 == B000001) && (curState1 == B100000))) | ((diff > 0) && (lastState1 == B100000) && (curState1 == B000001))) {
    joy1Buf.push(CW);  // add a pending clockwise keystroke to joystick 1 buffer
  }
  // joystick 1 counter-clockwise rotation has occurred
  else if (((diff > 0) && !((lastState1 == B100000) && (curState1 == B000001))) | ((diff < 0) && (lastState1 == B000001) && (curState1 == B100000))) {
    joy1Buf.push(CCW);  // add a pending counter clockwise keystroke to joystick 1 buffer
  }
  lastState1 = curState1;           // update joystick 1 historical state for next evaluation cycle
  diff = (lastState2 - curState2);  // check for a difference in joystick 2 readings

  // joystick 2 clockwise rotation has occurred
  if (((diff < 0) && !((lastState2 == B000001) && (curState2 == B100000))) | ((diff > 0) && (lastState2 == B100000) && (curState2 == B000001))) {
    joy2Buf.push(CW);  // add a pending clockwise keystroke to joystick 2 buffer
  }
  // joystick 2 counter-clockwise rotation has occurred
  else if (((diff > 0) && !((lastState2 == B100000) && (curState2 == B000001))) | ((diff < 0) && (lastState2 == B000001) && (curState2 == B100000))) {
    joy2Buf.push(CCW);  // add a pending counter clockwise keystroke to joystick 2 buffer
  }
  lastState2 = curState2;  // update joystick 2 historical state for next evaluation cycle
}  // end processJoysticks()

// generate keystrokes as required
void processKeystrokes() {

  enum rotateDirection joyDirectionValue;  // working variable for joystick/keypress direction evaluations

  // keypress timers
  static unsigned long joy1KeypressTimer = millis();
  static unsigned long joy2KeypressTimer = millis();

  // keypress flags, true if a key is being pressed and timer is running
  static bool joy1_CW_KeypressFlag = false;
  static bool joy1_CCW_KeypressFlag = false;
  static bool joy2_CW_KeypressFlag = false;
  static bool joy2_CCW_KeypressFlag = false;

  // joystick 1 check if keystrokes are pending in buffer and start a keypress if it is time
  if ((joy1Buf.size() > 0) && !joy1_CW_KeypressFlag && !joy1_CCW_KeypressFlag) {
    if (joy1Buf.pop(joyDirectionValue)) {  // get the pending joystick rotation direction from the buffer for the next keypress
      if (joyDirectionValue == CW) {
        Keyboard.press(cw1_char);     // press the joystick 1 clockwise keyboard key
        joy1_CW_KeypressFlag = true;  // set a flag to track that a keypress is active
      } else if (joyDirectionValue == CCW) {
        Keyboard.press(ccw1_char);    // press the joystick 1 counter clockwise keyboard key
        joy1_CCW_KeypressFlag = true;  // set a flag to track that a keypress is active
      }
    }
    joy1KeypressTimer = millis();  // start a timer for holding down the keypress
  }

  // joystick 2 check if keystrokes are pending in buffer and start a keypress if it is time
  if ((joy2Buf.size() > 0) && !joy2_CW_KeypressFlag && !joy2_CCW_KeypressFlag) {
    if (joy2Buf.pop(joyDirectionValue)) {  // get the pending joystick rotation direction from the buffer for the next keypress
      if (joyDirectionValue == CW) {
        Keyboard.press(cw2_char);     // press the joystick 2 clockwise keyboard key
        joy2_CW_KeypressFlag = true;  // set a flag to track that a keypress is active
      } else if (joyDirectionValue == CCW) {
        Keyboard.press(ccw2_char);    // press the joystick 2 counter clockwise keyboard key
        joy2_CCW_KeypressFlag = true;  // set a flag to track that a keypress is active
      }
    }
    joy2KeypressTimer = millis();  // start a timer for holding down the keypress
  }




  // check if it is time to release any keyboard keys that have been pressed
  if (joy1_CW_KeypressFlag) {
    if (millis() - joy1KeypressTimer >= keypressDuration) {
      Keyboard.release(cw1_char);  // release the keyboard key
      joy1_CW_KeypressFlag = false;
    }
  }

  if (joy1_CCW_KeypressFlag) {
    if (millis() - joy1KeypressTimer >= keypressDuration) {
      Keyboard.release(ccw1_char);  // release the keyboard key
      joy1_CCW_KeypressFlag = false;
    }
  }

  if (joy2_CW_KeypressFlag) {
    if (millis() - joy2KeypressTimer >= keypressDuration) {
      Keyboard.release(cw2_char);  // release the keyboard key
      joy2_CW_KeypressFlag = false;
    }
  }

  if (joy2_CCW_KeypressFlag) {
    if (millis() - joy2KeypressTimer >= keypressDuration) {
      Keyboard.release(ccw2_char);  // release the keyboard key
      joy2_CCW_KeypressFlag = false;
    }
  }
}
