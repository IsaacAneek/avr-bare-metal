#ifndef USART_H_
#define USART_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <avr/io.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>

void usart_init();
void usart_putc(uint8_t ch);
void usart_puts(char *str);
bool usart_is_string_available();
bool usart_is_byte_available();
char usart_getc();
void usart_gets(char *buffer);
void usart_putHex(char c);
void usart_println(char *str);


#ifdef __cplusplus
}
#endif

#endif // USART_H_