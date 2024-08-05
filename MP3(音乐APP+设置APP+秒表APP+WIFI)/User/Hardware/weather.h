#ifndef __WEATHER_H
#define __WEATHER_H

#include "stm32f4xx.h" 

u8 get_current_weather(void);
u8 get_3days_weather(void);
void show_weather(void);

u8 get_jinzhou_time(void);


typedef struct   //结构体。
{
    vu16  year;
    vu8   month;
    vu8   date;
    vu8   hour;
    vu8   min;
    vu8   sec;	 
}nt_calendar_obj;	 

extern nt_calendar_obj nwt;  //定义结构体变量


#endif


