#include <MIDI.h>
#include <stdint.h>

#include "encoder.h"
#include "display.h"
#include "bool_array.hpp"
#include "settings.h"

#define MIN_NOTE 36 /*note C2*/
#define TOTAL_NOTES 61 /* 5 octaves -> 12*5 = 60, plus 1 for the C that is 6 octaves above */
typedef int note_number_t; /* 0 - 60; MIN_NOTE is 0 */

int pitchbend = 0;

BoolArray notes_on = BoolArray();
MIDI_CREATE_DEFAULT_INSTANCE();

const int gate_pin = 4; // D4 -> pin PD4 on Atmega328p
const int gate_pin_bitmask = 4; // bit 4 on the `PORTD` register
const int enc_btn_pin = A3;
const int enc_btn_pin_bitmask = 3; //bit 3 on the `PORTC` register

int note_to_volt_per_oct(byte note);
note_number_t midinote_to_notenum(byte midi_note);
int highest_note_on(bool notes[]);
int lowest_note_on(bool notes[]);

void HandleNoteOn(byte channel, byte pitch, byte velocity);
void HandleNoteOff(byte channel, byte pitch, byte velocity) ;
void HandleCC(byte channel, byte control_function, byte parameter);
void HandlePitchBend(byte channel, int bend);

inline void updateNoteAndGate();
inline void updateMenu();

struct settings_s settings = {{4, 12}, 0b00000100, 0, false};


void setup() 
{
  // ------------------------------------------------- MIDI -------------------------------------------------- 
  MIDI.begin(MIDI_CHANNEL_OMNI);
  
  // As of the MIDI Library v3.1, the lib uses C style function 
  // pointers to create a callback system for handling input events. 
  MIDI.setHandleNoteOn(HandleNoteOn); 
  MIDI.setHandleControlChange(HandleCC);
  MIDI.setHandleNoteOff(HandleNoteOff);
  MIDI.setHandlePitchBend(HandlePitchBend);

  // ------------------------------------------------ display ------------------------------------------------
  display_setup();
  draw_text("hello");
  // ------------------------------------------------ encoder ------------------------------------------------ 
  setup_encoder();
  
  pinMode(gate_pin, OUTPUT);
  pinMode(A3, INPUT_PULLUP);

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
  pinMode(5, OUTPUT); // Velocity
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

  updateNoteAndGate();
  updateMenu();
}

// Loop subrutines
inline void updateNoteAndGate() {
  int prioritized_note;
  if(settings.flags & _BV(NOTE_PRIORITY)){
    //HIGHEST:
    prioritized_note = notes_on.highest();
  } else {
    //LOWEST:
    prioritized_note = notes_on.lowest();
  }

  if (prioritized_note == -1) {
    PORTD &= ~_BV(gate_pin_bitmask);
  } else {
    PORTD |= _BV(gate_pin_bitmask);
  
    //sets PWM on v/oct pin to the appropriate note
    OCR1A = max(0, note_to_volt_per_oct(prioritized_note)
                    + (pitchbend*settings.nums[PITCH_BEND])/480 );
  }
}

inline void updateMenu() {
  // encoder rotation
  int encoder_dir;
  switch(encoder_get()){
    case CW_ST:
      encoder_dir = -1;
      break;
    case NEUTRAL_ST:
      encoder_dir = 0;
      break;
    case CCW_ST:
      encoder_dir = 1;
      break;
  }

  // encoder_button
  static bool last_button = false;
  static int debounce_time = 0;
  
  bool button = ((~PINC) & _BV(enc_btn_pin_bitmask));
  bool rising_edge = (!last_button) && button;
  bool button_press;
  last_button = button;
  
  if(rising_edge) {
    if(debounce_time == 0) {
      debounce_time = 2000; // TODO ajust
      button_press = true;
    } else {
      button_press = false;
      debounce_time += 200;
    }
  }

  if(encoder_dir || button_press){
      edit_settings(encoder_dir, button_press, &settings);
      display_options(&settings);
  }

  if(debounce_time > 0)
    debounce_time--;
}

//------------------------------------------------------------------------------------------------------------------------------------------
//
// MIDI callbacks


void HandleNoteOn(byte channel, byte pitch, byte velocity) 
{
  if(settings.nums[MIDI_CHANNEL] == channel) {
    note_number_t nontenum = midinote_to_notenum(pitch);

    notes_on.setb(nontenum);

    //(velocity)
    OCR0B = velocity * 2;  

  }
}

void HandleNoteOff(byte channel, byte pitch, byte velocity) 
{
  if(settings.nums[MIDI_CHANNEL] == channel) {
    note_number_t nontenum = midinote_to_notenum(pitch);

    notes_on.clearb(nontenum);

    if(settings.flags & _BV(NOTEOFF_VEL))
      OCR0B = velocity;
  }
}

void HandleCC(byte channel, byte control_function, byte parameter) 
{
  if(settings.nums[MIDI_CHANNEL] == channel) {
    switch(control_function) {
      default:
        break;
      case midi::ModulationWheel : //MSB
        OCR0A = ((unsigned byte)parameter) * 2;
        break;
      case midi::ModulationWheel + 32 : //LSB
        OCR0A &= 0x11111110;
        OCR0A |= parameter >> 6;
        break;
      case midi::AllNotesOff :
        PORTD |= _BV(gate_pin_bitmask); // set gate pin low
        notes_on.clear_all();
        break;
    }
  }
}

void HandlePitchBend(byte channel, int bend) {
  if(settings.nums[MIDI_CHANNEL] == channel){
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

note_number_t midinote_to_notenum(byte midi_note) {
  int min_note = MIN_NOTE;
  int max_note = MIN_NOTE  + (12*5);
  if (settings.flags & _BV(BEND_GUARDS)) {
    min_note += settings.nums[PITCH_BEND];
    max_note -= settings.nums[PITCH_BEND];
  }
    
  return max(min_note, min(max_note, midi_note)) - min_note;
}

int highest_note_on(bool notes[]) {
  int rt = -1;
  for(int i = 0; i < TOTAL_NOTES; i++)
    if(notes[i])
      rt = i;

  return rt;
}
int lowest_note_on(bool notes[]) {
  int rt = -1;
  for(int i = TOTAL_NOTES-1; i >= 0; i--)
    if(notes[i])
      rt = i;

  return rt;
}
