#ifndef _OLED_IIC_H
#define _OLED_IIC_H


#include "stm32f1xx_hal.h"                  // Device header
#include "IICcom.h"
#include <stdarg.h>
#include <stdio.h>
#define OLED_Device_address 0x78
#define OLED_Device_Command 0x00
#define OLED_Device_Data 		0x40

void OLED_Fill(uint8_t fill_Data);
void OLED_Picture(uint8_t *image);
void OLED_FrameBufferRefresh(void);
void OLED_DrawPoint(uint16_t x,uint16_t y,uint16_t color);
void OLED_Init(void);

#endif
