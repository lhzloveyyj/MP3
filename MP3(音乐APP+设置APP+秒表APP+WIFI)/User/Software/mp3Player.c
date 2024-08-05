/*
******************************************************************************
* @file    mp3Player.c
* @author  fire
* @version V1.0
* @date    2023-08-13
* @brief   mp3解码
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

/* 推荐使用以下格式mp3文件：
 * 采样率：44100Hz
 * 声  道：2
 * 比特率：320kbps
 */

/* 处理立体声音频数据时，输出缓冲区需要的最大大小为2304*16/8字节(16为PCM数据为16位)，
 * 这里我们定义MP3BUFFER_SIZE为2304
 */
 
extern u16 volume;
extern QueueHandle_t music_Queue;
 
#define MP3BUFFER_SIZE  2304
#define INPUTBUF_SIZE   4096

static HMP3Decoder		Mp3Decoder;			/* mp3解码器指针	*/
static MP3FrameInfo		Mp3FrameInfo;		/* mP3帧信息  */
static MP3_TYPE mp3player;            /* mp3播放设备 */
volatile uint8_t Isread = 0;          /* DMA传输完成标志 */
volatile uint8_t dac_ht = 0;          //DAC dma 半传输标志

uint32_t led_delay = 0;

uint8_t inputbuf[INPUTBUF_SIZE]={0};     /* 解码输入缓冲区，1940字节为最大MP3帧大小  */
static short outbuffer[MP3BUFFER_SIZE];  /* 解码输出缓冲区*/

static FIL file;			/* file objects */
static UINT bw;       /* File R/W count */
FRESULT result; 

//从SD卡读取MP3源文件进行解码，并传入DAC缓冲区
int MP3DataDecoder(uint8_t **read_ptr, int *bytes_left)
{
	int err = 0, i = 0, outputSamps = 0;

	//bufflag开始解码 参数：mp3解码结构体、输入流指针、输入流大小、输出流指针、数据格式
	err = MP3Decode(Mp3Decoder, read_ptr, bytes_left, outbuffer, 0);
	
	if (err != ERR_MP3_NONE)	//错误处理
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
							// 跳过此帧
							if (*bytes_left > 0)
							{
								(*bytes_left) --;
								read_ptr ++;
							}
				break;
		}
		return 0;
	}
	else		//解码无错误，准备把数据输出到PCM
	{
		MP3GetLastFrameInfo(Mp3Decoder, &Mp3FrameInfo);		//获取解码信息				
		/* 输出到DAC */
		outputSamps = Mp3FrameInfo.outputSamps;						//PCM数据个数
		if (outputSamps > 0)
		{
			if (Mp3FrameInfo.nChans == 1)	//单声道
			{
				//单声道数据需要复制一份到另一个声道
				for (i = outputSamps - 1; i >= 0; i--)
				{
					outbuffer[i * 2] = outbuffer[i];
					outbuffer[i * 2 + 1] = outbuffer[i];
				}
				outputSamps *= 2;
			}//if (Mp3FrameInfo.nChans == 1)	//单声道
		}//if (outputSamps > 0)
					
		//将数据传送至DMA DAC缓冲区
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
	}//else 解码正常
}

//读取一段MP3数据，并把读取的指针赋值read_ptr，长度赋值bytes_left
uint8_t read_file(const char *mp3file, uint8_t **read_ptr, int *bytes_left)
{
	result = f_read(&file, inputbuf, INPUTBUF_SIZE, &bw);
	
	if(result != FR_OK)
	{
		printf("读取%s失败 -> %d\r\n", mp3file, result);
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
  * @brief  MP3格式音频播放主程序
  * @param  mp3file MP3文件路径
  * @retval 无
  */
void mp3PlayerDemo(const char **mp3file,u8 *n)
{
	u8 key;
	
	uint8_t *read_ptr = inputbuf;
	int	read_offset = 0;				/* 读偏移指针 */
	int	bytes_left = 0;					/* 剩余字节数 */	
	
	mp3player.ucStatus = STA_IDLE;
	mp3player.ucVolume = volume; //音量值，100满
	
	//尝试打开MP3文件
	result = f_open(&file, * (mp3file + *n) , FA_READ);
	if(result != FR_OK)
	{
		printf("Open mp3file :%s fail!!!->%d\r\n", * (mp3file + *n)  , result);
		result = f_close (&file);
		return;	/* 停止播放 */
	}
	printf("当前播放文件 -> %s\n", * (mp3file + *n));
	
	//初始化MP3解码器
	Mp3Decoder = MP3InitDecoder();	
	if(Mp3Decoder == 0)
	{
		printf("初始化helix解码库设备失败！\r\n");
		return;	/* 停止播放 */
	}
	else
	{
		printf("初始化helix解码库完成\r\n");
	}
	
	//尝试读取一段MP3数据，并把读取的指针赋值read_ptr，长度赋值bytes_left
	if(!read_file(* (mp3file + *n), &read_ptr, &bytes_left))
	{
		MP3FreeDecoder(Mp3Decoder);
		return;	/* 停止播放 */
	}
	
	//尝试解码成功
	if(MP3DataDecoder(&read_ptr, &bytes_left))
	{
		//打印MP3信息
		printf(" \r\n Bitrate       %dKbps", Mp3FrameInfo.bitrate/1000);
		printf(" \r\n Samprate      %dHz",   Mp3FrameInfo.samprate);
		printf(" \r\n BitsPerSample %db",    Mp3FrameInfo.bitsPerSample);
		printf(" \r\n nChans        %d",     Mp3FrameInfo.nChans);
		printf(" \r\n Layer         %d",     Mp3FrameInfo.layer);
		printf(" \r\n Version       %d",     Mp3FrameInfo.version);
		printf(" \r\n OutputSamps   %d",     Mp3FrameInfo.outputSamps);
		printf("\r\n");
		
		//启动DAC，开始发声
		if (Mp3FrameInfo.nChans == 1)	//单声道要将outputSamps*2
		{
			DAC_DMA_Start(Mp3FrameInfo.samprate, 2 * Mp3FrameInfo.outputSamps);
		}
		else//双声道直接用Mp3FrameInfo.outputSamps
		{
			DAC_DMA_Start(Mp3FrameInfo.samprate, Mp3FrameInfo.outputSamps);
		}
	}
	else //解码失败
	{
		MP3FreeDecoder(Mp3Decoder);
		return;
	}
	
	/* 放音状态 */
	mp3player.ucStatus = STA_PLAYING;
	
	
	/* 进入主程序循环体 */
	while(mp3player.ucStatus == STA_PLAYING)
	{
		
		if(music_Queue!=NULL)
		{
			if(xQueueReceive(music_Queue,&key,0))
			{
				if((key == 2) && (*n < 26))
				{
					DAC_DMA_Stop();//停止喂DAC数据	
					mp3player.ucStatus = STA_IDLE;
					MP3FreeDecoder(Mp3Decoder);//清理缓存
					f_close(&file);
					key=0;					
					break;
				}
				
				else if((key == 1) && (*n >= 0))
				{
					DAC_DMA_Stop();//停止喂DAC数据	
					mp3player.ucStatus = STA_IDLE;
					MP3FreeDecoder(Mp3Decoder);//清理缓存
					f_close(&file);	
					key=0;	
					break;
				}
				
			}
		}
			mp3player.ucVolume = volume; //音量值，100满
			//寻找帧同步，返回第一个同步字的位置
			read_offset = MP3FindSyncWord(read_ptr, bytes_left);
			if(read_offset < 0)					//没有找到同步字
			{
				if(!read_file(* (mp3file + *n), &read_ptr, &bytes_left))//重新读取一次文件再找
				{
					continue;//回到while(mp3player.ucStatus == STA_PLAYING)后面
				}
			}
			
			else//找到同步字
			{			
				read_ptr   += read_offset;	//偏移至同步字的位置
				bytes_left -= read_offset;	//同步字之后的数据大小	
				
				if(bytes_left < 1024)				//如果剩余的数据小于1024字节，补充数据
				{
					/* 注意这个地方因为采用的是DMA读取，所以一定要4字节对齐  */
					u16 i = (uint32_t)(bytes_left)&3;	//判断多余的字节
					if(i) i=4-i;						//需要补充的字节
					memcpy(inputbuf+i, read_ptr, bytes_left);	//从对齐位置开始复制
					read_ptr = inputbuf+i;										//指向数据对齐位置
					result = f_read(&file, inputbuf+bytes_left+i, INPUTBUF_SIZE-bytes_left-i, &bw);//补充数据
					if(result != FR_OK)
					{
						printf("读取%s失败 -> %d\r\n",*mp3file,result);
						break;
					}
					bytes_left += bw;		//有效数据流大小
				}
			}
			

			
			//MP3数据解码并送入DAC缓存
			if(!MP3DataDecoder(&read_ptr, &bytes_left))
			{//如果播放出错，Isread置1，避免卡住死循环
				Isread = 1;
			}
			
			//mp3文件读取完成，退出
			if(file.fptr == file.fsize)
			{
				printf("单曲播放完毕\r\n");
				if(*n<26)
					*n +=1;
				break;
			}	

			//等待DAC发送一半或全部中断
			while(Isread == 0)
			{
				led_delay++;
				if(led_delay == 0xffffff)
				{
					led_delay=0;
					LED1_TROG;
				}
				//Input_scan();		//等待DMA传输完成，此间可以运行按键扫描及处理事件
			}
			Isread = 0;
	}
	//运行到此处，说明单曲播放完成，收尾工作
	DAC_DMA_Stop();//停止喂DAC数据	
	mp3player.ucStatus = STA_IDLE;
	MP3FreeDecoder(Mp3Decoder);//清理缓存
	f_close(&file);	
}

void DMA1_Stream6_IRQHandler(void)
{
	if(DMA_GetITStatus(DMA1_Stream6, DMA_IT_HTIF6) != RESET) //半传输
	{	
		dac_ht = 1;		
		Isread=1;
		
    DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_HTIF6);
  }
	
	if(DMA_GetITStatus(DMA1_Stream6, DMA_IT_TCIF6) != RESET) //全传输
	{
		dac_ht = 0;
		Isread=1;
		
    DMA_ClearITPendingBit(DMA1_Stream6, DMA_IT_TCIF6);
  }
}

/***************************** (END OF FILE) *********************************/
