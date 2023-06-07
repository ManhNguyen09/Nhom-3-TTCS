#include "OLED_IIC.h"

uint8_t FrameBuffer[128][8];

uint8_t ack;

void IIC_Delay(void)
{
	uint8_t i=0;
   while(i--);
}
void IIC_Start(void)
{
	SCL_H;
	SDA_H;
	IIC_Delay();
	SDA_L;
	IIC_Delay();
	SCL_L;
	IIC_Delay();
}
 
void IIC_Stop(void)
{	
	SDA_L;
	SCL_H;
	IIC_Delay();
	SDA_H;
	IIC_Delay();
}

void IIC_Send_Byte(uint8_t byte)
{
	uint8_t i;
	for(i=0;i<8;i++)
	{
		if(byte & 0x80)
		{
			SDA_H;
		}
		else
		{
			SDA_L;
		}
		IIC_Delay();
		SCL_H;
		IIC_Delay();
		SCL_L;
		IIC_Delay();
		byte<<=1;
	}
	SDA_H;  
	IIC_Delay();
	SCL_H;
	IIC_Delay();
	if(SDA_read)
	{
		ack=1;
	}
	else
	{
		ack=0;
	}
	SCL_L;
	IIC_Delay();
}
uint8_t IIC_Write_Byte(uint8_t device_addr,uint8_t register_addr,uint8_t data)
{
	IIC_Start();
	IIC_Send_Byte(device_addr+0);
	if (ack == 1)return 0;
	IIC_Send_Byte(register_addr);   
	if (ack == 1)return 0;
	IIC_Send_Byte(data);
	if (ack == 1)return 0; 
	IIC_Stop();
	return 1;
}

/********************************************
// fill_Picture
********************************************/
void OLED_Fill(uint8_t fill_Data)
{
	uint8_t m,n;
	for(m=0;m<8;m++)
	{
		IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0xb0+m);		//rowe0-rowe1
		IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0x00);		//low column start address
		IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0x10);		//high column start address
		for(n=0;n<128;n++)
		{
			IIC_Write_Byte(OLED_Device_address,OLED_Device_Data,fill_Data);
		}
	}
}

void OLED_Picture(uint8_t *image)
{
  uint8_t x,y;
  for(y=0;y<8;y++)
    {
      IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0xb0+y);
      IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0x0);
      IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0x10);
      for(x=0;x<128;x++)
        {
          IIC_Write_Byte(OLED_Device_address,OLED_Device_Data,*image++);
        }
    }
}

void OLED_FrameBufferRefresh(void)
{
	uint8_t m,n;
	for(m=0;m<8;m++)
	{
		IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0xb0+m);		//rowe0-rowe1
		IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0x00);		//low column start address
		IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0x10);		//high column start address
		for(n=0;n<128;n++)
		{
			IIC_Write_Byte(OLED_Device_address,OLED_Device_Data,FrameBuffer[n][m]);
		}
	}
}

void OLED_DrawPoint(uint16_t x,uint16_t y,uint16_t color)
{
	if(x >= 128 || y >= 64)
		return;
	FrameBuffer[x][y/8] = (color) ? 
												FrameBuffer[x][y/8] | (0x01 << y%8):
												FrameBuffer[x][y/8] & (~(0x01 << y%8));
}


//SSD1306					    
void OLED_Init(void)
{ 	
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0xAE);   //display off
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0x20);	//Set Memory Addressing Mode	
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0x00);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,rowe Addressing Mode (RESET);11,Invalid
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0xb0);	//Set rowe Start Address for rowe Addressing Mode,0-7
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0xc8);	//Set COM Output Scan Direction
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0x00);//---set low column address
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0x10);//---set high column address
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0x40);//--set start line address
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0x81);//--set contrast control register
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0xdf);
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0xa1);//--set segment re-map 0 to 127
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0xa6);//--set normal display
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0xa8);//--set multiplex ratio(1 to 64)
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0x3F);//
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0xa4);//0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0xd3);//-set display offset
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0x00);//-not offset
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0xd5);//--set display clock divide ratio/oscillator frequency
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0xf0);//--set divide ratio
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0xd9);//--set pre-charge period
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0x22); //
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0xda);//--set com pins hardware configuration
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0x12);
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0xdb);//--set vcomh
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0x20);//0x20,0.77xVcc
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0x8d);//--set DC-DC enable
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0x14);//
	IIC_Write_Byte(OLED_Device_address,OLED_Device_Command,0xaf);//--turn on oled panel 
}  















