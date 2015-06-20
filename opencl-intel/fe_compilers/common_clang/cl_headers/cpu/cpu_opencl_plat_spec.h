
/**
 * The unsigned integer type of the result of the sizeof operator. This
 * is a 32-bit unsigned integer if CL_DEVICE_ADDRESS_BITS
 * defined in table 4.3 is 32-bits and is a 64-bit unsigned integer if
 * CL_DEVICE_ADDRESS_BITS is 64-bits.
 */
#if defined(__i386__) || defined(i386) || defined(_M_IX86) || defined(__SPIR32)
typedef uint size_t;
#elif defined (__x86_64__) || defined (_M_AMD64) || defined (_M_X64) || defined(__SPIR64)
typedef ulong size_t;

#endif

/**
 * A signed integer type that is the result of subtracting two pointers.
 * This is a 32-bit signed integer if CL_DEVICE_ADDRESS_BITS
 * defined in table 4.3 is 32-bits and is a 64-bit signed integer if
 * CL_DEVICE_ADDRESS_BITS is 64-bits.
 */
#if defined(__i386__) || defined(i386) || defined(_M_IX86) || defined(__SPIR32)
typedef int ptrdiff_t;
#elif defined (__x86_64__) || defined (_M_AMD64) || defined (_M_X64) || defined(__SPIR64)
typedef long ptrdiff_t;
#endif

typedef ptrdiff_t intptr_t;

typedef size_t uintptr_t;


typedef _Atomic(intptr_t) atomic_intptr_t;
typedef _Atomic(uintptr_t) atomic_uintptr_t;
typedef _Atomic(size_t) atomic_size_t;
typedef _Atomic(ptrdiff_t) atomic_ptrdiff_t;

/**
 * Returns the number of global work-items specified for
 * dimension identified by dimindx. This value is given by
 * the global_work_size argument to
 * clEnqueueNDRangeKernel. Valid values of dimindx
 * are 0 to get_work_dim()  1. For other values of
 * dimindx, get_global_size() returns 1.
 * For clEnqueueTask, this always returns 1.
 */
size_t  __attribute__((const)) __attribute__((overloadable)) get_global_size(uint dimindx);

/**
 * Returns the unique global work-item ID value for
 * dimension identified by dimindx. The global work-item
 * ID specifies the work-item ID based on the number of
 * global work-items specified to execute the kernel. Valid
 * values of dimindx are 0 to get_work_dim()  1. For
 * other values of dimindx, get_global_id() returns 0.
 * For clEnqueueTask, this returns 0.
 */
size_t __attribute__((const)) __attribute__((overloadable)) get_global_id(uint dimindx);

/**
 * Prior to OpenCL 2.0
 *    Returns the number of local work-items specified in
 *    dimension identified by dimindx. This value is given by
 *    the local_work_size argument to
 *    clEnqueueNDRangeKernel if local_work_size is not
 *    NULL; otherwise the OpenCL implementation chooses
 *    an appropriate local_work_size value which is returned
 *    by this function. Valid values of dimindx are 0 to
 *    get_work_dim() 1. For other values of dimindx,
 *    get_local_size() returns 1.
 *    For clEnqueueTask, this always returns 1.
 *
 * Since OpenCL 2.0
 *    Returns the number of local work-items specified in
 *    dimension identified by dimindx. This value is at most
 *    the value given by the local_work_size argument to
 *    clEnqueueNDRangeKernel if local_work_size is not
 *    NULL; otherwise the OpenCL implementation chooses
 *    an appropriate local_work_size value which is returned
 *    by this function. If the kernel is executed with a nonuniform
 *    work-group size, calls to this built-in from
 *    some work-groups may return different values than calls
 *    to this built-in from other work-groups.
 *    Valid values of dimindx are 0 to get_work_dim() -1.
 *    For other values of dimindx, get_local_size() returns 1.
 */
size_t  __attribute__((const)) __attribute__((overloadable)) get_local_size(uint dimindx);

/**
 * Returns the unique local work-item ID i.e. a work-item
 * within a specific work-group for dimension identified by
 * dimindx. Valid values of dimindx are 0 to
 * get_work_dim()  1. For other values of dimindx,
 * get_local_id() returns 0.
 * For clEnqueueTask, this returns 0.
 */
size_t __attribute__((const)) __attribute__((overloadable)) get_local_id(uint dimindx);

/**
 * Returns the number of work-groups that will execute a
 * kernel for dimension identified by dimindx.
 * Valid values of dimindx are 0 to get_work_dim()  1.
 * For other values of dimindx, get_num_groups () returns
 * 1.
 * For clEnqueueTask, this always returns 1.
 */
size_t  __attribute__((const)) __attribute__((overloadable)) get_num_groups(uint dimindx);

/**
 * get_group_id returns the work-group ID which is a
 * number from 0 .. get_num_groups(dimindx)  1.
 * Valid values of dimindx are 0 to get_work_dim()  1.
 * For other values, get_group_id() returns 0.
 * For clEnqueueTask, this returns 0.
 */
size_t  __attribute__((const)) __attribute__((overloadable)) get_group_id(uint dimindx);

/**
 * get_global_offset returns the offset values specified in
 * global_work_offset argument to
 * clEnqueueNDRangeKernel.
 * Valid values of dimindx are 0 to get_work_dim()  1.
 * For other values, get_global_offset() returns 0.
 * For clEnqueueTask, this returns 0.
 */
size_t __attribute__((const)) __attribute__((overloadable)) get_global_offset(uint dimindx);

/**
 * Return sizeof (gentypen) bytes of data read
 * from address (p + (offset * n)). The address
 * computed as (p + (offset * n)) must be 8-bit
 * aligned if gentype is char, uchar; 16-bit
 * aligned if gentype is short, ushort; 32-bit
 * aligned if gentype is int, uint, float; 64-bit
 * aligned if gentype is long, ulong.
 */

#if __OPENCL_C_VERSION__ < 200
char2 __attribute__((overloadable)) vload2(size_t offset, const __global char *p);
uchar2 __attribute__((overloadable)) vload2(size_t offset, const __global uchar *p);
short2 __attribute__((overloadable)) vload2(size_t offset, const __global short *p);
ushort2 __attribute__((overloadable)) vload2(size_t offset, const __global ushort *p);
int2 __attribute__((overloadable)) vload2(size_t offset, const __global int *p);
uint2 __attribute__((overloadable)) vload2(size_t offset, const __global uint *p);
long2 __attribute__((overloadable)) vload2(size_t offset, const __global long *p);
ulong2 __attribute__((overloadable)) vload2(size_t offset, const __global ulong *p);
float2 __attribute__((overloadable)) vload2(size_t offset, const __global float *p);
char3 __attribute__((overloadable)) vload3(size_t offset, const __global char *p);
uchar3 __attribute__((overloadable)) vload3(size_t offset, const __global uchar *p);
short3 __attribute__((overloadable)) vload3(size_t offset, const __global short *p);
ushort3 __attribute__((overloadable)) vload3(size_t offset, const __global ushort *p);
int3 __attribute__((overloadable)) vload3(size_t offset, const __global int *p);
uint3 __attribute__((overloadable)) vload3(size_t offset, const __global uint *p);
long3 __attribute__((overloadable)) vload3(size_t offset, const __global long *p);
ulong3 __attribute__((overloadable)) vload3(size_t offset, const __global ulong *p);
float3 __attribute__((overloadable)) vload3(size_t offset, const __global float *p);
char4 __attribute__((overloadable)) vload4(size_t offset, const __global char *p);
uchar4 __attribute__((overloadable)) vload4(size_t offset, const __global uchar *p);
short4 __attribute__((overloadable)) vload4(size_t offset, const __global short *p);
ushort4 __attribute__((overloadable)) vload4(size_t offset, const __global ushort *p);
int4 __attribute__((overloadable)) vload4(size_t offset, const __global int *p);
uint4 __attribute__((overloadable)) vload4(size_t offset, const __global uint *p);
long4 __attribute__((overloadable)) vload4(size_t offset, const __global long *p);
ulong4 __attribute__((overloadable)) vload4(size_t offset, const __global ulong *p);
float4 __attribute__((overloadable)) vload4(size_t offset, const __global float *p);
char8 __attribute__((overloadable)) vload8(size_t offset, const __global char *p);
uchar8 __attribute__((overloadable)) vload8(size_t offset, const __global uchar *p);
short8 __attribute__((overloadable)) vload8(size_t offset, const __global short *p);
ushort8 __attribute__((overloadable)) vload8(size_t offset, const __global ushort *p);
int8 __attribute__((overloadable)) vload8(size_t offset, const __global int *p);
uint8 __attribute__((overloadable)) vload8(size_t offset, const __global uint *p);
long8 __attribute__((overloadable)) vload8(size_t offset, const __global long *p);
ulong8 __attribute__((overloadable)) vload8(size_t offset, const __global ulong *p);
float8 __attribute__((overloadable)) vload8(size_t offset, const __global float *p);
char16 __attribute__((overloadable)) vload16(size_t offset, const __global char *p);
uchar16 __attribute__((overloadable)) vload16(size_t offset, const __global uchar *p);
short16 __attribute__((overloadable)) vload16(size_t offset, const __global short *p);
ushort16 __attribute__((overloadable)) vload16(size_t offset, const __global ushort *p);
int16 __attribute__((overloadable)) vload16(size_t offset, const __global int *p);
uint16 __attribute__((overloadable)) vload16(size_t offset, const __global uint *p);
long16 __attribute__((overloadable)) vload16(size_t offset, const __global long *p);
ulong16 __attribute__((overloadable)) vload16(size_t offset, const __global ulong *p);
float16 __attribute__((overloadable)) vload16(size_t offset, const __global float *p);
char2 __attribute__((overloadable)) vload2(size_t offset, const __local char *p);
uchar2 __attribute__((overloadable)) vload2(size_t offset, const __local uchar *p);
short2 __attribute__((overloadable)) vload2(size_t offset, const __local short *p);
ushort2 __attribute__((overloadable)) vload2(size_t offset, const __local ushort *p);
int2 __attribute__((overloadable)) vload2(size_t offset, const __local int *p);
uint2 __attribute__((overloadable)) vload2(size_t offset, const __local uint *p);
long2 __attribute__((overloadable)) vload2(size_t offset, const __local long *p);
ulong2 __attribute__((overloadable)) vload2(size_t offset, const __local ulong *p);
float2 __attribute__((overloadable)) vload2(size_t offset, const __local float *p);
char3 __attribute__((overloadable)) vload3(size_t offset, const __local char *p);
uchar3 __attribute__((overloadable)) vload3(size_t offset, const __local uchar *p);
short3 __attribute__((overloadable)) vload3(size_t offset, const __local short *p);
ushort3 __attribute__((overloadable)) vload3(size_t offset, const __local ushort *p);
int3 __attribute__((overloadable)) vload3(size_t offset, const __local int *p);
uint3 __attribute__((overloadable)) vload3(size_t offset, const __local uint *p);
long3 __attribute__((overloadable)) vload3(size_t offset, const __local long *p);
ulong3 __attribute__((overloadable)) vload3(size_t offset, const __local ulong *p);
float3 __attribute__((overloadable)) vload3(size_t offset, const __local float *p);
char4 __attribute__((overloadable)) vload4(size_t offset, const __local char *p);
uchar4 __attribute__((overloadable)) vload4(size_t offset, const __local uchar *p);
short4 __attribute__((overloadable)) vload4(size_t offset, const __local short *p);
ushort4 __attribute__((overloadable)) vload4(size_t offset, const __local ushort *p);
int4 __attribute__((overloadable)) vload4(size_t offset, const __local int *p);
uint4 __attribute__((overloadable)) vload4(size_t offset, const __local uint *p);
long4 __attribute__((overloadable)) vload4(size_t offset, const __local long *p);
ulong4 __attribute__((overloadable)) vload4(size_t offset, const __local ulong *p);
float4 __attribute__((overloadable)) vload4(size_t offset, const __local float *p);
char8 __attribute__((overloadable)) vload8(size_t offset, const __local char *p);
uchar8 __attribute__((overloadable)) vload8(size_t offset, const __local uchar *p);
short8 __attribute__((overloadable)) vload8(size_t offset, const __local short *p);
ushort8 __attribute__((overloadable)) vload8(size_t offset, const __local ushort *p);
int8 __attribute__((overloadable)) vload8(size_t offset, const __local int *p);
uint8 __attribute__((overloadable)) vload8(size_t offset, const __local uint *p);
long8 __attribute__((overloadable)) vload8(size_t offset, const __local long *p);
ulong8 __attribute__((overloadable)) vload8(size_t offset, const __local ulong *p);
float8 __attribute__((overloadable)) vload8(size_t offset, const __local float *p);
char16 __attribute__((overloadable)) vload16(size_t offset, const __local char *p);
uchar16 __attribute__((overloadable)) vload16(size_t offset, const __local uchar *p);
short16 __attribute__((overloadable)) vload16(size_t offset, const __local short *p);
ushort16 __attribute__((overloadable)) vload16(size_t offset, const __local ushort *p);
int16 __attribute__((overloadable)) vload16(size_t offset, const __local int *p);
uint16 __attribute__((overloadable)) vload16(size_t offset, const __local uint *p);
long16 __attribute__((overloadable)) vload16(size_t offset, const __local long *p);
ulong16 __attribute__((overloadable)) vload16(size_t offset, const __local ulong *p);
float16 __attribute__((overloadable)) vload16(size_t offset, const __local float *p);
char2 __attribute__((overloadable)) vload2(size_t offset, const __private char *p);
uchar2 __attribute__((overloadable)) vload2(size_t offset, const __private uchar *p);
short2 __attribute__((overloadable)) vload2(size_t offset, const __private short *p);
ushort2 __attribute__((overloadable)) vload2(size_t offset, const __private ushort *p);
int2 __attribute__((overloadable)) vload2(size_t offset, const __private int *p);
uint2 __attribute__((overloadable)) vload2(size_t offset, const __private uint *p);
long2 __attribute__((overloadable)) vload2(size_t offset, const __private long *p);
ulong2 __attribute__((overloadable)) vload2(size_t offset, const __private ulong *p);
float2 __attribute__((overloadable)) vload2(size_t offset, const __private float *p);
char3 __attribute__((overloadable)) vload3(size_t offset, const __private char *p);
uchar3 __attribute__((overloadable)) vload3(size_t offset, const __private uchar *p);
short3 __attribute__((overloadable)) vload3(size_t offset, const __private short *p);
ushort3 __attribute__((overloadable)) vload3(size_t offset, const __private ushort *p);
int3 __attribute__((overloadable)) vload3(size_t offset, const __private int *p);
uint3 __attribute__((overloadable)) vload3(size_t offset, const __private uint *p);
long3 __attribute__((overloadable)) vload3(size_t offset, const __private long *p);
ulong3 __attribute__((overloadable)) vload3(size_t offset, const __private ulong *p);
float3 __attribute__((overloadable)) vload3(size_t offset, const __private float *p);
char4 __attribute__((overloadable)) vload4(size_t offset, const __private char *p);
uchar4 __attribute__((overloadable)) vload4(size_t offset, const __private uchar *p);
short4 __attribute__((overloadable)) vload4(size_t offset, const __private short *p);
ushort4 __attribute__((overloadable)) vload4(size_t offset, const __private ushort *p);
int4 __attribute__((overloadable)) vload4(size_t offset, const __private int *p);
uint4 __attribute__((overloadable)) vload4(size_t offset, const __private uint *p);
long4 __attribute__((overloadable)) vload4(size_t offset, const __private long *p);
ulong4 __attribute__((overloadable)) vload4(size_t offset, const __private ulong *p);
float4 __attribute__((overloadable)) vload4(size_t offset, const __private float *p);
char8 __attribute__((overloadable)) vload8(size_t offset, const __private char *p);
uchar8 __attribute__((overloadable)) vload8(size_t offset, const __private uchar *p);
short8 __attribute__((overloadable)) vload8(size_t offset, const __private short *p);
ushort8 __attribute__((overloadable)) vload8(size_t offset, const __private ushort *p);
int8 __attribute__((overloadable)) vload8(size_t offset, const __private int *p);
uint8 __attribute__((overloadable)) vload8(size_t offset, const __private uint *p);
long8 __attribute__((overloadable)) vload8(size_t offset, const __private long *p);
ulong8 __attribute__((overloadable)) vload8(size_t offset, const __private ulong *p);
float8 __attribute__((overloadable)) vload8(size_t offset, const __private float *p);
char16 __attribute__((overloadable)) vload16(size_t offset, const __private char *p);
uchar16 __attribute__((overloadable)) vload16(size_t offset, const __private uchar *p);
short16 __attribute__((overloadable)) vload16(size_t offset, const __private short *p);
ushort16 __attribute__((overloadable)) vload16(size_t offset, const __private ushort *p);
int16 __attribute__((overloadable)) vload16(size_t offset, const __private int *p);
uint16 __attribute__((overloadable)) vload16(size_t offset, const __private uint *p);
long16 __attribute__((overloadable)) vload16(size_t offset, const __private long *p);
ulong16 __attribute__((overloadable)) vload16(size_t offset, const __private ulong *p);
float16 __attribute__((overloadable)) vload16(size_t offset, const __private float *p);
double2 __attribute__((overloadable)) vload2(size_t offset, const __global double *p);
double3 __attribute__((overloadable)) vload3(size_t offset, const __global double *p);
double4 __attribute__((overloadable)) vload4(size_t offset, const __global double *p);
double8 __attribute__((overloadable)) vload8(size_t offset, const __global double *p);
double16 __attribute__((overloadable)) vload16(size_t offset, const __global double *p);
double2 __attribute__((overloadable)) vload2(size_t offset, const __local double *p);
double3 __attribute__((overloadable)) vload3(size_t offset, const __local double *p);
double4 __attribute__((overloadable)) vload4(size_t offset, const __local double *p);
double8 __attribute__((overloadable)) vload8(size_t offset, const __local double *p);
double16 __attribute__((overloadable)) vload16(size_t offset, const __local double *p);
double2 __attribute__((overloadable)) vload2(size_t offset, const __private double *p);
double3 __attribute__((overloadable)) vload3(size_t offset, const __private double *p);
double4 __attribute__((overloadable)) vload4(size_t offset, const __private double *p);
double8 __attribute__((overloadable)) vload8(size_t offset, const __private double *p);
double16 __attribute__((overloadable)) vload16(size_t offset, const __private double *p);
//half __attribute__((overloadable)) vload(size_t offset, const __global half *p);
//half2 __attribute__((overloadable)) vload2(size_t offset, const __global half *p);
//half3 __attribute__((overloadable)) vload3(size_t offset, const __global half *p);
//half4 __attribute__((overloadable)) vload4(size_t offset, const __global half *p);
//half8 __attribute__((overloadable)) vload8(size_t offset, const __global half *p);
//half16 __attribute__((overloadable)) vload16(size_t offset, const __global half *p);
//half __attribute__((overloadable)) vload(size_t offset, const __local half *p);
//half2 __attribute__((overloadable)) vload2(size_t offset, const __local half *p);
//half3 __attribute__((overloadable)) vload3(size_t offset, const __local half *p);
//half4 __attribute__((overloadable)) vload4(size_t offset, const __local half *p);
//half8 __attribute__((overloadable)) vload8(size_t offset, const __local half *p);
//half16 __attribute__((overloadable)) vload16(size_t offset, const __local half *p);
//half __attribute__((overloadable)) vload(size_t offset, const __private half *p);
//half2 __attribute__((overloadable)) vload2(size_t offset, const __private half *p);
//half3 __attribute__((overloadable)) vload3(size_t offset, const __private half *p);
//half4 __attribute__((overloadable)) vload4(size_t offset, const __private half *p);
//half8 __attribute__((overloadable)) vload8(size_t offset, const __private half *p);
//half16 __attribute__((overloadable)) vload16(size_t offset, const __private half *p);

void __attribute__((overloadable)) vstore2(char2 data, size_t offset, __global char *p);
void __attribute__((overloadable)) vstore2(uchar2 data, size_t offset, __global uchar *p);
void __attribute__((overloadable)) vstore2(short2 data, size_t offset, __global short *p);
void __attribute__((overloadable)) vstore2(ushort2 data, size_t offset, __global ushort *p);
void __attribute__((overloadable)) vstore2(int2 data, size_t offset, __global int *p);
void __attribute__((overloadable)) vstore2(uint2 data, size_t offset, __global uint *p);
void __attribute__((overloadable)) vstore2(long2 data, size_t offset, __global long *p);
void __attribute__((overloadable)) vstore2(ulong2 data, size_t offset, __global ulong *p);
void __attribute__((overloadable)) vstore2(float2 data, size_t offset, __global float *p);
void __attribute__((overloadable)) vstore3(char3 data, size_t offset, __global char *p);
void __attribute__((overloadable)) vstore3(uchar3 data, size_t offset, __global uchar *p);
void __attribute__((overloadable)) vstore3(short3 data, size_t offset, __global short *p);
void __attribute__((overloadable)) vstore3(ushort3 data, size_t offset, __global ushort *p);
void __attribute__((overloadable)) vstore3(int3 data, size_t offset, __global int *p);
void __attribute__((overloadable)) vstore3(uint3 data, size_t offset, __global uint *p);
void __attribute__((overloadable)) vstore3(long3 data, size_t offset, __global long *p);
void __attribute__((overloadable)) vstore3(ulong3 data, size_t offset, __global ulong *p);
void __attribute__((overloadable)) vstore3(float3 data, size_t offset, __global float *p);
void __attribute__((overloadable)) vstore4(char4 data, size_t offset, __global char *p);
void __attribute__((overloadable)) vstore4(uchar4 data, size_t offset, __global uchar *p);
void __attribute__((overloadable)) vstore4(short4 data, size_t offset, __global short *p);
void __attribute__((overloadable)) vstore4(ushort4 data, size_t offset, __global ushort *p);
void __attribute__((overloadable)) vstore4(int4 data, size_t offset, __global int *p);
void __attribute__((overloadable)) vstore4(uint4 data, size_t offset, __global uint *p);
void __attribute__((overloadable)) vstore4(long4 data, size_t offset, __global long *p);
void __attribute__((overloadable)) vstore4(ulong4 data, size_t offset, __global ulong *p);
void __attribute__((overloadable)) vstore4(float4 data, size_t offset, __global float *p);
void __attribute__((overloadable)) vstore8(char8 data, size_t offset, __global char *p);
void __attribute__((overloadable)) vstore8(uchar8 data, size_t offset, __global uchar *p);
void __attribute__((overloadable)) vstore8(short8 data, size_t offset, __global short *p);
void __attribute__((overloadable)) vstore8(ushort8 data, size_t offset, __global ushort *p);
void __attribute__((overloadable)) vstore8(int8 data, size_t offset, __global int *p);
void __attribute__((overloadable)) vstore8(uint8 data, size_t offset, __global uint *p);
void __attribute__((overloadable)) vstore8(long8 data, size_t offset, __global long *p);
void __attribute__((overloadable)) vstore8(ulong8 data, size_t offset, __global ulong *p);
void __attribute__((overloadable)) vstore8(float8 data, size_t offset, __global float *p);
void __attribute__((overloadable)) vstore16(char16 data, size_t offset, __global char *p);
void __attribute__((overloadable)) vstore16(uchar16 data, size_t offset, __global uchar *p);
void __attribute__((overloadable)) vstore16(short16 data, size_t offset, __global short *p);
void __attribute__((overloadable)) vstore16(ushort16 data, size_t offset, __global ushort *p);
void __attribute__((overloadable)) vstore16(int16 data, size_t offset, __global int *p);
void __attribute__((overloadable)) vstore16(uint16 data, size_t offset, __global uint *p);
void __attribute__((overloadable)) vstore16(long16 data, size_t offset, __global long *p);
void __attribute__((overloadable)) vstore16(ulong16 data, size_t offset, __global ulong *p);
void __attribute__((overloadable)) vstore16(float16 data, size_t offset, __global float *p);
void __attribute__((overloadable)) vstore2(char2 data, size_t offset, __local char *p);
void __attribute__((overloadable)) vstore2(uchar2 data, size_t offset, __local uchar *p);
void __attribute__((overloadable)) vstore2(short2 data, size_t offset, __local short *p);
void __attribute__((overloadable)) vstore2(ushort2 data, size_t offset, __local ushort *p);
void __attribute__((overloadable)) vstore2(int2 data, size_t offset, __local int *p);
void __attribute__((overloadable)) vstore2(uint2 data, size_t offset, __local uint *p);
void __attribute__((overloadable)) vstore2(long2 data, size_t offset, __local long *p);
void __attribute__((overloadable)) vstore2(ulong2 data, size_t offset, __local ulong *p);
void __attribute__((overloadable)) vstore2(float2 data, size_t offset, __local float *p);
void __attribute__((overloadable)) vstore3(char3 data, size_t offset, __local char *p);
void __attribute__((overloadable)) vstore3(uchar3 data, size_t offset, __local uchar *p);
void __attribute__((overloadable)) vstore3(short3 data, size_t offset, __local short *p);
void __attribute__((overloadable)) vstore3(ushort3 data, size_t offset, __local ushort *p);
void __attribute__((overloadable)) vstore3(int3 data, size_t offset, __local int *p);
void __attribute__((overloadable)) vstore3(uint3 data, size_t offset, __local uint *p);
void __attribute__((overloadable)) vstore3(long3 data, size_t offset, __local long *p);
void __attribute__((overloadable)) vstore3(ulong3 data, size_t offset, __local ulong *p);
void __attribute__((overloadable)) vstore3(float3 data, size_t offset, __local float *p);
void __attribute__((overloadable)) vstore4(char4 data, size_t offset, __local char *p);
void __attribute__((overloadable)) vstore4(uchar4 data, size_t offset, __local uchar *p);
void __attribute__((overloadable)) vstore4(short4 data, size_t offset, __local short *p);
void __attribute__((overloadable)) vstore4(ushort4 data, size_t offset, __local ushort *p);
void __attribute__((overloadable)) vstore4(int4 data, size_t offset, __local int *p);
void __attribute__((overloadable)) vstore4(uint4 data, size_t offset, __local uint *p);
void __attribute__((overloadable)) vstore4(long4 data, size_t offset, __local long *p);
void __attribute__((overloadable)) vstore4(ulong4 data, size_t offset, __local ulong *p);
void __attribute__((overloadable)) vstore4(float4 data, size_t offset, __local float *p);
void __attribute__((overloadable)) vstore8(char8 data, size_t offset, __local char *p);
void __attribute__((overloadable)) vstore8(uchar8 data, size_t offset, __local uchar *p);
void __attribute__((overloadable)) vstore8(short8 data, size_t offset, __local short *p);
void __attribute__((overloadable)) vstore8(ushort8 data, size_t offset, __local ushort *p);
void __attribute__((overloadable)) vstore8(int8 data, size_t offset, __local int *p);
void __attribute__((overloadable)) vstore8(uint8 data, size_t offset, __local uint *p);
void __attribute__((overloadable)) vstore8(long8 data, size_t offset, __local long *p);
void __attribute__((overloadable)) vstore8(ulong8 data, size_t offset, __local ulong *p);
void __attribute__((overloadable)) vstore8(float8 data, size_t offset, __local float *p);
void __attribute__((overloadable)) vstore16(char16 data, size_t offset, __local char *p);
void __attribute__((overloadable)) vstore16(uchar16 data, size_t offset, __local uchar *p);
void __attribute__((overloadable)) vstore16(short16 data, size_t offset, __local short *p);
void __attribute__((overloadable)) vstore16(ushort16 data, size_t offset, __local ushort *p);
void __attribute__((overloadable)) vstore16(int16 data, size_t offset, __local int *p);
void __attribute__((overloadable)) vstore16(uint16 data, size_t offset, __local uint *p);
void __attribute__((overloadable)) vstore16(long16 data, size_t offset, __local long *p);
void __attribute__((overloadable)) vstore16(ulong16 data, size_t offset, __local ulong *p);
void __attribute__((overloadable)) vstore16(float16 data, size_t offset, __local float *p);
void __attribute__((overloadable)) vstore2(char2 data, size_t offset, __private char *p);
void __attribute__((overloadable)) vstore2(uchar2 data, size_t offset, __private uchar *p);
void __attribute__((overloadable)) vstore2(short2 data, size_t offset, __private short *p);
void __attribute__((overloadable)) vstore2(ushort2 data, size_t offset, __private ushort *p);
void __attribute__((overloadable)) vstore2(int2 data, size_t offset, __private int *p);
void __attribute__((overloadable)) vstore2(uint2 data, size_t offset, __private uint *p);
void __attribute__((overloadable)) vstore2(long2 data, size_t offset, __private long *p);
void __attribute__((overloadable)) vstore2(ulong2 data, size_t offset, __private ulong *p);
void __attribute__((overloadable)) vstore2(float2 data, size_t offset, __private float *p);
void __attribute__((overloadable)) vstore3(char3 data, size_t offset, __private char *p);
void __attribute__((overloadable)) vstore3(uchar3 data, size_t offset, __private uchar *p);
void __attribute__((overloadable)) vstore3(short3 data, size_t offset, __private short *p);
void __attribute__((overloadable)) vstore3(ushort3 data, size_t offset, __private ushort *p);
void __attribute__((overloadable)) vstore3(int3 data, size_t offset, __private int *p);
void __attribute__((overloadable)) vstore3(uint3 data, size_t offset, __private uint *p);
void __attribute__((overloadable)) vstore3(long3 data, size_t offset, __private long *p);
void __attribute__((overloadable)) vstore3(ulong3 data, size_t offset, __private ulong *p);
void __attribute__((overloadable)) vstore3(float3 data, size_t offset, __private float *p);
void __attribute__((overloadable)) vstore4(char4 data, size_t offset, __private char *p);
void __attribute__((overloadable)) vstore4(uchar4 data, size_t offset, __private uchar *p);
void __attribute__((overloadable)) vstore4(short4 data, size_t offset, __private short *p);
void __attribute__((overloadable)) vstore4(ushort4 data, size_t offset, __private ushort *p);
void __attribute__((overloadable)) vstore4(int4 data, size_t offset, __private int *p);
void __attribute__((overloadable)) vstore4(uint4 data, size_t offset, __private uint *p);
void __attribute__((overloadable)) vstore4(long4 data, size_t offset, __private long *p);
void __attribute__((overloadable)) vstore4(ulong4 data, size_t offset, __private ulong *p);
void __attribute__((overloadable)) vstore4(float4 data, size_t offset, __private float *p);
void __attribute__((overloadable)) vstore8(char8 data, size_t offset, __private char *p);
void __attribute__((overloadable)) vstore8(uchar8 data, size_t offset, __private uchar *p);
void __attribute__((overloadable)) vstore8(short8 data, size_t offset, __private short *p);
void __attribute__((overloadable)) vstore8(ushort8 data, size_t offset, __private ushort *p);
void __attribute__((overloadable)) vstore8(int8 data, size_t offset, __private int *p);
void __attribute__((overloadable)) vstore8(uint8 data, size_t offset, __private uint *p);
void __attribute__((overloadable)) vstore8(long8 data, size_t offset, __private long *p);
void __attribute__((overloadable)) vstore8(ulong8 data, size_t offset, __private ulong *p);
void __attribute__((overloadable)) vstore8(float8 data, size_t offset, __private float *p);
void __attribute__((overloadable)) vstore16(char16 data, size_t offset, __private char *p);
void __attribute__((overloadable)) vstore16(uchar16 data, size_t offset, __private uchar *p);
void __attribute__((overloadable)) vstore16(short16 data, size_t offset, __private short *p);
void __attribute__((overloadable)) vstore16(ushort16 data, size_t offset, __private ushort *p);
void __attribute__((overloadable)) vstore16(int16 data, size_t offset, __private int *p);
void __attribute__((overloadable)) vstore16(uint16 data, size_t offset, __private uint *p);
void __attribute__((overloadable)) vstore16(long16 data, size_t offset, __private long *p);
void __attribute__((overloadable)) vstore16(ulong16 data, size_t offset, __private ulong *p);
void __attribute__((overloadable)) vstore16(float16 data, size_t offset, __private float *p);
void __attribute__((overloadable)) vstore2(double2 data, size_t offset, __global double *p);
void __attribute__((overloadable)) vstore3(double3 data, size_t offset, __global double *p);
void __attribute__((overloadable)) vstore4(double4 data, size_t offset, __global double *p);
void __attribute__((overloadable)) vstore8(double8 data, size_t offset, __global double *p);
void __attribute__((overloadable)) vstore16(double16 data, size_t offset, __global double *p);
void __attribute__((overloadable)) vstore2(double2 data, size_t offset, __local double *p);
void __attribute__((overloadable)) vstore3(double3 data, size_t offset, __local double *p);
void __attribute__((overloadable)) vstore4(double4 data, size_t offset, __local double *p);
void __attribute__((overloadable)) vstore8(double8 data, size_t offset, __local double *p);
void __attribute__((overloadable)) vstore16(double16 data, size_t offset, __local double *p);
void __attribute__((overloadable)) vstore2(double2 data, size_t offset, __private double *p);
void __attribute__((overloadable)) vstore3(double3 data, size_t offset, __private double *p);
void __attribute__((overloadable)) vstore4(double4 data, size_t offset, __private double *p);
void __attribute__((overloadable)) vstore8(double8 data, size_t offset, __private double *p);
void __attribute__((overloadable)) vstore16(double16 data, size_t offset, __private double *p);
//void __attribute__((overloadable)) vstore(half data, size_t offset, __global half *p);
//void __attribute__((overloadable)) vstore2(half2 data, size_t offset, __global half *p);
//void __attribute__((overloadable)) vstore3(half3 data, size_t offset, __global half *p);
//void __attribute__((overloadable)) vstore4(half4 data, size_t offset, __global half *p);
//void __attribute__((overloadable)) vstore8(half8 data, size_t offset, __global half *p);
//void __attribute__((overloadable)) vstore16(half16 data, size_t offset, __global half *p);
//void __attribute__((overloadable)) vstore(half data, size_t offset, __local half *p);
//void __attribute__((overloadable)) vstore2(half2 data, size_t offset, __local half *p);
//void __attribute__((overloadable)) vstore3(half3 data, size_t offset, __local half *p);
//void __attribute__((overloadable)) vstore4(half4 data, size_t offset, __local half *p);
//void __attribute__((overloadable)) vstore8(half8 data, size_t offset, __local half *p);
//void __attribute__((overloadable)) vstore16(half16 data, size_t offset, __local half *p);
//void __attribute__((overloadable)) vstore(half data, size_t offset, __private half *p);
//void __attribute__((overloadable)) vstore2(half2 data, size_t offset, __private half *p);
//void __attribute__((overloadable)) vstore3(half3 data, size_t offset, __private half *p);
//void __attribute__((overloadable)) vstore4(half4 data, size_t offset, __private half *p);
//void __attribute__((overloadable)) vstore8(half8 data, size_t offset, __private half *p);
//void __attribute__((overloadable)) vstore16(half16 data, size_t offset, __private half *p);

float __attribute__((overloadable)) vload_half(size_t offset, const __global half *p);
float __attribute__((overloadable)) vload_half(size_t offset, const __local half *p);
float __attribute__((overloadable)) vload_half(size_t offset, const __private half *p);


float2 __attribute__((overloadable)) vload_half2(size_t offset, const __global half *p);
float3 __attribute__((overloadable)) vload_half3(size_t offset, const __global half *p);
float4 __attribute__((overloadable)) vload_half4(size_t offset, const __global half *p);
float8 __attribute__((overloadable)) vload_half8(size_t offset, const __global half *p);
float16 __attribute__((overloadable)) vload_half16(size_t offset, const __global half *p);
float2 __attribute__((overloadable)) vload_half2(size_t offset, const __local half *p);
float3 __attribute__((overloadable)) vload_half3(size_t offset, const __local half *p);
float4 __attribute__((overloadable)) vload_half4(size_t offset, const __local half *p);
float8 __attribute__((overloadable)) vload_half8(size_t offset, const __local half *p);
float16 __attribute__((overloadable)) vload_half16(size_t offset, const __local half *p);
float2 __attribute__((overloadable)) vload_half2(size_t offset, const __private half *p);
float3 __attribute__((overloadable)) vload_half3(size_t offset, const __private half *p);
float4 __attribute__((overloadable)) vload_half4(size_t offset, const __private half *p);
float8 __attribute__((overloadable)) vload_half8(size_t offset, const __private half *p);
float16 __attribute__((overloadable)) vload_half16(size_t offset, const __private half *p);

void __attribute__((overloadable)) vstore_half(float data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half_rte(float data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half_rtz(float data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half_rtp(float data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half_rtn(float data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half(float data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half_rte(float data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half_rtz(float data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half_rtp(float data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half_rtn(float data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half(float data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half_rte(float data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half_rtz(float data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half_rtp(float data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half_rtn(float data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half(double data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half_rte(double data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half_rtz(double data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half_rtp(double data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half_rtn(double data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half(double data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half_rte(double data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half_rtz(double data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half_rtp(double data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half_rtn(double data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half(double data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half_rte(double data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half_rtz(double data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half_rtp(double data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half_rtn(double data, size_t offset, __private half *p);

void __attribute__((overloadable)) vstore_half2(float2 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half3(float3 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half4(float4 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half8(float8 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half16(float16 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half2_rte(float2 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half3_rte(float3 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half4_rte(float4 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half8_rte(float8 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half16_rte(float16 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half2_rtz(float2 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half3_rtz(float3 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half4_rtz(float4 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half8_rtz(float8 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half16_rtz(float16 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half2_rtp(float2 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half3_rtp(float3 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half4_rtp(float4 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half8_rtp(float8 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half16_rtp(float16 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half2_rtn(float2 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half3_rtn(float3 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half4_rtn(float4 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half8_rtn(float8 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half16_rtn(float16 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half2(float2 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half3(float3 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half4(float4 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half8(float8 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half16(float16 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half2_rte(float2 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half3_rte(float3 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half4_rte(float4 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half8_rte(float8 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half16_rte(float16 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half2_rtz(float2 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half3_rtz(float3 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half4_rtz(float4 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half8_rtz(float8 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half16_rtz(float16 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half2_rtp(float2 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half3_rtp(float3 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half4_rtp(float4 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half8_rtp(float8 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half16_rtp(float16 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half2_rtn(float2 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half3_rtn(float3 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half4_rtn(float4 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half8_rtn(float8 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half16_rtn(float16 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half2(float2 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half3(float3 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half4(float4 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half8(float8 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half16(float16 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half2_rte(float2 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half3_rte(float3 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half4_rte(float4 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half8_rte(float8 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half16_rte(float16 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half2_rtz(float2 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half3_rtz(float3 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half4_rtz(float4 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half8_rtz(float8 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half16_rtz(float16 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half2_rtp(float2 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half3_rtp(float3 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half4_rtp(float4 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half8_rtp(float8 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half16_rtp(float16 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half2_rtn(float2 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half3_rtn(float3 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half4_rtn(float4 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half8_rtn(float8 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half16_rtn(float16 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half2(double2 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half3(double3 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half4(double4 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half8(double8 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half16(double16 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half2_rte(double2 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half3_rte(double3 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half4_rte(double4 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half8_rte(double8 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half16_rte(double16 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half2_rtz(double2 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half3_rtz(double3 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half4_rtz(double4 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half8_rtz(double8 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half16_rtz(double16 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half2_rtp(double2 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half3_rtp(double3 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half4_rtp(double4 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half8_rtp(double8 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half16_rtp(double16 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half2_rtn(double2 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half3_rtn(double3 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half4_rtn(double4 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half8_rtn(double8 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half16_rtn(double16 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstore_half2(double2 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half3(double3 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half4(double4 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half8(double8 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half16(double16 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half2_rte(double2 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half3_rte(double3 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half4_rte(double4 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half8_rte(double8 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half16_rte(double16 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half2_rtz(double2 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half3_rtz(double3 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half4_rtz(double4 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half8_rtz(double8 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half16_rtz(double16 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half2_rtp(double2 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half3_rtp(double3 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half4_rtp(double4 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half8_rtp(double8 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half16_rtp(double16 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half2_rtn(double2 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half3_rtn(double3 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half4_rtn(double4 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half8_rtn(double8 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half16_rtn(double16 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstore_half2(double2 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half3(double3 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half4(double4 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half8(double8 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half16(double16 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half2_rte(double2 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half3_rte(double3 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half4_rte(double4 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half8_rte(double8 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half16_rte(double16 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half2_rtz(double2 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half3_rtz(double3 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half4_rtz(double4 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half8_rtz(double8 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half16_rtz(double16 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half2_rtp(double2 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half3_rtp(double3 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half4_rtp(double4 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half8_rtp(double8 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half16_rtp(double16 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half2_rtn(double2 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half3_rtn(double3 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half4_rtn(double4 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half8_rtn(double8 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstore_half16_rtn(double16 data, size_t offset, __private half *p);

float __attribute__((overloadable)) vloada_half(size_t offset, const __global half *p);
float2 __attribute__((overloadable)) vloada_half2(size_t offset, const __global half *p);
float3 __attribute__((overloadable)) vloada_half3(size_t offset, const __global half *p);
float4 __attribute__((overloadable)) vloada_half4(size_t offset, const __global half *p);
float8 __attribute__((overloadable)) vloada_half8(size_t offset, const __global half *p);
float16 __attribute__((overloadable)) vloada_half16(size_t offset, const __global half *p);
float __attribute__((overloadable)) vloada_half(size_t offset, const __local half *p);
float2 __attribute__((overloadable)) vloada_half2(size_t offset, const __local half *p);
float3 __attribute__((overloadable)) vloada_half3(size_t offset, const __local half *p);
float4 __attribute__((overloadable)) vloada_half4(size_t offset, const __local half *p);
float8 __attribute__((overloadable)) vloada_half8(size_t offset, const __local half *p);
float16 __attribute__((overloadable)) vloada_half16(size_t offset, const __local half *p);
float __attribute__((overloadable)) vloada_half(size_t offset, const __private half *p);
float2 __attribute__((overloadable)) vloada_half2(size_t offset, const __private half *p);
float3 __attribute__((overloadable)) vloada_half3(size_t offset, const __private half *p);
float4 __attribute__((overloadable)) vloada_half4(size_t offset, const __private half *p);
float8 __attribute__((overloadable)) vloada_half8(size_t offset, const __private half *p);
float16 __attribute__((overloadable)) vloada_half16(size_t offset, const __private half *p);

void __attribute__((overloadable)) vstorea_half(float data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half2(float2 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half3(float3 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half4(float4 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half8(float8 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half16(float16 data, size_t offset, __global half *p);

void __attribute__((overloadable)) vstorea_half_rte(float data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half2_rte(float2 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half3_rte(float3 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half4_rte(float4 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half8_rte(float8 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half16_rte(float16 data, size_t offset, __global half *p);

void __attribute__((overloadable)) vstorea_half_rtz(float data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half2_rtz(float2 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half3_rtz(float3 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half4_rtz(float4 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half8_rtz(float8 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half16_rtz(float16 data, size_t offset, __global half *p);

void __attribute__((overloadable)) vstorea_half_rtp(float data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half2_rtp(float2 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half3_rtp(float3 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half4_rtp(float4 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half8_rtp(float8 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half16_rtp(float16 data, size_t offset, __global half *p);

void __attribute__((overloadable)) vstorea_half_rtn(float data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half2_rtn(float2 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half3_rtn(float3 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half4_rtn(float4 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half8_rtn(float8 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half16_rtn(float16 data, size_t offset, __global half *p);

void __attribute__((overloadable)) vstorea_half(float data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half2(float2 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half3(float3 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half4(float4 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half8(float8 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half16(float16 data, size_t offset, __local half *p);

void __attribute__((overloadable)) vstorea_half_rte(float data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half2_rte(float2 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half3_rte(float3 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half4_rte(float4 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half8_rte(float8 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half16_rte(float16 data, size_t offset, __local half *p);

void __attribute__((overloadable)) vstorea_half_rtz(float data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half2_rtz(float2 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half3_rtz(float3 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half4_rtz(float4 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half8_rtz(float8 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half16_rtz(float16 data, size_t offset, __local half *p);

void __attribute__((overloadable)) vstorea_half_rtp(float data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half2_rtp(float2 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half3_rtp(float3 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half4_rtp(float4 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half8_rtp(float8 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half16_rtp(float16 data, size_t offset, __local half *p);

void __attribute__((overloadable)) vstorea_half_rtn(float data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half2_rtn(float2 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half3_rtn(float3 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half4_rtn(float4 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half8_rtn(float8 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half16_rtn(float16 data, size_t offset, __local half *p);

void __attribute__((overloadable)) vstorea_half(float data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half2(float2 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half3(float3 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half4(float4 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half8(float8 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half16(float16 data, size_t offset, __private half *p);

void __attribute__((overloadable)) vstorea_half_rte(float data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half2_rte(float2 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half3_rte(float3 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half4_rte(float4 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half8_rte(float8 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half16_rte(float16 data, size_t offset, __private half *p);

void __attribute__((overloadable)) vstorea_half_rtz(float data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half2_rtz(float2 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half3_rtz(float3 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half4_rtz(float4 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half8_rtz(float8 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half16_rtz(float16 data, size_t offset, __private half *p);

void __attribute__((overloadable)) vstorea_half_rtp(float data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half2_rtp(float2 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half3_rtp(float3 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half4_rtp(float4 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half8_rtp(float8 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half16_rtp(float16 data, size_t offset, __private half *p);

void __attribute__((overloadable)) vstorea_half_rtn(float data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half2_rtn(float2 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half3_rtn(float3 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half4_rtn(float4 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half8_rtn(float8 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half16_rtn(float16 data, size_t offset, __private half *p);

void __attribute__((overloadable)) vstorea_half(double data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half2(double2 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half3(double3 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half4(double4 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half8(double8 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half16(double16 data, size_t offset, __global half *p);

void __attribute__((overloadable)) vstorea_half_rte(double data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half2_rte(double2 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half3_rte(double3 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half4_rte(double4 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half8_rte(double8 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half16_rte(double16 data, size_t offset, __global half *p);

void __attribute__((overloadable)) vstorea_half_rtz(double data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half2_rtz(double2 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half3_rtz(double3 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half4_rtz(double4 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half8_rtz(double8 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half16_rtz(double16 data, size_t offset, __global half *p);

void __attribute__((overloadable)) vstorea_half_rtp(double data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half2_rtp(double2 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half3_rtp(double3 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half4_rtp(double4 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half8_rtp(double8 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half16_rtp(double16 data, size_t offset, __global half *p);

void __attribute__((overloadable)) vstorea_half_rtn(double data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half2_rtn(double2 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half3_rtn(double3 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half4_rtn(double4 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half8_rtn(double8 data, size_t offset, __global half *p);
void __attribute__((overloadable)) vstorea_half16_rtn(double16 data, size_t offset, __global half *p);

void __attribute__((overloadable)) vstorea_half(double data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half2(double2 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half3(double3 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half4(double4 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half8(double8 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half16(double16 data, size_t offset, __local half *p);

void __attribute__((overloadable)) vstorea_half_rte(double data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half2_rte(double2 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half3_rte(double3 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half4_rte(double4 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half8_rte(double8 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half16_rte(double16 data, size_t offset, __local half *p);

void __attribute__((overloadable)) vstorea_half_rtz(double data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half2_rtz(double2 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half3_rtz(double3 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half4_rtz(double4 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half8_rtz(double8 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half16_rtz(double16 data, size_t offset, __local half *p);

void __attribute__((overloadable)) vstorea_half_rtp(double data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half2_rtp(double2 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half3_rtp(double3 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half4_rtp(double4 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half8_rtp(double8 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half16_rtp(double16 data, size_t offset, __local half *p);

void __attribute__((overloadable)) vstorea_half_rtn(double data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half2_rtn(double2 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half3_rtn(double3 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half4_rtn(double4 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half8_rtn(double8 data, size_t offset, __local half *p);
void __attribute__((overloadable)) vstorea_half16_rtn(double16 data, size_t offset, __local half *p);

void __attribute__((overloadable)) vstorea_half(double data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half2(double2 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half3(double3 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half4(double4 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half8(double8 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half16(double16 data, size_t offset, __private half *p);

void __attribute__((overloadable)) vstorea_half_rte(double data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half2_rte(double2 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half3_rte(double3 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half4_rte(double4 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half8_rte(double8 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half16_rte(double16 data, size_t offset, __private half *p);

void __attribute__((overloadable)) vstorea_half_rtz(double data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half2_rtz(double2 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half3_rtz(double3 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half4_rtz(double4 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half8_rtz(double8 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half16_rtz(double16 data, size_t offset, __private half *p);

void __attribute__((overloadable)) vstorea_half_rtp(double data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half2_rtp(double2 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half3_rtp(double3 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half4_rtp(double4 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half8_rtp(double8 data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half16_rtp(double16 data, size_t offset, __private half *p);

void __attribute__((overloadable)) vstorea_half_rtn(double data, size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half2_rtn(double2 data,size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half3_rtn(double3 data,size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half4_rtn(double4 data,size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half8_rtn(double8 data,size_t offset, __private half *p);
void __attribute__((overloadable)) vstorea_half16_rtn(double16 data,size_t offset, __private half *p);

float __attribute__((overloadable)) fract(float x, __global float *iptr);
float2 __attribute__((overloadable)) fract(float2 x, __global float2 *iptr);
float3 __attribute__((overloadable)) fract(float3 x, __global float3 *iptr);
float4 __attribute__((overloadable)) fract(float4 x, __global float4 *iptr);
float8 __attribute__((overloadable)) fract(float8 x, __global float8 *iptr);
float16 __attribute__((overloadable)) fract(float16 x, __global float16 *iptr);
float __attribute__((overloadable)) fract(float x, __local float *iptr);
float2 __attribute__((overloadable)) fract(float2 x, __local float2 *iptr);
float3 __attribute__((overloadable)) fract(float3 x, __local float3 *iptr);
float4 __attribute__((overloadable)) fract(float4 x, __local float4 *iptr);
float8 __attribute__((overloadable)) fract(float8 x, __local float8 *iptr);
float16 __attribute__((overloadable)) fract(float16 x, __local float16 *iptr);
float __attribute__((overloadable)) fract(float x, __private float *iptr);
float2 __attribute__((overloadable)) fract(float2 x, __private float2 *iptr);
float3 __attribute__((overloadable)) fract(float3 x, __private float3 *iptr);
float4 __attribute__((overloadable)) fract(float4 x, __private float4 *iptr);
float8 __attribute__((overloadable)) fract(float8 x, __private float8 *iptr);
float16 __attribute__((overloadable)) fract(float16 x, __private float16 *iptr);
double __attribute__((overloadable)) fract(double x, __global double *iptr);
double2 __attribute__((overloadable)) fract(double2 x, __global double2 *iptr);
double3 __attribute__((overloadable)) fract(double3 x, __global double3 *iptr);
double4 __attribute__((overloadable)) fract(double4 x, __global double4 *iptr);
double8 __attribute__((overloadable)) fract(double8 x, __global double8 *iptr);
double16 __attribute__((overloadable)) fract(double16 x, __global double16 *iptr);
double __attribute__((overloadable)) fract(double x, __local double *iptr);
double2 __attribute__((overloadable)) fract(double2 x, __local double2 *iptr);
double3 __attribute__((overloadable)) fract(double3 x, __local double3 *iptr);
double4 __attribute__((overloadable)) fract(double4 x, __local double4 *iptr);
double8 __attribute__((overloadable)) fract(double8 x, __local double8 *iptr);
double16 __attribute__((overloadable)) fract(double16 x, __local double16 *iptr);
double __attribute__((overloadable)) fract(double x, __private double *iptr);
double2 __attribute__((overloadable)) fract(double2 x, __private double2 *iptr);
double3 __attribute__((overloadable)) fract(double3 x, __private double3 *iptr);
double4 __attribute__((overloadable)) fract(double4 x, __private double4 *iptr);
double8 __attribute__((overloadable)) fract(double8 x, __private double8 *iptr);
double16 __attribute__((overloadable)) fract(double16 x, __private double16 *iptr);
//half __attribute__((overloadable)) fract(half x, __global half *iptr);
//half2 __attribute__((overloadable)) fract(half2 x, __global half2 *iptr);
//half3 __attribute__((overloadable)) fract(half3 x, __global half3 *iptr);
//half4 __attribute__((overloadable)) fract(half4 x, __global half4 *iptr);
//half8 __attribute__((overloadable)) fract(half8 x, __global half8 *iptr);
//half16 __attribute__((overloadable)) fract(half16 x, __global half16 *iptr);
//half __attribute__((overloadable)) fract(half x, __local half *iptr);
//half2 __attribute__((overloadable)) fract(half2 x, __local half2 *iptr);
//half3 __attribute__((overloadable)) fract(half3 x, __local half3 *iptr);
//half4 __attribute__((overloadable)) fract(half4 x, __local half4 *iptr);
//half8 __attribute__((overloadable)) fract(half8 x, __local half8 *iptr);
//half16 __attribute__((overloadable)) fract(half16 x, __local half16 *iptr);
//half __attribute__((overloadable)) fract(half x, __private half *iptr);
//half2 __attribute__((overloadable)) fract(half2 x, __private half2 *iptr);
//half3 __attribute__((overloadable)) fract(half3 x, __private half3 *iptr);
//half4 __attribute__((overloadable)) fract(half4 x, __private half4 *iptr);
//half8 __attribute__((overloadable)) fract(half8 x, __private half8 *iptr);
//half16 __attribute__((overloadable)) fract(half16 x, __private half16 *iptr);

float __attribute__((overloadable)) frexp(float x, __global int *exp);
float2 __attribute__((overloadable)) frexp(float2 x, __global int2 *exp);
float3 __attribute__((overloadable)) frexp(float3 x, __global int3 *exp);
float4 __attribute__((overloadable)) frexp(float4 x, __global int4 *exp);
float8 __attribute__((overloadable)) frexp(float8 x, __global int8 *exp);
float16 __attribute__((overloadable)) frexp(float16 x, __global int16 *exp);
float __attribute__((overloadable)) frexp(float x, __local int *exp);
float2 __attribute__((overloadable)) frexp(float2 x, __local int2 *exp);
float3 __attribute__((overloadable)) frexp(float3 x, __local int3 *exp);
float4 __attribute__((overloadable)) frexp(float4 x, __local int4 *exp);
float8 __attribute__((overloadable)) frexp(float8 x, __local int8 *exp);
float16 __attribute__((overloadable)) frexp(float16 x, __local int16 *exp);
float __attribute__((overloadable)) frexp(float x, __private int *exp);
float2 __attribute__((overloadable)) frexp(float2 x, __private int2 *exp);
float3 __attribute__((overloadable)) frexp(float3 x, __private int3 *exp);
float4 __attribute__((overloadable)) frexp(float4 x, __private int4 *exp);
float8 __attribute__((overloadable)) frexp(float8 x, __private int8 *exp);
float16 __attribute__((overloadable)) frexp(float16 x, __private int16 *exp);
double __attribute__((overloadable)) frexp(double x, __global int *exp);
double2 __attribute__((overloadable)) frexp(double2 x, __global int2 *exp);
double3 __attribute__((overloadable)) frexp(double3 x, __global int3 *exp);
double4 __attribute__((overloadable)) frexp(double4 x, __global int4 *exp);
double8 __attribute__((overloadable)) frexp(double8 x, __global int8 *exp);
double16 __attribute__((overloadable)) frexp(double16 x, __global int16 *exp);
double __attribute__((overloadable)) frexp(double x, __local int *exp);
double2 __attribute__((overloadable)) frexp(double2 x, __local int2 *exp);
double3 __attribute__((overloadable)) frexp(double3 x, __local int3 *exp);
double4 __attribute__((overloadable)) frexp(double4 x, __local int4 *exp);
double8 __attribute__((overloadable)) frexp(double8 x, __local int8 *exp);
double16 __attribute__((overloadable)) frexp(double16 x, __local int16 *exp);
double __attribute__((overloadable)) frexp(double x, __private int *exp);
double2 __attribute__((overloadable)) frexp(double2 x, __private int2 *exp);
double3 __attribute__((overloadable)) frexp(double3 x, __private int3 *exp);
double4 __attribute__((overloadable)) frexp(double4 x, __private int4 *exp);
double8 __attribute__((overloadable)) frexp(double8 x, __private int8 *exp);
double16 __attribute__((overloadable)) frexp(double16 x, __private int16 *exp);
//half __attribute__((overloadable)) frexp(half x, __global int *exp);
//half2 __attribute__((overloadable)) frexp(half2 x, __global int2 *exp);
//half3 __attribute__((overloadable)) frexp(half3 x, __global int3 *exp);
//half4 __attribute__((overloadable)) frexp(half4 x, __global int4 *exp);
//half8 __attribute__((overloadable)) frexp(half8 x, __global int8 *exp);
//half16 __attribute__((overloadable)) frexp(half16 x, __global int16 *exp);
//half __attribute__((overloadable)) frexp(half x, __local int *exp);
//half2 __attribute__((overloadable)) frexp(half2 x, __local int2 *exp);
//half3 __attribute__((overloadable)) frexp(half3 x, __local int3 *exp);
//half4 __attribute__((overloadable)) frexp(half4 x, __local int4 *exp);
//half8 __attribute__((overloadable)) frexp(half8 x, __local int8 *exp);
//half16 __attribute__((overloadable)) frexp(half16 x, __local int16 *exp);
//half __attribute__((overloadable)) frexp(half x, __private int *exp);
//half2 __attribute__((overloadable)) frexp(half2 x, __private int2 *exp);
//half3 __attribute__((overloadable)) frexp(half3 x, __private int3 *exp);
//half4 __attribute__((overloadable)) frexp(half4 x, __private int4 *exp);
//half8 __attribute__((overloadable)) frexp(half8 x, __private int8 *exp);
//half16 __attribute__((overloadable)) frexp(half16 x, __private int16 *exp);

float __attribute__((overloadable)) lgamma_r(float x, __global int *signp);
float2 __attribute__((overloadable)) lgamma_r(float2 x, __global int2 *signp);
float3 __attribute__((overloadable)) lgamma_r(float3 x, __global int3 *signp);
float4 __attribute__((overloadable)) lgamma_r(float4 x, __global int4 *signp);
float8 __attribute__((overloadable)) lgamma_r(float8 x, __global int8 *signp);
float16 __attribute__((overloadable)) lgamma_r(float16 x, __global int16 *signp);
float __attribute__((overloadable)) lgamma_r(float x, __local int *signp);
float2 __attribute__((overloadable)) lgamma_r(float2 x, __local int2 *signp);
float3 __attribute__((overloadable)) lgamma_r(float3 x, __local int3 *signp);
float4 __attribute__((overloadable)) lgamma_r(float4 x, __local int4 *signp);
float8 __attribute__((overloadable)) lgamma_r(float8 x, __local int8 *signp);
float16 __attribute__((overloadable)) lgamma_r(float16 x, __local int16 *signp);
float __attribute__((overloadable)) lgamma_r(float x, __private int *signp);
float2 __attribute__((overloadable)) lgamma_r(float2 x, __private int2 *signp);
float3 __attribute__((overloadable)) lgamma_r(float3 x, __private int3 *signp);
float4 __attribute__((overloadable)) lgamma_r(float4 x, __private int4 *signp);
float8 __attribute__((overloadable)) lgamma_r(float8 x, __private int8 *signp);
float16 __attribute__((overloadable)) lgamma_r(float16 x, __private int16 *signp);
double __attribute__((overloadable)) lgamma_r(double x, __global int *signp);
double2 __attribute__((overloadable)) lgamma_r(double2 x, __global int2 *signp);
double3 __attribute__((overloadable)) lgamma_r(double3 x, __global int3 *signp);
double4 __attribute__((overloadable)) lgamma_r(double4 x, __global int4 *signp);
double8 __attribute__((overloadable)) lgamma_r(double8 x, __global int8 *signp);
double16 __attribute__((overloadable)) lgamma_r(double16 x, __global int16 *signp);
double __attribute__((overloadable)) lgamma_r(double x, __local int *signp);
double2 __attribute__((overloadable)) lgamma_r(double2 x, __local int2 *signp);
double3 __attribute__((overloadable)) lgamma_r(double3 x, __local int3 *signp);
double4 __attribute__((overloadable)) lgamma_r(double4 x, __local int4 *signp);
double8 __attribute__((overloadable)) lgamma_r(double8 x, __local int8 *signp);
double16 __attribute__((overloadable)) lgamma_r(double16 x, __local int16 *signp);
double __attribute__((overloadable)) lgamma_r(double x, __private int *signp);
double2 __attribute__((overloadable)) lgamma_r(double2 x, __private int2 *signp);
double3 __attribute__((overloadable)) lgamma_r(double3 x, __private int3 *signp);
double4 __attribute__((overloadable)) lgamma_r(double4 x, __private int4 *signp);
double8 __attribute__((overloadable)) lgamma_r(double8 x, __private int8 *signp);
double16 __attribute__((overloadable)) lgamma_r(double16 x, __private int16 *signp);

float __attribute__((overloadable)) modf(float x, __global float *iptr);
float2 __attribute__((overloadable)) modf(float2 x, __global float2 *iptr);
float3 __attribute__((overloadable)) modf(float3 x, __global float3 *iptr);
float4 __attribute__((overloadable)) modf(float4 x, __global float4 *iptr);
float8 __attribute__((overloadable)) modf(float8 x, __global float8 *iptr);
float16 __attribute__((overloadable)) modf(float16 x, __global float16 *iptr);
float __attribute__((overloadable)) modf(float x, __local float *iptr);
float2 __attribute__((overloadable)) modf(float2 x, __local float2 *iptr);
float3 __attribute__((overloadable)) modf(float3 x, __local float3 *iptr);
float4 __attribute__((overloadable)) modf(float4 x, __local float4 *iptr);
float8 __attribute__((overloadable)) modf(float8 x, __local float8 *iptr);
float16 __attribute__((overloadable)) modf(float16 x, __local float16 *iptr);
float __attribute__((overloadable)) modf(float x, __private float *iptr);
float2 __attribute__((overloadable)) modf(float2 x, __private float2 *iptr);
float3 __attribute__((overloadable)) modf(float3 x, __private float3 *iptr);
float4 __attribute__((overloadable)) modf(float4 x, __private float4 *iptr);
float8 __attribute__((overloadable)) modf(float8 x, __private float8 *iptr);
float16 __attribute__((overloadable)) modf(float16 x, __private float16 *iptr);
double __attribute__((overloadable)) modf(double x, __global double *iptr);
double2 __attribute__((overloadable)) modf(double2 x, __global double2 *iptr);
double3 __attribute__((overloadable)) modf(double3 x, __global double3 *iptr);
double4 __attribute__((overloadable)) modf(double4 x, __global double4 *iptr);
double8 __attribute__((overloadable)) modf(double8 x, __global double8 *iptr);
double16 __attribute__((overloadable)) modf(double16 x, __global double16 *iptr);
double __attribute__((overloadable)) modf(double x, __local double *iptr);
double2 __attribute__((overloadable)) modf(double2 x, __local double2 *iptr);
double3 __attribute__((overloadable)) modf(double3 x, __local double3 *iptr);
double4 __attribute__((overloadable)) modf(double4 x, __local double4 *iptr);
double8 __attribute__((overloadable)) modf(double8 x, __local double8 *iptr);
double16 __attribute__((overloadable)) modf(double16 x, __local double16 *iptr);
double __attribute__((overloadable)) modf(double x, __private double *iptr);
double2 __attribute__((overloadable)) modf(double2 x, __private double2 *iptr);
double3 __attribute__((overloadable)) modf(double3 x, __private double3 *iptr);
double4 __attribute__((overloadable)) modf(double4 x, __private double4 *iptr);
double8 __attribute__((overloadable)) modf(double8 x, __private double8 *iptr);
double16 __attribute__((overloadable)) modf(double16 x, __private double16 *iptr);
//half __attribute__((overloadable)) modf(half x, __global half *iptr);
//half2 __attribute__((overloadable)) modf(half2 x, __global half2 *iptr);
//half3 __attribute__((overloadable)) modf(half3 x, __global half3 *iptr);
//half4 __attribute__((overloadable)) modf(half4 x, __global half4 *iptr);
//half8 __attribute__((overloadable)) modf(half8 x, __global half8 *iptr);
//half16 __attribute__((overloadable)) modf(half16 x, __global half16 *iptr);
//half __attribute__((overloadable)) modf(half x, __local half *iptr);
//half2 __attribute__((overloadable)) modf(half2 x, __local half2 *iptr);
//half3 __attribute__((overloadable)) modf(half3 x, __local half3 *iptr);
//half4 __attribute__((overloadable)) modf(half4 x, __local half4 *iptr);
//half8 __attribute__((overloadable)) modf(half8 x, __local half8 *iptr);
//half16 __attribute__((overloadable)) modf(half16 x, __local half16 *iptr);
//half __attribute__((overloadable)) modf(half x, __private half *iptr);
//half2 __attribute__((overloadable)) modf(half2 x, __private half2 *iptr);
//half3 __attribute__((overloadable)) modf(half3 x, __private half3 *iptr);
//half4 __attribute__((overloadable)) modf(half4 x, __private half4 *iptr);
//half8 __attribute__((overloadable)) modf(half8 x, __private half8 *iptr);
//half16 __attribute__((overloadable)) modf(half16 x, __private half16 *iptr);

float __attribute__((overloadable)) remquo(float x, float y, __global int *quo);
float2 __attribute__((overloadable)) remquo(float2 x, float2 y, __global int2 *quo);
float3 __attribute__((overloadable)) remquo(float3 x, float3 y, __global int3 *quo);
float4 __attribute__((overloadable)) remquo(float4 x, float4 y, __global int4 *quo);
float8 __attribute__((overloadable)) remquo(float8 x, float8 y, __global int8 *quo);
float16 __attribute__((overloadable)) remquo(float16 x, float16 y, __global int16 *quo);
float __attribute__((overloadable)) remquo(float x, float y, __local int *quo);
float2 __attribute__((overloadable)) remquo(float2 x, float2 y, __local int2 *quo);
float3 __attribute__((overloadable)) remquo(float3 x, float3 y, __local int3 *quo);
float4 __attribute__((overloadable)) remquo(float4 x, float4 y, __local int4 *quo);
float8 __attribute__((overloadable)) remquo(float8 x, float8 y, __local int8 *quo);
float16 __attribute__((overloadable)) remquo(float16 x, float16 y, __local int16 *quo);
float __attribute__((overloadable)) remquo(float x, float y, __private int *quo);
float2 __attribute__((overloadable)) remquo(float2 x, float2 y, __private int2 *quo);
float3 __attribute__((overloadable)) remquo(float3 x, float3 y, __private int3 *quo);
float4 __attribute__((overloadable)) remquo(float4 x, float4 y, __private int4 *quo);
float8 __attribute__((overloadable)) remquo(float8 x, float8 y, __private int8 *quo);
float16 __attribute__((overloadable)) remquo(float16 x, float16 y, __private int16 *quo);
double __attribute__((overloadable)) remquo(double x, double y, __global int *quo);
double2 __attribute__((overloadable)) remquo(double2 x, double2 y, __global int2 *quo);
double3 __attribute__((overloadable)) remquo(double3 x, double3 y, __global int3 *quo);
double4 __attribute__((overloadable)) remquo(double4 x, double4 y, __global int4 *quo);
double8 __attribute__((overloadable)) remquo(double8 x, double8 y, __global int8 *quo);
double16 __attribute__((overloadable)) remquo(double16 x, double16 y, __global int16 *quo);
double __attribute__((overloadable)) remquo(double x, double y, __local int *quo);
double2 __attribute__((overloadable)) remquo(double2 x, double2 y, __local int2 *quo);
double3 __attribute__((overloadable)) remquo(double3 x, double3 y, __local int3 *quo);
double4 __attribute__((overloadable)) remquo(double4 x, double4 y, __local int4 *quo);
double8 __attribute__((overloadable)) remquo(double8 x, double8 y, __local int8 *quo);
double16 __attribute__((overloadable)) remquo(double16 x, double16 y, __local int16 *quo);
double __attribute__((overloadable)) remquo(double x, double y, __private int *quo);
double2 __attribute__((overloadable)) remquo(double2 x, double2 y, __private int2 *quo);
double3 __attribute__((overloadable)) remquo(double3 x, double3 y, __private int3 *quo);
double4 __attribute__((overloadable)) remquo(double4 x, double4 y, __private int4 *quo);
double8 __attribute__((overloadable)) remquo(double8 x, double8 y, __private int8 *quo);
double16 __attribute__((overloadable)) remquo(double16 x, double16 y, __private int16 *quo);
//half __attribute__((overloadable)) remquo(half x, half y, __global int *quo);
//half2 __attribute__((overloadable)) remquo(half2 x, half2 y, __global int2 *quo);
//half3 __attribute__((overloadable)) remquo(half3 x, half3 y, __global int3 *quo);
//half4 __attribute__((overloadable)) remquo(half4 x, half4 y, __global int4 *quo);
//half8 __attribute__((overloadable)) remquo(half8 x, half8 y, __global int8 *quo);
//half16 __attribute__((overloadable)) remquo(half16 x, half16 y, __global int16 *quo);
//half __attribute__((overloadable)) remquo(half x, half y, __local int *quo);
//half2 __attribute__((overloadable)) remquo(half2 x, half2 y, __local int2 *quo);
//half3 __attribute__((overloadable)) remquo(half3 x, half3 y, __local int3 *quo);
//half4 __attribute__((overloadable)) remquo(half4 x, half4 y, __local int4 *quo);
//half8 __attribute__((overloadable)) remquo(half8 x, half8 y, __local int8 *quo);
//half16 __attribute__((overloadable)) remquo(half16 x, half16 y, __local int16 *quo);
//half __attribute__((overloadable)) remquo(half x, half y, __private int *quo);
//half2 __attribute__((overloadable)) remquo(half2 x, half2 y, __private int2 *quo);
//half3 __attribute__((overloadable)) remquo(half3 x, half3 y, __private int3 *quo);
//half4 __attribute__((overloadable)) remquo(half4 x, half4 y, __private int4 *quo);
//half8 __attribute__((overloadable)) remquo(half8 x, half8 y, __private int8 *quo);
//half16 __attribute__((overloadable)) remquo(half16 x, half16 y, __private int16 *quo);

float __attribute__((overloadable)) sincos(float x, __global float *cosval);
float2 __attribute__((overloadable)) sincos(float2 x, __global float2 *cosval);
float3 __attribute__((overloadable)) sincos(float3 x, __global float3 *cosval);
float4 __attribute__((overloadable)) sincos(float4 x, __global float4 *cosval);
float8 __attribute__((overloadable)) sincos(float8 x, __global float8 *cosval);
float16 __attribute__((overloadable)) sincos(float16 x, __global float16 *cosval);
float __attribute__((overloadable)) sincos(float x, __local float *cosval);
float2 __attribute__((overloadable)) sincos(float2 x, __local float2 *cosval);
float3 __attribute__((overloadable)) sincos(float3 x, __local float3 *cosval);
float4 __attribute__((overloadable)) sincos(float4 x, __local float4 *cosval);
float8 __attribute__((overloadable)) sincos(float8 x, __local float8 *cosval);
float16 __attribute__((overloadable)) sincos(float16 x, __local float16 *cosval);
float __attribute__((overloadable)) sincos(float x, __private float *cosval);
float2 __attribute__((overloadable)) sincos(float2 x, __private float2 *cosval);
float3 __attribute__((overloadable)) sincos(float3 x, __private float3 *cosval);
float4 __attribute__((overloadable)) sincos(float4 x, __private float4 *cosval);
float8 __attribute__((overloadable)) sincos(float8 x, __private float8 *cosval);
float16 __attribute__((overloadable)) sincos(float16 x, __private float16 *cosval);
double __attribute__((overloadable)) sincos(double x, __global double *cosval);
double2 __attribute__((overloadable)) sincos(double2 x, __global double2 *cosval);
double3 __attribute__((overloadable)) sincos(double3 x, __global double3 *cosval);
double4 __attribute__((overloadable)) sincos(double4 x, __global double4 *cosval);
double8 __attribute__((overloadable)) sincos(double8 x, __global double8 *cosval);
double16 __attribute__((overloadable)) sincos(double16 x, __global double16 *cosval);
double __attribute__((overloadable)) sincos(double x, __local double *cosval);
double2 __attribute__((overloadable)) sincos(double2 x, __local double2 *cosval);
double3 __attribute__((overloadable)) sincos(double3 x, __local double3 *cosval);
double4 __attribute__((overloadable)) sincos(double4 x, __local double4 *cosval);
double8 __attribute__((overloadable)) sincos(double8 x, __local double8 *cosval);
double16 __attribute__((overloadable)) sincos(double16 x, __local double16 *cosval);
double __attribute__((overloadable)) sincos(double x, __private double *cosval);
double2 __attribute__((overloadable)) sincos(double2 x, __private double2 *cosval);
double3 __attribute__((overloadable)) sincos(double3 x, __private double3 *cosval);
double4 __attribute__((overloadable)) sincos(double4 x, __private double4 *cosval);
double8 __attribute__((overloadable)) sincos(double8 x, __private double8 *cosval);
double16 __attribute__((overloadable)) sincos(double16 x, __private double16 *cosval);
//half __attribute__((overloadable)) sincos(half x, __global half *cosval);
//half2 __attribute__((overloadable)) sincos(half2 x, __global half2 *cosval);
//half3 __attribute__((overloadable)) sincos(half3 x, __global half3 *cosval);
//half4 __attribute__((overloadable)) sincos(half4 x, __global half4 *cosval);
//half8 __attribute__((overloadable)) sincos(half8 x, __global half8 *cosval);
//half16 __attribute__((overloadable)) sincos(half16 x, __global half16 *cosval);
//half __attribute__((overloadable)) sincos(half x, __local half *cosval);
//half2 __attribute__((overloadable)) sincos(half2 x, __local half2 *cosval);
//half3 __attribute__((overloadable)) sincos(half3 x, __local half3 *cosval);
//half4 __attribute__((overloadable)) sincos(half4 x, __local half4 *cosval);
//half8 __attribute__((overloadable)) sincos(half8 x, __local half8 *cosval);
//half16 __attribute__((overloadable)) sincos(half16 x, __local half16 *cosval);
//half __attribute__((overloadable)) sincos(half x, __private half *cosval);
//half2 __attribute__((overloadable)) sincos(half2 x, __private half2 *cosval);
//half3 __attribute__((overloadable)) sincos(half3 x, __private half3 *cosval);
//half4 __attribute__((overloadable)) sincos(half4 x, __private half4 *cosval);
//half8 __attribute__((overloadable)) sincos(half8 x, __private half8 *cosval);
//half16 __attribute__((overloadable)) sincos(half16 x, __private half16 *cosval);

#endif //__OPENCL_C_VERSION__ < 200

char2 __attribute__((overloadable)) vload2(size_t offset, const __constant char *p);
uchar2 __attribute__((overloadable)) vload2(size_t offset, const __constant uchar *p);
short2 __attribute__((overloadable)) vload2(size_t offset, const __constant short *p);
ushort2 __attribute__((overloadable)) vload2(size_t offset, const __constant ushort *p);
int2 __attribute__((overloadable)) vload2(size_t offset, const __constant int *p);
uint2 __attribute__((overloadable)) vload2(size_t offset, const __constant uint *p);
long2 __attribute__((overloadable)) vload2(size_t offset, const __constant long *p);
ulong2 __attribute__((overloadable)) vload2(size_t offset, const __constant ulong *p);
float2 __attribute__((overloadable)) vload2(size_t offset, const __constant float *p);
char3 __attribute__((overloadable)) vload3(size_t offset, const __constant char *p);
uchar3 __attribute__((overloadable)) vload3(size_t offset, const __constant uchar *p);
short3 __attribute__((overloadable)) vload3(size_t offset, const __constant short *p);
ushort3 __attribute__((overloadable)) vload3(size_t offset, const __constant ushort *p);
int3 __attribute__((overloadable)) vload3(size_t offset, const __constant int *p);
uint3 __attribute__((overloadable)) vload3(size_t offset, const __constant uint *p);
long3 __attribute__((overloadable)) vload3(size_t offset, const __constant long *p);
ulong3 __attribute__((overloadable)) vload3(size_t offset, const __constant ulong *p);
float3 __attribute__((overloadable)) vload3(size_t offset, const __constant float *p);
char4 __attribute__((overloadable)) vload4(size_t offset, const __constant char *p);
uchar4 __attribute__((overloadable)) vload4(size_t offset, const __constant uchar *p);
short4 __attribute__((overloadable)) vload4(size_t offset, const __constant short *p);
ushort4 __attribute__((overloadable)) vload4(size_t offset, const __constant ushort *p);
int4 __attribute__((overloadable)) vload4(size_t offset, const __constant int *p);
uint4 __attribute__((overloadable)) vload4(size_t offset, const __constant uint *p);
long4 __attribute__((overloadable)) vload4(size_t offset, const __constant long *p);
ulong4 __attribute__((overloadable)) vload4(size_t offset, const __constant ulong *p);
float4 __attribute__((overloadable)) vload4(size_t offset, const __constant float *p);
char8 __attribute__((overloadable)) vload8(size_t offset, const __constant char *p);
uchar8 __attribute__((overloadable)) vload8(size_t offset, const __constant uchar *p);
short8 __attribute__((overloadable)) vload8(size_t offset, const __constant short *p);
ushort8 __attribute__((overloadable)) vload8(size_t offset, const __constant ushort *p);
int8 __attribute__((overloadable)) vload8(size_t offset, const __constant int *p);
uint8 __attribute__((overloadable)) vload8(size_t offset, const __constant uint *p);
long8 __attribute__((overloadable)) vload8(size_t offset, const __constant long *p);
ulong8 __attribute__((overloadable)) vload8(size_t offset, const __constant ulong *p);
float8 __attribute__((overloadable)) vload8(size_t offset, const __constant float *p);
char16 __attribute__((overloadable)) vload16(size_t offset, const __constant char *p);
uchar16 __attribute__((overloadable)) vload16(size_t offset, const __constant uchar *p);
short16 __attribute__((overloadable)) vload16(size_t offset, const __constant short *p);
ushort16 __attribute__((overloadable)) vload16(size_t offset, const __constant ushort *p);
int16 __attribute__((overloadable)) vload16(size_t offset, const __constant int *p);
uint16 __attribute__((overloadable)) vload16(size_t offset, const __constant uint *p);
long16 __attribute__((overloadable)) vload16(size_t offset, const __constant long *p);
ulong16 __attribute__((overloadable)) vload16(size_t offset, const __constant ulong *p);
float16 __attribute__((overloadable)) vload16(size_t offset, const __constant float *p);
double2 __attribute__((overloadable)) vload2(size_t offset, const __constant double *p);
double3 __attribute__((overloadable)) vload3(size_t offset, const __constant double *p);
double4 __attribute__((overloadable)) vload4(size_t offset, const __constant double *p);
double8 __attribute__((overloadable)) vload8(size_t offset, const __constant double *p);
double16 __attribute__((overloadable)) vload16(size_t offset, const __constant double *p);
//half __attribute__((overloadable)) vload(size_t offset, const __constant half *p);
//half2 __attribute__((overloadable)) vload2(size_t offset, const __constant half *p);
//half3 __attribute__((overloadable)) vload3(size_t offset, const __constant half *p);
//half4 __attribute__((overloadable)) vload4(size_t offset, const __constant half *p);
//half8 __attribute__((overloadable)) vload8(size_t offset, const __constant half *p);
//half16 __attribute__((overloadable)) vload16(size_t offset, const __constant half *p);

/**
 * Write sizeof (gentypen) bytes given by data
 * to address (p + (offset * n)). The address
 * computed as (p + (offset * n)) must be 8-bit
 * aligned if gentype is char, uchar; 16-bit
 * aligned if gentype is short, ushort; 32-bit
 * aligned if gentype is int, uint, float; 64-bit
 * aligned if gentype is long, ulong.
 */

/**
 * Read sizeof (half) bytes of data from address
 * (p + offset). The data read is interpreted as a
 * half value. The half value is converted to a
 * float value and the float value is returned.
 * The read address computed as (p + offset)
 * must be 16-bit aligned.
 */

float __attribute__((overloadable)) vload_half(size_t offset, const __constant half *p);

/**
 * Read sizeof (halfn) bytes of data from address
 * (p + (offset * n)). The data read is interpreted
 * as a halfn value. The halfn value read is
 * converted to a floatn value and the floatn
 * value is returned. The read address computed
 * as (p + (offset * n)) must be 16-bit aligned.
 */
#if __OPENCL_C_VERSION__ >= 200

float __attribute__((overloadable)) vload_half(size_t offset, const half *p);

float2 __attribute__((overloadable)) vload_half2(size_t offset, const half *p);
float3 __attribute__((overloadable)) vload_half3(size_t offset, const half *p);
float4 __attribute__((overloadable)) vload_half4(size_t offset, const half *p);
float8 __attribute__((overloadable)) vload_half8(size_t offset, const half *p);
float16 __attribute__((overloadable)) vload_half16(size_t offset, const half *p);

void __attribute__((overloadable)) vstore_half(float data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half_rte(float data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half_rtz(float data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half_rtp(float data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half_rtn(float data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half(double data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half_rte(double data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half_rtz(double data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half_rtp(double data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half_rtn(double data, size_t offset, half *p);

void __attribute__((overloadable)) vstore_half2(float2 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half3(float3 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half4(float4 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half8(float8 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half16(float16 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half2_rte(float2 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half3_rte(float3 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half4_rte(float4 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half8_rte(float8 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half16_rte(float16 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half2_rtz(float2 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half3_rtz(float3 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half4_rtz(float4 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half8_rtz(float8 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half16_rtz(float16 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half2_rtp(float2 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half3_rtp(float3 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half4_rtp(float4 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half8_rtp(float8 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half16_rtp(float16 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half2_rtn(float2 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half3_rtn(float3 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half4_rtn(float4 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half8_rtn(float8 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half16_rtn(float16 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half2(double2 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half3(double3 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half4(double4 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half8(double8 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half16(double16 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half2_rte(double2 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half3_rte(double3 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half4_rte(double4 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half8_rte(double8 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half16_rte(double16 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half2_rtz(double2 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half3_rtz(double3 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half4_rtz(double4 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half8_rtz(double8 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half16_rtz(double16 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half2_rtp(double2 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half3_rtp(double3 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half4_rtp(double4 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half8_rtp(double8 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half16_rtp(double16 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half2_rtn(double2 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half3_rtn(double3 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half4_rtn(double4 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half8_rtn(double8 data, size_t offset, half *p);
void __attribute__((overloadable)) vstore_half16_rtn(double16 data, size_t offset, half *p);


float __attribute__((overloadable)) vloada_half(size_t offset, const half *p);
float2 __attribute__((overloadable)) vloada_half2(size_t offset, const half *p);
float3 __attribute__((overloadable)) vloada_half3(size_t offset, const half *p);
float4 __attribute__((overloadable)) vloada_half4(size_t offset, const half *p);
float8 __attribute__((overloadable)) vloada_half8(size_t offset, const half *p);
float16 __attribute__((overloadable)) vloada_half16(size_t offset, const half *p);

void __attribute__((overloadable)) vstorea_half(float data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half2(float2 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half3(float3 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half4(float4 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half8(float8 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half16(float16 data, size_t offset, half *p);

void __attribute__((overloadable)) vstorea_half_rte(float data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half2_rte(float2 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half3_rte(float3 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half4_rte(float4 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half8_rte(float8 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half16_rte(float16 data, size_t offset, half *p);

void __attribute__((overloadable)) vstorea_half_rtz(float data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half2_rtz(float2 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half3_rtz(float3 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half4_rtz(float4 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half8_rtz(float8 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half16_rtz(float16 data, size_t offset, half *p);

void __attribute__((overloadable)) vstorea_half_rtp(float data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half2_rtp(float2 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half3_rtp(float3 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half4_rtp(float4 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half8_rtp(float8 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half16_rtp(float16 data, size_t offset, half *p);

void __attribute__((overloadable)) vstorea_half_rtn(float data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half2_rtn(float2 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half3_rtn(float3 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half4_rtn(float4 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half8_rtn(float8 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half16_rtn(float16 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half(double data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half2(double2 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half3(double3 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half4(double4 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half8(double8 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half16(double16 data, size_t offset, half *p);

void __attribute__((overloadable)) vstorea_half_rte(double data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half2_rte(double2 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half3_rte(double3 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half4_rte(double4 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half8_rte(double8 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half16_rte(double16 data, size_t offset, half *p);

void __attribute__((overloadable)) vstorea_half_rtz(double data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half2_rtz(double2 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half3_rtz(double3 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half4_rtz(double4 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half8_rtz(double8 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half16_rtz(double16 data, size_t offset, half *p);

void __attribute__((overloadable)) vstorea_half_rtp(double data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half2_rtp(double2 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half3_rtp(double3 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half4_rtp(double4 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half8_rtp(double8 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half16_rtp(double16 data, size_t offset, half *p);

void __attribute__((overloadable)) vstorea_half_rtn(double data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half2_rtn(double2 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half3_rtn(double3 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half4_rtn(double4 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half8_rtn(double8 data, size_t offset, half *p);
void __attribute__((overloadable)) vstorea_half16_rtn(double16 data, size_t offset, half *p);

/**
 * All work-items in a work-group executing the kernel
 * on a processor must execute this function before any
 * are allowed to continue execution beyond the
 * work_group_barrier. This function must be
 * encountered by all work-items in a work-group
 * executing the kernel. These rules apply to NDranges
 * implemented with uniform and non-uniform
 * work-groups.
 * If work_group_barrier is inside a conditional
 * statement, then all work-items must enter the
 * conditional if any work-item enters the conditional
 * statement and executes the work_group_barrier.
 * If work_group_barrier is inside a loop, all workitems
 * must execute the work_group_barrier for
 * each iteration of the loop before any are allowed to
 * continue execution beyond the
 * work_group_barrier.
 * The work_group_barrier function also supports a
 * variant that specifies the memory scope. For the
 * work_group_barrier variant that does not take a
 * memory scope, the scope is
 * memory_scope_work_group.
 * The scope argument specifies whether the memory
 * accesses of work-items in the work-group to
 * memory address space(s) identified by flags become
 * visible to all work-items in the work-group, the
 * device or all SVM devices.
 * The work_group_barrier function can also be used
 * to specify which memory operations i.e. to global
 * memory, local memory or images become visible to
 * the appropriate memory scope identified by scope.
 * The flags argument specifies the memory address
 * spaces. This is a bitfield and can be set to 0 or a
 * combination of the following values ORed together.
 * CLK_LOCAL_MEM_FENCE - The
 * work_group_barrier function will ensure that all
 * local memory accesses become visible to all workitems
 * in the work-group. Note that the value of
 * scope is ignored as the memory scope is always
 * memory_scope_work_group.
 * CLK_GLOBAL_MEM_FENCE  The
 * work_group_barrier function ensure that all global
 * memory accesses become visible to all work-items
 * in the work-group.
 * CLK_IMAGE_MEM_FENCE  The
 * work_group_barrier function will ensure that all
 * image memory accesses become visible to all workitems
 * in the work-group. The value of scope must
 * be memory_scope_work_group or
 * memory_scope_device.
 */

typedef uint memory_scope;

void __attribute__((overloadable)) work_group_barrier(cl_mem_fence_flags flags);
void __attribute__((overloadable)) work_group_barrier(cl_mem_fence_flags flags, memory_scope scope);


float __attribute__((overloadable)) fract(float x, float *iptr);
float2 __attribute__((overloadable)) fract(float2 x, float2 *iptr);
float3 __attribute__((overloadable)) fract(float3 x, float3 *iptr);
float4 __attribute__((overloadable)) fract(float4 x, float4 *iptr);
float8 __attribute__((overloadable)) fract(float8 x, float8 *iptr);
float16 __attribute__((overloadable)) fract(float16 x, float16 *iptr);
double __attribute__((overloadable)) fract(double x, double *iptr);
double2 __attribute__((overloadable)) fract(double2 x, double2 *iptr);
double3 __attribute__((overloadable)) fract(double3 x, double3 *iptr);
double4 __attribute__((overloadable)) fract(double4 x, double4 *iptr);
double8 __attribute__((overloadable)) fract(double8 x, double8 *iptr);
double16 __attribute__((overloadable)) fract(double16 x, double16 *iptr);

float __attribute__((overloadable)) frexp(float x, int *exp);
float2 __attribute__((overloadable)) frexp(float2 x, int2 *exp);
float3 __attribute__((overloadable)) frexp(float3 x, int3 *exp);
float4 __attribute__((overloadable)) frexp(float4 x, int4 *exp);
float8 __attribute__((overloadable)) frexp(float8 x, int8 *exp);
float16 __attribute__((overloadable)) frexp(float16 x, int16 *exp);
double __attribute__((overloadable)) frexp(double x, int *exp);
double2 __attribute__((overloadable)) frexp(double2 x, int2 *exp);
double3 __attribute__((overloadable)) frexp(double3 x, int3 *exp);
double4 __attribute__((overloadable)) frexp(double4 x, int4 *exp);
double8 __attribute__((overloadable)) frexp(double8 x, int8 *exp);
double16 __attribute__((overloadable)) frexp(double16 x, int16 *exp);

float __attribute__((overloadable)) lgamma_r(float x, int *signp);
float2 __attribute__((overloadable)) lgamma_r(float2 x, int2 *signp);
float3 __attribute__((overloadable)) lgamma_r(float3 x, int3 *signp);
float4 __attribute__((overloadable)) lgamma_r(float4 x, int4 *signp);
float8 __attribute__((overloadable)) lgamma_r(float8 x, int8 *signp);
float16 __attribute__((overloadable)) lgamma_r(float16 x, int16 *signp);
double __attribute__((overloadable)) lgamma_r(double x, int *signp);
double2 __attribute__((overloadable)) lgamma_r(double2 x, int2 *signp);
double3 __attribute__((overloadable)) lgamma_r(double3 x, int3 *signp);
double4 __attribute__((overloadable)) lgamma_r(double4 x, int4 *signp);
double8 __attribute__((overloadable)) lgamma_r(double8 x, int8 *signp);
double16 __attribute__((overloadable)) lgamma_r(double16 x, int16 *signp);


float __attribute__((overloadable)) modf(float x, float *iptr);
float2 __attribute__((overloadable)) modf(float2 x, float2 *iptr);
float3 __attribute__((overloadable)) modf(float3 x, float3 *iptr);
float4 __attribute__((overloadable)) modf(float4 x, float4 *iptr);
float8 __attribute__((overloadable)) modf(float8 x, float8 *iptr);
float16 __attribute__((overloadable)) modf(float16 x, float16 *iptr);
double __attribute__((overloadable)) modf(double x, double *iptr);
double2 __attribute__((overloadable)) modf(double2 x, double2 *iptr);
double3 __attribute__((overloadable)) modf(double3 x, double3 *iptr);
double4 __attribute__((overloadable)) modf(double4 x, double4 *iptr);
double8 __attribute__((overloadable)) modf(double8 x, double8 *iptr);
double16 __attribute__((overloadable)) modf(double16 x, double16 *iptr);


float __attribute__((overloadable)) remquo(float x, float y, int *quo);
float2 __attribute__((overloadable)) remquo(float2 x, float2 y, int2 *quo);
float3 __attribute__((overloadable)) remquo(float3 x, float3 y, int3 *quo);
float4 __attribute__((overloadable)) remquo(float4 x, float4 y, int4 *quo);
float8 __attribute__((overloadable)) remquo(float8 x, float8 y, int8 *quo);
float16 __attribute__((overloadable)) remquo(float16 x, float16 y, int16 *quo);
double __attribute__((overloadable)) remquo(double x, double y, int *quo);
double2 __attribute__((overloadable)) remquo(double2 x, double2 y, int2 *quo);
double3 __attribute__((overloadable)) remquo(double3 x, double3 y, int3 *quo);
double4 __attribute__((overloadable)) remquo(double4 x, double4 y, int4 *quo);
double8 __attribute__((overloadable)) remquo(double8 x, double8 y, int8 *quo);
double16 __attribute__((overloadable)) remquo(double16 x, double16 y, int16 *quo);


/**
 * Compute sine and cosine of x. The computed sine
 * is the return value and computed cosine is returned
 * in cosval.
 */
float __attribute__((overloadable)) sincos(float x, float *cosval);
float2 __attribute__((overloadable)) sincos(float2 x, float2 *cosval);
float3 __attribute__((overloadable)) sincos(float3 x, float3 *cosval);
float4 __attribute__((overloadable)) sincos(float4 x, float4 *cosval);
float8 __attribute__((overloadable)) sincos(float8 x, float8 *cosval);
float16 __attribute__((overloadable)) sincos(float16 x, float16 *cosval);
double __attribute__((overloadable)) sincos(double x, double *cosval);
double2 __attribute__((overloadable)) sincos(double2 x, double2 *cosval);
double3 __attribute__((overloadable)) sincos(double3 x, double3 *cosval);
double4 __attribute__((overloadable)) sincos(double4 x, double4 *cosval);
double8 __attribute__((overloadable)) sincos(double8 x, double8 *cosval);
double16 __attribute__((overloadable)) sincos(double16 x, double16 *cosval);

#endif // __OPENCL_C_VERSION__ >= 200


float2 __attribute__((overloadable)) vload_half2(size_t offset, const __constant half *p);
float3 __attribute__((overloadable)) vload_half3(size_t offset, const __constant half *p);
float4 __attribute__((overloadable)) vload_half4(size_t offset, const __constant half *p);
float8 __attribute__((overloadable)) vload_half8(size_t offset, const __constant half *p);
float16 __attribute__((overloadable)) vload_half16(size_t offset, const __constant half *p);

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


float __attribute__((overloadable)) vloada_half(size_t offset, const __constant half *p);
float2 __attribute__((overloadable)) vloada_half2(size_t offset, const __constant half *p);
float3 __attribute__((overloadable)) vloada_half3(size_t offset, const __constant half *p);
float4 __attribute__((overloadable)) vloada_half4(size_t offset, const __constant half *p);
float8 __attribute__((overloadable)) vloada_half8(size_t offset, const __constant half *p);
float16 __attribute__((overloadable)) vloada_half16(size_t offset, const __constant half *p);

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
event_t __attribute__((overloadable)) async_work_group_copy(__local char *dst, const __global char *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local uchar *dst, const __global uchar *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local short *dst, const __global short *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local ushort *dst, const __global ushort *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local int *dst, const __global int *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local uint *dst, const __global uint *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local long *dst, const __global long *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local ulong *dst, const __global ulong *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local float *dst, const __global float *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local char2 *dst, const __global char2 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local uchar2 *dst, const __global uchar2 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local short2 *dst, const __global short2 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local ushort2 *dst, const __global ushort2 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local int2 *dst, const __global int2 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local uint2 *dst, const __global uint2 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local long2 *dst, const __global long2 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local ulong2 *dst, const __global ulong2 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local float2 *dst, const __global float2 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local char3 *dst, const __global char3 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local uchar3 *dst, const __global uchar3 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local short3 *dst, const __global short3 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local ushort3 *dst, const __global ushort3 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local int3 *dst, const __global int3 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local uint3 *dst, const __global uint3 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local long3 *dst, const __global long3 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local ulong3 *dst, const __global ulong3 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local float3 *dst, const __global float3 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local char4 *dst, const __global char4 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local uchar4 *dst, const __global uchar4 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local short4 *dst, const __global short4 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local ushort4 *dst, const __global ushort4 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local int4 *dst, const __global int4 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local uint4 *dst, const __global uint4 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local long4 *dst, const __global long4 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local ulong4 *dst, const __global ulong4 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local float4 *dst, const __global float4 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local char8 *dst, const __global char8 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local uchar8 *dst, const __global uchar8 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local short8 *dst, const __global short8 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local ushort8 *dst, const __global ushort8 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local int8 *dst, const __global int8 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local uint8 *dst, const __global uint8 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local long8 *dst, const __global long8 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local ulong8 *dst, const __global ulong8 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local float8 *dst, const __global float8 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local char16 *dst, const __global char16 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local uchar16 *dst, const __global uchar16 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local short16 *dst, const __global short16 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local ushort16 *dst, const __global ushort16 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local int16 *dst, const __global int16 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local uint16 *dst, const __global uint16 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local long16 *dst, const __global long16 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local ulong16 *dst, const __global ulong16 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local float16 *dst, const __global float16 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global char *dst, const __local char *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global uchar *dst, const __local uchar *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global short *dst, const __local short *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global ushort *dst, const __local ushort *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global int *dst, const __local int *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global uint *dst, const __local uint *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global long *dst, const __local long *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global ulong *dst, const __local ulong *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global float *dst, const __local float *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global char2 *dst, const __local char2 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global uchar2 *dst, const __local uchar2 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global short2 *dst, const __local short2 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global ushort2 *dst, const __local ushort2 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global int2 *dst, const __local int2 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global uint2 *dst, const __local uint2 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global long2 *dst, const __local long2 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global ulong2 *dst, const __local ulong2 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global float2 *dst, const __local float2 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global char3 *dst, const __local char3 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global uchar3 *dst, const __local uchar3 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global short3 *dst, const __local short3 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global ushort3 *dst, const __local ushort3 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global int3 *dst, const __local int3 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global uint3 *dst, const __local uint3 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global long3 *dst, const __local long3 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global ulong3 *dst, const __local ulong3 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global float3 *dst, const __local float3 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global char4 *dst, const __local char4 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global uchar4 *dst, const __local uchar4 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global short4 *dst, const __local short4 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global ushort4 *dst, const __local ushort4 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global int4 *dst, const __local int4 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global uint4 *dst, const __local uint4 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global long4 *dst, const __local long4 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global ulong4 *dst, const __local ulong4 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global float4 *dst, const __local float4 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global char8 *dst, const __local char8 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global uchar8 *dst, const __local uchar8 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global short8 *dst, const __local short8 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global ushort8 *dst, const __local ushort8 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global int8 *dst, const __local int8 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global uint8 *dst, const __local uint8 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global long8 *dst, const __local long8 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global ulong8 *dst, const __local ulong8 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global float8 *dst, const __local float8 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global char16 *dst, const __local char16 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global uchar16 *dst, const __local uchar16 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global short16 *dst, const __local short16 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global ushort16 *dst, const __local ushort16 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global int16 *dst, const __local int16 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global uint16 *dst, const __local uint16 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global long16 *dst, const __local long16 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global ulong16 *dst, const __local ulong16 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global float16 *dst, const __local float16 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local double *dst, const __global double *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local double2 *dst, const __global double2 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local double3 *dst, const __global double3 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local double4 *dst, const __global double4 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local double8 *dst, const __global double8 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__local double16 *dst, const __global double16 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global double *dst, const __local double *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global double2 *dst, const __local double2 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global double3 *dst, const __local double3 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global double4 *dst, const __local double4 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global double8 *dst, const __local double8 *src, size_t num_elements, event_t event);
event_t __attribute__((overloadable)) async_work_group_copy(__global double16 *dst, const __local double16 *src, size_t num_elements, event_t event);
//event_t __attribute__((overloadable)) async_work_group_copy(__local half2 *dst, const __global half2 *src, size_t num_elements, event_t event);
//event_t __attribute__((overloadable)) async_work_group_copy(__local half4 *dst, const __global half4 *src, size_t num_elements, event_t event);
//event_t __attribute__((overloadable)) async_work_group_copy(__local half8 *dst, const __global half8 *src, size_t num_elements, event_t event);
//event_t __attribute__((overloadable)) async_work_group_copy(__local half16 *dst, const __global half16 *src, size_t num_elements, event_t event);
//event_t __attribute__((overloadable)) async_work_group_copy(__global half2 *dst, const __local half2 *src, size_t num_elements, event_t event);
//event_t __attribute__((overloadable)) async_work_group_copy(__global half4 *dst, const __local half4 *src, size_t num_elements, event_t event);
//event_t __attribute__((overloadable)) async_work_group_copy(__global half8 *dst, const __local half8 *src, size_t num_elements, event_t event);
//event_t __attribute__((overloadable)) async_work_group_copy(__global half16 *dst, const __local half16 *src, size_t num_elements, event_t event);

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
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local char *dst, const __global char *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local uchar *dst, const __global uchar *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local short *dst, const __global short *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local ushort *dst, const __global ushort *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local int *dst, const __global int *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local uint *dst, const __global uint *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local long *dst, const __global long *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local ulong *dst, const __global ulong *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local float *dst, const __global float *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local char2 *dst, const __global char2 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local uchar2 *dst, const __global uchar2 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local short2 *dst, const __global short2 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local ushort2 *dst, const __global ushort2 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local int2 *dst, const __global int2 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local uint2 *dst, const __global uint2 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local long2 *dst, const __global long2 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local ulong2 *dst, const __global ulong2 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local float2 *dst, const __global float2 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local char3 *dst, const __global char3 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local uchar3 *dst, const __global uchar3 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local short3 *dst, const __global short3 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local ushort3 *dst, const __global ushort3 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local int3 *dst, const __global int3 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local uint3 *dst, const __global uint3 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local long3 *dst, const __global long3 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local ulong3 *dst, const __global ulong3 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local float3 *dst, const __global float3 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local char4 *dst, const __global char4 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local uchar4 *dst, const __global uchar4 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local short4 *dst, const __global short4 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local ushort4 *dst, const __global ushort4 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local int4 *dst, const __global int4 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local uint4 *dst, const __global uint4 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local long4 *dst, const __global long4 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local ulong4 *dst, const __global ulong4 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local float4 *dst, const __global float4 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local char8 *dst, const __global char8 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local uchar8 *dst, const __global uchar8 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local short8 *dst, const __global short8 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local ushort8 *dst, const __global ushort8 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local int8 *dst, const __global int8 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local uint8 *dst, const __global uint8 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local long8 *dst, const __global long8 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local ulong8 *dst, const __global ulong8 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local float8 *dst, const __global float8 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local char16 *dst, const __global char16 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local uchar16 *dst, const __global uchar16 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local short16 *dst, const __global short16 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local ushort16 *dst, const __global ushort16 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local int16 *dst, const __global int16 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local uint16 *dst, const __global uint16 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local long16 *dst, const __global long16 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local ulong16 *dst, const __global ulong16 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local float16 *dst, const __global float16 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global char *dst, const __local char *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global uchar *dst, const __local uchar *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global short *dst, const __local short *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global ushort *dst, const __local ushort *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global int *dst, const __local int *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global uint *dst, const __local uint *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global long *dst, const __local long *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global ulong *dst, const __local ulong *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global float *dst, const __local float *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global char2 *dst, const __local char2 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global uchar2 *dst, const __local uchar2 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global short2 *dst, const __local short2 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global ushort2 *dst, const __local ushort2 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global int2 *dst, const __local int2 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global uint2 *dst, const __local uint2 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global long2 *dst, const __local long2 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global ulong2 *dst, const __local ulong2 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global float2 *dst, const __local float2 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global char3 *dst, const __local char3 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global uchar3 *dst, const __local uchar3 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global short3 *dst, const __local short3 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global ushort3 *dst, const __local ushort3 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global int3 *dst, const __local int3 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global uint3 *dst, const __local uint3 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global long3 *dst, const __local long3 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global ulong3 *dst, const __local ulong3 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global float3 *dst, const __local float3 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global char4 *dst, const __local char4 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global uchar4 *dst, const __local uchar4 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global short4 *dst, const __local short4 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global ushort4 *dst, const __local ushort4 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global int4 *dst, const __local int4 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global uint4 *dst, const __local uint4 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global long4 *dst, const __local long4 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global ulong4 *dst, const __local ulong4 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global float4 *dst, const __local float4 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global char8 *dst, const __local char8 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global uchar8 *dst, const __local uchar8 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global short8 *dst, const __local short8 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global ushort8 *dst, const __local ushort8 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global int8 *dst, const __local int8 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global uint8 *dst, const __local uint8 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global long8 *dst, const __local long8 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global ulong8 *dst, const __local ulong8 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global float8 *dst, const __local float8 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global char16 *dst, const __local char16 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global uchar16 *dst, const __local uchar16 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global short16 *dst, const __local short16 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global ushort16 *dst, const __local ushort16 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global int16 *dst, const __local int16 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global uint16 *dst, const __local uint16 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global long16 *dst, const __local long16 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global ulong16 *dst, const __local ulong16 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global float16 *dst, const __local float16 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local double *dst, const __global double *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local double2 *dst, const __global double2 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local double3 *dst, const __global double3 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local double4 *dst, const __global double4 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local double8 *dst, const __global double8 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__local double16 *dst, const __global double16 *src, size_t num_elements, size_t src_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global double *dst, const __local double *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global double2 *dst, const __local double2 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global double3 *dst, const __local double3 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global double4 *dst, const __local double4 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global double8 *dst, const __local double8 *src, size_t num_elements, size_t dst_stride, event_t event);
event_t __attribute__((overloadable)) async_work_group_strided_copy(__global double16 *dst, const __local double16 *src, size_t num_elements, size_t dst_stride, event_t event);
//event_t __attribute__((overloadable)) async_work_group_strided_copy(__local half2 *dst, const __global half2 *src, size_t num_elements, size_t src_stride, event_t event);
//event_t __attribute__((overloadable)) async_work_group_strided_copy(__local half4 *dst, const __global half4 *src, size_t num_elements, size_t src_stride, event_t event);
//event_t __attribute__((overloadable)) async_work_group_strided_copy(__local half8 *dst, const __global half8 *src, size_t num_elements, size_t src_stride, event_t event);
//event_t __attribute__((overloadable)) async_work_group_strided_copy(__local half16 *dst, const __global half16 *src, size_t num_elements, size_t src_stride, event_t event);
//event_t __attribute__((overloadable)) async_work_group_strided_copy(__global half2 *dst, const __local half2 *src, size_t num_elements, size_t dst_stride, event_t event);
//event_t __attribute__((overloadable)) async_work_group_strided_copy(__global half4 *dst, const __local half4 *src, size_t num_elements, size_t dst_stride, event_t event);
//event_t __attribute__((overloadable)) async_work_group_strided_copy(__global half8 *dst, const __local half8 *src, size_t num_elements, size_t dst_stride, event_t event);
//event_t __attribute__((overloadable)) async_work_group_strided_copy(__global half16 *dst, const __local half16 *src, size_t num_elements, size_t dst_stride, event_t event);

/**
 * Prefetch num_elements * sizeof(gentype)
 * bytes into the global cache. The prefetch
 * instruction is applied to a work-item in a workgroup
 * and does not affect the functional
 * behavior of the kernel.
 */
void __attribute__((overloadable)) prefetch(const __global char *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global uchar *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global short *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global ushort *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global int *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global uint *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global long *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global ulong *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global float *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global char2 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global uchar2 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global short2 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global ushort2 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global int2 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global uint2 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global long2 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global ulong2 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global float2 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global char3 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global uchar3 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global short3 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global ushort3 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global int3 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global uint3 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global long3 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global ulong3 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global float3 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global char4 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global uchar4 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global short4 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global ushort4 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global int4 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global uint4 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global long4 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global ulong4 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global float4 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global char8 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global uchar8 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global short8 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global ushort8 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global int8 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global uint8 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global long8 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global ulong8 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global float8 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global char16 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global uchar16 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global short16 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global ushort16 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global int16 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global uint16 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global long16 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global ulong16 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global float16 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global double *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global double2 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global double3 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global double4 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global double8 *p, size_t num_elements);
void __attribute__((overloadable)) prefetch(const __global double16 *p, size_t num_elements);
//void __attribute__((overloadable)) prefetch(const __global half2 *p, size_t num_elements);
//void __attribute__((overloadable)) prefetch(const __global half4 *p, size_t num_elements);
//void __attribute__((overloadable)) prefetch(const __global half8 *p, size_t num_elements);
//void __attribute__((overloadable)) prefetch(const __global half16 *p, size_t num_elements);



/**
 * Extract mantissa and exponent from x. For each
 * component the mantissa returned is a float with
 * magnitude in the interval [1/2, 1) or 0. Each
 * component of x equals mantissa returned * 2^exp.
 */


typedef _Atomic(int) atomic_int;
typedef _Atomic(uint) atomic_uint;
typedef _Atomic(long) atomic_long;
typedef _Atomic(ulong) atomic_ulong;
typedef _Atomic(float) atomic_float;
#ifdef cl_khr_fp64
typedef _Atomic(double) atomic_double;
#endif


#if defined __IMAGE2D_DEPTH__ || __OPENCL_C_VERSION__ >= 200
// Enable the extension to make compilation of PCH possible for all supported standartds
#pragma OPENCL EXTENSION cl_khr_depth_images : enable
float __attribute__((overloadable))  read_imagef(__read_only image2d_depth_t image, sampler_t sampler, int2 coord);
float __attribute__((overloadable))  read_imagef(__read_only image2d_depth_t image, sampler_t sampler, float2 coord);
float __attribute__((overloadable))  read_imagef(__read_only image2d_depth_t image, int2 coord);
float __attribute__((overloadable))  read_imagef(__read_only image2d_array_depth_t image, sampler_t sampler, int4 coord);
float __attribute__((overloadable))  read_imagef(__read_only image2d_array_depth_t image, sampler_t sampler, float4 coord);
float __attribute__((overloadable))  read_imagef(__read_only image2d_array_depth_t image, int4 coord);

void __attribute__((overloadable)) write_imagef(__write_only image2d_depth_t image, int2 coord, float depth);
void __attribute__((overloadable)) write_imagef (__write_only image2d_array_depth_t image, int4 coord, float depth);

int  __attribute__((const)) __attribute__((overloadable)) get_image_width(image2d_depth_t image);
int  __attribute__((const)) __attribute__((overloadable)) get_image_width(image2d_array_depth_t image);

int  __attribute__((const)) __attribute__((overloadable)) get_image_height(image2d_depth_t image);
int  __attribute__((const)) __attribute__((overloadable)) get_image_height(image2d_array_depth_t image);

int  __attribute__((const)) __attribute__((overloadable)) get_image_channel_data_type(image2d_depth_t image);
int  __attribute__((const)) __attribute__((overloadable)) get_image_channel_data_type(image2d_array_depth_t image);

int  __attribute__((const)) __attribute__((overloadable)) get_image_channel_order(image2d_depth_t image);
int  __attribute__((const)) __attribute__((overloadable)) get_image_channel_order(image2d_array_depth_t image);

int2  __attribute__((const)) __attribute__((overloadable)) get_image_dim(image2d_depth_t image);
int2  __attribute__((const)) __attribute__((overloadable)) get_image_dim(image2d_array_depth_t image);

size_t __attribute__((const))  __attribute__((overloadable)) get_image_array_size(image2d_array_depth_t image_array);

#if __OPENCL_C_VERSION__ < 200
// Disable the extension for standards below 2.0
#pragma OPENCL EXTENSION cl_khr_depth_images : disable
#endif    // __OPENCL_C_VERSION__ < 200
#endif    //__IMAGE2D_DEPTH__ || __OPENCL_C_VERSION__ >= 200


// Since OpenCL 2.0 writes to image3d_t are mandatory
#if defined __WRITE_IMAGE3D__ || __OPENCL_C_VERSION__ >= 200
// Enable the extension to make compilation of PCH possible for all supported standartds
#pragma OPENCL EXTENSION cl_khr_3d_image_writes : enable
/**
 * Write color value to location specified by coordinate
 * (x, y, z) in the 3D image object specified by image.
 * Appropriate data format conversion to the specified
 * image format is done before writing the color value.
 * x & y are considered to be unnormalized coordinates
 * and must be in the range 0 ... image width - 1, and 0
 * ... image height - 1.
 * write_imagef can only be used with image objects
 * created with image_channel_data_type set to one of
 * the pre-defined packed formats or set to
 * CL_SNORM_INT8, CL_UNORM_INT8,
 * CL_SNORM_INT16, CL_UNORM_INT16,
 * CL_HALF_FLOAT or CL_FLOAT. Appropriate data
 * format conversion will be done to convert channel
 * data from a floating-point value to actual data format
 * in which the channels are stored.
 * write_imagei can only be used with image objects
 * created with image_channel_data_type set to one of
 * the following values:
 * CL_SIGNED_INT8,
 * CL_SIGNED_INT16 and
 * CL_SIGNED_INT32.
 * write_imageui can only be used with image objects
 * created with image_channel_data_type set to one of
 * the following values:
 * CL_UNSIGNED_INT8,
 * CL_UNSIGNED_INT16 and
 * CL_UNSIGNED_INT32.
 * The behavior of write_imagef, write_imagei and
 * write_imageui for image objects created with
 * image_channel_data_type values not specified in
 * the description above or with (x, y) coordinate
 * values that are not in the range (0 ... image width -
 * 1, 0 ... image height - 1), respectively, is undefined.
 */
void __attribute__((overloadable)) write_imagef(__write_only image3d_t image, int4 coord, float4 color);
void __attribute__((overloadable)) write_imagei(__write_only image3d_t image, int4 coord, int4 color);
void __attribute__((overloadable)) write_imageui(__write_only image3d_t image, int4 coord, uint4 color);
//void __attribute__((overloadable)) write_imageh(__write_only image3d_t image, int4 coord, half4 color);
#if __OPENCL_C_VERSION__ < 200
//  disable the extension for standards below 2.0
#pragma OPENCL EXTENSION cl_khr_3d_image_writes : disable
#endif      // __OPENCL_C_VERSION__ < 200
#endif

/**
 * Return the number of images in the 2D image array.
 */
size_t __attribute__((const)) __attribute__((overloadable)) get_image_array_size(image2d_array_t image_array);

/**
 * Return the number of images in the 1D image array.
 */
size_t __attribute__((const)) __attribute__((overloadable)) get_image_array_size(image1d_array_t image_array);

#if !defined (__MIC__) && !defined(__MIC2__)
//*****************************************************
//        OpenCl2.0 functions
//*****************************************************
/**
 * ctz-Returns the number of trailing 0-bits in x.
 */

char __attribute__((const)) __attribute__((overloadable)) ctz(char x);
uchar  __attribute__((const)) __attribute__((overloadable)) ctz(uchar x);
char2  __attribute__((const)) __attribute__((overloadable)) ctz(char2 x);
uchar2  __attribute__((const)) __attribute__((overloadable)) ctz(uchar2 x);
char3  __attribute__((const)) __attribute__((overloadable)) ctz(char3 x);
uchar3  __attribute__((const)) __attribute__((overloadable)) ctz(uchar3 x);
char4  __attribute__((const)) __attribute__((overloadable)) ctz(char4 x);
uchar4  __attribute__((const)) __attribute__((overloadable)) ctz(uchar4 x);
char8  __attribute__((const)) __attribute__((overloadable)) ctz(char8 x);
uchar8  __attribute__((const)) __attribute__((overloadable)) ctz(uchar8 x);
char16  __attribute__((const)) __attribute__((overloadable)) ctz(char16 x);
uchar16  __attribute__((const)) __attribute__((overloadable)) ctz(uchar16 x);
short  __attribute__((const)) __attribute__((overloadable)) ctz(short x);
ushort  __attribute__((const)) __attribute__((overloadable)) ctz(ushort x);
short2  __attribute__((const)) __attribute__((overloadable)) ctz(short2 x);
ushort2  __attribute__((const)) __attribute__((overloadable)) ctz(ushort2 x);
short3  __attribute__((const)) __attribute__((overloadable)) ctz(short3 x);
ushort3  __attribute__((const)) __attribute__((overloadable)) ctz(ushort3 x);
short4  __attribute__((const)) __attribute__((overloadable)) ctz(short4 x);
ushort4  __attribute__((const)) __attribute__((overloadable)) ctz(ushort4 x);
short8  __attribute__((const)) __attribute__((overloadable)) ctz(short8 x);
ushort8  __attribute__((const)) __attribute__((overloadable)) ctz(ushort8 x);
short16  __attribute__((const)) __attribute__((overloadable)) ctz(short16 x);
ushort16  __attribute__((const)) __attribute__((overloadable)) ctz(ushort16 x);
int  __attribute__((const)) __attribute__((overloadable)) ctz(int x);
uint  __attribute__((const)) __attribute__((overloadable)) ctz(uint x);
int2  __attribute__((const)) __attribute__((overloadable)) ctz(int2 x);
uint2  __attribute__((const)) __attribute__((overloadable)) ctz(uint2 x);
int3  __attribute__((const)) __attribute__((overloadable)) ctz(int3 x);
uint3  __attribute__((const)) __attribute__((overloadable)) ctz(uint3 x);
int4  __attribute__((const)) __attribute__((overloadable)) ctz(int4 x);
uint4  __attribute__((const)) __attribute__((overloadable)) ctz(uint4 x);
int8  __attribute__((const)) __attribute__((overloadable)) ctz(int8 x);
uint8  __attribute__((const)) __attribute__((overloadable)) ctz(uint8 x);
int16  __attribute__((const)) __attribute__((overloadable)) ctz(int16 x);
uint16  __attribute__((const)) __attribute__((overloadable)) ctz(uint16 x);
long  __attribute__((const)) __attribute__((overloadable)) ctz(long x);
ulong  __attribute__((const)) __attribute__((overloadable)) ctz(ulong x);
long2  __attribute__((const)) __attribute__((overloadable)) ctz(long2 x);
ulong2  __attribute__((const)) __attribute__((overloadable)) ctz(ulong2 x);
long3  __attribute__((const)) __attribute__((overloadable)) ctz(long3 x);
ulong3  __attribute__((const)) __attribute__((overloadable)) ctz(ulong3 x);
long4  __attribute__((const)) __attribute__((overloadable)) ctz(long4 x);
ulong4  __attribute__((const)) __attribute__((overloadable)) ctz(ulong4 x);
long8  __attribute__((const)) __attribute__((overloadable)) ctz(long8 x);
ulong8  __attribute__((const)) __attribute__((overloadable)) ctz(ulong8 x);
long16  __attribute__((const)) __attribute__((overloadable)) ctz(long16 x);
ulong16  __attribute__((const)) __attribute__((overloadable)) ctz(ulong16 x);

/**
 * Returns the same value as that returned by
 * get_local_size(dimindx) if the kernel is executed with a
 * uniform work-group size.
 * If the kernel is executed with a non-uniform work-group
 * size, returns the number of local work-items in each of
 * the work-groups that make up the uniform region of the
 * global range in the dimension identified by dimindx. If
 * the local_work_size argument to
 * clEnqueueNDRangeKernel is not NULL, this value
 * will match the value specified in
 * local_work_size[dimindx]. If local_work_size is NULL,
 * this value will match the local size that the
 * implementation determined would be most efficient at
 * implementing the uniform region of the global range.
 * Valid values of dimindx are 0 to get_work_dim()  1.
 * For other values of dimindx, get_enqueud_local_size()
 * returns 1.
 * For clEnqueueTask, this always returns 1.
 */
size_t  __attribute__((const)) __attribute__((overloadable)) get_enqueued_local_size(uint dimindx);

/**
 * Returns the work-items 1-dimensional global ID. For
 * 1D work-groups, it is the same value as
 * get_global_id(0).
 */
size_t __attribute__((const)) __attribute__((overloadable)) get_global_linear_id(void);

/**
 * Returns the work-items 1-dimensional local ID. For 1D
 * work-groups, it is the same value as get_local_id(0).
 */
size_t __attribute__((const)) __attribute__((overloadable)) get_local_linear_id(void);


// Work group funtions 6.13.15

/**
 * Evaluates predicate for all work-items in the
 * work-group and returns a non-zero value if
 * predicate evaluates to non-zero for all workitems
 * in the work-group.
 */
int __attribute__((overloadable)) work_group_all(int predicate);

/**
 * Evaluates predicate for all work-items in the
 * work-group and returns a non-zero value if
 * predicate evaluates to non-zero for any workitems
 * in the work-group.
 */
int __attribute__((overloadable)) work_group_any(int predicate);

/**
 * Broadcast function
 * one dimentional local_id
 */
int    __attribute__((overloadable)) work_group_broadcast(int a,    size_t local_id);
uint   __attribute__((overloadable)) work_group_broadcast(uint a,   size_t local_id);
long   __attribute__((overloadable)) work_group_broadcast(long a,   size_t local_id);
ulong  __attribute__((overloadable)) work_group_broadcast(ulong a,  size_t local_id);
float  __attribute__((overloadable)) work_group_broadcast(float a,  size_t local_id);
double __attribute__((overloadable)) work_group_broadcast(double a, size_t local_id);
//half __attribute__((overloadable)) work_group_broadcast(half a,   size_t local_id);

/**
 * Broadcast function
 * two dimentional local_id
 */
int    __attribute__((overloadable)) work_group_broadcast(int a,    size_t local_id_x, size_t local_id_y);
uint   __attribute__((overloadable)) work_group_broadcast(uint a,   size_t local_id_x, size_t local_id_y);
long   __attribute__((overloadable)) work_group_broadcast(long a,   size_t local_id_x, size_t local_id_y);
ulong  __attribute__((overloadable)) work_group_broadcast(ulong a,  size_t local_id_x, size_t local_id_y);
float  __attribute__((overloadable)) work_group_broadcast(float a,  size_t local_id_x, size_t local_id_y);
double __attribute__((overloadable)) work_group_broadcast(double a, size_t local_id_x, size_t local_id_y);
//half __attribute__((overloadable)) work_group_broadcast(half a,   size_t local_id_x, size_t local_id_y);

/**
 * Broadcast function
 * three dimentional local_id
 */
int    __attribute__((overloadable)) work_group_broadcast(int a,    size_t local_id_x, size_t local_id_y, size_t local_id_z);
uint   __attribute__((overloadable)) work_group_broadcast(uint a,   size_t local_id_x, size_t local_id_y, size_t local_id_z);
long   __attribute__((overloadable)) work_group_broadcast(long a,   size_t local_id_x, size_t local_id_y, size_t local_id_z);
ulong  __attribute__((overloadable)) work_group_broadcast(ulong a,  size_t local_id_x, size_t local_id_y, size_t local_id_z);
float  __attribute__((overloadable)) work_group_broadcast(float a,  size_t local_id_x, size_t local_id_y, size_t local_id_z);
double __attribute__((overloadable)) work_group_broadcast(double a, size_t local_id_x, size_t local_id_y, size_t local_id_z);
//half __attribute__((overloadable)) work_group_broadcast(half a,   size_t local_id_x, size_t local_id_y, size_t local_id_z);

/**
 * reduce functions
 * work_group_reduce_add
 */
int    __attribute__((overloadable)) work_group_reduce_add(int x);
uint   __attribute__((overloadable)) work_group_reduce_add(uint x);
long   __attribute__((overloadable)) work_group_reduce_add(long x);
ulong  __attribute__((overloadable)) work_group_reduce_add(ulong x);
float  __attribute__((overloadable)) work_group_reduce_add(float x);
double __attribute__((overloadable)) work_group_reduce_add(double x);
//half __attribute__((overloadable)) work_group_reduce_add(half x);

/**
 * reduce functions
 * work_group_reduce_min
 */
int    __attribute__((overloadable)) work_group_reduce_min(int x);
uint   __attribute__((overloadable)) work_group_reduce_min(uint x);
long   __attribute__((overloadable)) work_group_reduce_min(long x);
ulong  __attribute__((overloadable)) work_group_reduce_min(ulong x);
float  __attribute__((overloadable)) work_group_reduce_min(float x);
double __attribute__((overloadable)) work_group_reduce_min(double x);
//half __attribute__((overloadable)) work_group_reduce_min(half x);

/**
 * reduce functions
 * work_group_reduce_max
 */
int    __attribute__((overloadable)) work_group_reduce_max(int x);
uint   __attribute__((overloadable)) work_group_reduce_max(uint x);
long   __attribute__((overloadable)) work_group_reduce_max(long x);
ulong  __attribute__((overloadable)) work_group_reduce_max(ulong x);
float  __attribute__((overloadable)) work_group_reduce_max(float x);
double __attribute__((overloadable)) work_group_reduce_max(double x);
//half __attribute__((overloadable)) work_group_reduce_max(half x);

/**
 * scan functions
 * work_group_scan_inclusive_max
 */
int    __attribute__((overloadable)) work_group_scan_inclusive_max(int x);
uint   __attribute__((overloadable)) work_group_scan_inclusive_max(uint x);
long   __attribute__((overloadable)) work_group_scan_inclusive_max(long x);
ulong  __attribute__((overloadable)) work_group_scan_inclusive_max(ulong x);
float  __attribute__((overloadable)) work_group_scan_inclusive_max(float x);
double __attribute__((overloadable)) work_group_scan_inclusive_max(double x);
//half __attribute__((overloadable)) work_group_scan_inclusive_max(half x);

/**
 * scan functions
 * work_group_scan_inclusive_min
 */
int    __attribute__((overloadable)) work_group_scan_inclusive_min(int x);
uint   __attribute__((overloadable)) work_group_scan_inclusive_min(uint x);
long   __attribute__((overloadable)) work_group_scan_inclusive_min(long x);
ulong  __attribute__((overloadable)) work_group_scan_inclusive_min(ulong x);
float  __attribute__((overloadable)) work_group_scan_inclusive_min(float x);
double __attribute__((overloadable)) work_group_scan_inclusive_min(double x);
//half __attribute__((overloadable)) work_group_scan_inclusive_min(half x);

/**
 * scan functions
 * work_group_scan_inclusive_add
 */
int    __attribute__((overloadable)) work_group_scan_inclusive_add(int x);
uint   __attribute__((overloadable)) work_group_scan_inclusive_add(uint x);
long   __attribute__((overloadable)) work_group_scan_inclusive_add(long x);
ulong  __attribute__((overloadable)) work_group_scan_inclusive_add(ulong x);
float  __attribute__((overloadable)) work_group_scan_inclusive_add(float x);
double __attribute__((overloadable)) work_group_scan_inclusive_add(double x);
//half __attribute__((overloadable)) work_group_scan_inclusive_add(half x);

/**
 * scan functions
 * work_group_scan_exclusive_max
 */
int    __attribute__((overloadable)) work_group_scan_exclusive_max(int x);
uint   __attribute__((overloadable)) work_group_scan_exclusive_max(uint x);
long   __attribute__((overloadable)) work_group_scan_exclusive_max(long x);
ulong  __attribute__((overloadable)) work_group_scan_exclusive_max(ulong x);
float  __attribute__((overloadable)) work_group_scan_exclusive_max(float x);
double __attribute__((overloadable)) work_group_scan_exclusive_max(double x);
//half __attribute__((overloadable)) work_group_scan_exclusive_max(half x);

/**
 * scan functions
 * work_group_scan_exclusive_min
 */
int    __attribute__((overloadable)) work_group_scan_exclusive_min(int x);
uint   __attribute__((overloadable)) work_group_scan_exclusive_min(uint x);
long   __attribute__((overloadable)) work_group_scan_exclusive_min(long x);
ulong  __attribute__((overloadable)) work_group_scan_exclusive_min(ulong x);
float  __attribute__((overloadable)) work_group_scan_exclusive_min(float x);
double __attribute__((overloadable)) work_group_scan_exclusive_min(double x);
//half __attribute__((overloadable)) work_group_scan_exclusive_min(half x);

/**
 * scan functions
 * work_group_scan_exclusive_add
 */
int    __attribute__((overloadable)) work_group_scan_exclusive_add(int x);
uint   __attribute__((overloadable)) work_group_scan_exclusive_add(uint x);
long   __attribute__((overloadable)) work_group_scan_exclusive_add(long x);
ulong  __attribute__((overloadable)) work_group_scan_exclusive_add(ulong x);
float  __attribute__((overloadable)) work_group_scan_exclusive_add(float x);
double __attribute__((overloadable)) work_group_scan_exclusive_add(double x);
//half __attribute__((overloadable)) work_group_scan_exclusive_add(half x);
#if __OPENCL_C_VERSION__ >= 200


// Address Space Qualifier Functions 6.13.9
global void*        __attribute__((const)) __attribute__((overloadable)) to_global  (const void *ptr);
local void*         __attribute__((const)) __attribute__((overloadable)) to_local   (const void *ptr);
private void*       __attribute__((const)) __attribute__((overloadable)) to_private (const void *ptr);
cl_mem_fence_flags  __attribute__((const)) __attribute__((overloadable)) get_fence (const void *ptr);


typedef int kernel_enqueue_flags_t;
typedef int clk_profiling_info;


//ATTENTION! Size of arrays below should be = MAX_WORK_DIM
typedef struct {
    unsigned int workDimension;
    size_t globalWorkOffset[3];
    size_t globalWorkSize[3];
    size_t localWorkSize[3];
} ndrange_t;

// The functions with the always_inline attribute must be inlined to allow the PatchCallbackArgs pass to patch the callbacks called within these builtins
int __attribute__((overloadable)) __attribute__((always_inline)) enqueue_kernel( queue_t queue, kernel_enqueue_flags_t flags, const ndrange_t ndrange, void (^block)(void));
int __attribute__((overloadable)) __attribute__((always_inline)) enqueue_kernel( queue_t queue, kernel_enqueue_flags_t flags, const ndrange_t ndrange, uint num_events_in_wait_list, const clk_event_t *event_wait_list, clk_event_t *event_ret, void (^block)(void));
int __attribute__((overloadable)) __attribute__((always_inline)) enqueue_kernel( queue_t queue, kernel_enqueue_flags_t flags, const ndrange_t ndrange, void (^block)(local void *, ...), uint size0,...);
int __attribute__((overloadable)) __attribute__((always_inline)) enqueue_kernel( queue_t queue, kernel_enqueue_flags_t flags, const ndrange_t ndrange, uint num_events_in_wait_list, const clk_event_t *event_wait_list, clk_event_t *event_ret, void (^block)(local void *, ...), uint size0, ...);

int __attribute__((overloadable)) __attribute__((always_inline)) enqueue_marker(queue_t queue, uint num_events_in_wait_list, const clk_event_t *event_wait_list, clk_event_t *event_ret);

queue_t __attribute__((const)) __attribute__((always_inline)) get_default_queue(void);

ndrange_t  __attribute__((const)) __attribute__((overloadable)) ndrange_1D( size_t global_work_size);
ndrange_t  __attribute__((const)) __attribute__((overloadable)) ndrange_1D( size_t global_work_size, size_t local_work_size);
ndrange_t  __attribute__((const)) __attribute__((overloadable)) ndrange_1D( size_t global_work_offset, size_t global_work_size, size_t local_work_size);

ndrange_t  __attribute__((const)) __attribute__((overloadable)) ndrange_2D( const size_t global_work_size[2]);
ndrange_t  __attribute__((const)) __attribute__((overloadable)) ndrange_2D( const size_t global_work_size[2], const size_t local_work_size[2]);
ndrange_t  __attribute__((const)) __attribute__((overloadable)) ndrange_2D( const size_t global_work_offset[2], const size_t global_work_size[2], const size_t local_work_size[2]);

ndrange_t  __attribute__((const)) __attribute__((overloadable)) ndrange_3D( const size_t global_work_size[3]);
ndrange_t  __attribute__((const)) __attribute__((overloadable)) ndrange_3D( const size_t global_work_size[3], const size_t local_work_size[3]);
ndrange_t  __attribute__((const)) __attribute__((overloadable)) ndrange_3D( const size_t global_work_offset[3], const size_t global_work_size[3], const size_t local_work_size[3]);

void __attribute__((overloadable)) __attribute__((always_inline)) retain_event(clk_event_t event);
void __attribute__((overloadable)) __attribute__((always_inline)) release_event(clk_event_t event);
clk_event_t  __attribute__((always_inline)) create_user_event();
bool __attribute__((overloadable)) __attribute__((always_inline)) is_valid_event(clk_event_t event);
void __attribute__((overloadable)) __attribute__((always_inline)) set_user_event_status(clk_event_t event, int status);
void __attribute__((overloadable)) __attribute__((always_inline)) capture_event_profiling_info(clk_event_t event, clk_profiling_info name, global ulong *value);

uint __attribute__((overloadable)) __attribute__((always_inline)) __attribute__((pure)) get_kernel_work_group_size(void (^block)(void));
uint __attribute__((overloadable)) __attribute__((always_inline)) __attribute__((pure)) get_kernel_work_group_size(void (^block)(local void *,...));
uint __attribute__((overloadable)) __attribute__((always_inline)) __attribute__((pure)) get_kernel_preferred_work_group_size_multiple(void (^block)(void));
uint __attribute__((overloadable)) __attribute__((always_inline)) __attribute__((pure)) get_kernel_preferred_work_group_size_multiple(void (^block)(local void *,...));

// Most of OCL 2.0 pipe built-ins are treated as Clang built-ins.
// So that Clang appends an implicit argument of i32 type to a call.
// This constant determines the pipe packet size.
// The following OCL built-in isn't treated by Clang in this way.
// It is a common functions.
bool __attribute__((overloadable)) is_valid_reserve_id(reserve_id_t reserve_id);


#endif   // __OPENCL_C_VERSION__ >= 200
#endif   // !defined (__MIC__) && !defined(__MIC2__)
#if __OPENCL_C_VERSION__ >= 200

#if !defined (__MIC__) && !defined(__MIC2__)

typedef atomic_int atomic_flag;

/**
 * OpenCL 2.0:
 * Map atomic_intptr_t, atomic_ptrdiff_t, atomic_uintptr_t, and atomic_size_t
 * to corresponding integer atomic types
 * if CL_DEVICE_ADDRESS_BITS defined in table 4.3 is 32-bits.
 */
#if defined(__i386__) || defined(i386) || defined(_M_IX86)
  typedef  atomic_int   atomic_intptr_t;
  typedef  atomic_int   atomic_ptrdiff_t;
  typedef  atomic_uint  atomic_uintptr_t;
  typedef  atomic_uint  atomic_size_t;
#endif


typedef uint memory_order;


/**
 * OpenCL C 2.0 atomic builtins 6.13.11
 */
void __attribute__((overloadable)) atomic_work_item_fence(cl_mem_fence_flags flags, memory_order order, memory_scope scope);

void __attribute__((overloadable)) atomic_init(volatile atomic_int *object, int value);
void __attribute__((overloadable)) atomic_init(volatile atomic_uint *object, uint value);
void __attribute__((overloadable)) atomic_init(volatile atomic_float *object, float value);

// __generic
void __attribute__((overloadable)) atomic_store(volatile atomic_int *object, int desired);
void __attribute__((overloadable)) atomic_store(volatile atomic_uint *object, uint desired);
void __attribute__((overloadable)) atomic_store(volatile atomic_float *object, float desired);
void __attribute__((overloadable)) atomic_store_explicit(volatile atomic_int *object, int desired, memory_order order);
void __attribute__((overloadable)) atomic_store_explicit(volatile atomic_uint *object, uint desired, memory_order order);
void __attribute__((overloadable)) atomic_store_explicit(volatile atomic_float *object, float desired, memory_order order);
void __attribute__((overloadable)) atomic_store_explicit(volatile atomic_int *object, int desired, memory_order order, memory_scope scope);
void __attribute__((overloadable)) atomic_store_explicit(volatile atomic_uint *object, uint desired, memory_order order, memory_scope scope);
void __attribute__((overloadable)) atomic_store_explicit(volatile atomic_float *object, float desired, memory_order order, memory_scope scope);

int __attribute__((overloadable)) atomic_load(volatile atomic_int *object);
uint __attribute__((overloadable)) atomic_load(volatile atomic_uint *object);
float __attribute__((overloadable)) atomic_load(volatile atomic_float *object);
int __attribute__((overloadable)) atomic_load_explicit(volatile atomic_int *object, memory_order order);
uint __attribute__((overloadable)) atomic_load_explicit(volatile atomic_uint *object, memory_order order);
float __attribute__((overloadable)) atomic_load_explicit(volatile atomic_float *object, memory_order order);
int __attribute__((overloadable)) atomic_load_explicit(volatile atomic_int *object, memory_order order, memory_scope scope);
uint __attribute__((overloadable)) atomic_load_explicit(volatile atomic_uint *object, memory_order order, memory_scope scope);
float __attribute__((overloadable)) atomic_load_explicit(volatile atomic_float *object, memory_order order, memory_scope scope);

int __attribute__((overloadable)) atomic_exchange(volatile atomic_int *object, int desired);
uint __attribute__((overloadable)) atomic_exchange(volatile atomic_uint *object, uint desired);
float __attribute__((overloadable)) atomic_exchange(volatile atomic_float *object, float desired);
int __attribute__((overloadable)) atomic_exchange_explicit(volatile atomic_int *object, int desired, memory_order order);
uint __attribute__((overloadable)) atomic_exchange_explicit(volatile atomic_uint *object, uint desired, memory_order order);
float __attribute__((overloadable)) atomic_exchange_explicit(volatile atomic_float *object, float desired, memory_order order);
int __attribute__((overloadable)) atomic_exchange_explicit(volatile atomic_int *object, int desired, memory_order order, memory_scope scope);
uint __attribute__((overloadable)) atomic_exchange_explicit(volatile atomic_uint *object, uint desired, memory_order order, memory_scope scope);
float __attribute__((overloadable)) atomic_exchange_explicit(volatile atomic_float *object, float desired, memory_order order, memory_scope scope);

bool __attribute__((overloadable)) atomic_compare_exchange_strong(volatile atomic_int *object, int *expected, int desired);
bool __attribute__((overloadable)) atomic_compare_exchange_strong(volatile atomic_uint *object, uint *expected, uint desired);
bool __attribute__((overloadable)) atomic_compare_exchange_strong(volatile atomic_float *object, float *expected, float desired);
bool __attribute__((overloadable)) atomic_compare_exchange_strong_explicit(volatile atomic_int *object, int *expected, int desired,
                                                                           memory_order success, memory_order failure);
bool __attribute__((overloadable)) atomic_compare_exchange_strong_explicit(volatile atomic_uint *object, uint *expected, uint desired,
                                                                           memory_order success, memory_order failure);
bool __attribute__((overloadable)) atomic_compare_exchange_strong_explicit(volatile atomic_float *object, float *expected, float desired,
                                                                           memory_order success, memory_order failure);
bool __attribute__((overloadable)) atomic_compare_exchange_strong_explicit(volatile atomic_int *object, int *expected, int desired,
                                                                           memory_order success, memory_order failure, memory_scope scope);
bool __attribute__((overloadable)) atomic_compare_exchange_strong_explicit(volatile atomic_uint *object, uint *expected, uint desired,
                                                                           memory_order success, memory_order failure, memory_scope scope);
bool __attribute__((overloadable)) atomic_compare_exchange_strong_explicit(volatile atomic_float *object, float *expected, float desired,
                                                                           memory_order success, memory_order failure, memory_scope scope);
bool __attribute__((overloadable)) atomic_compare_exchange_weak(volatile atomic_int *object, int *expected, int desired);
bool __attribute__((overloadable)) atomic_compare_exchange_weak(volatile atomic_uint *object, uint *expected, uint desired);
bool __attribute__((overloadable)) atomic_compare_exchange_weak(volatile atomic_float *object, float *expected, float desired);
bool __attribute__((overloadable)) atomic_compare_exchange_weak_explicit(volatile atomic_int *object, int *expected, int desired,
                                                                           memory_order success, memory_order failure);
bool __attribute__((overloadable)) atomic_compare_exchange_weak_explicit(volatile atomic_uint *object, uint *expected, uint desired,
                                                                           memory_order success, memory_order failure);
bool __attribute__((overloadable)) atomic_compare_exchange_weak_explicit(volatile atomic_float *object, float *expected, float desired,
                                                                           memory_order success, memory_order failure);
bool __attribute__((overloadable)) atomic_compare_exchange_weak_explicit(volatile atomic_int *object, int *expected, int desired,
                                                                           memory_order success, memory_order failure, memory_scope scope);
bool __attribute__((overloadable)) atomic_compare_exchange_weak_explicit(volatile atomic_uint *object, uint *expected, uint desired,
                                                                           memory_order success, memory_order failure, memory_scope scope);
bool __attribute__((overloadable)) atomic_compare_exchange_weak_explicit(volatile atomic_float *object, float *expected, float desired,
                                                                           memory_order success, memory_order failure, memory_scope scope);

int __attribute__((overloadable)) atomic_fetch_add(volatile atomic_int *object, int operand);
uint __attribute__((overloadable)) atomic_fetch_add(volatile atomic_uint *object, uint operand);
float __attribute__((overloadable)) atomic_fetch_add(volatile atomic_float *object, float operand);
int __attribute__((overloadable)) atomic_fetch_add_explicit(volatile atomic_int *object, int operand, memory_order order);
uint __attribute__((overloadable)) atomic_fetch_add_explicit(volatile atomic_uint *object, uint operand, memory_order order);
float __attribute__((overloadable)) atomic_fetch_add_explicit(volatile atomic_float *object, float operand, memory_order order);
int __attribute__((overloadable)) atomic_fetch_add_explicit(volatile atomic_int *object, int operand, memory_order order, memory_scope scope);
uint __attribute__((overloadable)) atomic_fetch_add_explicit(volatile atomic_uint *object, uint operand, memory_order order, memory_scope scope);
float __attribute__((overloadable)) atomic_fetch_add_explicit(volatile atomic_float *object, float operand, memory_order order, memory_scope scope);

int __attribute__((overloadable)) atomic_fetch_sub(volatile atomic_int *object, int operand);
uint __attribute__((overloadable)) atomic_fetch_sub(volatile atomic_uint *object, uint operand);
float __attribute__((overloadable)) atomic_fetch_sub(volatile atomic_float *object, float operand);
int __attribute__((overloadable)) atomic_fetch_sub_explicit(volatile atomic_int *object, int operand, memory_order order);
uint __attribute__((overloadable)) atomic_fetch_sub_explicit(volatile atomic_uint *object, uint operand, memory_order order);
float __attribute__((overloadable)) atomic_fetch_sub_explicit(volatile atomic_float *object, float operand, memory_order order);
int __attribute__((overloadable)) atomic_fetch_sub_explicit(volatile atomic_int *object, int operand, memory_order order, memory_scope scope);
uint __attribute__((overloadable)) atomic_fetch_sub_explicit(volatile atomic_uint *object, uint operand, memory_order order, memory_scope scope);
float __attribute__((overloadable)) atomic_fetch_sub_explicit(volatile atomic_float *object, float operand, memory_order order, memory_scope scope);

int __attribute__((overloadable)) atomic_fetch_or(volatile atomic_int *object, int operand);
uint __attribute__((overloadable)) atomic_fetch_or(volatile atomic_uint *object, uint operand);
float __attribute__((overloadable)) atomic_fetch_or(volatile atomic_float *object, float operand);
int __attribute__((overloadable)) atomic_fetch_or_explicit(volatile atomic_int *object, int operand, memory_order order);
uint __attribute__((overloadable)) atomic_fetch_or_explicit(volatile atomic_uint *object, uint operand, memory_order order);
float __attribute__((overloadable)) atomic_fetch_or_explicit(volatile atomic_float *object, float operand, memory_order order);
int __attribute__((overloadable)) atomic_fetch_or_explicit(volatile atomic_int *object, int operand, memory_order order, memory_scope scope);
uint __attribute__((overloadable)) atomic_fetch_or_explicit(volatile atomic_uint *object, uint operand, memory_order order, memory_scope scope);
float __attribute__((overloadable)) atomic_fetch_or_explicit(volatile atomic_float *object, float operand, memory_order order, memory_scope scope);

int __attribute__((overloadable)) atomic_fetch_xor(volatile atomic_int *object, int operand);
uint __attribute__((overloadable)) atomic_fetch_xor(volatile atomic_uint *object, uint operand);
float __attribute__((overloadable)) atomic_fetch_xor(volatile atomic_float *object, float operand);
int __attribute__((overloadable)) atomic_fetch_xor_explicit(volatile atomic_int *object, int operand, memory_order order);
uint __attribute__((overloadable)) atomic_fetch_xor_explicit(volatile atomic_uint *object, uint operand, memory_order order);
float __attribute__((overloadable)) atomic_fetch_xor_explicit(volatile atomic_float *object, float operand, memory_order order);
int __attribute__((overloadable)) atomic_fetch_xor_explicit(volatile atomic_int *object, int operand, memory_order order, memory_scope scope);
uint __attribute__((overloadable)) atomic_fetch_xor_explicit(volatile atomic_uint *object, uint operand, memory_order order, memory_scope scope);
float __attribute__((overloadable)) atomic_fetch_xor_explicit(volatile atomic_float *object, float operand, memory_order order, memory_scope scope);

int __attribute__((overloadable)) atomic_fetch_and(volatile atomic_int *object, int operand);
uint __attribute__((overloadable)) atomic_fetch_and(volatile atomic_uint *object, uint operand);
float __attribute__((overloadable)) atomic_fetch_and(volatile atomic_float *object, float operand);
int __attribute__((overloadable)) atomic_fetch_and_explicit(volatile atomic_int *object, int operand, memory_order order);
uint __attribute__((overloadable)) atomic_fetch_and_explicit(volatile atomic_uint *object, uint operand, memory_order order);
float __attribute__((overloadable)) atomic_fetch_and_explicit(volatile atomic_float *object, float operand, memory_order order);
int __attribute__((overloadable)) atomic_fetch_and_explicit(volatile atomic_int *object, int operand, memory_order order, memory_scope scope);
uint __attribute__((overloadable)) atomic_fetch_and_explicit(volatile atomic_uint *object, uint operand, memory_order order, memory_scope scope);
float __attribute__((overloadable)) atomic_fetch_and_explicit(volatile atomic_float *object, float operand, memory_order order, memory_scope scope);

int __attribute__((overloadable)) atomic_fetch_min(volatile atomic_int *object, int operand);
uint __attribute__((overloadable)) atomic_fetch_min(volatile atomic_uint *object, uint operand);
float __attribute__((overloadable)) atomic_fetch_min(volatile atomic_float *object, float operand);
int __attribute__((overloadable)) atomic_fetch_min_explicit(volatile atomic_int *object, int operand, memory_order order);
uint __attribute__((overloadable)) atomic_fetch_min_explicit(volatile atomic_uint *object, uint operand, memory_order order);
float __attribute__((overloadable)) atomic_fetch_min_explicit(volatile atomic_float *object, float operand, memory_order order);
int __attribute__((overloadable)) atomic_fetch_min_explicit(volatile atomic_int *object, int operand, memory_order order, memory_scope scope);
uint __attribute__((overloadable)) atomic_fetch_min_explicit(volatile atomic_uint *object, uint operand, memory_order order, memory_scope scope);
float __attribute__((overloadable)) atomic_fetch_min_explicit(volatile atomic_float *object, float operand, memory_order order, memory_scope scope);

int __attribute__((overloadable)) atomic_fetch_max(volatile atomic_int *object, int operand);
uint __attribute__((overloadable)) atomic_fetch_max(volatile atomic_uint *object, uint operand);
float __attribute__((overloadable)) atomic_fetch_max(volatile atomic_float *object, float operand);
int __attribute__((overloadable)) atomic_fetch_max_explicit(volatile atomic_int *object, int operand, memory_order order);
uint __attribute__((overloadable)) atomic_fetch_max_explicit(volatile atomic_uint *object, uint operand, memory_order order);
float __attribute__((overloadable)) atomic_fetch_max_explicit(volatile atomic_float *object, float operand, memory_order order);
int __attribute__((overloadable)) atomic_fetch_max_explicit(volatile atomic_int *object, int operand, memory_order order, memory_scope scope);
uint __attribute__((overloadable)) atomic_fetch_max_explicit(volatile atomic_uint *object, uint operand, memory_order order, memory_scope scope);
float __attribute__((overloadable)) atomic_fetch_max_explicit(volatile atomic_float *object, float operand, memory_order order, memory_scope scope);

// atomic_flag builtins
bool __attribute__((overloadable)) atomic_flag_test_and_set(volatile atomic_flag *object);
bool __attribute__((overloadable)) atomic_flag_test_and_set_explicit(volatile atomic_flag *object, memory_order);
bool __attribute__((overloadable)) atomic_flag_test_and_set_explicit(volatile atomic_flag *object, memory_order, memory_scope scope);

void __attribute__((overloadable)) atomic_flag_clear(volatile atomic_flag *object);
void __attribute__((overloadable)) atomic_flag_clear_explicit(volatile atomic_flag *object, memory_order);
void __attribute__((overloadable)) atomic_flag_clear_explicit(volatile atomic_flag *object, memory_order, memory_scope scope);

#endif // !defined(MIC)
/** Returns the same value as that returned by
*   get_local_size(dimindx) if the kernel is executed with a
*   uniform work-group size.
*   If the kernel is executed with a non-uniform work-group
*   size, returns the number of local work-items in each of
*   the work-groups that make up the uniform region of the
*   global range in the dimension identified by dimindx. If
*   the local_work_size argument to
*   clEnqueueNDRangeKernel is not NULL, this value
*   will match the value specified in
*   local_work_size[dimindx]. If local_work_size is NULL,
*   this value will match the local size that the
*   implementation determined would be most efficient at
*   implementing the uniform region of the global range.
*   Valid values of dimindx are 0 to get_work_dim() Â¿ 1.
*   For other values of dimindx, get_enqueued_local_size()
*   returns 1.
*/
size_t  __attribute__((const)) __attribute__((overloadable)) get_enqueued_local_size(uint dimindx);

char2 __attribute__((overloadable)) vload2(size_t offset, const char *p);
uchar2 __attribute__((overloadable)) vload2(size_t offset, const uchar *p);
short2 __attribute__((overloadable)) vload2(size_t offset, const short *p);
ushort2 __attribute__((overloadable)) vload2(size_t offset, const ushort *p);
int2 __attribute__((overloadable)) vload2(size_t offset, const int *p);
uint2 __attribute__((overloadable)) vload2(size_t offset, const uint *p);
long2 __attribute__((overloadable)) vload2(size_t offset, const long *p);
ulong2 __attribute__((overloadable)) vload2(size_t offset, const ulong *p);
float2 __attribute__((overloadable)) vload2(size_t offset, const float *p);
char3 __attribute__((overloadable)) vload3(size_t offset, const char *p);
uchar3 __attribute__((overloadable)) vload3(size_t offset, const uchar *p);
short3 __attribute__((overloadable)) vload3(size_t offset, const short *p);
ushort3 __attribute__((overloadable)) vload3(size_t offset, const ushort *p);
int3 __attribute__((overloadable)) vload3(size_t offset, const int *p);
uint3 __attribute__((overloadable)) vload3(size_t offset, const uint *p);
long3 __attribute__((overloadable)) vload3(size_t offset, const long *p);
ulong3 __attribute__((overloadable)) vload3(size_t offset, const ulong *p);
float3 __attribute__((overloadable)) vload3(size_t offset, const float *p);
char4 __attribute__((overloadable)) vload4(size_t offset, const char *p);
uchar4 __attribute__((overloadable)) vload4(size_t offset, const uchar *p);
short4 __attribute__((overloadable)) vload4(size_t offset, const short *p);
ushort4 __attribute__((overloadable)) vload4(size_t offset, const ushort *p);
int4 __attribute__((overloadable)) vload4(size_t offset, const int *p);
uint4 __attribute__((overloadable)) vload4(size_t offset, const uint *p);
long4 __attribute__((overloadable)) vload4(size_t offset, const long *p);
ulong4 __attribute__((overloadable)) vload4(size_t offset, const ulong *p);
float4 __attribute__((overloadable)) vload4(size_t offset, const float *p);
char8 __attribute__((overloadable)) vload8(size_t offset, const char *p);
uchar8 __attribute__((overloadable)) vload8(size_t offset, const uchar *p);
short8 __attribute__((overloadable)) vload8(size_t offset, const short *p);
ushort8 __attribute__((overloadable)) vload8(size_t offset, const ushort *p);
int8 __attribute__((overloadable)) vload8(size_t offset, const int *p);
uint8 __attribute__((overloadable)) vload8(size_t offset, const uint *p);
long8 __attribute__((overloadable)) vload8(size_t offset, const long *p);
ulong8 __attribute__((overloadable)) vload8(size_t offset, const ulong *p);
float8 __attribute__((overloadable)) vload8(size_t offset, const float *p);
char16 __attribute__((overloadable)) vload16(size_t offset, const char *p);
uchar16 __attribute__((overloadable)) vload16(size_t offset, const uchar *p);
short16 __attribute__((overloadable)) vload16(size_t offset, const short *p);
ushort16 __attribute__((overloadable)) vload16(size_t offset, const ushort *p);
int16 __attribute__((overloadable)) vload16(size_t offset, const int *p);
uint16 __attribute__((overloadable)) vload16(size_t offset, const uint *p);
long16 __attribute__((overloadable)) vload16(size_t offset, const long *p);
ulong16 __attribute__((overloadable)) vload16(size_t offset, const ulong *p);
float16 __attribute__((overloadable)) vload16(size_t offset, const float *p);
double2 __attribute__((overloadable)) vload2(size_t offset, const double *p);
double3 __attribute__((overloadable)) vload3(size_t offset, const double *p);
double4 __attribute__((overloadable)) vload4(size_t offset, const double *p);
double8 __attribute__((overloadable)) vload8(size_t offset, const double *p);
double16 __attribute__((overloadable)) vload16(size_t offset, const double *p);

void __attribute__((overloadable)) vstore2(char2 data, size_t offset, char *p);
void __attribute__((overloadable)) vstore2(uchar2 data, size_t offset, uchar *p);
void __attribute__((overloadable)) vstore2(short2 data, size_t offset, short *p);
void __attribute__((overloadable)) vstore2(ushort2 data, size_t offset, ushort *p);
void __attribute__((overloadable)) vstore2(int2 data, size_t offset, int *p);
void __attribute__((overloadable)) vstore2(uint2 data, size_t offset, uint *p);
void __attribute__((overloadable)) vstore2(long2 data, size_t offset, long *p);
void __attribute__((overloadable)) vstore2(ulong2 data, size_t offset, ulong *p);
void __attribute__((overloadable)) vstore2(float2 data, size_t offset, float *p);
void __attribute__((overloadable)) vstore3(char3 data, size_t offset, char *p);
void __attribute__((overloadable)) vstore3(uchar3 data, size_t offset, uchar *p);
void __attribute__((overloadable)) vstore3(short3 data, size_t offset, short *p);
void __attribute__((overloadable)) vstore3(ushort3 data, size_t offset, ushort *p);
void __attribute__((overloadable)) vstore3(int3 data, size_t offset, int *p);
void __attribute__((overloadable)) vstore3(uint3 data, size_t offset, uint *p);
void __attribute__((overloadable)) vstore3(long3 data, size_t offset, long *p);
void __attribute__((overloadable)) vstore3(ulong3 data, size_t offset, ulong *p);
void __attribute__((overloadable)) vstore3(float3 data, size_t offset, float *p);
void __attribute__((overloadable)) vstore4(char4 data, size_t offset, char *p);
void __attribute__((overloadable)) vstore4(uchar4 data, size_t offset, uchar *p);
void __attribute__((overloadable)) vstore4(short4 data, size_t offset, short *p);
void __attribute__((overloadable)) vstore4(ushort4 data, size_t offset, ushort *p);
void __attribute__((overloadable)) vstore4(int4 data, size_t offset, int *p);
void __attribute__((overloadable)) vstore4(uint4 data, size_t offset, uint *p);
void __attribute__((overloadable)) vstore4(long4 data, size_t offset, long *p);
void __attribute__((overloadable)) vstore4(ulong4 data, size_t offset, ulong *p);
void __attribute__((overloadable)) vstore4(float4 data, size_t offset, float *p);
void __attribute__((overloadable)) vstore8(char8 data, size_t offset, char *p);
void __attribute__((overloadable)) vstore8(uchar8 data, size_t offset, uchar *p);
void __attribute__((overloadable)) vstore8(short8 data, size_t offset, short *p);
void __attribute__((overloadable)) vstore8(ushort8 data, size_t offset, ushort *p);
void __attribute__((overloadable)) vstore8(int8 data, size_t offset, int *p);
void __attribute__((overloadable)) vstore8(uint8 data, size_t offset, uint *p);
void __attribute__((overloadable)) vstore8(long8 data, size_t offset, long *p);
void __attribute__((overloadable)) vstore8(ulong8 data, size_t offset, ulong *p);
void __attribute__((overloadable)) vstore8(float8 data, size_t offset, float *p);
void __attribute__((overloadable)) vstore16(char16 data, size_t offset, char *p);
void __attribute__((overloadable)) vstore16(uchar16 data, size_t offset, uchar *p);
void __attribute__((overloadable)) vstore16(short16 data, size_t offset, short *p);
void __attribute__((overloadable)) vstore16(ushort16 data, size_t offset, ushort *p);
void __attribute__((overloadable)) vstore16(int16 data, size_t offset, int *p);
void __attribute__((overloadable)) vstore16(uint16 data, size_t offset, uint *p);
void __attribute__((overloadable)) vstore16(long16 data, size_t offset, long *p);
void __attribute__((overloadable)) vstore16(ulong16 data, size_t offset, ulong *p);
void __attribute__((overloadable)) vstore16(float16 data, size_t offset, float *p);
void __attribute__((overloadable)) vstore2(double2 data, size_t offset, double *p);
void __attribute__((overloadable)) vstore3(double3 data, size_t offset, double *p);
void __attribute__((overloadable)) vstore4(double4 data, size_t offset, double *p);
void __attribute__((overloadable)) vstore8(double8 data, size_t offset, double *p);
void __attribute__((overloadable)) vstore16(double16 data, size_t offset, double *p);
#endif // __OPENCL_C_VERSION__ >= 200
