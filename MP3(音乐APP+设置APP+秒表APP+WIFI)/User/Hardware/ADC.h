#ifndef __ADC_H
#define __ADC_H	
#include "stm32f4xx.h"  
 							   
void Adc_Init(void); 				//ADCͨ����ʼ��
u16  Get_charge_value(u8 ch); 				//���ĳ��ͨ��ֵ 
u16 Get_charge(u8 ch);

#endif 















