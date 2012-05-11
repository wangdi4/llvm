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

File Name:  imathLibd.cpp

\*****************************************************************************/
// imathLibd.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "imathLibd.h"
#if defined (_WIN32)
#include "mathimf.h"
#else
#include "math.h"
#endif
// This is the constructor of a class that has been exported.
// see imathLibd.h for the class definition
CimathLibd::CimathLibd()
{
	return;
}


long double  CimathLibd::imf_rint(long double x) {
    return rintl(x);
}

long double  CimathLibd::imf_asin(long double x) {
    return asinl(x);
}

long double  CimathLibd::imf_atan(long double x) {
    return atanl(x);
}

long double  CimathLibd::imf_tanh(long double x) {
    return tanhl(x);
}

long double  CimathLibd::imf_sqrt(long double x) {
    return sqrtl(x);
}

long double  CimathLibd::imf_log1p(long double x) {
    return log1pl(x);
}

long double CimathLibd::imf_pow(long double x, long double y) {
    return powl(x,y);
}

long double CimathLibd::imf_nextafter(long double x, long double y) {
    return nextafterl(x,y);
}

int CimathLibd::imf_ilogb(long double x) {
    return ilogbl(x);
}

long double  CimathLibd::imf_exp(long double x) {
    return expl(x);
}

long double  CimathLibd::imf_exp2(long double x) {
    return exp2l(x);
}

long double  CimathLibd::imf_expm1(long double x) {
    return expm1l(x);
}

long double  CimathLibd::imf_ldexp(long double x, int n) {
    return ldexpl(x,n);
}

long double  CimathLibd::imf_frexp(long double x, int* n) {
    return frexpl(x,n);
}

long double  CimathLibd::imf_fabs(long double x) {
    return fabsl(x);
}

long double CimathLibd::imf_floor(long double x) {
    return floorl(x);
}

long double  CimathLibd::imf_log2(long double x) {
    return log2l(x);
}

long double  CimathLibd::imf_sin(long double x) {
    return sinl(x);
}

long double  CimathLibd::imf_cos(long double x) {
    return cosl(x);
}

long double  CimathLibd::imf_tan(long double x) {
    return tanl(x);
}

long double  CimathLibd::imf_cosh(long double x) {
    return coshl(x);
}
