/**
  ******************************************************************************
  * @file    dac.h
  * @author  ZL
  * @version V0.0.1
  * @date    September-20-2019
  * @brief   dac configuration.
  ******************************************************************************
*/
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DAC_H
#define __DAC_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
	 
/* Exported constants --------------------------------------------------------*/
#define   DAC_BUF_LEN       2500
	 
/* Exported types ------------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variable ---------------------------------------------------------*/
extern uint16_t DAC_buff[2][DAC_BUF_LEN]; //DAC1¡¢DAC2Êä³ö»º³å
	 
/* Exported functions --------------------------------------------------------*/
void DAC_Config(void);
void DAC_Out1(uint16_t dat);
void DAC_Out2(uint16_t dat);
void DAC_DMA_Start(uint32_t freq, uint16_t len);
void DAC_DMA_Stop(void);

#ifdef __cplusplus
}
#endif
#endif /*__DAC_H */

/********************************************* *****END OF FILE****/
