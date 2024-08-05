/**
  ******************************************************************************
  * @file    delay.c
  * @author  ZL
  * @version V0.0.1
  * @date    January-31-2017
  * @brief   Drivers for systick delay.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "delay.h"

#define SYSTEM_SUPPORT_OS		0		//����ϵͳ�ļ����Ƿ�֧��OS
#if SYSTEM_SUPPORT_OS
	#include "includes.h"					//ucos ʹ��	  
#endif
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static u8  fac_us=0;							//us��ʱ������			   
static u16 fac_ms=0;							//ms��ʱ������,��os��,����ÿ�����ĵ�ms��

/* Private define ------------------------------------------------------------*/
#if SYSTEM_SUPPORT_OS							//���SYSTEM_SUPPORT_OS������,˵��Ҫ֧��OS��(������UCOS).

#ifdef 	OS_CRITICAL_METHOD						//OS_CRITICAL_METHOD������,˵��Ҫ֧��UCOSII				
#define delay_osrunning		OSRunning			//OS�Ƿ����б��,0,������;1,������
#define delay_ostickspersec	OS_TICKS_PER_SEC	//OSʱ�ӽ���,��ÿ����ȴ���
#define delay_osintnesting 	OSIntNesting		//�ж�Ƕ�׼���,���ж�Ƕ�״���
#endif

//֧��UCOSIII
#ifdef 	CPU_CFG_CRITICAL_METHOD					//CPU_CFG_CRITICAL_METHOD������,˵��Ҫ֧��UCOSIII	
#define delay_osrunning		OSRunning			//OS�Ƿ����б��,0,������;1,������
#define delay_ostickspersec	OSCfg_TickRate_Hz	//OSʱ�ӽ���,��ÿ����ȴ���
#define delay_osintnesting 	OSIntNestingCtr		//�ж�Ƕ�׼���,���ж�Ƕ�״���
#endif

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  us����ʱʱ,�ر��������(��ֹ���us���ӳ�)
  * @param  None
  * @retval None
*/
void delay_osschedlock(void)
{
#ifdef CPU_CFG_CRITICAL_METHOD   			//ʹ��UCOSIII
	OS_ERR err; 
	OSSchedLock(&err);						//UCOSIII�ķ�ʽ,��ֹ���ȣ���ֹ���us��ʱ
#else										//����UCOSII
	OSSchedLock();							//UCOSII�ķ�ʽ,��ֹ���ȣ���ֹ���us��ʱ
#endif
}

/**
  * @brief  us����ʱʱ,�ָ��������
  * @param  None
  * @retval None
*/
void delay_osschedunlock(void)
{	
#ifdef CPU_CFG_CRITICAL_METHOD   			//ʹ��UCOSIII
	OS_ERR err; 
	OSSchedUnlock(&err);					//UCOSIII�ķ�ʽ,�ָ�����
#else										//����UCOSII
	OSSchedUnlock();						//UCOSII�ķ�ʽ,�ָ�����
#endif
}

/**
  * @brief  ����OS�Դ�����ʱ������ʱ
  * @param  ticks:��ʱ�Ľ�����
  * @retval None
*/
void delay_ostimedly(u32 ticks)
{
#ifdef CPU_CFG_CRITICAL_METHOD
	OS_ERR err; 
	OSTimeDly(ticks,OS_OPT_TIME_PERIODIC,&err);//UCOSIII��ʱ��������ģʽ
#else
	OSTimeDly(ticks);						//UCOSII��ʱ
#endif 
}
 
/**
  * @brief  systick�жϷ�����,ʹ��OSʱ�õ�
  * @param  None
  * @retval None
*/
void SysTick_Handler(void)
{	
	if(delay_osrunning==1)					//OS��ʼ����,��ִ�������ĵ��ȴ���
	{
		OSIntEnter();						//�����ж�
		OSTimeTick();       				//����ucos��ʱ�ӷ������               
		OSIntExit();       	 				//���������л����ж�
	}
}
#endif
			   
/**
  * @brief  ��ʼ���ӳٺ���
						��ʹ��ucos��ʱ��,�˺������ʼ��ucos��ʱ�ӽ���
						SYSTICK��ʱ�ӹ̶�ΪAHBʱ�ӵ�1/8
  * @param  SYSCLK:ϵͳʱ��Ƶ��
  * @retval None
*/
void delay_init(u8 SYSCLK)
{
#if SYSTEM_SUPPORT_OS 						//�����Ҫ֧��OS.
	u32 reload;
#endif
 	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);//SYSTICKʹ���ⲿʱ��Դ	 
	fac_us = SYSCLK/8;						//�����Ƿ�ʹ��OS,fac_us����Ҫʹ��
#if SYSTEM_SUPPORT_OS 						//�����Ҫ֧��OS.
	reload = SYSCLK/8;						//ÿ���ӵļ������� ��λΪK	   
	reload *= 1000000/delay_ostickspersec;	//����delay_ostickspersec�趨���ʱ��
											//reloadΪ24λ�Ĵ���,���ֵ:16777216,��72M��,Լ��1.86s����	
	fac_ms = 1000/delay_ostickspersec;		//����OS������ʱ�����ٵ�λ	   
	SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;//����SYSTICK�ж�
	SysTick->LOAD = reload; 					//ÿ1/OS_TICKS_PER_SEC���ж�һ��	
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; //����SYSTICK
#else
	fac_ms=(u16)fac_us*1000;				//��OS��,����ÿ��ms��Ҫ��systickʱ����   
#endif
}								    

#if SYSTEM_SUPPORT_OS 						//�����Ҫ֧��OS.   

/**
  * @brief  ��ʱnus
  * @param  nus:Ҫ��ʱ��us��.nus:0~204522252(���ֵ��2^32/fac_us@fac_us=21)	
  * @retval None
*/								   
void delay_us(u32 nus)
{		
	u32 ticks;
	u32 told, tnow, tcnt=0;
	u32 reload=SysTick->LOAD;				//LOAD��ֵ	    	 
	ticks = nus * fac_us; 						//��Ҫ�Ľ����� 
	delay_osschedlock();					//��ֹOS���ȣ���ֹ���us��ʱ
	told = SysTick->VAL;        				//�ս���ʱ�ļ�����ֵ
	while(1)
	{
		tnow = SysTick->VAL;	
		if(tnow != told)
		{	    
			if(tnow < told)
			{
				tcnt += told - tnow;	//����ע��һ��SYSTICK��һ���ݼ��ļ������Ϳ�����.
			}
			else
			{
				tcnt += reload-tnow+told;
			}
			told = tnow;
			if(tcnt>=ticks)
				break;			//ʱ�䳬��/����Ҫ�ӳٵ�ʱ��,���˳�.
		}  
	};
	delay_osschedunlock();					//�ָ�OS����											    
}  

/**
  * @brief  ��ʱnms
  * @param  nms:Ҫ��ʱ��ms����nms:0~65535
  * @retval None
*/
void delay_ms(u16 nms)
{	
	if(delay_osrunning&&delay_osintnesting == 0)//���OS�Ѿ�������,���Ҳ������ж�����(�ж����治���������)	    
	{		 
		if(nms >= fac_ms)						//��ʱ��ʱ�����OS������ʱ������ 
		{ 
   			delay_ostimedly(nms / fac_ms);	//OS��ʱ
		}
		nms %= fac_ms;						//OS�Ѿ��޷��ṩ��ôС����ʱ��,������ͨ��ʽ��ʱ    
	}
	delay_us((u32)(nms*1000));				//��ͨ��ʽ��ʱ
}

#else  //����ucosʱ
/**
  * @brief  ��ʱnus
  * @param  nusΪҪ��ʱ��us��.	ע��:nus��ֵ,��Ҫ����798915us(���ֵ��2^24/fac_us@fac_us=21)
  * @retval None
*/
void delay_us(u32 nus)
{		
	u32 temp;	    	 
	SysTick->LOAD = nus * fac_us; 				//ʱ�����	  		 
	SysTick->VAL = 0x00;        				//��ռ�����
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;//��ʼ����	 
	do
	{
		temp = SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));	//�ȴ�ʱ�䵽��   
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;//�رռ�����
	SysTick->VAL = 0X00;       				//��ռ����� 
}

/**
  * @brief  ��ʱnms
						�����ʱΪ:nms<=0xffffff*8*1000/SYSCLK
						SYSCLK��λΪHz,nms��λΪms
						��168M������,nms<=798ms 
  * @param  nmsΪҪ��ʱ��ms��.
  * @retval None
*/
void delay_xms(u16 nms)
{	 		  	  
	u32 temp;		   
	SysTick->LOAD =(u32)nms * fac_ms;			//ʱ�����(SysTick->LOADΪ24bit)
	SysTick->VAL = 0x00;           			//��ռ�����
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;//��ʼ���� 
	do
	{
		temp = SysTick->CTRL;
	}while((temp&0x01)&&!(temp&(1<<16)));	//�ȴ�ʱ�䵽��   
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;	//�رռ�����
	SysTick->VAL = 0X00;     		  		//��ռ�����	  	    
} 

/**
  * @brief  ��ʱnms
  * @param  nmsΪҪ��ʱ��ms��.nms:0~65535
  * @retval None
*/
void delay_ms(u16 nms)
{	 	 
	u8 repeat = nms / 540;						//������540,�ǿ��ǵ�ĳЩ�ͻ����ܳ�Ƶʹ��,
											//���糬Ƶ��248M��ʱ��,delay_xms���ֻ����ʱ541ms������
	u16 remain = nms % 540;
	while(repeat)
	{
		delay_xms(540);
		repeat--;
	}
	if(remain)
		delay_xms(remain);
} 
#endif
/********************************************* *****END OF FILE****/
