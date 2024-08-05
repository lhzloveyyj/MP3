#include "usart3.h"
#include "stdarg.h"
#include "string.h"
#include "stdio.h"	
#include "time7.h"	

#define SYSTEM_SUPPORT_OS		0		//定义系统文件夹是否支持OS
#if SYSTEM_SUPPORT_OS
	#include "includes.h"					//ucos 使用	  
#endif

u8 USART3_RX_BUF[USART3_MAX_RECV_LEN]; 				//接收缓冲,最大USART3_MAX_RECV_LEN个字节.
u8  USART3_TX_BUF[USART3_MAX_SEND_LEN]; 
//接收状态
//bit15，	接收完成标志
//bit14，	接收到0x0d
//bit13~0，	接收到的有效字节数目
vu16 USART3_RX_STA=0;       //接收状态标记	
u8 usart3_port = USART_PORT3;

/* Private function prototypes -----------------------------------------------*/
static void usart3_GPIOConfig(void);

/**
	* @brief  usart初始化
	* @param  baud:波特率
	* @retval None
*/
void usart3_Init(u32 baud)
{
	USART_InitTypeDef USART_InitStucture;
	NVIC_InitTypeDef NVIC_InitStructure;
	usart3_GPIOConfig();
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	USART_InitStucture.USART_BaudRate = baud;
	USART_InitStucture.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
	USART_InitStucture.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
	USART_InitStucture.USART_Parity = USART_Parity_No;
	USART_InitStucture.USART_StopBits = USART_StopBits_1;
	USART_InitStucture.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART3, &USART_InitStucture); //初始化串口1
	USART_Cmd(USART3, ENABLE);  //使能串口1 
		
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启相关中断
	//Usart1 NVIC 配置
	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;//串口1中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=6;//抢占优先级0
	NVIC_InitStructure.NVIC_IRQChannelSubPriority =0;		//子优先级2
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
	NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器

	USART_ClearFlag(USART3,USART_FLAG_TC);
	
	TIM7_Int_Init(1000-1,7200-1);		//10ms中断
	USART3_RX_STA=0;		//清零
	TIM_Cmd(TIM7,DISABLE);			//关闭定时器7
}

/**
	* @brief  usart中断接收函数
	* @param  None
	* @retval None
*/
void USART3_IRQHandler(void)                	//串口1中断服务程序
{
	u8 res;
#if SYSTEM_SUPPORT_OS
	OSIntEnter();
#endif
	if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)//接收到数据
	{	 
		res =USART_ReceiveData(USART3);		 
		if((USART3_RX_STA&(1<<15))==0)//接收完的一批数据,还没有被处理,则不再接收其他数据
		{ 
			if(USART3_RX_STA<USART3_MAX_RECV_LEN)	//还可以接收数据
			{
				TIM_SetCounter(TIM7,0);//计数器清空          		
				
				if(USART3_RX_STA==0) 				//使能定时器7的中断 
				{
					TIM_Cmd(TIM7,ENABLE);//使能定时器7
				}
				USART3_RX_BUF[USART3_RX_STA++]=res;	//记录接收到的值	 
			}else 
			{
				USART3_RX_STA|=1<<15;				//强制标记接收完成
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
static void usart3_GPIOConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);

	//USART1端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11; //GPIOA9与GPIOA10
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;//复用功能
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//速度50MHz
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽复用输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOB,&GPIO_InitStructure); //初始化PA9，PA10

	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_USART3); 
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_USART3);
}

//串口3,printf 函数
//确保一次发送数据不超过USART3_MAX_SEND_LEN字节
void u3_printf(char* fmt,...)  
{  
	u16 i,j; 
	va_list ap; 
	va_start(ap,fmt);
	vsprintf((char*)USART3_TX_BUF,fmt,ap);
	va_end(ap);
	i=strlen((const char*)USART3_TX_BUF);		//此次发送数据的长度
	for(j=0;j<i;j++)							//循环发送数据
	{
	  while(USART_GetFlagStatus(USART3,USART_FLAG_TC)==RESET); //循环发送,直到发送完毕   
		USART_SendData(USART3,USART3_TX_BUF[j]); 
	} 
}


