/*
 * gpio.h
 *
 *  Created on: 19.11.2019
 *      Author: kurt
 */

#ifndef GPIO_H_
#define GPIO_H_

/*
 * PA0 .... UART ... TxD4 (J2)
 * PA1 .... UART ... RxD4 (J2)
 * PA2 .... UART ... TxD2 (J3)
 * PA3 .... UART ... RxD2 (J3)
 * PA4 .... Out GPIO  ... TX=0
 * PA5 .... Out GPIO  ... TX=0
 * PA6 .... GPIO OUT ... PWM (fan speed control)
 * PA7 .... (not used)
 * PA8 .... SCL0 (not used)
 * PA9 .... USBVB (not used) evt. als Flash Eingang
 * PA10 ... USBID (not used)
 * PA11 ... USBD-N (not used)
 * PA12 ... USBD-P (not used)
 * PA13 ... (not used) (JTMS)
 * PA14 ... GPIO OUT ... OnAir LED (JTCK) 0=AN 1=AUS
 * PA15 ... GPIO IN ... TAST (JTDI) 0=pressed function=Display Einschalten für x Minuten

 */
#define TXD4_OUT_PA0			GPIO_Pin_0
#define RXD4_IN_PA1				GPIO_Pin_1
#define TXD2_OUT_PA2			GPIO_Pin_2
#define RXD2_IN_PA3				GPIO_Pin_3
#define PTT2_OUT_PA4			GPIO_Pin_4
#define PTT1_OUT_PA5			GPIO_Pin_5
#define PWM_FANSPEED_OUT_PA6	GPIO_Pin_6
#define DRIVER_PTT_OUT_PA13		GPIO_Pin_13
#define ONAIR_LED_OUT_PA14		GPIO_Pin_14
#define TAST_IN_PA15			GPIO_Pin_15

#define DISPLAY_DC_OUT_PB1 		GPIO_Pin_1
#define PTTWB_IN_PB3			GPIO_Pin_3
#define PTTNB_IN_PB5			GPIO_Pin_5
#define TXD1STM_OUT_PB6			GPIO_Pin_6
#define RXD1STM_IN_PB7			GPIO_Pin_7
#define TACHO_FAN_IN_PB8 		GPIO_Pin_8
#define TACHO_PUMP_IN_PB9 		GPIO_Pin_9
#define DISPLAY_RES_OUT_PB10 	GPIO_Pin_10
#define DISPLAY_CS_OUT_PB12		GPIO_Pin_12
#define DISPLAY_SCK_OUT_PB13	GPIO_Pin_13
#define DISPLAY_MISO_IN_PB14	GPIO_Pin_14
#define DISPLAY_MOSI_OUT_PB15	GPIO_Pin_15

#define TMP_ADC_IN_PC1			GPIO_Pin_1
#define PTTPA_OUT_PC2			GPIO_Pin_2
#define VIN12_ADC_IN_PC3		GPIO_Pin_3
#define RXD3_IN_PC11			GPIO_Pin_11
#define WRR_IN_PC12				GPIO_Pin_12
#define STATUS_LED_OUT_PC13		GPIO_Pin_13
#define PON_OUT_PC14 			GPIO_Pin_14

#define WRT_OUT_PD2				GPIO_Pin_2


void init_gpio();
void read_user_PTTs();
void read_TAST();
void switch_PTT1(uint8_t onoff);
void switch_PTTPA(uint8_t onoff);
void switch_PTT2(uint8_t onoff);
void switch_PTTDRIVER(uint8_t onoff);
void switch_onair(uint8_t onoff);
void blink(int speed);
void toggle();


extern uint8_t PTT_WB;
extern uint8_t PTT_NB;
extern uint8_t tast;
extern uint8_t PTT_PA;

#endif /* GPIO_H_ */
