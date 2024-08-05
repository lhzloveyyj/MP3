/**
  ******************************************************************************
  * @file    usart.h
  * @author  ZL
  * @version V0.0.1
  * @date    February-1-2017
  * @brief   Drivers for usart.
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"

#define SYSTEM_SUPPORT_OS		0		//����ϵͳ�ļ����Ƿ�֧��OS
#if SYSTEM_SUPPORT_OS
	#include "includes.h"					//ucos ʹ��	  
#endif

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u8 USART_RX_BUF[USART_REC_LEN];     //���ջ���,���USART_REC_LEN���ֽ�.
//����״̬
//bit15��	������ɱ�־
//bit14��	���յ�0x0d
//bit13~0��	���յ�����Ч�ֽ���Ŀ
u16 USART_RX_STA=0;       //����״̬���	
u8 usart_port = USART_PORT1;

/* Private function prototypes -----------------------------------------------*/
static void usart1_GPIOConfig(void);
static void usart6_GPIOConfig(void);

/* Private functions ---------------------------------------------------------*/

//////////////////////////////////////////////////////////////////
//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;

//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
}

//�ض���fputc���� 
int fputc(int ch, FILE *f)
{ 	
#if SYSTEM_SUPPORT_OS
	OSIntEnter();
#endif
	if(usart_port == USART_PORT1)
	{
		while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
		USART1->DR = (u8) ch;
	}else if(usart_port == USART_PORT6)
	{
		while((USART6->SR&0X40)==0);//ѭ������,ֱ���������   
		USART6->DR = (u8) ch;
	}
#if SYSTEM_SUPPORT_OS
	OSIntExit();
#endif		
	return ch;
}
#endif

/**
	* @brief  usart��ʼ��
	* @param  baud:������
	* @retval None
*/
void usart1_Init(u32 baud)
{
	USART_InitTypeDef USART_InitStucture;
	NVIC_InitTypeDef NVIC_InitStructure;
	usart1_GPIOConfig();
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	USART_InitStucture.USART_BaudRate = baud;
	USART_InitStucture.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStucture.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_InitStucture.USART_Parity = USART_Parity_No;
	USART_InitStucture.USART_StopBits = USART_StopBits_1;
	USART_InitStucture.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &USART_InitStucture); //��ʼ������1
	USART_Cmd(USART1, ENABLE);  //ʹ�ܴ���1 
		
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//��������ж�
	//Usart1 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//����1�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���

	USART_ClearFlag(USART1,USART_FLAG_TC);
}

/**
	* @brief  usart�жϽ��պ���
	* @param  None
	* @retval None
*/
void USART1_IRQHandler(void)                	//����1�жϷ������
{
	u8 Res;
#if SYSTEM_SUPPORT_OS
	OSIntEnter();
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		Res =USART_ReceiveData(USART1);//(USART1->DR);	//��ȡ���յ�������
		
		if((USART_RX_STA&0x8000)==0)//����δ���
		{
			if(USART_RX_STA&0x4000)//���յ���0x0d
			{
				if(Res!=0x0a)USART_RX_STA=0;//���մ���,���¿�ʼ
				else USART_RX_STA|=0x8000;	//��������� 
			}
			else //��û�յ�0X0D
			{	
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
				}		 
			}
		}   		 
  }
#if SYSTEM_SUPPORT_OS
	OSIntExit();
#endif	
} 

/**
	* @brief  usart1����IO
	* @param  None
	* @retval None
*/
static void usart1_GPIOConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	//USART1�˿�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9��GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//�ٶ�50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //����
	GPIO_Init(GPIOA,&GPIO_InitStructure); //��ʼ��PA9��PA10

	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);
}

///////////////////////////////// USART6 /////////////////////////
/**
	* @brief  usart��ʼ��
	* @param  baud:������
	* @retval None
*/
void usart6_Init(u32 baud)
{
	USART_InitTypeDef USART_InitStucture;
	NVIC_InitTypeDef NVIC_InitStructure;
	usart6_GPIOConfig();
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

	USART_InitStucture.USART_BaudRate = baud;
	USART_InitStucture.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
	USART_InitStucture.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
	USART_InitStucture.USART_Parity = USART_Parity_No;
	USART_InitStucture.USART_StopBits = USART_StopBits_1;
	USART_InitStucture.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART6, &USART_InitStucture); //��ʼ������6
	USART_Cmd(USART6, ENABLE);  //ʹ�ܴ���6 
		
	USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);//��������ж�
	//Usart6 NVIC ����
	NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;//����6�ж�ͨ��
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//��ռ���ȼ�0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
	NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���

	USART_ClearFlag(USART6,USART_FLAG_TC);
}

/**
	* @brief  usart�жϽ��պ���
	* @param  None
	* @retval None
*/
void USART6_IRQHandler(void)                	//����6�жϷ������
{
	u8 Res;
#if SYSTEM_SUPPORT_OS
	OSIntEnter();
#endif
	if(USART_GetITStatus(USART6, USART_IT_RXNE) != RESET)  //�����ж�(���յ������ݱ�����0x0d 0x0a��β)
	{
		Res =USART_ReceiveData(USART6);//(USART6->DR);	//��ȡ���յ�������
		if((USART_RX_STA&0x8000)==0)//����δ���
		{
			if(USART_RX_STA&0x4000)//���յ���0x0d
			{
				if(Res!=0x0a)USART_RX_STA=0;//���մ���,���¿�ʼ
				else USART_RX_STA|=0x8000;	//��������� 
			}
			else //��û�յ�0X0D
			{	
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
				}		 
			}
		}		
	}
#if SYSTEM_SUPPORT_OS
	OSIntExit();
#endif	
} 

/**
	* @brief  usart����IO
	* @param  None
	* @retval None
*/
static void usart6_GPIOConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	//USART6�˿�����
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; //GPIOC6��GPIOC7
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//���ù���
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC,&GPIO_InitStructure); //��ʼ��PC6��PC7
	
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6); 
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);

	//���������Ϊ����ʹ�ã�Ҫ����wifiģ�飬�����޷�����
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_14);
	GPIO_ResetBits(GPIOB, GPIO_Pin_15);
}
/********************************************* *****END OF FILE****/
