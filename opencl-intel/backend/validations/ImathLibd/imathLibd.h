/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  ImathLibd.h

\*****************************************************************************/

// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the IMATHLIBD_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// IMATHLIBD_API functions as being imported from a DLL, whereas this DLL sees symbols
// defined with this macro as being exported.

#if defined(_WIN32)
    #define IMATHLIBD_API __declspec(dllexport)
//#define IMATHLIBD_API __declspec(dllimport)
#else
    #define IMATHLIBD_API
#endif
// This class is exported from the imathLibd.dll
class IMATHLIBD_API CimathLibd {
public:
	CimathLibd(void);

long double static  imf_rint(long double x);
long double static  imf_atan(long double x);
long double static  imf_asin(long double x);
long double static  imf_tanh(long double x);
long double static  imf_sqrt(long double x);
long double static  imf_log1p(long double x);
long double static  imf_pow(long double x, long double y);
long double static imf_nextafter(long double x, long double y);
int         static  imf_ilogb(long double x);
long double static  imf_exp(long double x);
long double static  imf_exp2(long double x);
long double static  imf_expm1(long double x);
long double static  imf_ldexp(long double x, int n);
long double static  imf_frexp(long double x, int* n);
long double static  imf_fabs(long double x);
long double static  imf_floor(long double x);
long double static  imf_log2(long double x);
long double static  imf_sin(long double x);
long double static  imf_cos(long double x);
long double static  imf_tan(long double x);
long double static  imf_cosh(long double x);
};
