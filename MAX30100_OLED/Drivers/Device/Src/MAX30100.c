/**
  ******************************************************************************
  * @file    MAX30100.c
  * @version V1.0.0
  ******************************************************************************
  */
/*--Include-start-------------------------------------------------------------*/
#include "MAX30100.h"
#include "IICcom.h"

/*define ---------------------------------------------------------------------*/
#define max30100_WR_address 0xAE

#define I2C_WR	0	
#define I2C_RD	1	


/*Global data space ----------------------------------------------------------*/

uint8_t max30100_Bus_Write(uint8_t Register_Address, uint8_t Word_Data)
{

	
	I2C_Start();

	I2C_SendByte(max30100_WR_address | I2C_WR);	

	if (I2C_WaitAck() != 0)
	{
		goto cmd_fail;	
	}

	I2C_SendByte(Register_Address);
	if (I2C_WaitAck() != 0)
	{
		goto cmd_fail;	
	}
	

	I2C_SendByte(Word_Data);

	
	if (I2C_WaitAck() != 0)
	{
		goto cmd_fail;	
	}

	I2C_Stop();
	return 1;	

cmd_fail: 
	I2C_Stop();
	return 0;
}


uint8_t max30100_Bus_Read(uint8_t Register_Address)
{
	uint8_t  data;


	
	I2C_Start();

	I2C_SendByte(max30100_WR_address | I2C_WR);	

	if (I2C_WaitAck() != 0)
	{
		goto cmd_fail;	
	}

	I2C_SendByte((uint8_t)Register_Address);
	if (I2C_WaitAck() != 0)
	{
		goto cmd_fail;	
	}
	

	I2C_Start();

	I2C_SendByte(max30100_WR_address | I2C_RD);	


	if (I2C_WaitAck() != 0)
	{
		goto cmd_fail;	
	}

	{
		data = I2C_RadeByte();	

		I2C_NoAck();	
	}
	I2C_Stop();
	return data;	

cmd_fail: 
	I2C_Stop();
	return 0;
}

void max30100_FIFO_Read(uint8_t Register_Address,uint16_t  Word_Data[][2],uint8_t count)
{
	uint8_t i=0;
	uint8_t no = count;
	uint8_t data1, data2;
	I2C_Start();

	I2C_SendByte(max30100_WR_address | I2C_WR);	


	if (I2C_WaitAck() != 0)
	{
		goto cmd_fail;	
	}

	I2C_SendByte((uint8_t)Register_Address);
	if (I2C_WaitAck() != 0)
	{
		goto cmd_fail;	
	}
	

	I2C_Start();

	I2C_SendByte(max30100_WR_address | I2C_RD);

	if (I2C_WaitAck() != 0)
	{
		goto cmd_fail;	
	}

	while (no)
	{
		data1 = I2C_RadeByte();	
		I2C_Ack();
		data2 = I2C_RadeByte();
		I2C_Ack();
		Word_Data[i][0] = (((uint16_t)data1 << 8) | data2); 

		
		data1 = I2C_RadeByte();	
		I2C_Ack();
		data2 = I2C_RadeByte();
		if(1==no)
			I2C_NoAck();	
		else
			I2C_Ack();
		Word_Data[i][1] = (((uint16_t)data1 << 8) | data2); 

		no--;	
		i++;
	}
	I2C_Stop();

cmd_fail: 
	I2C_Stop();
}

void max30100_init(void)
{
	max30100_Bus_Write(0x06, 0x0b);  //mode configuration : temp_en[3]      MODE[2:0]=010 HR only enabled    011 SP02 enabled
	//max30100_Bus_Write(0x06, 0x0a);  //MODE[2:0]=010 HR only enabled     when used is mode ,the red led is not used.
	max30100_Bus_Write(0x01, 0xF0); //open all of interrupt
	max30100_Bus_Write(INTERRUPT_REG, 0x00); //all interrupt clear
	max30100_Bus_Write(0x09, 0x33); //r_pa=3,ir_pa=3

#if (SAMPLES_PER_SECOND == 50)
	max30100_Bus_Write(0x07, 0x43); //SPO2_SR[4:2]=000   50 per second    LED_PW[1:0]=11  16BITS
#elif (SAMPLES_PER_SECOND == 100)
	max30100_Bus_Write(0x07, 0x47); //SPO2_SR[4:2]=001  100 per second    LED_PW[1:0]=11  16BITS
#elif (SAMPLES_PER_SECOND == 200)
	max30100_Bus_Write(0x07, 0x4F); 
#elif (SAMPLES_PER_SECOND == 400)
	max30100_Bus_Write(0x07, 0x53); 
#endif
	
	max30100_Bus_Write(0x02, 0x00); //set FIFO write Pointer reg = 0x00 for clear it
	max30100_Bus_Write(0x03, 0x00);	//set Over Flow Counter  reg = 0x00 for clear it
	max30100_Bus_Write(0x04, 0x0F);	//set FIFO Read Pointer  reg = 0x0f for   
											//waitting  write pointer eq read pointer   to   interrupts  INTERRUPT_REG_A_FULL
}


