// serial.h Header File
// This header containt the decleration of the serial check functions in order 
// to check the computation is correct.

#ifndef _S_CHK_
#define _S_CHK_

// serail Mul of A * B, putting the result on C
////////////////////////////////////////////////////////////////////////////////
//! Compute reference data set
//! C = A * B
//! @param C          reference data, computed but preallocated
//! @param A          matrix A as provided to device
//! @param B          matrix B as provided to device
//! @param hA         height of matrix A
//! @param wB         width of matrix B
////////////////////////////////////////////////////////////////////////////////

extern "C"
void runSerail_MUL( float*, const float*, const float*, unsigned int, 
				   unsigned int, unsigned int);

#endif