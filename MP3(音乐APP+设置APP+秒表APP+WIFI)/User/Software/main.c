/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "hw_includes.h"
#include "ff.h"  
#include "exfuns.h"  
#include "mp3Player.h"
#include "stdio.h"
#include "ADC.h"
#include "KEY.h"
#include "music.h"
#include "mpu6050.h"
#include "usart3.h"
#include "esp8266.h"
#include "weather.h"
#include "lcd_init.h"
#include "lcd.h"
#include "pic.h"
#include "PWM.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "event_groups.h"

/**********************开始任务**************************/
//任务优先级
#define START_TASK_PRIO		1
//任务堆栈大小	
#define START_STK_SIZE 		128
//任务句柄
TaskHandle_t StartTask_Handler;
//任务函数
void start_task(void *pvParameters);
/*******************************************************/

/*********************调节任务**************************/
//任务优先级
#define ADJUST_TASK_PRIO		3
//任务堆栈大小	
#define ADJUST_STK_SIZE 		128
//任务句柄
TaskHandle_t ADJUST_Task_Handler;
//任务函数
void ADJUST_task(void *pvParameters);
/*******************************************************/

/******************LED_APP任务**************************/
//任务优先级
#define LED_TASK_PRIO		3
//任务堆栈大小	
#define LED_STK_SIZE 		128
//任务句柄
TaskHandle_t LED_Task_Handler;
//任务函数
void LED_task(void *pvParameters);
/*******************************************************/

/******************播放音乐任务**************************/
//任务优先级
#define MUSIC_TASK_PRIO		2
//任务堆栈大小	
#define MUSIC_STK_SIZE 		256
//任务句柄
TaskHandle_t MUSIC_Task_Handler;
//任务函数
void MUSIC_task(void *pvParameters);
/*******************************************************/

/**********************输入任务**************************/
//任务优先级
#define INPUT_TASK_PRIO		5
//任务堆栈大小	
#define INPUT_STK_SIZE 		128
//任务句柄
TaskHandle_t INPUT_Task_Handler;
//任务函数
void INPUT_task(void *pvParameters);
/*******************************************************/

/**********************显示任务**************************/
//任务优先级
#define LCD_TASK_PRIO		2
//任务堆栈大小	
#define LCD_STK_SIZE 		256
//任务句柄
TaskHandle_t LCD_Task_Handler;
//任务函数
void LCD_task(void *pvParameters);
/*******************************************************/

/*********************WIFI任务**************************/
//任务优先级
#define WIFI_TASK_PRIO		4
//任务堆栈大小	
#define WIFI_STK_SIZE 		256
//任务句柄
TaskHandle_t WIFI_Task_Handler;
//任务函数
void WIFI_task(void *pvParameters);
/*******************************************************/

/*********************秒表任务**************************/
//任务优先级
#define WIFI_TASK_PRIO		4
//任务堆栈大小	
#define WIFI_STK_SIZE 		256
//任务句柄
TaskHandle_t WIFI_Task_Handler;
//任务函数
void WIFI_task(void *pvParameters);
/*******************************************************/

/************************队列***************************/
#define music_QUEUE_LEN	4	//队列长度
#define music_QUEUE_SIZE	4	//队列中每个消息的大小
QueueHandle_t music_Queue;	//按键消息队列句柄

#define LED_QUEUE_LEN	2	//队列长度
#define LED_QUEUE_SIZE	2	//队列中每个消息的大小
QueueHandle_t LED_Queue;	//按键消息队列句柄

#define time_QUEUE_LEN	2	//队列长度
#define time_QUEUE_SIZE	sizeof(nt_calendar_obj)	//队列中每个消息的大小
QueueHandle_t time_Queue;	//按键消息队列句柄
/*******************************************************/

/************************事件***************************/
EventGroupHandle_t EventGroupHandler;	
#define EVENTBIT_0	(1<<0)				//事件位
#define EVENTBIT_1	(1<<1)
#define EVENTBIT_2	(1<<2)
#define EVENTBIT_3	(1<<3)
/*******************************************************/

/**********************软件定时器************************/
TimerHandle_t 	AutoReloadTimer_Handle;			//周期定时器句柄
void AutoReloadCallback(TimerHandle_t xTimer); 	//周期定时器回调函数
/*******************************************************/

/************************全局变量************************/
#define 		music_max		26
u16 volume = 30;			//音量
int charge_value;	//电量
u16 charge_flag;	//充电标志位
u8 music_n;			//第n首音乐
int brightness =30;	//亮度
u8	m_sec;				//秒表毫秒
u8	sec;				//秒表秒
u8	min;				//秒表分
/*******************************************************/

/***********************自定义函数***********************/
u8 scan_files(u8 * path);
void start(void);
void meau(void);
void n_remove(int key,int n);
void meau_2(int n);
void music_app(void);
void set_app(void);
void get_charge_show(int value);
void set_app_2(int n_index);
void stopwatch(void);
void led_APP(void);
/*******************************************************/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */ 
int main(void)
{	
	delay_init(168);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	usart1_Init(115200);
	LED_Init();
	DAC_Config();
	Adc_Init();
	KEY_Init();
	MPU_Init();	
	usart3_Init(115200);
	LCD_Init();
	TIM2_PWM_Init(100-1,500-1);
	PWM_Init();
	
	p_brightness(brightness);
	led_brightness(0);
	
	start();

	if(!SD_Init())
 	{
		exfuns_init();							//为fatfs相关变量申请内存				 
		f_mount(fs[0],"0:",1); 					//挂载SD卡 
	}

	//打印SD目录和文件
	//scan_files("0:");
	
	//创建开始任务
    xTaskCreate((TaskFunction_t )start_task,            //任务函数
                (const char*    )"start_task",          //任务名称
                (uint16_t       )START_STK_SIZE,        //任务堆栈大小
                (void*          )NULL,                  //传递给任务函数的参数
                (UBaseType_t    )START_TASK_PRIO,       //任务优先级
                (TaskHandle_t*  )&StartTask_Handler);   //任务句柄              
    vTaskStartScheduler();          //开启任务调度
}
	
//开始任务任务函数
void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();           //进入临界区
	
	//创建事件
	EventGroupHandler=xEventGroupCreate();
	if(EventGroupHandler == NULL)
		printf("创建EventGroupHandler失败！\r\n");
	
	//创建消息队列
	music_Queue = xQueueCreate(music_QUEUE_LEN,music_QUEUE_SIZE);
	if(music_Queue == NULL)
		printf("创建Key_Queue失败！\r\n");
	
	//创建消息队列
	LED_Queue = xQueueCreate(LED_QUEUE_LEN,LED_QUEUE_SIZE);
	if(LED_Queue == NULL)
		printf("创建LED_Queue失败！\r\n");
	
	//创建消息队列
	time_Queue = xQueueCreate(time_QUEUE_LEN,time_QUEUE_SIZE);
	if(time_Queue == NULL)
		printf("创建Key_Queue失败！\r\n");
	
	//创建TASK2任务
    xTaskCreate((TaskFunction_t )INPUT_task,             
                (const char*    )"INPUT_task",           
                (uint16_t       )INPUT_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )INPUT_TASK_PRIO,        
                (TaskHandle_t*  )&INPUT_Task_Handler);  
	
	//创建TASK2任务
    xTaskCreate((TaskFunction_t )WIFI_task,             
                (const char*    )"WIFI_task",           
                (uint16_t       )WIFI_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )WIFI_TASK_PRIO,        
                (TaskHandle_t*  )&WIFI_Task_Handler);  
				
	//创建TASK2任务
    xTaskCreate((TaskFunction_t )ADJUST_task,             
                (const char*    )"ADJUST_task",           
                (uint16_t       )ADJUST_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )ADJUST_TASK_PRIO,        
                (TaskHandle_t*  )&ADJUST_Task_Handler);  
				
	//创建TASK2任务
    xTaskCreate((TaskFunction_t )LCD_task,             
                (const char*    )"LCD_task",           
                (uint16_t       )LCD_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )LCD_TASK_PRIO,        
                (TaskHandle_t*  )&LCD_Task_Handler);  
				
	//创建TASK2任务
    xTaskCreate((TaskFunction_t )LED_task,             
                (const char*    )"LED_task",           
                (uint16_t       )LED_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )LED_TASK_PRIO,        
                (TaskHandle_t*  )&LED_Task_Handler);
				
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
}


void MUSIC_task(void *pvParameters)
{
	while (1)
	{
		mp3PlayerDemo(music,&music_n);
		
		vTaskDelay(1000);  
	}
	
}



void INPUT_task(void *pvParameters)
{
	//short gyrox,gyroy,gyroz;
	u8 key=1;
	static u8 key_2=0;
	static u8 key_n=0;
	while (1)
	{
		//MPU_Get_Gyroscope(&gyrox,&gyroy,&gyroz);
		key = KEY_Scan();
		if(key == 3)
		{
			
			for(int i=0;i<10;i++)
			{
				key_2 = KEY_Scan();
				if(key_2==3)
				key_n++;
			}
		}
		if(key_n>=9)
		{
			key = 4;
		}
		key_n=0;
		
		switch(key)
			{
				case 1:
					xEventGroupSetBits(EventGroupHandler,EVENTBIT_0);
					break;
				case 2:
					xEventGroupSetBits(EventGroupHandler,EVENTBIT_1);
					break;	
				case 3:
					xEventGroupSetBits(EventGroupHandler,EVENTBIT_2);
					break;	
				case 4:
					xEventGroupSetBits(EventGroupHandler,EVENTBIT_3);
					break;	
				default:
					break; 
			}
		
		//printf("%d\r\n",adcx);
		//printf("%d  %d  %d\r\n",gyrox,gyroy,gyroz);
		//LED0_TROG;
		vTaskDelay(100);  
	}
	
}

void WIFI_task(void *pvParameters)
{
	int t=0;
	esp8266_start_trans();  
	esp8266_quit_trans();
	printf("wifi init ok!");
	while (1)
	{
		LED0_TROG;
		vTaskDelay(100); 
		get_jinzhou_time();	
		//printf("1     time : %d:%d:%d:%d:%d\r\n",nwt.year,nwt.month,nwt.date,nwt.hour,nwt.min);
		if((time_Queue!=NULL))
		{
			xQueueSend(time_Queue,&nwt,10);
		}
		t++;
		if(t==1000)
		{
			get_current_weather();
			t=0;
		}
		

	}
	
}

void ADJUST_task(void *pvParameters)
{
	uint16_t charge_ad=0;
	while (1)
	{	
		p_brightness(brightness);
		charge_flag=Get_charge_value(ADC_Channel_10);
		charge_ad=Get_charge(ADC_Channel_9);
		charge_value=(charge_ad-3020)*100/950;
		if(charge_value>=100)
			charge_value=100;
		vTaskDelay(200); 
		//printf("%d   \r\n",charge_value);
	}
	
}

void LED_task(void *pvParameters)
{
	static u8 led_switch=1;
	while (1)
	{	
		xQueueReceive(LED_Queue,&led_switch,0);
		
		switch(led_switch)
		{
			case 1:
				led_brightness(0);
				vTaskDelay(300);
				break;
			case 2:
				led_brightness(100);
				vTaskDelay(300);
				break;
			case 3:
				while(1)
				{
					xQueueReceive(LED_Queue,&led_switch,0);
					led_brightness(100);
					vTaskDelay(300);
					led_brightness(0);
					vTaskDelay(300);
					if(led_switch != 3)
						break;
				}
				break;
			case 4:
				while(1)
				{
					for(int i=0;i<100;i++)
					{
						xQueueReceive(LED_Queue,&led_switch,0);
						led_brightness(i);
						vTaskDelay(10);
					}
					for(int i=100;i>0;i--)
					{
						xQueueReceive(LED_Queue,&led_switch,0);
						led_brightness(i);
						vTaskDelay(10);
					}
					
					if(led_switch != 4)
						break;
				}
				break;
			default:
				break;
		}
		 
	}
	
}

void LCD_task(void *pvParameters)
{
	//指针图标
	LCD_ShowPicture(10,55,30,30,p_index);
	while (1)
	{
		meau();
		vTaskDelay(10); 
		//printf("%d\r\n",charge);
	}
	
}
/***********************自定义函数***********************/
void start(void)
{
	LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
	//FAZI
	LCD_ShowPicture(0,0,240,186,fazi);
	//进度条
	LCD_ShowString(30,200,"l love yyj",RED,DARKBLUE,32,1);
	LCD_Fill(20,260,LCD_W-20,300,DARKBLUE);
	LCD_Fill(25,265,LCD_W-25,295,WHITE);
	for(int i=28;i<213;i++)
	{
		LCD_Fill(28,268,i,292,DARKBLUE);
		delay_ms(10);
	}
	//清屏
	LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
}

void meau(void)
{
	EventBits_t EventValue;
	static int my_index = 1;
	static nt_calendar_obj timer = {0,0,0,0,0};
	
	//音量图标
	LCD_ShowPicture(5,295,20,20,volum_icon);
	LCD_ShowIntNum(26,297,volume,3,DARKBLUE,WHITE,16);
	
	//亮度图标
	LCD_ShowPicture(70,295,20,20,brightness_icon);
	LCD_ShowIntNum(91,297,brightness,3,DARKBLUE,WHITE,16);
	
	//电量图标
	get_charge_show(charge_value);
	
	//充电图标
	if(charge_flag>=2000)
		LCD_ShowPicture(220,0,20,19,charge_on);
	else
		LCD_Fill(220,0,240,20,WHITE);
	
	//wifi图标
	if(wifi_flag == 0)
		LCD_ShowPicture(125,0,20,20,wifi_off);
	else
		LCD_ShowPicture(125,0,20,20,wifi);
	
	xQueueReceive(time_Queue,&timer,0);

		//printf("time : %d:%d:%d:%d:%d\r\n",timer.year,timer.month,timer.date,timer.hour,timer.min);
	LCD_ShowIntNum(0,0,timer.year,4,BLACK,WHITE,16);
	LCD_ShowString(38,0,"-",BLACK,WHITE,16,0);
	LCD_ShowIntNum(45,0,timer.month,2,BLACK,WHITE,16);
	LCD_ShowString(63,0,"-",BLACK,WHITE,16,0);
	LCD_ShowIntNum(70,0,timer.date,2,BLACK,WHITE,16);
	
	LCD_ShowIntNum(170,290,timer.hour,2,BLACK,WHITE,24);
	LCD_ShowString(195,290,":",BLACK,WHITE,24,0);
	LCD_ShowIntNum(205,290,timer.min,2,BLACK,WHITE,24);
	
	//音乐图标
	LCD_ShowPicture(50,40,60,60,p_music);
	//秒表图标
	LCD_ShowPicture(50,120,60,60,time);
	//led图标
	LCD_ShowPicture(50,200,60,60,P_LED);
	//WIFI图标
	LCD_ShowPicture(160,40,60,60,p_wifi);
	//设置图标
	LCD_ShowPicture(160,200,60,60,p_set);
	//文件图标
	LCD_ShowPicture(160,120,60,60,txt);
	
	//等待事件组中的相应事件位
	EventValue=xEventGroupWaitBits((EventGroupHandle_t	)EventGroupHandler,		
								   (EventBits_t			)EVENTBIT_0 | EVENTBIT_1 | EVENTBIT_2  | EVENTBIT_3,
								   (BaseType_t			)pdTRUE,				
								   (BaseType_t			)pdFALSE,
						           (TickType_t			)10);
	switch(EventValue)
	{
		case 1:
			if(my_index >= 2)
			{
				n_remove(EventValue,my_index);
				my_index--;
			}
			break;
		case 2:
			if(my_index < 6)
			{
				n_remove(EventValue,my_index);
				my_index++;
			}
			break;
		case 4:
				meau_2(my_index);
			break;
		default:
			break;
	}
	
}


void n_remove(int key,int n)
{
	switch(n)
	{
		case 1:
			if(key == 2)
			{
				for(int i=55;i<=135;i++)
				{
					LCD_ShowPicture(10,i,30,30,p_index);
					LCD_ShowPicture(10,i-1,30,1,null_roll);
				}
			}
			break;
		case 2:
			if(key == 1)
			{
				for(int i=135;i>=55;i--)
				{
					LCD_ShowPicture(10,i,30,30,p_index);
					LCD_ShowPicture(10,i+30,30,1,null_roll);
				}
			}
			else if(key == 2)
			{
				for(int i=135;i<=215;i++)
				{
					LCD_ShowPicture(10,i,30,30,p_index);
					LCD_ShowPicture(10,i-1,30,1,null_roll);
				}
			}
			break;
			
		case 3:
			if(key == 1)
			{
				for(int i=215;i>=135;i--)
				{
					LCD_ShowPicture(10,i,30,30,p_index);
					LCD_ShowPicture(10,i+30,30,1,null_roll);
				}
			}
			else if(key == 2)
			{
				LCD_Fill(10,215,40,215+30,WHITE);
				LCD_ShowPicture(120,55,30,30,p_index);
			}
			break;
		case 4:
			
			if(key == 1)
			{
				LCD_Fill(120,55,120+30,55+30,WHITE);
				LCD_ShowPicture(10,215,30,30,p_index);
			}
			else if(key == 2)
			{
				for(int i=55;i<=135;i++)
				{
					LCD_ShowPicture(120,i,30,30,p_index);
					LCD_ShowPicture(120,i-1,30,1,null_roll);
				}
			}
			break;
		case 5:
			if(key == 1)
			{
				for(int i=135;i>=55;i--)
				{
					LCD_ShowPicture(120,i,30,30,p_index);
					LCD_ShowPicture(120,i+30,30,1,null_roll);
				}
			}
			else if(key == 2)
			{
				for(int i=135;i<=215;i++)
				{
					LCD_ShowPicture(120,i,30,30,p_index);
					LCD_ShowPicture(120,i-1,30,1,null_roll);
				}
			}
			break;
		case 6:
			if(key == 1)
			{
				for(int i=215;i>=135;i--)
				{
					LCD_ShowPicture(120,i,30,30,p_index);
					LCD_ShowPicture(120,i+30,30,1,null_roll);
				}
			}
			break;
		default:
			break;
	}
}

void meau_2(int n)
{
	switch(n)
	{
		case 1:
			music_app();
			break;
		case 2:
			stopwatch();
			break;
		case 3:
			led_APP();
			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			set_app();
			break;
		default:
			break;
	}
}

void music_app(void)
{
	EventBits_t EventValue;
	static int st_flag = 1;
	int break_flag=0;
	
	LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
	LCD_ShowPicture(20,260,40,40,music_last);
	LCD_ShowPicture(180,260,40,40,music_next);
	LCD_ShowPicture(100,260,40,40,music_start);
	LCD_ShowPicture(0,20,240,186,fazi);
	LCD_ShowPicture(10,210,30,30,music_2);
	LCD_ShowIntNum(40,210,music_n+1,2,BLACK,WHITE,24);
	
	//创建TASK1任务
	taskENTER_CRITICAL();
	
    xTaskCreate((TaskFunction_t )MUSIC_task,             
                (const char*    )"MUSIC_task",           
                (uint16_t       )MUSIC_STK_SIZE,        
                (void*          )NULL,                  
                (UBaseType_t    )MUSIC_TASK_PRIO,        
                (TaskHandle_t*  )&MUSIC_Task_Handler);   
	
	vTaskSuspend(MUSIC_Task_Handler);
	DAC_Cmd(DAC_Channel_1, DISABLE);
	DAC_Cmd(DAC_Channel_2, DISABLE);
				
	taskEXIT_CRITICAL();
				
	
	LCD_ShowChinese(70,211,*(music_num+music_n),RED,WHITE,24,0);
				
	while(1)
	{
		EventValue=xEventGroupWaitBits((EventGroupHandle_t	)EventGroupHandler,		
								   (EventBits_t			)EVENTBIT_0 | EVENTBIT_1 | EVENTBIT_2 | EVENTBIT_3,
								   (BaseType_t			)pdTRUE,				
								   (BaseType_t			)pdFALSE,
						           (TickType_t			)1000);	
		switch(EventValue)
		{
			case 0:
				vTaskDelay(100);
				break;
			case 1:
				for(int i=260;i>=240;i--)
				{
					LCD_ShowPicture(20,i,40,40,music_last);
					LCD_ShowPicture(20,i+40,30,1,null_roll);
					vTaskDelay(1);
				}
				for(int i=240;i<=260;i++)
				{
					LCD_ShowPicture(20,i,40,40,music_last);
					LCD_ShowPicture(20,i-1,30,1,null_roll);
					vTaskDelay(1);
				}
				if(music_n>0)
				music_n--;
				LCD_ShowIntNum(40,210,music_n+1,2,BLACK,WHITE,24);
				LCD_Fill(70,211,240,235,WHITE);
				LCD_ShowChinese(70,211,*(music_num+music_n),RED,WHITE,24,0);
				
				
				
				if((music_Queue!=NULL)&&(EventValue))
				{
					xQueueSend(music_Queue,&EventValue,10);
				}
				break;
			case 2:
				for(int i=260;i>=240;i--)
				{
					LCD_ShowPicture(180,i,40,40,music_next);
					LCD_ShowPicture(180,i+40,30,1,null_roll);
					vTaskDelay(1);
				}
				for(int i=240;i<=260;i++)
				{
					LCD_ShowPicture(180,i,40,40,music_next);
					LCD_ShowPicture(180,i-1,30,1,null_roll);
					vTaskDelay(1);
				}
				if(music_n<music_max)
				music_n++;
				LCD_ShowIntNum(40,210,music_n+1,2,BLACK,WHITE,24);
				LCD_Fill(70,211,240,235,WHITE);
				LCD_ShowChinese(70,211,*(music_num+music_n),RED,WHITE,24,0);
				
				
				if((music_Queue!=NULL)&&(EventValue))
				{
					xQueueSend(music_Queue,&EventValue,10);
				}
				break;
			case 4:
				st_flag = -st_flag;
				if(st_flag == -1)
				{
					for(int i=260;i>=240;i--)
					{
						LCD_ShowPicture(100,i,40,40,music_start);
						LCD_ShowPicture(100,i+40,30,1,null_roll);
						vTaskDelay(1);
					}
					
					for(int i=240;i<=260;i++)
					{
						LCD_ShowPicture(100,i,40,40,music_stop);
						LCD_ShowPicture(100,i-1,30,1,null_roll);
						vTaskDelay(1);
					}
					
					vTaskResume(MUSIC_Task_Handler);
					DAC_Cmd(DAC_Channel_1, ENABLE);
					DAC_Cmd(DAC_Channel_2, ENABLE);
				}
				
				if(st_flag == 1)
				{
					for(int i=260;i>=240;i--)
					{
						LCD_ShowPicture(100,i,40,40,music_stop);
						LCD_ShowPicture(100,i+40,30,1,null_roll);
						vTaskDelay(1);
					}
					
					for(int i=240;i<=260;i++)
					{
						LCD_ShowPicture(100,i,40,40,music_start);
						LCD_ShowPicture(100,i-1,30,1,null_roll);
						vTaskDelay(1);
					}
					
					vTaskSuspend(MUSIC_Task_Handler);
					DAC_Cmd(DAC_Channel_1, DISABLE);
					DAC_Cmd(DAC_Channel_2, DISABLE);
				}
				
				break;
			case 8:
				break_flag=1;
				break;
			default:
				break;
		}
		vTaskDelay(10); 
		if(break_flag == 1)
		{
			LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
			DAC_Cmd(DAC_Channel_1, DISABLE);
			DAC_Cmd(DAC_Channel_2, DISABLE);
			//DAC_DMA_Stop();
			LCD_ShowPicture(10,55,30,30,p_index);
			vTaskDelete(MUSIC_Task_Handler);
			xEventGroupClearBits(EventGroupHandler,EVENTBIT_3);
			break;
			
		}
	}
	
}

void get_charge_show(int value)
{
	switch(value/18)
	{
		case 5:
			LCD_ShowPicture(175,0,40,20,charge_6);
			break;
		case 4:
			LCD_ShowPicture(175,0,40,20,charge_5);
			break;
		case 3:
			LCD_ShowPicture(175,0,40,20,charge_4);
			break;
		case 2:
			LCD_ShowPicture(175,0,40,20,charge_3);
			break;
		case 1:
			LCD_ShowPicture(175,0,40,20,charge_2);
			break;
		case 0:
			LCD_ShowPicture(175,0,40,20,charge_1);
			break;
		default:
			break;
	}
	LCD_ShowIntNum(150,3,value,3,DARKBLUE,WHITE,16);
}


void set_app(void)
{
	int stop_flag = 0;
	int n_index=1;
	EventBits_t EventValue;
	LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
	//指针图标
	LCD_ShowPicture(10,55,30,30,p_index);
	
	//音量图标
	LCD_ShowPicture(45,55,30,30,p_volume);
	LCD_ShowPicture(95+volume,55,10,20,p__index);
	LCD_Fill(100,75,200,78,DARKBLUE);
	LCD_ShowIntNum(203,55,volume,3,DARKBLUE,WHITE,16);
	
	//亮度图标
	LCD_ShowPicture(45,95,30,30,p__brightness);
	LCD_ShowPicture(95+brightness,95,10,20,p__index);
	LCD_Fill(100,115,200,118,DARKBLUE);
	LCD_ShowIntNum(203,95,brightness,3,DARKBLUE,WHITE,16);
	
	while(1)
	{
		EventValue=xEventGroupWaitBits((EventGroupHandle_t	)EventGroupHandler,		
								   (EventBits_t			)EVENTBIT_0 | EVENTBIT_1 | EVENTBIT_2 | EVENTBIT_3,
								   (BaseType_t			)pdTRUE,				
								   (BaseType_t			)pdFALSE,
						           (TickType_t			)10);
		switch(EventValue)
		{
			case 1:
				if(n_index>1)
				{
					n_index--;
					for(int i=95;i>=55;i--)
					{
						LCD_ShowPicture(10,i,30,30,p_index);
						LCD_ShowPicture(10,i+30,30,1,null_roll);
					}
				}
				break;
			case 2:
				if(n_index<2)
				{
					n_index++;
					for(int i=55;i<=95;i++)
					{
						LCD_ShowPicture(10,i,30,30,p_index);
						LCD_ShowPicture(10,i-1,30,1,null_roll);
					}
				}
				break;
			case 4:
				set_app_2(n_index);
				break;
			case 8:
				stop_flag=1;
				break;
			default:
				break;
		}
		if(stop_flag==1)
		{
			LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
			LCD_ShowPicture(120,215,30,30,p_index);
			xEventGroupClearBits(EventGroupHandler,EVENTBIT_3);
			break;
		}
		vTaskDelay(10); 
		
	}
}

void set_app_2(int n_index)
{
	EventBits_t EventValue;
	while(1)
	{
		EventValue=xEventGroupWaitBits((EventGroupHandle_t	)EventGroupHandler,		
								   (EventBits_t			)EVENTBIT_0 | EVENTBIT_1 | EVENTBIT_2,
								   (BaseType_t			)pdTRUE,				
								   (BaseType_t			)pdFALSE,
						           (TickType_t			)10);
		if(n_index==1)
		{
			LCD_Fill(100,75,200,78,RED);
			if(EventValue==1)
			{
				if(volume>=1)
				{
					volume--;
					LCD_ShowIntNum(203,55,volume,3,DARKBLUE,WHITE,16);
					LCD_ShowPicture(95+volume,55,10,20,p__index);
					LCD_Fill(95+volume+10,55,95+volume+11,75,WHITE);
					
				}
			}
			else if(EventValue==2)
			{
				if(volume<=99)
				{
					volume++;
					LCD_ShowIntNum(203,55,volume,3,DARKBLUE,WHITE,16);
					LCD_ShowPicture(95+volume,55,10,20,p__index);
					LCD_Fill(95+volume-1,55,95+volume,75,WHITE);
				}
			}
			else if(EventValue==4)
			{
				LCD_Fill(100,75,200,78,DARKBLUE);
				break;
			}
		}
		if(n_index==2)
		{
			LCD_Fill(100,115,200,118,RED);
			if(EventValue==1)
			{
				if(brightness>=1)
				{
					brightness--;
					LCD_ShowIntNum(203,95,brightness,3,DARKBLUE,WHITE,16);
					LCD_ShowPicture(95+brightness,95,10,20,p__index);
					LCD_Fill(95+brightness+10,95,95+brightness+11,115,WHITE);
					
				}
			}
			else if(EventValue==2)
			{
				if(brightness<=99)
				{
					brightness++;
					LCD_ShowIntNum(203,95,brightness,3,DARKBLUE,WHITE,16);
					LCD_ShowPicture(95+brightness,95,10,20,p__index);
					LCD_Fill(95+brightness-1,95,95+brightness,115,WHITE);
				}
			}
			else if(EventValue==4)
			{
				LCD_Fill(100,115,200,118,DARKBLUE);
				break;
			}
		}
		vTaskDelay(10); 
	}
}


void stopwatch(void)
{
	EventBits_t EventValue;
	
	LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
	LCD_ShowPicture(0,0,240,120,build);
	
	LCD_Fill(37,147,208,185,RED);
	LCD_Fill(39,149,206,183,WHITE);
	
	LCD_ShowPicture(20,240,40,40,music_start);
	LCD_ShowPicture(180,240,40,40,music_stop);
	
	LCD_ShowIntNum(40,150,min,2,DARKBLUE,WHITE,32);
	LCD_ShowIntNum(100,150,sec,2,DARKBLUE,WHITE,32);
	LCD_ShowIntNum(160,150,m_sec,2,DARKBLUE,WHITE,32);
	
	AutoReloadTimer_Handle=xTimerCreate((const char*		)"AutoReloadTimer",
									    (TickType_t			)10,
							            (UBaseType_t		)pdTRUE,
							            (void*				)1,
							            (TimerCallbackFunction_t)AutoReloadCallback);
	
	while(1)
	{
		EventValue=xEventGroupWaitBits((EventGroupHandle_t	)EventGroupHandler,		
								   (EventBits_t			)EVENTBIT_0 | EVENTBIT_1 | EVENTBIT_2 | EVENTBIT_3,
								   (BaseType_t			)pdTRUE,				
								   (BaseType_t			)pdFALSE,
						           (TickType_t			)10);
		
		LCD_ShowIntNum(40,150,min,2,DARKBLUE,WHITE,32);
		LCD_ShowIntNum(100,150,sec,2,DARKBLUE,WHITE,32);
		LCD_ShowIntNum(160,150,m_sec,2,DARKBLUE,WHITE,32);
		
		if(EventValue == 1)
		{
			LCD_Fill(180,280,220,285,WHITE);
			LCD_Fill(20,280,60,285,DARKBLUE);
			xTimerStart(AutoReloadTimer_Handle,0);
		}
		else if(EventValue == 2)
		{
			LCD_Fill(180,280,220,285,DARKBLUE);
			LCD_Fill(20,280,60,285,WHITE);
			
			xTimerStop(AutoReloadTimer_Handle,0); 
		}
		
		else if(EventValue == 4)
		{
			m_sec=0;
			sec=0;
			LCD_ShowIntNum(40,150,min,2,DARKBLUE,WHITE,32);
			LCD_ShowIntNum(100,150,sec,2,DARKBLUE,WHITE,32);
			LCD_ShowIntNum(160,150,m_sec,2,DARKBLUE,WHITE,32);
		}
		
		else if(EventValue == 8)
		{
			LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
			LCD_ShowPicture(10,135,30,30,p_index);
			xEventGroupClearBits(EventGroupHandler,EVENTBIT_3);
			xTimerDelete(AutoReloadTimer_Handle,0);
			break;
		}
		
		vTaskDelay(10); 
	}
}

void AutoReloadCallback(TimerHandle_t xTimer)
{
	m_sec++;
	if(m_sec == 100)
	{
		m_sec = 0;
		sec ++;
		if(sec == 60)
		{
			sec = 0;
			min++;
		}
	}
}

void led_APP(void)
{
	EventBits_t EventValue;
	BaseType_t err;
	static int my_index=1;
	int index_y = 150;
	
	LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
	LCD_ShowPicture(0,0,240,120,build);
	
	LCD_ShowString(80,150,"off",BLACK,WHITE,24,0);
	LCD_ShowString(80,180,"Mode 1",BRRED,WHITE,24,0);
	LCD_ShowString(80,210,"Mode 2",MAGENTA,WHITE,24,0);
	LCD_ShowString(80,240,"Mode 3",DARKBLUE,WHITE,24,0);
	
	while(1)
	{
		EventValue=xEventGroupWaitBits((EventGroupHandle_t	)EventGroupHandler,		
								   (EventBits_t			)EVENTBIT_0 | EVENTBIT_1 | EVENTBIT_2 | EVENTBIT_3,
								   (BaseType_t			)pdTRUE,				
								   (BaseType_t			)pdFALSE,
						           (TickType_t			)10);
		if(EventValue == 2)
		{
			my_index++;
			if(my_index==5)
			my_index = 1;
			
			switch(my_index)
		{
			case 1:
				LCD_Fill(10,240,40,270,WHITE);
				index_y = 150;
				break;
			case 2:
				LCD_Fill(10,150,40,180,WHITE);
				index_y = 180;
				break;
			case 3:
				LCD_Fill(10,180,40,210,WHITE);
				index_y = 210;
				break;
			case 4:
				LCD_Fill(10,210,40,240,WHITE);
				index_y = 240;
				break;
			default:
				break;
		}
		}
		
		
		LCD_ShowPicture(10,index_y,30,30,p_index);
		
		if((LED_Queue!=NULL)&&(EventValue == 4))
		{
			err=xQueueSend(LED_Queue,&my_index,10);
			if(err==errQUEUE_FULL)
			{
				printf("队列Key_Queue已满，数据发送失败!\r\n");
			}
			
		}
		
		if(EventValue == 8)
		{
			LCD_Fill(0,0,LCD_W,LCD_H,WHITE);
			LCD_ShowPicture(10,215,30,30,p_index);
			xEventGroupClearBits(EventGroupHandler,EVENTBIT_3);
			break;
		}
		vTaskDelay(10);
	}
}

//遍历目录文件并打印输出
u8 scan_files(u8 * path)
{
	FRESULT res;
	char buf[512] = {0};	
  char *fn;
	
#if _USE_LFN
 	fileinfo.lfsize = _MAX_LFN * 2 + 1;
	fileinfo.lfname = buf;
#endif
 
	res = f_opendir(&dir,(const TCHAR*)path);
	if (res == FR_OK) 
	{	
		printf("\r\n"); 
		
		while(1){
			
			res = f_readdir(&dir, &fileinfo);                
			if (res != FR_OK || fileinfo.fname[0] == 0) break;  
 
#if _USE_LFN
			fn = *fileinfo.lfname ? fileinfo.lfname : fileinfo.fname;
#else							   
			fn = fileinfo.fname;
#endif	    

			printf("%s/", path);			
			printf("%s\r\n", fn);			
		} 
  }	  
 
  return res;	  
}
/*******************************************************/
