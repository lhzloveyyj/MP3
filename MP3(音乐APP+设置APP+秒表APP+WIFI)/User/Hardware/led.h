/**
  ******************************************************************************
  * @file    led.h
  * @author  ZL
  * @version V0.0.1
  * @date    January-31-2017
  * @brief   Drivers for LED.
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LED_H
#define __LED_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
	 
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define   LED0_OFF   GPIO_ResetBits(GPIOB,GPIO_Pin_0)
#define   LED0_ON    GPIO_SetBits(GPIOB,GPIO_Pin_0)
#define   LED0_TROG  GPIO_ToggleBits(GPIOB,GPIO_Pin_0)

#define   LED1_ON    GPIO_ResetBits(GPIOB,GPIO_Pin_1)
#define   LED1_OFF   GPIO_SetBits(GPIOB,GPIO_Pin_1)
#define   LED1_TROG  GPIO_ToggleBits(GPIOB,GPIO_Pin_1)

/* Exported variable ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void LED_Init(void);

#ifdef __cplusplus
}
#endif

#endif /* __LED_H */
/********************************************* *****END OF FILE****/
