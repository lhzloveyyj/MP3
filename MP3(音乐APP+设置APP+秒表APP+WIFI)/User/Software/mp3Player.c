/*
******************************************************************************
* @file    mp3Player.c
* @author  fire
* @version V1.0
* @date    2023-08-13
* @brief   mp3����
******************************************************************************
*/
#include <stdio.h>
#include <string.h>
#include "ff.h" 
#include "mp3Player.h"
#include "mp3dec.h"
#include "dac.h"
#include "led.h"

#include "FreeRTOS.h"
#include "queue.h"

/* �Ƽ�ʹ�����¸�ʽmp3�ļ���
 * �����ʣ�44100Hz
 * ��  ����2
 * �����ʣ�320kbps
 */

/* ������������Ƶ����ʱ�������������Ҫ������СΪ2304*16/8�ֽ�(16ΪPCM����Ϊ16λ)��
 * �������Ƕ���MP3BUFFER_SIZEΪ2304
 */
 
extern u16 volume;
extern QueueHandle_t music_Queue;
 
#define MP3BUFFER_SIZE  2304
#define INPUTBUF_SIZE   4096

static HMP3Decoder		Mp3Decoder;			/* mp3������ָ��	*/
static MP3FrameInfo		Mp3FrameInfo;		/* mP3֡��Ϣ  */
static MP3_TYPE mp3player;            /* mp3�����豸 */
volatile uint8_t Isread = 0;          /* DMA������ɱ�־ */
volatile uint8_t dac_ht = 0;          //DAC dma �봫���־

uint32_t led_delay = 0;

uint8_t inputbuf[INPUTBUF_SIZE]={0};     /* �������뻺������1940�ֽ�Ϊ���MP3֡��С  */
static short outbuffer[MP3BUFFER_SIZE];  /* �������������*/

static FIL file;			/* file objects */
static UINT bw;       /* File R/W count */
FRESULT result; 

//��SD����ȡMP3Դ�ļ����н��룬������DAC������
int MP3DataDecoder(uint8_t **read_ptr, int *bytes_left)
{
	int err = 0, i = 0, outputSamps = 0;

	//bufflag��ʼ���� ������mp3����ṹ�塢������ָ�롢��������С�������ָ�롢���ݸ�ʽ
	err = MP3Decode(Mp3Decoder, read_ptr, bytes_left, outbuffer, 0);
	
	if (err != ERR_MP3_NONE)	//������
	{
		switch (err)
		{
			case ERR_MP3_INDATA_UNDERFLOW:
							printf("ERR_MP3_INDATA_UNDERFLOW\r\n");
							result = f_read(&file, inputbuf, INPUTBUF_SIZE, &bw);
							*read_ptr = inputbuf;
							*bytes_left = bw;
				break;		
			case ERR_MP3_MAINDATA_UNDERFLOW:
							/* do nothing - next call to decode will provide more mainData */
							printf("ERR_MP3_MAINDATA_UNDERFLOW\r\n");
				break;		
			default:
							printf("UNKNOWN ERROR:%d\r\n", err);		
							// ������֡
							if (*bytes_left > 0)
							{
								(*bytes_left) --;
								read_ptr ++;
							}
				break;
		}
		return 0;
	}
	else		//�����޴���׼�������������PCM
	{
		MP3GetLastFrameInfo(Mp3Decoder, &Mp3FrameInfo);		//��ȡ������Ϣ				
		/* �����DAC */
		outputSamps = Mp3FrameInfo.outputSamps;						//PCM���ݸ���
		if (outputSamps > 0)
		{
			if (Mp3FrameInfo.nChans == 1)	//������
			{
				//������������Ҫ����һ�ݵ���һ������
				for (i = outputSamps - 1; i >= 0; i--)
				{
					outbuffer[i * 2] = outbuffer[i];
					outbuffer[i * 2 + 1] = outbuffer[i];
				}
				outputSamps *= 2;
			}//if (Mp3FrameInfo.nChans == 1)	//������
		}//if (outputSamps > 0)
					
		//�����ݴ�����DMA DAC������
		for (i = 0; i < outputSamps/2; i++)
		{
			if(dac_ht == 1)
			{
				DAC_buff[0][i] = outbuffer[2*i] * mp3player.ucVolume /100 + 32768;
				DAC_buff[1][i] = outbuffer[2*i+1] * mp3player.ucVolume /100 + 32768;
			}
			else
			{
				DAC_buff[0][i+outputSamps/2] = outbuffer[2*i] * mp3player.ucVolume /100 + 32768;
				DAC_buff[1][i+outputSamps/2] = outbuffer[2*i+1] * mp3player.ucVolume /100 + 32768;
			}
		}
		
		return 1;
	}//else ��������
}

//��ȡһ��MP3���ݣ����Ѷ�ȡ��ָ�븳ֵread_ptr�����ȸ�ֵbytes_left
uint8_t read_file(const char *mp3file, uint8_t **read_ptr, int *bytes_left)
{
	result = f_read(&file, inputbuf, INPUTBUF_SIZE, &bw);
	
	if(result != FR_OK)
	{
		printf("��ȡ%sʧ�� -> %d\r\n", mp3file, result);
		return 0;
	}
	else
	{
		*read_ptr = inputbuf;
		*bytes_left = bw;
		
		return 1;
	}
}

/**
  * @brief  MP3��ʽ��Ƶ����������
  * @param  mp3file MP3�ļ�·��
  * @retval ��
  */
void mp3PlayerDemo(const char **mp3file,u8 *n)
{
	u8 key;
	
	uint8_t *read_ptr = inputbuf;
	int	read_offset = 0;				/* ��ƫ��ָ�� */
	int	bytes_left = 0;					/* ʣ���ֽ��� */	
	
	mp3player.ucStatus = STA_IDLE;
	mp3player.ucVolume = volume; //����ֵ��100��
	
	//���Դ�MP3�ļ�
	result = f_open(&file, * (mp3file + *n) , FA_READ);
	if(result != FR_OK)
	{
		printf("Open mp3file :%s fail!!!->%d\r\n", * (mp3file + *n)  , result);
		result = f_close (&file);
		return;	/* ֹͣ���� */
	}
	printf("��ǰ�����ļ� -> %s\n", * (mp3file + *n));
	
	//��ʼ��MP3������
	Mp3Decoder = MP3InitDecoder();	
	if(Mp3Decoder == 0)
	{
		printf("��ʼ��helix������豸ʧ�ܣ�\r\n");
		return;	/* ֹͣ���� */
	}
	else
	{
		printf("��ʼ��helix��������\r\n");
	}
	
	//���Զ�ȡһ��MP3���ݣ����Ѷ�ȡ��ָ�븳ֵread_ptr�����ȸ�ֵbytes_left
	if(!read_file(* (mp3file + *n), &read_ptr, &bytes_left))
	{
		MP3FreeDecoder(Mp3Decoder);
		return;	/* ֹͣ���� */
	}
	
	//���Խ���ɹ�
	if(MP3DataDecoder(&read_ptr, &bytes_left))
	{
		//��ӡMP3��Ϣ
		printf(" \r\n Bitrate       %dKbps", Mp3FrameInfo.bitrate/1000);
		printf(" \r\n Samprate      %dHz",   Mp3FrameInfo.samprate);
		printf(" \r\n BitsPerSample %db",    Mp3FrameInfo.bitsPerSample);
		printf(" \r\n nChans        %d",     Mp3FrameInfo.nChans);
		printf(" \r\n Layer         %d",     Mp3FrameInfo.layer);
		printf(" \r\n Version       %d",     Mp3FrameInfo.version);
		printf(" \r\n OutputSamps   %d",     Mp3FrameInfo.outputSamps);
		printf("\r\n");
		
		//����DAC����ʼ����
		if (Mp3FrameInfo.nChans == 1)	//������Ҫ��outputSamps*2
		{
			DAC_DMA_Start(Mp3FrameInfo.samprate, 2 * Mp3FrameInfo.outputSamps);
		}
		else//˫����ֱ����Mp3FrameInfo.outputSamps
		{
			DAC_DMA_Start(Mp3FrameInfo.samprate, Mp3FrameInfo.outputSamps);
		}
	}
	else //����ʧ��
	{
		MP3FreeDecoder(Mp3Decoder);
		return;
	}
	
	/* ����״̬ */
	mp3player.ucStatus = STA_PLAYING;
	
	
	/* ����������ѭ���� */
	while(mp3player.ucStatus == STA_PLAYING)
	{
		
		if(music_Queue!=NULL)
		{
			if(xQueueReceive(music_Queue,&key,0))
			{
				if((key == 2) && (*n < 26))
				{
					DAC_DMA_Stop();//ֹͣιDAC����	
					mp3player.ucStatus = STA_IDLE;
					MP3FreeDecoder(Mp3Decoder);//������
					f_close(&file);
					key=0;					
					break;
				}
				
				else if((key == 1) && (*n >= 0))
				{
					DAC_DMA_Stop();//ֹͣιDAC����	
					mp3player.ucStatus = STA_IDLE;
					MP3FreeDecoder(Mp3Decoder);//������
					f_close(&file);	
					key=0;	
					break;
				}
				
			}
		}
			mp3player.ucVolume = volume; //����ֵ��100��
			//Ѱ��֡ͬ�������ص�һ��ͬ���ֵ�λ��
			read_offset = MP3FindSyncWord(read_ptr, bytes_left);
			if(read_offset < 0)					//û���ҵ�ͬ����
			{
				if(!read_file(* (mp3file + *n), &read_ptr, &bytes_left))//���¶�ȡһ���ļ�����
				{
					continue;//�ص�while(mp3player.ucStatus == STA_PLAYING)����
				}
			}
			
			else//�ҵ�ͬ����
			{			
				read_ptr   += read_offset;	//ƫ����ͬ���ֵ�λ��
				bytes_left -= read_offset;	//ͬ����֮������ݴ�С	
				
				if(bytes_left < 1024)				//���ʣ�������С��1024�ֽڣ���������
				{
					/* ע������ط���Ϊ���õ���DMA��ȡ������һ��Ҫ4�ֽڶ���  */
					u16 i = (uint32_t)(bytes_left)&3;	//�ж϶�����ֽ�
					if(i) i=4-i;						//��Ҫ������ֽ�
					memcpy(inputbuf+i, read_ptr, bytes_left);	//�Ӷ���λ�ÿ�ʼ����
					read_ptr = inputbuf+i;										//ָ�����ݶ���λ��
					result = f_read(&file, inputbuf+bytes_left+i, INPUTBUF_SIZE-bytes_left-i, &bw);//��������
					if(result != FR_OK)
					{
						printf("��ȡ%sʧ�� -> %d\r\n",*mp3file,result);
						break;
					}
					bytes_left += bw;		//��Ч��������С
				}
			}
			

			
			//MP3���ݽ��벢����DAC����
			if(!MP3DataDecoder(&read_ptr, &bytes_left))
			{//������ų���Isread��1�����⿨ס��ѭ��
				Isread = 1;
			}
			
			//mp3�ļ���ȡ��ɣ��˳�
			if(file.fptr == file.fsize)
			{
				printf("�����������\r\n");
				if(*n<26)
					*n +=1;
				break;
			}	

			//�ȴ�DAC����һ���ȫ���ж�
			while(Isread == 0)
			{
				led_delay++;
				if(led_delay == 0xffffff)
				{
					led_delay=0;
					LED1_TROG;
				}
				//Input_scan();		//�ȴ�DMA������ɣ��˼�������а���ɨ�輰�����¼�
			}
			Isread = 0;
	}
	//���е��˴���˵������������ɣ���β����
	DAC_DMA_Stop();//ֹͣιDAC����	
	mp3player.ucStatus = STA_IDLE;
	MP3FreeDecoder(Mp3Decoder);//������
	f_close(&file);	
}

void DMA1_Stream6_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_Stream6, DMA_IT_HTIF6) != RESET) //�봫��
	{	
		dac_ht = 1;		
		Isread=1;
		
    DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_HTIF6);
  }
	
	if(DMA_GetITStatus(DMA1_Stream6, DMA_IT_TCIF6) != RESET) //ȫ����
	{
		dac_ht = 0;
		Isread=1;
		
    DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TCIF6);
  }
}

/***************************** (END OF FILE) *********************************/
