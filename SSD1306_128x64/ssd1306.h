#ifndef SSD1306_H_
#define SSD1306_H_

#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <math.h>
#include <stdbool.h>
#include <util/delay.h>
#include "cursor.h"

void ssd1306_init();
void clear_screen();
void fill_screen();
void fill_screen_with_buffer();
void draw_pixel(uint8_t x, uint8_t y);
void clear_pixel(uint8_t x, uint8_t y);
void draw_line(uint8_t start_x, uint8_t start_y, uint8_t finish_x, uint8_t finish_y);
int edge_function(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x, uint8_t y);
void draw_triangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t x3, uint8_t y3);
void draw_poly_line(uint8_t *points, size_t size, bool is_loop);
void draw_bitmap(const uint8_t *bitmap_buffer, size_t size, uint16_t x, uint8_t y);

#endif // SSD1306_H_