// hy18spi.c
//
// Example program for HY-1.8 SPI LCD using bcm2835 library.
// Shows how to interface with SPI and draws couple of patterns.
//
// After installing bcm2835, you can build it with this command 
// gcc -o hy18spi hy18spi.c -l bcm2835
// and then test
// sudo ./hy18spi
//
// Or you can you can modify Makefile 
//
// Detailed Raspberry Pi GPIO info can be found here:
// http://www.combinatorialdesign.com/boards/Raspberry_Pi/
//
// Author: Pawel Wodnicki
// Copyright (C) 2013 Pawel Wodnicki
// $Id: $

/*
   Most of this code has been borrowed from ST7735 frame 
   buffer driver in drivers/video/st7735 and some form 
   bcm2835 library spi example.
   This example is licensed under GPL V2 same as bcm2835 library.
   
*/

/*
HY-1.8 SPI connections

HY-1.8 J1        Raspberry Pi P1      bcm2835 lib pin
1 - GND          - P1.6 GND
2 - VCC          - P1.2 5V Power
3 - NC
4 - NC
5 - NC
6 - RESET        - P1.22 GPIO25        RPI_V2_GPIO_P1_22
7 - A0           - P1.18 GPIO24        RPI_V2_GPIO_P1_18
8 - SDA          - P1.19 GPIO10 MOSI   RPI_V2_GPIO_P1_19
9 - SCK          - P1.23 GPIO11 SCLK   RPI_V2_GPIO_P1_23
10 - CS          - P1.24 GPIO8 CE0     RPI_V2_GPIO_P1_24
11 - SD_SCK
12 - SD_MISO
13 - SD_MOSI
14 - SD_CS
15 - LED+         - P1.4 5V Power
16 - LED-         - P1.9 GND

*/

#include <bcm2835.h>
#include <stdio.h>
#include <unistd.h>
#include "st7735.h"
#include "progfont.h"

#define pgm_read_byte(addr) (*(const unsigned char *)(addr))

// forwad declarations
void hy18spi_init_gpio(void);
void hy18spi_end_gpio(void);
void hy18spi_reset();
void hy18spi_init(void);
void hy18spi_end(void);
uint8_t hy18spi_transfer_cmd(uint8_t value);
uint8_t hy18spi_transfer_data(uint8_t value);
void hy18spi_probe();
void hy18spi_set_rgb(uint16_t color);
void hy18spi_set_pixel(uint16_t x, uint16_t y, uint16_t color);
void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size);
void writeText(uint8_t c);
void setCursor(int16_t x, int16_t y);
void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color);
void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color);
void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color);
void swap(int16_t a, int16_t b);
void printText(const char *str);

#define HY18SPI_WIDTH 128
#define HY18SPI_HEIGHT 160
#define textcolor 0xffff
#define textbgcolor 0x0000
#define wrap 1
#define textsize 1

// configuration - comment to disable
#define CONFIG_FB_ST7735_PANEL_TYPE_RED_TAB 1
#define CONFIG_FB_ST7735_RGB_ORDER_REVERSED 1

#if ( CONFIG_FB_ST7735_PANEL_TYPE_RED_TAB == 1 )
# define ST7735_COLSTART 0
# define ST7735_ROWSTART 0
#else
# define ST7735_COLSTART 2
# define ST7735_ROWSTART 1
#endif

int16_t cursor_y=0;
int16_t cursor_x=0;

/* Init script function */
struct st7735_function {
        uint16_t cmd;
        uint16_t data;
};

/* Init script commands */
enum st7735_cmd {
        ST7735_START,
        ST7735_END,
        ST7735_CMD,
        ST7735_DATA,
        ST7735_DELAY
};

static struct st7735_function st7735_cfg_script[] = {
        { ST7735_START, ST7735_START},
        { ST7735_CMD, ST7735_SWRESET},
        { ST7735_DELAY, 150},
        { ST7735_CMD, ST7735_SLPOUT},
        { ST7735_DELAY, 500},
        { ST7735_CMD, ST7735_FRMCTR1},
        { ST7735_DATA, 0x01},
        { ST7735_DATA, 0x2c},
        { ST7735_DATA, 0x2d},
        { ST7735_CMD, ST7735_FRMCTR2},
        { ST7735_DATA, 0x01},
        { ST7735_DATA, 0x2c},
        { ST7735_DATA, 0x2d},
        { ST7735_CMD, ST7735_FRMCTR3},
        { ST7735_DATA, 0x01},
        { ST7735_DATA, 0x2c},
        { ST7735_DATA, 0x2d},
        { ST7735_DATA, 0x01},
        { ST7735_DATA, 0x2c},
        { ST7735_DATA, 0x2d},
        { ST7735_CMD, ST7735_INVCTR},
        { ST7735_DATA, 0x07},
        { ST7735_CMD, ST7735_PWCTR1},
        { ST7735_DATA, 0xa2},
        { ST7735_DATA, 0x02},
        { ST7735_DATA, 0x84},
        { ST7735_CMD, ST7735_PWCTR2},
        { ST7735_DATA, 0xc5},
        { ST7735_CMD, ST7735_PWCTR3},
        { ST7735_DATA, 0x0a},
        { ST7735_DATA, 0x00},
        { ST7735_CMD, ST7735_PWCTR4},
        { ST7735_DATA, 0x8a},
        { ST7735_DATA, 0x2a},
        { ST7735_CMD, ST7735_PWCTR5},
        { ST7735_DATA, 0x8a},
        { ST7735_DATA, 0xee},
        { ST7735_CMD, ST7735_VMCTR1},
        { ST7735_DATA, 0x0e},
        { ST7735_CMD, ST7735_INVOFF},
        { ST7735_CMD, ST7735_MADCTL},
#if ( CONFIG_FB_ST7735_RGB_ORDER_REVERSED == 1 )
        { ST7735_DATA, 0xc0},
#else
        { ST7735_DATA, 0xc8},
#endif
        { ST7735_CMD, ST7735_COLMOD},
        { ST7735_DATA, 0x05},
#if 0 /* set_addr_win will set these, so no need to set them at init */
        { ST7735_CMD, ST7735_CASET},
        { ST7735_DATA, 0x00},
        { ST7735_DATA, 0x00 + ST7735_COLSTART},
        { ST7735_DATA, 0x00},
        { ST7735_DATA, HY18SPI_WIDTH - 1 + ST7735_COLSTART},
        { ST7735_CMD, ST7735_RASET},
        { ST7735_DATA, 0x00},
        { ST7735_DATA, 0x00 + ST7735_ROWSTART},
        { ST7735_DATA, 0x00},
        { ST7735_DATA, HY18SPI_HEIGHT - 1 + ST7735_ROWSTART},
#endif
        { ST7735_CMD, ST7735_GMCTRP1},
        { ST7735_DATA, 0x02},
        { ST7735_DATA, 0x1c},
        { ST7735_DATA, 0x07},
        { ST7735_DATA, 0x12},
        { ST7735_DATA, 0x37},
        { ST7735_DATA, 0x32},
        { ST7735_DATA, 0x29},
        { ST7735_DATA, 0x2d},
        { ST7735_DATA, 0x29},
        { ST7735_DATA, 0x25},
        { ST7735_DATA, 0x2b},
        { ST7735_DATA, 0x39},
        { ST7735_DATA, 0x00},
        { ST7735_DATA, 0x01},
        { ST7735_DATA, 0x03},
        { ST7735_DATA, 0x10},
        { ST7735_CMD, ST7735_GMCTRN1},
        { ST7735_DATA, 0x03},
        { ST7735_DATA, 0x1d},
        { ST7735_DATA, 0x07},
        { ST7735_DATA, 0x06},
        { ST7735_DATA, 0x2e},
        { ST7735_DATA, 0x2c},
        { ST7735_DATA, 0x29},
        { ST7735_DATA, 0x2d},
        { ST7735_DATA, 0x2e},
        { ST7735_DATA, 0x2e},
        { ST7735_DATA, 0x37},
        { ST7735_DATA, 0x3f},
        { ST7735_DATA, 0x00},
        { ST7735_DATA, 0x00},
        { ST7735_DATA, 0x02},
        { ST7735_DATA, 0x10},
///#if 0 /* init_display will turn on the display after clearing it */
        { ST7735_CMD, ST7735_DISPON},
        { ST7735_DELAY, 100},
///#endif
        { ST7735_CMD, ST7735_NORON},
        { ST7735_DELAY, 10},
        { ST7735_END, ST7735_END},
};

void bild()
{
   unsigned int pin;
   int x,y;
	
// If you call this, it will not actually access the GPIO
// Use for testing
//        bcm2835_set_debug(1);

    // intialize bcm2835 library
    if (!bcm2835_init())
        return;

    // initialize spi
    bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE3);                   // MODE 3
///    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
/// NOT OK    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_4); //< 4 = 16ns = 62.5MHz
/// OK
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_8); //< 8 = 32ns = 31.25MHz
/// OK   bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_16); //< 16 = 64ns = 15.625MHz
/// OK   bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32); //< 32 = 128ns = 7.8125MHz
/// OK   bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64); //< 64 = 256ns = 3.90625MHz - 4000000 spi clock
/// OK   bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_65536); // The default
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default

    // initialize gpio
    hy18spi_init_gpio();

// pin testing - uncomment one line here and from bcm2835_gpio_fsel to do{...}while(1); loop
// 6 OK pin = RPI_V2_GPIO_P1_22; // RESET - GPIO25
// 7 OK pin = RPI_V2_GPIO_P1_18; // A0 - GPIO24
// 8 OK pin = RPI_V2_GPIO_P1_19; // SDA - GPIO10 MOSI
// 9 OK pin = RPI_V2_GPIO_P1_23; // SCK - GPIO11 SCLK
// 10 OK pin = RPI_V2_GPIO_P1_24; // CS - GPIO8 CE0
//    bcm2835_gpio_fsel(pin, BCM2835_GPIO_FSEL_OUTP);
//    do {
//       bcm2835_gpio_clr(pin);
//       delay(5);
//       bcm2835_gpio_set(pin);
//       delay(5);
//    } while(1);

    // initialize display
    hy18spi_init();
  /*  
    // draw pattern
     hy18spi_set_rgb(0x07e0);
 
    // wait a 10 sec
    delay(3000);
 hy18spi_set_rgb(0x00ff);
 
    // wait a 10 sec
    delay(3000);
  */  
  hy18spi_set_rgb(0x0000);
    drawChar(1, 0, 'h', 0xffff, 0x0000, 1);
    drawChar(10, 2, 'a', 0xffff, 0x0000, 1);
    drawChar(20, 2, 'l', 0xffff, 0x0000, 1);
    drawChar(30, 2, 'l', 0xffff, 0x0000, 1);
    drawChar(40, 2, 'o', 0xffff, 0x0000, 1);
    setCursor(20,20);
    char *text="hallo dies ist ein langer Text um das tft zu testen blablabla ud so weiter mal schauen";
    printText(text);
    delay(3000);
    // deinitialize display
    hy18spi_end();

    // deintialize spi
    bcm2835_spi_end();

    // deintialize bcm2835 library
    bcm2835_close();

}

void startTft() {
	unsigned int pin;
   int x,y;
    if (!bcm2835_init())
        return;
    bcm2835_spi_begin();
    bcm2835_spi_setBitOrder(BCM2835_SPI_BIT_ORDER_MSBFIRST);      // The default
    bcm2835_spi_setDataMode(BCM2835_SPI_MODE3);                   // MODE 3
///    bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);                   // The default
/// NOT OK    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_4); //< 4 = 16ns = 62.5MHz
/// OK
    bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_8); //< 8 = 32ns = 31.25MHz
/// OK   bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_16); //< 16 = 64ns = 15.625MHz
/// OK   bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_32); //< 32 = 128ns = 7.8125MHz
/// OK   bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_64); //< 64 = 256ns = 3.90625MHz - 4000000 spi clock
/// OK   bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_65536); // The default
    bcm2835_spi_chipSelect(BCM2835_SPI_CS0);                      // The default
    bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, LOW);      // the default
    hy18spi_init_gpio();
    hy18spi_init();
}

void printText(const char *str) {
	while(*str)
	writeText(*str++);
}

// hardware reset
void hy18spi_reset()
{
  bcm2835_gpio_clr(RPI_V2_GPIO_P1_22); //GPIO25 - RESET
  delay(10);
  bcm2835_gpio_set(RPI_V2_GPIO_P1_22); //GPIO25 - RESET
  delay(120);
}

void hy18spi_init_gpio()
{
   // SPI pins have been initialized including GPIO8 - CS
   // A0 and RESET pins set to output
   bcm2835_gpio_fsel(RPI_V2_GPIO_P1_18, BCM2835_GPIO_FSEL_OUTP); // A0 - GPIO24
   bcm2835_gpio_fsel(RPI_V2_GPIO_P1_22, BCM2835_GPIO_FSEL_OUTP); // RESET - GPIO25
   // A0 set high
   bcm2835_gpio_set(RPI_V2_GPIO_P1_18); //GPIO24 - A0
   // RESET set high
   bcm2835_gpio_set(RPI_V2_GPIO_P1_22); //GPIO25 - RESET
   // CS set high
   bcm2835_gpio_set(RPI_V2_GPIO_P1_24); //GPIO8 - CS

}

// gpio deinit
void hy18spi_end_gpio()
{
   // A0 and RESET pins set to input
   bcm2835_gpio_fsel(RPI_V2_GPIO_P1_18, BCM2835_GPIO_FSEL_INPT); // A0 - GPIO24
   bcm2835_gpio_fsel(RPI_V2_GPIO_P1_22, BCM2835_GPIO_FSEL_INPT); // RESET - GPIO25
}

uint8_t hy18spi_transfer_cmd(uint8_t value)
{
   bcm2835_gpio_clr(RPI_V2_GPIO_P1_18); //GPIO24 - A0
   return bcm2835_spi_transfer(value);
}

uint8_t hy18spi_transfer_data(uint8_t value)
{
   bcm2835_gpio_set(RPI_V2_GPIO_P1_18); //GPIO24 - A0
   return bcm2835_spi_transfer(value);
}

void hy18spi_set_addr_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
  hy18spi_transfer_cmd(ST7735_CASET);         // column addr set
  hy18spi_transfer_data(0x00);
  hy18spi_transfer_data(x0+ST7735_COLSTART);  // XSTART
  hy18spi_transfer_data(0x00);
  hy18spi_transfer_data(x1+ST7735_COLSTART);  // XEND

  hy18spi_transfer_cmd(ST7735_RASET);         // row addr set
  hy18spi_transfer_data(0x00);
  hy18spi_transfer_data(y0+ST7735_ROWSTART);  // YSTART
  hy18spi_transfer_data(0x00);
  hy18spi_transfer_data(y1+ST7735_ROWSTART);  // YEND
}

void hy18spi_set_rgb(uint16_t color)
{
  uint8_t x, y;
  hy18spi_set_addr_window(0, 0, HY18SPI_WIDTH - 1, HY18SPI_HEIGHT - 1);
  hy18spi_transfer_cmd(ST7735_RAMWR);  // write to RAM
  for (x=0; x < HY18SPI_WIDTH; x++)
  {
    for (y=0; y < HY18SPI_HEIGHT; y++)
    {
      hy18spi_transfer_data(color >> 8);
      hy18spi_transfer_data(color);
    }
  }
  hy18spi_transfer_cmd(ST7735_NOP);
}

void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) {
	if((x >= HY18SPI_WIDTH) || (y >= HY18SPI_HEIGHT) || ((x + 6*size-1)<0) || ((y + 8 *size-1<0)))
		return;
		int8_t i;
		int8_t  j;
	for (i=0;i<6;i++) {
		uint8_t line;
		if (i==5)
			line=0x0;
		else
			line=pgm_read_byte(font+(c*5)+i);
		for (j = 0; j<8; j++) {
		  if (line & 0x1) {
			if (size == 1) // default size
			  hy18spi_set_pixel(x+i, y+j, color);
			else { // big size
			  fillRect(x+i*size, y+j*size, size, size, color);
			}
		  } else if (bg != color) {
			if (size == 1) // default size
			  hy18spi_set_pixel(x+i, y+j, bg);
			else { // big size
			  fillRect(x+i*size, y+j*size, size, size, bg);
			}
		  }
		  line >>= 1;
		}
	}
}

void writeText(uint8_t c) {
  if (c == '\n') {
    cursor_y += textsize*8;
    cursor_x = 0;
  } else if (c == '\r') {
    // skip em
  } else {
    drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
    cursor_x += textsize*6;
    if (wrap && (cursor_x > (HY18SPI_WIDTH - textsize*6))) {
      cursor_y += textsize*8;
      cursor_x = 0;
    }
  }
}

void setCursor(int16_t x, int16_t y) {
  cursor_x = x;
  cursor_y = y;
}

void hy18spi_set_pixel(uint16_t x, uint16_t y, uint16_t color)
{
  hy18spi_set_addr_window(x,y,x+1,y+1);
  hy18spi_transfer_cmd(ST7735_RAMWR);  // write to RAM
  hy18spi_transfer_data(color >> 8);
  hy18spi_transfer_data(color);
}

static void hy18spi_run_cfg_script()
{
        int i = 0;
        int end_script = 0;

        do {
                switch (st7735_cfg_script[i].cmd)
                {
                case ST7735_START:
                        break;
                case ST7735_CMD:
                        hy18spi_transfer_cmd(st7735_cfg_script[i].data & 0xff);
                        break;
                case ST7735_DATA:
                        hy18spi_transfer_data(st7735_cfg_script[i].data & 0xff);
                        break;
                case ST7735_DELAY:
                        delay(st7735_cfg_script[i].data);
                        break;
                case ST7735_END:
                        end_script = 1;
                }
                i++;
        } while (!end_script);
}
// hy-1.8 init
void hy18spi_init(void)
{
  hy18spi_reset();
  hy18spi_run_cfg_script();
}

// hy-1.8 deinit
void hy18spi_end(void)
{
  hy18spi_end_gpio();
}


//Bresenham Algorythm
void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
  int dx =  abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1;
  int err = dx+dy, e2; /* error value e_xy */
 
  for(;;){  /* loop */
    hy18spi_set_pixel(x0,y0, color);
    if (x0==x1 && y0==y1) break;
    e2 = 2*err;
    if (e2 > dy) { err += dy; x0 += sx; } /* e_xy+e_x > 0 */
    if (e2 < dx) { err += dx; y0 += sy; } /* e_xy+e_y < 0 */
  }
}

void drawFastVLine(int16_t x, int16_t y, int16_t h, uint16_t color) {
  // Update in subclasses if desired!
  drawLine(x, y, x, y+h-1, color);
}

void drawFastHLine(int16_t x, int16_t y, int16_t w, uint16_t color) {
  // Update in subclasses if desired!
  drawLine(x, y, x+w-1, y, color);
}

void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  // Update in subclasses if desired!
  int16_t i;
  for (i=x; i<x+w; i++) {
    drawFastVLine(i, y, h, color);
  }
}

void swap(int16_t a, int16_t b) {
	int16_t t=a;
	a=b;
	b=t;
}
