#include "bitmap.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdbool.h>
#include <util/delay.h>

#define CHECK_TWINT_SET !(TWCR & (1 << TWINT))
#define TWI_STATUS_CODE (TWSR & 0xF8)
#define START 0x08
#define MT_DATA_ACK 0x28
#define MT_SLA_ACK 0x18
#define TWI_FREQ 100000L

#define SSD1306_ADDRESS_WRITE 0x78
#define CONTROL_MULTIPLEDATA_DATA 0x40
#define CONTROL_SINGLEDATA_DATA 0xC0
#define CONTROL_MULTIPLEDATA_COMMAND 0x00
#define CONTROL_SINGLEDATA_COMMAND 0x80
#define SCREEN_WIDTH 128
#define PAGE_SIZE 8

uint8_t ssd1306_init_commands[] = {
    0xAE, // display OFF initially
    0xA4, // display follows RAM content
    0x81, // contrast command
    0x7F, // set contrast value
    0xA6, // normal (non-inverted) display mode
    0x20, // set addressing mode command
    0x00, // horizontal addressing mode
    0x21, // set col address cmd
    0x00, // set col start to 0
    0x7F, // set col end adderss to 127
    0x40, // set display start line
    0xA0, // col 0 is mapped to seg 0
    0xA8, // set mux ratio cmd
    0x3F, // set mux ratio value
    0xC0, // com scan direction
    0xD9, // set pre charge period cmd
    0x22, // pre charge value
    0xD5, // set display clock cmd
    0x80, // set display clock value
    0x22, // set page address cmd
    0x00, // set page start addr to 0
    0x07, // set page end addr to 7
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

  int i = 4;
  while (i--) {
    i2c_send_start_bit();
    i2c_send_address(SSD1306_ADDRESS_WRITE);
    i2c_send_byte(CONTROL_MULTIPLEDATA_DATA);
    for (int i = 0; i < 32; i++) {
      i2c_send_byte(0x00);
    }
    i2c_send_stop();
  }
}

void fill_screen() {
    for (int page = 0; page < 8; page++) {
      i2c_send_start_bit();
      i2c_send_address(SSD1306_ADDRESS_WRITE);
      i2c_send_byte(CONTROL_MULTIPLEDATA_DATA);
      for (int i = 0; i < 128; i++) {
        i2c_send_byte(0xFF);
      }
      i2c_send_stop();
    }
}

void fill_screen_with_buffer(const unsigned char* buffer) {
  i2c_send_start_bit();
  i2c_send_address(SSD1306_ADDRESS_WRITE);
  i2c_send_byte(CONTROL_MULTIPLEDATA_DATA);
    
  uint8_t count = 0;
  for(int page = 0; page < 8; page++) {
    for(int i = 0; i < 128; i++) {
      i2c_send_byte(~hello[count++]);
   }
    // for(int i = 112; i < 128; i++) {
    //   i2c_send_byte(0xFF);
    // }
  }
  
  i2c_send_stop();
}

int main(void) {
  i2c_init();
  _delay_ms(100);
  ssd1306_init();
  _delay_ms(500);
  //fill_screen();
  _delay_ms(1000);
  // clear_screen();
  fill_screen_with_buffer(hello);
  while (1) {
    // i2c_send_start_bit();
    // i2c_send_address(SSD1306_ADDRESS_WRITE);
    // i2c_send_byte(CONTROL_MULTIPLEDATA_DATA);

    int i = 200;
    while (i--) {
      // i2c_send_byte(0xFF);
    }
    // i2c_send_stop();
  }
  return 0;
}
