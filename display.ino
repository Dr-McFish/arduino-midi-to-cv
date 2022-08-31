#include <U8glib.h>
#include "display.h"
#include "settings.h"

# define LINE_HIGHT 12
# define CHAR_WIDTH 7
# define LEFT_MARGIN 7
# define MAX_LINES (64/LINE_HIGHT)

U8GLIB_SH1106_128X64 u8g(8, 7, 4); // SCK=13, MOSI=11, CS=8, DC=7, Reset=4

const uint8_t cursor_bitmap[] U8G_PROGMEM = {
  0b01000000,
  0b01100000,
  0b01110000,
  0b01111000,
  0b01110000,
  0b01100000,
  0b01000000
};

int modulo(int x,int N){
    return (x % N + N) %N;
}

void display_setup(){
  //u8g.setFont(u8g_font_unifont);
  //u8g.setFont(u8g_font_6x12);
  u8g.setFont(u8g_font_profont15);
}


void display_options(int encoder_dir, bool encoder_button, struct settings_s* settings){
  static signed int selected_option = 0;
  static bool is_editing;

  if (encoder_button){
    switch(selected_option){
      default:
        break;
      case 0: //note priority
        settings->note_priority = settings->note_priority == HIGHEST ? LOWEST : HIGHEST;
        break;
      case 1://Pitch bend in semitones
        is_editing = is_editing != encoder_button; /* XOR to flip the value */
        break;
      case 2: //bend guards
        settings->bend_guards = settings->bend_guards != true;
        break;
      case 3: //Midi_Monitor
        settings->Midi_Monitor = settings->Midi_Monitor != true;
        break;
      case 4: //retrigger
        settings->retrigger = settings->retrigger != true;
        break;
    }
  }
  if(!is_editing){
    selected_option = modulo(selected_option + encoder_dir, MAX_LINES);
  } else {
    switch(selected_option){
      default:
        break;
      case 1://Pitch bend in semitones
        settings->pitch_bend_semitones = max(0, min(12, settings->pitch_bend_semitones + encoder_dir));
        break;
    }
  }
  
  u8g.firstPage();
  do {
    u8g.drawBitmapP( is_editing ? 2 : 0, 
                    4+ selected_option*LINE_HIGHT, 
                    1, 7, cursor_bitmap);
    for(int i = 0; i < MAX_LINES; i++){
      u8g.drawStr(LEFT_MARGIN, LINE_HIGHT * (i+1), settings_names[i]);
    }
    u8g.drawStr(LEFT_MARGIN+ CHAR_WIDTH*10, LINE_HIGHT * 1, settings->note_priority == HIGHEST ? "Highest": " Lowest");
    u8g.setPrintPos(LEFT_MARGIN+ CHAR_WIDTH*(settings->pitch_bend_semitones<10 ? 14 : 13), LINE_HIGHT * 2);
    u8g.print(settings->pitch_bend_semitones);
    u8g.drawStr(LEFT_MARGIN+ CHAR_WIDTH*14, LINE_HIGHT * 3, settings->bend_guards  ? " ON" : "OFF");
    u8g.drawStr(LEFT_MARGIN+ CHAR_WIDTH*14, LINE_HIGHT * 4, settings->Midi_Monitor ? " ON" : "OFF");
    u8g.drawStr(LEFT_MARGIN+ CHAR_WIDTH*14, LINE_HIGHT * 5, settings->retrigger    ? " ON" : "OFF");
    
  } while( u8g.nextPage() );
}


void display_midi_monitor(/* argumets */){


}










//to delete
void draw_text(const char * txt) {
  //for testing only
  u8g.firstPage();  
  do {
    u8g.drawStr(0, 32, txt);  // put string of display at position X, Y
  } while( u8g.nextPage() );
}

void draw_midi_command(const char * cmd_type, byte pitch, byte velocity) {
  //for testing only
  u8g.firstPage();  
  do {
    u8g.drawStr(0, 32, cmd_type);  // put string of display at position X, Y

    enum {BufSize=5}; // If a is short use a smaller number, eg 5 or 6 
    char buf[BufSize];
    snprintf (buf, BufSize, "%3d", pitch);
    u8g.drawStr(33, 32, buf);
    
    snprintf (buf, BufSize, "%3d", velocity);
    u8g.drawStr(64, 32, buf);
    
  } while( u8g.nextPage() );
}
