#ifndef _OPENCL_PLATFORM_H_
#define _OPENCL_PLATFORM_H_

#include "opencl-c-common.h"

// opencl-c-common.h is already contains this statement, but enabled OpenCL
// extensions are not loaded from PCH
#ifdef cl_khr_fp16
#pragma OPENCL EXTENSION cl_khr_fp16 : enable
#endif


/**
 * The unsigned integer type of the result of the sizeof operator. This
 * is a 32-bit unsigned integer if CL_DEVICE_ADDRESS_BITS
 * defined in table 4.3 is 32-bits and is a 64-bit unsigned integer if
 * CL_DEVICE_ADDRESS_BITS is 64-bits.
 */
typedef __SIZE_TYPE__ size_t;

/**
 * A signed integer type that is the result of subtracting two pointers.
 * This is a 32-bit signed integer if CL_DEVICE_ADDRESS_BITS
 * defined in table 4.3 is 32-bits and is a 64-bit signed integer if
 * CL_DEVICE_ADDRESS_BITS is 64-bits.
 */
typedef __PTRDIFF_TYPE__ ptrdiff_t;

/**
* A signed integer type with the property that any valid pointer to
* void can be converted to this type, then converted back to pointer
* to void, and the result will compare equal to the original pointer.
*/
typedef __INTPTR_TYPE__ intptr_t;

/**
* An unsigned integer type with the property that any valid pointer to
* void can be converted to this type, then converted back to pointer
* to void, and the result will compare equal to the original pointer.
*/
typedef __UINTPTR_TYPE__ uintptr_t;

// OpenCL v1.1 s6.11.1, v1.2 s6.12.1, v2.0 s6.13.1 - Work-item Functions

/**
 * Returns the number of global work-items specified for
 * dimension identified by dimindx. This value is given by
 * the global_work_size argument to
 * clEnqueueNDRangeKernel. Valid values of dimindx
 * are 0 to get_work_dim() - 1. For other values of
 * dimindx, get_global_size() returns 1.
 * For clEnqueueTask, this always returns 1.
 */
size_t __ovld __cnfn get_global_size(uint dimindx);

/**
 * Returns the unique global work-item ID value for
 * dimension identified by dimindx. The global work-item
 * ID specifies the work-item ID based on the number of
 * global work-items specified to execute the kernel. Valid
 * values of dimindx are 0 to get_work_dim() - 1. For
 * other values of dimindx, get_global_id() returns 0.
 * For clEnqueueTask, this returns 0.
 */
size_t __ovld __cnfn get_global_id(uint dimindx);

/**
 * Returns the number of local work-items specified in
 * dimension identified by dimindx. This value is given by
 * the local_work_size argument to
 * clEnqueueNDRangeKernel if local_work_size is not
 * NULL; otherwise the OpenCL implementation chooses
 * an appropriate local_work_size value which is returned
 * by this function. Valid values of dimindx are 0 to
 * get_work_dim() - 1. For other values of dimindx,
 * get_local_size() returns 1.
 * For clEnqueueTask, this always returns 1.
 */
size_t __ovld __cnfn get_local_size(uint dimindx);

/**
 * Returns the unique local work-item ID i.e. a work-item
 * within a specific work-group for dimension identified by
 * dimindx. Valid values of dimindx are 0 to
 * get_work_dim() - 1. For other values of dimindx,
 * get_local_id() returns 0.
 * For clEnqueueTask, this returns 0.
 */
size_t __ovld __cnfn get_local_id(uint dimindx);

/**
 * Returns the number of work-groups that will execute a
 * kernel for dimension identified by dimindx.
 * Valid values of dimindx are 0 to get_work_dim() - 1.
 * For other values of dimindx, get_num_groups () returns
 * 1.
 * For clEnqueueTask, this always returns 1.
 */
size_t __ovld __cnfn get_num_groups(uint dimindx);

/**
 * get_group_id returns the work-group ID which is a
 * number from 0 .. get_num_groups(dimindx) - 1.
 * Valid values of dimindx are 0 to get_work_dim() - 1.
 * For other values, get_group_id() returns 0.
 * For clEnqueueTask, this returns 0.
 */
size_t __ovld __cnfn get_group_id(uint dimindx);

/**
 * get_global_offset returns the offset values specified in
 * global_work_offset argument to
 * clEnqueueNDRangeKernel.
 * Valid values of dimindx are 0 to get_work_dim() - 1.
 * For other values, get_global_offset() returns 0.
 * For clEnqueueTask, this returns 0.
 */
size_t __ovld __cnfn get_global_offset(uint dimindx);

/**
 * Wait for events that identify the
 * async_work_group_copy operations to
 * complete. The event objects specified in
 * event_list will be released after the wait is
 * performed.
 * This function must be encountered by all workitems
 * in a work-group executing the kernel with
 * the same num_events and event objects specified
 * in event_list; otherwise the results are undefined.
 */
void __ovld wait_group_events(int num_events, event_t *event_list);

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

char2 __ovld vload2(size_t offset, const __constant char *p);
uchar2 __ovld vload2(size_t offset, const __constant uchar *p);
short2 __ovld vload2(size_t offset, const __constant short *p);
ushort2 __ovld vload2(size_t offset, const __constant ushort *p);
int2 __ovld vload2(size_t offset, const __constant int *p);
uint2 __ovld vload2(size_t offset, const __constant uint *p);
long2 __ovld vload2(size_t offset, const __constant long *p);
ulong2 __ovld vload2(size_t offset, const __constant ulong *p);
float2 __ovld vload2(size_t offset, const __constant float *p);
char3 __ovld vload3(size_t offset, const __constant char *p);
uchar3 __ovld vload3(size_t offset, const __constant uchar *p);
short3 __ovld vload3(size_t offset, const __constant short *p);
ushort3 __ovld vload3(size_t offset, const __constant ushort *p);
int3 __ovld vload3(size_t offset, const __constant int *p);
uint3 __ovld vload3(size_t offset, const __constant uint *p);
long3 __ovld vload3(size_t offset, const __constant long *p);
ulong3 __ovld vload3(size_t offset, const __constant ulong *p);
float3 __ovld vload3(size_t offset, const __constant float *p);
char4 __ovld vload4(size_t offset, const __constant char *p);
uchar4 __ovld vload4(size_t offset, const __constant uchar *p);
short4 __ovld vload4(size_t offset, const __constant short *p);
ushort4 __ovld vload4(size_t offset, const __constant ushort *p);
int4 __ovld vload4(size_t offset, const __constant int *p);
uint4 __ovld vload4(size_t offset, const __constant uint *p);
long4 __ovld vload4(size_t offset, const __constant long *p);
ulong4 __ovld vload4(size_t offset, const __constant ulong *p);
float4 __ovld vload4(size_t offset, const __constant float *p);
char8 __ovld vload8(size_t offset, const __constant char *p);
uchar8 __ovld vload8(size_t offset, const __constant uchar *p);
short8 __ovld vload8(size_t offset, const __constant short *p);
ushort8 __ovld vload8(size_t offset, const __constant ushort *p);
int8 __ovld vload8(size_t offset, const __constant int *p);
uint8 __ovld vload8(size_t offset, const __constant uint *p);
long8 __ovld vload8(size_t offset, const __constant long *p);
ulong8 __ovld vload8(size_t offset, const __constant ulong *p);
float8 __ovld vload8(size_t offset, const __constant float *p);
char16 __ovld vload16(size_t offset, const __constant char *p);
uchar16 __ovld vload16(size_t offset, const __constant uchar *p);
short16 __ovld vload16(size_t offset, const __constant short *p);
ushort16 __ovld vload16(size_t offset, const __constant ushort *p);
int16 __ovld vload16(size_t offset, const __constant int *p);
uint16 __ovld vload16(size_t offset, const __constant uint *p);
long16 __ovld vload16(size_t offset, const __constant long *p);
ulong16 __ovld vload16(size_t offset, const __constant ulong *p);
float16 __ovld vload16(size_t offset, const __constant float *p);
#ifdef cl_khr_fp64
double2 __ovld vload2(size_t offset, const __constant double *p);
double3 __ovld vload3(size_t offset, const __constant double *p);
double4 __ovld vload4(size_t offset, const __constant double *p);
double8 __ovld vload8(size_t offset, const __constant double *p);
double16 __ovld vload16(size_t offset, const __constant double *p);
#endif //cl_khr_fp64

#ifdef cl_khr_fp16
half __ovld vload(size_t offset, const __constant half *p);
half2 __ovld vload2(size_t offset, const __constant half *p);
half3 __ovld vload3(size_t offset, const __constant half *p);
half4 __ovld vload4(size_t offset, const __constant half *p);
half8 __ovld vload8(size_t offset, const __constant half *p);
half16 __ovld vload16(size_t offset, const __constant half *p);
#endif //cl_khr_fp16

/**
 * Read sizeof (half) bytes of data from address
 * (p + offset). The data read is interpreted as a
 * half value. The half value is converted to a
 * float value and the float value is returned.
 * The read address computed as (p + offset)
 * must be 16-bit aligned.
 */
float __ovld vload_half(size_t offset, const __constant half *p);

/**
 * Read sizeof (halfn) bytes of data from address
 * (p + (offset * n)). The data read is interpreted
 * as a halfn value. The halfn value read is
 * converted to a floatn value and the floatn
 * value is returned. The read address computed
 * as (p + (offset * n)) must be 16-bit aligned.
 */
float2 __ovld vload_half2(size_t offset, const __constant half *p);
float3 __ovld vload_half3(size_t offset, const __constant half *p);
float4 __ovld vload_half4(size_t offset, const __constant half *p);
float8 __ovld vload_half8(size_t offset, const __constant half *p);
float16 __ovld vload_half16(size_t offset, const __constant half *p);

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
float __ovld vloada_half(size_t offset, const __constant half *p);
float2 __ovld vloada_half2(size_t offset, const __constant half *p);
float3 __ovld vloada_half3(size_t offset, const __constant half *p);
float4 __ovld vloada_half4(size_t offset, const __constant half *p);
float8 __ovld vloada_half8(size_t offset, const __constant half *p);
float16 __ovld vloada_half16(size_t offset, const __constant half *p);

// OpenCL v1.1 s6.11.10, v1.2 s6.12.10, v2.0 s6.13.10 - Async Copies from Global to Local Memory, Local to Global Memory, and Prefetch

/**
 * event_t async_work_group_copy (
 * __global gentype *dst,
 * const __local gentype *src,
 * size_t num_elements,
 * event_t event)
 * Perform an async copy of num_elements
 * gentype elements from src to dst. The async
 * copy is performed by all work-items in a workgroup
 * and this built-in function must therefore
 * be encountered by all work-items in a workgroup
 * executing the kernel with the same
 * argument values; otherwise the results are
 * undefined.
 * Returns an event object that can be used by
 * wait_group_events to wait for the async copy
 * to finish. The event argument can also be used
 * to associate the async_work_group_copy with
 * a previous async copy allowing an event to be
 * shared by multiple async copies; otherwise event
 * should be zero.
 * If event argument is non-zero, the event object
 * supplied in event argument will be returned.
 * This function does not perform any implicit
 * synchronization of source data such as using a
 * barrier before performing the copy.
 */
event_t __ovld async_work_group_copy(__local char *dst, const __global char *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local uchar *dst, const __global uchar *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local short *dst, const __global short *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local ushort *dst, const __global ushort *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local int *dst, const __global int *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local uint *dst, const __global uint *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local long *dst, const __global long *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local ulong *dst, const __global ulong *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local float *dst, const __global float *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local char2 *dst, const __global char2 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local uchar2 *dst, const __global uchar2 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local short2 *dst, const __global short2 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local ushort2 *dst, const __global ushort2 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local int2 *dst, const __global int2 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local uint2 *dst, const __global uint2 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local long2 *dst, const __global long2 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local ulong2 *dst, const __global ulong2 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local float2 *dst, const __global float2 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local char3 *dst, const __global char3 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local uchar3 *dst, const __global uchar3 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local short3 *dst, const __global short3 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local ushort3 *dst, const __global ushort3 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local int3 *dst, const __global int3 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local uint3 *dst, const __global uint3 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local long3 *dst, const __global long3 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local ulong3 *dst, const __global ulong3 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local float3 *dst, const __global float3 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local char4 *dst, const __global char4 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local uchar4 *dst, const __global uchar4 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local short4 *dst, const __global short4 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local ushort4 *dst, const __global ushort4 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local int4 *dst, const __global int4 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local uint4 *dst, const __global uint4 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local long4 *dst, const __global long4 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local ulong4 *dst, const __global ulong4 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local float4 *dst, const __global float4 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local char8 *dst, const __global char8 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local uchar8 *dst, const __global uchar8 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local short8 *dst, const __global short8 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local ushort8 *dst, const __global ushort8 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local int8 *dst, const __global int8 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local uint8 *dst, const __global uint8 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local long8 *dst, const __global long8 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local ulong8 *dst, const __global ulong8 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local float8 *dst, const __global float8 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local char16 *dst, const __global char16 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local uchar16 *dst, const __global uchar16 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local short16 *dst, const __global short16 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local ushort16 *dst, const __global ushort16 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local int16 *dst, const __global int16 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local uint16 *dst, const __global uint16 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local long16 *dst, const __global long16 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local ulong16 *dst, const __global ulong16 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local float16 *dst, const __global float16 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global char *dst, const __local char *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global uchar *dst, const __local uchar *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global short *dst, const __local short *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global ushort *dst, const __local ushort *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global int *dst, const __local int *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global uint *dst, const __local uint *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global long *dst, const __local long *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global ulong *dst, const __local ulong *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global float *dst, const __local float *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global char2 *dst, const __local char2 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global uchar2 *dst, const __local uchar2 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global short2 *dst, const __local short2 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global ushort2 *dst, const __local ushort2 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global int2 *dst, const __local int2 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global uint2 *dst, const __local uint2 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global long2 *dst, const __local long2 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global ulong2 *dst, const __local ulong2 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global float2 *dst, const __local float2 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global char3 *dst, const __local char3 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global uchar3 *dst, const __local uchar3 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global short3 *dst, const __local short3 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global ushort3 *dst, const __local ushort3 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global int3 *dst, const __local int3 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global uint3 *dst, const __local uint3 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global long3 *dst, const __local long3 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global ulong3 *dst, const __local ulong3 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global float3 *dst, const __local float3 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global char4 *dst, const __local char4 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global uchar4 *dst, const __local uchar4 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global short4 *dst, const __local short4 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global ushort4 *dst, const __local ushort4 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global int4 *dst, const __local int4 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global uint4 *dst, const __local uint4 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global long4 *dst, const __local long4 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global ulong4 *dst, const __local ulong4 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global float4 *dst, const __local float4 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global char8 *dst, const __local char8 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global uchar8 *dst, const __local uchar8 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global short8 *dst, const __local short8 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global ushort8 *dst, const __local ushort8 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global int8 *dst, const __local int8 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global uint8 *dst, const __local uint8 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global long8 *dst, const __local long8 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global ulong8 *dst, const __local ulong8 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global float8 *dst, const __local float8 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global char16 *dst, const __local char16 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global uchar16 *dst, const __local uchar16 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global short16 *dst, const __local short16 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global ushort16 *dst, const __local ushort16 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global int16 *dst, const __local int16 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global uint16 *dst, const __local uint16 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global long16 *dst, const __local long16 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global ulong16 *dst, const __local ulong16 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global float16 *dst, const __local float16 *src, size_t num_elements, event_t event);
#ifdef cl_khr_fp64
event_t __ovld async_work_group_copy(__local double *dst, const __global double *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local double2 *dst, const __global double2 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local double3 *dst, const __global double3 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local double4 *dst, const __global double4 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local double8 *dst, const __global double8 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local double16 *dst, const __global double16 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global double *dst, const __local double *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global double2 *dst, const __local double2 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global double3 *dst, const __local double3 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global double4 *dst, const __local double4 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global double8 *dst, const __local double8 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global double16 *dst, const __local double16 *src, size_t num_elements, event_t event);
#endif //cl_khr_fp64
#ifdef cl_khr_fp16
event_t __ovld async_work_group_copy(__local half *dst, const __global half *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local half2 *dst, const __global half2 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local half3 *dst, const __global half3 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local half4 *dst, const __global half4 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local half8 *dst, const __global half8 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__local half16 *dst, const __global half16 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global half *dst, const __local half *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global half2 *dst, const __local half2 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global half3 *dst, const __local half3 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global half4 *dst, const __local half4 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global half8 *dst, const __local half8 *src, size_t num_elements, event_t event);
event_t __ovld async_work_group_copy(__global half16 *dst, const __local half16 *src, size_t num_elements, event_t event);
#endif //cl_khr_fp16

/**
 * Perform an async gather of num_elements
 * gentype elements from src to dst. The
 * src_stride is the stride in elements for each
 * gentype element read from src. The dst_stride
 * is the stride in elements for each gentype
 * element written to dst. The async gather is
 * performed by all work-items in a work-group.
 * This built-in function must therefore be
 * encountered by all work-items in a work-group
 * executing the kernel with the same argument
 * values; otherwise the results are undefined.
 * Returns an event object that can be used by
 * wait_group_events to wait for the async copy
 * to finish. The event argument can also be used
 * to associate the
 * async_work_group_strided_copy with a
 * previous async copy allowing an event to be
 * shared by multiple async copies; otherwise event
 * should be zero.
 * If event argument is non-zero, the event object
 * supplied in event argument will be returned.
 * This function does not perform any implicit
 * synchronization of source data such as using a
 * barrier before performing the copy.
 */
event_t __ovld async_work_group_strided_copy(__local char *dst, const __global char *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local uchar *dst, const __global uchar *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local short *dst, const __global short *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local ushort *dst, const __global ushort *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local int *dst, const __global int *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local uint *dst, const __global uint *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local long *dst, const __global long *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local ulong *dst, const __global ulong *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local float *dst, const __global float *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local char2 *dst, const __global char2 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local uchar2 *dst, const __global uchar2 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local short2 *dst, const __global short2 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local ushort2 *dst, const __global ushort2 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local int2 *dst, const __global int2 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local uint2 *dst, const __global uint2 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local long2 *dst, const __global long2 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local ulong2 *dst, const __global ulong2 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local float2 *dst, const __global float2 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local char3 *dst, const __global char3 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local uchar3 *dst, const __global uchar3 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local short3 *dst, const __global short3 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local ushort3 *dst, const __global ushort3 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local int3 *dst, const __global int3 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local uint3 *dst, const __global uint3 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local long3 *dst, const __global long3 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local ulong3 *dst, const __global ulong3 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local float3 *dst, const __global float3 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local char4 *dst, const __global char4 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local uchar4 *dst, const __global uchar4 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local short4 *dst, const __global short4 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local ushort4 *dst, const __global ushort4 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local int4 *dst, const __global int4 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local uint4 *dst, const __global uint4 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local long4 *dst, const __global long4 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local ulong4 *dst, const __global ulong4 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local float4 *dst, const __global float4 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local char8 *dst, const __global char8 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local uchar8 *dst, const __global uchar8 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local short8 *dst, const __global short8 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local ushort8 *dst, const __global ushort8 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local int8 *dst, const __global int8 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local uint8 *dst, const __global uint8 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local long8 *dst, const __global long8 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local ulong8 *dst, const __global ulong8 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local float8 *dst, const __global float8 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local char16 *dst, const __global char16 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local uchar16 *dst, const __global uchar16 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local short16 *dst, const __global short16 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local ushort16 *dst, const __global ushort16 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local int16 *dst, const __global int16 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local uint16 *dst, const __global uint16 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local long16 *dst, const __global long16 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local ulong16 *dst, const __global ulong16 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local float16 *dst, const __global float16 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global char *dst, const __local char *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global uchar *dst, const __local uchar *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global short *dst, const __local short *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global ushort *dst, const __local ushort *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global int *dst, const __local int *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global uint *dst, const __local uint *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global long *dst, const __local long *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global ulong *dst, const __local ulong *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global float *dst, const __local float *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global char2 *dst, const __local char2 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global uchar2 *dst, const __local uchar2 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global short2 *dst, const __local short2 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global ushort2 *dst, const __local ushort2 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global int2 *dst, const __local int2 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global uint2 *dst, const __local uint2 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global long2 *dst, const __local long2 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global ulong2 *dst, const __local ulong2 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global float2 *dst, const __local float2 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global char3 *dst, const __local char3 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global uchar3 *dst, const __local uchar3 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global short3 *dst, const __local short3 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global ushort3 *dst, const __local ushort3 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global int3 *dst, const __local int3 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global uint3 *dst, const __local uint3 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global long3 *dst, const __local long3 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global ulong3 *dst, const __local ulong3 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global float3 *dst, const __local float3 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global char4 *dst, const __local char4 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global uchar4 *dst, const __local uchar4 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global short4 *dst, const __local short4 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global ushort4 *dst, const __local ushort4 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global int4 *dst, const __local int4 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global uint4 *dst, const __local uint4 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global long4 *dst, const __local long4 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global ulong4 *dst, const __local ulong4 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global float4 *dst, const __local float4 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global char8 *dst, const __local char8 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global uchar8 *dst, const __local uchar8 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global short8 *dst, const __local short8 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global ushort8 *dst, const __local ushort8 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global int8 *dst, const __local int8 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global uint8 *dst, const __local uint8 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global long8 *dst, const __local long8 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global ulong8 *dst, const __local ulong8 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global float8 *dst, const __local float8 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global char16 *dst, const __local char16 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global uchar16 *dst, const __local uchar16 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global short16 *dst, const __local short16 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global ushort16 *dst, const __local ushort16 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global int16 *dst, const __local int16 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global uint16 *dst, const __local uint16 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global long16 *dst, const __local long16 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global ulong16 *dst, const __local ulong16 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global float16 *dst, const __local float16 *src, size_t num_elements, size_t dst_stride, event_t event);
#ifdef cl_khr_fp64
event_t __ovld async_work_group_strided_copy(__local double *dst, const __global double *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local double2 *dst, const __global double2 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local double3 *dst, const __global double3 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local double4 *dst, const __global double4 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local double8 *dst, const __global double8 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local double16 *dst, const __global double16 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global double *dst, const __local double *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global double2 *dst, const __local double2 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global double3 *dst, const __local double3 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global double4 *dst, const __local double4 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global double8 *dst, const __local double8 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global double16 *dst, const __local double16 *src, size_t num_elements, size_t dst_stride, event_t event);
#endif //cl_khr_fp64
#ifdef cl_khr_fp16
event_t __ovld async_work_group_strided_copy(__local half *dst, const __global half *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local half2 *dst, const __global half2 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local half3 *dst, const __global half3 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local half4 *dst, const __global half4 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local half8 *dst, const __global half8 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__local half16 *dst, const __global half16 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global half *dst, const __local half *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global half2 *dst, const __local half2 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global half3 *dst, const __local half3 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global half4 *dst, const __local half4 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global half8 *dst, const __local half8 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __ovld async_work_group_strided_copy(__global half16 *dst, const __local half16 *src, size_t num_elements, size_t dst_stride, event_t event);
#endif //cl_khr_fp16

/**
 * Prefetch num_elements * sizeof(gentype)
 * bytes into the global cache. The prefetch
 * instruction is applied to a work-item in a workgroup
 * and does not affect the functional
 * behavior of the kernel.
 */
void __ovld prefetch(const __global char *p, size_t num_elements);
void __ovld prefetch(const __global uchar *p, size_t num_elements);
void __ovld prefetch(const __global short *p, size_t num_elements);
void __ovld prefetch(const __global ushort *p, size_t num_elements);
void __ovld prefetch(const __global int *p, size_t num_elements);
void __ovld prefetch(const __global uint *p, size_t num_elements);
void __ovld prefetch(const __global long *p, size_t num_elements);
void __ovld prefetch(const __global ulong *p, size_t num_elements);
void __ovld prefetch(const __global float *p, size_t num_elements);
void __ovld prefetch(const __global char2 *p, size_t num_elements);
void __ovld prefetch(const __global uchar2 *p, size_t num_elements);
void __ovld prefetch(const __global short2 *p, size_t num_elements);
void __ovld prefetch(const __global ushort2 *p, size_t num_elements);
void __ovld prefetch(const __global int2 *p, size_t num_elements);
void __ovld prefetch(const __global uint2 *p, size_t num_elements);
void __ovld prefetch(const __global long2 *p, size_t num_elements);
void __ovld prefetch(const __global ulong2 *p, size_t num_elements);
void __ovld prefetch(const __global float2 *p, size_t num_elements);
void __ovld prefetch(const __global char3 *p, size_t num_elements);
void __ovld prefetch(const __global uchar3 *p, size_t num_elements);
void __ovld prefetch(const __global short3 *p, size_t num_elements);
void __ovld prefetch(const __global ushort3 *p, size_t num_elements);
void __ovld prefetch(const __global int3 *p, size_t num_elements);
void __ovld prefetch(const __global uint3 *p, size_t num_elements);
void __ovld prefetch(const __global long3 *p, size_t num_elements);
void __ovld prefetch(const __global ulong3 *p, size_t num_elements);
void __ovld prefetch(const __global float3 *p, size_t num_elements);
void __ovld prefetch(const __global char4 *p, size_t num_elements);
void __ovld prefetch(const __global uchar4 *p, size_t num_elements);
void __ovld prefetch(const __global short4 *p, size_t num_elements);
void __ovld prefetch(const __global ushort4 *p, size_t num_elements);
void __ovld prefetch(const __global int4 *p, size_t num_elements);
void __ovld prefetch(const __global uint4 *p, size_t num_elements);
void __ovld prefetch(const __global long4 *p, size_t num_elements);
void __ovld prefetch(const __global ulong4 *p, size_t num_elements);
void __ovld prefetch(const __global float4 *p, size_t num_elements);
void __ovld prefetch(const __global char8 *p, size_t num_elements);
void __ovld prefetch(const __global uchar8 *p, size_t num_elements);
void __ovld prefetch(const __global short8 *p, size_t num_elements);
void __ovld prefetch(const __global ushort8 *p, size_t num_elements);
void __ovld prefetch(const __global int8 *p, size_t num_elements);
void __ovld prefetch(const __global uint8 *p, size_t num_elements);
void __ovld prefetch(const __global long8 *p, size_t num_elements);
void __ovld prefetch(const __global ulong8 *p, size_t num_elements);
void __ovld prefetch(const __global float8 *p, size_t num_elements);
void __ovld prefetch(const __global char16 *p, size_t num_elements);
void __ovld prefetch(const __global uchar16 *p, size_t num_elements);
void __ovld prefetch(const __global short16 *p, size_t num_elements);
void __ovld prefetch(const __global ushort16 *p, size_t num_elements);
void __ovld prefetch(const __global int16 *p, size_t num_elements);
void __ovld prefetch(const __global uint16 *p, size_t num_elements);
void __ovld prefetch(const __global long16 *p, size_t num_elements);
void __ovld prefetch(const __global ulong16 *p, size_t num_elements);
void __ovld prefetch(const __global float16 *p, size_t num_elements);
#ifdef cl_khr_fp64
void __ovld prefetch(const __global double *p, size_t num_elements);
void __ovld prefetch(const __global double2 *p, size_t num_elements);
void __ovld prefetch(const __global double3 *p, size_t num_elements);
void __ovld prefetch(const __global double4 *p, size_t num_elements);
void __ovld prefetch(const __global double8 *p, size_t num_elements);
void __ovld prefetch(const __global double16 *p, size_t num_elements);
#endif //cl_khr_fp64
#ifdef cl_khr_fp16
void __ovld prefetch(const __global half *p, size_t num_elements);
void __ovld prefetch(const __global half2 *p, size_t num_elements);
void __ovld prefetch(const __global half3 *p, size_t num_elements);
void __ovld prefetch(const __global half4 *p, size_t num_elements);
void __ovld prefetch(const __global half8 *p, size_t num_elements);
void __ovld prefetch(const __global half16 *p, size_t num_elements);
#endif // cl_khr_fp16

/**
 * Return the image array size.
 */
size_t __ovld __cnfn get_image_array_size(read_only image1d_array_t image_array);
size_t __ovld __cnfn get_image_array_size(read_only image2d_array_t image_array);
#ifdef cl_khr_depth_images
size_t __ovld __cnfn get_image_array_size(read_only image2d_array_depth_t image_array);
#endif //cl_khr_depth_images
#if defined(cl_khr_gl_msaa_sharing)
size_t __ovld __cnfn get_image_array_size(read_only image2d_array_msaa_t image_array);
size_t __ovld __cnfn get_image_array_size(read_only image2d_array_msaa_depth_t image_array);
#endif //cl_khr_gl_msaa_sharing

size_t __ovld __cnfn get_image_array_size(write_only image1d_array_t image_array);
size_t __ovld __cnfn get_image_array_size(write_only image2d_array_t image_array);
#ifdef cl_khr_depth_images
size_t __ovld __cnfn get_image_array_size(write_only image2d_array_depth_t image_array);
#endif //cl_khr_depth_images
#if defined(cl_khr_gl_msaa_sharing)
size_t __ovld __cnfn get_image_array_size(write_only image2d_array_msaa_t image_array);
size_t __ovld __cnfn get_image_array_size(write_only image2d_array_msaa_depth_t image_array);
#endif //cl_khr_gl_msaa_sharing

// --- end of size_t

// OpenCL v1.1 s6.11.1, v1.2 s6.12.11 - Atomic Functions

#if defined(cl_khr_int64_base_atomics) && defined(cl_khr_int64_extended_atomics)
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
#endif
/**
 * Read the 32-bit value (referred to as old)
 * stored at location pointed by p. Compute
 * (old + val) and store result at location
 * pointed by p. The function returns old.
 */
int __ovld atomic_add(volatile __global int *p, int val);
unsigned int __ovld atomic_add(volatile __global unsigned int *p, unsigned int val);
int __ovld atomic_add(volatile __local int *p, int val);
unsigned int __ovld atomic_add(volatile __local unsigned int *p, unsigned int val);

#if defined(cl_khr_global_int32_base_atomics)
int __ovld atom_add(volatile __global int *p, int val);
unsigned int __ovld atom_add(volatile __global unsigned int *p, unsigned int val);
#endif
#if defined(cl_khr_local_int32_base_atomics)
int __ovld atom_add(volatile __local int *p, int val);
unsigned int __ovld atom_add(volatile __local unsigned int *p, unsigned int val);
#endif

#if defined(cl_khr_int64_base_atomics)
long __ovld atom_add(volatile __global long *p, long val);
unsigned long __ovld atom_add(volatile __global unsigned long *p, unsigned long val);
long __ovld atom_add(volatile __local long *p, long val);
unsigned long __ovld atom_add(volatile __local unsigned long *p, unsigned long val);
#endif

/**
 * Read the 32-bit value (referred to as old) stored at location pointed by p.
 * Compute (old - val) and store result at location pointed by p. The function
 * returns old.
 */
int __ovld atomic_sub(volatile __global int *p, int val);
unsigned int __ovld atomic_sub(volatile __global unsigned int *p, unsigned int val);
int __ovld atomic_sub(volatile __local int *p, int val);
unsigned int __ovld atomic_sub(volatile __local unsigned int *p, unsigned int val);

#if defined(cl_khr_global_int32_base_atomics)
int __ovld atom_sub(volatile __global int *p, int val);
unsigned int __ovld atom_sub(volatile __global unsigned int *p, unsigned int val);
#endif
#if defined(cl_khr_local_int32_base_atomics)
int __ovld atom_sub(volatile __local int *p, int val);
unsigned int __ovld atom_sub(volatile __local unsigned int *p, unsigned int val);
#endif

#if defined(cl_khr_int64_base_atomics)
long __ovld atom_sub(volatile __global long *p, long val);
unsigned long __ovld atom_sub(volatile __global unsigned long *p, unsigned long val);
long __ovld atom_sub(volatile __local long *p, long val);
unsigned long __ovld atom_sub(volatile __local unsigned long *p, unsigned long val);
#endif

/**
 * Swaps the old value stored at location p
 * with new value given by val. Returns old
 * value.
 */
int __ovld atomic_xchg(volatile __global int *p, int val);
unsigned int __ovld atomic_xchg(volatile __global unsigned int *p, unsigned int val);
int __ovld atomic_xchg(volatile __local int *p, int val);
unsigned int __ovld atomic_xchg(volatile __local unsigned int *p, unsigned int val);
float __ovld atomic_xchg(volatile __global float *p, float val);
float __ovld atomic_xchg(volatile __local float *p, float val);

#if defined(cl_khr_global_int32_base_atomics)
int __ovld atom_xchg(volatile __global int *p, int val);
unsigned int __ovld atom_xchg(volatile __global unsigned int *p, unsigned int val);
#endif
#if defined(cl_khr_local_int32_base_atomics)
int __ovld atom_xchg(volatile __local int *p, int val);
unsigned int __ovld atom_xchg(volatile __local unsigned int *p, unsigned int val);
#endif

#if defined(cl_khr_int64_base_atomics)
long __ovld atom_xchg(volatile __global long *p, long val);
long __ovld atom_xchg(volatile __local long *p, long val);
unsigned long __ovld atom_xchg(volatile __global unsigned long *p, unsigned long val);
unsigned long __ovld atom_xchg(volatile __local unsigned long *p, unsigned long val);
#endif

/**
 * Read the 32-bit value (referred to as old)
 * stored at location pointed by p. Compute
 * (old + 1) and store result at location
 * pointed by p. The function returns old.
 */
int __ovld atomic_inc(volatile __global int *p);
unsigned int __ovld atomic_inc(volatile __global unsigned int *p);
int __ovld atomic_inc(volatile __local int *p);
unsigned int __ovld atomic_inc(volatile __local unsigned int *p);

#if defined(cl_khr_global_int32_base_atomics)
int __ovld atom_inc(volatile __global int *p);
unsigned int __ovld atom_inc(volatile __global unsigned int *p);
#endif
#if defined(cl_khr_local_int32_base_atomics)
int __ovld atom_inc(volatile __local int *p);
unsigned int __ovld atom_inc(volatile __local unsigned int *p);
#endif

#if defined(cl_khr_int64_base_atomics)
long __ovld atom_inc(volatile __global long *p);
unsigned long __ovld atom_inc(volatile __global unsigned long *p);
long __ovld atom_inc(volatile __local long *p);
unsigned long __ovld atom_inc(volatile __local unsigned long *p);
#endif

/**
 * Read the 32-bit value (referred to as old)
 * stored at location pointed by p. Compute
 * (old - 1) and store result at location
 * pointed by p. The function returns old.
 */
int __ovld atomic_dec(volatile __global int *p);
unsigned int __ovld atomic_dec(volatile __global unsigned int *p);
int __ovld atomic_dec(volatile __local int *p);
unsigned int __ovld atomic_dec(volatile __local unsigned int *p);

#if defined(cl_khr_global_int32_base_atomics)
int __ovld atom_dec(volatile __global int *p);
unsigned int __ovld atom_dec(volatile __global unsigned int *p);
#endif
#if defined(cl_khr_local_int32_base_atomics)
int __ovld atom_dec(volatile __local int *p);
unsigned int __ovld atom_dec(volatile __local unsigned int *p);
#endif

#if defined(cl_khr_int64_base_atomics)
long __ovld atom_dec(volatile __global long *p);
unsigned long __ovld atom_dec(volatile __global unsigned long *p);
long __ovld atom_dec(volatile __local long *p);
unsigned long __ovld atom_dec(volatile __local unsigned long *p);
#endif

/**
 * Read the 32-bit value (referred to as old)
 * stored at location pointed by p. Compute
 * (old == cmp) ? val : old and store result at
 * location pointed by p. The function
 * returns old.
 */
int __ovld atomic_cmpxchg(volatile __global int *p, int cmp, int val);
unsigned int __ovld atomic_cmpxchg(volatile __global unsigned int *p, unsigned int cmp, unsigned int val);
int __ovld atomic_cmpxchg(volatile __local int *p, int cmp, int val);
unsigned int __ovld atomic_cmpxchg(volatile __local unsigned int *p, unsigned int cmp, unsigned int val);

#if defined(cl_khr_global_int32_base_atomics)
int __ovld atom_cmpxchg(volatile __global int *p, int cmp, int val);
unsigned int __ovld atom_cmpxchg(volatile __global unsigned int *p, unsigned int cmp, unsigned int val);
#endif
#if defined(cl_khr_local_int32_base_atomics)
int __ovld atom_cmpxchg(volatile __local int *p, int cmp, int val);
unsigned int __ovld atom_cmpxchg(volatile __local unsigned int *p, unsigned int cmp, unsigned int val);
#endif

#if defined(cl_khr_int64_base_atomics)
long __ovld atom_cmpxchg(volatile __global long *p, long cmp, long val);
unsigned long __ovld atom_cmpxchg(volatile __global unsigned long *p, unsigned long cmp, unsigned long val);
long __ovld atom_cmpxchg(volatile __local long *p, long cmp, long val);
unsigned long __ovld atom_cmpxchg(volatile __local unsigned long *p, unsigned long cmp, unsigned long val);
#endif

/**
 * Read the 32-bit value (referred to as old)
 * stored at location pointed by p. Compute
 * min(old, val) and store minimum value at
 * location pointed by p. The function
 * returns old.
 */
int __ovld atomic_min(volatile __global int *p, int val);
unsigned int __ovld atomic_min(volatile __global unsigned int *p, unsigned int val);
int __ovld atomic_min(volatile __local int *p, int val);
unsigned int __ovld atomic_min(volatile __local unsigned int *p, unsigned int val);

#if defined(cl_khr_global_int32_extended_atomics)
int __ovld atom_min(volatile __global int *p, int val);
unsigned int __ovld atom_min(volatile __global unsigned int *p, unsigned int val);
#endif
#if defined(cl_khr_local_int32_extended_atomics)
int __ovld atom_min(volatile __local int *p, int val);
unsigned int __ovld atom_min(volatile __local unsigned int *p, unsigned int val);
#endif

#if defined(cl_khr_int64_extended_atomics)
long __ovld atom_min(volatile __global long *p, long val);
unsigned long __ovld atom_min(volatile __global unsigned long *p, unsigned long val);
long __ovld atom_min(volatile __local long *p, long val);
unsigned long __ovld atom_min(volatile __local unsigned long *p, unsigned long val);
#endif

/**
 * Read the 32-bit value (referred to as old)
 * stored at location pointed by p. Compute
 * max(old, val) and store maximum value at
 * location pointed by p. The function
 * returns old.
 */
int __ovld atomic_max(volatile __global int *p, int val);
unsigned int __ovld atomic_max(volatile __global unsigned int *p, unsigned int val);
int __ovld atomic_max(volatile __local int *p, int val);
unsigned int __ovld atomic_max(volatile __local unsigned int *p, unsigned int val);

#if defined(cl_khr_global_int32_extended_atomics)
int __ovld atom_max(volatile __global int *p, int val);
unsigned int __ovld atom_max(volatile __global unsigned int *p, unsigned int val);
#endif
#if defined(cl_khr_local_int32_extended_atomics)
int __ovld atom_max(volatile __local int *p, int val);
unsigned int __ovld atom_max(volatile __local unsigned int *p, unsigned int val);
#endif

#if defined(cl_khr_int64_extended_atomics)
long __ovld atom_max(volatile __global long *p, long val);
unsigned long __ovld atom_max(volatile __global unsigned long *p, unsigned long val);
long __ovld atom_max(volatile __local long *p, long val);
unsigned long __ovld atom_max(volatile __local unsigned long *p, unsigned long val);
#endif

/**
 * Read the 32-bit value (referred to as old)
 * stored at location pointed by p. Compute
 * (old & val) and store result at location
 * pointed by p. The function returns old.
 */
int __ovld atomic_and(volatile __global int *p, int val);
unsigned int __ovld atomic_and(volatile __global unsigned int *p, unsigned int val);
int __ovld atomic_and(volatile __local int *p, int val);
unsigned int __ovld atomic_and(volatile __local unsigned int *p, unsigned int val);

#if defined(cl_khr_global_int32_extended_atomics)
int __ovld atom_and(volatile __global int *p, int val);
unsigned int __ovld atom_and(volatile __global unsigned int *p, unsigned int val);
#endif
#if defined(cl_khr_local_int32_extended_atomics)
int __ovld atom_and(volatile __local int *p, int val);
unsigned int __ovld atom_and(volatile __local unsigned int *p, unsigned int val);
#endif

#if defined(cl_khr_int64_extended_atomics)
long __ovld atom_and(volatile __global long *p, long val);
unsigned long __ovld atom_and(volatile __global unsigned long *p, unsigned long val);
long __ovld atom_and(volatile __local long *p, long val);
unsigned long __ovld atom_and(volatile __local unsigned long *p, unsigned long val);
#endif

/**
 * Read the 32-bit value (referred to as old)
 * stored at location pointed by p. Compute
 * (old | val) and store result at location
 * pointed by p. The function returns old.
 */
int __ovld atomic_or(volatile __global int *p, int val);
unsigned int __ovld atomic_or(volatile __global unsigned int *p, unsigned int val);
int __ovld atomic_or(volatile __local int *p, int val);
unsigned int __ovld atomic_or(volatile __local unsigned int *p, unsigned int val);

#if defined(cl_khr_global_int32_extended_atomics)
int __ovld atom_or(volatile __global int *p, int val);
unsigned int __ovld atom_or(volatile __global unsigned int *p, unsigned int val);
#endif
#if defined(cl_khr_local_int32_extended_atomics)
int __ovld atom_or(volatile __local int *p, int val);
unsigned int __ovld atom_or(volatile __local unsigned int *p, unsigned int val);
#endif

#if defined(cl_khr_int64_extended_atomics)
long __ovld atom_or(volatile __global long *p, long val);
unsigned long __ovld atom_or(volatile __global unsigned long *p, unsigned long val);
long __ovld atom_or(volatile __local long *p, long val);
unsigned long __ovld atom_or(volatile __local unsigned long *p, unsigned long val);
#endif

/**
 * Read the 32-bit value (referred to as old)
 * stored at location pointed by p. Compute
 * (old ^ val) and store result at location
 * pointed by p. The function returns old.
 */
int __ovld atomic_xor(volatile __global int *p, int val);
unsigned int __ovld atomic_xor(volatile __global unsigned int *p, unsigned int val);
int __ovld atomic_xor(volatile __local int *p, int val);
unsigned int __ovld atomic_xor(volatile __local unsigned int *p, unsigned int val);

#if defined(cl_khr_global_int32_extended_atomics)
int __ovld atom_xor(volatile __global int *p, int val);
unsigned int __ovld atom_xor(volatile __global unsigned int *p, unsigned int val);
#endif
#if defined(cl_khr_local_int32_extended_atomics)
int __ovld atom_xor(volatile __local int *p, int val);
unsigned int __ovld atom_xor(volatile __local unsigned int *p, unsigned int val);
#endif

#if defined(cl_khr_int64_extended_atomics)
long __ovld atom_xor(volatile __global long *p, long val);
unsigned long __ovld atom_xor(volatile __global unsigned long *p, unsigned long val);
long __ovld atom_xor(volatile __local long *p, long val);
unsigned long __ovld atom_xor(volatile __local unsigned long *p, unsigned long val);
#endif

#if defined(cl_khr_int64_base_atomics) && defined(cl_khr_int64_extended_atomics)
#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : disable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : disable
#endif

#if defined(__OPENCL_CPP_VERSION__) || (__OPENCL_C_VERSION__ >= CL_VERSION_1_2)
// OpenCL v1.2 s6.12.13, v2.0 s6.13.13 - printf

int printf(__constant const char* st, ...) __attribute__((format(printf, 1, 2)));
#endif

// Intel-Specific Sub Group Functions
#if defined(cl_intel_subgroups)
uint    __ovld __conv intel_sub_group_block_read( const __global uint* p );
uint2   __ovld __conv intel_sub_group_block_read2( const __global uint* p );
uint4   __ovld __conv intel_sub_group_block_read4( const __global uint* p );
uint8   __ovld __conv intel_sub_group_block_read8( const __global uint* p );

void    __ovld __conv intel_sub_group_block_write( __global uint* p, uint data );
void    __ovld __conv intel_sub_group_block_write2( __global uint* p, uint2 data );
void    __ovld __conv intel_sub_group_block_write4( __global uint* p, uint4 data );
void    __ovld __conv intel_sub_group_block_write8( __global uint* p, uint8 data );
#endif //cl_intel_subgroups

#if defined(cl_intel_subgroups_short)
uint    __ovld __conv intel_sub_group_block_read_ui( const __global uint* p );
uint2   __ovld __conv intel_sub_group_block_read_ui2( const __global uint* p );
uint4   __ovld __conv intel_sub_group_block_read_ui4( const __global uint* p );
uint8   __ovld __conv intel_sub_group_block_read_ui8( const __global uint* p );

void    __ovld __conv intel_sub_group_block_write_ui( __global uint* p, uint data );
void    __ovld __conv intel_sub_group_block_write_ui2( __global uint* p, uint2 data );
void    __ovld __conv intel_sub_group_block_write_ui4( __global uint* p, uint4 data );
void    __ovld __conv intel_sub_group_block_write_ui8( __global uint* p, uint8 data );

ushort  __ovld __conv intel_sub_group_block_read_us(  const __global ushort* p );
ushort2 __ovld __conv intel_sub_group_block_read_us2( const __global ushort* p );
ushort4 __ovld __conv intel_sub_group_block_read_us4( const __global ushort* p );
ushort8 __ovld __conv intel_sub_group_block_read_us8( const __global ushort* p );

void    __ovld __conv intel_sub_group_block_write_us(  __global ushort* p, ushort  data );
void    __ovld __conv intel_sub_group_block_write_us2( __global ushort* p, ushort2 data );
void    __ovld __conv intel_sub_group_block_write_us4( __global ushort* p, ushort4 data );
void    __ovld __conv intel_sub_group_block_write_us8( __global ushort* p, ushort8 data );
#endif // cl_intel_subgroups_short

#endif // _OPENCL_PLATFORM_H_
