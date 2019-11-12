//==--- opencl_cth_pre_release.h - Pre-release extensions -*- C++ -*----------==//
////
//// Copyright (C) 2019 Intel Corporation. All rights reserved.
////
//// The information and source code contained herein is the exclusive property
//// of Intel Corporation and may not be disclosed, examined or reproduced in
//// whole or in part without explicit written authorization from the company.
////
//// ===--------------------------------------------------------------------=== //

#ifndef _OPENCL_CTH_
#define _OPENCL_CTH_

//
// Float atomics
//
#if defined (float_atomics_enable)

// atom_min
float __attribute__((overloadable)) atomic_min(volatile __global float *p, float val);
float __attribute__((overloadable)) atomic_min(volatile __local float *p, float val);

// atom_max
float __attribute__((overloadable)) atomic_max(volatile __global float *p, float val);
float __attribute__((overloadable)) atomic_max(volatile __local float *p, float val);

// atom_cmpxchg
float __attribute__((overloadable)) atomic_cmpxchg(volatile __global float *p, float cmp, float val);
float __attribute__((overloadable)) atomic_cmpxchg(volatile __local float *p, float cmp, float val);

#endif

#if defined(cl_intel_subgroups) || defined(cl_khr_subgroups)
// Shared Sub Group Functions
uint    __attribute__((overloadable)) get_sub_group_size( void );
uint    __attribute__((overloadable)) get_max_sub_group_size( void );
uint    __attribute__((overloadable)) get_num_sub_groups( void );
#if (__OPENCL_C_VERSION__ >= CL_VERSION_2_0)
uint    __attribute__((overloadable)) get_enqueued_num_sub_groups( void );
#endif
uint    __attribute__((overloadable)) get_sub_group_id( void );
uint    __attribute__((overloadable)) get_sub_group_local_id( void );

void    __attribute__((overloadable)) sub_group_barrier( cl_mem_fence_flags flags );
#if (__OPENCL_C_VERSION__ >= CL_VERSION_2_0)
void    __attribute__((overloadable)) sub_group_barrier( cl_mem_fence_flags flags, memory_scope scope );
#endif

int     __attribute__((overloadable)) sub_group_all( int predicate );
int     __attribute__((overloadable)) sub_group_any( int predicate );

int     __attribute__((overloadable)) sub_group_broadcast( int   x, uint sub_group_local_id );
uint    __attribute__((overloadable)) sub_group_broadcast( uint  x, uint sub_group_local_id );
long    __attribute__((overloadable)) sub_group_broadcast( long  x, uint sub_group_local_id );
ulong   __attribute__((overloadable)) sub_group_broadcast( ulong x, uint sub_group_local_id );
float   __attribute__((overloadable)) sub_group_broadcast( float x, uint sub_group_local_id );

int     __attribute__((overloadable)) sub_group_reduce_add( int   x );
uint    __attribute__((overloadable)) sub_group_reduce_add( uint  x );
long    __attribute__((overloadable)) sub_group_reduce_add( long  x );
ulong   __attribute__((overloadable)) sub_group_reduce_add( ulong x );
float   __attribute__((overloadable)) sub_group_reduce_add( float x );
int     __attribute__((overloadable)) sub_group_reduce_min( int   x );
uint    __attribute__((overloadable)) sub_group_reduce_min( uint  x );
long    __attribute__((overloadable)) sub_group_reduce_min( long  x );
ulong   __attribute__((overloadable)) sub_group_reduce_min( ulong x );
float   __attribute__((overloadable)) sub_group_reduce_min( float x );
int     __attribute__((overloadable)) sub_group_reduce_max( int   x );
uint    __attribute__((overloadable)) sub_group_reduce_max( uint  x );
long    __attribute__((overloadable)) sub_group_reduce_max( long  x );
ulong   __attribute__((overloadable)) sub_group_reduce_max( ulong x );
float   __attribute__((overloadable)) sub_group_reduce_max( float x );

int     __attribute__((overloadable)) sub_group_scan_exclusive_add( int   x );
uint    __attribute__((overloadable)) sub_group_scan_exclusive_add( uint  x );
long    __attribute__((overloadable)) sub_group_scan_exclusive_add( long  x );
ulong   __attribute__((overloadable)) sub_group_scan_exclusive_add( ulong x );
float   __attribute__((overloadable)) sub_group_scan_exclusive_add( float x );
int     __attribute__((overloadable)) sub_group_scan_exclusive_min( int   x );
uint    __attribute__((overloadable)) sub_group_scan_exclusive_min( uint  x );
long    __attribute__((overloadable)) sub_group_scan_exclusive_min( long  x );
ulong   __attribute__((overloadable)) sub_group_scan_exclusive_min( ulong x );
float   __attribute__((overloadable)) sub_group_scan_exclusive_min( float x );
int     __attribute__((overloadable)) sub_group_scan_exclusive_max( int   x );
uint    __attribute__((overloadable)) sub_group_scan_exclusive_max( uint  x );
long    __attribute__((overloadable)) sub_group_scan_exclusive_max( long  x );
ulong   __attribute__((overloadable)) sub_group_scan_exclusive_max( ulong x );
float   __attribute__((overloadable)) sub_group_scan_exclusive_max( float x );

int     __attribute__((overloadable)) sub_group_scan_inclusive_add( int   x );
uint    __attribute__((overloadable)) sub_group_scan_inclusive_add( uint  x );
long    __attribute__((overloadable)) sub_group_scan_inclusive_add( long  x );
ulong   __attribute__((overloadable)) sub_group_scan_inclusive_add( ulong x );
float   __attribute__((overloadable)) sub_group_scan_inclusive_add( float x );
int     __attribute__((overloadable)) sub_group_scan_inclusive_min( int   x );
uint    __attribute__((overloadable)) sub_group_scan_inclusive_min( uint  x );
long    __attribute__((overloadable)) sub_group_scan_inclusive_min( long  x );
ulong   __attribute__((overloadable)) sub_group_scan_inclusive_min( ulong x );
float   __attribute__((overloadable)) sub_group_scan_inclusive_min( float x );
int     __attribute__((overloadable)) sub_group_scan_inclusive_max( int   x );
uint    __attribute__((overloadable)) sub_group_scan_inclusive_max( uint  x );
long    __attribute__((overloadable)) sub_group_scan_inclusive_max( long  x );
ulong   __attribute__((overloadable)) sub_group_scan_inclusive_max( ulong x );
float   __attribute__((overloadable)) sub_group_scan_inclusive_max( float x );

#if defined(cl_khr_fp64)
double  __attribute__((overloadable)) sub_group_broadcast( double x, uint sub_group_local_id );
double  __attribute__((overloadable)) sub_group_reduce_add( double x );
double  __attribute__((overloadable)) sub_group_reduce_min( double x );
double  __attribute__((overloadable)) sub_group_reduce_max( double x );
double  __attribute__((overloadable)) sub_group_scan_exclusive_add( double x );
double  __attribute__((overloadable)) sub_group_scan_exclusive_min( double x );
double  __attribute__((overloadable)) sub_group_scan_exclusive_max( double x );
double  __attribute__((overloadable)) sub_group_scan_inclusive_add( double x );
double  __attribute__((overloadable)) sub_group_scan_inclusive_min( double x );
double  __attribute__((overloadable)) sub_group_scan_inclusive_max( double x );
#endif
#endif

#if defined(cl_intel_subgroups)
// Intel Sub Group Functions
float   __attribute__((overloadable)) intel_sub_group_shuffle( float  x, uint c );
float2  __attribute__((overloadable)) intel_sub_group_shuffle( float2 x, uint c );
float3  __attribute__((overloadable)) intel_sub_group_shuffle( float3 x, uint c );
float4  __attribute__((overloadable)) intel_sub_group_shuffle( float4 x, uint c );
float8  __attribute__((overloadable)) intel_sub_group_shuffle( float8 x, uint c );
float16 __attribute__((overloadable)) intel_sub_group_shuffle( float16 x, uint c );

int     __attribute__((overloadable)) intel_sub_group_shuffle( int  x, uint c );
int2    __attribute__((overloadable)) intel_sub_group_shuffle( int2 x, uint c );
int3    __attribute__((overloadable)) intel_sub_group_shuffle( int3 x, uint c );
int4    __attribute__((overloadable)) intel_sub_group_shuffle( int4 x, uint c );
int8    __attribute__((overloadable)) intel_sub_group_shuffle( int8 x, uint c );
int16   __attribute__((overloadable)) intel_sub_group_shuffle( int16 x, uint c );

uint    __attribute__((overloadable)) intel_sub_group_shuffle( uint  x, uint c );
uint2   __attribute__((overloadable)) intel_sub_group_shuffle( uint2 x, uint c );
uint3   __attribute__((overloadable)) intel_sub_group_shuffle( uint3 x, uint c );
uint4   __attribute__((overloadable)) intel_sub_group_shuffle( uint4 x, uint c );
uint8   __attribute__((overloadable)) intel_sub_group_shuffle( uint8 x, uint c );
uint16  __attribute__((overloadable)) intel_sub_group_shuffle( uint16 x, uint c );

long    __attribute__((overloadable)) intel_sub_group_shuffle( long x, uint c );
ulong   __attribute__((overloadable)) intel_sub_group_shuffle( ulong x, uint c );

float   __attribute__((overloadable)) intel_sub_group_shuffle_down( float  cur, float  next, uint c );
float2  __attribute__((overloadable)) intel_sub_group_shuffle_down( float2 cur, float2 next, uint c );
float3  __attribute__((overloadable)) intel_sub_group_shuffle_down( float3 cur, float3 next, uint c );
float4  __attribute__((overloadable)) intel_sub_group_shuffle_down( float4 cur, float4 next, uint c );
float8  __attribute__((overloadable)) intel_sub_group_shuffle_down( float8 cur, float8 next, uint c );
float16 __attribute__((overloadable)) intel_sub_group_shuffle_down( float16 cur, float16 next, uint c );

int     __attribute__((overloadable)) intel_sub_group_shuffle_down( int  cur, int  next, uint c );
int2    __attribute__((overloadable)) intel_sub_group_shuffle_down( int2 cur, int2 next, uint c );
int3    __attribute__((overloadable)) intel_sub_group_shuffle_down( int3 cur, int3 next, uint c );
int4    __attribute__((overloadable)) intel_sub_group_shuffle_down( int4 cur, int4 next, uint c );
int8    __attribute__((overloadable)) intel_sub_group_shuffle_down( int8 cur, int8 next, uint c );
int16   __attribute__((overloadable)) intel_sub_group_shuffle_down( int16 cur, int16 next, uint c );

uint    __attribute__((overloadable)) intel_sub_group_shuffle_down( uint  cur, uint  next, uint c );
uint2   __attribute__((overloadable)) intel_sub_group_shuffle_down( uint2 cur, uint2 next, uint c );
uint3   __attribute__((overloadable)) intel_sub_group_shuffle_down( uint3 cur, uint3 next, uint c );
uint4   __attribute__((overloadable)) intel_sub_group_shuffle_down( uint4 cur, uint4 next, uint c );
uint8   __attribute__((overloadable)) intel_sub_group_shuffle_down( uint8 cur, uint8 next, uint c );
uint16  __attribute__((overloadable)) intel_sub_group_shuffle_down( uint16 cur, uint16 next, uint c );

long    __attribute__((overloadable)) intel_sub_group_shuffle_down( long prev, long cur, uint c );
ulong   __attribute__((overloadable)) intel_sub_group_shuffle_down( ulong prev, ulong cur, uint c );

float   __attribute__((overloadable)) intel_sub_group_shuffle_up( float  prev, float  cur, uint c );
float2  __attribute__((overloadable)) intel_sub_group_shuffle_up( float2 prev, float2 cur, uint c );
float3  __attribute__((overloadable)) intel_sub_group_shuffle_up( float3 prev, float3 cur, uint c );
float4  __attribute__((overloadable)) intel_sub_group_shuffle_up( float4 prev, float4 cur, uint c );
float8  __attribute__((overloadable)) intel_sub_group_shuffle_up( float8 prev, float8 cur, uint c );
float16 __attribute__((overloadable)) intel_sub_group_shuffle_up( float16 prev, float16 cur, uint c );

int     __attribute__((overloadable)) intel_sub_group_shuffle_up( int  prev, int  cur, uint c );
int2    __attribute__((overloadable)) intel_sub_group_shuffle_up( int2 prev, int2 cur, uint c );
int3    __attribute__((overloadable)) intel_sub_group_shuffle_up( int3 prev, int3 cur, uint c );
int4    __attribute__((overloadable)) intel_sub_group_shuffle_up( int4 prev, int4 cur, uint c );
int8    __attribute__((overloadable)) intel_sub_group_shuffle_up( int8 prev, int8 cur, uint c );
int16   __attribute__((overloadable)) intel_sub_group_shuffle_up( int16 prev, int16 cur, uint c );

uint    __attribute__((overloadable)) intel_sub_group_shuffle_up( uint  prev, uint  cur, uint c );
uint2   __attribute__((overloadable)) intel_sub_group_shuffle_up( uint2 prev, uint2 cur, uint c );
uint3   __attribute__((overloadable)) intel_sub_group_shuffle_up( uint3 prev, uint3 cur, uint c );
uint4   __attribute__((overloadable)) intel_sub_group_shuffle_up( uint4 prev, uint4 cur, uint c );
uint8   __attribute__((overloadable)) intel_sub_group_shuffle_up( uint8 prev, uint8 cur, uint c );
uint16  __attribute__((overloadable)) intel_sub_group_shuffle_up( uint16 prev, uint16 cur, uint c );

long    __attribute__((overloadable)) intel_sub_group_shuffle_up( long prev, long cur, uint c );
ulong   __attribute__((overloadable)) intel_sub_group_shuffle_up( ulong prev, ulong cur, uint c );

float   __attribute__((overloadable)) intel_sub_group_shuffle_xor( float  x, uint c );
float2  __attribute__((overloadable)) intel_sub_group_shuffle_xor( float2 x, uint c );
float3  __attribute__((overloadable)) intel_sub_group_shuffle_xor( float3 x, uint c );
float4  __attribute__((overloadable)) intel_sub_group_shuffle_xor( float4 x, uint c );
float8  __attribute__((overloadable)) intel_sub_group_shuffle_xor( float8 x, uint c );
float16 __attribute__((overloadable)) intel_sub_group_shuffle_xor( float16 x, uint c );

int     __attribute__((overloadable)) intel_sub_group_shuffle_xor( int  x, uint c );
int2    __attribute__((overloadable)) intel_sub_group_shuffle_xor( int2 x, uint c );
int3    __attribute__((overloadable)) intel_sub_group_shuffle_xor( int3 x, uint c );
int4    __attribute__((overloadable)) intel_sub_group_shuffle_xor( int4 x, uint c );
int8    __attribute__((overloadable)) intel_sub_group_shuffle_xor( int8 x, uint c );
int16   __attribute__((overloadable)) intel_sub_group_shuffle_xor( int16 x, uint c );

uint    __attribute__((overloadable)) intel_sub_group_shuffle_xor( uint  x, uint c );
uint2   __attribute__((overloadable)) intel_sub_group_shuffle_xor( uint2 x, uint c );
uint3   __attribute__((overloadable)) intel_sub_group_shuffle_xor( uint3 x, uint c );
uint4   __attribute__((overloadable)) intel_sub_group_shuffle_xor( uint4 x, uint c );
uint8   __attribute__((overloadable)) intel_sub_group_shuffle_xor( uint8 x, uint c );
uint16  __attribute__((overloadable)) intel_sub_group_shuffle_xor( uint16 x, uint c );

long    __attribute__((overloadable)) intel_sub_group_shuffle_xor( long x, uint c );
ulong   __attribute__((overloadable)) intel_sub_group_shuffle_xor( ulong x, uint c );

uint    __attribute__((overloadable)) intel_sub_group_block_read( read_only image2d_t image, int2 coord );
uint2   __attribute__((overloadable)) intel_sub_group_block_read2( read_only image2d_t image, int2 coord );
uint4   __attribute__((overloadable)) intel_sub_group_block_read4( read_only image2d_t image, int2 coord );
uint8   __attribute__((overloadable)) intel_sub_group_block_read8( read_only image2d_t image, int2 coord );

uint    __attribute__((overloadable)) intel_sub_group_block_read( const __global uint* p );
uint2   __attribute__((overloadable)) intel_sub_group_block_read2( const __global uint* p );
uint4   __attribute__((overloadable)) intel_sub_group_block_read4( const __global uint* p );
uint8   __attribute__((overloadable)) intel_sub_group_block_read8( const __global uint* p );

// ui
uint    __attribute__((overloadable)) intel_sub_group_block_read_ui( read_only image2d_t image, int2 coord );
uint2   __attribute__((overloadable)) intel_sub_group_block_read_ui2( read_only image2d_t image, int2 coord );
uint4   __attribute__((overloadable)) intel_sub_group_block_read_ui4( read_only image2d_t image, int2 coord );
uint8   __attribute__((overloadable)) intel_sub_group_block_read_ui8( read_only image2d_t image, int2 coord );

uint    __attribute__((overloadable)) intel_sub_group_block_read_ui( const __global uint* p );
uint2   __attribute__((overloadable)) intel_sub_group_block_read_ui2( const __global uint* p );
uint4   __attribute__((overloadable)) intel_sub_group_block_read_ui4( read_only image2d_t image, int2 coord );
uint8   __attribute__((overloadable)) intel_sub_group_block_read_ui8( read_only image2d_t image, int2 coord );

void    __attribute__((overloadable)) intel_sub_group_block_write_ui( image2d_t image, int2 coord, uint data );
void    __attribute__((overloadable)) intel_sub_group_block_write_ui2( image2d_t image, int2 coord, uint2 data );
void    __attribute__((overloadable)) intel_sub_group_block_write_ui4( image2d_t image, int2 coord, uint4 data );
void    __attribute__((overloadable)) intel_sub_group_block_write_ui8( image2d_t image, int2 coord, uint8 data );

void    __attribute__((overloadable)) intel_sub_group_block_write_ui(write_only image2d_t image, int2 coord, uint data);
void    __attribute__((overloadable)) intel_sub_group_block_write_ui2(write_only image2d_t image, int2 coord, uint2 data);
void    __attribute__((overloadable)) intel_sub_group_block_write_ui4(write_only image2d_t image, int2 coord, uint4 data);
void    __attribute__((overloadable)) intel_sub_group_block_write_ui8(write_only image2d_t image, int2 coord, uint8 data);

void    __attribute__((overloadable)) intel_sub_group_block_write( image2d_t image, int2 coord, uint data );
void    __attribute__((overloadable)) intel_sub_group_block_write2( image2d_t image, int2 coord, uint2 data );
void    __attribute__((overloadable)) intel_sub_group_block_write4( image2d_t image, int2 coord, uint4 data );
void    __attribute__((overloadable)) intel_sub_group_block_write8( image2d_t image, int2 coord, uint8 data );

void    __attribute__((overloadable)) intel_sub_group_block_write(write_only image2d_t image, int2 coord, uint data);
void    __attribute__((overloadable)) intel_sub_group_block_write2(write_only image2d_t image, int2 coord, uint2 data);
void    __attribute__((overloadable)) intel_sub_group_block_write4(write_only image2d_t image, int2 coord, uint4 data);
void    __attribute__((overloadable)) intel_sub_group_block_write8(write_only image2d_t image, int2 coord, uint8 data);

#if (__OPENCL_C_VERSION__ >= CL_VERSION_2_0)
uint    __attribute__((overloadable)) intel_sub_group_block_read(read_write image2d_t image, int2 coord);
uint2   __attribute__((overloadable)) intel_sub_group_block_read2(read_write image2d_t image, int2 coord);
uint4   __attribute__((overloadable)) intel_sub_group_block_read4(read_write image2d_t image, int2 coord);
uint8   __attribute__((overloadable)) intel_sub_group_block_read8(read_write image2d_t image, int2 coord);

void    __attribute__((overloadable)) intel_sub_group_block_write(read_write image2d_t image, int2 coord, uint data);
void    __attribute__((overloadable)) intel_sub_group_block_write2(read_write image2d_t image, int2 coord, uint2 data);
void    __attribute__((overloadable)) intel_sub_group_block_write4(read_write image2d_t image, int2 coord, uint4 data);
void    __attribute__((overloadable)) intel_sub_group_block_write8(read_write image2d_t image, int2 coord, uint8 data);
#endif // (__OPENCL_C_VERSION__ >= CL_VERSION_2_0)

void    __attribute__((overloadable)) intel_sub_group_block_write( __global uint* p, uint data );
void    __attribute__((overloadable)) intel_sub_group_block_write2( __global uint* p, uint2 data );
void    __attribute__((overloadable)) intel_sub_group_block_write4( __global uint* p, uint4 data );
void    __attribute__((overloadable)) intel_sub_group_block_write8( __global uint* p, uint8 data );

#ifdef cl_intel_subgroups_half
ushort   __attribute__((overloadable)) intel_sub_group_block_read_half(read_only image2d_t image, int2 coord);
ushort2  __attribute__((overloadable)) intel_sub_group_block_read2_half(read_only image2d_t image, int2 coord);
ushort4  __attribute__((overloadable)) intel_sub_group_block_read4_half(read_only image2d_t image, int2 coord);
ushort8  __attribute__((overloadable)) intel_sub_group_block_read8_half(read_only image2d_t image, int2 coord);
ushort16 __attribute__((overloadable)) intel_sub_group_block_read16_half(read_only image2d_t image, int2 coord);

#if (__OPENCL_C_VERSION__ >= CL_VERSION_2_0)
ushort   __attribute__((overloadable)) intel_sub_group_block_read_half(read_write image2d_t image, int2 coord);
ushort2  __attribute__((overloadable)) intel_sub_group_block_read2_half(read_write image2d_t image, int2 coord);
ushort4  __attribute__((overloadable)) intel_sub_group_block_read4_half(read_write image2d_t image, int2 coord);
ushort8  __attribute__((overloadable)) intel_sub_group_block_read8_half(read_write image2d_t image, int2 coord);
ushort16 __attribute__((overloadable)) intel_sub_group_block_read16_half(read_write image2d_t image, int2 coord);

void    __attribute__((overloadable)) intel_sub_group_block_write(read_write image2d_t image, int2 coord, ushort data);
void    __attribute__((overloadable)) intel_sub_group_block_write2(read_write image2d_t image, int2 coord, ushort2 data);
void    __attribute__((overloadable)) intel_sub_group_block_write4(read_write image2d_t image, int2 coord, ushort4 data);
void    __attribute__((overloadable)) intel_sub_group_block_write8(read_write image2d_t image, int2 coord, ushort8 data);
void    __attribute__((overloadable)) intel_sub_group_block_write16(read_write image2d_t image, int2 coord, ushort16 data);
#endif // (__OPENCL_C_VERSION__ >= CL_VERSION_2_0)

ushort   __attribute__((overloadable)) intel_sub_group_block_read(const __global ushort* p);
ushort2  __attribute__((overloadable)) intel_sub_group_block_read2(const __global ushort* p);
ushort4  __attribute__((overloadable)) intel_sub_group_block_read4(const __global ushort* p);
ushort8  __attribute__((overloadable)) intel_sub_group_block_read8(const __global ushort* p);
ushort16 __attribute__((overloadable)) intel_sub_group_block_read16(const __global ushort* p);

void    __attribute__((overloadable)) intel_sub_group_block_write(image2d_t image, int2 coord, ushort data);
void    __attribute__((overloadable)) intel_sub_group_block_write2(image2d_t image, int2 coord, ushort2 data);
void    __attribute__((overloadable)) intel_sub_group_block_write4(image2d_t image, int2 coord, ushort4 data);
void    __attribute__((overloadable)) intel_sub_group_block_write8(image2d_t image, int2 coord, ushort8 data);
void    __attribute__((overloadable)) intel_sub_group_block_write16(image2d_t image, int2 coord, ushort16 data);

void    __attribute__((overloadable)) intel_sub_group_block_write(__global ushort* p, ushort data);
void    __attribute__((overloadable)) intel_sub_group_block_write2(__global ushort* p, ushort2 data);
void    __attribute__((overloadable)) intel_sub_group_block_write4(__global ushort* p, ushort4 data);
void    __attribute__((overloadable)) intel_sub_group_block_write8(__global ushort* p, ushort8 data);
#endif // cl_intel_subgroups_half
#ifdef cl_intel_subgroups_short
short    __attribute__((overloadable)) intel_sub_group_broadcast( short  x, uint sub_group_local_id );
short2   __attribute__((overloadable)) intel_sub_group_broadcast( short2 x, uint sub_group_local_id );
short3   __attribute__((overloadable)) intel_sub_group_broadcast( short3 x, uint sub_group_local_id );
short4   __attribute__((overloadable)) intel_sub_group_broadcast( short4 x, uint sub_group_local_id );
short8   __attribute__((overloadable)) intel_sub_group_broadcast( short8 x, uint sub_group_local_id );

ushort   __attribute__((overloadable)) intel_sub_group_broadcast( ushort  x, uint sub_group_local_id );
ushort2  __attribute__((overloadable)) intel_sub_group_broadcast( ushort2 x, uint sub_group_local_id );
ushort3  __attribute__((overloadable)) intel_sub_group_broadcast( ushort3 x, uint sub_group_local_id );
ushort4  __attribute__((overloadable)) intel_sub_group_broadcast( ushort4 x, uint sub_group_local_id );
ushort8  __attribute__((overloadable)) intel_sub_group_broadcast( ushort8 x, uint sub_group_local_id );

short    __attribute__((overloadable)) intel_sub_group_shuffle( short   x, uint c );
short2   __attribute__((overloadable)) intel_sub_group_shuffle( short2  x, uint c );
short3   __attribute__((overloadable)) intel_sub_group_shuffle( short3  x, uint c );
short4   __attribute__((overloadable)) intel_sub_group_shuffle( short4  x, uint c );
short8   __attribute__((overloadable)) intel_sub_group_shuffle( short8  x, uint c );
short16  __attribute__((overloadable)) intel_sub_group_shuffle( short16 x, uint c);

ushort   __attribute__((overloadable)) intel_sub_group_shuffle( ushort   x, uint c );
ushort2  __attribute__((overloadable)) intel_sub_group_shuffle( ushort2  x, uint c );
ushort3  __attribute__((overloadable)) intel_sub_group_shuffle( ushort3  x, uint c );
ushort4  __attribute__((overloadable)) intel_sub_group_shuffle( ushort4  x, uint c );
ushort8  __attribute__((overloadable)) intel_sub_group_shuffle( ushort8  x, uint c );
ushort16 __attribute__((overloadable)) intel_sub_group_shuffle( ushort16 x, uint c );

short    __attribute__((overloadable)) intel_sub_group_shuffle_down( short   cur, short   next, uint c );
short2   __attribute__((overloadable)) intel_sub_group_shuffle_down( short2  cur, short2  next, uint c );
short3   __attribute__((overloadable)) intel_sub_group_shuffle_down( short3  cur, short3  next, uint c );
short4   __attribute__((overloadable)) intel_sub_group_shuffle_down( short4  cur, short4  next, uint c );
short8   __attribute__((overloadable)) intel_sub_group_shuffle_down( short8  cur, short8  next, uint c );
short16  __attribute__((overloadable)) intel_sub_group_shuffle_down( short16 cur, short16 next, uint c );

ushort   __attribute__((overloadable)) intel_sub_group_shuffle_down( ushort   cur, ushort   next, uint c );
ushort2  __attribute__((overloadable)) intel_sub_group_shuffle_down( ushort2  cur, ushort2  next, uint c );
ushort3  __attribute__((overloadable)) intel_sub_group_shuffle_down( ushort3  cur, ushort3  next, uint c );
ushort4  __attribute__((overloadable)) intel_sub_group_shuffle_down( ushort4  cur, ushort4  next, uint c );
ushort8  __attribute__((overloadable)) intel_sub_group_shuffle_down( ushort8  cur, ushort8  next, uint c );
ushort16 __attribute__((overloadable)) intel_sub_group_shuffle_down( ushort16 cur, ushort16 next, uint c );

short    __attribute__((overloadable)) intel_sub_group_shuffle_up( short   cur, short   next, uint c );
short2   __attribute__((overloadable)) intel_sub_group_shuffle_up( short2  cur, short2  next, uint c );
short3   __attribute__((overloadable)) intel_sub_group_shuffle_up( short3  cur, short3  next, uint c );
short4   __attribute__((overloadable)) intel_sub_group_shuffle_up( short4  cur, short4  next, uint c );
short8   __attribute__((overloadable)) intel_sub_group_shuffle_up( short8  cur, short8  next, uint c );
short16  __attribute__((overloadable)) intel_sub_group_shuffle_up( short16 cur, short16 next, uint c );

ushort   __attribute__((overloadable)) intel_sub_group_shuffle_up( ushort   cur, ushort   next, uint c );
ushort2  __attribute__((overloadable)) intel_sub_group_shuffle_up( ushort2  cur, ushort2  next, uint c );
ushort3  __attribute__((overloadable)) intel_sub_group_shuffle_up( ushort3  cur, ushort3  next, uint c );
ushort4  __attribute__((overloadable)) intel_sub_group_shuffle_up( ushort4  cur, ushort4  next, uint c );
ushort8  __attribute__((overloadable)) intel_sub_group_shuffle_up( ushort8  cur, ushort8  next, uint c );
ushort16 __attribute__((overloadable)) intel_sub_group_shuffle_up( ushort16 cur, ushort16 next, uint c );

short    __attribute__((overloadable)) intel_sub_group_shuffle_xor( short   x, uint c );
short2   __attribute__((overloadable)) intel_sub_group_shuffle_xor( short2  x, uint c );
short3   __attribute__((overloadable)) intel_sub_group_shuffle_xor( short3  x, uint c );
short4   __attribute__((overloadable)) intel_sub_group_shuffle_xor( short4  x, uint c );
short8   __attribute__((overloadable)) intel_sub_group_shuffle_xor( short8  x, uint c );
short16  __attribute__((overloadable)) intel_sub_group_shuffle_xor( short16 x, uint c );

ushort   __attribute__((overloadable)) intel_sub_group_shuffle_xor( ushort   x, uint c );
ushort2  __attribute__((overloadable)) intel_sub_group_shuffle_xor( ushort2  x, uint c );
ushort3  __attribute__((overloadable)) intel_sub_group_shuffle_xor( ushort3  x, uint c );
ushort4  __attribute__((overloadable)) intel_sub_group_shuffle_xor( ushort4  x, uint c );
ushort8  __attribute__((overloadable)) intel_sub_group_shuffle_xor( ushort8  x, uint c );
ushort16 __attribute__((overloadable)) intel_sub_group_shuffle_xor( ushort16 x, uint c );

short    __attribute__((overloadable)) intel_sub_group_reduce_add( short   x );
ushort   __attribute__((overloadable)) intel_sub_group_reduce_add( ushort  x );
short    __attribute__((overloadable)) intel_sub_group_reduce_min( short   x );
ushort   __attribute__((overloadable)) intel_sub_group_reduce_min( ushort  x );
short    __attribute__((overloadable)) intel_sub_group_reduce_max( short   x );
ushort   __attribute__((overloadable)) intel_sub_group_reduce_max( ushort  x );

short    __attribute__((overloadable)) intel_sub_group_scan_exclusive_add( short   x );
ushort   __attribute__((overloadable)) intel_sub_group_scan_exclusive_add( ushort  x );
short    __attribute__((overloadable)) intel_sub_group_scan_exclusive_min( short   x );
ushort   __attribute__((overloadable)) intel_sub_group_scan_exclusive_min( ushort  x );
short    __attribute__((overloadable)) intel_sub_group_scan_exclusive_max( short   x );
ushort   __attribute__((overloadable)) intel_sub_group_scan_exclusive_max( ushort  x );

short    __attribute__((overloadable)) intel_sub_group_scan_inclusive_add( short   x );
ushort   __attribute__((overloadable)) intel_sub_group_scan_inclusive_add( ushort  x );
short    __attribute__((overloadable)) intel_sub_group_scan_inclusive_min( short   x );
ushort   __attribute__((overloadable)) intel_sub_group_scan_inclusive_min( ushort  x );
short    __attribute__((overloadable)) intel_sub_group_scan_inclusive_max( short   x );
ushort   __attribute__((overloadable)) intel_sub_group_scan_inclusive_max( ushort  x );

ushort   __attribute__((overloadable)) intel_sub_group_block_read_us( read_only image2d_t image, int2 coord );
ushort2  __attribute__((overloadable)) intel_sub_group_block_read_us2( read_only image2d_t image, int2 coord );
ushort4  __attribute__((overloadable)) intel_sub_group_block_read_us4( read_only image2d_t image, int2 coord );
ushort8  __attribute__((overloadable)) intel_sub_group_block_read_us8( read_only image2d_t image, int2 coord );

#if (__OPENCL_C_VERSION__ >= CL_VERSION_2_0)
ushort   __attribute__((overloadable)) intel_sub_group_block_read_us(read_write image2d_t image, int2 coord);
ushort2  __attribute__((overloadable)) intel_sub_group_block_read_us2(read_write image2d_t image, int2 coord);
ushort4  __attribute__((overloadable)) intel_sub_group_block_read_us4(read_write image2d_t image, int2 coord);
ushort8  __attribute__((overloadable)) intel_sub_group_block_read_us8(read_write image2d_t image, int2 coord);

void    __attribute__((overloadable)) intel_sub_group_block_write_us(read_write image2d_t image, int2 coord, ushort  data);
void    __attribute__((overloadable)) intel_sub_group_block_write_us2(read_write image2d_t image, int2 coord, ushort2 data);
void    __attribute__((overloadable)) intel_sub_group_block_write_us4(read_write image2d_t image, int2 coord, ushort4 data);
void    __attribute__((overloadable)) intel_sub_group_block_write_us8(read_write image2d_t image, int2 coord, ushort8 data);
#endif // (__OPENCL_C_VERSION__ >= CL_VERSION_2_0)

ushort    __attribute__((overloadable)) intel_sub_group_block_read_us(  const __global ushort* p );
ushort2   __attribute__((overloadable)) intel_sub_group_block_read_us2( const __global ushort* p );
ushort4   __attribute__((overloadable)) intel_sub_group_block_read_us4( const __global ushort* p );
ushort8   __attribute__((overloadable)) intel_sub_group_block_read_us8( const __global ushort* p );

void    __attribute__((overloadable)) intel_sub_group_block_write_us(write_only image2d_t image, int2 coord, ushort  data);
void    __attribute__((overloadable)) intel_sub_group_block_write_us2(write_only image2d_t image, int2 coord, ushort2 data);
void    __attribute__((overloadable)) intel_sub_group_block_write_us4(write_only image2d_t image, int2 coord, ushort4 data);
void    __attribute__((overloadable)) intel_sub_group_block_write_us8(write_only image2d_t image, int2 coord, ushort8 data);

void    __attribute__((overloadable)) intel_sub_group_block_write_us(  __global ushort* p, ushort  data );
void    __attribute__((overloadable)) intel_sub_group_block_write_us2( __global ushort* p, ushort2 data );
void    __attribute__((overloadable)) intel_sub_group_block_write_us4( __global ushort* p, ushort4 data );
void    __attribute__((overloadable)) intel_sub_group_block_write_us8( __global ushort* p, ushort8 data );

#endif // cl_intel_subgroups_short
#ifdef cl_intel_subgroups_char
char    __attribute__((overloadable)) intel_sub_group_broadcast(char  x, uint sub_group_local_id);
char2   __attribute__((overloadable)) intel_sub_group_broadcast(char2 x, uint sub_group_local_id);
char3   __attribute__((overloadable)) intel_sub_group_broadcast(char3 x, uint sub_group_local_id);
char4   __attribute__((overloadable)) intel_sub_group_broadcast(char4 x, uint sub_group_local_id);
char8   __attribute__((overloadable)) intel_sub_group_broadcast(char8 x, uint sub_group_local_id);

uchar   __attribute__((overloadable)) intel_sub_group_broadcast(uchar  x, uint sub_group_local_id);
uchar2  __attribute__((overloadable)) intel_sub_group_broadcast(uchar2 x, uint sub_group_local_id);
uchar3  __attribute__((overloadable)) intel_sub_group_broadcast(uchar3 x, uint sub_group_local_id);
uchar4  __attribute__((overloadable)) intel_sub_group_broadcast(uchar4 x, uint sub_group_local_id);
uchar8  __attribute__((overloadable)) intel_sub_group_broadcast(uchar8 x, uint sub_group_local_id);

char    __attribute__((overloadable)) intel_sub_group_shuffle(char   x, uint c);
char2   __attribute__((overloadable)) intel_sub_group_shuffle(char2  x, uint c);
char3   __attribute__((overloadable)) intel_sub_group_shuffle(char3  x, uint c);
char4   __attribute__((overloadable)) intel_sub_group_shuffle(char4  x, uint c);
char8   __attribute__((overloadable)) intel_sub_group_shuffle(char8  x, uint c);
char16  __attribute__((overloadable)) intel_sub_group_shuffle(char16 x, uint c);

uchar   __attribute__((overloadable)) intel_sub_group_shuffle(uchar   x, uint c);
uchar2  __attribute__((overloadable)) intel_sub_group_shuffle(uchar2  x, uint c);
uchar3  __attribute__((overloadable)) intel_sub_group_shuffle(uchar3  x, uint c);
uchar4  __attribute__((overloadable)) intel_sub_group_shuffle(uchar4  x, uint c);
uchar8  __attribute__((overloadable)) intel_sub_group_shuffle(uchar8  x, uint c);
uchar16 __attribute__((overloadable)) intel_sub_group_shuffle(uchar16 x, uint c);

char    __attribute__((overloadable)) intel_sub_group_shuffle_down(char   cur, char   next, uint c);
char2   __attribute__((overloadable)) intel_sub_group_shuffle_down(char2  cur, char2  next, uint c);
char3   __attribute__((overloadable)) intel_sub_group_shuffle_down(char3  cur, char3  next, uint c);
char4   __attribute__((overloadable)) intel_sub_group_shuffle_down(char4  cur, char4  next, uint c);
char8   __attribute__((overloadable)) intel_sub_group_shuffle_down(char8  cur, char8  next, uint c);
char16  __attribute__((overloadable)) intel_sub_group_shuffle_down(char16 cur, char16 next, uint c);

uchar   __attribute__((overloadable)) intel_sub_group_shuffle_down(uchar   cur, uchar   next, uint c);
uchar2  __attribute__((overloadable)) intel_sub_group_shuffle_down(uchar2  cur, uchar2  next, uint c);
uchar3  __attribute__((overloadable)) intel_sub_group_shuffle_down(uchar3  cur, uchar3  next, uint c);
uchar4  __attribute__((overloadable)) intel_sub_group_shuffle_down(uchar4  cur, uchar4  next, uint c);
uchar8  __attribute__((overloadable)) intel_sub_group_shuffle_down(uchar8  cur, uchar8  next, uint c);
uchar16 __attribute__((overloadable)) intel_sub_group_shuffle_down(uchar16 cur, uchar16 next, uint c);

char    __attribute__((overloadable)) intel_sub_group_shuffle_up(char   cur, char   next, uint c);
char2   __attribute__((overloadable)) intel_sub_group_shuffle_up(char2  cur, char2  next, uint c);
char3   __attribute__((overloadable)) intel_sub_group_shuffle_up(char3  cur, char3  next, uint c);
char4   __attribute__((overloadable)) intel_sub_group_shuffle_up(char4  cur, char4  next, uint c);
char8   __attribute__((overloadable)) intel_sub_group_shuffle_up(char8  cur, char8  next, uint c);
char16  __attribute__((overloadable)) intel_sub_group_shuffle_up(char16 cur, char16 next, uint c);

uchar   __attribute__((overloadable)) intel_sub_group_shuffle_up(uchar   cur, uchar   next, uint c);
uchar2  __attribute__((overloadable)) intel_sub_group_shuffle_up(uchar2  cur, uchar2  next, uint c);
uchar3  __attribute__((overloadable)) intel_sub_group_shuffle_up(uchar3  cur, uchar3  next, uint c);
uchar4  __attribute__((overloadable)) intel_sub_group_shuffle_up(uchar4  cur, uchar4  next, uint c);
uchar8  __attribute__((overloadable)) intel_sub_group_shuffle_up(uchar8  cur, uchar8  next, uint c);
uchar16 __attribute__((overloadable)) intel_sub_group_shuffle_up(uchar16 cur, uchar16 next, uint c);

char    __attribute__((overloadable)) intel_sub_group_shuffle_xor(char   x, uint c);
char2   __attribute__((overloadable)) intel_sub_group_shuffle_xor(char2  x, uint c);
char3   __attribute__((overloadable)) intel_sub_group_shuffle_xor(char3  x, uint c);
char4   __attribute__((overloadable)) intel_sub_group_shuffle_xor(char4  x, uint c);
char8   __attribute__((overloadable)) intel_sub_group_shuffle_xor(char8  x, uint c);
char16  __attribute__((overloadable)) intel_sub_group_shuffle_xor(char16 x, uint c);

uchar   __attribute__((overloadable)) intel_sub_group_shuffle_xor(uchar   x, uint c);
uchar2  __attribute__((overloadable)) intel_sub_group_shuffle_xor(uchar2  x, uint c);
uchar3  __attribute__((overloadable)) intel_sub_group_shuffle_xor(uchar3  x, uint c);
uchar4  __attribute__((overloadable)) intel_sub_group_shuffle_xor(uchar4  x, uint c);
uchar8  __attribute__((overloadable)) intel_sub_group_shuffle_xor(uchar8  x, uint c);
uchar16 __attribute__((overloadable)) intel_sub_group_shuffle_xor(uchar16 x, uint c);

char    __attribute__((overloadable)) intel_sub_group_reduce_add(char   x);
uchar   __attribute__((overloadable)) intel_sub_group_reduce_add(uchar  x);
char    __attribute__((overloadable)) intel_sub_group_reduce_min(char   x);
uchar   __attribute__((overloadable)) intel_sub_group_reduce_min(uchar  x);
char    __attribute__((overloadable)) intel_sub_group_reduce_max(char   x);
uchar   __attribute__((overloadable)) intel_sub_group_reduce_max(uchar  x);

char    __attribute__((overloadable)) intel_sub_group_scan_exclusive_add(char   x);
uchar   __attribute__((overloadable)) intel_sub_group_scan_exclusive_add(uchar  x);
char    __attribute__((overloadable)) intel_sub_group_scan_exclusive_min(char   x);
uchar   __attribute__((overloadable)) intel_sub_group_scan_exclusive_min(uchar  x);
char    __attribute__((overloadable)) intel_sub_group_scan_exclusive_max(char   x);
uchar   __attribute__((overloadable)) intel_sub_group_scan_exclusive_max(uchar  x);

char    __attribute__((overloadable)) intel_sub_group_scan_inclusive_add(char   x);
uchar   __attribute__((overloadable)) intel_sub_group_scan_inclusive_add(uchar  x);
char    __attribute__((overloadable)) intel_sub_group_scan_inclusive_min(char   x);
uchar   __attribute__((overloadable)) intel_sub_group_scan_inclusive_min(uchar  x);
char    __attribute__((overloadable)) intel_sub_group_scan_inclusive_max(char   x);
uchar   __attribute__((overloadable)) intel_sub_group_scan_inclusive_max(uchar  x);

uchar   __attribute__((overloadable)) intel_sub_group_block_read_uc(read_only image2d_t image, int2 coord);
uchar2  __attribute__((overloadable)) intel_sub_group_block_read_uc2(read_only image2d_t image, int2 coord);
uchar4  __attribute__((overloadable)) intel_sub_group_block_read_uc4(read_only image2d_t image, int2 coord);
uchar8  __attribute__((overloadable)) intel_sub_group_block_read_uc8(read_only image2d_t image, int2 coord);

#if (__OPENCL_C_VERSION__ >= CL_VERSION_2_0)
uchar   __attribute__((overloadable)) intel_sub_group_block_read_uc(read_write image2d_t image, int2 coord);
uchar2  __attribute__((overloadable)) intel_sub_group_block_read_uc2(read_write image2d_t image, int2 coord);
uchar4  __attribute__((overloadable)) intel_sub_group_block_read_uc4(read_write image2d_t image, int2 coord);
uchar8  __attribute__((overloadable)) intel_sub_group_block_read_uc8(read_write image2d_t image, int2 coord);

void    __attribute__((overloadable)) intel_sub_group_block_write_uc(read_write image2d_t image, int2 coord, uchar  data);
void    __attribute__((overloadable)) intel_sub_group_block_write_uc2(read_write image2d_t image, int2 coord, uchar2 data);
void    __attribute__((overloadable)) intel_sub_group_block_write_uc4(read_write image2d_t image, int2 coord, uchar4 data);
void    __attribute__((overloadable)) intel_sub_group_block_write_uc8(read_write image2d_t image, int2 coord, uchar8 data);
#endif // (__OPENCL_C_VERSION__ >= CL_VERSION_2_0)

uchar    __attribute__((overloadable)) intel_sub_group_block_read_uc(const __global uchar* p);
uchar2   __attribute__((overloadable)) intel_sub_group_block_read_uc2(const __global uchar* p);
uchar4   __attribute__((overloadable)) intel_sub_group_block_read_uc4(const __global uchar* p);
uchar8   __attribute__((overloadable)) intel_sub_group_block_read_uc8(const __global uchar* p);

void    __attribute__((overloadable)) intel_sub_group_block_write_uc(write_only image2d_t image, int2 coord, uchar  data);
void    __attribute__((overloadable)) intel_sub_group_block_write_uc2(write_only image2d_t image, int2 coord, uchar2 data);
void    __attribute__((overloadable)) intel_sub_group_block_write_uc4(write_only image2d_t image, int2 coord, uchar4 data);
void    __attribute__((overloadable)) intel_sub_group_block_write_uc8(write_only image2d_t image, int2 coord, uchar8 data);

void    __attribute__((overloadable)) intel_sub_group_block_write_uc(__global uchar* p, uchar  data);
void    __attribute__((overloadable)) intel_sub_group_block_write_uc2(__global uchar* p, uchar2 data);
void    __attribute__((overloadable)) intel_sub_group_block_write_uc4(__global uchar* p, uchar4 data);
void    __attribute__((overloadable)) intel_sub_group_block_write_uc8(__global uchar* p, uchar8 data);

#endif // cl_intel_subgroups_char


#ifdef cl_intel_subgroups_long
ulong   __attribute__((overloadable)) intel_sub_group_block_read_ul(read_only image2d_t image, int2 coord);
ulong2  __attribute__((overloadable)) intel_sub_group_block_read_ul2(read_only image2d_t image, int2 coord);
ulong4  __attribute__((overloadable)) intel_sub_group_block_read_ul4(read_only image2d_t image, int2 coord);
ulong8  __attribute__((overloadable)) intel_sub_group_block_read_ul8(read_only image2d_t image, int2 coord);

long    __attribute__((overloadable)) intel_sub_group_shuffle(long   x, uint c);
long2   __attribute__((overloadable)) intel_sub_group_shuffle(long2  x, uint c);
long3   __attribute__((overloadable)) intel_sub_group_shuffle(long3  x, uint c);
long4   __attribute__((overloadable)) intel_sub_group_shuffle(long4  x, uint c);
long8   __attribute__((overloadable)) intel_sub_group_shuffle(long8  x, uint c);
long16  __attribute__((overloadable)) intel_sub_group_shuffle(long16 x, uint c);

ulong   __attribute__((overloadable)) intel_sub_group_shuffle(ulong   x, uint c);
ulong2  __attribute__((overloadable)) intel_sub_group_shuffle(ulong2  x, uint c);
ulong3  __attribute__((overloadable)) intel_sub_group_shuffle(ulong3  x, uint c);
ulong4  __attribute__((overloadable)) intel_sub_group_shuffle(ulong4  x, uint c);
ulong8  __attribute__((overloadable)) intel_sub_group_shuffle(ulong8  x, uint c);
ulong16 __attribute__((overloadable)) intel_sub_group_shuffle(ulong16 x, uint c);

long    __attribute__((overloadable)) intel_sub_group_shuffle_down(long   cur, long   next, uint c);
long2   __attribute__((overloadable)) intel_sub_group_shuffle_down(long2  cur, long2  next, uint c);
long3   __attribute__((overloadable)) intel_sub_group_shuffle_down(long3  cur, long3  next, uint c);
long4   __attribute__((overloadable)) intel_sub_group_shuffle_down(long4  cur, long4  next, uint c);
long8   __attribute__((overloadable)) intel_sub_group_shuffle_down(long8  cur, long8  next, uint c);
long16  __attribute__((overloadable)) intel_sub_group_shuffle_down(long16 cur, long16 next, uint c);

ulong   __attribute__((overloadable)) intel_sub_group_shuffle_down(ulong   cur, ulong   next, uint c);
ulong2  __attribute__((overloadable)) intel_sub_group_shuffle_down(ulong2  cur, ulong2  next, uint c);
ulong3  __attribute__((overloadable)) intel_sub_group_shuffle_down(ulong3  cur, ulong3  next, uint c);
ulong4  __attribute__((overloadable)) intel_sub_group_shuffle_down(ulong4  cur, ulong4  next, uint c);
ulong8  __attribute__((overloadable)) intel_sub_group_shuffle_down(ulong8  cur, ulong8  next, uint c);
ulong16 __attribute__((overloadable)) intel_sub_group_shuffle_down(ulong16 cur, ulong16 next, uint c);

long    __attribute__((overloadable)) intel_sub_group_shuffle_up(long   cur, long   next, uint c);
long2   __attribute__((overloadable)) intel_sub_group_shuffle_up(long2  cur, long2  next, uint c);
long3   __attribute__((overloadable)) intel_sub_group_shuffle_up(long3  cur, long3  next, uint c);
long4   __attribute__((overloadable)) intel_sub_group_shuffle_up(long4  cur, long4  next, uint c);
long8   __attribute__((overloadable)) intel_sub_group_shuffle_up(long8  cur, long8  next, uint c);
long16  __attribute__((overloadable)) intel_sub_group_shuffle_up(long16 cur, long16 next, uint c);

ulong   __attribute__((overloadable)) intel_sub_group_shuffle_up(ulong   cur, ulong   next, uint c);
ulong2  __attribute__((overloadable)) intel_sub_group_shuffle_up(ulong2  cur, ulong2  next, uint c);
ulong3  __attribute__((overloadable)) intel_sub_group_shuffle_up(ulong3  cur, ulong3  next, uint c);
ulong4  __attribute__((overloadable)) intel_sub_group_shuffle_up(ulong4  cur, ulong4  next, uint c);
ulong8  __attribute__((overloadable)) intel_sub_group_shuffle_up(ulong8  cur, ulong8  next, uint c);
ulong16 __attribute__((overloadable)) intel_sub_group_shuffle_up(ulong16 cur, ulong16 next, uint c);

long    __attribute__((overloadable)) intel_sub_group_shuffle_xor(long   x, uint c);
long2   __attribute__((overloadable)) intel_sub_group_shuffle_xor(long2  x, uint c);
long3   __attribute__((overloadable)) intel_sub_group_shuffle_xor(long3  x, uint c);
long4   __attribute__((overloadable)) intel_sub_group_shuffle_xor(long4  x, uint c);
long8   __attribute__((overloadable)) intel_sub_group_shuffle_xor(long8  x, uint c);
long16  __attribute__((overloadable)) intel_sub_group_shuffle_xor(long16 x, uint c);

ulong   __attribute__((overloadable)) intel_sub_group_shuffle_xor(ulong   x, uint c);
ulong2  __attribute__((overloadable)) intel_sub_group_shuffle_xor(ulong2  x, uint c);
ulong3  __attribute__((overloadable)) intel_sub_group_shuffle_xor(ulong3  x, uint c);
ulong4  __attribute__((overloadable)) intel_sub_group_shuffle_xor(ulong4  x, uint c);
ulong8  __attribute__((overloadable)) intel_sub_group_shuffle_xor(ulong8  x, uint c);
ulong16 __attribute__((overloadable)) intel_sub_group_shuffle_xor(ulong16 x, uint c);

#if (__OPENCL_C_VERSION__ >= CL_VERSION_2_0)
ulong   __attribute__((overloadable)) intel_sub_group_block_read_ul(read_write image2d_t image, int2 coord);
ulong2  __attribute__((overloadable)) intel_sub_group_block_read_ul2(read_write image2d_t image, int2 coord);
ulong4  __attribute__((overloadable)) intel_sub_group_block_read_ul4(read_write image2d_t image, int2 coord);
ulong8  __attribute__((overloadable)) intel_sub_group_block_read_ul8(read_write image2d_t image, int2 coord);

void    __attribute__((overloadable)) intel_sub_group_block_write_ul(read_write image2d_t image, int2 coord, ulong  data);
void    __attribute__((overloadable)) intel_sub_group_block_write_ul2(read_write image2d_t image, int2 coord, ulong2 data);
void    __attribute__((overloadable)) intel_sub_group_block_write_ul4(read_write image2d_t image, int2 coord, ulong4 data);
void    __attribute__((overloadable)) intel_sub_group_block_write_ul8(read_write image2d_t image, int2 coord, ulong8 data);
#endif // (__OPENCL_C_VERSION__ >= CL_VERSION_2_0)

ulong    __attribute__((overloadable)) intel_sub_group_block_read_ul(const __global ulong* p);
ulong2   __attribute__((overloadable)) intel_sub_group_block_read_ul2(const __global ulong* p);
ulong4   __attribute__((overloadable)) intel_sub_group_block_read_ul4(const __global ulong* p);
ulong8   __attribute__((overloadable)) intel_sub_group_block_read_ul8(const __global ulong* p);

void    __attribute__((overloadable)) intel_sub_group_block_write_ul(write_only image2d_t image, int2 coord, ulong  data);
void    __attribute__((overloadable)) intel_sub_group_block_write_ul2(write_only image2d_t image, int2 coord, ulong2 data);
void    __attribute__((overloadable)) intel_sub_group_block_write_ul4(write_only image2d_t image, int2 coord, ulong4 data);
void    __attribute__((overloadable)) intel_sub_group_block_write_ul8(write_only image2d_t image, int2 coord, ulong8 data);

void    __attribute__((overloadable)) intel_sub_group_block_write_ul(__global ulong* p, ulong  data);
void    __attribute__((overloadable)) intel_sub_group_block_write_ul2(__global ulong* p, ulong2 data);
void    __attribute__((overloadable)) intel_sub_group_block_write_ul4(__global ulong* p, ulong4 data);
void    __attribute__((overloadable)) intel_sub_group_block_write_ul8(__global ulong* p, ulong8 data);

#endif // cl_intel_subgroups_long

#if defined(cl_khr_fp64)
double  __attribute__((overloadable)) intel_sub_group_shuffle( double x, uint c );
double  __attribute__((overloadable)) intel_sub_group_shuffle_down( double prev, double cur, uint c );
double  __attribute__((overloadable)) intel_sub_group_shuffle_up( double prev, double cur, uint c );
double  __attribute__((overloadable)) intel_sub_group_shuffle_xor( double x, uint c );
#endif

#endif // defined(cl_intel_subgroups)

#endif // #ifndef _OPENCL_CTH_

