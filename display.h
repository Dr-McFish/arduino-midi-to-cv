#ifndef DISPLAY_H
# define DISPLAY_H

//TODO: Remove this after it is no longer needed
void draw_text(const char * txt);
void draw_midi_command(const char * cmd_type, byte pitch, byte velocity);

//main functions:
void display_setup();
void display_options(const struct settings_s* settings);
void display_midi_monitor(/* argumets */);

#endif
