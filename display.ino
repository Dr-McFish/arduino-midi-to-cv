#include <U8glib.h>
#include "display.h"

# define LINE_HIGHT 12
# define CHAR_WIDTH 8
# define LEFT_MARGIN 6
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

void display_setup(){
  u8g.setFont(u8g_font_unifont);
}


void display_options(int encoder_dir, bool encoder_button){
  static int selected_option = 0;
  static bool is_editing;

  is_editing = is_editing != encoder_button; /* XOR to flip the value */
  if(!is_editing)
    selected_option = (selected_option + encoder_dir) % MAX_LINES;
  
  u8g.firstPage();  //*
  do {
    u8g.drawBitmapP( is_editing ? 10*CHAR_WIDTH : 0, 
                    4+ selected_option*LINE_HIGHT, 
                    1, 7, cursor_bitmap);
    for(int i = 0; i < MAX_LINES; i++){
      u8g.drawStr(LEFT_MARGIN, LINE_HIGHT * (i+1), "Option ");  // put string of display at position X, Y
      u8g.setPrintPos(LEFT_MARGIN + 7*CHAR_WIDTH, LINE_HIGHT * (i+1));
      u8g.print(i+1, 1); 
    }
  } while( u8g.nextPage() );//*/
}


void display_midi_monitor(/* argumets */){


}










//to delete
void draw_text(const char * txt) {
  //for testing only
  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_unifont);  // select font
    u8g.drawStr(0, 32, txt);  // put string of display at position X, Y
  } while( u8g.nextPage() );
}

void draw_midi_command(const char * cmd_type, byte pitch, byte velocity) {
  //for testing only
  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_unifont);  // select font
    u8g.drawStr(0, 32, cmd_type);  // put string of display at position X, Y

    enum {BufSize=5}; // If a is short use a smaller number, eg 5 or 6 
    char buf[BufSize];
    snprintf (buf, BufSize, "%3d", pitch);
    u8g.drawStr(33, 32, buf);
    
    snprintf (buf, BufSize, "%3d", velocity);
    u8g.drawStr(64, 32, buf);
    
  } while( u8g.nextPage() );
}
