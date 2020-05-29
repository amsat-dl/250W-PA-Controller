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
 * adc.c
 *
 *  Created on: 20.11.2019
 *      Author: kurt
 */

#include <main.h>

/*
 * this works ONLY with ADC3 and not with ADC1, reason unknown
 *
 * two ADC inputs:
 * PC1 .... TMP (measure NTC on PA board) ... ADC123_IN11
 * PC3 .... VIN12 (measure 12V supply) ... ADC123_IN13
 *
 * we use ADC3, which is mapped to DMA2 - Channel 2 - Stream 1
 */

int16_t cf_calc_temp(uint32_t uin);

#define ADC_DR_OFFSET	0x4c
#define st  ADC_SampleTime_480Cycles

#define ADCMITTELANZ	60
#define ADCCHANNELS		2

#define VREF 3300	// internally connected to Vcc

volatile uint16_t ADC3ConvertedValue[ADCMITTELANZ * ADCCHANNELS];

volatile int adccnv_ready = 0;

void init_adc()
{
	ADC_InitTypeDef       ADC_InitStructure;
	ADC_CommonInitTypeDef ADC_CommonInitStructure;
	DMA_InitTypeDef       DMA_InitStructure;

	// Enable ADC3, DMA2 and GPIO clocks
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2 | RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);

	// DMA2 Stream1 channel0 configuration
	DMA_InitStructure.DMA_Channel = DMA_Channel_2;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)(ADC3_BASE + ADC_DR_OFFSET);
	DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)ADC3ConvertedValue;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralToMemory;
	DMA_InitStructure.DMA_BufferSize = ADCMITTELANZ * ADCCHANNELS;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
	DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA2_Stream1, &DMA_InitStructure);
	DMA_Cmd(DMA2_Stream1, ENABLE);

	NVIC_InitTypeDef   NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = DMA2_Stream1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	DMA_ITConfig(DMA2_Stream1, DMA_IT_TC, ENABLE);

	// ADC Common Init
	ADC_CommonInitStructure.ADC_Mode = ADC_Mode_Independent;
	ADC_CommonInitStructure.ADC_Prescaler = ADC_Prescaler_Div8;
	ADC_CommonInitStructure.ADC_DMAAccessMode = ADC_DMAAccessMode_Disabled;
	ADC_CommonInitStructure.ADC_TwoSamplingDelay = ADC_TwoSamplingDelay_5Cycles;
	ADC_CommonInit(&ADC_CommonInitStructure);

	// ADC3 Init
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Left;
	ADC_InitStructure.ADC_NbrOfConversion = 2;
	ADC_Init(ADC3, &ADC_InitStructure);

	// ADC3 regular channel0 configuration
	ADC_RegularChannelConfig(ADC3, ADC_Channel_11, 1, ADC_SampleTime_3Cycles);
	ADC_RegularChannelConfig(ADC3, ADC_Channel_13, 2, ADC_SampleTime_3Cycles);

	// Enable DMA request after last transfer (Single-ADC mode) */
	ADC_DMARequestAfterLastTransferCmd(ADC3, ENABLE);

	// Enable ADC3 DMA
	ADC_DMACmd(ADC3, ENABLE);

	// Enable ADC3
	ADC_Cmd(ADC3, ENABLE);

	ADC_SoftwareStartConv(ADC3);   // A/D convert start

}

void DMA2_Stream1_IRQHandler()
{
	if(DMA_GetITStatus(DMA2_Stream1,DMA_IT_TCIF1) == SET)
	{
		adccnv_ready = 1;
		DMA_ClearITPendingBit(DMA2_Stream1,DMA_IT_TCIF1);
	}
}

int isAdcConvReady()
{
	if(adccnv_ready == 1)
	{
		adccnv_ready = 0;
		return 1;
	}

	return 0;
}

/*
 * the DMA makes ADCMITTELANZ conversions per ADC-input and stores
 * them in ADC3ConvertedValue
 * this functions makes the average value of all measurements
 * channel: 0=temp, 1=supply voltage
 */
uint32_t ui16_Read_ADC3_ConvertedValue(int channel)
{
	uint32_t v=0;
	static uint32_t lastv[ADCCHANNELS];

	for(uint32_t i=0; i<ADCMITTELANZ; i++)
		v += (ADC3ConvertedValue[i * ADCCHANNELS +channel] >> 4);

	v /= ADCMITTELANZ;

	// clean digit 0 jittering
	if(v == (lastv[channel]-1) || v == (lastv[channel]+1))
	{
		// if digit 0 jitters, then take the last measurement
		v = lastv[channel];
	}
	else
		lastv[channel] = v;

	return (int)((v*VREF)/4096);      // Read and return conversion result in mV
}


int16_t patemperature = 0;
uint16_t v12 = 0;

#define Rupper 15000
#define Rlower 1000
#define Vdiode 4	// voltage drop on V12 protection diode in 1/10v

void read_adc()
{
	uint32_t v;

	if(isAdcConvReady())
	{
		v = ui16_Read_ADC3_ConvertedValue(0);	// temp
		patemperature = cf_calc_temp(v);
		//deb_printf("PA temp: %d\n",patemperature);

		v = ui16_Read_ADC3_ConvertedValue(1);	// Vsupply
		v *= (Rupper + Rlower);
		v /= Rlower;
		v12 = (v / 100);	// volt * 10
		v12 += Vdiode;
		//deb_printf("Supply (x10): %d\n",v12);
	}
}

// calculate the temperature of the PA boad in deg.C
// v = voltage at the ADC input in mV
// returns: temp in deg.C.

#define RV		2200   // NTC to GND and RV to +3v3
#define VREF	3300

/* Umess (ADC Input) = UV *R[ntc] / (RV + R[ntc])

NTC resistance:
R[ntc] = Umess * RV / (UV - Umess)
*/

// R table for NTC on Ampleon board ERTJ1VT152J
float temptab[] = {
	5949 , //    Ohms at 0 Grad usw...
	4430 , //    5
	3332 , //    10
	2530 , //    15
	1940 , //    20
	1500 , //    25
	1170  , //   30
	920  , //    35
	728  , //    40
	581  , //    45
	467  , //    50
	377  , //    55
	306  , //    60
	251  , //    65
	207  , //    70
	172  , //    75
	143  , //    80
	121  , //    85
	102   , //   90
	86   , //    95
	74   , //    100
	63   , //    105
	54   , //    110
	-1
};

// calculate the NTC temperature
// uin: voltage at ADC input in mV
// returns: deg.C
int16_t cf_calc_temp(uint32_t uin)
{
	float Rntc;
	int i;
	float x;
	float Umess = (float)uin/1000.0;

	// Umess ist die Spannung am ADC Eingang
	// jetzt berechne daraus den Widerstand des NTCs
	Rntc = Umess * (float)RV / ((float)VREF/1000.0 - Umess);

	if(Rntc >= temptab[0])
	{
		// lower than minimum value
		return 0;
	}

	// suche den Bereich in der Tabelle
	i=0;
	while(temptab[i]!=-1)
	{
		if(temptab[i] <= Rntc) break;
		i++;
	}

	if(i==0)
	{
		return 0; // kleiner als kleinster Wert
	}

	if(temptab[i]!=-1)
	{
		// Widerstandsbereich gefunden, interpoliere
		x = i - (Rntc - temptab[i])/(temptab[i-1] - temptab[i]);

		// x ist jetzt der interpolierte Tabellenindex
		// rechne ihn in die Temperatur um
		return  (int16_t)(x*5.0);
	}

	return 115; // größer als größter Wert
}
