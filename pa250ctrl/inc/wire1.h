/*
 * wire1.h
 *
 *  Created on: 23.11.2019
 *      Author: kurt
 */

#ifndef WIRE1_H_
#define WIRE1_H_

void wire1_init();
void read_wire1();

extern int8_t temp_ds18s02;
extern uint8_t humidity;


#endif /* WIRE1_H_ */
