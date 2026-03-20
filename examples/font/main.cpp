#include <avr/io.h>
#include <avr/interrupt.h>
#include <ssd1306.h>
#include <i2c.h>
#include <usart.h>
#include "font8x8_basic.h"


void convert_to_column_major(const uint8_t source[][8], uint8_t dest[][8]) {
  uint8_t byte = 0;
  uint8_t col = 0;
  uint8_t row = 0;
  for(int i = 0; i < 95; i++) {
    for(int bit_pos = 0; bit_pos < 8; bit_pos++) {
      byte = 0;
      for(int j = 0; j < 8; j++) {
        byte |= ((source[i][j] >> bit_pos) & 1) << j;
      }
      
      if(col == 8) {
        col = 0;
        row++;
      }
      
      dest[row][col++] = byte;
    }
  }
}

int main(void)
{
  i2c_init();
  usart_init();
  ssd1306_init();
  int count = 0;
  uint8_t font_converted[95][8];
  convert_to_column_major(font8x8_basic, font_converted);
  // clear_screen();
  // for(int i = 33; i < 37; i++) {
    //   for(int j = 0; j < 8; j++) {
      //     framebuffer[count++] = font_converted[i][j];
      //   }
  // }
  fill_screen_with_buffer(framebuffer, sizeof(framebuffer));
  while(true);
}