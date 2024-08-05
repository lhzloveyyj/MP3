#ifndef _PWM_H
#define _PWM_H
#include "stm32f4xx.h"  	

void TIM2_PWM_Init(u32 arr,u32 psc);
void PWM_Init(void);
void p_brightness(uint16_t Compare);
void beep_key(uint16_t Compare);
void led_brightness(uint16_t Compare);
#endif
