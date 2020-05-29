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
 * fan_pump.c
 *
 *  Created on: 19.11.2019
 *      Author: kurt
 */

#include <main.h>

/* check if the FAN is running
 *
 * this functions checks every 10ms if the level of the TACHO signal changes
 * if it does not change within 10 seconds, than it is not running
*/

#define FAN_CHECKTIME	1000	// in 10ms, 1000 = 10.000ms = 10s

uint16_t fan_running = FAN_CHECKTIME;
uint16_t pump_running = FAN_CHECKTIME;

#define FAN_MINSPEED 	20 	// in % depends how slow the fan can start up
#define FAN_MAXSPEED	100 // in %
#define TEMP_MIN		30	// degC
#define TEMP_MAX		50	// degC

uint8_t fan_speed = 0; 	// in % FAN_MINSPEED..100

uint8_t fan_pump_alarm = 0;

// 0=off
// 1=switch on
// 2=switch off with temp check
uint8_t fan_pump_onoff = 0;
uint8_t old_fan_pump_onoff = 0;

uint8_t old_patemperature = 0;

// called every main loop
void fan_pump_running()
{
	// ====== detect if fan / pump is running ======
	static uint8_t oldfan = 0;
	static uint8_t oldpump = 0;

	if(fan_pump_onoff != 0 && fan_10ms_TO == 1)
	{
		fan_10ms_TO = 0;
		// check level of tacho pins every 10ms
		// check fan
		uint16_t fan = GPIO_ReadInputDataBit(GPIOB,TACHO_FAN_IN_PB8);
		if(fan != oldfan)
		{
			oldfan = fan;
			fan_running = FAN_CHECKTIME;
		}
		else
		{
			// if no pulse, count down to 0
			// it will be 0 after 10s
			if(fan_running) fan_running--;
			if(!fan_running) fan_pump_alarm = 1;
		}

		uint16_t pump = GPIO_ReadInputDataBit(GPIOB,TACHO_PUMP_IN_PB9);
		if(pump != oldpump)
		{
			oldpump = pump;
			pump_running = FAN_CHECKTIME;
		}
		else
		{
			// if no pulse, count down to 0
			// it will be 0 after 10s
			if(pump_running) pump_running--;
			if(!pump_running) fan_pump_alarm = 1;
		}
	}

	if(fan_running == 0 && pump_running != 0)
	{
		print_string("!!!  FAN ALARM", 0, 7, BRIGHT, 1);
	}
	else if(pump_running == 0 && fan_running != 0)
	{
		print_string("!!! PUMP ALARM", 0, 7, BRIGHT, 1);
	}
	else if(pump_running == 0 && fan_running == 0)
	{
		print_string("FAN/PUMP ALARM", 0, 7, BRIGHT, 1);
	}
}

void restartFanPumpTimer()
{
	fan_running = FAN_CHECKTIME;
	pump_running = FAN_CHECKTIME;
}

// ON ... switch on immediately, start fan with lowest speed
// OFF .. switch off if < 30 deg.C.

void switch_fan_pump(uint8_t onoff)
{
	if(onoff == 0)
		fan_pump_onoff = 2;
	else
	{
		fan_pump_onoff = 1;
		fan_speed = FAN_MINSPEED;
		setPWM();
	}
}

// set fan speed according PA temperature
void fan_speed_control()
{
	// ====== handle switch on/off of fan and pump =====
	if(fan_pump_onoff != old_fan_pump_onoff)
	{
		// fan/pump status changed
		if(fan_pump_onoff == 1)
		{
			// switch ON immediately
			GPIO_WriteBit(GPIOC,PON_OUT_PC14,1);
		}
		if(fan_pump_onoff == 0)
		{
			// switch OFF immediately
			GPIO_WriteBit(GPIOC,PON_OUT_PC14,0);
		}
	}
	old_fan_pump_onoff = fan_pump_onoff;

	if(fan_pump_onoff == 2)
	{
		// switch OFF if temp < 30 deg.C
		if(patemperature <= 30)
			fan_pump_onoff = 0;
	}

	// ===== set fan speed =====
	if(fan_pump_onoff == 1)
	{
		// change speed every 5 deg difference
		// 40deg.C = 25%, 60deg.C = 100%
		if((patemperature) != (old_patemperature))
		{
			uint16_t temp = patemperature;
			if(temp < TEMP_MIN) temp = TEMP_MIN;
			if(temp > TEMP_MAX) temp = TEMP_MAX;
			uint16_t speed = ((temp-TEMP_MIN)*(FAN_MAXSPEED-FAN_MINSPEED))/(TEMP_MAX-TEMP_MIN) + FAN_MINSPEED;
			fan_speed = speed;

			setPWM();
			old_patemperature = patemperature;

		}
	}
}

