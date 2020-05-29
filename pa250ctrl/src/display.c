/*
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/*
 * display.c
 *
 *  Created on: 19.11.2019
 *      Author: kurt
 */

// Low level, hardware related display functions

#include <main.h>

extern const uint8_t font8x8_basic[97][8];

void spi_writeBytes(uint8_t *pb, uint32_t len, uint8_t dc);
void display_setpos(u8 x, u8 y);

void spi_chipSelect(void)
{
	GPIO_WriteBit(GPIOB,DISPLAY_CS_OUT_PB12,0);
}

void spi_chipDeselect(void)
{
	GPIO_WriteBit(GPIOB,DISPLAY_CS_OUT_PB12,1);
}

#define BUFFER_SIZE 200

volatile uint8_t spiTxCounter, spiTXlen;
volatile uint8_t spiDataBuffer[BUFFER_SIZE];
volatile uint8_t dummy;
volatile uint8_t spi_txready = 1;
volatile uint8_t spi_fin = 1;

void SPI2_IRQHandler(void)
{
	if(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == SET)
	{
		// we don't need RX data from display, just ignore it
		dummy  = SPI_I2S_ReceiveData(SPI2);

		if(spi_txready)
		{
			spi_chipDeselect();
			spi_fin = 1;
		}
	}

	if(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == SET)
	{
		if(spiTxCounter < spiTXlen)
		{
			SPI_I2S_SendData(SPI2, spiDataBuffer[spiTxCounter]);
			spiTxCounter++;
		}
		else
		{
			SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, DISABLE);
			spi_txready = 1;
		}
	}
}

void init_SPI()
{
NVIC_InitTypeDef NVIC_InitStructure;
SPI_InitTypeDef SPI_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = SPI2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_128;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CRCPolynomial = 0;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_Init(SPI2, &SPI_InitStructure);

	// enable RX int but not TX interrupt
	SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);

	SPI_Cmd(SPI2, ENABLE);
}

void spi_writeBytes(uint8_t *pb, uint32_t len, uint8_t dc)
{
	uint32_t to=0;

	if(len > BUFFER_SIZE)
		return;

	while(spi_fin == 0)
	{
		if(++to >= 1000000)
		{
			spi_txready = 1;
			spi_fin = 1;
			return;	// SPI is busy for 1s, clean and give up
		}
		delay_1us(1);
	}

	while(spi_fin == 0);

	spi_txready = 0;
	spi_fin = 0;
	spiTxCounter = 0;
	spiTXlen = len;
	for(uint32_t i=0; i<len; i++)
		spiDataBuffer[i] = pb[i];

	spi_chipSelect();
	GPIO_WriteBit(GPIOB, DISPLAY_DC_OUT_PB1, dc);
	SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_TXE, ENABLE);
}

void ssd1322_command(uint8_t cmd)
{
	spi_writeBytes(&cmd,1,RESET);
}

void ssd1322_data(uint8_t data)
{
	spi_writeBytes(&data,1, SET);
}

void ssd1322_dataBytes(uint8_t *buf, uint32_t size)
{
	spi_writeBytes(buf,size, SET);
}

void init_display()
{
	init_SPI();

	// reset display
	GPIO_WriteBit(GPIOB, DISPLAY_RES_OUT_PB10, RESET);
	delay_1ms(10);
	GPIO_WriteBit(GPIOB, DISPLAY_RES_OUT_PB10, SET);
	delay_1ms(10);

	ssd1322_command(SSD1322_SETCOMMANDLOCK);// 0xFD
	ssd1322_data(0x12);// Unlock OLED driver IC

	ssd1322_command(SSD1322_DISPLAYOFF);// 0xAE

	ssd1322_command(SSD1322_SETCLOCKDIVIDER);// 0xB3
	ssd1322_data(0x51);// 0x91 0xB3

	ssd1322_command(SSD1322_SETMUXRATIO);// 0xCA
	ssd1322_data(0x3F);// duty = 1/64

	ssd1322_command(SSD1322_SETDISPLAYOFFSET);// 0xA2
	ssd1322_data(0x00);

	ssd1322_command(SSD1322_SETSTARTLINE);// 0xA1
	ssd1322_data(0x00);

	ssd1322_command(SSD1322_SETREMAP);// 0xA0
	ssd1322_data(0x14);//Horizontal address increment,Disable Column Address Re-map,Enable Nibble Re-map,Scan from COM[N-1] to COM0,Disable COM Split Odd Even
	ssd1322_data(0x11);//Enable Dual COM mode

	ssd1322_command(SSD1322_SETGPIO);// 0xB5
	ssd1322_data(0x00);// Disable GPIO Pins Input

	ssd1322_command(SSD1322_FUNCTIONSEL);// 0xAB
	ssd1322_data(0x01);// selection external vdd

	ssd1322_command(SSD1322_DISPLAYENHANCE);// 0xB4
	ssd1322_data(0xA0);// enables the external VSL
	ssd1322_data(0xFD);// 0xfFD,Enhanced low GS display quality;default is 0xb5(normal),

	ssd1322_command(SSD1322_SETCONTRASTCURRENT);// 0xC1
	ssd1322_data(0xFF);// 0xFF - default is 0x7f
	//ssd1322_data(0x14);

	ssd1322_command(SSD1322_MASTERCURRENTCONTROL);// 0xC7
	ssd1322_data(0x0F);// default is 0x0F

	// Set grayscale
	ssd1322_command(SSD1322_SELECTDEFAULTGRAYSCALE); // 0xB9

 	ssd1322_command(SSD1322_SETPHASELENGTH);// 0xB1
	ssd1322_data(0xE2);// default is 0x74

	ssd1322_command(SSD1322_DISPLAYENHANCEB);// 0xD1
	ssd1322_data(0x82);// Reserved;default is 0xa2(normal)
	ssd1322_data(0x20);//

	ssd1322_command(SSD1322_SETPRECHARGEVOLTAGE);// 0xBB
	ssd1322_data(0x04);// 0.6xVcc

	ssd1322_command(SSD1322_SETSECONDPRECHARGEPERIOD);// 0xB6
	ssd1322_data(0x02);// default

	ssd1322_command(SSD1322_SETVCOMH);// 0xBE
	ssd1322_data(0x07);// 0.86xVcc;default is 0x04

	ssd1322_command(SSD1322_NORMALDISPLAY);// 0xA6

	ssd1322_command(SSD1322_EXITPARTIALDISPLAY);// 0xA9

	ssd1322_command(SSD1322_DISPLAYON);// 0xAF

	display_fill(0);	// Clear image ram

	//display_test();
}

void display_onoff(uint8_t onoff)
{
	if(onoff)
		ssd1322_command(SSD1322_DISPLAYON);// 0xAF
	else
		ssd1322_command(SSD1322_DISPLAYOFF);// 0xAE
}

void display_bright(uint8_t val)
{
	ssd1322_command(SSD1322_SETCONTRASTCURRENT);// 0xC1
	if(val < 0x14) val = 0x14;	// darkest
	ssd1322_data(val);
}

void display_invert(uint8_t i)
{
	if (i) {
		ssd1322_command(SSD1322_INVERSEDISPLAY);
	} else {
		ssd1322_command(SSD1322_NORMALDISPLAY);
	}
}

/*
 * Fill the display with the specified color
 */

void display_fill(uint8_t color)
{
    uint8_t x,y;

    ssd1322_command(SSD1322_SETCOLUMNADDR);
    ssd1322_data(MIN_SEG);
    ssd1322_data(MAX_SEG);

    ssd1322_command(SSD1322_SETROWADDR);
    ssd1322_data(0);
    ssd1322_data(63);

    color = (color & 0x0F) | (color << 4);

    ssd1322_command(SSD1322_WRITERAM);

	for(y=0; y<64; y++)
    {
		for(x=0; x<64; x++)
		{
		    ssd1322_data(color);
		    ssd1322_data(color);
		}
    }
}

// x,y = Position of 8x8 fields
void display_setpos(u8 x, u8 y)
{
	ssd1322_command(SSD1322_SETCOLUMNADDR);
	ssd1322_data(MIN_SEG + x*2);
	ssd1322_data(MIN_SEG + x*2 + 1);

	ssd1322_command(SSD1322_SETROWADDR);
	ssd1322_data(y*8);
    ssd1322_data(y*8+7);
}

// Dim the display
// dim: 1=dimmed, 0=normal
void display_dim(uint8_t dim)
{
	ssd1322_command(SSD1322_SETCONTRASTCURRENT);
	ssd1322_command(1-dim);
}

void display_draw8x8(u8 *buffer, u8 x, u8 y)
{
	// print an 8x8 pixel block
	display_setpos(x,y);

	ssd1322_command(SSD1322_WRITERAM);

	ssd1322_dataBytes(buffer,8*4);
}

// the font map goes top down
// display is y in lines
// x-pos ... in 64 Segments, so 4 Pixel per Segment
// x-pixels ... each pixel has 4 bits, so 4 Bytes (32bit) for the horizontal 8-pixel line
// color ... 0-15
void print_char(char c, uint8_t xpos, uint8_t ypos, uint32_t color, uint8_t mode)
{
	uint8_t idx = c - ' ';
	if(idx >= 97) idx = 0;

	color &= 0x0f;

	// turn font by 90 deg
	uint8_t map[8*4];
	memset(map,0,sizeof(map));

	for(uint8_t y=0; y<8; y++)
	{
		uint32_t pixrow = 0;
		for(uint8_t x=0; x<8; x++)
		{
			uint8_t b = (font8x8_basic[idx][y] & (1<<x))?1:0;
			if(mode == 1)
			{
				// Reverse
				b = 1-b;
			}
			if(b)
			{
				pixrow |=  (color << ((7-x)*4));
			}
		}

		map[y*4 + 0] = (uint8_t)(pixrow >> 24);
		map[y*4 + 1] = (uint8_t)(pixrow >> 16);
		map[y*4 + 2] = (uint8_t)(pixrow >> 8);
		map[y*4 + 3] = (uint8_t)(pixrow >> 0);
	}

	display_draw8x8(map,xpos,ypos);
}

void print_string(char *s, uint8_t x, uint8_t y, uint8_t color, uint8_t mode)
{
	for(u8 i=0; i<strlen(s); i++)
	{
		print_char(s[i],x+i,y,color,mode);
	}

	// and send to USART
	if(strlen(s) < 180)
	{
		char text[200];
		sprintf(text,"PAC %02d %02d %02d %s\n",x,y,mode,s);

		deb_printf(text,strlen(text));
	}
}
