#include "pwm.h"
 
//TIM14 PWM���ֳ�ʼ�� 
//PWM�����ʼ��
//arr���Զ���װֵ
//psc��ʱ��Ԥ��Ƶ��
void TIM2_PWM_Init(u32 arr,u32 psc)
{		 					 
	//�˲������ֶ��޸�IO������
	
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);  	//TIM14ʱ��ʹ��    
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE); 	//ʹ��PORTFʱ��	
	
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_TIM2); //GPIOF9����Ϊ��ʱ��14
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource1,GPIO_AF_TIM2);
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_1;           //GPIOF9
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;        //���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//�ٶ�100MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;      //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;        //����
	GPIO_Init(GPIOA,&GPIO_InitStructure);              //��ʼ��PF9
	  
	TIM_TimeBaseStructure.TIM_Prescaler=psc;  //��ʱ����Ƶ
	TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up; //���ϼ���ģʽ
	TIM_TimeBaseStructure.TIM_Period=arr;   //�Զ���װ��ֵ
	TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1; 
	
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseStructure);//��ʼ����ʱ��14
	
	//��ʼ��TIM14 Channel1 PWMģʽ	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; //ѡ��ʱ��ģʽ:TIM�����ȵ���ģʽ2
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; //�Ƚ����ʹ��
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; //�������:TIM����Ƚϼ��Ե�
	TIM_OC3Init(TIM2, &TIM_OCInitStructure);  //����Tָ���Ĳ�����ʼ������TIM1 4OC1
	TIM_OC3PreloadConfig(TIM2, TIM_OCPreload_Enable);  //ʹ��TIM14��CCR1�ϵ�Ԥװ�ؼĴ���
	
	TIM_OC2Init(TIM2, &TIM_OCInitStructure);  //����Tָ���Ĳ�����ʼ������TIM1 4OC1
	TIM_OC2PreloadConfig(TIM2, TIM_OCPreload_Enable);  //ʹ��TIM14��CCR1�ϵ�Ԥװ�ؼĴ���
	
  TIM_ARRPreloadConfig(TIM2,ENABLE);//ARPEʹ�� 
	
	TIM_Cmd(TIM2, ENABLE);  //ʹ��TIM14
 
										  
} 

void PWM_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    TIM_OCInitTypeDef TIM_OCInitStructure;
    
    RCC_AHB1PeriphClockCmd (RCC_AHB1Periph_GPIOA, ENABLE); 
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);

    GPIO_PinAFConfig(GPIOA,GPIO_PinSource11,GPIO_AF_TIM1); 

    
    GPIO_InitStructure.GPIO_Pin =GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;    
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOA,&GPIO_InitStructure);
    
    TIM_TimeBaseStructure.TIM_ClockDivision=TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode=TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_Period=100-1; 
    TIM_TimeBaseStructure.TIM_Prescaler=1680-1;
    TIM_TimeBaseStructure.TIM_RepetitionCounter=0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);
      
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_OutputNState=TIM_OutputNState_Disable;
    TIM_OCInitStructure.TIM_OCIdleState=TIM_OCIdleState_Set;
    TIM_OCInitStructure.TIM_Pulse=0;

    TIM_OC4Init(TIM1,&TIM_OCInitStructure);
     
    TIM_OC4PreloadConfig(TIM1,TIM_OCPreload_Enable);
      
    TIM_ARRPreloadConfig(TIM1,ENABLE);
    TIM_Cmd(TIM1,ENABLE);
    TIM_CtrlPWMOutputs(TIM1,ENABLE);
}

void p_brightness(uint16_t Compare)
{
    TIM_SetCompare4(TIM1, Compare);//�ı�ARR��ֵ
}

void beep_key(uint16_t Compare)
{
	TIM_SetCompare3(TIM2,Compare);
}
void led_brightness(uint16_t Compare)
{
	TIM_SetCompare2(TIM2, Compare);
}



