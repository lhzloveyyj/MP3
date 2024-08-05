#include "weather.h"
#include "usart.h"
#include "malloc.h"
#include "usart3.h"
#include "parsejson.h"
#include "esp8266.h"
#include "string.h"

#include "FreeRTOS.h"
#include "task.h"


//�������Ӷ˿ں�:80	
#define WEATHER_PORTNUM 	"80"
//����������IP
#define WEATHER_SERVERIP 	"api.seniverse.com"

//ʱ��˿ں�
#define TIME_PORTNUM			"80"
//ʱ�������IP
#define TIME_SERVERIP			"www.beijing-time.org"

	
  
//����ṹ�����


nt_calendar_obj nwt;  //����ṹ�����

//��ȡһ��ʵʱ����
//���أ�0---��ȡ�ɹ���1---��ȡʧ��


u8 get_current_weather(void)
{
	u8 *p = mymalloc(SRAMIN,40); //����40�ֽ��ڴ�
	u8 res;				
	
	sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",%s",WEATHER_SERVERIP,WEATHER_PORTNUM);    //����Ŀ��TCP������
	
	res = esp8266_send_cmd(p,"OK",200);//���ӵ�Ŀ��TCP������
	if(res==1)
	{
		myfree(SRAMIN,p);
		return 1;
	}
	
	
	vTaskDelay(500);
	esp8266_send_cmd("AT+CIPMODE=1","OK",100);      //����ģʽΪ��͸��	
	esp8266_send_cmd("AT+CIPSEND","OK",100);         //��ʼ͸��
	//printf("start trans...\r\n");

	

	u3_printf("GET https://api.seniverse.com/v3/weather/now.json?key=SPj3iO1_yUJFIMphD&location=jingzhou&language=zh-Hans&unit=c\n\n");	

	vTaskDelay(10);//��ʱ10ms���ص���ָ��ͳɹ���״̬
	
	USART3_RX_STA = 0;
	
	vTaskDelay(500);
	if(USART3_RX_STA != 0)		//��ʱ�ٴνӵ�һ�����ݣ�Ϊ����������
	{ 
		USART3_RX_BUF[USART3_RX_STA]=0;//��ӽ�����
	} 
//		parse_now_weather();
	printf("%s\r\n",USART3_RX_BUF);
	
	USART3_RX_STA = 0;
	
	esp8266_quit_trans();//�˳�͸��
	
	esp8266_send_cmd("AT+CIPCLOSE","OK",50);         //AT+CIPCLOSE�ر�����TCP��UDP���ӣ�
	
	
	myfree(SRAMIN,p);
	return 0;
}



//��ȡ����ʱ��
u8 get_jinzhou_time(void)
{
	u8 *p;
	u8 res;
	
	u8 *resp;
	u8 *p_end;

	p=mymalloc(SRAMIN,40);							//����40�ֽ��ڴ�
	resp=mymalloc(SRAMIN,10);
	p_end=mymalloc(SRAMIN,40);
	
	
	sprintf((char*)p,"AT+CIPSTART=\"TCP\",\"%s\",%s",WEATHER_SERVERIP,WEATHER_PORTNUM);    //����Ŀ��TCP������
	res = esp8266_send_cmd(p,"OK",200);//���ӵ�Ŀ��TCP������
	
		if(res==1)
	{
		myfree(SRAMIN,p);
		return 1;
	}
	
		//�Ƿ���͸��ģʽ  0����ʾ�ر� 1����ʾ����͸��
	esp8266_send_cmd("AT+CIPMODE=1","OK",200);
		USART3_RX_STA = 0;
	
		//͸��ģʽ�� ��ʼ�������ݵ�ָ�� ���ָ��֮��Ϳ���ֱ�ӷ�������
	esp8266_send_cmd("AT+CIPSEND","OK",50);
	
	u3_printf("GET /time15.asp HTTP/1.1\r\nHost:www.beijing-time.org\n\n");
	
	vTaskDelay(20);

	USART3_RX_STA=0;	
	vTaskDelay(500);
	
	if(USART3_RX_STA & 0x8000)
                        {
													resp="Date";
                        USART3_RX_BUF[USART3_RX_STA & 0x7ff] = 0;
													//printf("get_tim_srt��%s\r\n",USART3_RX_BUF);
                        if(strstr((char*)USART3_RX_BUF,(char*)resp)) 
                         {       resp="GMT";
                                p_end = (u8*)strstr((char*)USART3_RX_BUF,(char*)resp);
                                p = p_end - 9; 
                               //printf("get_net_str %s\r\n",p);
                               nwt.hour = ((*p - 0x30)*10 + (*(p+1) - 0x30) + 8) % 24;  //GMT0-->GMT8
													      
                                nwt.min = ((*(p+3) - 0x30)*10 + (*(p+4) - 0x30)) % 60;
													     
                                nwt.sec = ((*(p+6) - 0x30)*10 + (*(p+7) - 0x30)) % 60;
                             
													       nwt.year = ((*(p-5) - 0x30)*1000 + (*(p-4) - 0x30)*100+ (*(p-3) - 0x30)*10+ (*(p-2) - 0x30)); 
                                
													       nwt.date = ((*(p-12) - 0x30)*10 + (*(p-11) - 0x30)); 

                                if        ((u8*)strstr((char*)USART3_RX_BUF,(char*) "Jan")) nwt.month=1; 
                                else if   ((u8*)strstr((char*)USART3_RX_BUF,(char*) "Feb")) nwt.month=2; 
                                else if   ((u8*)strstr((char*)USART3_RX_BUF,(char*) "Mar")) nwt.month=3; 
                                else if   ((u8*)strstr((char*)USART3_RX_BUF,(char*) "Apr")) nwt.month=4; 
                                else if   ((u8*)strstr((char*)USART3_RX_BUF,(char*) "May")) nwt.month=5; 
                                else if   ((u8*)strstr((char*)USART3_RX_BUF,(char*) "Jun")) nwt.month=6; 
                                else if   ((u8*)strstr((char*)USART3_RX_BUF,(char*) "Jul")) nwt.month=7; 
                                else if   ((u8*)strstr((char*)USART3_RX_BUF,(char*) "Aug")) nwt.month=8; 
                                else if   ((u8*)strstr((char*)USART3_RX_BUF,(char*) "Sep")) nwt.month=9; 
                                else if   ((u8*)strstr((char*)USART3_RX_BUF,(char*) "Oct")) nwt.month=10; 
                                else if   ((u8*)strstr((char*)USART3_RX_BUF,(char*) "Nov")) nwt.month=11; 
                                else if   ((u8*)strstr((char*)USART3_RX_BUF,(char*) "Dec")) nwt.month=12;
                                                              
														     
//													       printf("nwt.year = %d\r\n",nwt.year);
//													       printf("nwt.month = %d\r\n",nwt.month);
//  													     printf("nwt.date = %d\r\n",nwt.date);  //��ȡdata 28��
//                   
//													       printf("nwt.hour = %d\r\n",nwt.hour);
//													       printf("nwt.min = %d\r\n",nwt.min);
//													       printf("nwt.sec = %d\r\n",nwt.sec);
                                USART3_RX_STA = 0;
                                 
																 
														//printf("uddate:nettime!!!");
														
                                }
                        USART3_RX_STA = 0;
																
												myfree(SRAMIN,resp);
											  myfree(SRAMIN,p_end);
																
						
                        }               
	printf("\r\n\r\n");
	
	esp8266_quit_trans();//�˳�͸��
	
	esp8266_send_cmd("AT+CIPCLOSE","OK",50);         //AT+CIPCLOSE�ر�����TCP��UDP���ӣ�
												
												
	myfree(SRAMIN,p);
	return 0;
}
