/*
 * uart.h
 *
 *  Created on: 21.11.2019
 *      Author: kurt
 */

#ifndef UART_H_
#define UART_H_

void init_uart();
void send_WB(uint8_t *data, uint16_t len);
void send_NB(uint8_t *data, uint16_t len);
void deb_printf(char *fmt, ...);
void usart2_rxirq(uint8_t onoff);
void usart3_rxirq(uint8_t onoff);
void uart4_rxirq(uint8_t onoff);
int16_t getdata3();


#endif /* UART_H_ */
