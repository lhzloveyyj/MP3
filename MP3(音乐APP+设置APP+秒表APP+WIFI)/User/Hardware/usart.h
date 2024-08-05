/**
  ******************************************************************************
  * @file    usart.h
  * @author  ZL
  * @version V0.0.1
  * @date    February-1-2017
  * @brief   Drivers for usart.
  ******************************************************************************
  */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USART_H
#define __USART_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "stdio.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define USART_REC_LEN  			200  	//定义最大接收字节数 200

#define USART_PORT1    1
#define USART_PORT6    6

/* Exported macro ------------------------------------------------------------*/
/* Exported variable ---------------------------------------------------------*/
extern u8  USART_RX_BUF[USART_REC_LEN]; //接收缓冲,最大USART_REC_LEN个字节.末字节为换行符 
extern u16 USART_RX_STA;         		//接收状态标记	
extern u8 usart_port;

/* Exported functions --------------------------------------------------------*/
void usart1_Init(u32 baud);
void usart6_Init(u32 baud);

#ifdef __cplusplus
}
#endif

#endif /* __USART_H */
/********************************************* *****END OF FILE****/
