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
 * gpio.c
 *
 *  Created on: 19.11.2019
 *      Author: kurt
 */

#include <main.h>


/*
 * GPIO Ports
 * ==========
 *
 * PA0 .... UART ... TxD4 (J2)
 * PA1 .... UART ... RxD4 (J2)
 * PA2 .... UART ... TxD2 (J3)
 * PA3 .... UART ... RxD2 (J3)
 * PA4 .... Out GPIO  ... TX=0 to UpConv-1
 * PA5 .... Out GPIO  ... TX=0 to UpConv-2
 * PA6 .... TIM13_CH1 ... PWM (fan speed control)
 * PA7 .... (not used)
 * PA8 .... SCL0 (not used)
 * PA9 .... USBVB (not used) evt. als Flash Eingang
 * PA10 ... USBID (not used)
 * PA11 ... USBD-N (not used)
 * PA12 ... USBD-P (not used)
 * PA13 ... Driver-PA PTT TX=0 RX=1
 * PA14 ... GPIO OUT ... OnAir LED (JTCK) 0=AN 1=AUS
 * PA15 ... GPIO IN ... TAST (JTDI) 0=pressed function=Display Einschalten für x Minuten
 *
 * PB0 .... INT I2C connector (not used)
 * PB1 .... GPIO OUT ... DC (Display SPI Data/Command)
 * PB2 .... BOOT1 (47k to GND)
 * PB3 .... PTT-WB (JTDO) Input, vom Benutzer gedrückt, TX=0
 * PB4 .... (not used)
 * PB5 .... PTT-NB Input, vom Benutzer gedrückt, TX=0
 * PB6 .... TxD1-STM (RS-232 to J7)
 * PB7 .... RxD1-STM (RS-232 to J7)
 * PB8 .... GPIO IN ... TACHO-F (tacho signal from fan)
 * PB9 .... GPIO IN ... TACHO-P (tacho signal from pump, if available)
 * PB10 ... GPIO OUT ... RES (Display Reset Line)
 * PB11 ... (not used)
 * PB12 ... CS2 SPI CS (Display)
 * PB13 ... SCK2 SPI SCK (Display)
 * PB14 ... MISO2 SPI (Display)
 * PB15 ... MOSI2 SPI (Display)
 *
 * PC0 .... (not used)
 * PC1 .... ADC ... TMP (measure NTC on PA board)
 * PC2 .... GPIO OUT ... PTTPA (PTT line to PA) 1=TX RX=0
 * PC3 .... ADC ... VIN12 (measure 12V supply)
 * PC4 .... (not used)
 * PC5 .... (not used)
 * PC6 .... (not used)
 * PC7 .... (not used)
 * PC8 .... (not used)
 * PC9 .... SDA0 I2C (not used)
 * PC10 ... (not used)
 * PC11 ... UART ... RxD3 Daten vom Downconverter
 * PC12 ... (unused)
 * PC13 ... GPIO OUT default=1 ... Status LED (1=off 0=on)
 * PC14 ... GPIO OUT default=0 ... P-ON switch fan/pump on/off 1=on
 * PC15 ... (not used)
 *
 * PD2 .... GPIO OUT ... 1WRT ext. 1-wire temp sensor DS18S20
 *
 * Connectors:
 * ===========
 *
 * Connector J2:
 * PTT1 & Serial-4
 *
 * Connector J3:
 * PTT2 & Serial-2
 *
 * ST2:
 * Temperature from PA-Module
 * TMP ... analog voltage
 *
 */

void init_gpio()
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, ENABLE);

	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;

	// ========== GPIO A ==========
	// GPIO A outputs
	GPIO_InitStructure.GPIO_Pin=PTT1_OUT_PA5 | DRIVER_PTT_OUT_PA13 | ONAIR_LED_OUT_PA14 | PWM_FANSPEED_OUT_PA6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// Default output values
	// PTT to Upconverter high = RX
	GPIO_WriteBit(GPIOA,PTT2_OUT_PA4,1);
	GPIO_WriteBit(GPIOA,PTT1_OUT_PA5,1);
	GPIO_WriteBit(GPIOA,DRIVER_PTT_OUT_PA13,1);
	// OnAir LED off
	GPIO_WriteBit(GPIOA,ONAIR_LED_OUT_PA14,1);

	// GPIO A inputs
	GPIO_InitStructure.GPIO_Pin=TAST_IN_PA15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// GPIO A alternative function: PWM,UART2,UART4
	GPIO_InitStructure.GPIO_Pin=PWM_FANSPEED_OUT_PA6 | TXD4_OUT_PA0 | TXD2_OUT_PA2 | RXD4_IN_PA1 | RXD2_IN_PA3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource6, GPIO_AF_TIM13);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_UART4);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_UART4);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);

	// ========== GPIO B ==========
	// GPIO B outputs
	GPIO_InitStructure.GPIO_Pin=TXD1STM_OUT_PB6 | DISPLAY_DC_OUT_PB1 | DISPLAY_RES_OUT_PB10 | DISPLAY_CS_OUT_PB12 | DISPLAY_SCK_OUT_PB13 | DISPLAY_MOSI_OUT_PB15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// GPIO B inputs (with Pullup)
	GPIO_InitStructure.GPIO_Pin=TACHO_FAN_IN_PB8 | TACHO_PUMP_IN_PB9;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// no pullup
	GPIO_InitStructure.GPIO_Pin=PTTWB_IN_PB3 | PTTNB_IN_PB5 | RXD1STM_IN_PB7;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	// GPIO B alternative function, SPI for Display,
	GPIO_InitStructure.GPIO_Pin=TXD1STM_OUT_PB6 | DISPLAY_SCK_OUT_PB13 | DISPLAY_MISO_IN_PB14 | DISPLAY_MOSI_OUT_PB15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOB, GPIO_PinSource6, GPIO_AF_USART1);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);
	GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);

	// set SPI CS high
	GPIO_WriteBit(GPIOB,DISPLAY_CS_OUT_PB12,1);

	// ========== GPIO C ==========
	// GPIO C outputs
	GPIO_InitStructure.GPIO_Pin=PTTPA_OUT_PC2 | STATUS_LED_OUT_PC13 | PON_OUT_PC14;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// Default output values
	// PTT to PA low = RX
	GPIO_WriteBit(GPIOC,PTTPA_OUT_PC2,0);
	// fan/pump off
	GPIO_WriteBit(GPIOC,PON_OUT_PC14,0);

	// GPIO C inputs
	GPIO_InitStructure.GPIO_Pin=RXD3_IN_PC11 | WRR_IN_PC12;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// GPIO C alternative function, UART3 for Downconverter-RX (no TX)
	GPIO_InitStructure.GPIO_Pin=RXD3_IN_PC11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_USART3);

	// GPIOC analog inputs
	GPIO_InitStructure.GPIO_Pin=TMP_ADC_IN_PC1 | VIN12_ADC_IN_PC3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	// ========== GPIO D ==========
	// GPIO D outputs as open drain output for ext Sensor DHT11 (corresponding input is WRR PC12)
	GPIO_InitStructure.GPIO_Pin=WRT_OUT_PD2;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_WriteBit(GPIOD,WRT_OUT_PD2,1);
}

void blink(int speed)
{
	while(1)
	{
		GPIO_WriteBit(GPIOC,STATUS_LED_OUT_PC13,0);
		delay_1ms(speed);
		GPIO_WriteBit(GPIOC,STATUS_LED_OUT_PC13,1);
		delay_1ms(speed);
	}
}

void toggle()
{
static uint8_t state = 0;

	GPIO_WriteBit(GPIOC,STATUS_LED_OUT_PC13,state);
	state = 1-state;
}

// ====== read status of GPIO port into a variable ======

uint8_t PTT_WB = 0;
uint8_t PTT_NB = 0;

void read_user_PTTs()
{
	if(GPIO_ReadInputDataBit(GPIOB, PTTNB_IN_PB5) == 0)
		PTT_NB = 1;
	else
		PTT_NB = 0;

	if(GPIO_ReadInputDataBit(GPIOB, PTTWB_IN_PB3) == 0)
		PTT_WB = 1;
	else
		PTT_WB = 0;
}

uint8_t tast=0;

void read_TAST()
{
	if(GPIO_ReadInputDataBit(GPIOA, TAST_IN_PA15) == 0)
		tast = 1;
	else
		tast = 0;
}


// ===== set GPIO port ======

uint8_t PTT_PA = 0;

// NB
void switch_PTT1(uint8_t onoff)
{
	GPIO_WriteBit(GPIOA,PTT1_OUT_PA5,1-onoff);
}

void switch_PTTPA(uint8_t onoff)
{
	PTT_PA = onoff;
	GPIO_WriteBit(GPIOC,PTTPA_OUT_PC2,onoff);
}

// WB
void switch_PTT2(uint8_t onoff)
{
	GPIO_WriteBit(GPIOA,PTT2_OUT_PA4,1-onoff);
}

void switch_onair(uint8_t onoff)
{
	GPIO_WriteBit(GPIOA,ONAIR_LED_OUT_PA14,1-onoff);
}

// Driver
void switch_PTTDRIVER(uint8_t onoff)
{
	GPIO_WriteBit(GPIOA,DRIVER_PTT_OUT_PA13,1-onoff);
}
