#ifndef __SPI_H
#define __SPI_H
#include "stm32f4xx.h"  

void SPI2_Init(void);
void SPI2_SetSpeed(u8 SPI_BaudRatePrescaler);
u8 SPI2_ReadWriteByte(u8 TxData);

#endif
