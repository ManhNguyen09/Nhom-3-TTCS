#ifndef __BLOOD_H
#define __BLOOD_H

#include "stm32f1xx_hal.h"



#define X_HR_TEXT  		2
#define Y_HR_TEXT  		2
#define X_SPO2_TEXT  	2
#define Y_SPO2_TEXT  	18

#define WAVE_MAX_H 		24
#define WAVE_TOP_Y    35
#define WAVE_BOTTOM_Y 60
#define WAVE_START_X 	4
#define WAVE_LEN_X 		124
#define WAVE_BUF_LEN  120
#define WAVE_AVG_FIKN 30
#define WAVE_AVG_SUMN 100

#define HR_CALC_BN 7
#define HR_CALC_CN 3
#define HR_CALC_MINVPP 10
typedef struct
{
	float buf[2][128];
	uint16_t index;
}BufferTypedef;

typedef struct
{
	float val[HR_CALC_BN];
	float tick[HR_CALC_BN];
	uint16_t index_src[HR_CALC_BN];
	uint16_t index;
}PeakPoint;

typedef struct
{
	PeakPoint top;
	PeakPoint btm;
	uint32_t cnt[HR_CALC_BN];
	uint32_t cnterror;
	uint32_t avgcnt;
	uint32_t avgvpp;
}TriggerCounter;

typedef struct
{
	float irbuf[WAVE_AVG_SUMN];
	uint16_t ircur;
	float redbuf[WAVE_AVG_SUMN];
	uint16_t redcur;

	float irsum;
	float redsum;
	float irtop;
	float irtf;
	float redtop;
	float cirdiff;

}SpO2Typedef;


typedef struct
{
	BufferTypedef source;
	BufferTypedef wave;
	TriggerCounter hrcnt;
	SpO2Typedef	SpO2data;
	
	int HeartRate;
	float SpO2;
	
	uint8_t correct;
	uint8_t update;
	
}BloodDataTypedef;


void blood_Setup(void);
void blood_Loop(void);
void blood_Interrupt(void);


#endif /*__BLOOD_H*/
