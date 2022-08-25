#include <MIDI.h>
#include <stdint.h>

#define MIN_NOTE 36 //note C2
#define TOTAL_NOTES 61 // 5 octaves -> 12*5 = 60, plus 1 for the C that is 6 octaves above
typedef int note_number_t; /* 0 - 60; MIN_NOTE is 0 */

const int gate_pin = 12; // D12 -> pin PB4 on Atmega328p
const int gate_pin_bitmask = 4; // bit 4 on the `PORTB` register

byte controller_channel = 4;
bool notes_on[TOTAL_NOTES];
int pitchbend = 0;
//TODO ^ replace this with bitshift magic,
//wasting 7/8 of the memmory is driving my ocd insane

MIDI_CREATE_DEFAULT_INSTANCE();

int note_to_volt_per_oct(byte note);
note_number_t midinote_to_notemum(byte midi_note);
int highest_note_on(bool notes[]);

void HandleNoteOn(byte channel, byte pitch, byte velocity);
void HandleNoteOff(byte channel, byte pitch, byte velocity) ;
void HandleCC(byte channel, byte control_function, byte parameter);
void HandlePitchBend(byte channel, int bend);


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

  pinMode(gate_pin, OUTPUT);

  //timer nonsense
  pinMode(3, OUTPUT); // velocity
  pinMode(11, OUTPUT); // Mod Wheel
  // ------------------------------------------------ TIMER 2 ------------------------------------------------
  //        Enable PWM pin D11             Enable PWM pin D3                Fast PWM mode
  //        Clear OC2A on compare match    Clear OC2B on compare match      TOP = 0xFF;
  //        set OC2A at BOTTOM             set OC2B at BOTTOM,              Update of OCRx at BOTTOM
  TCCR2A = _BV(COM2A1)                   | _BV(COM2B1)                     | _BV(WGM21) | _BV(WGM20);
  //        PRESCALER is set to 1 (62.5 kHz sheesh), more than 20
  TCCR2B = _BV(CS20); 
  OCR2A = 180; // pin D11 PWM value (0-255) (Mod)
  OCR2B = 50;  // pin D3  PWM value (0-255) (velocity)


  pinMode(9, OUTPUT); // Volt per octave
  pinMode(10, OUTPUT); // Mod Wheel raw
  // ------------------------------------------------ TIMER 2 ------------------------------------------------
  //       Enable PWM out on pin D9(OC1A)    Enable PWM out on pin D10(OC1B)      Fast PWM 10bit mode
  TCCR1A = _BV(COM1A1)                     | _BV(COM1B1)                        | _BV(WGM11) | _BV(WGM10); //WGM12
  //        Fast PWM 10bit mode         prescaler of 1, 15,6kHz
  TCCR1B = _BV(WGM12)                | _BV(CS10);
  OCR1A = 0; // pin D9  pwm value (0-1023) (v/oct)
  OCR1B = 0; // pin D10 pwm value (0-1023) (pitchbend)
}

void loop()
{
  MIDI.read();

  int highest_note = highest_note_on(notes_on);

  if (highest_note == -1) {
    PORTB &= ~_BV(gate_pin_bitmask); // set gate pin high
    return;
  }
  PORTB |= _BV(gate_pin_bitmask); // set gate pin low
  
  //sets PWM on v/oct pin to the highest note
  OCR1A = max(0, note_to_volt_per_oct(highest_note) + (pitchbend/40));
  // /40 1 octave of pitchbend. | /240 whole tone pichbend
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
    OCR2B = velocity * 2;  

  }
}

void HandleNoteOff(byte channel, byte pitch, byte velocity) 
{
  if(controller_channel == channel) {
    note_number_t nontenum = midinote_to_notemum(pitch);

    notes_on[nontenum] = false;

    //(velocity)
    //OCR2B = velocity;
  }
}

void HandleCC(byte channel, byte control_function, byte parameter) 
{
  if(controller_channel == channel) {
    switch(control_function) {
      default:
        break;
      case midi::ModulationWheel : //MSB
        OCR2A = ((unsigned byte)parameter) * 2;
        break;
      case midi::ModulationWheel + 32 : //LSB
        OCR2A &= 0x11111110;
        OCR2A |= parameter / 127;
        break;
      case midi::AllNotesOff :
        PORTB |= _BV(gate_pin_bitmask); // set gate pin low
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
