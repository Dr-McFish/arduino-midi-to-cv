#include <MCP_DAC.h>
#include <MIDI.h>
#include <stdint.h>

#include "encoder.h"
#include "display.h"
#include "bool_array.hpp"
#include "settings.h"

constexpr int8_t MIN_NOTE =36; /*note C2*/
constexpr int8_t NUM_OCTAVES = 8;
typedef int note_number_t; /* 0 - 60; MIN_NOTE is 0 */

int pitchbend = 0;

BoolArray notes_on = BoolArray();
bool note_on_update = false;
MIDI_CREATE_DEFAULT_INSTANCE();

const int gate_pin = 4; // D4 -> pin PD4 on Atmega328p
const int gate_pin_bitmask = 4; // bit 4 on the `PORTD` register
const int enc_btn_pin = A3;
const int enc_btn_pin_bitmask = 3; //bit 3 on the `PORTC` register

int note_to_volt_per_oct_10bit(byte note);
note_number_t midinote_to_notenum(byte midi_note);

void HandleNoteOn(byte channel, byte pitch, byte velocity);
void HandleNoteOff(byte channel, byte pitch, byte velocity) ;
void HandleCC(byte channel, byte control_function, byte parameter);
void HandlePitchBend(byte channel, int bend);

inline void updateNoteAndGate();
inline void updateMenu();

MCP4922 MCP;  // HW SPI DAC
enum DAC_CHANNEL {
	CH_V_OCT = 0, //Channel A
	CH_PITCH_BEND = 1, //Channel B
};

struct settings_s settings = {{4, 12}, 0b00000100, 0, false};


void setup() 
{
	// ------------------------------------------------- MIDI -------------------------------------------------- 
	MIDI.begin(MIDI_CHANNEL_OMNI);
	MIDI.turnThruOff();
	//Serial.begin(9600);
	
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
	
	// -------------------------------------------------- DAC --------------------------------------------------
	// Todo: it may be fasater to use SPI library insted of this MCP library
	MCP.begin(A4);
	MCP.setGain(2);


	// ----------------------------------------------- Gate pin ------------------------------------------------  

	pinMode(gate_pin, OUTPUT);
	pinMode(A3, INPUT_PULLUP);


	// TODO : migrate mod and velocity to Timer 1(pins 9 and 10) for more resolution(and less scary high frequencies? I have no idea what I am doing)

	// ###################################### DEPRICATED! Replaced with DAC! ######################################

	// pinMode(9, OUTPUT); // Volt per octave
	// pinMode(10, OUTPUT); // Pitch Wheel raw
	// ------------------------------------------------ TIMER 1 ------------------------------------------------
	// //       Enable PWM out on pin D9(OC1A)    Enable PWM out on pin D10(OC1B)      Fast PWM 10bit mode
	// TCCR1A = _BV(COM1A1)                     | _BV(COM1B1)                        | _BV(WGM11) | _BV(WGM10); //WGM12
	// //        prescaler of 1, 15,6kHz
	// TCCR1B = _BV(CS10)                                                            | _BV(WGM12);
	// OCR1A = 0; // pin D9  pwm value (0-1023) (v/oct)
	// OCR1B = 0x01Ff; // pin D10 pwm value (0-1023) (pitchbend)
	write_bend(0);

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

int16_t note_to_volt_per_oct_12bit(byte note){
	// because of the 4 x 2.048V voltage refference, the value writen to the DAC is in 2 milivolts
	// (int)1 <=> 2 mV
	const int16_t note_range_limited = max(0, min((int16_t)(note) - MIN_NOTE, 12 * NUM_OCTAVES)); // range of 4 octaves
	Serial.print("note #");
	Serial.print(note_range_limited);
	Serial.print("  |  ");
	Serial.print((note_range_limited * (int16_t)500) / (int16_t)12);
	Serial.println("mV");
	return (note_range_limited * (int16_t)500) / (int16_t)12;
}


inline void write_v_oct(byte note_num) {
	MCP.analogWrite(note_to_volt_per_oct_12bit(note_num), DAC_CHANNEL::CH_V_OCT);
}
/* bend is the MIDI bend value, so 14 bits */
inline void write_bend(int16_t bend) {
	//Serial.println((bend - MIDI_PITCHBEND_MIN) / 4);
	MCP.analogWrite( ( MIDI_PITCHBEND_MAX - bend ) / 4, DAC_CHANNEL::CH_PITCH_BEND);
}

inline void write_mod(uint8_t val) {
	OCR0A = val;
}
inline void write_velocity(uint8_t val) {
	OCR0B = val;
}


void loop() {
	MIDI.read();

	updateNoteAndGate();
	updateMenu();
}

// Loop subrutines
inline void updateNoteAndGate() {
	if (note_on_update) {
		note_on_update = false;
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
			write_v_oct(prioritized_note);
			// TODO (maybe) software pitch bend settings.nums[PITCH_BEND]) if the circuit version is insuficient
		}
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
			debounce_time = 8000; // TODO ajust
			button_press = true;
		} else {
			button_press = false;
			debounce_time += 2000;
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
		notes_on.setb(pitch);

		//(velocity)
		OCR0B = velocity * 2;  

	}
	note_on_update = true;
	Serial.print("Recive note: ");
	Serial.println(pitch);
}

void HandleNoteOff(byte channel, byte pitch, byte velocity) 
{
	if(settings.nums[MIDI_CHANNEL] == channel) {
		notes_on.clearb(pitch);

		if(settings.flags & _BV(NOTEOFF_VEL))
		OCR0B = velocity;
	}
	note_on_update = true;
}

void HandleCC(byte channel, byte control_function, byte parameter)
{
	if(settings.nums[MIDI_CHANNEL] == channel) {
		switch(control_function) {
		  default:
			break;
		  case midi::ModulationWheel : //MSB
			OCR0A = ((uint8_t)parameter) * 2;
			break;
		  case midi::ModulationWheel + 32 : //LSB
			OCR0A &= 0x11111110;
			OCR0A |= parameter >> 6;
			break;
		  case midi::AllNotesOff :
			PORTD |= _BV(gate_pin_bitmask); // set gate pin low
			notes_on.clear_all();
			note_on_update = true;
			break;
		}
	}
}

void HandlePitchBend(byte channel, int bend) {
	if(settings.nums[MIDI_CHANNEL] == channel){
		pitchbend = bend;
		write_bend(bend);
	}
}

//------------------------------------------------------------------------------------------------------------------------------------------
//
// functions

int note_to_volt_per_oct_10bit(byte note){
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
