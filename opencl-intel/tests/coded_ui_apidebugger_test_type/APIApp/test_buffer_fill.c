/******************************************************************
 //
 //  OpenCL Conformance Tests
 // 
 //  Copyright:	(c) 2008-2013 by Apple Inc. All Rights Reserved.
 //
 ******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <string.h>
//#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "procs.h"
#include "harness/errorHelpers.h"

const cl_mem_flags flag_set[] = {
    CL_MEM_ALLOC_HOST_PTR, 
    CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR,
    CL_MEM_USE_HOST_PTR,
    CL_MEM_COPY_HOST_PTR,
    0
};
const char* flag_set_names[] = {
    "CL_MEM_ALLOC_HOST_PTR", 
    "CL_MEM_ALLOC_HOST_PTR | CL_MEM_COPY_HOST_PTR",
    "CL_MEM_USE_HOST_PTR",
    "CL_MEM_COPY_HOST_PTR",
    "0"
};  

#define NUM_FLAGS 1

#define USE_LOCAL_WORK_GROUP	1

#define TEST_PRIME_CHAR		0x77
#define TEST_PRIME_INT		((1<<16)+1)
#define TEST_PRIME_UINT		((1U<<16)+1U)
#define TEST_PRIME_LONG		((1LL<<32)+1LL)
#define TEST_PRIME_ULONG	((1ULL<<32)+1ULL)
#define TEST_PRIME_SHORT	(cl_short)((1<<8)+1)
#define TEST_PRIME_USHORT   (cl_ushort)((1<<8)+1)
#define TEST_PRIME_FLOAT	(cl_float)3.40282346638528860e+38
#define TEST_PRIME_HALF		119.f

#ifndef TestStruct
typedef struct{
    cl_int     a;
    cl_float   b;
} TestStruct;
#endif

const char *buffer_fill_int_kernel_code[] = {
    "__kernel void test_buffer_fill_int(__global int *src, __global int *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_int2(__global int2 *src, __global int2 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_int4(__global int4 *src, __global int4 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_int8(__global int8 *src, __global int8 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_int16(__global int16 *src, __global int16 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n" };

static const char *int_kernel_name[] = { "test_buffer_fill_int", "test_buffer_fill_int2", "test_buffer_fill_int4", "test_buffer_fill_int8", "test_buffer_fill_int16" };


const char *buffer_fill_uint_kernel_code[] = {
    "__kernel void test_buffer_fill_uint(__global uint *src, __global uint *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_uint2(__global uint2 *src, __global uint2 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_uint4(__global uint4 *src, __global uint4 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_uint8(__global uint8 *src, __global uint8 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_uint16(__global uint16 *src, __global uint16 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n" };

static const char *uint_kernel_name[] = { "test_buffer_fill_uint", "test_buffer_fill_uint2", "test_buffer_fill_uint4", "test_buffer_fill_uint8", "test_buffer_fill_uint16" };


const char *buffer_fill_short_kernel_code[] = {
    "__kernel void test_buffer_fill_short(__global short *src, __global short *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_short2(__global short2 *src, __global short2 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_short4(__global short4 *src, __global short4 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_short8(__global short8 *src, __global short8 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_short16(__global short16 *src, __global short16 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n" };

static const char *short_kernel_name[] = { "test_buffer_fill_short", "test_buffer_fill_short2", "test_buffer_fill_short4", "test_buffer_fill_short8", "test_buffer_fill_short16" };


const char *buffer_fill_ushort_kernel_code[] = {
    "__kernel void test_buffer_fill_ushort(__global ushort *src, __global ushort *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_ushort2(__global ushort2 *src, __global ushort2 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_ushort4(__global ushort4 *src, __global ushort4 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_ushort8(__global ushort8 *src, __global ushort8 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_ushort16(__global ushort16 *src, __global ushort16 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n" };

static const char *ushort_kernel_name[] = { "test_buffer_fill_ushort", "test_buffer_fill_ushort2", "test_buffer_fill_ushort4", "test_buffer_fill_ushort8", "test_buffer_fill_ushort16" };


const char *buffer_fill_char_kernel_code[] = {
    "__kernel void test_buffer_fill_char(__global char *src, __global char *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_char2(__global char2 *src, __global char2 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_char4(__global char4 *src, __global char4 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_char8(__global char8 *src, __global char8 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_char16(__global char16 *src, __global char16 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n" };

static const char *char_kernel_name[] = { "test_buffer_fill_char", "test_buffer_fill_char2", "test_buffer_fill_char4", "test_buffer_fill_char8", "test_buffer_fill_char16" };


const char *buffer_fill_uchar_kernel_code[] = {
    "__kernel void test_buffer_fill_uchar(__global uchar *src, __global uchar *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_uchar2(__global uchar2 *src, __global uchar2 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_uchar4(__global uchar4 *src, __global uchar4 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_uchar8(__global uchar8 *src, __global uchar8 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_uchar16(__global uchar16 *src, __global uchar16 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n" };

static const char *uchar_kernel_name[] = { "test_buffer_fill_uchar", "test_buffer_fill_uchar2", "test_buffer_fill_uchar4", "test_buffer_fill_uchar8", "test_buffer_fill_uchar16" };


const char *buffer_fill_long_kernel_code[] = {
    "__kernel void test_buffer_fill_long(__global long *src, __global long *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_long2(__global long2 *src, __global long2 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_long4(__global long4 *src, __global long4 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_long8(__global long8 *src, __global long8 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_long16(__global long16 *src, __global long16 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n" };

static const char *long_kernel_name[] = { "test_buffer_fill_long", "test_buffer_fill_long2", "test_buffer_fill_long4", "test_buffer_fill_long8", "test_buffer_fill_long16" };


const char *buffer_fill_ulong_kernel_code[] = {
    "__kernel void test_buffer_fill_ulong(__global ulong *src, __global ulong *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_ulong2(__global ulong2 *src, __global ulong2 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_ulong4(__global ulong4 *src, __global ulong4 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_ulong8(__global ulong8 *src, __global ulong8 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_ulong16(__global ulong16 *src, __global ulong16 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n" };

static const char *ulong_kernel_name[] = { "test_buffer_fill_ulong", "test_buffer_fill_ulong2", "test_buffer_fill_ulong4", "test_buffer_fill_ulong8", "test_buffer_fill_ulong16" };


const char *buffer_fill_float_kernel_code[] = {
    "__kernel void test_buffer_fill_float(__global float *src, __global float *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_float2(__global float2 *src, __global float2 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_float4(__global float4 *src, __global float4 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_float8(__global float8 *src, __global float8 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n",
    
    "__kernel void test_buffer_fill_float16(__global float16 *src, __global float16 *dst)\n"
    "{\n"
    "    int  tid = get_global_id(0);\n"
    "\n"
    "    dst[tid] = src[tid];\n"
    "}\n" };

static const char *float_kernel_name[] = { "test_buffer_fill_float", "test_buffer_fill_float2", "test_buffer_fill_float4", "test_buffer_fill_float8", "test_buffer_fill_float16" };


static const char *struct_kernel_code = 
"typedef struct{\n"
"int	a;\n"
"float	b;\n"
"} TestStruct;\n"
"__kernel void read_fill_struct(__global TestStruct *src, __global TestStruct *dst)\n"
"{\n"
"    int  tid = get_global_id(0);\n"
"\n"
"    dst[tid].a = src[tid].a;\n"
"	 dst[tid].b = src[tid].b;\n"
"}\n";



static int verify_fill_struct( void *ptr1, void *ptr2, int n )
{
    int         i;
    TestStruct  *inptr = (TestStruct *)ptr1;
    TestStruct  *outptr = (TestStruct *)ptr2;
    
    for (i=0; i<n; i++){
        if ( ( outptr[i].a != inptr[i].a ) || ( outptr[i].b != outptr[i].b ) )
            return -1;
    }
    
    return 0;
}

int test_buffer_fill_struct( cl_device_id deviceID, cl_context context, cl_command_queue queue, int num_elements )
{
    cl_mem      buffers[2];
    void        *outptr;
    TestStruct  *inptr;
    TestStruct  *hostptr;
    TestStruct  *pattern;
    cl_program  program;
    cl_kernel   kernel;
    cl_event    event[2];
    size_t      ptrSize = sizeof( TestStruct );
    size_t      global_work_size[3];
#ifdef USE_LOCAL_WORK_GROUP
    size_t      local_work_size[3];
#endif
    int         n, err;
    size_t      j, offset_elements, fill_elements;
    int         src_flag_id;
    int         total_errors = 0;
    MTdata      d = init_genrand( gRandomSeed );
    
    size_t      min_alignment = get_min_alignment(context);
    
    global_work_size[0] = (size_t)num_elements;
    
    // Test with random offsets and fill sizes
    for ( n = 0; n < 1/*8*/; n++ ){
        offset_elements = (size_t)get_random_float( 0.f, (float)(num_elements - 8), d );
        fill_elements = (size_t)get_random_float( 8.f, (float)(num_elements - offset_elements), d );
        log_info( "Testing random fill from offset %d for %d elements: \n", (int)offset_elements, (int)fill_elements );
        
        pattern = (TestStruct *)malloc(ptrSize);
        pattern->a = (cl_int)genrand_int32(d);
        pattern->b = (cl_float)get_random_float( -FLT_MAX, FLT_MAX, d );
        
        inptr = (TestStruct *)align_malloc(ptrSize * num_elements, min_alignment);
        for ( j = 0; j < offset_elements; j++ ) {
            inptr[j].a = 0;
            inptr[j].b =0;
        }
        for ( j = offset_elements; j < offset_elements + fill_elements; j++ ) {
            inptr[j].a = pattern->a;
            inptr[j].b = pattern->b;
        }
        for ( j = offset_elements + fill_elements; j < (size_t)num_elements; j++ ) {
            inptr[j].a = 0;
            inptr[j].b = 0;
        }
        
        hostptr = (TestStruct *)align_malloc(ptrSize * num_elements, min_alignment);
        memset(hostptr, 0, ptrSize * num_elements);
        
        for (src_flag_id=0; src_flag_id < NUM_FLAGS; src_flag_id++) {
            log_info("Testing with cl_mem_flags: %s\n", flag_set_names[src_flag_id]);
            
            if ((flag_set[src_flag_id] & CL_MEM_USE_HOST_PTR) || (flag_set[src_flag_id] & CL_MEM_COPY_HOST_PTR))
                buffers[0] = clCreateBuffer(context, flag_set[src_flag_id],  ptrSize * num_elements, hostptr, &err);
            else
                buffers[0] = clCreateBuffer(context, flag_set[src_flag_id],  ptrSize * num_elements, NULL, &err);
            if ( err ){
                print_error(err, " clCreateBuffer failed\n" );
                clReleaseEvent( event[0] );
                clReleaseEvent( event[1] );
                free( (void *)pattern );
                align_free( (void *)inptr );
                align_free( (void *)hostptr );
                free_mtdata(d);
                return -1;
            }
            if (!((flag_set[src_flag_id] & CL_MEM_USE_HOST_PTR) || (flag_set[src_flag_id] & CL_MEM_COPY_HOST_PTR))) {
                err = clEnqueueWriteBuffer(queue, buffers[0], CL_FALSE, 0, ptrSize * num_elements, hostptr, 0, NULL, NULL);
                if ( err != CL_SUCCESS ){
                    print_error(err, " clEnqueueWriteBuffer failed\n" );
                    clReleaseEvent( event[0] );
                    clReleaseEvent( event[1] );
                    free( (void *)pattern );
                    align_free( (void *)inptr );
                    align_free( (void *)hostptr );
                    free_mtdata(d);
                    return -1;
                }
            }
            outptr = align_malloc( ptrSize * num_elements, min_alignment);
            memset(outptr, 0, ptrSize * num_elements);
            buffers[1] = clCreateBuffer(context, CL_MEM_COPY_HOST_PTR,  ptrSize * num_elements, outptr, &err);
            if ( ! buffers[1] || err){
                print_error(err, " clCreateBuffer failed\n" );
                align_free( outptr );
                clReleaseMemObject(buffers[0]);
                clReleaseEvent( event[0] );
                clReleaseEvent( event[1] );
                free( (void *)pattern );
                align_free( (void *)inptr );
                align_free( (void *)hostptr );
                free_mtdata(d);
                return -1;
            }

            err = clEnqueueFillBuffer(queue, NULL, pattern, ptrSize, 
                                      ptrSize * offset_elements, ptrSize * fill_elements,
                                      0, NULL, &(event[0]));
            err = clEnqueueFillBuffer(queue, buffers[0], pattern, ptrSize, 
                                      ptrSize * offset_elements, ptrSize * fill_elements,
                                      0, NULL, &(event[0]));
            /* uncomment for test debugging
             err = clEnqueueWriteBuffer(queue, buffers[0], CL_FALSE, 0, ptrSize * num_elements, inptr, 0, NULL, &(event[0]));
             */
            if ( err != CL_SUCCESS ){
                print_error( err, " clEnqueueFillBuffer failed" );
                align_free( outptr );
                clReleaseMemObject(buffers[0]);
                clReleaseMemObject(buffers[1]);
                clReleaseEvent( event[0] );
                clReleaseEvent( event[1] );
                free( (void *)pattern );
                align_free( (void *)inptr );
                align_free( (void *)hostptr );
                free_mtdata(d);
                return -1;
            }
            
            err = create_single_kernel_helper( context, &program, &kernel, 1, &struct_kernel_code, "read_fill_struct" );
            if ( err ){
                log_error( " Error creating program for struct\n" );
                align_free( outptr );
                clReleaseMemObject(buffers[0]);
                clReleaseMemObject(buffers[1]);
                clReleaseEvent( event[0] );
                clReleaseEvent( event[1] );
                free( (void *)pattern );
                align_free( (void *)inptr );
                align_free( (void *)hostptr );
                free_mtdata(d);
                return -1;
            }
            
#ifdef USE_LOCAL_WORK_GROUP
            err = get_max_common_work_group_size( context, kernel, global_work_size[0], &local_work_size[0] );
            test_error( err, "Unable to get work group size to use" );
#endif
            
            err = clSetKernelArg( kernel, 0, sizeof( cl_mem ), (void *)&buffers[0] );
            err |= clSetKernelArg( kernel, 1, sizeof( cl_mem ), (void *)&buffers[1] );
            if ( err != CL_SUCCESS ){
                print_error( err, " clSetKernelArg failed" );
                clReleaseKernel( kernel );
                clReleaseProgram( program );
                align_free( outptr );
                clReleaseMemObject(buffers[0]);
                clReleaseMemObject(buffers[1]);
                clReleaseEvent( event[0] );
                clReleaseEvent( event[1] );
                free( (void *)pattern );
                align_free( (void *)inptr );
                align_free( (void *)hostptr );
                free_mtdata(d);
                return -1;
            }
            
            err = clWaitForEvents(  1, &(event[0]) );
            if ( err != CL_SUCCESS ){
                print_error( err, "clWaitForEvents() failed" );
                clReleaseKernel( kernel );
                clReleaseProgram( program );
                align_free( outptr );
                clReleaseMemObject(buffers[0]);
                clReleaseMemObject(buffers[1]);
                clReleaseEvent( event[0] );
                clReleaseEvent( event[1] );
                free( (void *)pattern );
                align_free( (void *)inptr );
                align_free( (void *)hostptr );
                free_mtdata(d);
                return -1;
            }
            clReleaseEvent( event[0] );
            
#ifdef USE_LOCAL_WORK_GROUP
            err = clEnqueueNDRangeKernel( queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL );
#else
            err = clEnqueueNDRangeKernel( queue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL );
#endif
            if ( err != CL_SUCCESS ){
                print_error( err, " clEnqueueNDRangeKernel failed" );
                clReleaseKernel( kernel );
                clReleaseProgram( program );
                align_free( outptr );
                clReleaseMemObject(buffers[0]);
                clReleaseMemObject(buffers[1]);
                clReleaseEvent( event[0] );
                clReleaseEvent( event[1] );
                free( (void *)pattern );
                align_free( (void *)inptr );
                align_free( (void *)hostptr );
                free_mtdata(d);
                return -1;
            }
            
            err = clEnqueueReadBuffer( queue, buffers[1], CL_FALSE, 0, ptrSize * num_elements, outptr, 0, NULL, &(event[1]) );
            if ( err != CL_SUCCESS ){
                print_error( err, " clEnqueueReadBuffer failed" );
                clReleaseKernel( kernel );
                clReleaseProgram( program );
                align_free( outptr );
                clReleaseMemObject(buffers[0]);
                clReleaseMemObject(buffers[1]);
                clReleaseEvent( event[0] );
                clReleaseEvent( event[1] );
                free( (void *)pattern );
                align_free( (void *)inptr );
                align_free( (void *)hostptr );
                free_mtdata(d);
                return -1;
            }
            
            err = clWaitForEvents( 1, &(event[1]) );
            if ( err != CL_SUCCESS ){
                print_error( err, "clWaitForEvents() failed" );
            }
            clReleaseEvent( event[1] );
            
            if ( verify_fill_struct( inptr, outptr, num_elements) ) {
                log_error( " buffer_FILL async struct test failed\n" );
                total_errors++;
            }
            else{
                log_info( " buffer_FILL async struct test passed\n" );
            }
            // cleanup
            clReleaseKernel( kernel );
            clReleaseProgram( program );
            align_free( outptr );
            clReleaseMemObject( buffers[0] );
            clReleaseMemObject( buffers[1] );
        } // src cl_mem_flag
        free( (void *)pattern );
        align_free( (void *)inptr );
        align_free( (void *)hostptr );
    }
    
    free_mtdata(d);
    
    return total_errors;
    
}   // end test_buffer_fill_struct()

