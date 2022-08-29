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
