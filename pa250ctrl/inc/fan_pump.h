/*
 * fan_pump.h
 *
 *  Created on: 19.11.2019
 *      Author: kurt
 */

#ifndef FAN_PUMP_H_
#define FAN_PUMP_H_

void fan_pump_running();
void switch_fan_pump(uint8_t onoff);
void fan_speed_control();
void restartFanPumpTimer();

extern uint16_t fan_running;
extern uint16_t pump_running;
extern uint8_t fan_speed;
extern uint8_t fan_pump_alarm;

#endif /* FAN_PUMP_H_ */
