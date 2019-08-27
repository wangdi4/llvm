#ifndef _OPENCL_20_H_
#define _OPENCL_20_H_

#include "opencl-c-common.h"

// opencl-c-common.h is already contains this statement, but enabled OpenCL
// extensions are not loaded from PCH
#ifdef cl_khr_fp16
#pragma OPENCL EXTENSION cl_khr_fp16 : enable
#endif


typedef int kernel_enqueue_flags_t;
typedef int clk_profiling_info;

typedef enum memory_scope {
  memory_scope_work_item = __OPENCL_MEMORY_SCOPE_WORK_ITEM,
  memory_scope_work_group = __OPENCL_MEMORY_SCOPE_WORK_GROUP,
  memory_scope_device = __OPENCL_MEMORY_SCOPE_DEVICE,
  memory_scope_all_svm_devices = __OPENCL_MEMORY_SCOPE_ALL_SVM_DEVICES,
#if defined(cl_intel_subgroups) || defined(cl_khr_subgroups)
  memory_scope_sub_group = __OPENCL_MEMORY_SCOPE_SUB_GROUP
#endif
} memory_scope;

#ifndef ATOMIC_VAR_INIT
#define ATOMIC_VAR_INIT(x) (x)
#endif //ATOMIC_VAR_INIT
#define ATOMIC_FLAG_INIT 0

// enum values aligned with what clang uses in EmitAtomicExpr()
typedef enum memory_order
{
  memory_order_relaxed = __ATOMIC_RELAXED,
  memory_order_acquire = __ATOMIC_ACQUIRE,
  memory_order_release = __ATOMIC_RELEASE,
  memory_order_acq_rel = __ATOMIC_ACQ_REL,
  memory_order_seq_cst = __ATOMIC_SEQ_CST
} memory_order;

#ifndef cl_khr_depth_images
#define cl_khr_depth_images
#endif //cl_khr_depth_images

#define NULL ((void*)0)

/**
 * Queue a memory fence to ensure correct ordering of memory
 * operations between work-items of a work-group to
 * image memory.
 */
#define CLK_IMAGE_MEM_FENCE  0x04

// Channel order, numbering must be aligned with cl_channel_order in cl.h
#define CLK_sRGB              0x10BF
#define CLK_sRGBx             0x10C0
#define CLK_sRGBA             0x10C1
#define CLK_sBGRA             0x10C2
#define CLK_ABGR              0x10C3

// OpenCL v2.0 s6.13.16 - Pipe Functions
#define CLK_NULL_RESERVE_ID (__builtin_astype((void *)__SIZE_MAX__, reserve_id_t))

// OpenCL v2.0 s6.13.17 - Enqueue Kernels
#define CL_COMPLETE                                 0x0
#define CL_RUNNING                                  0x1
#define CL_SUBMITTED                                0x2
#define CL_QUEUED                                   0x3

#define CLK_SUCCESS                                 0
#define CLK_ENQUEUE_FAILURE                         -101
#define CLK_INVALID_QUEUE                           -102
#define CLK_INVALID_NDRANGE                         -160
#define CLK_INVALID_EVENT_WAIT_LIST                 -57
#define CLK_DEVICE_QUEUE_FULL                       -161
#define CLK_INVALID_ARG_SIZE                        -51
#define CLK_EVENT_ALLOCATION_FAILURE                -100
#define CLK_OUT_OF_RESOURCES                        -5

#define CLK_NULL_QUEUE                              0
#define CLK_NULL_EVENT (__builtin_astype(((__SIZE_MAX__)), clk_event_t))

// execution model related definitions
#define CLK_ENQUEUE_FLAGS_NO_WAIT                   0x0
#define CLK_ENQUEUE_FLAGS_WAIT_KERNEL               0x1
#define CLK_ENQUEUE_FLAGS_WAIT_WORK_GROUP           0x2

// Profiling info name (see capture_event_profiling_info)
#define CLK_PROFILING_COMMAND_EXEC_TIME 0x1

/**
 * Returns the count of trailing 0-bits in x. If x is 0,
 * returns the size in bits of the type of x or
 * component type of x, if x is a vector.
 */
char __ovld ctz(char x);
uchar __ovld ctz(uchar x);
char2 __ovld ctz(char2 x);
uchar2 __ovld ctz(uchar2 x);
char3 __ovld ctz(char3 x);
uchar3 __ovld ctz(uchar3 x);
char4 __ovld ctz(char4 x);
uchar4 __ovld ctz(uchar4 x);
char8 __ovld ctz(char8 x);
uchar8 __ovld ctz(uchar8 x);
char16 __ovld ctz(char16 x);
uchar16 __ovld ctz(uchar16 x);
short __ovld ctz(short x);
ushort __ovld ctz(ushort x);
short2 __ovld ctz(short2 x);
ushort2 __ovld ctz(ushort2 x);
short3 __ovld ctz(short3 x);
ushort3 __ovld ctz(ushort3 x);
short4 __ovld ctz(short4 x);
ushort4 __ovld ctz(ushort4 x);
short8 __ovld ctz(short8 x);
ushort8 __ovld ctz(ushort8 x);
short16 __ovld ctz(short16 x);
ushort16 __ovld ctz(ushort16 x);
int __ovld ctz(int x);
uint __ovld ctz(uint x);
int2 __ovld ctz(int2 x);
uint2 __ovld ctz(uint2 x);
int3 __ovld ctz(int3 x);
uint3 __ovld ctz(uint3 x);
int4 __ovld ctz(int4 x);
uint4 __ovld ctz(uint4 x);
int8 __ovld ctz(int8 x);
uint8 __ovld ctz(uint8 x);
int16 __ovld ctz(int16 x);
uint16 __ovld ctz(uint16 x);
long __ovld ctz(long x);
ulong __ovld ctz(ulong x);
long2 __ovld ctz(long2 x);
ulong2 __ovld ctz(ulong2 x);
long3 __ovld ctz(long3 x);
ulong3 __ovld ctz(ulong3 x);
long4 __ovld ctz(long4 x);
ulong4 __ovld ctz(ulong4 x);
long8 __ovld ctz(long8 x);
ulong8 __ovld ctz(ulong8 x);
long16 __ovld ctz(long16 x);
ulong16 __ovld ctz(ulong16 x);


// OpenCL v1.1 s6.11.8, v1.2 s6.12.8, v2.0 s6.13.8 - Synchronization Functions

void __ovld __conv work_group_barrier(cl_mem_fence_flags flags, memory_scope scope);
void __ovld __conv work_group_barrier(cl_mem_fence_flags flags);

// OpenCL Extension v2.0 s9.17 - Sub-groups
#if defined(cl_intel_subgroups) || defined(cl_khr_subgroups)
uint    __ovld get_enqueued_num_sub_groups(void);
void    __ovld __conv sub_group_barrier(cl_mem_fence_flags flags, memory_scope scope);
#endif

// Intel-Specific Sub Group Functions
#if defined(cl_intel_subgroups)
uint    __ovld __conv intel_sub_group_block_read(read_write image2d_t image, int2 coord);
uint2   __ovld __conv intel_sub_group_block_read2(read_write image2d_t image, int2 coord);
uint4   __ovld __conv intel_sub_group_block_read4(read_write image2d_t image, int2 coord);
uint8   __ovld __conv intel_sub_group_block_read8(read_write image2d_t image, int2 coord);

void    __ovld __conv intel_sub_group_block_write(read_write image2d_t image, int2 coord, uint data);
void    __ovld __conv intel_sub_group_block_write2(read_write image2d_t image, int2 coord, uint2 data);
void    __ovld __conv intel_sub_group_block_write4(read_write image2d_t image, int2 coord, uint4 data);
void    __ovld __conv intel_sub_group_block_write8(read_write image2d_t image, int2 coord, uint8 data);
#endif //cl_intel_subgroups

#if defined(cl_intel_subgroups_short)
uint    __ovld __conv intel_sub_group_block_read_ui( read_write image2d_t image, int2 byte_coord );
uint2   __ovld __conv intel_sub_group_block_read_ui2( read_write image2d_t image, int2 byte_coord );
uint4   __ovld __conv intel_sub_group_block_read_ui4( read_write image2d_t image, int2 byte_coord );
uint8   __ovld __conv intel_sub_group_block_read_ui8( read_write image2d_t image, int2 byte_coord );

void    __ovld __conv intel_sub_group_block_write_ui( read_write image2d_t image, int2 byte_coord, uint data );
void    __ovld __conv intel_sub_group_block_write_ui2( read_write image2d_t image, int2 byte_coord, uint2 data );
void    __ovld __conv intel_sub_group_block_write_ui4( read_write image2d_t image, int2 byte_coord, uint4 data );
void    __ovld __conv intel_sub_group_block_write_ui8( read_write image2d_t image, int2 byte_coord, uint8 data );

ushort  __ovld __conv intel_sub_group_block_read_us( read_write image2d_t image, int2 coord);
ushort2 __ovld __conv intel_sub_group_block_read_us2( read_write image2d_t image, int2 coord);
ushort4 __ovld __conv intel_sub_group_block_read_us4( read_write image2d_t image, int2 coord);
ushort8 __ovld __conv intel_sub_group_block_read_us8( read_write image2d_t image, int2 coord);

void    __ovld __conv intel_sub_group_block_write_us( read_write image2d_t image, int2 coord, ushort  data);
void    __ovld __conv intel_sub_group_block_write_us2( read_write image2d_t image, int2 coord, ushort2 data);
void    __ovld __conv intel_sub_group_block_write_us4( read_write image2d_t image, int2 coord, ushort4 data);
void    __ovld __conv intel_sub_group_block_write_us8( read_write image2d_t image, int2 coord, ushort8 data);
#endif // cl_intel_subgroups_short

/**
 * Builtin functions to_global, to_local, and to_private need to be declared as Clang builtin functions
 * and checked in Sema since they should be declared as
 *   addr gentype* to_addr (gentype*);
 * where gentype is builtin type or user defined type.
 */

// Image read functions for read_write images
float4 __purefn __ovld read_imagef(read_write image1d_t image, int coord);
int4 __purefn __ovld read_imagei(read_write image1d_t image, int coord);
uint4 __purefn __ovld read_imageui(read_write image1d_t image, int coord);

float4 __purefn __ovld read_imagef(read_write image1d_buffer_t image, int coord);
int4 __purefn __ovld read_imagei(read_write image1d_buffer_t image, int coord);
uint4 __purefn __ovld read_imageui(read_write image1d_buffer_t image, int coord);

float4 __purefn __ovld read_imagef(read_write image1d_array_t image, int2 coord);
int4 __purefn __ovld read_imagei(read_write image1d_array_t image, int2 coord);
uint4 __purefn __ovld read_imageui(read_write image1d_array_t image, int2 coord);

float4 __purefn __ovld read_imagef(read_write image2d_t image, int2 coord);
int4 __purefn __ovld read_imagei(read_write image2d_t image, int2 coord);
uint4 __purefn __ovld read_imageui(read_write image2d_t image, int2 coord);

float4 __purefn __ovld read_imagef(read_write image2d_array_t image, int4 coord);
int4 __purefn __ovld read_imagei(read_write image2d_array_t image, int4 coord);
uint4 __purefn __ovld read_imageui(read_write image2d_array_t image, int4 coord);

float4 __purefn __ovld read_imagef(read_write image3d_t image, int4 coord);
int4 __purefn __ovld read_imagei(read_write image3d_t image, int4 coord);
uint4 __purefn __ovld read_imageui(read_write image3d_t image, int4 coord);

#ifdef cl_khr_depth_images
float __purefn __ovld read_imagef(read_write image2d_depth_t image, int2 coord);
float __purefn __ovld read_imagef(read_write image2d_array_depth_t image, int4 coord);
#endif //cl_khr_depth_images

#if cl_khr_gl_msaa_sharing
float4 __purefn __ovld read_imagef(read_write image2d_msaa_t image, int2 coord, int sample);
int4 __purefn __ovld read_imagei(read_write image2d_msaa_t image, int2 coord, int sample);
uint4 __purefn __ovld read_imageui(read_write image2d_msaa_t image, int2 coord, int sample);

float4 __purefn __ovld read_imagef(read_write image2d_array_msaa_t image, int4 coord, int sample);
int4 __purefn __ovld read_imagei(read_write image2d_array_msaa_t image, int4 coord, int sample);
uint4 __purefn __ovld read_imageui(read_write image2d_array_msaa_t image, int4 coord, int sample);

float __purefn __ovld read_imagef(read_write image2d_msaa_depth_t image, int2 coord, int sample);
float __purefn __ovld read_imagef(read_write image2d_array_msaa_depth_t image, int4 coord, int sample);
#endif //cl_khr_gl_msaa_sharing

#if defined(__OPENCL_CPP_VERSION__) || (__OPENCL_C_VERSION__ >= CL_VERSION_2_0)
#ifdef cl_khr_mipmap_image
float4 __purefn __ovld read_imagef(read_write image1d_t image, sampler_t sampler, float coord, float lod);
int4 __purefn __ovld read_imagei(read_write image1d_t image, sampler_t sampler, float coord, float lod);
uint4 __purefn __ovld read_imageui(read_write image1d_t image, sampler_t sampler, float coord, float lod);

float4 __purefn __ovld read_imagef(read_write image1d_array_t image_array, sampler_t sampler, float2 coord, float lod);
int4 __purefn __ovld read_imagei(read_write image1d_array_t image_array, sampler_t sampler, float2 coord, float lod);
uint4 __purefn __ovld read_imageui(read_write image1d_array_t image_array, sampler_t sampler, float2 coord, float lod);

float4 __purefn __ovld read_imagef(read_write image2d_t image, sampler_t sampler, float2 coord, float lod);
int4 __purefn __ovld read_imagei(read_write image2d_t image, sampler_t sampler, float2 coord, float lod);
uint4 __purefn __ovld read_imageui(read_write image2d_t image, sampler_t sampler, float2 coord, float lod);

float __purefn __ovld read_imagef(read_write image2d_depth_t image, sampler_t sampler, float2 coord, float lod);

float4 __purefn __ovld read_imagef(read_write image2d_array_t image_array, sampler_t sampler, float4 coord, float lod);
int4 __purefn __ovld read_imagei(read_write image2d_array_t image_array, sampler_t sampler, float4 coord, float lod);
uint4 __purefn __ovld read_imageui(read_write image2d_array_t image_array, sampler_t sampler, float4 coord, float lod);

float __purefn __ovld read_imagef(read_write image2d_array_depth_t image, sampler_t sampler, float4 coord, float lod);

float4 __purefn __ovld read_imagef(read_write image3d_t image, sampler_t sampler, float4 coord, float lod);
int4 __purefn __ovld read_imagei(read_write image3d_t image, sampler_t sampler, float4 coord, float lod);
uint4 __purefn __ovld read_imageui(read_write image3d_t image, sampler_t sampler, float4 coord, float lod);

float4 __purefn __ovld read_imagef(read_write image1d_t image, sampler_t sampler, float coord, float gradientX, float gradientY);
int4 __purefn __ovld read_imagei(read_write image1d_t image, sampler_t sampler, float coord, float gradientX, float gradientY);
uint4 __purefn __ovld read_imageui(read_write image1d_t image, sampler_t sampler, float coord, float gradientX, float gradientY);

float4 __purefn __ovld read_imagef(read_write image1d_array_t image_array, sampler_t sampler, float2 coord, float gradientX, float gradientY);
int4 __purefn __ovld read_imagei(read_write image1d_array_t image_array, sampler_t sampler, float2 coord, float gradientX, float gradientY);
uint4 __purefn __ovld read_imageui(read_write image1d_array_t image_array, sampler_t sampler, float2 coord, float gradientX, float gradientY);

float4 __purefn __ovld read_imagef(read_write image2d_t image, sampler_t sampler, float2 coord, float2 gradientX, float2 gradientY);
int4 __purefn __ovld read_imagei(read_write image2d_t image, sampler_t sampler, float2 coord, float2 gradientX, float2 gradientY);
uint4 __purefn __ovld read_imageui(read_write image2d_t image, sampler_t sampler, float2 coord, float2 gradientX, float2 gradientY);

float __purefn __ovld read_imagef(read_write image2d_depth_t image, sampler_t sampler, float2 coord, float2 gradientX, float2 gradientY);

float4 __purefn __ovld read_imagef(read_write image2d_array_t image_array, sampler_t sampler, float4 coord, float2 gradientX, float2 gradientY);
int4 __purefn __ovld read_imagei(read_write image2d_array_t image_array, sampler_t sampler, float4 coord, float2 gradientX, float2 gradientY);
uint4 __purefn __ovld read_imageui(read_write image2d_array_t image_array, sampler_t sampler, float4 coord, float2 gradientX, float2 gradientY);

float __purefn __ovld read_imagef(read_write image2d_array_depth_t image, sampler_t sampler, float4 coord, float2 gradientX, float2 gradientY);

float4 __purefn __ovld read_imagef(read_write image3d_t image, sampler_t sampler, float4 coord, float4 gradientX, float4 gradientY);
int4 __purefn __ovld read_imagei(read_write image3d_t image, sampler_t sampler, float4 coord, float4 gradientX, float4 gradientY);
uint4 __purefn __ovld read_imageui(read_write image3d_t image, sampler_t sampler, float4 coord, float4 gradientX, float4 gradientY);
#endif //cl_khr_mipmap_image
#endif // defined(__OPENCL_CPP_VERSION__) || (__OPENCL_C_VERSION__ >= CL_VERSION_2_0)

// Image read functions returning half4 type
#ifdef cl_khr_fp16
half4 __purefn __ovld read_imageh(read_write image1d_t image, int coord);
half4 __purefn __ovld read_imageh(read_write image2d_t image, int2 coord);
half4 __purefn __ovld read_imageh(read_write image3d_t image, int4 coord);
half4 __purefn __ovld read_imageh(read_write image1d_array_t image, int2 coord);
half4 __purefn __ovld read_imageh(read_write image2d_array_t image, int4 coord);
half4 __purefn __ovld read_imageh(read_write image1d_buffer_t image, int coord);
#endif //cl_khr_fp16

// Image write functions for read_write images
void __ovld write_imagef(read_write image2d_t image, int2 coord, float4 color);
void __ovld write_imagei(read_write image2d_t image, int2 coord, int4 color);
void __ovld write_imageui(read_write image2d_t image, int2 coord, uint4 color);

void __ovld write_imagef(read_write image2d_array_t image_array, int4 coord, float4 color);
void __ovld write_imagei(read_write image2d_array_t image_array, int4 coord, int4 color);
void __ovld write_imageui(read_write image2d_array_t image_array, int4 coord, uint4 color);

void __ovld write_imagef(read_write image1d_t image, int coord, float4 color);
void __ovld write_imagei(read_write image1d_t image, int coord, int4 color);
void __ovld write_imageui(read_write image1d_t image, int coord, uint4 color);

void __ovld write_imagef(read_write image1d_buffer_t image, int coord, float4 color);
void __ovld write_imagei(read_write image1d_buffer_t image, int coord, int4 color);
void __ovld write_imageui(read_write image1d_buffer_t image, int coord, uint4 color);

void __ovld write_imagef(read_write image1d_array_t image_array, int2 coord, float4 color);
void __ovld write_imagei(read_write image1d_array_t image_array, int2 coord, int4 color);
void __ovld write_imageui(read_write image1d_array_t image_array, int2 coord, uint4 color);

#ifdef cl_khr_3d_image_writes
void __ovld write_imagef(read_write image3d_t image, int4 coord, float4 color);
void __ovld write_imagei(read_write image3d_t image, int4 coord, int4 color);
void __ovld write_imageui(read_write image3d_t image, int4 coord, uint4 color);
#endif

#ifdef cl_khr_depth_images
void __ovld write_imagef(read_write image2d_depth_t image, int2 coord, float color);
void __ovld write_imagef(read_write image2d_array_depth_t image, int4 coord, float color);
#endif //cl_khr_depth_images

#if defined(__OPENCL_CPP_VERSION__) || (__OPENCL_C_VERSION__ >= CL_VERSION_2_0)
#ifdef cl_khr_mipmap_image
void __ovld write_imagef(read_write image1d_t image, int coord, int lod, float4 color);
void __ovld write_imagei(read_write image1d_t image, int coord, int lod, int4 color);
void __ovld write_imageui(read_write image1d_t image, int coord, int lod, uint4 color);

void __ovld write_imagef(read_write image1d_array_t image_array, int2 coord, int lod, float4 color);
void __ovld write_imagei(read_write image1d_array_t image_array, int2 coord, int lod, int4 color);
void __ovld write_imageui(read_write image1d_array_t image_array, int2 coord, int lod, uint4 color);

void __ovld write_imagef(read_write image2d_t image, int2 coord, int lod, float4 color);
void __ovld write_imagei(read_write image2d_t image, int2 coord, int lod, int4 color);
void __ovld write_imageui(read_write image2d_t image, int2 coord, int lod, uint4 color);

void __ovld write_imagef(read_write image2d_array_t image_array, int4 coord, int lod, float4 color);
void __ovld write_imagei(read_write image2d_array_t image_array, int4 coord, int lod, int4 color);
void __ovld write_imageui(read_write image2d_array_t image_array, int4 coord, int lod, uint4 color);

void __ovld write_imagef(read_write image2d_depth_t image, int2 coord, int lod, float color);
void __ovld write_imagef(read_write image2d_array_depth_t image, int4 coord, int lod, float color);

#ifdef cl_khr_3d_image_writes
void __ovld write_imagef(read_write image3d_t image, int4 coord, int lod, float4 color);
void __ovld write_imagei(read_write image3d_t image, int4 coord, int lod, int4 color);
void __ovld write_imageui(read_write image3d_t image, int4 coord, int lod, uint4 color);
#endif
#endif //cl_khr_mipmap_image
#endif // defined(__OPENCL_CPP_VERSION__) || (__OPENCL_C_VERSION__ >= CL_VERSION_2_0)

// Image write functions for half4 type
#ifdef cl_khr_fp16
void __ovld write_imageh(read_write image1d_t image, int coord, half4 color);
void __ovld write_imageh(read_write image2d_t image, int2 coord, half4 color);
#ifdef cl_khr_3d_image_writes
void __ovld write_imageh(read_write image3d_t image, int4 coord, half4 color);
#endif
void __ovld write_imageh(read_write image1d_array_t image, int2 coord, half4 color);
void __ovld write_imageh(read_write image2d_array_t image, int4 coord, half4 color);
void __ovld write_imageh(read_write image1d_buffer_t image, int coord, half4 color);
#endif //cl_khr_fp16

/**
 * Return the image width in pixels.
 */
int __ovld __cnfn get_image_width(read_write image1d_t image);
int __ovld __cnfn get_image_width(read_write image1d_buffer_t image);
int __ovld __cnfn get_image_width(read_write image2d_t image);
int __ovld __cnfn get_image_width(read_write image3d_t image);
int __ovld __cnfn get_image_width(read_write image1d_array_t image);
int __ovld __cnfn get_image_width(read_write image2d_array_t image);
#ifdef cl_khr_depth_images
int __ovld __cnfn get_image_width(read_write image2d_depth_t image);
int __ovld __cnfn get_image_width(read_write image2d_array_depth_t image);
#endif //cl_khr_depth_images
#if defined(cl_khr_gl_msaa_sharing)
int __ovld __cnfn get_image_width(read_write image2d_msaa_t image);
int __ovld __cnfn get_image_width(read_write image2d_msaa_depth_t image);
int __ovld __cnfn get_image_width(read_write image2d_array_msaa_t image);
int __ovld __cnfn get_image_width(read_write image2d_array_msaa_depth_t image);
#endif //cl_khr_gl_msaa_sharing

/**
 * Return the image height in pixels.
 */
int __ovld __cnfn get_image_height(read_write image2d_t image);
int __ovld __cnfn get_image_height(read_write image3d_t image);
int __ovld __cnfn get_image_height(read_write image2d_array_t image);
#ifdef cl_khr_depth_images
int __ovld __cnfn get_image_height(read_write image2d_depth_t image);
int __ovld __cnfn get_image_height(read_write image2d_array_depth_t image);
#endif //cl_khr_depth_images
#if defined(cl_khr_gl_msaa_sharing)
int __ovld __cnfn get_image_height(read_write image2d_msaa_t image);
int __ovld __cnfn get_image_height(read_write image2d_msaa_depth_t image);
int __ovld __cnfn get_image_height(read_write image2d_array_msaa_t image);
int __ovld __cnfn get_image_height(read_write image2d_array_msaa_depth_t image);
#endif //cl_khr_gl_msaa_sharing

/**
 * Return the image depth in pixels.
 */
int __ovld __cnfn get_image_depth(read_write image3d_t image);

/**
 * Return the image miplevels.
 */
int __ovld get_image_num_mip_levels(read_write image1d_t image);
int __ovld get_image_num_mip_levels(read_write image2d_t image);
int __ovld get_image_num_mip_levels(read_write image3d_t image);

int __ovld get_image_num_mip_levels(read_write image1d_array_t image);
int __ovld get_image_num_mip_levels(read_write image2d_array_t image);
int __ovld get_image_num_mip_levels(read_write image2d_array_depth_t image);
int __ovld get_image_num_mip_levels(read_write image2d_depth_t image);

/**
 * Return the channel data type. Valid values are:
 * CLK_SNORM_INT8
 * CLK_SNORM_INT16
 * CLK_UNORM_INT8
 * CLK_UNORM_INT16
 * CLK_UNORM_SHORT_565
 * CLK_UNORM_SHORT_555
 * CLK_UNORM_SHORT_101010
 * CLK_SIGNED_INT8
 * CLK_SIGNED_INT16
 * CLK_SIGNED_INT32
 * CLK_UNSIGNED_INT8
 * CLK_UNSIGNED_INT16
 * CLK_UNSIGNED_INT32
 * CLK_HALF_FLOAT
 * CLK_FLOAT
 */
int __ovld __cnfn get_image_channel_data_type(read_write image1d_t image);
int __ovld __cnfn get_image_channel_data_type(read_write image1d_buffer_t image);
int __ovld __cnfn get_image_channel_data_type(read_write image2d_t image);
int __ovld __cnfn get_image_channel_data_type(read_write image3d_t image);
int __ovld __cnfn get_image_channel_data_type(read_write image1d_array_t image);
int __ovld __cnfn get_image_channel_data_type(read_write image2d_array_t image);
#ifdef cl_khr_depth_images
int __ovld __cnfn get_image_channel_data_type(read_write image2d_depth_t image);
int __ovld __cnfn get_image_channel_data_type(read_write image2d_array_depth_t image);
#endif //cl_khr_depth_images
#if defined(cl_khr_gl_msaa_sharing)
int __ovld __cnfn get_image_channel_data_type(read_write image2d_msaa_t image);
int __ovld __cnfn get_image_channel_data_type(read_write image2d_msaa_depth_t image);
int __ovld __cnfn get_image_channel_data_type(read_write image2d_array_msaa_t image);
int __ovld __cnfn get_image_channel_data_type(read_write image2d_array_msaa_depth_t image);
#endif //cl_khr_gl_msaa_sharing

/**
 * Return the image channel order. Valid values are:
 * CLK_A
 * CLK_R
 * CLK_Rx
 * CLK_RG
 * CLK_RGx
 * CLK_RA
 * CLK_RGB
 * CLK_RGBx
 * CLK_RGBA
 * CLK_ARGB
 * CLK_BGRA
 * CLK_INTENSITY
 * CLK_LUMINANCE
 */
int __ovld __cnfn get_image_channel_order(read_write image1d_t image);
int __ovld __cnfn get_image_channel_order(read_write image1d_buffer_t image);
int __ovld __cnfn get_image_channel_order(read_write image2d_t image);
int __ovld __cnfn get_image_channel_order(read_write image3d_t image);
int __ovld __cnfn get_image_channel_order(read_write image1d_array_t image);
int __ovld __cnfn get_image_channel_order(read_write image2d_array_t image);
#ifdef cl_khr_depth_images
int __ovld __cnfn get_image_channel_order(read_write image2d_depth_t image);
int __ovld __cnfn get_image_channel_order(read_write image2d_array_depth_t image);
#endif //cl_khr_depth_images
#if defined(cl_khr_gl_msaa_sharing)
int __ovld __cnfn get_image_channel_order(read_write image2d_msaa_t image);
int __ovld __cnfn get_image_channel_order(read_write image2d_msaa_depth_t image);
int __ovld __cnfn get_image_channel_order(read_write image2d_array_msaa_t image);
int __ovld __cnfn get_image_channel_order(read_write image2d_array_msaa_depth_t image);
#endif //cl_khr_gl_msaa_sharing

/**
 * Return the 2D image width and height as an int2
 * type. The width is returned in the x component, and
 * the height in the y component.
 */
int2 __ovld __cnfn get_image_dim(read_write image2d_t image);
int2 __ovld __cnfn get_image_dim(read_write image2d_array_t image);
#ifdef cl_khr_depth_images
int2 __ovld __cnfn get_image_dim(read_write image2d_array_depth_t image);
int2 __ovld __cnfn get_image_dim(read_write image2d_depth_t image);
#endif //cl_khr_depth_images
#if defined(cl_khr_gl_msaa_sharing)
int2 __ovld __cnfn get_image_dim(read_write image2d_msaa_t image);
int2 __ovld __cnfn get_image_dim(read_write image2d_msaa_depth_t image);
int2 __ovld __cnfn get_image_dim(read_write image2d_array_msaa_t image);
int2 __ovld __cnfn get_image_dim(read_write image2d_array_msaa_depth_t image);
#endif //cl_khr_gl_msaa_sharing

/**
 * Return the 3D image width, height, and depth as an
 * int4 type. The width is returned in the x
 * component, height in the y component, depth in the z
 * component and the w component is 0.
 */
int4 __ovld __cnfn get_image_dim(read_write image3d_t image);


/**
* Return the number of samples associated with image
*/
#if defined(cl_khr_gl_msaa_sharing)
int __ovld get_image_num_samples(read_write image2d_msaa_t image);
int __ovld get_image_num_samples(read_write image2d_msaa_depth_t image);
int __ovld get_image_num_samples(read_write image2d_array_msaa_depth_t image);
int __ovld get_image_num_samples(read_write image2d_array_msaa_t image);
#endif

// OpenCL v2.0 s6.13.15 - Work-group Functions

int __ovld __conv work_group_all(int predicate);
int __ovld __conv work_group_any(int predicate);

#ifdef cl_khr_fp16
half __ovld __conv work_group_reduce_add(half x);
half __ovld __conv work_group_reduce_min(half x);
half __ovld __conv work_group_reduce_max(half x);
half __ovld __conv work_group_scan_exclusive_add(half x);
half __ovld __conv work_group_scan_exclusive_min(half x);
half __ovld __conv work_group_scan_exclusive_max(half x);
half __ovld __conv work_group_scan_inclusive_add(half x);
half __ovld __conv work_group_scan_inclusive_min(half x);
half __ovld __conv work_group_scan_inclusive_max(half x);
#endif
int __ovld __conv work_group_reduce_add(int x);
int __ovld __conv work_group_reduce_min(int x);
int __ovld __conv work_group_reduce_max(int x);
int __ovld __conv work_group_scan_exclusive_add(int x);
int __ovld __conv work_group_scan_exclusive_min(int x);
int __ovld __conv work_group_scan_exclusive_max(int x);
int __ovld __conv work_group_scan_inclusive_add(int x);
int __ovld __conv work_group_scan_inclusive_min(int x);
int __ovld __conv work_group_scan_inclusive_max(int x);
uint __ovld __conv work_group_reduce_add(uint x);
uint __ovld __conv work_group_reduce_min(uint x);
uint __ovld __conv work_group_reduce_max(uint x);
uint __ovld __conv work_group_scan_exclusive_add(uint x);
uint __ovld __conv work_group_scan_exclusive_min(uint x);
uint __ovld __conv work_group_scan_exclusive_max(uint x);
uint __ovld __conv work_group_scan_inclusive_add(uint x);
uint __ovld __conv work_group_scan_inclusive_min(uint x);
uint __ovld __conv work_group_scan_inclusive_max(uint x);
long __ovld __conv work_group_reduce_add(long x);
long __ovld __conv work_group_reduce_min(long x);
long __ovld __conv work_group_reduce_max(long x);
long __ovld __conv work_group_scan_exclusive_add(long x);
long __ovld __conv work_group_scan_exclusive_min(long x);
long __ovld __conv work_group_scan_exclusive_max(long x);
long __ovld __conv work_group_scan_inclusive_add(long x);
long __ovld __conv work_group_scan_inclusive_min(long x);
long __ovld __conv work_group_scan_inclusive_max(long x);
ulong __ovld __conv work_group_reduce_add(ulong x);
ulong __ovld __conv work_group_reduce_min(ulong x);
ulong __ovld __conv work_group_reduce_max(ulong x);
ulong __ovld __conv work_group_scan_exclusive_add(ulong x);
ulong __ovld __conv work_group_scan_exclusive_min(ulong x);
ulong __ovld __conv work_group_scan_exclusive_max(ulong x);
ulong __ovld __conv work_group_scan_inclusive_add(ulong x);
ulong __ovld __conv work_group_scan_inclusive_min(ulong x);
ulong __ovld __conv work_group_scan_inclusive_max(ulong x);
float __ovld __conv work_group_reduce_add(float x);
float __ovld __conv work_group_reduce_min(float x);
float __ovld __conv work_group_reduce_max(float x);
float __ovld __conv work_group_scan_exclusive_add(float x);
float __ovld __conv work_group_scan_exclusive_min(float x);
float __ovld __conv work_group_scan_exclusive_max(float x);
float __ovld __conv work_group_scan_inclusive_add(float x);
float __ovld __conv work_group_scan_inclusive_min(float x);
float __ovld __conv work_group_scan_inclusive_max(float x);
#ifdef cl_khr_fp64
double __ovld __conv work_group_reduce_add(double x);
double __ovld __conv work_group_reduce_min(double x);
double __ovld __conv work_group_reduce_max(double x);
double __ovld __conv work_group_scan_exclusive_add(double x);
double __ovld __conv work_group_scan_exclusive_min(double x);
double __ovld __conv work_group_scan_exclusive_max(double x);
double __ovld __conv work_group_scan_inclusive_add(double x);
double __ovld __conv work_group_scan_inclusive_min(double x);
double __ovld __conv work_group_scan_inclusive_max(double x);
#endif //cl_khr_fp64

// OpenCL v2.0 s6.13.16 - Pipe Functions
bool __ovld is_valid_reserve_id(reserve_id_t reserve_id);

// OpenCL Extension v2.0 s9.18 - Mipmaps
#if defined(__OPENCL_CPP_VERSION__) || (__OPENCL_C_VERSION__ >= CL_VERSION_2_0)
#ifdef cl_khr_mipmap_image

float4 __purefn __ovld read_imagef(read_only image1d_t image, sampler_t sampler, float coord, float lod);
int4 __purefn __ovld read_imagei(read_only image1d_t image, sampler_t sampler, float coord, float lod);
uint4 __purefn __ovld read_imageui(read_only image1d_t image, sampler_t sampler, float coord, float lod);

float4 __purefn __ovld read_imagef(read_only image1d_array_t image_array, sampler_t sampler, float2 coord, float lod);
int4 __purefn __ovld read_imagei(read_only image1d_array_t image_array, sampler_t sampler, float2 coord, float lod);
uint4 __purefn __ovld read_imageui(read_only image1d_array_t image_array, sampler_t sampler, float2 coord, float lod);

float4 __purefn __ovld read_imagef(read_only image2d_t image, sampler_t sampler, float2 coord, float lod);
int4 __purefn __ovld read_imagei(read_only image2d_t image, sampler_t sampler, float2 coord, float lod);
uint4 __purefn __ovld read_imageui(read_only image2d_t image, sampler_t sampler, float2 coord, float lod);

float __purefn __ovld read_imagef(read_only image2d_depth_t image, sampler_t sampler, float2 coord, float lod);

float4 __purefn __ovld read_imagef(read_only image2d_array_t image_array, sampler_t sampler, float4 coord, float lod);
int4 __purefn __ovld read_imagei(read_only image2d_array_t image_array, sampler_t sampler, float4 coord, float lod);
uint4 __purefn __ovld read_imageui(read_only image2d_array_t image_array, sampler_t sampler, float4 coord, float lod);

float __purefn __ovld read_imagef(read_only image2d_array_depth_t image, sampler_t sampler, float4 coord, float lod);

float4 __purefn __ovld read_imagef(read_only image3d_t image, sampler_t sampler, float4 coord, float lod);
int4 __purefn __ovld read_imagei(read_only image3d_t image, sampler_t sampler, float4 coord, float lod);
uint4 __purefn __ovld read_imageui(read_only image3d_t image, sampler_t sampler, float4 coord, float lod);

float4 __purefn __ovld read_imagef(read_only image1d_t image, sampler_t sampler, float coord, float gradientX, float gradientY);
int4 __purefn __ovld read_imagei(read_only image1d_t image, sampler_t sampler, float coord, float gradientX, float gradientY);
uint4 __purefn __ovld read_imageui(read_only image1d_t image, sampler_t sampler, float coord, float gradientX, float gradientY);

float4 __purefn __ovld read_imagef(read_only image1d_array_t image_array, sampler_t sampler, float2 coord, float gradientX, float gradientY);
int4 __purefn __ovld read_imagei(read_only image1d_array_t image_array, sampler_t sampler, float2 coord, float gradientX, float gradientY);
uint4 __purefn __ovld read_imageui(read_only image1d_array_t image_array, sampler_t sampler, float2 coord, float gradientX, float gradientY);

float4 __purefn __ovld read_imagef(read_only image2d_t image, sampler_t sampler, float2 coord, float2 gradientX, float2 gradientY);
int4 __purefn __ovld read_imagei(read_only image2d_t image, sampler_t sampler, float2 coord, float2 gradientX, float2 gradientY);
uint4 __purefn __ovld read_imageui(read_only image2d_t image, sampler_t sampler, float2 coord, float2 gradientX, float2 gradientY);

float __purefn __ovld read_imagef(read_only image2d_depth_t image, sampler_t sampler, float2 coord, float2 gradientX, float2 gradientY);

float4 __purefn __ovld read_imagef(read_only image2d_array_t image_array, sampler_t sampler, float4 coord, float2 gradientX, float2 gradientY);
int4 __purefn __ovld read_imagei(read_only image2d_array_t image_array, sampler_t sampler, float4 coord, float2 gradientX, float2 gradientY);
uint4 __purefn __ovld read_imageui(read_only image2d_array_t image_array, sampler_t sampler, float4 coord, float2 gradientX, float2 gradientY);

float __purefn __ovld read_imagef(read_only image2d_array_depth_t image, sampler_t sampler, float4 coord, float2 gradientX, float2 gradientY);

float4 __purefn __ovld read_imagef(read_only image3d_t image, sampler_t sampler, float4 coord, float4 gradientX, float4 gradientY);
int4 __purefn __ovld read_imagei(read_only image3d_t image, sampler_t sampler, float4 coord, float4 gradientX, float4 gradientY);
uint4 __purefn __ovld read_imageui(read_only image3d_t image, sampler_t sampler, float4 coord, float4 gradientX, float4 gradientY);

void __ovld write_imagef(write_only image1d_t image, int coord, int lod, float4 color);
void __ovld write_imagei(write_only image1d_t image, int coord, int lod, int4 color);
void __ovld write_imageui(write_only image1d_t image, int coord, int lod, uint4 color);

void __ovld write_imagef(write_only image1d_array_t image_array, int2 coord, int lod, float4 color);
void __ovld write_imagei(write_only image1d_array_t image_array, int2 coord, int lod, int4 color);
void __ovld write_imageui(write_only image1d_array_t image_array, int2 coord, int lod, uint4 color);

void __ovld write_imagef(write_only image2d_t image, int2 coord, int lod, float4 color);
void __ovld write_imagei(write_only image2d_t image, int2 coord, int lod, int4 color);
void __ovld write_imageui(write_only image2d_t image, int2 coord, int lod, uint4 color);

void __ovld write_imagef(write_only image2d_array_t image_array, int4 coord, int lod, float4 color);
void __ovld write_imagei(write_only image2d_array_t image_array, int4 coord, int lod, int4 color);
void __ovld write_imageui(write_only image2d_array_t image_array, int4 coord, int lod, uint4 color);

void __ovld write_imagef(write_only image2d_depth_t image, int2 coord, int lod, float color);
void __ovld write_imagef(write_only image2d_array_depth_t image, int4 coord, int lod, float color);

#ifdef cl_khr_3d_image_writes
void __ovld write_imagef(write_only image3d_t image, int4 coord, int lod, float4 color);
void __ovld write_imagei(write_only image3d_t image, int4 coord, int lod, int4 color);
void __ovld write_imageui(write_only image3d_t image, int4 coord, int lod, uint4 color);
#endif
#endif //cl_khr_mipmap_image
#endif // defined(__OPENCL_CPP_VERSION__) || (__OPENCL_C_VERSION__ >= CL_VERSION_2_0)

// OpenCL Extension v2.0 s9.18 - Mipmaps
#if defined(__OPENCL_CPP_VERSION__) || (__OPENCL_C_VERSION__ >= CL_VERSION_2_0)
#ifdef cl_khr_mipmap_image
/**
 * Return the image miplevels.
 */

int __ovld get_image_num_mip_levels(read_only image1d_t image);
int __ovld get_image_num_mip_levels(read_only image2d_t image);
int __ovld get_image_num_mip_levels(read_only image3d_t image);

int __ovld get_image_num_mip_levels(write_only image1d_t image);
int __ovld get_image_num_mip_levels(write_only image2d_t image);
#ifdef cl_khr_3d_image_writes
int __ovld get_image_num_mip_levels(write_only image3d_t image);
#endif

int __ovld get_image_num_mip_levels(read_only image1d_array_t image);
int __ovld get_image_num_mip_levels(read_only image2d_array_t image);
int __ovld get_image_num_mip_levels(read_only image2d_array_depth_t image);
int __ovld get_image_num_mip_levels(read_only image2d_depth_t image);

int __ovld get_image_num_mip_levels(write_only image1d_array_t image);
int __ovld get_image_num_mip_levels(write_only image2d_array_t image);
int __ovld get_image_num_mip_levels(write_only image2d_array_depth_t image);
int __ovld get_image_num_mip_levels(write_only image2d_depth_t image);

#endif //cl_khr_mipmap_image
#endif // defined(__OPENCL_CPP_VERSION__) || (__OPENCL_C_VERSION__ >= CL_VERSION_2_0)


#endif //_OPENCL_20_H_
