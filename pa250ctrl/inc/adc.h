/*
 * adc.h
 *
 *  Created on: 20.11.2019
 *      Author: kurt
 */

#ifndef ADC_H_
#define ADC_H_

void init_adc();
void read_adc();

extern int16_t patemperature;
extern uint16_t v12;

#endif /* ADC_H_ */
