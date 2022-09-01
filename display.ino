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

void display_int(int num, int line_num, int left_egdge);
int modulo(int x,int N){
    return (x % N + N) %N;
}

void display_setup(){
  //u8g.setFont(u8g_font_unifont);
  //u8g.setFont(u8g_font_6x12);
  u8g.setFont(u8g_font_profont15);
}


void display_options(const struct settings_s* settings){
  
  u8g.firstPage();
  do {
    u8g.drawBitmapP( settings->is_editing ? 2 : 0, 
                    4+ settings->selected_option*LINE_HIGHT, 
                    1, 7, cursor_bitmap);
    for(int i = 0; i < MAX_LINES; i++){
      u8g.drawStr(LEFT_MARGIN, LINE_HIGHT * (i+1), settings_names[i]);
      switch(setting_type[i]){
        case ONOFF_LBL_ST:
          u8g.drawStr(LEFT_MARGIN+ CHAR_WIDTH*10, LINE_HIGHT * (i + 1),
                      settings->flags & _BV(i  - SETTINGS_NUM_OF_NUMS) ? onoff_labels[i][0]: onoff_labels[i][1]);
  		    break;
        case ONOFF_ST:
  		    u8g.drawStr(LEFT_MARGIN+ CHAR_WIDTH*14, LINE_HIGHT * (i + 1),
                      ( settings->flags & _BV(i - SETTINGS_NUM_OF_NUMS) ) ? " ON" : "OFF");
  		    break;
	      case NUMBER_ST:
		      display_int(settings->nums[i], i+1, 16 - settings_num_units[i]);
          break;
      }
    }
  } while( u8g.nextPage() );
}


void display_midi_monitor(/* argumets */){

}

void display_int(int num, int line_num, int left_egdge) {
  u8g.setPrintPos(LEFT_MARGIN+ CHAR_WIDTH*(num<10 ? left_egdge : left_egdge-1), LINE_HIGHT * line_num);
  u8g.print(num);
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
