#include <U8glib.h>
#include "display.h"

U8GLIB_SH1106_128X64 u8g(8, 7, 4); // SCK=13, MOSI=11, CS=8, DC=7, Reset=4

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
