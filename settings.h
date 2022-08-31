#ifndef SETTINGS_H
# define SETTINGS_H

enum note_priority_e{HIGHEST, LOWEST};
struct settings_s{
  enum note_priority_e note_priority;
  int8_t pitch_bend_semitones; // (0-12)
  bool bend_guards;
  bool Midi_Monitor;
  bool retrigger;
  bool note_off_velocity;
  byte controller_channel; // (0-16/OMNI)
};

const char * settings_names[] ={
// |  |  |  |  |   |<- end of the screen
  "Note pr.",
  "Pitch bend     st",
  "Bend guards",
  "Midi watch",
  "Retrigger",
  "NoteOff vel",
  "Midi channel",
  "Exit"
};

enum settings_e{
  NOTE_PRIORITY,
  PITCH_BEND,
  BEND_GUARDS,
  MIDI_WATCH,
  RETRIGGER,
  NOTEOFF_VEL,
  MIDI_CHANNEL,
  EXIT
}

struct onoff_label {
  const char* on;
  const char* off;
};


//TODO
void load_settings_from_EEPROM(struct settings_s* settings);
void save_settings_to_EEPROM(struct settings_s* settings);

#endif
