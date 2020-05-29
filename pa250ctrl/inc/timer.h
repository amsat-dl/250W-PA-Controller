/*
 * timer.h
 *
 *  Created on: 19.11.2019
 *      Author: kurt
 */

#ifndef TIMER_H_
#define TIMER_H_

void init_timer();
void delay_1ms(uint32_t del);
void delay_1us(uint32_t del);
uint16_t calcPWM();
void setPWM();
uint32_t micros();

extern volatile uint8_t fan_10ms_TO;
extern uint8_t fan_10s_TO;
extern volatile uint8_t second1;
extern volatile uint8_t display_off;
extern volatile uint32_t microticks;
extern volatile uint32_t display_dimval;
extern volatile uint32_t display_offtimer;

#endif /* TIMER_H_ */
