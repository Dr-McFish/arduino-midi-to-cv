/*******Interrupt-based Rotary Encoder Sketch*******
by Simon Merrett, based on insight from Oleg Mazurov, Nick Gammon, rt, Steve Spence
modified by Dr_McFish
*/
#include "encoder.h"

volatile byte aFlag = 0; // let's us know when we're expecting a rising edge on ENCODER_PIN_A to signal that the encoder has arrived at a detent
volatile byte bFlag = 0; // let's us know when we're expecting a rising edge on ENCODER_PIN_B to signal that the encoder has arrived at a detent (opposite direction to when aFlag is set)
volatile byte reading = 0; //somewhere to store the direct values we read from our interrupt pins before checking to see if we have moved a whole detent

volatile byte encoder_state = NEUTRAL_ST;

void setup_encoder() {
  pinMode(ENCODER_PIN_A, INPUT_PULLUP); // set ENCODER_PIN_A as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  pinMode(ENCODER_PIN_B, INPUT_PULLUP); // set ENCODER_PIN_B as an input, pulled HIGH to the logic voltage (5V or 3.3V for most cases)
  attachInterrupt(0,PinA,RISING); // set an interrupt on PinA, looking for a rising edge signal and executing the "PinA" Interrupt Service Routine (below)
  attachInterrupt(1,PinB,RISING); // set an interrupt on PinB, looking for a rising edge signal and executing the "PinB" Interrupt Service Routine (below)
}

void PinA(){
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; // read all eight pin values then strip away all but ENCODER_PIN_A and ENCODER_PIN_B's values
  if(reading == B00001100 && aFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoder_state = CCW_ST;
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00000100) bFlag = 1; //signal that we're expecting ENCODER_PIN_B to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

void PinB(){
  cli(); //stop interrupts happening before we read pin values
  reading = PIND & 0xC; //read all eight pin values then strip away all but ENCODER_PIN_A and ENCODER_PIN_B's values
  if (reading == B00001100 && bFlag) { //check that we have both pins at detent (HIGH) and that we are expecting detent on this pin's rising edge
    encoder_state = CW_ST;
    bFlag = 0; //reset flags for the next turn
    aFlag = 0; //reset flags for the next turn
  }
  else if (reading == B00001000) aFlag = 1; //signal that we're expecting ENCODER_PIN_A to signal the transition to detent from free rotation
  sei(); //restart interrupts
}

byte encoder_get() {
  cli();
  if (encoder_state != NEUTRAL_ST) {
    byte temp = encoder_state;
    encoder_state = NEUTRAL_ST;
    sei();
    return temp;
  } else {
    sei();
    return encoder_state;
  }
}
