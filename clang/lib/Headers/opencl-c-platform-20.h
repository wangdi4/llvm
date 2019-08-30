#ifndef _OPENCL_PLATFORM_20_H_
#define _OPENCL_PLATFORM_20_H_

#include "opencl-c-platform.h"
#include "opencl-c-20.h"

#define MAX_WORK_DIM        3

typedef struct {
    unsigned int workDimension;
    size_t globalWorkOffset[MAX_WORK_DIM];
    size_t globalWorkSize[MAX_WORK_DIM];
    size_t localWorkSize[MAX_WORK_DIM];
} ndrange_t;

// OpenCL v1.1 s6.11.1, v1.2 s6.12.1, v2.0 s6.13.1 - Work-item Functions

size_t __ovld get_enqueued_local_size(uint dimindx);
size_t __ovld get_global_linear_id(void);
size_t __ovld get_local_linear_id(void);

// OpenCL v1.1 s6.11.2, v1.2 s6.12.2, v2.0 s6.13.2 - Math functions

/**
 * Returns fmin(x - floor (x), 0x1.fffffep-1f ).
 * floor(x) is returned in iptr.
 */
float __ovld fract(float x, float *iptr);
float2 __ovld fract(float2 x, float2 *iptr);
float3 __ovld fract(float3 x, float3 *iptr);
float4 __ovld fract(float4 x, float4 *iptr);
float8 __ovld fract(float8 x, float8 *iptr);
float16 __ovld fract(float16 x, float16 *iptr);
#ifdef cl_khr_fp64
double __ovld fract(double x, double *iptr);
double2 __ovld fract(double2 x, double2 *iptr);
double3 __ovld fract(double3 x, double3 *iptr);
double4 __ovld fract(double4 x, double4 *iptr);
double8 __ovld fract(double8 x, double8 *iptr);
double16 __ovld fract(double16 x, double16 *iptr);
#endif //cl_khr_fp64
#ifdef cl_khr_fp16
half __ovld fract(half x, half *iptr);
half2 __ovld fract(half2 x, half2 *iptr);
half3 __ovld fract(half3 x, half3 *iptr);
half4 __ovld fract(half4 x, half4 *iptr);
half8 __ovld fract(half8 x, half8 *iptr);
half16 __ovld fract(half16 x, half16 *iptr);
#endif //cl_khr_fp16

/**
 * Extract mantissa and exponent from x. For each
 * component the mantissa returned is a float with
 * magnitude in the interval [1/2, 1) or 0. Each
 * component of x equals mantissa returned * 2^exp.
 */
float __ovld frexp(float x, int *exp);
float2 __ovld frexp(float2 x, int2 *exp);
float3 __ovld frexp(float3 x, int3 *exp);
float4 __ovld frexp(float4 x, int4 *exp);
float8 __ovld frexp(float8 x, int8 *exp);
float16 __ovld frexp(float16 x, int16 *exp);
#ifdef cl_khr_fp64
double __ovld frexp(double x, int *exp);
double2 __ovld frexp(double2 x, int2 *exp);
double3 __ovld frexp(double3 x, int3 *exp);
double4 __ovld frexp(double4 x, int4 *exp);
double8 __ovld frexp(double8 x, int8 *exp);
double16 __ovld frexp(double16 x, int16 *exp);
#endif //cl_khr_fp64
#ifdef cl_khr_fp16
half __ovld frexp(half x, int *exp);
half2 __ovld frexp(half2 x, int2 *exp);
half3 __ovld frexp(half3 x, int3 *exp);
half4 __ovld frexp(half4 x, int4 *exp);
half8 __ovld frexp(half8 x, int8 *exp);
half16 __ovld frexp(half16 x, int16 *exp);
#endif //cl_khr_fp16

/**
 * Log gamma function. Returns the natural
 * logarithm of the absolute value of the gamma
 * function. The sign of the gamma function is
 * returned in the signp argument of lgamma_r.
 */
float __ovld lgamma_r(float x, int *signp);
float2 __ovld lgamma_r(float2 x, int2 *signp);
float3 __ovld lgamma_r(float3 x, int3 *signp);
float4 __ovld lgamma_r(float4 x, int4 *signp);
float8 __ovld lgamma_r(float8 x, int8 *signp);
float16 __ovld lgamma_r(float16 x, int16 *signp);
#ifdef cl_khr_fp64
double __ovld lgamma_r(double x, int *signp);
double2 __ovld lgamma_r(double2 x, int2 *signp);
double3 __ovld lgamma_r(double3 x, int3 *signp);
double4 __ovld lgamma_r(double4 x, int4 *signp);
double8 __ovld lgamma_r(double8 x, int8 *signp);
double16 __ovld lgamma_r(double16 x, int16 *signp);
#endif //cl_khr_fp64
#ifdef cl_khr_fp16
half __ovld lgamma_r(half x, int *signp);
half2 __ovld lgamma_r(half2 x, int2 *signp);
half3 __ovld lgamma_r(half3 x, int3 *signp);
half4 __ovld lgamma_r(half4 x, int4 *signp);
half8 __ovld lgamma_r(half8 x, int8 *signp);
half16 __ovld lgamma_r(half16 x, int16 *signp);
#endif //cl_khr_fp16

/**
 * Decompose a floating-point number. The modf
 * function breaks the argument x into integral and
 * fractional parts, each of which has the same sign as
 * the argument. It stores the integral part in the object
 * pointed to by iptr.
 */
float __ovld modf(float x, float *iptr);
float2 __ovld modf(float2 x, float2 *iptr);
float3 __ovld modf(float3 x, float3 *iptr);
float4 __ovld modf(float4 x, float4 *iptr);
float8 __ovld modf(float8 x, float8 *iptr);
float16 __ovld modf(float16 x, float16 *iptr);
#ifdef cl_khr_fp64
double __ovld modf(double x, double *iptr);
double2 __ovld modf(double2 x, double2 *iptr);
double3 __ovld modf(double3 x, double3 *iptr);
double4 __ovld modf(double4 x, double4 *iptr);
double8 __ovld modf(double8 x, double8 *iptr);
double16 __ovld modf(double16 x, double16 *iptr);
#endif //cl_khr_fp64
#ifdef cl_khr_fp16
half __ovld modf(half x, half *iptr);
half2 __ovld modf(half2 x, half2 *iptr);
half3 __ovld modf(half3 x, half3 *iptr);
half4 __ovld modf(half4 x, half4 *iptr);
half8 __ovld modf(half8 x, half8 *iptr);
half16 __ovld modf(half16 x, half16 *iptr);
#endif //cl_khr_fp16

/**
 * The remquo function computes the value r such
 * that r = x - n*y, where n is the integer nearest the
 * exact value of x/y. If there are two integers closest
 * to x/y, n shall be the even one. If r is zero, it is
 * given the same sign as x. This is the same value
 * that is returned by the remainder function.
 * remquo also calculates the lower seven bits of the
 * integral quotient x/y, and gives that value the same
 * sign as x/y. It stores this signed value in the object
 * pointed to by quo.
 */
/**
 * The remquo function computes the value r such
 * that r = x - n*y, where n is the integer nearest the
 * exact value of x/y. If there are two integers closest
 * to x/y, n shall be the even one. If r is zero, it is
 * given the same sign as x. This is the same value
 * that is returned by the remainder function.
 * remquo also calculates the lower seven bits of the
 * integral quotient x/y, and gives that value the same
 * sign as x/y. It stores this signed value in the object
 * pointed to by quo.
 */
float __ovld remquo(float x, float y, int *quo);
float2 __ovld remquo(float2 x, float2 y, int2 *quo);
float3 __ovld remquo(float3 x, float3 y, int3 *quo);
float4 __ovld remquo(float4 x, float4 y, int4 *quo);
float8 __ovld remquo(float8 x, float8 y, int8 *quo);
float16 __ovld remquo(float16 x, float16 y, int16 *quo);
#ifdef cl_khr_fp64
double __ovld remquo(double x, double y, int *quo);
double2 __ovld remquo(double2 x, double2 y, int2 *quo);
double3 __ovld remquo(double3 x, double3 y, int3 *quo);
double4 __ovld remquo(double4 x, double4 y, int4 *quo);
double8 __ovld remquo(double8 x, double8 y, int8 *quo);
double16 __ovld remquo(double16 x, double16 y, int16 *quo);
#endif //cl_khr_fp64
#ifdef cl_khr_fp16
half __ovld remquo(half x, half y, int *quo);
half2 __ovld remquo(half2 x, half2 y, int2 *quo);
half3 __ovld remquo(half3 x, half3 y, int3 *quo);
half4 __ovld remquo(half4 x, half4 y, int4 *quo);
half8 __ovld remquo(half8 x, half8 y, int8 *quo);
half16 __ovld remquo(half16 x, half16 y, int16 *quo);
#endif //cl_khr_fp16

/**
 * Compute sine and cosine of x. The computed sine
 * is the return value and computed cosine is returned
 * in cosval.
 */
float __ovld sincos(float x, float *cosval);
float2 __ovld sincos(float2 x, float2 *cosval);
float3 __ovld sincos(float3 x, float3 *cosval);
float4 __ovld sincos(float4 x, float4 *cosval);
float8 __ovld sincos(float8 x, float8 *cosval);
float16 __ovld sincos(float16 x, float16 *cosval);
#ifdef cl_khr_fp64
double __ovld sincos(double x, double *cosval);
double2 __ovld sincos(double2 x, double2 *cosval);
double3 __ovld sincos(double3 x, double3 *cosval);
double4 __ovld sincos(double4 x, double4 *cosval);
double8 __ovld sincos(double8 x, double8 *cosval);
double16 __ovld sincos(double16 x, double16 *cosval);
#endif //cl_khr_fp64
#ifdef cl_khr_fp16
half __ovld sincos(half x, half *cosval);
half2 __ovld sincos(half2 x, half2 *cosval);
half3 __ovld sincos(half3 x, half3 *cosval);
half4 __ovld sincos(half4 x, half4 *cosval);
half8 __ovld sincos(half8 x, half8 *cosval);
half16 __ovld sincos(half16 x, half16 *cosval);
#endif //cl_khr_fp16

// OpenCL v1.1 s6.11.7, v1.2 s6.12.7, v2.0 s6.13.7 - Vector Data Load and Store Functions
// OpenCL extensions v1.1 s9.6.6, v1.2 s9.5.6, v2.0 s9.4.6 - Vector Data Load and Store Functions for Half Type

/**
 * Use generic type gentype to indicate the built-in data types
 * char, uchar, short, ushort, int, uint, long, ulong, float,
 * double or half.
 *
 * vloadn return sizeof (gentypen) bytes of data read from address (p + (offset * n)).
 *
 * vstoren write sizeof (gentypen) bytes given by data to address (p + (offset * n)).
 *
 * The address computed as (p + (offset * n)) must be
 * 8-bit aligned if gentype is char, uchar;
 * 16-bit aligned if gentype is short, ushort, half;
 * 32-bit aligned if gentype is int, uint, float;
 * 64-bit aligned if gentype is long, ulong, double.
 */
char2 __ovld vload2(size_t offset, const char *p);
uchar2 __ovld vload2(size_t offset, const uchar *p);
short2 __ovld vload2(size_t offset, const short *p);
ushort2 __ovld vload2(size_t offset, const ushort *p);
int2 __ovld vload2(size_t offset, const int *p);
uint2 __ovld vload2(size_t offset, const uint *p);
long2 __ovld vload2(size_t offset, const long *p);
ulong2 __ovld vload2(size_t offset, const ulong *p);
float2 __ovld vload2(size_t offset, const float *p);
char3 __ovld vload3(size_t offset, const char *p);
uchar3 __ovld vload3(size_t offset, const uchar *p);
short3 __ovld vload3(size_t offset, const short *p);
ushort3 __ovld vload3(size_t offset, const ushort *p);
int3 __ovld vload3(size_t offset, const int *p);
uint3 __ovld vload3(size_t offset, const uint *p);
long3 __ovld vload3(size_t offset, const long *p);
ulong3 __ovld vload3(size_t offset, const ulong *p);
float3 __ovld vload3(size_t offset, const float *p);
char4 __ovld vload4(size_t offset, const char *p);
uchar4 __ovld vload4(size_t offset, const uchar *p);
short4 __ovld vload4(size_t offset, const short *p);
ushort4 __ovld vload4(size_t offset, const ushort *p);
int4 __ovld vload4(size_t offset, const int *p);
uint4 __ovld vload4(size_t offset, const uint *p);
long4 __ovld vload4(size_t offset, const long *p);
ulong4 __ovld vload4(size_t offset, const ulong *p);
float4 __ovld vload4(size_t offset, const float *p);
char8 __ovld vload8(size_t offset, const char *p);
uchar8 __ovld vload8(size_t offset, const uchar *p);
short8 __ovld vload8(size_t offset, const short *p);
ushort8 __ovld vload8(size_t offset, const ushort *p);
int8 __ovld vload8(size_t offset, const int *p);
uint8 __ovld vload8(size_t offset, const uint *p);
long8 __ovld vload8(size_t offset, const long *p);
ulong8 __ovld vload8(size_t offset, const ulong *p);
float8 __ovld vload8(size_t offset, const float *p);
char16 __ovld vload16(size_t offset, const char *p);
uchar16 __ovld vload16(size_t offset, const uchar *p);
short16 __ovld vload16(size_t offset, const short *p);
ushort16 __ovld vload16(size_t offset, const ushort *p);
int16 __ovld vload16(size_t offset, const int *p);
uint16 __ovld vload16(size_t offset, const uint *p);
long16 __ovld vload16(size_t offset, const long *p);
ulong16 __ovld vload16(size_t offset, const ulong *p);
float16 __ovld vload16(size_t offset, const float *p);

#ifdef cl_khr_fp64
double2 __ovld vload2(size_t offset, const double *p);
double3 __ovld vload3(size_t offset, const double *p);
double4 __ovld vload4(size_t offset, const double *p);
double8 __ovld vload8(size_t offset, const double *p);
double16 __ovld vload16(size_t offset, const double *p);
#endif //cl_khr_fp64

#ifdef cl_khr_fp16
half __ovld vload(size_t offset, const half *p);
half2 __ovld vload2(size_t offset, const half *p);
half3 __ovld vload3(size_t offset, const half *p);
half4 __ovld vload4(size_t offset, const half *p);
half8 __ovld vload8(size_t offset, const half *p);
half16 __ovld vload16(size_t offset, const half *p);
#endif //cl_khr_fp16

void __ovld vstore2(char2 data, size_t offset, char *p);
void __ovld vstore2(uchar2 data, size_t offset, uchar *p);
void __ovld vstore2(short2 data, size_t offset, short *p);
void __ovld vstore2(ushort2 data, size_t offset, ushort *p);
void __ovld vstore2(int2 data, size_t offset, int *p);
void __ovld vstore2(uint2 data, size_t offset, uint *p);
void __ovld vstore2(long2 data, size_t offset, long *p);
void __ovld vstore2(ulong2 data, size_t offset, ulong *p);
void __ovld vstore2(float2 data, size_t offset, float *p);
void __ovld vstore3(char3 data, size_t offset, char *p);
void __ovld vstore3(uchar3 data, size_t offset, uchar *p);
void __ovld vstore3(short3 data, size_t offset, short *p);
void __ovld vstore3(ushort3 data, size_t offset, ushort *p);
void __ovld vstore3(int3 data, size_t offset, int *p);
void __ovld vstore3(uint3 data, size_t offset, uint *p);
void __ovld vstore3(long3 data, size_t offset, long *p);
void __ovld vstore3(ulong3 data, size_t offset, ulong *p);
void __ovld vstore3(float3 data, size_t offset, float *p);
void __ovld vstore4(char4 data, size_t offset, char *p);
void __ovld vstore4(uchar4 data, size_t offset, uchar *p);
void __ovld vstore4(short4 data, size_t offset, short *p);
void __ovld vstore4(ushort4 data, size_t offset, ushort *p);
void __ovld vstore4(int4 data, size_t offset, int *p);
void __ovld vstore4(uint4 data, size_t offset, uint *p);
void __ovld vstore4(long4 data, size_t offset, long *p);
void __ovld vstore4(ulong4 data, size_t offset, ulong *p);
void __ovld vstore4(float4 data, size_t offset, float *p);
void __ovld vstore8(char8 data, size_t offset, char *p);
void __ovld vstore8(uchar8 data, size_t offset, uchar *p);
void __ovld vstore8(short8 data, size_t offset, short *p);
void __ovld vstore8(ushort8 data, size_t offset, ushort *p);
void __ovld vstore8(int8 data, size_t offset, int *p);
void __ovld vstore8(uint8 data, size_t offset, uint *p);
void __ovld vstore8(long8 data, size_t offset, long *p);
void __ovld vstore8(ulong8 data, size_t offset, ulong *p);
void __ovld vstore8(float8 data, size_t offset, float *p);
void __ovld vstore16(char16 data, size_t offset, char *p);
void __ovld vstore16(uchar16 data, size_t offset, uchar *p);
void __ovld vstore16(short16 data, size_t offset, short *p);
void __ovld vstore16(ushort16 data, size_t offset, ushort *p);
void __ovld vstore16(int16 data, size_t offset, int *p);
void __ovld vstore16(uint16 data, size_t offset, uint *p);
void __ovld vstore16(long16 data, size_t offset, long *p);
void __ovld vstore16(ulong16 data, size_t offset, ulong *p);
void __ovld vstore16(float16 data, size_t offset, float *p);
#ifdef cl_khr_fp64
void __ovld vstore2(double2 data, size_t offset, double *p);
void __ovld vstore3(double3 data, size_t offset, double *p);
void __ovld vstore4(double4 data, size_t offset, double *p);
void __ovld vstore8(double8 data, size_t offset, double *p);
void __ovld vstore16(double16 data, size_t offset, double *p);
#endif //cl_khr_fp64
#ifdef cl_khr_fp16
void __ovld vstore(half data, size_t offset, half *p);
void __ovld vstore2(half2 data, size_t offset, half *p);
void __ovld vstore3(half3 data, size_t offset, half *p);
void __ovld vstore4(half4 data, size_t offset, half *p);
void __ovld vstore8(half8 data, size_t offset, half *p);
void __ovld vstore16(half16 data, size_t offset, half *p);
#endif //cl_khr_fp16

/**
 * Read sizeof (half) bytes of data from address
 * (p + offset). The data read is interpreted as a
 * half value. The half value is converted to a
 * float value and the float value is returned.
 * The read address computed as (p + offset)
 * must be 16-bit aligned.
 */
float __ovld vload_half(size_t offset, const half *p);

/**
 * Read sizeof (halfn) bytes of data from address
 * (p + (offset * n)). The data read is interpreted
 * as a halfn value. The halfn value read is
 * converted to a floatn value and the floatn
 * value is returned. The read address computed
 * as (p + (offset * n)) must be 16-bit aligned.
 */
float2 __ovld vload_half2(size_t offset, const half *p);
float3 __ovld vload_half3(size_t offset, const half *p);
float4 __ovld vload_half4(size_t offset, const half *p);
float8 __ovld vload_half8(size_t offset, const half *p);
float16 __ovld vload_half16(size_t offset, const half *p);

/**
 * The float value given by data is first
 * converted to a half value using the appropriate
 * rounding mode. The half value is then written
 * to address computed as (p + offset). The
 * address computed as (p + offset) must be 16-
 * bit aligned.
 * vstore_half use the current rounding mode.
 * The default current rounding mode is round to
 * nearest even.
 */
void __ovld vstore_half(float data, size_t offset, half *p);
void __ovld vstore_half_rte(float data, size_t offset, half *p);
void __ovld vstore_half_rtz(float data, size_t offset, half *p);
void __ovld vstore_half_rtp(float data, size_t offset, half *p);
void __ovld vstore_half_rtn(float data, size_t offset, half *p);
#ifdef cl_khr_fp64
void __ovld vstore_half(double data, size_t offset, half *p);
void __ovld vstore_half_rte(double data, size_t offset, half *p);
void __ovld vstore_half_rtz(double data, size_t offset, half *p);
void __ovld vstore_half_rtp(double data, size_t offset, half *p);
void __ovld vstore_half_rtn(double data, size_t offset, half *p);
#endif //cl_khr_fp64

/**
 * The floatn value given by data is converted to
 * a halfn value using the appropriate rounding
 * mode. The halfn value is then written to
 * address computed as (p + (offset * n)). The
 * address computed as (p + (offset * n)) must be
 * 16-bit aligned.
 * vstore_halfn uses the current rounding mode.
 * The default current rounding mode is round to
 * nearest even.
 */
void __ovld vstore_half2(float2 data, size_t offset, half *p);
void __ovld vstore_half3(float3 data, size_t offset, half *p);
void __ovld vstore_half4(float4 data, size_t offset, half *p);
void __ovld vstore_half8(float8 data, size_t offset, half *p);
void __ovld vstore_half16(float16 data, size_t offset, half *p);
void __ovld vstore_half2_rte(float2 data, size_t offset, half *p);
void __ovld vstore_half3_rte(float3 data, size_t offset, half *p);
void __ovld vstore_half4_rte(float4 data, size_t offset, half *p);
void __ovld vstore_half8_rte(float8 data, size_t offset, half *p);
void __ovld vstore_half16_rte(float16 data, size_t offset, half *p);
void __ovld vstore_half2_rtz(float2 data, size_t offset, half *p);
void __ovld vstore_half3_rtz(float3 data, size_t offset, half *p);
void __ovld vstore_half4_rtz(float4 data, size_t offset, half *p);
void __ovld vstore_half8_rtz(float8 data, size_t offset, half *p);
void __ovld vstore_half16_rtz(float16 data, size_t offset, half *p);
void __ovld vstore_half2_rtp(float2 data, size_t offset, half *p);
void __ovld vstore_half3_rtp(float3 data, size_t offset, half *p);
void __ovld vstore_half4_rtp(float4 data, size_t offset, half *p);
void __ovld vstore_half8_rtp(float8 data, size_t offset, half *p);
void __ovld vstore_half16_rtp(float16 data, size_t offset, half *p);
void __ovld vstore_half2_rtn(float2 data, size_t offset, half *p);
void __ovld vstore_half3_rtn(float3 data, size_t offset, half *p);
void __ovld vstore_half4_rtn(float4 data, size_t offset, half *p);
void __ovld vstore_half8_rtn(float8 data, size_t offset, half *p);
void __ovld vstore_half16_rtn(float16 data, size_t offset, half *p);
#ifdef cl_khr_fp64
void __ovld vstore_half2(double2 data, size_t offset, half *p);
void __ovld vstore_half3(double3 data, size_t offset, half *p);
void __ovld vstore_half4(double4 data, size_t offset, half *p);
void __ovld vstore_half8(double8 data, size_t offset, half *p);
void __ovld vstore_half16(double16 data, size_t offset, half *p);
void __ovld vstore_half2_rte(double2 data, size_t offset, half *p);
void __ovld vstore_half3_rte(double3 data, size_t offset, half *p);
void __ovld vstore_half4_rte(double4 data, size_t offset, half *p);
void __ovld vstore_half8_rte(double8 data, size_t offset, half *p);
void __ovld vstore_half16_rte(double16 data, size_t offset, half *p);
void __ovld vstore_half2_rtz(double2 data, size_t offset, half *p);
void __ovld vstore_half3_rtz(double3 data, size_t offset, half *p);
void __ovld vstore_half4_rtz(double4 data, size_t offset, half *p);
void __ovld vstore_half8_rtz(double8 data, size_t offset, half *p);
void __ovld vstore_half16_rtz(double16 data, size_t offset, half *p);
void __ovld vstore_half2_rtp(double2 data, size_t offset, half *p);
void __ovld vstore_half3_rtp(double3 data, size_t offset, half *p);
void __ovld vstore_half4_rtp(double4 data, size_t offset, half *p);
void __ovld vstore_half8_rtp(double8 data, size_t offset, half *p);
void __ovld vstore_half16_rtp(double16 data, size_t offset, half *p);
void __ovld vstore_half2_rtn(double2 data, size_t offset, half *p);
void __ovld vstore_half3_rtn(double3 data, size_t offset, half *p);
void __ovld vstore_half4_rtn(double4 data, size_t offset, half *p);
void __ovld vstore_half8_rtn(double8 data, size_t offset, half *p);
void __ovld vstore_half16_rtn(double16 data, size_t offset, half *p);
#endif //cl_khr_fp64

/**
 * For n = 1, 2, 4, 8 and 16 read sizeof (halfn)
 * bytes of data from address (p + (offset * n)).
 * The data read is interpreted as a halfn value.
 * The halfn value read is converted to a floatn
 * value and the floatn value is returned.
 * The address computed as (p + (offset * n))
 * must be aligned to sizeof (halfn) bytes.
 * For n = 3, vloada_half3 reads a half3 from
 * address (p + (offset * 4)) and returns a float3.
 * The address computed as (p + (offset * 4))
 * must be aligned to sizeof (half) * 4 bytes.
 */
float __ovld vloada_half(size_t offset, const half *p);
float2 __ovld vloada_half2(size_t offset, const half *p);
float3 __ovld vloada_half3(size_t offset, const half *p);
float4 __ovld vloada_half4(size_t offset, const half *p);
float8 __ovld vloada_half8(size_t offset, const half *p);
float16 __ovld vloada_half16(size_t offset, const half *p);

/**
 * The floatn value given by data is converted to
 * a halfn value using the appropriate rounding
 * mode.
 * For n = 1, 2, 4, 8 and 16, the halfn value is
 * written to the address computed as (p + (offset
 * * n)). The address computed as (p + (offset *
 * n)) must be aligned to sizeof (halfn) bytes.
 * For n = 3, the half3 value is written to the
 * address computed as (p + (offset * 4)). The
 * address computed as (p + (offset * 4)) must be
 * aligned to sizeof (half) * 4 bytes.
 * vstorea_halfn uses the current rounding
 * mode. The default current rounding mode is
 * round to nearest even.
 */
void __ovld vstorea_half(float data, size_t offset, half *p);
void __ovld vstorea_half2(float2 data, size_t offset, half *p);
void __ovld vstorea_half3(float3 data, size_t offset, half *p);
void __ovld vstorea_half4(float4 data, size_t offset, half *p);
void __ovld vstorea_half8(float8 data, size_t offset, half *p);
void __ovld vstorea_half16(float16 data, size_t offset, half *p);

void __ovld vstorea_half_rte(float data, size_t offset, half *p);
void __ovld vstorea_half2_rte(float2 data, size_t offset, half *p);
void __ovld vstorea_half3_rte(float3 data, size_t offset, half *p);
void __ovld vstorea_half4_rte(float4 data, size_t offset, half *p);
void __ovld vstorea_half8_rte(float8 data, size_t offset, half *p);
void __ovld vstorea_half16_rte(float16 data, size_t offset, half *p);

void __ovld vstorea_half_rtz(float data, size_t offset, half *p);
void __ovld vstorea_half2_rtz(float2 data, size_t offset, half *p);
void __ovld vstorea_half3_rtz(float3 data, size_t offset, half *p);
void __ovld vstorea_half4_rtz(float4 data, size_t offset, half *p);
void __ovld vstorea_half8_rtz(float8 data, size_t offset, half *p);
void __ovld vstorea_half16_rtz(float16 data, size_t offset, half *p);

void __ovld vstorea_half_rtp(float data, size_t offset, half *p);
void __ovld vstorea_half2_rtp(float2 data, size_t offset, half *p);
void __ovld vstorea_half3_rtp(float3 data, size_t offset, half *p);
void __ovld vstorea_half4_rtp(float4 data, size_t offset, half *p);
void __ovld vstorea_half8_rtp(float8 data, size_t offset, half *p);
void __ovld vstorea_half16_rtp(float16 data, size_t offset, half *p);

void __ovld vstorea_half_rtn(float data, size_t offset, half *p);
void __ovld vstorea_half2_rtn(float2 data, size_t offset, half *p);
void __ovld vstorea_half3_rtn(float3 data, size_t offset, half *p);
void __ovld vstorea_half4_rtn(float4 data, size_t offset, half *p);
void __ovld vstorea_half8_rtn(float8 data, size_t offset, half *p);
void __ovld vstorea_half16_rtn(float16 data, size_t offset, half *p);

#ifdef cl_khr_fp64
void __ovld vstorea_half(double data, size_t offset, half *p);
void __ovld vstorea_half2(double2 data, size_t offset, half *p);
void __ovld vstorea_half3(double3 data, size_t offset, half *p);
void __ovld vstorea_half4(double4 data, size_t offset, half *p);
void __ovld vstorea_half8(double8 data, size_t offset, half *p);
void __ovld vstorea_half16(double16 data, size_t offset, half *p);

void __ovld vstorea_half_rte(double data, size_t offset, half *p);
void __ovld vstorea_half2_rte(double2 data, size_t offset, half *p);
void __ovld vstorea_half3_rte(double3 data, size_t offset, half *p);
void __ovld vstorea_half4_rte(double4 data, size_t offset, half *p);
void __ovld vstorea_half8_rte(double8 data, size_t offset, half *p);
void __ovld vstorea_half16_rte(double16 data, size_t offset, half *p);

void __ovld vstorea_half_rtz(double data, size_t offset, half *p);
void __ovld vstorea_half2_rtz(double2 data, size_t offset, half *p);
void __ovld vstorea_half3_rtz(double3 data, size_t offset, half *p);
void __ovld vstorea_half4_rtz(double4 data, size_t offset, half *p);
void __ovld vstorea_half8_rtz(double8 data, size_t offset, half *p);
void __ovld vstorea_half16_rtz(double16 data, size_t offset, half *p);

void __ovld vstorea_half_rtp(double data, size_t offset, half *p);
void __ovld vstorea_half2_rtp(double2 data, size_t offset, half *p);
void __ovld vstorea_half3_rtp(double3 data, size_t offset, half *p);
void __ovld vstorea_half4_rtp(double4 data, size_t offset, half *p);
void __ovld vstorea_half8_rtp(double8 data, size_t offset, half *p);
void __ovld vstorea_half16_rtp(double16 data, size_t offset, half *p);

void __ovld vstorea_half_rtn(double data, size_t offset, half *p);
void __ovld vstorea_half2_rtn(double2 data, size_t offset, half *p);
void __ovld vstorea_half3_rtn(double3 data, size_t offset, half *p);
void __ovld vstorea_half4_rtn(double4 data, size_t offset, half *p);
void __ovld vstorea_half8_rtn(double8 data, size_t offset, half *p);
void __ovld vstorea_half16_rtn(double16 data, size_t offset, half *p);
#endif //cl_khr_fp64

// OpenCL v2.0 s6.13.9 - Address Space Qualifier Functions

cl_mem_fence_flags __ovld get_fence(const void *ptr);
cl_mem_fence_flags __ovld get_fence(void *ptr);

// OpenCL v2.0 s6.13.11 - Atomics Functions
#ifndef ATOMIC_VAR_INIT
#define ATOMIC_VAR_INIT(x) (x)
#endif //ATOMIC_VAR_INIT
#define ATOMIC_FLAG_INIT 0

// double atomics support requires extensions cl_khr_int64_base_atomics and cl_khr_int64_extended_atomics
#if defined(cl_khr_int64_base_atomics) && defined(cl_khr_int64_extended_atomics)
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
#endif

// atomic_init()
void __ovld atomic_init(volatile atomic_int *object, int value);
void __ovld atomic_init(volatile atomic_uint *object, uint value);
void __ovld atomic_init(volatile atomic_float *object, float value);
#if defined(cl_khr_int64_base_atomics) && defined(cl_khr_int64_extended_atomics)
void __ovld atomic_init(volatile atomic_long *object, long value);
void __ovld atomic_init(volatile atomic_ulong *object, ulong value);
#ifdef cl_khr_fp64
void __ovld atomic_init(volatile atomic_double *object, double value);
#endif //cl_khr_fp64
#endif

// atomic_work_item_fence()
void __ovld atomic_work_item_fence(cl_mem_fence_flags flags, memory_order order, memory_scope scope);

// atomic_fetch()

int __ovld atomic_fetch_add(volatile atomic_int *object, int operand);
int __ovld atomic_fetch_add_explicit(volatile atomic_int *object, int operand, memory_order order);
int __ovld atomic_fetch_add_explicit(volatile atomic_int *object, int operand, memory_order order, memory_scope scope);
uint __ovld atomic_fetch_add(volatile atomic_uint *object, uint operand);
uint __ovld atomic_fetch_add_explicit(volatile atomic_uint *object, uint operand, memory_order order);
uint __ovld atomic_fetch_add_explicit(volatile atomic_uint *object, uint operand, memory_order order, memory_scope scope);
int __ovld atomic_fetch_sub(volatile atomic_int *object, int operand);
int __ovld atomic_fetch_sub_explicit(volatile atomic_int *object, int operand, memory_order order);
int __ovld atomic_fetch_sub_explicit(volatile atomic_int *object, int operand, memory_order order, memory_scope scope);
uint __ovld atomic_fetch_sub(volatile atomic_uint *object, uint operand);
uint __ovld atomic_fetch_sub_explicit(volatile atomic_uint *object, uint operand, memory_order order);
uint __ovld atomic_fetch_sub_explicit(volatile atomic_uint *object, uint operand, memory_order order, memory_scope scope);
int __ovld atomic_fetch_or(volatile atomic_int *object, int operand);
int __ovld atomic_fetch_or_explicit(volatile atomic_int *object, int operand, memory_order order);
int __ovld atomic_fetch_or_explicit(volatile atomic_int *object, int operand, memory_order order, memory_scope scope);
uint __ovld atomic_fetch_or(volatile atomic_uint *object, uint operand);
uint __ovld atomic_fetch_or_explicit(volatile atomic_uint *object, uint operand, memory_order order);
uint __ovld atomic_fetch_or_explicit(volatile atomic_uint *object, uint operand, memory_order order, memory_scope scope);
int __ovld atomic_fetch_xor(volatile atomic_int *object, int operand);
int __ovld atomic_fetch_xor_explicit(volatile atomic_int *object, int operand, memory_order order);
int __ovld atomic_fetch_xor_explicit(volatile atomic_int *object, int operand, memory_order order, memory_scope scope);
uint __ovld atomic_fetch_xor(volatile atomic_uint *object, uint operand);
uint __ovld atomic_fetch_xor_explicit(volatile atomic_uint *object, uint operand, memory_order order);
uint __ovld atomic_fetch_xor_explicit(volatile atomic_uint *object, uint operand, memory_order order, memory_scope scope);
int __ovld atomic_fetch_and(volatile atomic_int *object, int operand);
int __ovld atomic_fetch_and_explicit(volatile atomic_int *object, int operand, memory_order order);
int __ovld atomic_fetch_and_explicit(volatile atomic_int *object, int operand, memory_order order, memory_scope scope);
uint __ovld atomic_fetch_and(volatile atomic_uint *object, uint operand);
uint __ovld atomic_fetch_and_explicit(volatile atomic_uint *object, uint operand, memory_order order);
uint __ovld atomic_fetch_and_explicit(volatile atomic_uint *object, uint operand, memory_order order, memory_scope scope);
int __ovld atomic_fetch_min(volatile atomic_int *object, int operand);
int __ovld atomic_fetch_min_explicit(volatile atomic_int *object, int operand, memory_order order);
int __ovld atomic_fetch_min_explicit(volatile atomic_int *object, int operand, memory_order order, memory_scope scope);
uint __ovld atomic_fetch_min(volatile atomic_uint *object, uint operand);
uint __ovld atomic_fetch_min_explicit(volatile atomic_uint *object, uint operand, memory_order order);
uint __ovld atomic_fetch_min_explicit(volatile atomic_uint *object, uint operand, memory_order order, memory_scope scope);
uint __ovld atomic_fetch_min(volatile atomic_uint *object, int operand);
uint __ovld atomic_fetch_min_explicit(volatile atomic_uint *object, int operand, memory_order order);
uint __ovld atomic_fetch_min_explicit(volatile atomic_uint *object, int operand, memory_order order, memory_scope scope);
int __ovld atomic_fetch_max(volatile atomic_int *object, int operand);
int __ovld atomic_fetch_max_explicit(volatile atomic_int *object, int operand, memory_order order);
int __ovld atomic_fetch_max_explicit(volatile atomic_int *object, int operand, memory_order order, memory_scope scope);
uint __ovld atomic_fetch_max(volatile atomic_uint *object, uint operand);
uint __ovld atomic_fetch_max_explicit(volatile atomic_uint *object, uint operand, memory_order order);
uint __ovld atomic_fetch_max_explicit(volatile atomic_uint *object, uint operand, memory_order order, memory_scope scope);
uint __ovld atomic_fetch_max(volatile atomic_uint *object, int operand);
uint __ovld atomic_fetch_max_explicit(volatile atomic_uint *object, int operand, memory_order order);
uint __ovld atomic_fetch_max_explicit(volatile atomic_uint *object, int operand, memory_order order, memory_scope scope);

#if defined(cl_khr_int64_base_atomics) && defined(cl_khr_int64_extended_atomics)
long __ovld atomic_fetch_add(volatile atomic_long *object, long operand);
long __ovld atomic_fetch_add_explicit(volatile atomic_long *object, long operand, memory_order order);
long __ovld atomic_fetch_add_explicit(volatile atomic_long *object, long operand, memory_order order, memory_scope scope);
ulong __ovld atomic_fetch_add(volatile atomic_ulong *object, ulong operand);
ulong __ovld atomic_fetch_add_explicit(volatile atomic_ulong *object, ulong operand, memory_order order);
ulong __ovld atomic_fetch_add_explicit(volatile atomic_ulong *object, ulong operand, memory_order order, memory_scope scope);
long __ovld atomic_fetch_sub(volatile atomic_long *object, long operand);
long __ovld atomic_fetch_sub_explicit(volatile atomic_long *object, long operand, memory_order order);
long __ovld atomic_fetch_sub_explicit(volatile atomic_long *object, long operand, memory_order order, memory_scope scope);
ulong __ovld atomic_fetch_sub(volatile atomic_ulong *object, ulong operand);
ulong __ovld atomic_fetch_sub_explicit(volatile atomic_ulong *object, ulong operand, memory_order order);
ulong __ovld atomic_fetch_sub_explicit(volatile atomic_ulong *object, ulong operand, memory_order order, memory_scope scope);
long __ovld atomic_fetch_or(volatile atomic_long *object, long operand);
long __ovld atomic_fetch_or_explicit(volatile atomic_long *object, long operand, memory_order order);
long __ovld atomic_fetch_or_explicit(volatile atomic_long *object, long operand, memory_order order, memory_scope scope);
ulong __ovld atomic_fetch_or(volatile atomic_ulong *object, ulong operand);
ulong __ovld atomic_fetch_or_explicit(volatile atomic_ulong *object, ulong operand, memory_order order);
ulong __ovld atomic_fetch_or_explicit(volatile atomic_ulong *object, ulong operand, memory_order order, memory_scope scope);
long __ovld atomic_fetch_xor(volatile atomic_long *object, long operand);
long __ovld atomic_fetch_xor_explicit(volatile atomic_long *object, long operand, memory_order order);
long __ovld atomic_fetch_xor_explicit(volatile atomic_long *object, long operand, memory_order order, memory_scope scope);
ulong __ovld atomic_fetch_xor(volatile atomic_ulong *object, ulong operand);
ulong __ovld atomic_fetch_xor_explicit(volatile atomic_ulong *object, ulong operand, memory_order order);
ulong __ovld atomic_fetch_xor_explicit(volatile atomic_ulong *object, ulong operand, memory_order order, memory_scope scope);
long __ovld atomic_fetch_and(volatile atomic_long *object, long operand);
long __ovld atomic_fetch_and_explicit(volatile atomic_long *object, long operand, memory_order order);
long __ovld atomic_fetch_and_explicit(volatile atomic_long *object, long operand, memory_order order, memory_scope scope);
ulong __ovld atomic_fetch_and(volatile atomic_ulong *object, ulong operand);
ulong __ovld atomic_fetch_and_explicit(volatile atomic_ulong *object, ulong operand, memory_order order);
ulong __ovld atomic_fetch_and_explicit(volatile atomic_ulong *object, ulong operand, memory_order order, memory_scope scope);
long __ovld atomic_fetch_min(volatile atomic_long *object, long operand);
long __ovld atomic_fetch_min_explicit(volatile atomic_long *object, long operand, memory_order order);
long __ovld atomic_fetch_min_explicit(volatile atomic_long *object, long operand, memory_order order, memory_scope scope);
ulong __ovld atomic_fetch_min(volatile atomic_ulong *object, ulong operand);
ulong __ovld atomic_fetch_min_explicit(volatile atomic_ulong *object, ulong operand, memory_order order);
ulong __ovld atomic_fetch_min_explicit(volatile atomic_ulong *object, ulong operand, memory_order order, memory_scope scope);
ulong __ovld atomic_fetch_min(volatile atomic_ulong *object, long operand);
ulong __ovld atomic_fetch_min_explicit(volatile atomic_ulong *object, long operand, memory_order order);
ulong __ovld atomic_fetch_min_explicit(volatile atomic_ulong *object, long operand, memory_order order, memory_scope scope);
long __ovld atomic_fetch_max(volatile atomic_long *object, long operand);
long __ovld atomic_fetch_max_explicit(volatile atomic_long *object, long operand, memory_order order);
long __ovld atomic_fetch_max_explicit(volatile atomic_long *object, long operand, memory_order order, memory_scope scope);
ulong __ovld atomic_fetch_max(volatile atomic_ulong *object, ulong operand);
ulong __ovld atomic_fetch_max_explicit(volatile atomic_ulong *object, ulong operand, memory_order order);
ulong __ovld atomic_fetch_max_explicit(volatile atomic_ulong *object, ulong operand, memory_order order, memory_scope scope);
ulong __ovld atomic_fetch_max(volatile atomic_ulong *object, long operand);
ulong __ovld atomic_fetch_max_explicit(volatile atomic_ulong *object, long operand, memory_order order);
ulong __ovld atomic_fetch_max_explicit(volatile atomic_ulong *object, long operand, memory_order order, memory_scope scope);
#endif //defined(cl_khr_int64_base_atomics) && defined(cl_khr_int64_extended_atomics)

// OpenCL v2.0 s6.13.11.7.5:
// add/sub: atomic type argument can be uintptr_t/intptr_t, value type argument can be ptrdiff_t.
// or/xor/and/min/max: atomic type argument can be intptr_t/uintptr_t, value type argument can be intptr_t/uintptr_t.

#if defined(cl_khr_int64_base_atomics) && defined(cl_khr_int64_extended_atomics)
uintptr_t __ovld atomic_fetch_add(volatile atomic_uintptr_t *object, ptrdiff_t operand);
uintptr_t __ovld atomic_fetch_add_explicit(volatile atomic_uintptr_t *object, ptrdiff_t operand, memory_order order);
uintptr_t __ovld atomic_fetch_add_explicit(volatile atomic_uintptr_t *object, ptrdiff_t operand, memory_order order, memory_scope scope);
uintptr_t __ovld atomic_fetch_sub(volatile atomic_uintptr_t *object, ptrdiff_t operand);
uintptr_t __ovld atomic_fetch_sub_explicit(volatile atomic_uintptr_t *object, ptrdiff_t operand, memory_order order);
uintptr_t __ovld atomic_fetch_sub_explicit(volatile atomic_uintptr_t *object, ptrdiff_t operand, memory_order order, memory_scope scope);

uintptr_t __ovld atomic_fetch_or(volatile atomic_uintptr_t *object, intptr_t operand);
uintptr_t __ovld atomic_fetch_or_explicit(volatile atomic_uintptr_t *object, intptr_t operand, memory_order order);
uintptr_t __ovld atomic_fetch_or_explicit(volatile atomic_uintptr_t *object, intptr_t operand, memory_order order, memory_scope scope);
uintptr_t __ovld atomic_fetch_xor(volatile atomic_uintptr_t *object, intptr_t operand);
uintptr_t __ovld atomic_fetch_xor_explicit(volatile atomic_uintptr_t *object, intptr_t operand, memory_order order);
uintptr_t __ovld atomic_fetch_xor_explicit(volatile atomic_uintptr_t *object, intptr_t operand, memory_order order, memory_scope scope);
uintptr_t __ovld atomic_fetch_and(volatile atomic_uintptr_t *object, intptr_t operand);
uintptr_t __ovld atomic_fetch_and_explicit(volatile atomic_uintptr_t *object, intptr_t operand, memory_order order);
uintptr_t __ovld atomic_fetch_and_explicit(volatile atomic_uintptr_t *object, intptr_t operand, memory_order order, memory_scope scope);
uintptr_t __ovld atomic_fetch_min(volatile atomic_uintptr_t *object, intptr_t opermax);
uintptr_t __ovld atomic_fetch_min_explicit(volatile atomic_uintptr_t *object, intptr_t opermax, memory_order minder);
uintptr_t __ovld atomic_fetch_min_explicit(volatile atomic_uintptr_t *object, intptr_t opermax, memory_order minder, memory_scope scope);
uintptr_t __ovld atomic_fetch_max(volatile atomic_uintptr_t *object, intptr_t opermax);
uintptr_t __ovld atomic_fetch_max_explicit(volatile atomic_uintptr_t *object, intptr_t opermax, memory_order minder);
uintptr_t __ovld atomic_fetch_max_explicit(volatile atomic_uintptr_t *object, intptr_t opermax, memory_order minder, memory_scope scope);

intptr_t __ovld atomic_fetch_or(volatile atomic_intptr_t *object, uintptr_t operand);
intptr_t __ovld atomic_fetch_or_explicit(volatile atomic_intptr_t *object, uintptr_t operand, memory_order order);
intptr_t __ovld atomic_fetch_or_explicit(volatile atomic_intptr_t *object, uintptr_t operand, memory_order order, memory_scope scope);
intptr_t __ovld atomic_fetch_xor(volatile atomic_intptr_t *object, uintptr_t operand);
intptr_t __ovld atomic_fetch_xor_explicit(volatile atomic_intptr_t *object, uintptr_t operand, memory_order order);
intptr_t __ovld atomic_fetch_xor_explicit(volatile atomic_intptr_t *object, uintptr_t operand, memory_order order, memory_scope scope);
intptr_t __ovld atomic_fetch_and(volatile atomic_intptr_t *object, uintptr_t operand);
intptr_t __ovld atomic_fetch_and_explicit(volatile atomic_intptr_t *object, uintptr_t operand, memory_order order);
intptr_t __ovld atomic_fetch_and_explicit(volatile atomic_intptr_t *object, uintptr_t operand, memory_order order, memory_scope scope);
intptr_t __ovld atomic_fetch_min(volatile atomic_intptr_t *object, uintptr_t opermax);
intptr_t __ovld atomic_fetch_min_explicit(volatile atomic_intptr_t *object, uintptr_t opermax, memory_order minder);
intptr_t __ovld atomic_fetch_min_explicit(volatile atomic_intptr_t *object, uintptr_t opermax, memory_order minder, memory_scope scope);
intptr_t __ovld atomic_fetch_max(volatile atomic_intptr_t *object, uintptr_t opermax);
intptr_t __ovld atomic_fetch_max_explicit(volatile atomic_intptr_t *object, uintptr_t opermax, memory_order minder);
intptr_t __ovld atomic_fetch_max_explicit(volatile atomic_intptr_t *object, uintptr_t opermax, memory_order minder, memory_scope scope);
#endif

// atomic_store()

void __ovld atomic_store(volatile atomic_int *object, int desired);
void __ovld atomic_store_explicit(volatile atomic_int *object, int desired, memory_order order);
void __ovld atomic_store_explicit(volatile atomic_int *object, int desired, memory_order order, memory_scope scope);
void __ovld atomic_store(volatile atomic_uint *object, uint desired);
void __ovld atomic_store_explicit(volatile atomic_uint *object, uint desired, memory_order order);
void __ovld atomic_store_explicit(volatile atomic_uint *object, uint desired, memory_order order, memory_scope scope);
void __ovld atomic_store(volatile atomic_float *object, float desired);
void __ovld atomic_store_explicit(volatile atomic_float *object, float desired, memory_order order);
void __ovld atomic_store_explicit(volatile atomic_float *object, float desired, memory_order order, memory_scope scope);
#if defined(cl_khr_int64_base_atomics) && defined(cl_khr_int64_extended_atomics)
#ifdef cl_khr_fp64
void __ovld atomic_store(volatile atomic_double *object, double desired);
void __ovld atomic_store_explicit(volatile atomic_double *object, double desired, memory_order order);
void __ovld atomic_store_explicit(volatile atomic_double *object, double desired, memory_order order, memory_scope scope);
#endif //cl_khr_fp64
void __ovld atomic_store(volatile atomic_long *object, long desired);
void __ovld atomic_store_explicit(volatile atomic_long *object, long desired, memory_order order);
void __ovld atomic_store_explicit(volatile atomic_long *object, long desired, memory_order order, memory_scope scope);
void __ovld atomic_store(volatile atomic_ulong *object, ulong desired);
void __ovld atomic_store_explicit(volatile atomic_ulong *object, ulong desired, memory_order order);
void __ovld atomic_store_explicit(volatile atomic_ulong *object, ulong desired, memory_order order, memory_scope scope);
#endif

// atomic_load()

int __ovld atomic_load(volatile atomic_int *object);
int __ovld atomic_load_explicit(volatile atomic_int *object, memory_order order);
int __ovld atomic_load_explicit(volatile atomic_int *object, memory_order order, memory_scope scope);
uint __ovld atomic_load(volatile atomic_uint *object);
uint __ovld atomic_load_explicit(volatile atomic_uint *object, memory_order order);
uint __ovld atomic_load_explicit(volatile atomic_uint *object, memory_order order, memory_scope scope);
float __ovld atomic_load(volatile atomic_float *object);
float __ovld atomic_load_explicit(volatile atomic_float *object, memory_order order);
float __ovld atomic_load_explicit(volatile atomic_float *object, memory_order order, memory_scope scope);
#if defined(cl_khr_int64_base_atomics) && defined(cl_khr_int64_extended_atomics)
#ifdef cl_khr_fp64
double __ovld atomic_load(volatile atomic_double *object);
double __ovld atomic_load_explicit(volatile atomic_double *object, memory_order order);
double __ovld atomic_load_explicit(volatile atomic_double *object, memory_order order, memory_scope scope);
#endif //cl_khr_fp64
long __ovld atomic_load(volatile atomic_long *object);
long __ovld atomic_load_explicit(volatile atomic_long *object, memory_order order);
long __ovld atomic_load_explicit(volatile atomic_long *object, memory_order order, memory_scope scope);
ulong __ovld atomic_load(volatile atomic_ulong *object);
ulong __ovld atomic_load_explicit(volatile atomic_ulong *object, memory_order order);
ulong __ovld atomic_load_explicit(volatile atomic_ulong *object, memory_order order, memory_scope scope);
#endif

// atomic_exchange()

int __ovld atomic_exchange(volatile atomic_int *object, int desired);
int __ovld atomic_exchange_explicit(volatile atomic_int *object, int desired, memory_order order);
int __ovld atomic_exchange_explicit(volatile atomic_int *object, int desired, memory_order order, memory_scope scope);
uint __ovld atomic_exchange(volatile atomic_uint *object, uint desired);
uint __ovld atomic_exchange_explicit(volatile atomic_uint *object, uint desired, memory_order order);
uint __ovld atomic_exchange_explicit(volatile atomic_uint *object, uint desired, memory_order order, memory_scope scope);
float __ovld atomic_exchange(volatile atomic_float *object, float desired);
float __ovld atomic_exchange_explicit(volatile atomic_float *object, float desired, memory_order order);
float __ovld atomic_exchange_explicit(volatile atomic_float *object, float desired, memory_order order, memory_scope scope);
#if defined(cl_khr_int64_base_atomics) && defined(cl_khr_int64_extended_atomics)
#ifdef cl_khr_fp64
double __ovld atomic_exchange(volatile atomic_double *object, double desired);
double __ovld atomic_exchange_explicit(volatile atomic_double *object, double desired, memory_order order);
double __ovld atomic_exchange_explicit(volatile atomic_double *object, double desired, memory_order order, memory_scope scope);
#endif //cl_khr_fp64
long __ovld atomic_exchange(volatile atomic_long *object, long desired);
long __ovld atomic_exchange_explicit(volatile atomic_long *object, long desired, memory_order order);
long __ovld atomic_exchange_explicit(volatile atomic_long *object, long desired, memory_order order, memory_scope scope);
ulong __ovld atomic_exchange(volatile atomic_ulong *object, ulong desired);
ulong __ovld atomic_exchange_explicit(volatile atomic_ulong *object, ulong desired, memory_order order);
ulong __ovld atomic_exchange_explicit(volatile atomic_ulong *object, ulong desired, memory_order order, memory_scope scope);
#endif

// atomic_compare_exchange_strong() and atomic_compare_exchange_weak()

bool __ovld atomic_compare_exchange_strong(volatile atomic_int *object, int *expected, int desired);
bool __ovld atomic_compare_exchange_strong_explicit(volatile atomic_int *object, int *expected,
                                                                                 int desired, memory_order success, memory_order failure);
bool __ovld atomic_compare_exchange_strong_explicit(volatile atomic_int *object, int *expected,
                                                                                 int desired, memory_order success, memory_order failure, memory_scope scope);
bool __ovld atomic_compare_exchange_strong(volatile atomic_uint *object, uint *expected, uint desired);
bool __ovld atomic_compare_exchange_strong_explicit(volatile atomic_uint *object, uint *expected,
                                                                                 uint desired, memory_order success, memory_order failure);
bool __ovld atomic_compare_exchange_strong_explicit(volatile atomic_uint *object, uint *expected,
                                                                                 uint desired, memory_order success, memory_order failure, memory_scope scope);
bool __ovld atomic_compare_exchange_weak(volatile atomic_int *object, int *expected, int desired);
bool __ovld atomic_compare_exchange_weak_explicit(volatile atomic_int *object, int *expected,
                                                                                 int desired, memory_order success, memory_order failure);
bool __ovld atomic_compare_exchange_weak_explicit(volatile atomic_int *object, int *expected,
                                                                                 int desired, memory_order success, memory_order failure, memory_scope scope);
bool __ovld atomic_compare_exchange_weak(volatile atomic_uint *object, uint *expected, uint desired);
bool __ovld atomic_compare_exchange_weak_explicit(volatile atomic_uint *object, uint *expected,
                                                                                 uint desired, memory_order success, memory_order failure);
bool __ovld atomic_compare_exchange_weak_explicit(volatile atomic_uint *object, uint *expected,
                                                                                 uint desired, memory_order success, memory_order failure, memory_scope scope);
bool __ovld atomic_compare_exchange_strong(volatile atomic_float *object, float *expected, float desired);
bool __ovld atomic_compare_exchange_strong_explicit(volatile atomic_float *object, float *expected,
                                                                                 float desired, memory_order success, memory_order failure);
bool __ovld atomic_compare_exchange_strong_explicit(volatile atomic_float *object, float *expected,
                                                                                 float desired, memory_order success, memory_order failure, memory_scope scope);
bool __ovld atomic_compare_exchange_weak(volatile atomic_float *object, float *expected, float desired);
bool __ovld atomic_compare_exchange_weak_explicit(volatile atomic_float *object, float *expected,
                                                                                 float desired, memory_order success, memory_order failure);
bool __ovld atomic_compare_exchange_weak_explicit(volatile atomic_float *object, float *expected,
                                                                                 float desired, memory_order success, memory_order failure, memory_scope scope);
#if defined(cl_khr_int64_base_atomics) && defined(cl_khr_int64_extended_atomics)
#ifdef cl_khr_fp64
bool __ovld atomic_compare_exchange_strong(volatile atomic_double *object, double *expected, double desired);
bool __ovld atomic_compare_exchange_strong_explicit(volatile atomic_double *object, double *expected,
                                                                                 double desired, memory_order success, memory_order failure);
bool __ovld atomic_compare_exchange_strong_explicit(volatile atomic_double *object, double *expected,
                                                                                 double desired, memory_order success, memory_order failure, memory_scope scope);
bool __ovld atomic_compare_exchange_weak(volatile atomic_double *object, double *expected, double desired);
bool __ovld atomic_compare_exchange_weak_explicit(volatile atomic_double *object, double *expected,
                                                                                 double desired, memory_order success, memory_order failure);
bool __ovld atomic_compare_exchange_weak_explicit(volatile atomic_double *object, double *expected,
                                                                                 double desired, memory_order success, memory_order failure, memory_scope scope);
#endif //cl_khr_fp64
bool __ovld atomic_compare_exchange_strong(volatile atomic_long *object, long *expected, long desired);
bool __ovld atomic_compare_exchange_strong_explicit(volatile atomic_long *object, long *expected,
                                                                                 long desired, memory_order success, memory_order failure);
bool __ovld atomic_compare_exchange_strong_explicit(volatile atomic_long *object, long *expected,
                                                                                 long desired, memory_order success, memory_order failure, memory_scope scope);
bool __ovld atomic_compare_exchange_weak(volatile atomic_long *object, long *expected, long desired);
bool __ovld atomic_compare_exchange_weak_explicit(volatile atomic_long *object, long *expected,
                                                                                 long desired, memory_order success, memory_order failure);
bool __ovld atomic_compare_exchange_weak_explicit(volatile atomic_long *object, long *expected,
                                                                                 long desired, memory_order success, memory_order failure, memory_scope scope);
bool __ovld atomic_compare_exchange_strong(volatile atomic_ulong *object, ulong *expected, ulong desired);
bool __ovld atomic_compare_exchange_strong_explicit(volatile atomic_ulong *object, ulong *expected,
                                                                                 ulong desired, memory_order success, memory_order failure);
bool __ovld atomic_compare_exchange_strong_explicit(volatile atomic_ulong *object, ulong *expected,
                                                                                 ulong desired, memory_order success, memory_order failure, memory_scope scope);
bool __ovld atomic_compare_exchange_weak(volatile atomic_ulong *object, ulong *expected, ulong desired);
bool __ovld atomic_compare_exchange_weak_explicit(volatile atomic_ulong *object, ulong *expected,
                                                                                 ulong desired, memory_order success, memory_order failure);
bool __ovld atomic_compare_exchange_weak_explicit(volatile atomic_ulong *object, ulong *expected,
                                                                                 ulong desired, memory_order success, memory_order failure, memory_scope scope);
#endif

// atomic_flag_test_and_set() and atomic_flag_clear()

bool __ovld atomic_flag_test_and_set(volatile atomic_flag *object);
bool __ovld atomic_flag_test_and_set_explicit(volatile atomic_flag *object, memory_order order);
bool __ovld atomic_flag_test_and_set_explicit(volatile atomic_flag *object, memory_order order, memory_scope scope);
void __ovld atomic_flag_clear(volatile atomic_flag *object);
void __ovld atomic_flag_clear_explicit(volatile atomic_flag *object, memory_order order);
void __ovld atomic_flag_clear_explicit(volatile atomic_flag *object, memory_order order, memory_scope scope);


/**
 * Return the image array size.
 */
size_t __ovld __cnfn get_image_array_size(read_write image1d_array_t image_array);
size_t __ovld __cnfn get_image_array_size(read_write image2d_array_t image_array);
#ifdef cl_khr_depth_images
size_t __ovld __cnfn get_image_array_size(read_write image2d_array_depth_t image_array);
#endif //cl_khr_depth_images
#if defined(cl_khr_gl_msaa_sharing)
size_t __ovld __cnfn get_image_array_size(read_write image2d_array_msaa_t image_array);
size_t __ovld __cnfn get_image_array_size(read_write image2d_array_msaa_depth_t image_array);
#endif //cl_khr_gl_msaa_sharing

// OpenCL v2.0 s6.13.15 - Work-group Functions

#ifdef cl_khr_fp16
half __ovld __conv work_group_broadcast(half a, size_t local_id);
half __ovld __conv work_group_broadcast(half a, size_t x, size_t y);
half __ovld __conv work_group_broadcast(half a, size_t x, size_t y, size_t z);
#endif
int __ovld __conv work_group_broadcast(int a, size_t local_id);
int __ovld __conv work_group_broadcast(int a, size_t x, size_t y);
int __ovld __conv work_group_broadcast(int a, size_t x, size_t y, size_t z);
uint __ovld __conv work_group_broadcast(uint a, size_t local_id);
uint __ovld __conv work_group_broadcast(uint a, size_t x, size_t y);
uint __ovld __conv work_group_broadcast(uint a, size_t x, size_t y, size_t z);
long __ovld __conv work_group_broadcast(long a, size_t local_id);
long __ovld __conv work_group_broadcast(long a, size_t x, size_t y);
long __ovld __conv work_group_broadcast(long a, size_t x, size_t y, size_t z);
ulong __ovld __conv work_group_broadcast(ulong a, size_t local_id);
ulong __ovld __conv work_group_broadcast(ulong a, size_t x, size_t y);
ulong __ovld __conv work_group_broadcast(ulong a, size_t x, size_t y, size_t z);
float __ovld __conv work_group_broadcast(float a, size_t local_id);
float __ovld __conv work_group_broadcast(float a, size_t x, size_t y);
float __ovld __conv work_group_broadcast(float a, size_t x, size_t y, size_t z);
#ifdef cl_khr_fp64
double __ovld __conv work_group_broadcast(double a, size_t local_id);
double __ovld __conv work_group_broadcast(double a, size_t x, size_t y);
double __ovld __conv work_group_broadcast(double a, size_t x, size_t y, size_t z);
#endif //cl_khr_fp64

// OpenCL v2.0 s6.13.17 - Enqueue Kernels

ndrange_t __ovld ndrange_1D(size_t);
ndrange_t __ovld ndrange_1D(size_t, size_t);
ndrange_t __ovld ndrange_1D(size_t, size_t, size_t);

ndrange_t __ovld ndrange_2D(const size_t[2]);
ndrange_t __ovld ndrange_2D(const size_t[2], const size_t[2]);
ndrange_t __ovld ndrange_2D(const size_t[2], const size_t[2], const size_t[2]);

ndrange_t __ovld ndrange_3D(const size_t[3]);
ndrange_t __ovld ndrange_3D(const size_t[3], const size_t[3]);
ndrange_t __ovld ndrange_3D(const size_t[3], const size_t[3], const size_t[3]);

void __ovld retain_event(clk_event_t);

void __ovld release_event(clk_event_t);

clk_event_t __ovld create_user_event(void);

void __ovld set_user_event_status(clk_event_t e, int state);

bool __ovld is_valid_event (clk_event_t event);

void __ovld capture_event_profiling_info(clk_event_t, clk_profiling_info, __global void* value);

queue_t __ovld get_default_queue(void);

int __ovld enqueue_marker(queue_t, uint, const clk_event_t*, clk_event_t*);

#endif // _OPENCL_PLATFORM_H_
