#include <avr/io.h>
#include <avr/interrupt.h>
#include <ssd1306.h>
#include <i2c.h>
#include <usart.h>

int main(void)
{
  i2c_init();
  usart_init();
  ssd1306_init();
  draw_line(0, 0, 50, 63);
  fill_screen_with_buffer();
  while(true);
}