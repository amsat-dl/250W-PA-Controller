/*
 * display.h
 *
 *  Created on: 19.11.2019
 *      Author: kurt
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#define SSD1322_LCDWIDTH                256
#define SSD1322_LCDHEIGHT               64
#define SSD1322_BITS_PER_PIXEL			4

#define MIN_SEG	0x1C
#define MAX_SEG	0x5B

#define SSD1322_SETCOMMANDLOCK 0xFD
#define SSD1322_DISPLAYOFF 0xAE
#define SSD1322_DISPLAYON 0xAF
#define SSD1322_SETCLOCKDIVIDER 0xB3
#define SSD1322_SETDISPLAYOFFSET 0xA2
#define SSD1322_SETSTARTLINE 0xA1
#define SSD1322_SETREMAP 0xA0
#define SSD1322_FUNCTIONSEL 0xAB
#define SSD1322_DISPLAYENHANCE 0xB4
#define SSD1322_SETCONTRASTCURRENT 0xC1
#define SSD1322_MASTERCURRENTCONTROL 0xC7
#define SSD1322_SETPHASELENGTH 0xB1
#define SSD1322_DISPLAYENHANCEB 0xD1
#define SSD1322_SETPRECHARGEVOLTAGE 0xBB
#define SSD1322_SETSECONDPRECHARGEPERIOD 0xB6
#define SSD1322_SETVCOMH 0xBE
#define SSD1322_NORMALDISPLAY 0xA6
#define SSD1322_INVERSEDISPLAY 0xA7
#define SSD1322_SETMUXRATIO 0xCA
#define SSD1322_SETCOLUMNADDR 0x15
#define SSD1322_SETROWADDR 0x75
#define SSD1322_WRITERAM 0x5C
#define SSD1322_ENTIREDISPLAYON 0xA5
#define SSD1322_ENTIREDISPLAYOFF 0xA4
#define SSD1322_SETGPIO 0xB5
#define SSD1322_EXITPARTIALDISPLAY 0xA9
#define SSD1322_SELECTDEFAULTGRAYSCALE 0xB9

// resolution
#define SSD1322_MODE          256*64
#define SSD1322_PIXDEPTH        4
#define SSD1322_COL_SEG_START   0x1C    // 28
#define SSD1322_COL_SEG_END     0x5B    // 91
#define SSD1322_ROW_START       0x00    // 0
#define SSD1322_ROW_END         0x3F    // 63
#define SSD1322_SEGMENTS        32
#define SSD1322_ROWS            64
#define SSD1322_MAX_BRIGHTNESS  127
#define SSD1322_FBSIZE_INT32    SSD1322_SEGMENTS * SSD1322_ROWS                             // 2048
#define SSD1322_MAX_CHARS       ((((SSD1322_SEGMENTS * 8) / FNT_MIN_CHAR_WIDTH) * \
                                    (SSD1322_ROWS / FNT_MAX_CHAR_HEIGHT)) + 2)            // floor

#define DARK	8
#define BRIGHT  15

void init_display();
void display_invert(uint8_t i);
void display_fill(uint8_t color);
void display_dim(uint8_t dim);
void print_char(char c, uint8_t xpos, uint8_t ypos, uint32_t color, uint8_t mode);
void print_string(char *s, uint8_t x, uint8_t y, uint8_t color, uint8_t mode);
void display_onoff(uint8_t onoff);
void display_bright(uint8_t val);


#endif /* DISPLAY_H_ */
