For the last couple of weeks, I was trying to learn how to write bare metal driver code for different peripherals/sensors for AVR. This repo is just a manifestation of my hobby challenge.
I managed to write a couple of driver codes for different features from scratch and many more to write in future.

### Here is my current progress:
 - [ ] UART
   - [x] Minimal driver
   - [ ] Advanced UART/USART 
 - [ ] I2C
   - [x] Minimal driver
   - [ ] High speed I2C
   - [ ] Advanced I2C (Multi master/multi slave, bus arbitration) 
 - [ ] SSD_1306_OLED 128x64
   - [x] Minimal driver
   - [x] Rasterizer
   - [ ] High FPS
     - [ ] Try high speed I2C
       - [x] 400khz
       - [x] 800khz
     - [ ] Try high speed SPI
     - [ ] SPI + DMA
   - [ ] High display refresh rate
     - [x] Increase oscillator frequency
     - [ ] Use external oscillator
     - [ ] Decrease pre-charge period (default is 2, try 1)
     - [ ] Check if BANK0 pulse width is configurable
     - [ ] Try decreasing MUX ratio
   - [ ] Profile code to display latency
     - [ ] Probe SEG/COM pins with an oscilloscope
   - [ ] Profile funtion cycle count
     - [x] GPIO based profiling
     - [ ] Timer with x1 prescaler based profiling
     - [ ] Check ASM dump
 - [ ] ISP
 - [ ] SPI
 - [ ] USB
 - [ ] Interrupt
 - [ ] Bootloader
 - [ ] Linker script
 - [ ] Startup code
