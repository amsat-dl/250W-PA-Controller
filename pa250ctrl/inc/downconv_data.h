/*
 * downconv_data.h
 *
 *  Created on: 21.11.2019
 *      Author: kurt
 */

#ifndef DOWNCONV_DATA_H_
#define DOWNCONV_DATA_H_

#define DOWN_MAXRXLEN 150

void receive_downconverter_data(uint8_t rxdatabyte);
void eval_downconverter_data();

extern char act_time[];
extern char gps_lockstatus[];
extern char num_gpssats[];
extern char qthloc[];
extern char latit[];
extern char longit[];

#endif /* DOWNCONV_DATA_H_ */
