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
 * dht11.c
 *
 *  Created on: 27.11.2019
 *      Author: kurt
 */

#include <main.h>

int8_t dht11_temp = -99;
uint8_t dht11_humidity = 250;

// 1=ok, 0=timeout
uint8_t wait_for_low()
{
	uint32_t to = 1000;
	while(GPIO_ReadInputDataBit(GPIOC, WRR_IN_PC12) != 0)
	{
		delay_1us(1);
		if (to-- == 0) return 0;
	}
	return 1;
}

// 1=ok, 0=timeout
uint8_t wait_for_high()
{
	uint32_t to = 1000;
	while(GPIO_ReadInputDataBit(GPIOC, WRR_IN_PC12) == 0)
	{
		delay_1us(1);
		if (to-- == 0) return 0;
	}
	return 1;
}

uint8_t _readDHT11()
{
	uint8_t data[5];
	uint8_t idx = 0;
	uint8_t bitpos = 7;

	memset(data,0,sizeof(data));

	// send a start signal to the DHT11
	// this is a low level with >18ms
	GPIO_WriteBit(GPIOD,WRT_OUT_PD2,0);
	delay_1ms(20);
	GPIO_WriteBit(GPIOD,WRT_OUT_PD2,1);

	// after 70us the DHT11 acks with low level
	delay_1us(70);
	if(GPIO_ReadInputDataBit(GPIOC, WRR_IN_PC12) != 0)
		return 255;	// DHT11 does not respond

	// wait until ack is released
	if(wait_for_high() == 0)  return 254;	// bus not released

	// now the DHT11 keeps bus high for 80us
	delay_1us(40);
	if(GPIO_ReadInputDataBit(GPIOC, WRR_IN_PC12) == 0)
		return 253;	// DHT11 does not keep it high

	// now the data transmission of 40 bits = 5 byte starts

	// each bit begins with a start condition, which is a low level for 50us
	// followed by a high level with len=26-28us: bit=0
	// or len=70us: bit=1
	for (uint8_t i=0; i<40; i++)
	{
		// wait for start condition and measure length
		if(wait_for_low() == 0) return 252;	// no start condition
		uint32_t t = micros();
		if(wait_for_high() == 0) return 251;	// start condition did not end
		uint32_t tlen = micros() - t;
		if(tlen < 40 || tlen > 60) return 250;	// start condition wrong length

		// after the start the bus now is high, which is the data bit
		// measure the length of the data bit
		t = micros();
		if(wait_for_low() == 0) return 249;	// next start condition
		tlen = micros() - t;				// length of the data bit
		uint8_t bit = 0;
		if(tlen > 45) bit = 1;

		// write bit into data buffer
		if(bit == 1) data[idx] |= (1<<bitpos);

		if(bitpos == 0)
		{
			bitpos = 7;
			idx++;
		}
		else
			bitpos--;
	}

	// all 5 bytes read
	uint8_t sum = data[0] + data[1] + data[2] + data[3];

	if (data[4] != sum)
		return 248;

	// data evaluation for DHT22 or DS2302
	int16_t v = data[0];
	v <<= 8;
	v |= data[1];
	v /= 10;

	dht11_humidity = v;

	v = data[2];
	v <<= 8;
	v |= data[3];
	// v = Temperature in 1/10 degC, we only need full degC
	v /= 10;
	// negative if data[2] bit 7 is set
	if(data[2] & 0x80)
		v = -v;

	dht11_temp = (int8_t)v;

	/*char s[40];
	sprintf(s,"%d %d %d %d %d:%d  ",data[0],data[1],data[2],data[3],data[4],sum);
	print_string(s, 0, 1, BRIGHT, 1);*/

	return 0;
}

void readDHT11()
{
	uint8_t ret = _readDHT11();

	char s[40];
	sprintf(s,"%d %d %d",ret,dht11_temp,dht11_humidity);
	//print_string(s, 0, 1, BRIGHT, 1);
}
