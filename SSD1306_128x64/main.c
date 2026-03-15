#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <math.h>
#include <stdbool.h>
#include <util/delay.h>
#include "cursor.h"

#define CHECK_TWINT_SET !(TWCR & (1 << TWINT))
#define TWI_STATUS_CODE (TWSR & 0xF8)
#define START 0x08
#define MT_DATA_ACK 0x28
#define MT_SLA_ACK 0x18
#define TWI_FREQ 800000L

#define SSD1306_ADDRESS_WRITE 0x78
#define CONTROL_MULTIPLEDATA_DATA 0x40
#define CONTROL_SINGLEDATA_DATA 0xC0
#define CONTROL_MULTIPLEDATA_COMMAND 0x00
#define CONTROL_SINGLEDATA_COMMAND 0x80
#define SCREEN_WIDTH 128
#define PAGE_SIZE 8

#define CLEAR_PIN_7 PORTD &= ~(1 << PORTD7);
#define SET_PIN_7 PORTD |= (1 << PORTD7);

void draw_pixel(uint8_t x, uint8_t y);
void clear_pixel(uint8_t x, uint8_t y);

uint8_t framebuffer[1024] = {0x00};

uint8_t ssd1306_init_commands[] = {
    0xAE, // display OFF initially
    0xA4, // display follows RAM content
    0x81, // contrast command
    0x7F, // set contrast value
    0xA6, // normal (non-inverted) display mode
    0x20, // set addressing mode command
    0x02, // page addressing mode
    0xB0, // set page start
    0x00, // set lower column address (page)
    0x10, // set upper column address (page)
    // 0x21, // set col address cmd
    // 0x00, // set col start to 0
    // 0x7F, // set col end adderss to 127
    0x40, // set display start line
    0xA0, // col 0 is mapped to seg 0
    0xA8, // set mux ratio cmd
    0x3F, // set mux ratio value
    0xC0, // com scan direction
    0xD9, // set pre charge period cmd
    0x22, // pre charge value (see datasheet page 32)
    0xD5, // set display clock cmd
    0xF0, // set display clock value (max clock)
    // 0x22, // set page address cmd
    // 0x00, // set page start addr to 0
    // 0x07, // set page end addr to 7
    0xD3, // display offset cmd
    0x00, // set display offset to 0
    0xDA, // set COM pin hardware config
    0x12, // COM config default (reset)
    0x8D, // charge pump cmd
    0x14, // enable charge pump
    0xAF, // display ON
};

void i2c_init() {
  // enable internal pullups
  DDRC &= ~((1 << PC4) | (1 << PC5));
  PORTC |= (1 << PC4) | (1 << PC5);

  // set prescaler (00) and init bit rate
  TWSR &= ~(1 << TWPS0);
  TWSR &= ~(1 << TWPS1);
  TWBR = ((F_CPU / TWI_FREQ) - 16) / 2;
  TWCR |= (1 << TWEN) | (1 << TWEA);
}

bool i2c_send_start_bit() {
  TWCR = (1 << TWINT) | (1 << TWSTA) | (1 << TWEN);
  while (CHECK_TWINT_SET)
    ;
  if (TWI_STATUS_CODE == START) {
    return true;
  } else {
    return false;
  }
}

bool i2c_send_address(char addr) {
  TWDR = addr;
  TWCR = (1 << TWINT) | (1 << TWEN);
  while (CHECK_TWINT_SET)
    ;
  if (TWI_STATUS_CODE == MT_SLA_ACK) {
    return true;
  } else {
    return false;
  }
}

bool i2c_send_byte(char byte) {
  TWDR = byte;
  TWCR = (1 << TWINT) | (1 << TWEN);
  while (CHECK_TWINT_SET)
    ;
  if (TWI_STATUS_CODE == MT_DATA_ACK) {
    return true;
  } else {
    return false;
  }
}

void i2c_send_stop() { TWCR = (1 << TWINT) | (1 << TWSTO) | (1 << TWEN); }

bool i2c_send_buffer(uint8_t *buffer, uint8_t size) {
  bool success = 0;
  for (int i = 0; i < size; i++) {
    i2c_send_start_bit();
    success = i2c_send_byte(buffer[i]);
  }
  i2c_send_stop();
  return success;
}

void ssd1306_init() {
  i2c_send_start_bit();
  i2c_send_address(SSD1306_ADDRESS_WRITE);
  i2c_send_byte(CONTROL_MULTIPLEDATA_COMMAND);
  for (int i = 0; i < sizeof(ssd1306_init_commands); i++) {
    // i2c_send_start_bit();
    i2c_send_byte(ssd1306_init_commands[i]);
  }
  i2c_send_stop();
}

void clear_screen() {
  for(int i = 0; i < 128; i++) {
    for(int j = 0; j < 64; j++) {
        clear_pixel(i, j);
    }
  }
}

void fill_screen() {
  for(int i = 0; i < 128; i++) {
    for(int j = 0; j < 64; j++) {
        draw_pixel(i, j);
    }
  }
}

void fill_screen_with_buffer() {
  // -----Page addressing mode-----
  // takes roughly around 16ms@800Kbps
  // takes roughly around 26.832ms@400Kbps
  // overhead 5.168ms and I2C 10.832ms? @800Kbps
  SET_PIN_7
  uint16_t count = 0;
  for (int page = 0; page < 8; page++) {
    i2c_send_start_bit();
    i2c_send_address(SSD1306_ADDRESS_WRITE);
    i2c_send_byte(CONTROL_MULTIPLEDATA_COMMAND);
    i2c_send_byte(0xB0 | page);
    i2c_send_byte(0x00);
    i2c_send_byte(0x10);
    i2c_send_stop();

    i2c_send_start_bit();
    i2c_send_address(SSD1306_ADDRESS_WRITE);
    i2c_send_byte(CONTROL_MULTIPLEDATA_DATA);

    for (int i = 0; i < 128; i++) {
      i2c_send_byte(framebuffer[count++]);
    }
    i2c_send_stop();
  }
  CLEAR_PIN_7
}

void draw_pixel(uint8_t x, uint8_t y) {
  // (x, y) is pixel coordinate
  // origin is top left
  // positive direction of x is same (right), for y positive direction is down
  uint8_t col = x;
  uint8_t row = (y / 8);    // find out which page this pixel belongs to
  uint8_t bit_position = y % 8; // find out which bit does the 'y' corresponds to in the byte
  framebuffer[(row) * 128 + col] |= (1 << bit_position);
} // there is a little offset, need to resolve that

void clear_pixel(uint8_t x, uint8_t y) {
  // (x, y) is pixel coordinate
  // origin is top left
  // positive direction of x is same (right), for y positive direction is down
  uint8_t col = x;
  uint8_t row = (y / 8);    // find out which page this pixel belongs to
  uint8_t bit_position = y % 8; // find out which bit does the 'y' corresponds to in the byte
  framebuffer[(row) * 128 + col] &= ~(1 << bit_position);
} 

void draw_line(uint8_t start_x, uint8_t start_y, uint8_t finish_x, uint8_t finish_y) {
  // takes roughly around 203uS
  SET_PIN_7
  // Bresenham's line drawing algorithm
  int dx = finish_x - start_x, dy = finish_y - start_y;
  int x = start_x, y = start_y;
  int p_initial, p_inc, p_inc_xy;
  int x_inc = 1;
  int y_inc = 1;
 
  if(dy < 0) {
    y_inc = -1;
    dy = -dy; // check wikipedia pseudocode
  }
  if(dx < 0) {
    x_inc = -1;
    dx = -dx;
  }

  if(dy < dx) {
    p_initial = 2 * dy - dx;
    p_inc = 2 * dy;
    p_inc_xy = 2 * (dy - dx);
  }
  else {
    p_initial = 2 * dx - dy;
    p_inc = 2 * dx;
    p_inc_xy = 2 * (dx - dy);
  }
  
  int p = p_initial;
  draw_pixel(x, y);
  while((x != finish_x && y != finish_y)) {
    if(dy < dx)
      x += x_inc;
    else
      y += y_inc;
    
    if(p < 0) {
      p += p_inc;
    }
    else {
      if(dy < dx)
        y += y_inc;
      else
        x += x_inc;
      p += p_inc_xy;
    }
    draw_pixel(x, y);
  }
  CLEAR_PIN_7
}

int edge_function(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x, uint8_t y) {
  return (int)(x2 - x1) * (y - y1) - (y2 - y1) * (x - x1);
}

void draw_triangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3) {
  // takes roughly around 14ms
  SET_PIN_7
  for(int i = 0; i < 128; i++) {
    for(int j = 0; j < 64; j++) {
      int abp = edge_function(x1, y1, x2, y2, i, j);
      int bcp = edge_function(x2, y2, x3, y3, i, j);
      int cap = edge_function(x3, y3, x1, y1, i, j);

      if((abp >= 0 && bcp >= 0 && cap >= 0) || (abp <= 0 && bcp <= 0 && cap <= 0)) {
        draw_pixel(i, j);
      }
    }
  }
  CLEAR_PIN_7
}

void draw_poly_line(uint8_t *points, size_t size, bool is_loop) {
  PORTD |= (1 << PORTD7);
  int i = 0;
  for(i = 0; i < size/2; i+=2) {
    draw_line(points[i], points[i+1], points[i+2], points[i+3]);
  }

  if(is_loop) {
    draw_line(points[i], points[i+1], points[0], points[1]);
  }
  PORTD &= ~(1 << PORTD7);
}

void draw_bitmap(const uint8_t *bitmap_buffer, size_t size, uint16_t x, uint8_t y) {
  SET_PIN_7
  uint8_t index = 0;
  for(int j = 0; j < 2; j++)
    for(int i = 0; i < size/2; i++) {
      framebuffer[(j+y)*128 + i + x] = bitmap_buffer[index++];
    }
  CLEAR_PIN_7
}

int main(void) {
  DDRD |= (1 << PORTD7);
  PORTD &= ~(1 << PORTD7);
  i2c_init();
  _delay_ms(100);
  ssd1306_init();
  _delay_ms(500);
  clear_screen();
  uint8_t points[] = {0, 0, 40, 13, 89, 43};
  //draw_poly_line(points, sizeof(points), true);
  //draw_triangle(15, 15, 50, 0, 63, 63);
  //draw_line(0, 0, 25, 63);
  fill_screen_with_buffer();
  // fill_screen();
  //_delay_ms(1000);
  while (1) {
    for(int i = 50; i < 100; i++) {
      //draw_line(0, 63, i, 0);
      draw_bitmap(slick_arrow_delta, sizeof(slick_arrow_delta), i, 2);
      fill_screen_with_buffer();
      //clear_screen();
    }
  }
}
