#ifndef __ADC_H
#define __ADC_H	
#include "stm32f4xx.h"  
 							   
void Adc_Init(void); 				//ADC通道初始化
u16  Get_charge_value(u8 ch); 				//获得某个通道值 
u16 Get_charge(u8 ch);

#endif 















