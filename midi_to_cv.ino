#include <MIDI.h>
#include <U8glib.h>
#include <stdint.h>

#include "encoder.h"

#define MIN_NOTE 36 //note C2
#define TOTAL_NOTES 61 // 5 octaves -> 12*5 = 60, plus 1 for the C that is 6 octaves above
typedef int note_number_t; /* 0 - 60; MIN_NOTE is 0 */

byte controller_channel = 4;
bool notes_on[TOTAL_NOTES];
int pitchbend = 0;
//TODO ^ replace this with bitshift magic,
//wasting 7/8 of the memmory is driving my ocd insane

MIDI_CREATE_DEFAULT_INSTANCE();
U8GLIB_SH1106_128X64 u8g(8, 7, 4); // SCK=13, MOSI=11, CS=8, DC=7, Reset=4


const int gate_pin = 4; // D4 -> pin PD4 on Atmega328p
const int gate_pin_bitmask = 4; // bit 4 on the `PORTD` register

int note_to_volt_per_oct(byte note);
note_number_t midinote_to_notemum(byte midi_note);
int highest_note_on(bool notes[]);

void HandleNoteOn(byte channel, byte pitch, byte velocity);
void HandleNoteOff(byte channel, byte pitch, byte velocity) ;
void HandleCC(byte channel, byte control_function, byte parameter);
void HandlePitchBend(byte channel, int bend);

void draw_text(const char * txt);
void setup() 
{
  
  MIDI.begin(MIDI_CHANNEL_OMNI);
  
  // As of the MIDI Library v3.1, the lib uses C style function 
  // pointers to create a callback system for handling input events. 
  MIDI.setHandleNoteOn(HandleNoteOn); 
  MIDI.setHandleControlChange(HandleCC);
  MIDI.setHandleNoteOff(HandleNoteOff);
  MIDI.setHandlePitchBend(HandlePitchBend);
  controller_channel = 4;

  // ------------ display ------------ 
  draw_text("hello");
  // ------------ encoder ------------ 
  setup_encoder();
  
  pinMode(gate_pin, OUTPUT);

  pinMode(9, OUTPUT); // Volt per octave
  pinMode(10, OUTPUT); // Pitch Wheel raw
  // ------------------------------------------------ TIMER 1 ------------------------------------------------
  //       Enable PWM out on pin D9(OC1A)    Enable PWM out on pin D10(OC1B)      Fast PWM 10bit mode
  TCCR1A = _BV(COM1A1)                     | _BV(COM1B1)                        | _BV(WGM11) | _BV(WGM10); //WGM12
  //        prescaler of 1, 15,6kHz
  TCCR1B = _BV(CS10)                                                            | _BV(WGM12);
  OCR1A = 0; // pin D9  pwm value (0-1023) (v/oct)
  OCR1B = 0x01Ff; // pin D10 pwm value (0-1023) (pitchbend)

  pinMode(6, OUTPUT); // Mod Wheel
  pinMode(5, OUTPUT); // undefined
  // ------------------------------------------------ TIMER 0 ------------------------------------------------
  //        Enable PWM pin D6              Enable PWM pin D5                Fast PWM mode
  //        Clear OC0A on compare match    Clear OC0B on compare match      TOP = 0xFF;
  //        set OC0A at BOTTOM             set OC2B at BOTTOM,              Update of OCRx at BOTTOM
  TCCR0A = _BV(COM0A1)                   | _BV(COM0B1)                     | _BV(WGM01) | _BV(WGM00);
  //        PRESCALER is set to 1 (62.5 kHz sheesh), more than 20
  TCCR0B = _BV(CS00);
  OCR0A = 0; // pin D6  pwm value (0-255) (Mod Wheel)
  OCR0B = 0; // pin D5  pwm value (0-255) (velocity)
}

void loop()
{
  MIDI.read();


  switch (encoder_get()) {
    case NEUTRAL_ST:
      break;
    case CCW_ST:
      digitalWrite(gate_pin, HIGH);
      draw_text("ccw");
      break;
    case CW_ST:
      draw_text("cw");
      break;
  }

  
  int highest_note = highest_note_on(notes_on);

  if (highest_note == -1) {
    PORTD &= ~_BV(gate_pin_bitmask);
  } else {
    PORTD |= _BV(gate_pin_bitmask);
  
    //sets PWM on v/oct pin to the highest note
    OCR1A = max(0, note_to_volt_per_oct(highest_note) + (pitchbend/40));
    // /40 1 octave of pitchbend. | /240 whole tone pichbend
  }
}


//------------------------------------------------------------------------------------------------------------------------------------------
//
// MIDI callbacks


void HandleNoteOn(byte channel, byte pitch, byte velocity) 
{ 
  if(controller_channel == channel) {
    note_number_t nontenum = midinote_to_notemum(pitch);

    notes_on[nontenum] = true;

    //(velocity)
    OCR0B = velocity * 2;  

  }
}

void HandleNoteOff(byte channel, byte pitch, byte velocity) 
{
  if(controller_channel == channel) {
    note_number_t nontenum = midinote_to_notemum(pitch);

    notes_on[nontenum] = false;

    //(velocity)
    //OCR0B = velocity;
  }
}

void HandleCC(byte channel, byte control_function, byte parameter) 
{
  if(controller_channel == channel) {
    switch(control_function) {
      default:
        break;
      case midi::ModulationWheel : //MSB
        OCR0A = ((unsigned byte)parameter) * 2;
        break;
      case midi::ModulationWheel + 32 : //LSB
        OCR0A &= 0x11111110;
        OCR0A |= parameter / 127;
        break;
      case midi::AllNotesOff :
        PORTD |= _BV(gate_pin_bitmask); // set gate pin low
        for(int i = 0; i < TOTAL_NOTES; i++)
            notes_on[i] = 0;
        break;
    }
  }
}

void HandlePitchBend(byte channel, int bend) {
  if(controller_channel == channel){
    pitchbend = bend;
    OCR1B = (bend - MIDI_PITCHBEND_MIN)>>4;
  }
}

//------------------------------------------------------------------------------------------------------------------------------------------
//
// functions

int note_to_volt_per_oct(byte note){
  int16_t pitch = note;
  pitch *= 341;
  pitch = (pitch / 20) + ((pitch % 20) >= 10 ? 1 : 0);
  return pitch;
}

note_number_t midinote_to_notemum(byte midi_note) {
  return max(MIN_NOTE, min(MIN_NOTE + (12*5), midi_note)) - MIN_NOTE;
}

int highest_note_on(bool notes[]) {
  int rt = -1;
  for(int i = 0; i < TOTAL_NOTES; i++)
    if(notes[i])
      rt = i;

  return rt;
}

void draw_text(const char * txt) {
  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_unifont);  // select font
    u8g.drawStr(0, 32, txt);  // put string of display at position X, Y
  } while( u8g.nextPage() );
}
