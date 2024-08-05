#ifndef __KEY_H
#define __KEY_H	 
#include "stm32f4xx.h"             

/*����ķ�ʽ��ͨ��ֱ�Ӳ����⺯����ʽ��ȡIO*/
#define KEY1 		GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) //PE4
#define KEY2 		GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3)	//PE3 
#define KEY0 		GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6) //PE2

void KEY_Init(void);	//IO��ʼ��
u8 KEY_Scan(void);  		//����ɨ�躯��	

#endif
