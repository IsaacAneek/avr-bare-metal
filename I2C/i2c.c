#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>

#define CHECK_TWINT_SET !(TWCR & (1 << TWINT))
#define TWI_STATUS_CODE (TWSR & 0xF8)
#define START 0x08
#define MT_DATA_ACK 0x28
#define MT_SLA_ACK 0x18
#define TWI_FREQ 100000L

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