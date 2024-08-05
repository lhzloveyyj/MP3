/**
  ******************************************************************************
  * @file    dac.c
  * @author  ZL
  * @version V0.0.1
  * @date    September-20-2019
  * @brief   DAC configuration.
  ******************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "dac.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define   CNT_FREQ          84000000      // TIM6 counter clock (prescaled APB1)

/* DHR registers offsets */
#define DHR12R1_OFFSET             ((uint32_t)0x00000008)
#define DHR12R2_OFFSET             ((uint32_t)0x00000014)
#define DHR12RD_OFFSET             ((uint32_t)0x00000020)

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t DAC_DHR12R1_ADDR = (uint32_t)DAC_BASE + DHR12R1_OFFSET + DAC_Align_12b_L;
uint32_t DAC_DHR12R2_ADDR = (uint32_t)DAC_BASE + DHR12R2_OFFSET + DAC_Align_12b_L;

uint16_t DAC_buff[2][DAC_BUF_LEN]; //DAC1��DAC2�������

/* Private function prototypes -----------------------------------------------*/
static void TIM6_Config(void);

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  DAC��ʼ��
  * @param  none
  * @retval none
*/
void DAC_Config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	DAC_InitTypeDef  DAC_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
		
	DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
	DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
	DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
	DAC_Init(DAC_Channel_1, &DAC_InitStructure);
	DAC_Init(DAC_Channel_2, &DAC_InitStructure);
	
	//����DMA
	DMA_InitTypeDef DMA_InitStruct;
	DMA_StructInit(&DMA_InitStruct);
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1, ENABLE);
	
	DMA_InitStruct.DMA_PeripheralBaseAddr = (u32)DAC_DHR12R1_ADDR;
	DMA_InitStruct.DMA_Memory0BaseAddr = (u32)&DAC_buff[0];//DAC1
	DMA_InitStruct.DMA_DIR = DMA_DIR_MemoryToPeripheral;
	DMA_InitStruct.DMA_BufferSize = DAC_BUF_LEN;
	DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStruct.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStruct.DMA_Priority = DMA_Priority_High;
	DMA_InitStruct.DMA_Channel = DMA_Channel_7;
	DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable;
	DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_HalfFull;
  DMA_InitStruct.DMA_MemoryBurst   = DMA_MemoryBurst_Single;
  DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;
	
	DMA_Init(DMA1_Stream5, &DMA_InitStruct);
		
	DMA_InitStruct.DMA_PeripheralBaseAddr = (u32)DAC_DHR12R2_ADDR;
	DMA_InitStruct.DMA_Memory0BaseAddr = (u32)&DAC_buff[1];//DAC2
	DMA_Init(DMA1_Stream6, &DMA_InitStruct);
		
	//����DMA��������ж�
	NVIC_InitTypeDef NVIC_InitStructure;
	
  NVIC_InitStructure.NVIC_IRQChannel = DMA1_Stream6_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
	
	DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TCIF6);
	DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_HTIF6);
	DMA_ITConfig(DMA1_Stream6, DMA_IT_TC, ENABLE);
	DMA_ITConfig(DMA1_Stream6, DMA_IT_HT, ENABLE);

//	DMA_Cmd(DMA1_Stream5, ENABLE);
//	DMA_Cmd(DMA1_Stream6, ENABLE);
	DAC_Cmd(DAC_Channel_1, ENABLE);
  DAC_Cmd(DAC_Channel_2, ENABLE);
	
	DAC_DMACmd(DAC_Channel_1, ENABLE);
	DAC_DMACmd(DAC_Channel_2, ENABLE);
	
	TIM6_Config();
}

//����DAC�����ʺ�DMA���ݳ��ȣ�������DMA DAC
void DAC_DMA_Start(uint32_t freq, uint16_t len)
{
	//����DMA���峤����ҪֹͣDMA
	DAC_DMA_Stop();
	//����DMA DAC���峤��
	DMA_SetCurrDataCounter(DMA1_Stream5, len);
	DMA_SetCurrDataCounter(DMA1_Stream6, len);
	
	//���ö�ʱ��
	TIM_SetAutoreload(TIM6, (uint16_t)((CNT_FREQ)/freq));
	
	//����
	DMA_Cmd(DMA1_Stream5, ENABLE);
	DMA_Cmd(DMA1_Stream6, ENABLE);
}

//ֹͣDMA DAC
void DAC_DMA_Stop(void)
{
	DMA_Cmd(DMA1_Stream5, DISABLE);
	DMA_Cmd(DMA1_Stream6, DISABLE);
}

//��ʱ��6��������DACˢ����
static void TIM6_Config(void)
{
  TIM_TimeBaseInitTypeDef TIM6_TimeBase;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
  TIM_TimeBaseStructInit(&TIM6_TimeBase); 
	
  TIM6_TimeBase.TIM_Period        = (uint16_t)((CNT_FREQ)/44100);
  TIM6_TimeBase.TIM_Prescaler     = 0;
  TIM6_TimeBase.TIM_ClockDivision = 0;
  TIM6_TimeBase.TIM_CounterMode   = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM6, &TIM6_TimeBase);
	
  TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);
  TIM_Cmd(TIM6, ENABLE);
}

/**
  * @brief  DAC out1 PA4�����ѹ
  * @param  dat��dac��ֵ:��0~4095
  * @retval none
*/
void DAC_Out1(uint16_t dat)
{
	DAC_SetChannel1Data(DAC_Align_12b_R,  dat);
	DAC_SoftwareTriggerCmd(DAC_Channel_1, ENABLE);
}

/**
  * @brief  DAC out2 PA5�����ѹ
  * @param  dat��dac��ֵ:��0~4095
  * @retval none
*/
void DAC_Out2(uint16_t dat)
{
	DAC_SetChannel2Data(DAC_Align_12b_R,  dat);
	DAC_SoftwareTriggerCmd(DAC_Channel_2, ENABLE);
}

/********************************************* *****END OF FILE****/
