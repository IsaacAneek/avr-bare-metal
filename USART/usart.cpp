#include <avr/io.h>
#include <string.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#define UART_INTERRUPT_BASED
#define BAUD_RATE 9600
#define UBRR0_VAL F_CPU / 16 / BAUD_RATE - 1

uint8_t uart_receive_buffer[64];
uint8_t tail = 0;
uint8_t head = 0;

ISR(USART_RX_vect)
{
  cli();
  uart_receive_buffer[tail] = UDR0;
  tail = (tail + 1) % 64;
  sei();
}

void usart_init(uint16_t ubrr)
{
  sei();
  // Initialize Baud rate
  UBRR0L = (uint8_t)(ubrr & 0xFF);
  UBRR0H = (uint8_t)(ubrr >> 8);
  // Enable USART Receive and Transmit
  UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
  // 8N1
  UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

void usart_putc(uint8_t ch)
{
  while (!(UCSR0A & (1 << UDRE0)))
    ;
  UDR0 = ch;
}

void usart_puts(char *str)
{
  // Assumes the string ends in NULL character '\0'
  while (*str > 0)
    usart_putc(*str++);
}

bool usart_is_string_available()
{
  // We store the byte and then increment the tail
  return uart_receive_buffer[tail - 1] == '\n';
}

bool usart_is_byte_available()
{
#ifdef UART_POLLING_BASED
  // Returns 1 when there is a byte available in RX buffer.
  // But still its problematic, because what if the main loop
  // is busy doing other work and the user misses this availability
  // of a new byte. Before they get the time to process this new byte
  // another byte might just come and replace it. Hence
  // data loss occurs.
  // Solution : use interrupt
  return ((UCSR0A & (1 << RXC0)));
#endif
#ifdef UART_INTERRUPT_BASED
  return (tail - head) != 0;
#endif
}

char usart_getc()
{
#ifdef UART_POLLING_BASED
  // This part is blocking (?)
  while (!(UCSR0A & (1 << RXC0)))
    ;
  return UDR0;
#endif
#ifdef UART_INTERRUPT_BASED
  char c = uart_receive_buffer[head];
  head = (head + 1) % 64;
  return c;
#endif
}

void usart_gets(char *buffer)
{
  char ch;
  uint8_t i = 0;
  do
  {
    ch = usart_getc();
    buffer[i] = ch;
    // calling another usart_getc() before the loop  will cause issues,
    // loop will attempt to read a (n+1)th byte which isnt there,
    // so it will block until another key is pressed
  } while (buffer[i++] != '\n');
  // windows terminal  sends '\r\n' at the end of a string so yea

  buffer[i] = '\0';
}

void usart_putHex(char c)
{
  // divide the 8 byte into 4 byte upper, 4 byte lower
  // convert the 4 byte into a number if less than 10 or alphabet if greater or equal than 10
  uint8_t lowerNibble = c & 0x0F;
  uint8_t upperNibble = (c >> 4) & 0x0F;

  uint8_t hexUpperNibble = (upperNibble <= 9) ? '0' + upperNibble : 'A' + (upperNibble - 10);
  uint8_t hexLowerNibble = (lowerNibble <= 9) ? '0' + lowerNibble : 'A' + (lowerNibble - 10);
  usart_putc(hexUpperNibble);
  usart_putc(hexLowerNibble);
}

void usart_println(char *str)
{
  usart_puts(str);
  usart_putc('\r');
  usart_putc('\n');
}

int main(void)
{
  // Serial.println("SOMETHING");

  usart_init(UBRR0_VAL);
  // usart_puts("Hello World\n\0");
  char buffer[40] = "\nDo you wanna proceed?\r\n";
  usart_puts(buffer);

  while (1)
  {
    if (usart_is_byte_available())
    {
      usart_puts("A byte is here yeeeh\r\n");

      if (usart_is_string_available())
      {
        usart_gets(buffer);
        usart_puts(buffer);
        if (strcasecmp(buffer, "YES\r\n\0") == 0)
        {
          usart_println("Proceeding......!");
        }
        else
        {
          usart_println("Skipping procedure....!");
        }

        // converts the response from the terminal into hex
        // just for debugging purposes
        // int i = 0;
        // do
        // {
        //   usart_putHex(buffer[i]);
        //   usart_putc(' ');
        // } while (buffer[i++] != '\0');
        // usart_putc('\r');
        // char c = usart_getc();
        // usart_putc(c);
      }
    }
    _delay_ms(1000);
  }
  return 0;
}