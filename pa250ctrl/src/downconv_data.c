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
 * downconv_data.c
 *
 *  Created on: 21.11.2019
 *      Author: kurt
 */

#include <main.h>

/*
 * format of messages from Downconverter:
 * "OLD xx yy text\n"
 * OLD ... header
 * xx .... x position if OLED display
 * yy .... y  position if OLED display
 * text .. text written into OLED display
 *
 * all messages without the "OLD " header are debug messages and must be ignored
 */



uint8_t down_rxbuf[DOWN_MAXRXLEN+1]; // one extra byte for string terminator
uint8_t down_rxidx = 0;
uint8_t down_dataready = 0;

// called inside USART-3 interrupt routine
void receive_downconverter_data(uint8_t rxdatabyte)
{
static uint8_t idx = 0;

	if(down_dataready == 1)
		return;	// old data not processed

	switch(idx)
	{
	case 0: if(rxdatabyte == 'O')
			{
				idx++;
			}
			break;
	case 1: if(rxdatabyte == 'L')
			{
				idx++;
			}
			else
				idx = 0;
			break;
	case 2: if(rxdatabyte == 'D')
				idx++;
			else
				idx = 0;
			break;
	case 3:
			if(rxdatabyte == ' ')
			{
				// header found, read all data until '\n'
				down_rxidx = 0;
				idx++;
			}
			else
				idx = 0;
			break;

	case 4:	// read all data until '\n'
			if(down_rxidx >= DOWN_MAXRXLEN)
			{
				// too long, ignore
				idx = 0;
				break;
			}

			down_rxbuf[down_rxidx++] = rxdatabyte;

			if(rxdatabyte == '\n')
			{
				// finished
				down_rxbuf[down_rxidx] = 0;	// string terminator
				down_dataready = 1;
				idx = 0;
				break;
			}
			break;
	}
}

/*
 * get data from the messages from the down converter
 * each string starts with "xx yy " which is the OLED display position
 *
 * list of messages (not complete)
 * 00 01 13:57:58 (current time)
 * 56 05 4 (number of GPS satellites) or
 * 48 05 12 (number of GPS satellites)
 * 80 05 lock (GPS locked)
 * 80 05 unlock (GPS unlocked)
 * 72 01 JN68NT (own QTH locator)
 * 48 06 48d49.30N (latitude)
 * 40 07 013d07.72E (longitude)
 */

#define DOWN_STRLEN (DOWN_MAXRXLEN+1)

char act_time[DOWN_STRLEN];
char gps_lockstatus[DOWN_STRLEN];
char num_gpssats[DOWN_STRLEN];
char qthloc[DOWN_STRLEN];
char latit[DOWN_STRLEN];
char longit[DOWN_STRLEN];

void eval_downconverter_data()
{
	while(1)
	{
		int16_t d = getdata3();
		if(d == -1) return;	// no more data

		receive_downconverter_data((uint8_t)d);

		if(down_dataready)
		{
			down_dataready = 0;
			break;	// continue evaluating the received string
		}
	}

	char *rxdata = (char *)down_rxbuf;

	// send to PC
	deb_printf("OLD ");
	deb_printf(rxdata,strlen(rxdata));

	if(strlen(rxdata) > 6)
	{
		toggle();
		if(!memcmp(rxdata,"00 01 ", 6))
		{
			memcpy(act_time, rxdata+6, DOWN_STRLEN);
			act_time[DOWN_STRLEN-1] = 0;
		}
		if(!memcmp(rxdata,"56 05 ", 6))
		{
			memcpy(num_gpssats, rxdata+6, DOWN_STRLEN);
			num_gpssats[DOWN_STRLEN-1] = 0;
		}
		if(!memcmp(rxdata,"48 05 ", 6))
		{
			memcpy(num_gpssats, rxdata+6, DOWN_STRLEN);
			num_gpssats[DOWN_STRLEN-1] = 0;
		}
		if(!memcmp(rxdata,"80 05 ", 6))
		{
			memcpy(gps_lockstatus, rxdata+6, DOWN_STRLEN);
			gps_lockstatus[DOWN_STRLEN-1] = 0;
		}
		if(!memcmp(rxdata,"72 01 ", 6))
		{
			memcpy(qthloc, rxdata+6, DOWN_STRLEN);
			qthloc[DOWN_STRLEN-1] = 0;
		}
		if(!memcmp(rxdata,"48 06 ", 6))
		{
			memcpy(latit, rxdata+6, DOWN_STRLEN);
			latit[DOWN_STRLEN-1] = 0;
		}
		if(!memcmp(rxdata,"40 07 ", 6))
		{
			memcpy(longit, rxdata+6, DOWN_STRLEN);
			longit[DOWN_STRLEN-1] = 0;
		}
	}
}
