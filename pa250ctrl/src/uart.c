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
 * uart.c
 *
 *  Created on: 21.11.2019
 *      Author: kurt
 */

#include <main.h>

/*
 * UART usage on the PA250 controller board:
 *
 * UART-3: Downconverter (only RxD is connected)
 * UART-4: NB-Upconverter
 * UART-2: WB-Upconverter
 * UART-1: to PC, (only TX used for debug messages)
 * (UART-6: to GPS connector, currently not used)
 */

void init_USART1_DMA_TX(int len);
void init_USART2_DMA_TX(int len);
void init_UART4_DMA_TX(int len);

#define UARTBUFLEN	200

uint8_t usart1_tx_buffer[UARTBUFLEN];
uint8_t usart2_tx_buffer[UARTBUFLEN];
uint8_t uart4_tx_buffer[UARTBUFLEN];

uint8_t uart1rdy = 1;

void init_uart()
{
	// the alternative GPIO functions are initialized in init_gpio()

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, ENABLE);

	// init UART-2 and 4 (WB Upconverter)
	USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_BaudRate = 115200;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStruct.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStruct.USART_Parity = USART_Parity_No;
	USART_InitStruct.USART_StopBits = USART_StopBits_1;
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;

	//USART_Init(USART2,&USART_InitStruct);
	//USART_Init(UART4,&USART_InitStruct);

	// init UART-3 (rx only from Downconverter)
	USART_InitStruct.USART_BaudRate = 9600;
	USART_InitStruct.USART_Mode = USART_Mode_Rx;
	USART_Init(USART3,&USART_InitStruct);

	// init UART-1 (tx for debugging)
	USART_InitStruct.USART_BaudRate = 115200;
	USART_InitStruct.USART_Mode = USART_Mode_Tx;
	USART_Init(USART1,&USART_InitStruct);

	// enable RX IRQ for all UARTs
	NVIC_InitTypeDef   NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	//NVIC_Init(&NVIC_InitStructure);
	//USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);

	NVIC_InitStructure.NVIC_IRQChannel = UART4_IRQn;
	//NVIC_Init(&NVIC_InitStructure);
	//USART_ITConfig(UART4, USART_IT_RXNE, ENABLE);

	// init DMA TX transfer for USART1, USART2 and UART4
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);
	init_USART1_DMA_TX(UARTBUFLEN);
	//init_USART2_DMA_TX(UARTBUFLEN);
	//init_UART4_DMA_TX(UARTBUFLEN);

	//NVIC_InitTypeDef   NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);

	// enable UARTs
	USART_Cmd(USART1,ENABLE);
	//USART_Cmd(USART2,ENABLE);
	USART_Cmd(USART3,ENABLE);
	//USART_Cmd(UART4,ENABLE);
}

void DMA2_Stream7_IRQHandler()
{
	if(DMA_GetITStatus(DMA2_Stream7,DMA_IT_TCIF7) == SET)
	{
		uart1rdy = 1;
		DMA_ClearITPendingBit(DMA2_Stream7,DMA_IT_TCIF7);
	}
}

void usart2_rxirq(uint8_t onoff)
{
	USART_ITConfig(USART2, USART_IT_RXNE, onoff);
}

void usart3_rxirq(uint8_t onoff)
{
	USART_ITConfig(USART3, USART_IT_RXNE, onoff);
}

void uart4_rxirq(uint8_t onoff)
{
	USART_ITConfig(UART4, USART_IT_RXNE, onoff);
}

void init_USART1_DMA_TX(int len)
{
	DMA_InitTypeDef DMA_InitStructure;
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART1->DR;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)usart1_tx_buffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = len;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream7, &DMA_InitStructure);
}

void init_USART2_DMA_TX(int len)
{
	DMA_InitTypeDef DMA_InitStructure;
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&USART2->DR;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)usart2_tx_buffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = len;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream6, &DMA_InitStructure);
}

void init_UART4_DMA_TX(int len)
{
	DMA_InitTypeDef DMA_InitStructure;
	DMA_InitStructure.DMA_Channel = DMA_Channel_4;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&UART4->DR;
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)uart4_tx_buffer;
	DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStructure.DMA_BufferSize = len;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream4, &DMA_InitStructure);
}

// RX Interrupt routines

void USART2_IRQHandler(void)
{
	if ((USART2->SR & USART_FLAG_RXNE) != (u16)RESET)
	{
		uint8_t rxdatabyte = USART_ReceiveData(USART2) & 0xff;
		eval_Upconverter_data(0,rxdatabyte);

		USART_ClearFlag(USART2, USART_FLAG_RXNE);
	}

	if ((USART2->SR & USART_FLAG_ORE) != (u16)RESET)
	{
		// Receiver overrun
		// führt beim Debuggen zu ununterbrochenem Auslösen des IRQs
		USART_ClearFlag(USART2, USART_FLAG_ORE);
	}
}

#define RX_BUFFER_SIZE3 256
uint8_t rx_buffer3[RX_BUFFER_SIZE3];
volatile uint16_t rx_wr_index3=0, rx_rd_index3=0;

void USART3_IRQHandler(void)
{
	if ((USART3->SR & USART_FLAG_RXNE) != (u16)RESET)
	{
		uint8_t rxdatabyte = USART_ReceiveData(USART3) & 0xff;
		//receive_downconverter_data(rxdatabyte);

		// write data to RX buffer
		rx_buffer3[rx_wr_index3++] = rxdatabyte;
		if (rx_wr_index3 >= RX_BUFFER_SIZE3)
			rx_wr_index3 = 0;

		USART_ClearFlag(USART3, USART_FLAG_RXNE);
	}

	if ((USART3->SR & USART_FLAG_ORE) != (u16)RESET)
	{
		// Receiver overrun
		// führt beim Debuggen zu ununterbrochenem Auslösen des IRQs
		USART_ClearFlag(USART3, USART_FLAG_ORE);
	}
}

// read from RX buffer, unblocked
// -1 ... no data
int16_t getdata3()
{
int16_t data = -1;

	// data available ?
	usart3_rxirq(0);
	if(rx_wr_index3 != rx_rd_index3)
	{
		data = rx_buffer3[rx_rd_index3++];
		if (rx_rd_index3 == RX_BUFFER_SIZE3) rx_rd_index3 = 0;
	}
	usart3_rxirq(1);

	return data;
}

void UART4_IRQHandler(void)
{
	if ((UART4->SR & USART_FLAG_RXNE) != (u16)RESET)
	{
		uint8_t rxdatabyte = USART_ReceiveData(UART4) & 0xff;
		eval_Upconverter_data(1,rxdatabyte);

		USART_ClearFlag(UART4, USART_FLAG_RXNE);
	}

	if ((UART4->SR & USART_FLAG_ORE) != (u16)RESET)
	{
		// Receiver overrun
		// führt beim Debuggen zu ununterbrochenem Auslösen des IRQs
		USART_ClearFlag(UART4, USART_FLAG_ORE);
	}
}

// TX via DMA

void send_WB(uint8_t *data, uint16_t len)
{
	if(!DMA_GetCmdStatus(DMA1_Stream6) && len <= UARTBUFLEN)
	{
		memcpy(usart2_tx_buffer, data, len);

		init_USART2_DMA_TX(len);
		DMA_ITConfig(DMA1_Stream6, DMA_IT_TC, ENABLE);
		DMA_Cmd(DMA1_Stream6, ENABLE);
		USART_DMACmd(USART2, USART_DMAReq_Tx, ENABLE);
	}
}

void send_NB(uint8_t *data, uint16_t len)
{
	if(!DMA_GetCmdStatus(DMA1_Stream4) && len <= UARTBUFLEN)
	{
		memcpy(uart4_tx_buffer, data, len);

		init_UART4_DMA_TX(len);
		DMA_ITConfig(DMA1_Stream4, DMA_IT_TC, ENABLE);
		DMA_Cmd(DMA1_Stream4, ENABLE);
		USART_DMACmd(UART4, USART_DMAReq_Tx, ENABLE);
	}
}

void send_PC(char *data, uint16_t len)
{
	int to = 0;
	while(uart1rdy == 0)
	{
		delay_1us(100);
		if(++to >= 10000) break; // wait max. 1s
	}

	uart1rdy = 0;
	memcpy(usart1_tx_buffer, data, len);

	init_USART1_DMA_TX(len);
	DMA_ITConfig(DMA2_Stream7, DMA_IT_TC, ENABLE);
	DMA_Cmd(DMA2_Stream7, ENABLE);
	USART_DMACmd(USART1, USART_DMAReq_Tx, ENABLE);
}

void deb_printf(char *fmt, ...)
{
	char s[200];

	va_list ap;
	va_start(ap, fmt);
	vsprintf(s,fmt,ap);
	va_end(ap);

	int len = strlen(s);
	if(s[len-1] == '\n')
	{
		s[len] = '\r';
		s[len+1] = 0;
	}

	send_PC(s, strlen(s));
}
