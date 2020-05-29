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
 * timer.c
 *
 *  Created on: 19.11.2019
 *      Author: kurt
 */
/*
 * usage of timers:
 *
 * TIM3:  1ms timer for general purpose use in this software (i.e. delay)
 * TIM7:  1-wire timer (us delay)
 * TIM13: PWM generation for fan
 */
#include <main.h>

void init_timer()
{
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	// ====== TIM3_Initialization: 1ms timer for general usage in this software ======
	// Enable the TIM3 gloabal Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// TIM3 clock enable
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

	// APB1: 90 MHz
	TIM_TimeBaseStructure.TIM_Period = 90;			// divide it to 1MHz = 1us
	TIM_TimeBaseStructure.TIM_Prescaler = 1000;		// and down to 1ms
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	// Interrupt enable
	TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

	// TIM3 enable
	TIM_Cmd(TIM3, ENABLE);

	// ====== TIM7_Initialization: 1us timer for general usage in this software ======
	// Enable the TIM7 gloabal Interrupt
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	// TIM7 clock enable
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);

	// APB1: 90 MHz
	TIM_TimeBaseStructure.TIM_Period = 9;			// divide it to 10MHz = 0.1us
	TIM_TimeBaseStructure.TIM_Prescaler = 10;		// and down to 1us
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);

	// Interrupt enable
	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);

	// TIM7 enable
	TIM_Cmd(TIM7, ENABLE);

	// ====== TIM13: PWM generation for the fan ======
	// Enable clock for TIM13
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM13, ENABLE);
	// APB1 Timer Clock = 90 MHz (2x APB1 Clock) (no prescaler)
	TIM_TimeBaseStructure.TIM_Prescaler = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	// TIM_Period = timer_tick_frequency / PWM_frequency - 1
	// TIM_Period = 90000000 / 25000 - 1 = 3599 (max. allowed is 65535)
	TIM_TimeBaseStructure.TIM_Period = 3599; // 25kHz PWM
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TIM13, &TIM_TimeBaseStructure);
	TIM_Cmd(TIM13, ENABLE);

    setPWM();

    // the output PA6 is already configured in init_gpio() as alternate function
}

// calculate the timer value for the requested fan speed
uint16_t calcPWM()
{
	// requested speed is in "fan_speed" in %
	// pulse_length = ((TIM_Period + 1) * fan_speed) / 100 - 1 = 2099
	return ((3599+1)*(uint16_t)fan_speed)/100-1;
}

void setPWM()
{
	// PWM generation values
	TIM_OCInitTypeDef TIM_OCStruct;
	// PWM mode 2 = Clear on compare match
	// PWM mode 1 = Set on compare match
	TIM_OCStruct.TIM_OCMode = TIM_OCMode_PWM2;
	TIM_OCStruct.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
	TIM_OCStruct.TIM_Pulse = calcPWM();
	TIM_OC1Init(TIM13, &TIM_OCStruct);
	TIM_OC1PreloadConfig(TIM13, TIM_OCPreload_Enable);
}

volatile uint8_t fan_10ms_TO = 0;
volatile uint16_t del_1s = 0;
volatile uint8_t second1 = 0;
volatile uint32_t delay = 0;
volatile uint32_t delay_us = 0;
volatile uint8_t display_off = 0;
volatile uint32_t display_dimval = 0xff;
volatile uint32_t display_offtimer = 0;

#define DISPLAY_ON_TIME  (1000 * 60 * 5)	// ms
#define DISPLAY_DIMTIME	 5000	// ms

// Handle TIM3 interrupt, every Millisecond
void TIM3_IRQHandler(void)
{
static uint8_t fan_10ms = 0;

	if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
	{
		if(++fan_10ms >= 10)
		{
			// 10ms timer for fan/pump tacho detector
			fan_10ms = 0;
			fan_10ms_TO = 1;
		}

		if(++del_1s >= 1000)
		{
			del_1s = 0;
			second1 = 1;
		}

		if(display_offtimer < DISPLAY_ON_TIME)
		{
			display_offtimer++;

			if(display_offtimer >= (DISPLAY_ON_TIME-DISPLAY_DIMTIME))
			{
				display_dimval = (DISPLAY_ON_TIME - display_offtimer) * 0xff / DISPLAY_DIMTIME;
				display_off = 2;	// start dimming in the last second
			}

			if(display_offtimer == DISPLAY_ON_TIME)
			{
				display_off = 1;	// total dimmed
			}
		}

		delay++;

		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
}

volatile uint32_t microticks = 0;

// Handle TIM7 interrupt, every Microsecond
void TIM7_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM7, TIM_IT_Update) == SET)
	{
		delay_us++;
		microticks++;

		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
	}
}

uint32_t micros()
{
	return microticks;
}

void delay_1ms(uint32_t del)
{
	delay = 0;
	while(delay < del);
}

void delay_1us(uint32_t del)
{
	delay_us = 0;
	while(delay_us < del);
}
