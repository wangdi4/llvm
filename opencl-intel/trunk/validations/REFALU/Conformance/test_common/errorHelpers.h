/******************************************************************
//
//  OpenCL Conformance Tests
//
//  Copyright:  (c) 2008-2009 by Apple Inc. All Rights Reserved.
//
******************************************************************/

#ifndef _errorHelpers_h
#define _errorHelpers_h

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif
#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif

#define LOWER_IS_BETTER     0
#define HIGHER_IS_BETTER    1

// If USE_ATF is defined, all log_error and log_info calls can be routed to test library
// functions as described below. This is helpful for integration into an automated testing
// system.
#if USE_ATF
// export BUILD_WITH_ATF=1
    #include <ATF/ATF.h>
    #define test_start() ATFTestStart()
    #define log_info ATFLogInfo
    #define log_error ATFLogError
    #define log_perf(_number, _higherBetter, _numType, _format, ...) ATFLogPerformanceNumber(_number, _higherBetter, _numType, _format, ##__VA_ARGS__)
    #define test_finish() ATFTestFinish()
    #define vlog_perf(_number, _higherBetter, _numType, _format, ...) ATFLogPerformanceNumber(_number, _higherBetter, _numType, _format,##__VA_ARGS__)
    #define vlog ATFLogInfo
    #define vlog_error ATFLogError
#else
    #define test_start()
    #define log_info printf
    #define log_error printf
    #define log_perf(_number, _higherBetter, _numType, _format, ...) printf("Performance Number " _format " (in %s, %s): %g\n",##__VA_ARGS__, _numType,     \
                        _higherBetter?"higher is better":"lower is better", _number )
    #define test_finish()
    #define vlog_perf(_number, _higherBetter, _numType, _format, ...) printf("Performance Number " _format " (in %s, %s): %g\n",##__VA_ARGS__, _numType,    \
                        _higherBetter?"higher is better":"lower is better" , _number)
    #ifdef _WIN32
        #ifdef __MINGW32__
            // Use __mingw_printf since it supports "%a" format specifier
            #define vlog __mingw_printf
            #define vlog_error __mingw_printf
        #else
            // Use home-baked function that treats "%a" as "%f"
        static int vlog_win32(const char *format, ...);
        #define vlog vlog_win32
        #define vlog_error vlog_win32
        #endif
    #else
        #define vlog_error printf
        #define vlog printf
    #endif
#endif

#define ct_assert(b)          ct_assert_i(b, __LINE__)
#define ct_assert_i(b, line)  ct_assert_ii(b, line)
#define ct_assert_ii(b, line) int _compile_time_assertion_on_line_##line[b ? 1 : -1];

#define test_error(errCode,msg) test_error_ret(errCode,msg,errCode)
#define test_error_ret(errCode,msg,retValue)    { if( errCode != CL_SUCCESS ) { print_error( errCode, msg ); return retValue ; } }
#define print_error(errCode,msg)    log_error( "ERROR: %s! (%s from %s:%d)\n", msg, IGetErrorString( errCode ), __FILE__, __LINE__ );


extern const char   *IGetErrorString( int clErrorCode );

extern float Ulp_Error_Half( cl_ushort test, float reference );
extern float Ulp_Error( float test, double reference );
extern float Ulp_Error_Double( double test, long double reference );

extern const char *GetChannelTypeName( cl_channel_type type );
extern const char *GetChannelOrderName( cl_channel_order order );
extern const char *GetAddressModeName( cl_addressing_mode mode );

extern const char *GetDeviceTypeName( cl_device_type type );

// NON-REENTRANT UNLESS YOU PROVIDE A BUFFER PTR (pass null to use static storage, but it's not reentrant then!)
extern const char *GetDataVectorString( void *dataBuffer, size_t typeSize, size_t vecSize, char *buffer );

#if defined (_WIN32) && !defined(__MINGW32__)
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
static int vlog_win32(const char *format, ...)
{
    const char *new_format = format;

    if (strstr(format, "%a")) {
        char *temp;
        if ((temp = strdup(format)) == NULL) {
            printf("vlog_win32: Failed to allocate memory for strdup\n");
            return -1;
        }
        new_format = temp;
        while (*temp) {
            // replace %a with %f
            if ((*temp == '%') && (*(temp+1) == 'a')) {
                *(temp+1) = 'f';
            }
            temp++;
        }
    }

    va_list args;
    va_start(args, format);
    vprintf(new_format, args);
    va_end(args);

    if (new_format != format) {
        free((void*)new_format);
    }

    return 0;
}
#endif


#ifdef __cplusplus
}
#endif

#endif // _errorHelpers_h


