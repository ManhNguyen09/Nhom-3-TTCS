/**
  ******************************************************************************
  * @file    beep.c
  * @version V1.0.0
  ******************************************************************************
  */
/*--Include-start-------------------------------------------------------------*/
#include "beep.h"
#include "stm32f1xx_hal.h"
#include "tim.h"

/*Global data space ----------------------------------------------------------*/

const uint16_t FREQ[21] = {
		
		262 ,294 ,330 ,349 ,392 ,440 ,494 ,
		523 ,587 ,659 ,698 ,784 ,880 ,988 ,
		1047,1175,1319,1397,1568,1760,1776,
	
};


uint16_t g_tone = 0;




/**
  * @brief  PWM.
  * @param  freq pwm.
  * @param  duty pwm.
  * @retval none.
  */
void set_pwm_value(uint16_t freq)
{
	htim2.Init.Period = 1000000/freq;
	HAL_TIM_PWM_Init(&htim2);
}


void Led_Control(int pl)
{
	if(pl >= 0 && pl < 100)
	{	
		TIM2->CCR2 = 10000 / (FREQ[g_tone]) * pl;
	}
}

void Beep_Control(int pl)
{
	if(pl >= 0 && pl < 100)
	{	
		TIM2->CCR1 = 10000 / (FREQ[g_tone]) * pl;
	}
}


void settone(int t)
{
	if(t >= 0 && t < 21)
	{	
		g_tone = t;
		set_pwm_value((FREQ[g_tone]));
	}
}
















