/*

The MIT License (MIT)

Copyright (c) 2015 Tinotenda Chemvura, John Odetokun, Othniel Konan, Herman Kouassi

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

 */

/*
 * AudioStuff.h
 *
 *  Created on: Sep 27, 2016
 *      Author: Othniel Konan
 *     Version: Jan 10, 2017
 * Description: Contains set of functions to generate sound
 */

#ifndef AUDIOSTUFF_H_
#define AUDIOSTUFF_H_

#include "Utils080.h"

#define AUDIO_FREQUENCY 11000
#define DMA_FREQUENCY  (86000000/(2*AUDIO_FREQUENCY))

void AudioDisable(void);
void AudioInit(uint16_t *, uint16_t,uint16_t);
void Timer_configuration(uint16_t);
void TIM2_IRQHandler(void);
void TempoSetValue(uint16_t);
void TempoDisable(void);
void AudioPlay(uint16_t * , uint16_t, uint16_t);
void AudioFreestyle(uint16_t *);
void AudioComposerPlayback(uint16_t);

/*
 * @brief Disable the DMA
 */
void AudioDisable(){
	DMA_ClearFlag(DMA1_Stream5,DMA_FLAG_TCIF5);
	DMA_Cmd(DMA1_Stream5,DISABLE);
}


/*
 * @brief	Perform audio initialization
 * @param	DACBuffer		:	Array to be pushed to the DMA
 * @param	Mode			: 	DMA Mode (default:DMA_Mode_Normal)
 * @param	SIze			: 	sample size (default:SAMPLE_SIZE)
 * @return 	None
 */
void AudioInit(uint16_t *DACBuffer, uint16_t Mode, uint16_t Size){

	// RCC configuration
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1 | RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC | RCC_APB1Periph_TIM6, ENABLE);

	// GPIO configuration [GPIOA_4]
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4;
	GPIO_Init(GPIOA, &GPIO_InitStruct);

	// TIMER configuration [TIM6]
	TIM_TimeBaseInitTypeDef	TIM_TimeBaseStruct;
	TIM_TimeBaseStruct.TIM_Period = DMA_FREQUENCY;
	TIM_TimeBaseStruct.TIM_Prescaler = 1;
	TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStruct.TIM_RepetitionCounter = 0x0000;
	TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStruct);
	TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);
	TIM_Cmd(TIM6, ENABLE);

	// DMA configuration	[Channel_7, Stream_5]
	DMA_InitTypeDef DMA_InitStruct;
	DMA_StructInit(&DMA_InitStruct);
	DMA_InitStruct.DMA_Channel=DMA_Channel_7;
	DMA_InitStruct.DMA_PeripheralBaseAddr=(uint32_t)(DAC_BASE + 0x08);  //DAC channel1 12-bit right-aligned data holding register (ref manual pg. 264)
	DMA_InitStruct.DMA_Memory0BaseAddr=(uint32_t)DACBuffer;
	DMA_InitStruct.DMA_DIR=DMA_DIR_MemoryToPeripheral;
	DMA_InitStruct.DMA_BufferSize = Size;
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStruct.DMA_Mode = Mode;
	DMA_InitStruct.DMA_Priority = DMA_Priority_High;
	DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
	DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;
	DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	DMA_Init(DMA1_Stream5, &DMA_InitStruct);
	DMA_Cmd(DMA1_Stream5, ENABLE);

	// DAC configuration	[Channel_1,TIM6]
	DAC_InitTypeDef DAC_InitStruct;
	DAC_InitStruct.DAC_Trigger = DAC_Trigger_T6_TRGO;
	DAC_InitStruct.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_InitStruct.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;
	DAC_InitStruct.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_Init(DAC_Channel_1, &DAC_InitStruct);
	DAC_DMACmd(DAC_Channel_1,ENABLE);
	DAC_Cmd(DAC_Channel_1, ENABLE);
}


/*
 * @brief	Play a sample
 * @param	DACBuffer		:	Array to be pushed to the DMA
 * @param	Mode			: 	DMA Mode (default:DMA_Mode_Normal)
 * @param	Size			: 	sample size (default:SAMPLE_SIZE)
 * @return 	None
 */
void AudioPlay(uint16_t *DACBuffer, uint16_t Mode, uint16_t Size ){
	AudioDisable();
	AudioInit(DACBuffer,Mode,Size);
}

/*
 * @brief	Play sample in freestyle mode (play one sample once)
 * @param	DACBuffer		:	Array to be pushed to the DMA
 * @return 	None
 */
void AudioFreestyle(uint16_t *DACBuffer){
	AudioPlay(DACBuffer, DMA_Mode_Normal, SAMPLE_SIZE);
}


/*
 * @brief	Play each sample from a 16 array of sample at the given
 * 			interval of time
 * @param	tempo		: 	Tempo of the sound (default=60bpm)
 * @return 	None
 */
void AudioComposerPlayback(uint16_t tempo){
	// Set Timer
	Timer_configuration(tempo);
}

/*
 * @brief	Tempo Timer Configuration
 */
void Timer_configuration(uint16_t tempo){

	// Enable TIM2
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2 ,ENABLE);
	// Setup the timer
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStruct;
	TIM_TimeBaseStruct.TIM_Prescaler = 10000;
	TIM_TimeBaseStruct.TIM_Period = (uint32_t) 126000/tempo + 1;
	TIM_TimeBaseStruct.TIM_ClockDivision = 0;
	TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStruct);
	TIM_Cmd(TIM2, ENABLE);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

	// Enable the TIM2 global Interrupt
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

/*
 * @brief	TIM2 Handler
 **/
void TIM2_IRQHandler(void)
{
	static uint8_t index = 0;
	static uint16_t sample_size = DEFAULT_COMPOSER_BUFFERSIZE/16;
	static bool boo;
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

		boo = GPIO_ReadOutputDataBit(getGPIO(index,0).GPIO,getGPIO(index,0).pin);
		updateLED(index,boo,0);
		AudioPlay(&ComposerBuffer[index*sample_size],DMA_Mode_Normal,SAMPLE_SIZE);
		updateLED(index,!boo,0);

		// Update index
		if(index>=15)index=0;
		else index++;
	}
	uint16_t temp = Tempo_Convert();
	float gradient = (140.0-40.0)/(4096.0);
	uint16_t _tempo = (uint16_t) (gradient*(float)temp+40);
	TempoSetValue(_tempo);
//	check if tempo has changed: then refresh LED
//	added by Hermann
	if (_tempo != tempo) {
		UPDATE_TEMPO = true;
		tempo = _tempo;
	}
}

/*
 * @brief Set the tempo
 */
void TempoSetValue(uint16_t tempo){
	// Compute ARR based on tempo
	float arr = 126000/tempo + 1;
	TIM2->ARR = (uint32_t) arr;
}

/*
 * @brief Disable tempo
 * @important	TO BE USED WHEN MOVING TO FREESTYLE MODE
 */
void TempoDisable(){
	TIM_Cmd(TIM2,DISABLE);
}

#endif /* AUDIOSTUFF_H_ */
