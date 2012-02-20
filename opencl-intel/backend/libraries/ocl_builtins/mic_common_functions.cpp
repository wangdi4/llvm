// Copyright (c) 2006-2007 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

///////////////////////////////////////////////////////////
//  svml_naive_functions.cpp
///////////////////////////////////////////////////////////

#if defined (__MIC__) || defined(__MIC2__)

#ifdef __cplusplus
extern "C" {
#endif

#include <intrin.h>

#define INRNL_PI        3.14159265358979323846264338327950288   /* pi */

#define INTRL_PI_180    0.017453292519943295769236907684883 //pi/180 this precision needed for conformance
#define INTRL_180_PI    57.295779513082320876798154814105 //180/pi this precision needed for conformance

#define FLOAT(c)    ((float)((c)))
#define DOUBLE(c)   ((double)((c)))

#define MAX(a,b)    (((a) > (b)) ? (a) : (b))
#define MIN(a,b)    (((a) < (b)) ? (a) : (b))


//clamp
//Returns fmin(fmax(x, minval), maxval).
//Results are undefined if minval > maxval.
float16  __attribute__((overloadable)) clamp(float16 x, float16 minval, float16 maxval)
{
    return _mm512_min_ps(_mm512_max_ps(x, minval), maxval);
}
float16  __attribute__((overloadable)) mask_clamp(ushort m16, float16 x, float16 minval, float16 maxval)
{
    return _mm512_mask_min_ps(x, m16, _mm512_mask_max_ps(x, m16, x, minval), maxval);
}

double8 __attribute__((overloadable)) clamp(double8 x, double8 minval, double8 maxval)
{
    return _mm512_min_pd(_mm512_max_pd(x, minval), maxval);
}
double8 __attribute__((overloadable)) mask_clamp(uchar m8, double8 x, double8 minval, double8 maxval)
{
    return _mm512_mask_min_pd(x, m8, _mm512_mask_max_pd(x, m8, x, minval), maxval);
}


//degrees
//Converts radians to degrees, i.e. (180 / pi) *
//radians
float16  __attribute__((overloadable)) degrees(float16 radians)
{
    return _mm512_mul_ps(radians, (float16)FLOAT(INTRL_180_PI));
}
float16  __attribute__((overloadable)) mask_degrees(ushort m16, float16 radians)
{
    return _mm512_mask_mul_ps(radians, m16, radians, (float16)FLOAT(INTRL_180_PI));
}

double8   __attribute__((overloadable)) degrees(double8 radians)
{
    return _mm512_mul_pd(radians, (double8)DOUBLE(INTRL_180_PI));
}
double8  __attribute__((overloadable)) mask_degrees(uchar m8, double8 radians)
{
    return _mm512_mask_mul_pd(radians, m8, radians, (double8)DOUBLE(INTRL_180_PI));
}


//max
//Returns y if x < y, otherwise it returns x. If x and y
//are infinite or NaN, the return values are undefined
float16  __attribute__((overloadable)) max(float16 x, float16 y)
{
    return _mm512_max_ps(x, y);
}
float16  __attribute__((overloadable)) mask_max(ushort m16, float16 x, float16 y)
{
    return _mm512_mask_max_ps(x, m16, x, y);
}

double8  __attribute__((overloadable)) max(double8 x, double8 y)
{
    return _mm512_max_pd(x, y);
}
double8  __attribute__((overloadable)) mask_max(uchar m8, double8 x, double8 y)
{
    return _mm512_mask_max_pd(x, m8, x, y);
}


//min
//Returns y if x > y, otherwise it returns x. If x and y
//are infinite or NaN, the return values are undefined
float16  __attribute__((overloadable)) min(float16 x, float16 y)
{
    return _mm512_min_ps(x,y);
}
float16  __attribute__((overloadable)) mask_min(ushort m16, float16 x, float16 y)
{
    return _mm512_mask_min_ps(x, m16, x, y);
}

double8  __attribute__((overloadable)) min(double8 x, double8 y)
{
    return _mm512_min_pd(x,y);
}
double8  __attribute__((overloadable)) mask_min(uchar m8, double8 x, double8 y)
{
    return _mm512_mask_min_pd(x, m8, x, y);
}


//mix
//Returns the linear blend of x & y implemented as:
//x + (y - x) * a
//a must be a value in the range 0.0 ~ 1.0. If a is not
//in the range 0.0 ~ 1.0, the return values are
//undefined.
float16 __attribute__((overloadable)) mix(float16 x, float16 y, float16 a)
{
    return _mm512_add_ps(x, _mm512_mul_ps(_mm512_sub_ps(y, x), a));
}
float16 __attribute__((overloadable)) mask_mix(ushort m16, float16 x, float16 y, float16 a)
{
    return _mm512_mask_add_ps(x, m16, x, _mm512_mask_mul_ps(a, m16, _mm512_mask_sub_ps(y, m16, y, x), a));
}


double8 __attribute__((overloadable)) mix(double8 x, double8 y, double8 a)
{
    return _mm512_add_pd(x, _mm512_mul_pd(_mm512_sub_pd(y, x), a));
}
double8 __attribute__((overloadable)) mask_mix(uchar m8, double8 x, double8 y, double8 a)
{
    return _mm512_mask_add_pd(x, m8, x, _mm512_mask_mul_pd(a, m8, _mm512_mask_sub_pd(y, m8, y, x), a));
}


//radians
//Converts  __degrees to radians, i.e. (pi / 180) *
// __degrees.
float16  __attribute__((overloadable)) radians(float16 radians)
{
    return _mm512_mul_ps(radians, (float16)FLOAT(INTRL_PI_180));
}
float16  __attribute__((overloadable)) mask_radians(ushort m16, float16 radians)
{
    return _mm512_mask_mul_ps(radians, m16, radians, (float16)FLOAT(INTRL_PI_180));
}

double8  __attribute__((overloadable)) radians(double8 radians)
{
    return _mm512_mul_pd(radians, (double8)DOUBLE(INTRL_PI_180));
}
double8  __attribute__((overloadable)) mask_radians(uchar m8, double8 radians)
{
    return _mm512_mask_mul_pd(radians, m8, radians, (double8)DOUBLE(INTRL_PI_180));
}


//step
//Returns 0.0 if x < edge, otherwise it returns 1.0
float16  __attribute__((overloadable)) step(float16 edge, float16 x)
{
    __mmask16 k = _mm512_cmplt_ps(x, edge);
    return _mm512_mask_mov_ps((float16)FLOAT(1.0), k, (float16)FLOAT(0.0));
}
float16  __attribute__((overloadable)) mask_step(ushort m16, float16 edge, float16 x)
{
    __mmask16 k = _mm512_mask_cmplt_ps(m16, x, edge);
    return _mm512_mask_mov_ps((float16)FLOAT(1.0), k, (float16)FLOAT(0.0));
}

double8  __attribute__((overloadable)) step(double8 edge, double8 x)
{
    __mmask8 k = _mm512_cmplt_pd(x, edge);
    return _mm512_mask_mov_pd((double8)DOUBLE(1.0), k, (double8)DOUBLE(0.0));
}
double8  __attribute__((overloadable)) mask_step(uchar m8, double8 edge, double8 x)
{
    __mmask8 k = _mm512_mask_cmplt_pd(m8, x, edge);
    return _mm512_mask_mov_pd((double8)DOUBLE(1.0), k, (double8)DOUBLE(0.0));
}


//Returns 0.0 if x <= edge0 and 1.0 if x >= edge1 and
//performs smooth Hermite interpolation between 0
//and 1when edge0 < x < edge1. This is useful in
//cases where you would want a threshold function
//with a smooth transition.
//This is equivalent to:
//gentype t;
//t = clamp ((x - edge0) / (edge1 - edge0), 0, 1);
//return t * t * (3 - 2 * t);
//Results are undefined if edge0 >= edge1 or if x,
//edge0 or edge1 is a NaN.
float16  __attribute__((overloadable)) smoothstep(float16 edge0, float16 edge1, float16 x)
{
    float16 t = _mm512_div_ps(_mm512_sub_ps(x, edge0), _mm512_sub_ps(edge1, edge0));
    t = _mm512_clampz_ps(t, (float16)FLOAT(1.0));
    float16 t1 = _mm512_mul_ps(t, t);
    float16 t2 = _mm512_add_ps(t, t);
    float16 t3 = _mm512_sub_ps((float16)FLOAT(3.0), t2);
    return _mm512_mul_ps(t1, t3);
}
float16  __attribute__((overloadable)) mask_smoothstep(ushort m16, float16 edge0, float16 edge1, float16 x)
{
    float16 t = _mm512_mask_div_ps(x, m16, _mm512_mask_sub_ps(x, m16, x, edge0), _mm512_mask_sub_ps(edge1, m16, edge1, edge0));
    t = _mm512_mask_clampz_ps(t, m16, t, (float16)FLOAT(1.0));
    float16 t1 = _mm512_mask_mul_ps(t, m16, t, t);
    float16 t2 = _mm512_mask_add_ps(t, m16, t, t);
    float16 t3 = _mm512_mask_sub_ps(t2, m16, (float16)FLOAT(3.0), t2);
    return _mm512_mask_mul_ps(t1, m16, t1, t3);
}

double8  __attribute__((overloadable)) smoothstep(double8 edge0, double8 edge1, double8 x)
{
    double8 t = _mm512_div_pd(_mm512_sub_pd(x, edge0), _mm512_sub_pd(edge1, edge0));
    t = _mm512_max_pd(t, (double8)DOUBLE(0.0));
    t = _mm512_min_pd(t, (double8)DOUBLE(1.0));
    double8 t1 = _mm512_mul_pd(t, t);
    double8 t2 = _mm512_add_pd(t, t);
    double8 t3 = _mm512_sub_pd((double8)DOUBLE(3.0), t2);
    return _mm512_mul_pd(t1, t3);
}
double8  __attribute__((overloadable)) mask_smoothstep(uchar m8, double8 edge0, double8 edge1, double8 x)
{
    double8 t = _mm512_mask_div_pd(x, m8, _mm512_mask_sub_pd(x, m8, x, edge0), _mm512_mask_sub_pd(edge1, m8, edge1, edge0));
    t = _mm512_mask_max_pd(t, m8, t, (double8)DOUBLE(0.0));
    t = _mm512_mask_min_pd(t, m8, t, (double8)DOUBLE(1.0));
    double8 t1 = _mm512_mask_mul_pd(t, m8, t, t);
    double8 t2 = _mm512_mask_add_pd(t, m8, t, t);
    double8 t3 = _mm512_mask_sub_pd(t2, m8, (double8)DOUBLE(3.0), t2);
    return _mm512_mask_mul_pd(t1, m8, t1, t3);
}


//gentype sign (gentype x) Returns 1.0 if x > 0, -0.0 if x = -0.0, +0.0 if x =
//+0.0, or -1.0 if x < 0. Returns 0.0 if x is a NaN.
float16  __attribute__((overloadable)) sign(float16 x)
{
    __mmask16 k1 = _mm512_cmpneq_ps(x, x);
    __m512 res = _mm512_mask_mov_ps(x, k1, (float16)FLOAT(0.0));
    __mmask16 k2 = _mm512_cmple_ps((float16)FLOAT(0.0), x);
    res = _mm512_mask_mov_ps(res, k2, (float16)FLOAT(1.0));
    __mmask16 k3 = _mm512_cmple_ps(x, (float16)FLOAT(0.0));
    res = _mm512_mask_mov_ps(res, k3, (float16)FLOAT(-1.0));
    return res;
}
float16  __attribute__((overloadable)) mask_sign(ushort m16, float16 x)
{
    __mmask16 k1 = _mm512_mask_cmpneq_ps(m16, x, x);
    __m512 res = _mm512_mask_mov_ps(x, k1, (float16)FLOAT(0.0));
    __mmask16 k2 = _mm512_mask_cmple_ps(m16, (float16)FLOAT(0.0), x);
    res = _mm512_mask_mov_ps(res, k2, (float16)FLOAT(1.0));
    __mmask16 k3 = _mm512_mask_cmple_ps(m16, x, (float16)FLOAT(0.0));
    res = _mm512_mask_mov_ps(res, k3, (float16)FLOAT(-1.0));
    return res;
}

double8  __attribute__((overloadable)) sign(double8 x)
{
    __mmask8 k1 = _mm512_cmpneq_pd(x, x);
    __m512d res = _mm512_mask_mov_pd(x, k1, (float16)FLOAT(0.0));
    __mmask8 k2 = _mm512_cmple_pd((float16)FLOAT(0.0), x);
    res = _mm512_mask_mov_pd(res, k2, (float16)FLOAT(1.0));
    __mmask8 k3 = _mm512_cmple_pd(x, (float16)FLOAT(0.0));
    res = _mm512_mask_mov_pd(res, k3, (float16)FLOAT(-1.0));
    return res;
}
double8  __attribute__((overloadable)) mask_sign(uchar m8, double8 x)
{
    __mmask8 k1 = _mm512_mask_cmpneq_pd(m8, x, x);
    __m512d res = _mm512_mask_mov_pd(x, k1, (float16)FLOAT(0.0));
    __mmask8 k2 = _mm512_mask_cmple_pd(m8, (float16)FLOAT(0.0), x);
    res = _mm512_mask_mov_pd(res, k2, (float16)FLOAT(1.0));
    __mmask8 k3 = _mm512_mask_cmple_pd(m8, x, (float16)FLOAT(0.0));
    res = _mm512_mask_mov_pd(res, k3, (float16)FLOAT(-1.0));
    return res;
}

#include "mic_cl_common_declaration.h"

#ifdef __cplusplus
}
#endif

#endif // defined (__MIC__) || defined(__MIC2__)
