#ifndef I2C_H_
#define I2C_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <avr/interrupt.h>
#include <avr/io.h>
#include <stdbool.h>
#include <util/delay.h>

void i2c_init();
bool i2c_send_start_bit();
bool i2c_send_address(char addr);
bool i2c_send_byte(char byte);
void i2c_send_stop();
bool i2c_send_buffer(uint8_t *buffer, uint8_t size);

#ifdef __cplusplus
}
#endif

#endif // I2C_H_