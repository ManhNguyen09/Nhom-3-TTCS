/**
  ******************************************************************************
  * @file    blood.c
  * @version V1.0.0
  ******************************************************************************
  */
/*--Include-start-------------------------------------------------------------*/
#include "blood.h"
#include "MAX30100.h"
#include "algorithm.h"
#include "math.h"
#include "gui.h"
#include "stdio.h"
#include "beep.h"
/*Global data space ----------------------------------------------------------*/

BloodDataTypedef g_bloData;
/*funcation start ------------------------------------------------------------*/


/*Sensor func -----------------------------------------------------------------*/
void blood_data_update(void)
{
	static DC_FilterData dc1 = {.w = 0,.init = 0,.a = 0.8};
	static DC_FilterData dc2 = {.w = 0,.init = 0,.a = 0.8};
	
	static float data1buf[WAVE_AVG_FIKN];
	static uint16_t data1cur = 0;
	static float data2buf[WAVE_AVG_FIKN];
	static uint16_t data2cur = 0;
	
	uint16_t temp_num=0;
	uint16_t fifo_word_buff[1][2];

	temp_num = max30100_Bus_Read(INTERRUPT_REG);
	
	if (INTERRUPT_REG_A_FULL&temp_num)
	{
		//��ȡFIFO
		max30100_FIFO_Read(0x05,fifo_word_buff,1); //read the hr and spo2 data form fifo in reg=0x05
		
		float data1 = dc_filter(fifo_word_buff[0][0],&dc1)+100.0;
		float data2 = dc_filter(fifo_word_buff[0][1],&dc2)+100.0;
		
		data1buf[data1cur] = data1;
		data2buf[data2cur] = data2;
		
		data1 = 0;
		data2 = 0;
		
		for(int i = 0;i < WAVE_AVG_FIKN;i++)
		{
			data1 += data1buf[i];
			data2 += data2buf[i];
		}

		data1 /= WAVE_AVG_FIKN;
		data2 /= WAVE_AVG_FIKN;
		
		g_bloData.SpO2data.redbuf[g_bloData.SpO2data.redcur] = data1;
		g_bloData.SpO2data.irbuf[g_bloData.SpO2data.ircur] = data2;

		float data3_avg,data4_avg;
		for(int i = 0;i < WAVE_AVG_SUMN;i++)
		{
				data3_avg += g_bloData.SpO2data.irbuf[i];
				data4_avg += g_bloData.SpO2data.redbuf[i];
		}
		
		data3_avg /= WAVE_AVG_SUMN;
		data4_avg /= WAVE_AVG_SUMN;

		data1cur = (data1cur < WAVE_AVG_FIKN - 1) ? data1cur + 1 : 0;
		data2cur = (data2cur < WAVE_AVG_FIKN - 1) ? data2cur + 1 : 0;
		g_bloData.SpO2data.ircur  = (g_bloData.SpO2data.ircur  < WAVE_AVG_SUMN - 1) ?
																g_bloData.SpO2data.ircur  + 1 : 0;
		g_bloData.SpO2data.redcur = (g_bloData.SpO2data.redcur < WAVE_AVG_SUMN - 1) ?
																g_bloData.SpO2data.redcur + 1 : 0;

		g_bloData.SpO2data.irsum  = data3_avg + 50;
		g_bloData.SpO2data.redsum = data4_avg + 50;
		g_bloData.wave.buf[0][g_bloData.wave.index] = data1 + 50;
		g_bloData.wave.buf[1][g_bloData.wave.index] = data2 + 50;
		
		g_bloData.wave.index = (g_bloData.wave.index + 1) % WAVE_BUF_LEN;
		
		g_bloData.update++;
		
	}
}


void blood_data_Calculator(void)
{
	static int lastHeartRate = 0;
	float * pbloBuf = g_bloData.wave.buf[0];
	uint16_t pbloInx = g_bloData.wave.index;
	
	if(g_bloData.wave.index > 3 && g_bloData.update > 0)
	{
		g_bloData.update--;
		//find top
		if(pbloBuf[pbloInx - 3] < pbloBuf[pbloInx - 2] &&
			 pbloBuf[pbloInx - 1] < pbloBuf[pbloInx - 2])
		{
			g_bloData.hrcnt.top.tick[g_bloData.hrcnt.top.index] = HAL_GetTick();
			g_bloData.hrcnt.top.val[g_bloData.hrcnt.top.index ] = pbloBuf[pbloInx - 2];
			g_bloData.hrcnt.top.index_src[g_bloData.hrcnt.top.index] = pbloInx - 2;
			g_bloData.hrcnt.top.index = (g_bloData.hrcnt.top.index + 1) % HR_CALC_BN;

		}		
		
		//find bottom
		if(pbloBuf[pbloInx - 3] > pbloBuf[pbloInx - 2] &&
			 pbloBuf[pbloInx - 1] > pbloBuf[pbloInx - 2])
		{
			g_bloData.hrcnt.btm.tick[g_bloData.hrcnt.btm.index] = HAL_GetTick();
			g_bloData.hrcnt.btm.val[g_bloData.hrcnt.btm.index ] = pbloBuf[pbloInx - 2];
			g_bloData.hrcnt.btm.index_src[g_bloData.hrcnt.btm.index] = pbloInx - 2;
			g_bloData.hrcnt.btm.index = (g_bloData.hrcnt.btm.index + 1) % HR_CALC_BN;
		}	
		//delete error
		if(g_bloData.hrcnt.top.index > (g_bloData.hrcnt.btm.index + 1) || 
			g_bloData.hrcnt.btm.index > (g_bloData.hrcnt.top.index + 1))
		{
			g_bloData.hrcnt.top.index = 0;
			g_bloData.hrcnt.btm.index = 0;
		}
		
		//Calculate heart rate
		if(	g_bloData.hrcnt.top.index == 0 && 
				g_bloData.hrcnt.btm.index == 0 )
		{
			uint32_t x1,x2,ucnt = 0;
			float maxerror = 0;
			
			//find Vtop
			for(int i = 0;i < HR_CALC_BN;i++)
			{
				x1 = g_bloData.hrcnt.top.tick[i];
				x2 = g_bloData.hrcnt.top.tick[i + 1];
				g_bloData.hrcnt.cnt[i] = (x2 > x1) ? x2 - x1 : x1 - x2;
			}
			for(int i = 0;i < HR_CALC_CN;i++)
			{
				ucnt+=g_bloData.hrcnt.cnt[i];
			}
			ucnt /= HR_CALC_CN;
			g_bloData.hrcnt.avgcnt = ucnt;
			
			//find Vbottom
			g_bloData.hrcnt.cnterror = 0;
			for(int i = 0;i < HR_CALC_CN;i++)
			{
				uint32_t cntdiff,ncnt;
				ncnt = g_bloData.hrcnt.cnt[i];
				cntdiff = (ucnt > ncnt) ? ucnt - ncnt : ncnt - ucnt;
				
				if(g_bloData.hrcnt.cnterror < (cntdiff))
				{
					g_bloData.hrcnt.cnterror = cntdiff;
				}
			}
			//Calc Vpp
			float vpp = 0;

			for(int i = 0; i < HR_CALC_BN;i++)
			{
				vpp += (g_bloData.hrcnt.top.val[i] > g_bloData.hrcnt.btm.val[i]) ? 
									(g_bloData.hrcnt.top.val[i] - g_bloData.hrcnt.btm.val[i]) :
									(g_bloData.hrcnt.btm.val[i] - g_bloData.hrcnt.top.val[i]) ;
				
			}
			vpp /= 6;
			g_bloData.hrcnt.avgvpp = vpp;
			
			//check error and vpp value
			if((g_bloData.hrcnt.cnterror < 200) && (HR_CALC_MINVPP < vpp))
			{
				// heart rate
				g_bloData.correct = 1;
				g_bloData.HeartRate = 60 * 1000 / ucnt ;
				lastHeartRate = g_bloData.HeartRate;

				//SpO2
				//float irtop = 0,redtop = 0;
				g_bloData.SpO2data.irtop = 0;
				g_bloData.SpO2data.redtop = 0;
				for(int i = 0;i < 3;i++)
				{
					uint16_t ind = g_bloData.hrcnt.top.index_src[i];
					g_bloData.SpO2data.redtop  += g_bloData.wave.buf[0][ind];
					g_bloData.SpO2data.irtop  += g_bloData.wave.buf[1][ind];
				}
				g_bloData.SpO2data.irtop /= 3;
				g_bloData.SpO2data.redtop  /= 3;
				g_bloData.SpO2data.cirdiff = (g_bloData.SpO2data.redsum - g_bloData.SpO2data.irsum);
				g_bloData.SpO2data.irtf = g_bloData.SpO2data.irtop + g_bloData.SpO2data.cirdiff;

				if(g_bloData.SpO2data.redtop > g_bloData.SpO2data.irtf)
				{
					g_bloData.SpO2 = 100 * log(g_bloData.SpO2data.redtop) /  log(g_bloData.SpO2data.irtf);
				}
			}
			else
			{
				if((HR_CALC_MINVPP < vpp))
				{
					g_bloData.SpO2 = 0;
					g_bloData.correct = 0;
					g_bloData.HeartRate = 0;
					g_bloData.SpO2 = 0;
				}
			}
		}
	}
}

/*tft display ---------------------------------------------------------------*/
void tft_draw_wave(void)
{
	uint16_t ulineHigh = g_bloData.wave.buf[0][g_bloData.wave.index - 1] / 10;
	ulineHigh = (ulineHigh > WAVE_MAX_H ) ? WAVE_MAX_H : ulineHigh;
	uint16_t ux_n = WAVE_START_X + g_bloData.wave.index;
	uint16_t uy_n = WAVE_BOTTOM_Y - ulineHigh;
	

	Gui_DrawLine(ux_n,WAVE_BOTTOM_Y - WAVE_MAX_H,ux_n,WAVE_BOTTOM_Y,BLACK);
	Gui_DrawLine(ux_n + 1,WAVE_BOTTOM_Y - WAVE_MAX_H,ux_n + 1,WAVE_BOTTOM_Y,WHITE);

	Gui_DrawLine(ux_n,uy_n,ux_n,WAVE_BOTTOM_Y,WHITE);
	
}


void tft_draw_hrsp(void)
{
	uint8_t str[50];
	
	sprintf((char *)str,"HR:%3d  ",g_bloData.HeartRate);
	Gui_DrawFont_GBK16(X_HR_TEXT,Y_HR_TEXT,0xffe0,0,str);

	g_bloData.SpO2 = (g_bloData.SpO2 > 99.99) ? 99.99:g_bloData.SpO2;
	sprintf((char *)str,"SP:%2.2f%%  ",g_bloData.SpO2);
	Gui_DrawFont_GBK16(X_SPO2_TEXT,Y_SPO2_TEXT,0x07ff,0,str);
}



void tft_draw_windows(void)
{
	gui_draw_square(0,1,127,63,WHITE);
	gui_draw_square(WAVE_START_X - 2,WAVE_TOP_Y,WAVE_LEN_X + 1,WAVE_BOTTOM_Y + 1,WHITE);
}


void tft_display_update(void)
{
	tft_draw_wave();
	tft_draw_hrsp();
}
/*Setup and loop func -----------------------------------------------------*/

void blood_Setup(void)
{
	tft_draw_windows();
	settone(14);
}

void blood_Loop(void)
{
	blood_data_update();
	blood_data_Calculator();
	
	tft_display_update();
	
	OLED_FrameBufferRefresh();
	
}

void blood_Interrupt(void)
{
	static int16_t div = 0;
	static int16_t div2 = 0;
	div++;
	if(div > 10)
	{
		div = 0;
		//blood_data_update();
	}
	
	div2++;
	if(div > 50)
	{
		div2 = 0;
		//tft_display_update();
	}
}


