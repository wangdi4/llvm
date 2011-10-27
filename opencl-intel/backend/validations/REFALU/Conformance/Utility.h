/******************************************************************
//
//  OpenCL Conformance Tests
//
//  Copyright:  (c) 2008-2009 by Apple Inc. All Rights Reserved.
//
******************************************************************/


#ifndef UTILITY_H
#define UTILITY_H

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include "CL/cl.h"
#endif
#include <stdio.h>
#if !defined(_MSC_VER)
#include <stdint.h>
#endif

#include "test_common/compat.h"
#include "test_common/rounding_mode.h"
#include <float.h>
#include <math.h>

#if defined( _WIN32) && defined (_MSC_VER)
//#include "../../test_common/harness/testHarness.h"
#endif

//#include "../../test_common/harness/compat.h"
//#include "../../test_common/harness/ThreadPool.h"
#define BUFFER_SIZE         (1024*1024*2)

#if defined( __GNUC__ )
    #define UNUSED  __attribute__ ((unused))
#else
    #define UNUSED
#endif

#define VECTOR_SIZE_COUNT   6
extern const char *sizeNames[VECTOR_SIZE_COUNT];
extern const int   sizeValues[VECTOR_SIZE_COUNT];

extern cl_device_type   gDeviceType;
extern cl_device_id     gDevice;
extern cl_context       gContext;
extern cl_command_queue gQueue;
extern void             *gIn;
extern void             *gIn2;
extern void             *gIn3;
extern void             *gOut_Ref;
extern void             *gOut_Ref2;
extern void             *gOut[VECTOR_SIZE_COUNT];
extern void             *gOut2[VECTOR_SIZE_COUNT];
extern cl_mem           gInBuffer;
extern cl_mem           gInBuffer2;
extern cl_mem           gInBuffer3;
extern cl_mem           gOutBuffer[VECTOR_SIZE_COUNT];
extern cl_mem           gOutBuffer2[VECTOR_SIZE_COUNT];
extern uint32_t         gComputeDevices;
extern uint32_t         gSimdSize;
extern int              gSkipCorrectnessTesting;
extern int              gMeasureTimes;
extern int              gReportAverageTimes;
extern int              gForceFTZ;
extern int              gWimpyMode;
extern int              gHasDouble;
extern int              gIsInRTZMode;
extern int              gInfNanSupport;
extern int              gIsEmbedded;
extern uint32_t         gMaxVectorSizeIndex;
extern uint32_t         gMinVectorSizeIndex;
extern uint32_t         gDeviceFrequency;
extern cl_device_fp_config gFloatCapabilities;
extern cl_device_fp_config gDoubleCapabilities;

#if !defined( _MSC_VER)
    #include <fenv.h>
#endif

#define LOWER_IS_BETTER     0
#define HIGHER_IS_BETTER    1

//#if USE_ATF
//
//    #include <ATF/ATF.h>
//    #define test_start()        ATFTestStart()
//    #define test_finish()       ATFTestFinish()
//    #define vlog( ... )         ATFLogInfo(__VA_ARGS__)
//    #define vlog_error( ... )   ATFLogError(__VA_ARGS__)
//    #define vlog_perf( _number, _higherIsBetter, _units, _nameFmt, ... )    ATFLogPerformanceNumber(_number, _higherIsBetter, _units, _nameFmt, __VA_ARGS__ )
//
//#else
//
//    #define test_start()
//    #define test_finish()
//    #define vlog( ... )         printf( __VA_ARGS__ )
//    #define vlog_error( ... ) printf( __VA_ARGS__ )
//    #define vlog_perf( _number, _higherIsBetter, _units, _nameFmt, ... )  printf( "\t%8.2f", _number )
//
//    void _logPerf(double number, int higherIsBetter, const char *units, const char *nameFormat, ...);
//#endif

#if defined (_MSC_VER )
    //Deal with missing scalbn on windows
    #define scalbnf( _a, _i )       ldexpf( _a, _i )
    #define scalbn( _a, _i )        ldexp( _a, _i )
    #define scalbnl( _a, _i )       ldexpl( _a, _i )
#endif

#ifdef __cplusplus
extern "C" {
#endif
float Ulp_Error( float test, double reference );
//float Ulp_Error_Half( float test, double reference );
float Ulp_Error_Double( double test, long double reference );
#ifdef __cplusplus
} //extern "C"
#endif

uint64_t GetTime( void );
double SubtractTime( uint64_t endTime, uint64_t startTime );
int MakeKernel( const char **c, cl_uint count, const char *name, cl_kernel *k, cl_program *p );
int MakeKernels( const char **c, cl_uint count, const char *name, cl_uint kernel_count, cl_kernel *k, cl_program *p );

// used to convert a bucket of bits into a search pattern through double
static inline double DoubleFromUInt32( uint32_t bits );
static inline double DoubleFromUInt32( uint32_t bits )
{
    union{ uint64_t u; double d;} u;

    // split 0x89abcdef to 0x89abc00000000def
    u.u = bits & 0xfffU;
    u.u |= (uint64_t) (bits & ~0xfffU) << 32;

    // sign extend the leading bit of def segment as sign bit so that the middle region consists of either all 1s or 0s
    u.u -= (bits & 0x800U) << 1;

    // return result
    return u.d;
}

void _LogBuildError( cl_program p, int line, const char *file );
#define LogBuildError( program )        _LogBuildError( program, __LINE__, __FILE__ )

#ifndef MAX
    #define MAX(_a, _b)     ((_a) > (_b) ? (_a) : (_b))
#endif
#ifndef MIN
    #define MIN(_a, _b)     ((_a) < (_b) ? (_a) : (_b))
#endif

#define PERF_LOOP_COUNT 100

// Note: though this takes a double, this is for use with single precision tests
static inline int IsFloatSubnormal( double x )
{
#if 2 == FLT_RADIX
    // Do this in integer to avoid problems with FTZ behavior
    union{ float d; uint32_t u;}u;
    u.d = fabsf((float)x);
    return (u.u-1) < 0x007fffffU;
#else
    // rely on floating point hardware for non-radix2 non-IEEE-754 hardware -- will fail if you flush subnormals to zero
    return fabs(x) < (double) FLT_MIN && x != 0.0;
#endif
}


static inline int IsDoubleSubnormal( long double x )
{
#if 2 == FLT_RADIX
    // Do this in integer to avoid problems with FTZ behavior
    union{ double d; uint64_t u;}u;
    u.d = fabs((double) x);
    return (u.u-1) < 0x000fffffffffffffULL;
#else
    // rely on floating point hardware for non-radix2 non-IEEE-754 hardware -- will fail if you flush subnormals to zero
    return fabs(x) < (double) DBL_MIN && x != 0.0;
#endif
}

//The spec is fairly clear that we may enforce a hard cutoff to prevent premature flushing to zero.
// However, to avoid conflict for 1.0, we are letting results at TYPE_MIN + ulp_limit to be flushed to zero.
static inline int IsFloatResultSubnormal( double x, float ulps )
{
    //x = fabs(x) - MAKE_HEX_DOUBLE( 0x1.0p-149, 0x1, -149) * (double) ulps;
    x=MAKE_HEX_DOUBLE( 0.01, 0x1, -149);
    return x < MAKE_HEX_DOUBLE( 0x1.0p-126, 0x1, -126 );
}

static inline int IsDoubleResultSubnormal( long double x, float ulps )
{
    x = fabsl(x) - MAKE_HEX_LONG( 0x1.0p-1074, 0x1, -1074) * (long double) ulps;
    return x < MAKE_HEX_LONG( 0x1.0p-1022, 0x1, -1022 );
}

static inline int IsFloatInfinity(double x)
{
  union { cl_float d; cl_uint u; } u;
  u.d = (cl_float) x;
  return ((u.u & 0x7fffffffU) == 0x7F800000U);
}

static inline int IsFloatMaxFloat(double x)
{
  union { cl_float d; cl_uint u; } u;
  u.d = (cl_float) x;
  return ((u.u & 0x7fffffffU) == 0x7F7FFFFFU);
}

static inline int IsFloatNaN(double x)
{
  union { cl_float d; cl_uint u; } u;
  u.d = (cl_float) x;
  return ((u.u & 0x7fffffffU) > 0x7F800000U);
}

extern cl_uint RoundUpToNextPowerOfTwo( cl_uint x );


// In order to get tests for correctly rounded operations (e.g. multiply) to work properly we need to be able to set the reference hardware
// to FTZ mode if the device hardware is running in that mode.  We have explored all other options short of writing correctly rounded operations
// in integer code, and have found this is the only way to correctly verify operation.
//
// Non-Apple implementations will need to provide their own implentation for these features.  If the reference hardware and device are both
// running in the same state (either FTZ or IEEE compliant modes) then these functions may be empty.  If the device is running in non-default
// rounding mode (e.g. round toward zero), then these functions should also set the reference device into that rounding mode.
#if defined( __APPLE__ ) || defined( _MSC_VER ) || defined( __linux__ ) || defined (__MINGW32__)
    typedef int     FPU_mode_type;
#if defined( __i386__ ) || defined( __x86_64__ )
    #include <xmmintrin.h>
#elif defined( __PPC__ )
    #include <fpu_control.h>
#endif
    // Set the reference hardware floating point unit to FTZ mode
    static inline void ForceFTZ( FPU_mode_type *mode )
    {
#if defined( __i386__ ) || defined( __x86_64__ ) || defined( _MSC_VER ) || defined (__MINGW32__)
        *mode = _mm_getcsr();
        _mm_setcsr( *mode | 0x8040);
#elif defined( __PPC__ )
        fpu_control_t flags = 0;
        _FPU_GETCW(flags);
        flags |= _FPU_MASK_NI;
        _FPU_SETCW(flags);
#else
    #error ForceFTZ needs an implentation
#endif
    }

    // Restore the reference hardware to floating point state indicated by *mode
    static inline void RestoreFPState( FPU_mode_type *mode )
    {
#if defined( __i386__ ) || defined( __x86_64__ ) || defined( _MSC_VER ) || defined (__MINGW32__)
        _mm_setcsr( *mode );
#elif defined( __PPC__)
        fpu_control_t flags = 0;
        _FPU_GETCW(flags);
        flags &= ~_FPU_MASK_NI;
        _FPU_SETCW(flags);
#else
    #error RestoreFPState needs an implementation
#endif
    }
#else
    #error ForceFTZ and RestoreFPState need implentations
#endif

#ifdef __cplusplus
extern "C"
#else
extern
#endif
void memset_pattern4(void *dest, const void *src_pattern, size_t bytes );

typedef union
{
    int32_t i;
    float   f;
}int32f_t;

typedef union
{
    int64_t l;
    double  d;
}int64d_t;

void MulD(double *rhi, double *rlo, double u, double v);
void AddD(double *rhi, double *rlo, double a, double b);
void MulDD(double *rhi, double *rlo, double xh, double xl, double yh, double yl);
void AddDD(double *rhi, double *rlo, double xh, double xl, double yh, double yl);
int compareFloats(float x, float y);
int compareDoubles(double x, double y);

#endif /* UTILITY_H */


