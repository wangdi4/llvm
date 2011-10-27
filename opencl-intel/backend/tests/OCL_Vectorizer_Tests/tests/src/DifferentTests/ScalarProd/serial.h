// *******************************************************************************
// serial.h Header File
// This header contain the decleration of the serial check functions in order 
// to check the computation is correct.
//
//		Author : Mohammed Agabaria
//		Updates : 27/11/08	Release
// *******************************************************************************
#ifndef _S_CHK_
#define _S_CHK_

//*******************************************************************************
// ScalarProdSerial -
// Calculate scalar products of VectorN vectors of ElementN elements. (Serial)
//*******************************************************************************

extern "C"
void scalarProdSerial(
				   float *h_C,
				   float *h_A,
				   float *h_B,
				   int vectorN,
				   int elementN
				   );
#endif