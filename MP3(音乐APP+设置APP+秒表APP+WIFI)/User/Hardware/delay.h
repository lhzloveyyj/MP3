/**
  ******************************************************************************
  * @file    delay.h
  * @author  ZL
  * @version V0.0.1
  * @date    January-31-2017
  * @brief   Drivers for systick delay.
  ******************************************************************************
//�޸�˵��
//1,delay_us,��Ӳ�������0�ж�,�����������0,��ֱ���˳�. 
//2,�޸�ucosii��,delay_ms����,����OSLockNesting���ж�,�ڽ����жϺ�,Ҳ����׼ȷ��ʱ.
//V1.2 20150411  
//�޸�OS֧�ַ�ʽ,��֧������OS(������UCOSII��UCOSIII,����������OS������֧��)
//���:delay_osrunning/delay_ostickspersec/delay_osintnesting�����궨��
//���:delay_osschedlock/delay_osschedunlock/delay_ostimedly��������
//V1.3 20150521
//����UCOSIII֧��ʱ��2��bug��
//delay_tickspersec��Ϊ��delay_ostickspersec
//delay_intnesting��Ϊ��delay_osintnesting
////////////////////////////////////////////////////////////////////////////////// 
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DELAY_H
#define __DELAY_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"	  

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/	 
/* Exported variable ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/
void delay_init(u8 SYSCLK);
void delay_ms(u16 nms);
void delay_us(u32 nus);

#ifdef __cplusplus
}
#endif

#endif /* __DELAY_H */
/********************************************* *****END OF FILE****/
