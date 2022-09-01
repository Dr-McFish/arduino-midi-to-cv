#ifndef SETTINGS_H
# define SETTINGS_H

enum note_priority_e{HIGHEST=true, LOWEST=false};
struct settings_s{
  int8_t nums[2]; //PITCH_BEND is st, and MIDI_CHANNEL_S
  uint16_t flags;
};
# define SETTINGS_NUM_OF_NUMS (sizeof(settings->nums)/sizeof(settings->nums[0]))

const char * settings_names[] ={
// |  |  |  |  |   |<- end of the screen
  "Midi channel",
  "Pitch bend     st",
  "Bend guards",
  "Note pr.",
  "Midi watch",
  "Retrigger",
  "NoteOff vel",
  "Exit"
};

enum num_settings_e{
  MIDI_CHANNEL,
  PITCH_BEND
};
struct range_s {
	int min;
	int max;
};

const struct range_s settings_num_ranges[] {
	{0, 15},
	{0, 12}
};
const int settings_num_units[] {
	0,  // no units for MIDI_CHANNEL
  2 //PITCH_BEND int st -> 2 letters in st
};

enum bool_settings_e{
  BEND_GUARDS,
  NOTE_PRIORITY,
  MIDI_WATCH,
  RETRIGGER,
  NOTEOFF_VEL,
  EXIT
};

enum setting_type_e {ONOFF_ST, ONOFF_LBL_ST, NUMBER_ST, EXIT_ST};
const enum setting_type_e setting_type[]{
  NUMBER_ST,//MIDI_CHANNEL
  NUMBER_ST,//PITCH_BEND
  ONOFF_ST,//BEND_GUARDS
  ONOFF_LBL_ST, //NOTE_PRIORITY
  ONOFF_ST, //MIDI_WATCH
  ONOFF_ST,//RETRIGGER
  ONOFF_ST,//NOTEOFF_VEL
  EXIT_ST//EXIT
};

const char* onoff_labels[4][2] = {
  {"", ""},
  {"", ""},
  {"", ""},
  {"Highest", " Lowest"}
};

//TODO
void load_settings_from_EEPROM(struct settings_s* settings);
void save_settings_to_EEPROM(cosnt struct settings_s* settings);

#endif
