#include "esp8266.h"
#include "string.h"
#include "usart.h"
#include "usart3.h"

#include "FreeRTOS.h"
#include "task.h"

int wifi_flag = 0;

//ESP8266ģ���PC����͸��ģʽ
void esp8266_start_trans(void)
{
	//delay_ms(500);
	//���ù���ģʽ 1��stationģʽ   2��APģʽ  3������ AP+stationģʽ
	esp8266_send_cmd("AT+CWMODE=1","OK",100);
	printf("AT+CWMODE=1 OK\r\n");
	vTaskDelay(50);
	//printf("ok!");
	//��Wifiģ������������
	//esp8266_send_cmd("AT+RST","ready",200);
	u3_printf("AT+RST\r\n");
	printf("AT+RST OK\r\n");
	
	vTaskDelay(1000);         //��ʱ3S�ȴ������ɹ�
	vTaskDelay(1000);
	
	//��ģ���������Լ���·��
	while(esp8266_send_cmd("AT+CWJAP=\"hz\",\"12345678\"","WIFI GOT IP",600));
	printf("WIFI GOT IP OK\r\n");
	vTaskDelay(50);
	
	//=0����·����ģʽ     =1����·����ģʽ
	esp8266_send_cmd("AT+CIPMUX=0","OK",40);
	printf("AT+CIPMUX=0 OK\r\n");
	vTaskDelay(50);
	
	//����TCP����  ������ֱ������ Ҫ���ӵ�ID��0~4   ��������  Զ�̷�����IP��ַ   Զ�̷������˿ں�
	while(esp8266_send_cmd("AT+CIPSTART=\"TCP\",\"api.seniverse.com\",80","CONNECT",400));
	printf("CONNECT OK\r\n");
	wifi_flag = 1;
	vTaskDelay(50);
	
	//�Ƿ���͸��ģʽ  0����ʾ�ر� 1����ʾ����͸��
	esp8266_send_cmd("AT+CIPMODE=1","OK",400);
	vTaskDelay(50);
	
	//͸��ģʽ�� ��ʼ�������ݵ�ָ�� ���ָ��֮��Ϳ���ֱ�ӷ�������
	esp8266_send_cmd("AT+CIPSEND","OK",100);
	
	
}



//ESP8266�˳�͸��ģʽ   ����ֵ:0,�˳��ɹ�;1,�˳�ʧ��
//����wifiģ�飬ͨ����wifiģ����������3��+��ÿ��+��֮�� ����10ms,������Ϊ���������η���+��
u8 esp8266_quit_trans(void)
{
	u8 result=1;
	u3_printf("+++");
	vTaskDelay(1000);					//�ȴ�500ms̫�� Ҫ1000ms�ſ����˳�
	result=esp8266_send_cmd("AT","OK",40);//�˳�͸���ж�.
	if(result)
		printf("quit_trans failed!\r\n");
//	else
//		//printf("quit_trans success!\r\n");
	return result;
}


//��ESP8266��������
//cmd:���͵������ַ���;ack:�ڴ���Ӧ����,���Ϊ��,���ʾ����Ҫ�ȴ�Ӧ��;waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:0,���ͳɹ�(�õ����ڴ���Ӧ����);1,����ʧ��

u8 esp8266_send_cmd(u8 *cmd,u8 *ack,u16 waittime)
{
	u8 res=0; 
	USART3_RX_STA=0;
	u3_printf("%s\r\n",cmd);	//��������
	if(ack&&waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{
			vTaskDelay(10);
			if(USART3_RX_STA&0X8000)//���յ��ڴ���Ӧ����
			{
				if(esp8266_check_cmd(ack))
				{
					//printf("ack:%s\r\n",(u8*)ack);
					break;//�õ���Ч���� 
				}
					USART3_RX_STA=0;
			} 
		}
		if(waittime==0)res=1; 
	}
	return res;
} 


//ESP8266���������,�����յ���Ӧ��
//str:�ڴ���Ӧ����
//����ֵ:0,û�еõ��ڴ���Ӧ����;����,�ڴ�Ӧ������λ��(str��λ��)
u8* esp8266_check_cmd(u8 *str)
{
	char *strx=0;
	if(USART3_RX_STA&0X8000)		//���յ�һ��������
	{ 
		USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//��ӽ�����
		strx=strstr((const char*)USART3_RX_BUF,(const char*)str);
	} 
	return (u8*)strx;
}


//��ESP8266��������
//cmd:���͵������ַ���;waittime:�ȴ�ʱ��(��λ:10ms)
//����ֵ:�������ݺ󣬷������ķ�����֤��
u8* esp8266_send_data(u8 *cmd,u16 waittime)
{
	char temp[5];
	char *ack=temp;
	USART3_RX_STA=0;
	u3_printf("%s",cmd);	//��������
	if(waittime)		//��Ҫ�ȴ�Ӧ��
	{
		while(--waittime)	//�ȴ�����ʱ
		{
			vTaskDelay(10);
			if(USART3_RX_STA&0X8000)//���յ��ڴ���Ӧ����
			{
				USART3_RX_BUF[USART3_RX_STA&0X7FFF]=0;//��ӽ�����
				ack=(char*)USART3_RX_BUF;
				printf("ack:%s\r\n",(u8*)ack);
				USART3_RX_STA=0;
				break;//�õ���Ч���� 
			} 
		}
	}
	return (u8*)ack;
} 


