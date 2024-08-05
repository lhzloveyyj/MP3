#ifndef __KEY_H
#define __KEY_H	 
#include "stm32f4xx.h"             

/*下面的方式是通过直接操作库函数方式读取IO*/
#define KEY1 		GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0) //PE4
#define KEY2 		GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_3)	//PE3 
#define KEY0 		GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_6) //PE2

void KEY_Init(void);	//IO初始化
u8 KEY_Scan(void);  		//按键扫描函数	

#endif
