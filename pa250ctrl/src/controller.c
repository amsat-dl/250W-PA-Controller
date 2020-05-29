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
 * controller.c
 *
 *  Created on: 20.11.2019
 *      Author: kurt
 */

// functions of the PA controller

#include <main.h>

void switch_PTT();
void emergency_off();

uint8_t emergency = 0;

void controller()
{
	// ===== read all lines and sensors =====

	// check these inputs every second only
	if(second1)
	{
		second1 = 0;

		// read 1-wire sensors
		// result in temp_ds18s02: degC.
		// and humidity in %
		readDHT11();

		// read temperature from PA module and the supply voltage
		// result in patemperature in deg.C
		// result in v12 in volts*10
		read_adc();
	}

	// check if fan/pump is running
	// result in: fan_running and pump_running (0=not running, <>0=running)
	fan_pump_running();

	// check if a user PTT is active
	// result in PTT_NB and PTT_WB, 1=active(TX), 0=inactive(RX)
	read_user_PTTs();

	// check if the user has pushed the button
	// result in tast: 1=pushed, 0=not pushed
	read_TAST();

	// ===== handle normal operations =====
	switch_PTT();

	// ===== read information received via UARTs =====
	eval_downconverter_data();

	// ===== handle alarm conditions =====
	fan_speed_control();
	emergency_off();

	// ===== handle the display =====
	display();
}

/*
 * if the user goes to NB-TX, first activate PTTPA and after 10ms PTT1 (NB Upconverter)
 * if the user goes to WB-TX, first activate PTTPA and after 10ms PTT2 (NB Upconverter)
 * additionally if the user requests NB or WB-TX then switch ON the pump and
 * switch on the fan with lowest speed. Also activate the OnAir LED
 */
void switch_PTT()
{
static uint8_t pttnb_old = 0;
static uint8_t pttwb_old = 0;
uint8_t pttnb_active = 255, pttwb_active = 255;

	// in case of emergency ignore the PTT inputs and set them to inactive
	if(emergency)
	{
		PTT_NB = PTT_WB = 0;
		pttnb_old = pttwb_old = 1;
	}

	// test for change of a ptt input
	if(PTT_NB == 1 && pttnb_old == 0)
	{
		// just pressed
		pttnb_active = 1;
		restartFanPumpTimer();
	}

	if(PTT_NB == 0 && pttnb_old == 1)
		pttnb_active = 0;

	if(PTT_WB == 1 && pttwb_old == 0)
	{
		// just pressed
		pttwb_active = 1;
		restartFanPumpTimer();
	}

	if(PTT_WB == 0 && pttwb_old == 1)
		pttwb_active = 0;

	if(pttnb_active != 255 || pttwb_active != 255)
	{
		// one of the PTT inputs has been changed
		if(PTT_NB == 1)
		{
			// PTT NB was activated
			switch_PTTPA(1);
			delay_1ms(10);
			switch_PTT1(1);
		}
		else
		{
			switch_PTT1(0);
		}

		if(PTT_WB == 1)
		{
			// PTT WB was activated
			switch_PTTPA(1);
			delay_1ms(10);
			switch_PTT2(1);
			switch_PTTDRIVER(1);
		}
		else
		{
			switch_PTT2(0);
		}

		if(PTT_NB == 1 && PTT_WB == 0)
		{
			// only NB is active, switch off WB driver
			switch_PTTDRIVER(0);
		}

		if(PTT_NB == 1 || PTT_WB == 1)
		{
			// one of the two PTT inputs is active, we need to transmit
			// show OnAir and activate fan/pump
			switch_onair(1);
			switch_fan_pump(1);
		}
		else
		{
			// no PTT input is active, we can switch off the PA
			// OnAir off and switch off fan/pump depending on temperature
			switch_PTTDRIVER(0);
			switch_PTTPA(0);
			switch_onair(0);
			if(emergency == 0)
			{
				// no overheat condition, switch fun/pump off
				switch_fan_pump(0);
			}
		}
	}

	pttnb_old = PTT_NB;
	pttwb_old = PTT_WB;
}

// switch OFF transmission if PA temperature is > 90 deg.C.
// switch OFF is fan or pump is not running
#define PA_MAXTEMP	70	// deg.C

void emergency_off()
{
static uint8_t old_emergency = 0;

	if(patemperature >= PA_MAXTEMP)
	{
		emergency = 1;
		print_string("EMERGENCY OFF", 16, 7, BRIGHT, 1);
		print_string("!!! OVERHEAT", 0, 7, BRIGHT, 1);
	}
	else if(fan_pump_alarm == 1)
	{
		emergency = 1;
		print_string("EMERGENCY OFF", 16, 7, BRIGHT, 1);
	}
	else
	{
		if(old_emergency == 1)
		{
			// there was an overheat condition, but now it is cooled down
			// lets switch off the fan/pump if below min temp
			switch_fan_pump(0);
			print_string("                                ", 0, 7, BRIGHT, 0);
		}
		emergency = 0;
	}
	old_emergency = emergency;
}
