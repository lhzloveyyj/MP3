#include "key.h"
#include "pwm.h" 

#include "FreeRTOS.h"
#include "task.h"

//������ʼ������
void KEY_Init(void)
{
	
	GPIO_InitTypeDef  GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA|RCC_AHB1Periph_GPIOA, ENABLE);//ʹ��GPIOA,GPIOEʱ��
 
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_3|GPIO_Pin_6; //KEY0 KEY1 KEY2��Ӧ����
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;//��ͨ����ģʽ
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;//100M
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;//����
  GPIO_Init(GPIOA, &GPIO_InitStructure);//��ʼ��GPIOE2,3,4
 
} 

u8 KEY_Scan(void)
{	  
	if(KEY0 == 0)
	{
		beep_key(80);
		vTaskDelay(30);
		beep_key(0);
		if(KEY0 == 0)
		return 1;
	}
	else if(KEY1 == 0)
	{
		beep_key(80);
		vTaskDelay(30);
		beep_key(0);
		if(KEY1 == 0)
		return 2;
	}
	else if(KEY2 == 0)
	{
		beep_key(80);
		vTaskDelay(40);
		beep_key(0);
		if(KEY2 == 0)
		return 3;
	}
 	return 0;// �ް�������
}




















