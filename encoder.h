#ifndef ENCODER_H
# define ENCODER_H

#define ENCODER_PIN_A 2 // Our first hardware interrupt pin is digital pin 2
#define ENCODER_PIN_B 3 // Our second hardware interrupt pin is digital pin 3

enum ENCODER_STATE {NEUTRAL_ST, CCW_ST, CW_ST};

void setup_encoder();
byte encoder_get();

#endif
