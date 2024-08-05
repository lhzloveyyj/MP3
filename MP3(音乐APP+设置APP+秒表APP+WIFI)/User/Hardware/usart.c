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

#define SYSTEM_SUPPORT_OS		0		//定义系统文件夹是否支持OS
#if SYSTEM_SUPPORT_OS
	#include "includes.h"					//ucos 使用	  
#endif

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
u8 USART_RX_BUF[USART_REC_LEN];     //接收缓冲,最大USART_REC_LEN个字节.
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
u16 USART_RX_STA=0;       //接收状态标记	
u8 usart_port = USART_PORT1;

/* Private function prototypes -----------------------------------------------*/
static void usart1_GPIOConfig(void);
static void usart6_GPIOConfig(void);

/* Private functions ---------------------------------------------------------*/

//////////////////////////////////////////////////////////////////
//加入以下代码,支持printf函数,而不需要选择use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;

//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
}

//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
#if SYSTEM_SUPPORT_OS
	OSIntEnter();
#endif
	if(usart_port == USART_PORT1)
	{
		while((USART1->SR&0X40)==0);//循环发送,直到发送完毕   
		USART1->DR = (u8) ch;
	}else if(usart_port == USART_PORT6)
	{
		while((USART6->SR&0X40)==0);//循环发送,直到发送完毕   
		USART6->DR = (u8) ch;
	}
#if SYSTEM_SUPPORT_OS
	OSIntExit();
#endif		
	return ch;
}
#endif

/**
	* @brief  usart初始化
	* @param  baud:波特率
	* @retval None
*/
void usart1_Init(u32 baud)
{
	USART_InitTypeDef USART_InitStucture;
	NVIC_InitTypeDef NVIC_InitStructure;
	usart1_GPIOConfig();
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

	USART_InitStucture.USART_BaudRate = baud;
	USART_InitStucture.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStucture.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_InitStucture.USART_Parity = USART_Parity_No;
	USART_InitStucture.USART_StopBits = USART_StopBits_1;
	USART_InitStucture.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &USART_InitStucture); //初始化串口1
	USART_Cmd(USART1, ENABLE);  //使能串口1 
		
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//开启相关中断
	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=3;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

	USART_ClearFlag(USART1,USART_FLAG_TC);
}

/**
	* @brief  usart中断接收函数
	* @param  None
	* @retval None
*/
void USART1_IRQHandler(void)                	//串口1中断服务程序
{
	u8 Res;
#if SYSTEM_SUPPORT_OS
	OSIntEnter();
#endif
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res =USART_ReceiveData(USART1);//(USART1->DR);	//读取接收到的数据
		
		if((USART_RX_STA&0x8000)==0)//接收未完成
		{
			if(USART_RX_STA&0x4000)//接收到了0x0d
			{
				if(Res!=0x0a)USART_RX_STA=0;//接收错误,重新开始
				else USART_RX_STA|=0x8000;	//接收完成了 
			}
			else //还没收到0X0D
			{	
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
				}		 
			}
		}   		 
  }
#if SYSTEM_SUPPORT_OS
	OSIntExit();
#endif	
} 

/**
	* @brief  usart1配置IO
	* @param  None
	* @retval None
*/
static void usart1_GPIOConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

	//USART1端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9 | GPIO_Pin_10; //GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA,&GPIO_InitStructure); //初始化PA9，PA10

	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1); 
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);
}

///////////////////////////////// USART6 /////////////////////////
/**
	* @brief  usart初始化
	* @param  baud:波特率
	* @retval None
*/
void usart6_Init(u32 baud)
{
	USART_InitTypeDef USART_InitStucture;
	NVIC_InitTypeDef NVIC_InitStructure;
	usart6_GPIOConfig();
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART6, ENABLE);

	USART_InitStucture.USART_BaudRate = baud;
	USART_InitStucture.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStucture.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_InitStucture.USART_Parity = USART_Parity_No;
	USART_InitStucture.USART_StopBits = USART_StopBits_1;
	USART_InitStucture.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART6, &USART_InitStucture); //初始化串口6
	USART_Cmd(USART6, ENABLE);  //使能串口6 
		
	USART_ITConfig(USART6, USART_IT_RXNE, ENABLE);//开启相关中断
	//Usart6 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART6_IRQn;//串口6中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =3;		//子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

	USART_ClearFlag(USART6,USART_FLAG_TC);
}

/**
	* @brief  usart中断接收函数
	* @param  None
	* @retval None
*/
void USART6_IRQHandler(void)                	//串口6中断服务程序
{
	u8 Res;
#if SYSTEM_SUPPORT_OS
	OSIntEnter();
#endif
	if(USART_GetITStatus(USART6, USART_IT_RXNE) != RESET)  //接收中断(接收到的数据必须是0x0d 0x0a结尾)
	{
		Res =USART_ReceiveData(USART6);//(USART6->DR);	//读取接收到的数据
		if((USART_RX_STA&0x8000)==0)//接收未完成
		{
			if(USART_RX_STA&0x4000)//接收到了0x0d
			{
				if(Res!=0x0a)USART_RX_STA=0;//接收错误,重新开始
				else USART_RX_STA|=0x8000;	//接收完成了 
			}
			else //还没收到0X0D
			{	
				if(Res==0x0d)USART_RX_STA|=0x4000;
				else
				{
					USART_RX_BUF[USART_RX_STA&0X3FFF]=Res ;
					USART_RX_STA++;
					if(USART_RX_STA>(USART_REC_LEN-1))USART_RX_STA=0;//接收数据错误,重新开始接收	  
				}		 
			}
		}		
	}
#if SYSTEM_SUPPORT_OS
	OSIntExit();
#endif	
} 

/**
	* @brief  usart配置IO
	* @param  None
	* @retval None
*/
static void usart6_GPIOConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	//USART6端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7; //GPIOC6与GPIOC7
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC,&GPIO_InitStructure); //初始化PC6，PC7
	
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource6, GPIO_AF_USART6); 
	GPIO_PinAFConfig(GPIOC, GPIO_PinSource7, GPIO_AF_USART6);

	//如果单独作为串口使用，要禁用wifi模块，否则无法接收
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
