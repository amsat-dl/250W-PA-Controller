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
 * display_highlevel.c
 *
 *  Created on: 22.11.2019
 *      Author: kurt
 */

#include <main.h>

/* formatting and printing to the display
 * ======================================
 *
 * the display is 256 x 64 pixels
 * our font needs 8x8 per character
 * so we can print 32 characters in 8 lines
 */

// fixed text
void display_greeting()
{
	print_string("AMSAT-DL PA-Controller", 0, 0, BRIGHT, 0);

	print_string("PA-Temp:", 0, 2, DARK, 0);
	print_string("CaseTmp:", 0, 3, DARK, 0);
	print_string("CaseHum:", 0, 4, DARK, 0);
	print_string("Voltage:", 0, 6, DARK, 0);

	print_string("NB TX ", 13, 2, DARK, 0);
	print_string("WB TX ", 13, 3, DARK, 0);
	print_string("PA TX ", 13, 4, DARK, 0);

	print_string("GPS:", 19, 2, DARK, 0);
	print_string("QTH:", 19, 3, DARK, 0);
	print_string("Lat:", 19, 4, DARK, 0);
	print_string("Lon:", 19, 5, DARK, 0);
	print_string("Sat:", 19, 6, DARK, 0);
}

void display()
{
	char s[40];
static char old_act_time[DOWN_MAXRXLEN] = {0};
static char old_gps_lockstatus[DOWN_MAXRXLEN] = {0};
static char old_num_gpssats[DOWN_MAXRXLEN] = {0};
static char old_qthloc[DOWN_MAXRXLEN] = {0};
static char old_latit[DOWN_MAXRXLEN] = {0};
static char old_longit[DOWN_MAXRXLEN] = {0};
static int16_t old_patemperature = -100;
static uint16_t old_v12 = 255;
static int8_t old_temp_ds18s02 = -100;
static uint8_t old_humidity = 255;
static uint8_t old_PTT_WB = 255;
static uint8_t old_PTT_NB = 255;
static uint8_t old_PTT_PA = 255;

	// display on/off
	if(tast)
	{
		if(display_off != 0)
			display_bright(0xff);
		display_off = 0;
		display_offtimer = 0;
	}
	if(display_off == 2)
	{
		display_bright(display_dimval);
	}
	if(display_off == 1)
	{
		display_bright(0x14);
	}

	if(old_PTT_NB != PTT_NB)
	{
		print_string("NB TX", 13, 2, PTT_NB?BRIGHT:DARK, PTT_NB);
	}
	if(old_PTT_WB != PTT_WB)
	{
		print_string("WB TX", 13, 3, PTT_WB?BRIGHT:DARK, PTT_WB);
	}
	if(old_PTT_PA != PTT_PA)
	{
		print_string("PA TX", 13, 4, PTT_PA?BRIGHT:DARK, PTT_PA);
	}
	if(memcmp(act_time,old_act_time,strlen(act_time)))
	{
		print_string(act_time, 24, 0, BRIGHT, 0);
	}
	if(memcmp(gps_lockstatus,old_gps_lockstatus,strlen(gps_lockstatus)))
	{
		print_string(gps_lockstatus, 24, 2, BRIGHT, 0);
	}
	if(memcmp(num_gpssats,old_num_gpssats,strlen(num_gpssats)))
	{
		print_string(num_gpssats, 24, 6, BRIGHT, 0);
	}
	if(memcmp(qthloc,old_qthloc,strlen(qthloc)))
	{
		print_string(qthloc, 24, 3, BRIGHT, 0);
	}
	if(memcmp(latit,old_latit,strlen(latit)))
	{
		print_string(latit, 24, 4, BRIGHT, 0);
	}
	if(memcmp(longit,old_longit,strlen(longit)))
	{
		print_string(longit, 23, 5, BRIGHT, 0);
	}
	if(old_patemperature != patemperature)
	{
		sprintf(s,"%3dC",patemperature);
		print_string(s, 8, 2, BRIGHT, 0);
	}
	if(old_v12 != v12)
	{
		sprintf(s,"%2.1fv",((float)v12)/10);
		print_string(s, 8, 6, BRIGHT, 0);
	}
	if(old_temp_ds18s02 != dht11_temp)
	{
		if(dht11_temp == -99)
			sprintf(s,"---C");
		else
			sprintf(s,"% 3dC",dht11_temp);
		print_string(s, 8, 3, BRIGHT, 0);
	}
	if(old_humidity != dht11_humidity)
	{
		if(dht11_humidity >= 250)
			sprintf(s,"---%%");
		else
		{
			if(dht11_humidity > 99) dht11_humidity = 99;
			sprintf(s,"% 2d%%",dht11_humidity);
		}
		print_string(s, 8, 4, BRIGHT, 0);
	}

	old_PTT_NB = PTT_NB;
	old_PTT_WB = PTT_WB;
	old_PTT_PA = PTT_PA;
	old_humidity = dht11_humidity;
	old_temp_ds18s02 = dht11_temp;
	old_v12 = v12;
	old_patemperature = patemperature;
	memcpy(old_act_time,act_time,DOWN_MAXRXLEN);
	memcpy(old_gps_lockstatus,gps_lockstatus,DOWN_MAXRXLEN);
	memcpy(old_num_gpssats,num_gpssats,DOWN_MAXRXLEN);
	memcpy(old_qthloc,qthloc,DOWN_MAXRXLEN);
	memcpy(old_latit,latit,DOWN_MAXRXLEN);
	memcpy(old_longit,longit,DOWN_MAXRXLEN);
}
