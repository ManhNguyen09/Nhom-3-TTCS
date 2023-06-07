#ifndef __ALGORITHM_H
#define __ALGORITHM_H

#define FFT_N 				1024  
#define START_INDEX 	10  

struct compx     
{
	float real;
	float imag;
};  


double my_floor(double x);

double my_fmod(double x, double y);

double XSin( double x );

double XCos( double x );

int qsqrt(int a);


struct compx EE(struct compx a,struct compx b);
void FFT(struct compx *xin);


int find_max_num_index(struct compx *data,int count);

typedef struct
{
	float w;
	int init;
	float a;

}DC_FilterData;


int dc_filter(int input,DC_FilterData * df);

typedef struct
{
	float v0;
	float v1;
}BW_FilterData;

int bw_filter(int input,BW_FilterData * bw);


#endif  /*__ALGORITHM_H*/

