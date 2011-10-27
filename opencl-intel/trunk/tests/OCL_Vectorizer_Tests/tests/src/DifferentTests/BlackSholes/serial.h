// serial.h Header File
// This header containt the decleration of the serial check functions in order 
// to check the computation is correct.

#ifndef _S_CHK_
#define _S_CHK_

// This function computes BlackSholes on the CPU, Serial way.
extern "C" void BlackScholesCPU(
								float *h_CallResult,
								float *h_PutResult,
								float *h_StockPrice,
								float *h_OptionStrike,
								float *h_OptionYears,
								float Riskfree,
								float Volatility,
								int optN
								);

#endif