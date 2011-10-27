// serial.cpp
// Implementation page of the serial Header File functions

#include "serial.h"
#include <math.h>

///////////////////////////////////////////////////////////////////////////////
// Polynomial approximation of cumulative normal distribution function
///////////////////////////////////////////////////////////////////////////////
#define A1			0.31938153f
#define A2			-0.356563782f
#define A3			1.781477937f
#define A4			-1.821255978f
#define A5			1.330274429f
#define RSQRT2PI	0.3989422804f

float cndCPU(float d){
    float
	K = 1.0f / (1.0f + 0.2316419f * fabsf(d));
	
    float
	cnd = RSQRT2PI * expf(- 0.5f * d * d) * 
	(K * (A1 + K * (A2 + K * (A3 + K * (A4 + K * A5)))));
	
    if(d > 0)
        cnd = 1.0f - cnd;
	
    return cnd;
}


///////////////////////////////////////////////////////////////////////////////
// Black-Scholes formula for both call and put
///////////////////////////////////////////////////////////////////////////////
void BlackScholesBodyCPU(
						 float& CallResult,
						 float& PutResult,
						 float S, //Stock price
						 float X, //Option strike
						 float T, //Option years
						 float R, //Riskless rate
						 float V  //Volatility rate
						 ){
    float sqrtT, expRT;
    float d1, d2, CNDD1, CNDD2;
	
    sqrtT = sqrtf(T);
    d1 = (logf(S / X) + (R + 0.5f * V * V) * T) / (V * sqrtT);
    d2 = d1 - V * sqrtT;
	
    CNDD1 = cndCPU(d1);
    CNDD2 = cndCPU(d2);
	
    //Calculate Call and Put simultaneously
    expRT = expf(- R * T);
    
	CallResult = S * CNDD1 - X * expRT * CNDD2;
    PutResult  = X * expRT * (1.0f - CNDD2) - S * (1.0f - CNDD1);
}


////////////////////////////////////////////////////////////////////////////////
// Process an array of optN options
////////////////////////////////////////////////////////////////////////////////
extern "C" void BlackScholesCPU(
								float *h_CallResult,
								float *h_PutResult,
								float *h_StockPrice,
								float *h_OptionStrike,
								float *h_OptionYears,
								float Riskfree,
								float Volatility,
								int optN
								){
    for(int opt = 0; opt < optN; opt++)
        BlackScholesBodyCPU(
							h_CallResult[opt],
							h_PutResult[opt],
							h_StockPrice[opt],
							h_OptionStrike[opt],
							h_OptionYears[opt],
							Riskfree,
							Volatility
							);
}