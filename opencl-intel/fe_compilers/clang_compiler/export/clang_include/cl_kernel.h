/*******************************************************************************
 * Copyright:  (c) 2007-2008 by Apple, Inc., All Rights Reserved.
 ******************************************************************************/

#ifndef __CL_KERNEL_H
#define __CL_KERNEL_H

// Macro machinery for C-based type overloading of builtin functions
#define __CLFN_F1(x,R) __builtin_overload(1, x, __##R##f, __##R##f2, __##R##f4, __##R##f8, __##R##f16, R, __##R##d2, __##R##d4, __##R##d8, __##R##d16 )
#define __CLFN_F11(x,R) __builtin_overload(1, x, __##R##f1, __##R##f2, __##R##f4, __##R##f8, __##R##f16, R, __##R##d2, __##R##d4, __##R##d8, __##R##d16 )
#define __CLFN_F2(x,y,R) __builtin_overload(2, x, y, __##R##f, __##R##f2, __##R##f4, __##R##f8, __##R##f16, R, __##R##d2, __##R##d4, __##R##d8, __##R##d16)
#define __CLFN_F21(x,y,R) __builtin_overload(2, x, y, __##R##f1, __##R##f2, __##R##f4, __##R##f8, __##R##f16, R, __##R##d2, __##R##d4, __##R##d8, __##R##d16)
#define __CLFN_F3(x,y,z,R) __builtin_overload(3, x, y, z, __##R##f, __##R##f2, __##R##f4, __##R##f8, __##R##f16, R, __##R##d2, __##R##d4, __##R##d8, __##R##d16)
#define __CLFN_I1(x,R) __builtin_overload(1, x, \
                                          __##R##_1i8, __##R##_2i8, __##R##_4i8, __##R##_8i8, __##R##_16i8, \
                                          __##R##_1u8, __##R##_2u8, __##R##_4u8, __##R##_8u8, __##R##_16u8, \
                                          __##R##_1i16, __##R##_2i16, __##R##_4i16, __##R##_8i16, __##R##_16i16, \
                                          __##R##_1u16, __##R##_2u16, __##R##_4u16, __##R##_8u16, __##R##_16u16, \
                                          __##R##_1i32, __##R##_2i32, __##R##_4i32, __##R##_8i32, __##R##_16i32, \
                                          __##R##_1u32, __##R##_2u32, __##R##_4u32, __##R##_8u32, __##R##_16u32, \
                                          __##R##_1i64, __##R##_2i64, __##R##_4i64, __##R##_8i64, __##R##_16i64, \
                                          __##R##_1u64, __##R##_2u64, __##R##_4u64, __##R##_8u64, __##R##_16u64) 
#define __CLFN_I2(x,y,R) __builtin_overload(2, x, y,\
                                          __##R##_1i8, __##R##_2i8, __##R##_4i8, __##R##_8i8, __##R##_16i8, \
                                          __##R##_1u8, __##R##_2u8, __##R##_4u8, __##R##_8u8, __##R##_16u8, \
                                          __##R##_1i16, __##R##_2i16, __##R##_4i16, __##R##_8i16, __##R##_16i16, \
                                          __##R##_1u16, __##R##_2u16, __##R##_4u16, __##R##_8u16, __##R##_16u16, \
                                          __##R##_1i32, __##R##_2i32, __##R##_4i32, __##R##_8i32, __##R##_16i32, \
                                          __##R##_1u32, __##R##_2u32, __##R##_4u32, __##R##_8u32, __##R##_16u32, \
                                          __##R##_1i64, __##R##_2i64, __##R##_4i64, __##R##_8i64, __##R##_16i64, \
                                          __##R##_1u64, __##R##_2u64, __##R##_4u64, __##R##_8u64, __##R##_16u64 )

#define __CLFN_I3(x,y,z,R) __builtin_overload(3, x, y, z,\
                                          __##R##_1i8, __##R##_2i8, __##R##_4i8, __##R##_8i8, __##R##_16i8, \
                                          __##R##_1u8, __##R##_2u8, __##R##_4u8, __##R##_8u8, __##R##_16u8, \
                                          __##R##_1i16, __##R##_2i16, __##R##_4i16, __##R##_8i16, __##R##_16i16, \
                                          __##R##_1u16, __##R##_2u16, __##R##_4u16, __##R##_8u16, __##R##_16u16, \
                                          __##R##_1i32, __##R##_2i32, __##R##_4i32, __##R##_8i32, __##R##_16i32, \
                                          __##R##_1u32, __##R##_2u32, __##R##_4u32, __##R##_8u32, __##R##_16u32, \
                                          __##R##_1i64, __##R##_2i64, __##R##_4i64, __##R##_8i64, __##R##_16i64, \
                                          __##R##_1u64, __##R##_2u64, __##R##_4u64, __##R##_8u64, __##R##_16u64 )
           
#define __CLFN_I2I2(x,y,R) __builtin_overload(2, x, y,\
                                          __##R##_1i8u8, __##R##_2i8u8, __##R##_4i8u8, __##R##_8i8u8, __##R##_16i8u8, \
                                          __##R##_1u8u8, __##R##_2u8u8, __##R##_4u8u8, __##R##_8u8u8, __##R##_16u8u8, \
                                          __##R##_1i16u16, __##R##_2i16u16, __##R##_4i16u16, __##R##_8i16u16, __##R##_16i16u16, \
                                          __##R##_1u16u16, __##R##_2u16u16, __##R##_4u16u16, __##R##_8u16u16, __##R##_16u16u16, \
                                          __##R##_1i32u32, __##R##_2i32u32, __##R##_4i32u32, __##R##_8i32u32, __##R##_16i32u32, \
                                          __##R##_1u32u32, __##R##_2u32u32, __##R##_4u32u32, __##R##_8u32u32, __##R##_16u32u32 )


// These macros are for the operations that support SPI types
#define __CLFN_F1_SPI(x,R) __builtin_overload(1, x, __##R##f, __##R##f2, __##R##f3, __##R##f4, __##R##f8, __##R##f16, \
                                              R, __##R##d2, __##R##d4, __##R##d8, __##R##d16 )
#define __CLFN_F2_SPI(x,y,R) __builtin_overload(2, x, y, __##R##f, __##R##f2, __##R##f3, __##R##f4, __##R##f8, __##R##f16, \
                                                R, __##R##d2, __##R##d4, __##R##d8, __##R##d16)
#define __CLFN_F1_SINGLE(x,R) __builtin_overload(1, x, __##R##f, __##R##f2, __##R##f3, __##R##f4, __##R##f8, __##R##f16 )
#define __CLFN_F2_SINGLE(x,y,R) __builtin_overload(2, x, y, __##R##f, __##R##f2, __##R##f3, __##R##f4, __##R##f8, __##R##f16 )


// 5.4 Variable Type Qualifiers
// private -> ptx .local, default
// global -> ptx .global
// constant -> ptx .constant
// local -> ptx .shared
#define __private
#define __global    __attribute__((address_space(1)))
#define __constant  __attribute__((address_space(2)))
#define __local     __attribute__((address_space(3)))

#define private
#define global    __attribute__((address_space(1)))
#define constant  __attribute__((address_space(2)))
#define local     __attribute__((address_space(3)))


// 5.1.2 OpenCL Vector Data Types
typedef __attribute__(( ext_vector_type(2) ))  char char2;
typedef __attribute__(( ext_vector_type(4) ))  char char4;
typedef __attribute__(( ext_vector_type(8) ))  char char8;
typedef __attribute__(( ext_vector_type(16) )) char char16;
typedef unsigned char uchar;
typedef __attribute__(( ext_vector_type(2) ))  unsigned char uchar2;
typedef __attribute__(( ext_vector_type(4) ))  unsigned char uchar4;
typedef __attribute__(( ext_vector_type(8) ))  unsigned char uchar8;
typedef __attribute__(( ext_vector_type(16) )) unsigned char uchar16;
typedef __attribute__(( ext_vector_type(2) ))  short short2;
typedef __attribute__(( ext_vector_type(4) ))  short short4;
typedef __attribute__(( ext_vector_type(8) ))  short short8;
typedef __attribute__(( ext_vector_type(16) )) short short16;
typedef unsigned short ushort;
typedef __attribute__(( ext_vector_type(2) ))  unsigned short ushort2;
typedef __attribute__(( ext_vector_type(4) ))  unsigned short ushort4;
typedef __attribute__(( ext_vector_type(8) ))  unsigned short ushort8;
typedef __attribute__(( ext_vector_type(16) )) unsigned short ushort16;
typedef __attribute__(( ext_vector_type(2) ))  int int2;
typedef __attribute__(( ext_vector_type(3) ))  int __int3_SPI;          //  This type is UNSUPPORTED!!!  It is for Apple internal use only. It will go away without warning and break your app.  PLEASE do not add a regular int3 type. 3-wide types are performace hostile.
typedef __attribute__(( ext_vector_type(4) ))  int int4;
typedef __attribute__(( ext_vector_type(8) ))  int int8;
typedef __attribute__(( ext_vector_type(16) )) int int16;
typedef unsigned int uint;
typedef __attribute__(( ext_vector_type(2) ))  unsigned int uint2;
typedef __attribute__(( ext_vector_type(4) ))  unsigned int uint4;
typedef __attribute__(( ext_vector_type(8) ))  unsigned int uint8;
typedef __attribute__(( ext_vector_type(16) )) unsigned int uint16;
typedef __attribute__(( ext_vector_type(2) ))  long long2;
typedef __attribute__(( ext_vector_type(4) ))  long long4;
typedef __attribute__(( ext_vector_type(8) ))  long long8;
typedef __attribute__(( ext_vector_type(16) )) long long16;
typedef unsigned long ulong;
typedef __attribute__(( ext_vector_type(2) ))  unsigned long ulong2;
typedef __attribute__(( ext_vector_type(4) ))  unsigned long ulong4;
typedef __attribute__(( ext_vector_type(8) ))  unsigned long ulong8;
typedef __attribute__(( ext_vector_type(16) )) unsigned long ulong16;
typedef __attribute__(( ext_vector_type(2) ))  float float2;
typedef __attribute__(( ext_vector_type(3) ))  float __float3_SPI;   //  This type is UNSUPPORTED!!!   It is for Apple internal use only. It will go away without warning and break your app.   PLEASE do not add a regular float3 type.  3-wide types are performace hostile.
typedef __attribute__(( ext_vector_type(4) ))  float float4;
typedef __attribute__(( ext_vector_type(8) ))  float float8;
typedef __attribute__(( ext_vector_type(16) )) float float16;
typedef __attribute__(( ext_vector_type(2) ))  double double2;
typedef __attribute__(( ext_vector_type(4) ))  double double4;
typedef __attribute__(( ext_vector_type(8) ))  double double8;
typedef __attribute__(( ext_vector_type(16) )) double double16;

// Half data type
typedef unsigned short half;

// Defend reserved types
typedef struct __Reserved_Name__Do_not_use_bool2        bool2;
typedef struct __Reserved_Name__Do_not_use_bool4        bool4;
typedef struct __Reserved_Name__Do_not_use_bool8        bool8;
typedef struct __Reserved_Name__Do_not_use_bool16       bool16;
typedef struct __Reserved_Name__Do_not_use_quad         quad;
typedef struct __Reserved_Name__Do_not_use_quad2        quad2;
typedef struct __Reserved_Name__Do_not_use_quad4        quad4;
typedef struct __Reserved_Name__Do_not_use_quad8        quad8;
typedef struct __Reserved_Name__Do_not_use_quad16       quad16;
typedef struct __Reserved_Name__Do_not_use_complex      complex;
typedef struct __Reserved_Name__Do_not_use_imaginary    imaginary;
typedef struct __Reserved_Name__Do_not_use_float2x2     float2x2;
typedef struct __Reserved_Name__Do_not_use_float2x3     float2x3;
typedef struct __Reserved_Name__Do_not_use_float3x2     float3x2;
typedef struct __Reserved_Name__Do_not_use_float2x4     float2x4;
typedef struct __Reserved_Name__Do_not_use_float4x2     float4x2;
typedef struct __Reserved_Name__Do_not_use_float3x4     float3x4;
typedef struct __Reserved_Name__Do_not_use_float4x3     float4x3;
typedef struct __Reserved_Name__Do_not_use_float4x4     float4x4;
typedef struct __Reserved_Name__Do_not_use_float8x8     float8x8;
typedef struct __Reserved_Name__Do_not_use_float16x16   float16x16;
typedef struct __Reserved_Name__Do_not_use_double2x2    double2x2;
typedef struct __Reserved_Name__Do_not_use_double2x3    double2x3;
typedef struct __Reserved_Name__Do_not_use_double3x2    double3x2;
typedef struct __Reserved_Name__Do_not_use_double2x4    double2x4;
typedef struct __Reserved_Name__Do_not_use_double4x2    double4x2;
typedef struct __Reserved_Name__Do_not_use_double3x4    double3x4;
typedef struct __Reserved_Name__Do_not_use_double4x3    double4x3;
typedef struct __Reserved_Name__Do_not_use_double4x4    double4x4;
typedef struct __Reserved_Name__Do_not_use_double8x8    double8x8;
typedef struct __Reserved_Name__Do_not_use_double16x16  double16x16;
typedef struct __Reserved_Name__Do_not_use_half2        half2;
typedef struct __Reserved_Name__Do_not_use_half4        half4;
typedef struct __Reserved_Name__Do_not_use_half8        half8;
typedef struct __Reserved_Name__Do_not_use_half16       half16;
typedef struct __Reserved_Name__Do_not_use_float3       float3;
typedef struct __Reserved_Name__Do_not_use_float5       float5;
typedef struct __Reserved_Name__Do_not_use_float6       float6;
typedef struct __Reserved_Name__Do_not_use_float7       float7;
typedef struct __Reserved_Name__Do_not_use_float9       float9;
typedef struct __Reserved_Name__Do_not_use_float10      float10;
typedef struct __Reserved_Name__Do_not_use_float11      float11;
typedef struct __Reserved_Name__Do_not_use_float12      float12;
typedef struct __Reserved_Name__Do_not_use_float13      float13;
typedef struct __Reserved_Name__Do_not_use_float14      float14;
typedef struct __Reserved_Name__Do_not_use_float15      float15;
typedef struct __Reserved_Name__Do_not_use_float32      float32;
typedef struct __Reserved_Name__Do_not_use_double3      double3;
typedef struct __Reserved_Name__Do_not_use_double5      double5;
typedef struct __Reserved_Name__Do_not_use_double6      double6;
typedef struct __Reserved_Name__Do_not_use_double7      double7;
typedef struct __Reserved_Name__Do_not_use_double9      double9;
typedef struct __Reserved_Name__Do_not_use_double10     double10;
typedef struct __Reserved_Name__Do_not_use_double11     double11;
typedef struct __Reserved_Name__Do_not_use_double12     double12;
typedef struct __Reserved_Name__Do_not_use_double13     double13;
typedef struct __Reserved_Name__Do_not_use_double14     double14;
typedef struct __Reserved_Name__Do_not_use_double15     double15;
typedef struct __Reserved_Name__Do_not_use_double32     double32;
typedef struct __Reserved_Name__Do_not_use_char3        char3;
typedef struct __Reserved_Name__Do_not_use_char5        char5;
typedef struct __Reserved_Name__Do_not_use_char6        char6;
typedef struct __Reserved_Name__Do_not_use_char7        char7;
typedef struct __Reserved_Name__Do_not_use_char9        char9;
typedef struct __Reserved_Name__Do_not_use_char10       char10;
typedef struct __Reserved_Name__Do_not_use_char11       char11;
typedef struct __Reserved_Name__Do_not_use_char12       char12;
typedef struct __Reserved_Name__Do_not_use_char13       char13;
typedef struct __Reserved_Name__Do_not_use_char14       char14;
typedef struct __Reserved_Name__Do_not_use_char15       char15;
typedef struct __Reserved_Name__Do_not_use_char32       char32;
typedef struct __Reserved_Name__Do_not_use_uchar3       uchar3;
typedef struct __Reserved_Name__Do_not_use_uchar5       uchar5;
typedef struct __Reserved_Name__Do_not_use_uchar6       uchar6;
typedef struct __Reserved_Name__Do_not_use_uchar7       uchar7;
typedef struct __Reserved_Name__Do_not_use_uchar9       uchar9;
typedef struct __Reserved_Name__Do_not_use_uchar10      uchar10;
typedef struct __Reserved_Name__Do_not_use_uchar11      uchar11;
typedef struct __Reserved_Name__Do_not_use_uchar12      uchar12;
typedef struct __Reserved_Name__Do_not_use_uchar13      uchar13;
typedef struct __Reserved_Name__Do_not_use_uchar14      uchar14;
typedef struct __Reserved_Name__Do_not_use_uchar15      uchar15;
typedef struct __Reserved_Name__Do_not_use_uchar32      uchar32;
typedef struct __Reserved_Name__Do_not_use_short3       short3;
typedef struct __Reserved_Name__Do_not_use_short5       short5;
typedef struct __Reserved_Name__Do_not_use_short6       short6;
typedef struct __Reserved_Name__Do_not_use_short7       short7;
typedef struct __Reserved_Name__Do_not_use_short9       short9;
typedef struct __Reserved_Name__Do_not_use_short10      short10;
typedef struct __Reserved_Name__Do_not_use_short11      short11;
typedef struct __Reserved_Name__Do_not_use_short12      short12;
typedef struct __Reserved_Name__Do_not_use_short13      short13;
typedef struct __Reserved_Name__Do_not_use_short14      short14;
typedef struct __Reserved_Name__Do_not_use_short15      short15;
typedef struct __Reserved_Name__Do_not_use_short32      short32;
typedef struct __Reserved_Name__Do_not_use_ushort3      ushort3;
typedef struct __Reserved_Name__Do_not_use_ushort5      ushort5;
typedef struct __Reserved_Name__Do_not_use_ushort6      ushort6;
typedef struct __Reserved_Name__Do_not_use_ushort7      ushort7;
typedef struct __Reserved_Name__Do_not_use_ushort9      ushort9;
typedef struct __Reserved_Name__Do_not_use_ushort10     ushort10;
typedef struct __Reserved_Name__Do_not_use_ushort11     ushort11;
typedef struct __Reserved_Name__Do_not_use_ushort12     ushort12;
typedef struct __Reserved_Name__Do_not_use_ushort13     ushort13;
typedef struct __Reserved_Name__Do_not_use_ushort14     ushort14;
typedef struct __Reserved_Name__Do_not_use_ushort15     ushort15;
typedef struct __Reserved_Name__Do_not_use_ushort32     ushort32;
typedef struct __Reserved_Name__Do_not_use_int3         int3;
typedef struct __Reserved_Name__Do_not_use_int5         int5;
typedef struct __Reserved_Name__Do_not_use_int6         int6;
typedef struct __Reserved_Name__Do_not_use_int7         int7;
typedef struct __Reserved_Name__Do_not_use_int9         int9;
typedef struct __Reserved_Name__Do_not_use_int10        int10;
typedef struct __Reserved_Name__Do_not_use_int11        int11;
typedef struct __Reserved_Name__Do_not_use_int12        int12;
typedef struct __Reserved_Name__Do_not_use_int13        int13;
typedef struct __Reserved_Name__Do_not_use_int14        int14;
typedef struct __Reserved_Name__Do_not_use_int15        int15;
typedef struct __Reserved_Name__Do_not_use_int32        int32;
typedef struct __Reserved_Name__Do_not_use_uint3        uint3;
typedef struct __Reserved_Name__Do_not_use_uint5        uint5;
typedef struct __Reserved_Name__Do_not_use_uint6        uint6;
typedef struct __Reserved_Name__Do_not_use_uint7        uint7;
typedef struct __Reserved_Name__Do_not_use_uint9        uint9;
typedef struct __Reserved_Name__Do_not_use_uint10       uint10;
typedef struct __Reserved_Name__Do_not_use_uint11       uint11;
typedef struct __Reserved_Name__Do_not_use_uint12       uint12;
typedef struct __Reserved_Name__Do_not_use_uint13       uint13;
typedef struct __Reserved_Name__Do_not_use_uint14       uint14;
typedef struct __Reserved_Name__Do_not_use_uint15       uint15;
typedef struct __Reserved_Name__Do_not_use_uint32       uint32;
typedef struct __Reserved_Name__Do_not_use_long3        long3;
typedef struct __Reserved_Name__Do_not_use_long5        long5;
typedef struct __Reserved_Name__Do_not_use_long6        long6;
typedef struct __Reserved_Name__Do_not_use_long7        long7;
typedef struct __Reserved_Name__Do_not_use_long9        long9;
typedef struct __Reserved_Name__Do_not_use_long10       long10;
typedef struct __Reserved_Name__Do_not_use_long11       long11;
typedef struct __Reserved_Name__Do_not_use_long12       long12;
typedef struct __Reserved_Name__Do_not_use_long13       long13;
typedef struct __Reserved_Name__Do_not_use_long14       long14;
typedef struct __Reserved_Name__Do_not_use_long15       long15;
typedef struct __Reserved_Name__Do_not_use_long32       long32;
typedef struct __Reserved_Name__Do_not_use_ulong3       ulong3;
typedef struct __Reserved_Name__Do_not_use_ulong5       ulong5;
typedef struct __Reserved_Name__Do_not_use_ulong6       ulong6;
typedef struct __Reserved_Name__Do_not_use_ulong7       ulong7;
typedef struct __Reserved_Name__Do_not_use_ulong9       ulong9;
typedef struct __Reserved_Name__Do_not_use_ulong10      ulong10;
typedef struct __Reserved_Name__Do_not_use_ulong11      ulong11;
typedef struct __Reserved_Name__Do_not_use_ulong12      ulong12;
typedef struct __Reserved_Name__Do_not_use_ulong13      ulong13;
typedef struct __Reserved_Name__Do_not_use_ulong14      ulong14;
typedef struct __Reserved_Name__Do_not_use_ulong15      ulong15;
typedef struct __Reserved_Name__Do_not_use_ulong32      ulong32;
typedef struct __Reserved_Name__Do_not_use_quad3        quad3;
typedef struct __Reserved_Name__Do_not_use_quad5        quad5;
typedef struct __Reserved_Name__Do_not_use_quad6        quad6;
typedef struct __Reserved_Name__Do_not_use_quad7        quad7;
typedef struct __Reserved_Name__Do_not_use_quad9        quad9;
typedef struct __Reserved_Name__Do_not_use_quad10       quad10;
typedef struct __Reserved_Name__Do_not_use_quad11       quad11;
typedef struct __Reserved_Name__Do_not_use_quad12       quad12;
typedef struct __Reserved_Name__Do_not_use_quad13       quad13;
typedef struct __Reserved_Name__Do_not_use_quad14       quad14;
typedef struct __Reserved_Name__Do_not_use_quad15       quad15;
typedef struct __Reserved_Name__Do_not_use_quad32       quad32;
typedef struct __Reserved_Name__Do_not_use_half3        half3;
typedef struct __Reserved_Name__Do_not_use_half5        half5;
typedef struct __Reserved_Name__Do_not_use_half6        half6;
typedef struct __Reserved_Name__Do_not_use_half7        half7;
typedef struct __Reserved_Name__Do_not_use_half9        half9;
typedef struct __Reserved_Name__Do_not_use_half10       half10;
typedef struct __Reserved_Name__Do_not_use_half11       half11;
typedef struct __Reserved_Name__Do_not_use_half12       half12;
typedef struct __Reserved_Name__Do_not_use_half13       half13;
typedef struct __Reserved_Name__Do_not_use_half14       half14;
typedef struct __Reserved_Name__Do_not_use_half15       half15;
typedef struct __Reserved_Name__Do_not_use_half32       half32;



typedef __typeof__(((int*)0)-((int*)0)) ptrdiff_t;
typedef __SIZE_TYPE__ size_t;
typedef __SIZE_TYPE__ uintptr_t;
typedef __PTRDIFF_TYPE__ intptr_t;
typedef size_t       event_t;

#define CHAR_BIT    8
#define	SCHAR_MAX	127		/* min value for a signed char */
#define	SCHAR_MIN	(-128)		/* max value for a signed char */
#define	UCHAR_MAX	255		/* max value for an unsigned char */
#define	CHAR_MAX	SCHAR_MAX		/* max value for a char */
#define	CHAR_MIN	SCHAR_MIN		/* min value for a char */
#define	USHRT_MAX	65535		/* max value for an unsigned short */
#define	SHRT_MAX	32767		/* max value for a short */
#define	SHRT_MIN	(-32768)	/* min value for a short */
#define	UINT_MAX	0xffffffff	/* max value for an unsigned int */
#define	INT_MAX		2147483647	/* max value for an int */
#define	INT_MIN		(-2147483647-1)	/* min value for an int */
#define	ULONG_MAX	0xffffffffffffffffUL	/* max unsigned long */
#define	LONG_MAX	((long)0x7fffffffffffffffL)	/* max signed long */
#define	LONG_MIN	((long)(-0x7fffffffffffffffL-1)) /* min signed long */

#define FLT_DIG         6 
#define FLT_MANT_DIG    24 
#define FLT_MAX_10_EXP  +38 
#define FLT_MAX_EXP     +128 
#define FLT_MIN_10_EXP  -37 
#define FLT_MIN_EXP     -125 
#define FLT_RADIX       2 
#define FLT_MAX         0x1.fffffep127f 
#define FLT_MIN         0x1.0p-126f 
#define FLT_EPSILON     0x1.0p-23f 

#define FP_ILOGB0       INT_MIN
#define FP_ILOGBNAN     INT_MIN

//These defines are here to satisfy a demand from the committee that 
//CL's vector types in the namespace be undefinable.  This implementation
//is backwards from what is proposed for 1.1, and is merely intended to 
//allow direct programming with __typen until such time as the main type
//changes its name to that and we #define these the other way.
#define __char2         char2
#define __char4         char4
#define __char8         char8
#define __char16        char16
#define __uchar2        uchar2
#define __uchar4        uchar4
#define __uchar8        uchar8
#define __uchar16       uchar16
#define __short2        short2
#define __short4        short4
#define __short8        short8
#define __short16       short16
#define __ushort2       ushort2
#define __ushort4       ushort4
#define __ushort8       ushort8
#define __ushort16      ushort16
#define __int2          int2
#define __int4          int4
#define __int8          int8
#define __int16         int16
#define __uint2         uint2
#define __uint4         uint4
#define __uint8         uint8
#define __uint16        uint16
#define __long2         long2
#define __long4         long4
#define __long8         long8
#define __long16        long16
#define __ulong2        ulong2
#define __ulong4        ulong4
#define __ulong8        ulong8
#define __ulong16       ulong16
#define __float2        float2
#define __float4        float4
#define __float8        float8
#define __float16       float16

#if defined( __i386__ ) || defined( __i686__ ) || defined( __x86_64__ )

    #define __double2   double2
    #define __double4   double4
    #define __double8   double8
    #define __double16  double16

    #define DBL_DIG         15 
    #define DBL_MANT_DIG    53 
    #define DBL_MAX_10_EXP  +308 
    #define DBL_MAX_EXP     +1024 
    #define DBL_MIN_10_EXP  -307 
    #define DBL_MIN_EXP     -1021 
    #define DBL_RADIX       2 
    #define DBL_MAX         0x1.fffffffffffffp1023 
    #define DBL_MIN         0x1.0p-1022 
    #define DBL_EPSILON     0x1.0p-52

    #define HUGE_VAL        __builtin_huge_val()

    #define M_E         2.71828182845904523536028747135266250   /* e */
    #define M_LOG2E     1.44269504088896340735992468100189214   /* log 2e */
    #define M_LOG10E    0.434294481903251827651128918916605082  /* log 10e */
    #define M_LN2       0.693147180559945309417232121458176568  /* log e2 */
    #define M_LN10      2.30258509299404568401799145468436421   /* log e10 */
    #define M_PI        3.14159265358979323846264338327950288   /* pi */
    #define M_PI_2      1.57079632679489661923132169163975144   /* pi/2 */
    #define M_PI_4      0.785398163397448309615660845819875721  /* pi/4 */
    #define M_1_PI      0.318309886183790671537767526745028724  /* 1/pi */
    #define M_2_PI      0.636619772367581343075535053490057448  /* 2/pi */
    #define M_2_SQRTPI  1.12837916709551257389615890312154517   /* 2/sqrt(pi) */
    #define M_SQRT2     1.41421356237309504880168872420969808   /* sqrt(2) */
    #define M_SQRT1_2   0.707106781186547524400844362104849039  /* 1/sqrt(2) */

#endif

#define __OPENCL_TYPES_DEFINED__ 1

#define vload2(X,Y)     __builtin_overload( 2, X, Y,    __vload2_2i8, __vload2_2u8, __vload2_2i16, __vload2_2u16, __vload2_2i32, __vload2_2u32, __vload2_2i64, __vload2_2u64, __vload2_f2, __vload2_d2,             \
                                                        __vload2_g2i8, __vload2_g2u8, __vload2_g2i16, __vload2_g2u16, __vload2_g2i32, __vload2_g2u32, __vload2_g2i64, __vload2_g2u64, __vload2_gf2, __vload2_gd2,    \
                                                        __vload2_l2i8, __vload2_l2u8, __vload2_l2i16, __vload2_l2u16, __vload2_l2i32, __vload2_l2u32, __vload2_l2i64, __vload2_l2u64, __vload2_lf2, __vload2_ld2,    \
                                                        __vload2_c2i8, __vload2_c2u8, __vload2_c2i16, __vload2_c2u16, __vload2_c2i32, __vload2_c2u32, __vload2_c2i64, __vload2_c2u64, __vload2_cf2, __vload2_cd2    )

#define vload4(X,Y)     __builtin_overload( 2, X, Y,    __vload4_4i8, __vload4_4u8, __vload4_4i16, __vload4_4u16, __vload4_4i32, __vload4_4u32, __vload4_4i64, __vload4_4u64, __vload4_f4, __vload4_d4,             \
                                                        __vload4_g4i8, __vload4_g4u8, __vload4_g4i16, __vload4_g4u16, __vload4_g4i32, __vload4_g4u32, __vload4_g4i64, __vload4_g4u64, __vload4_gf4, __vload4_gd4,    \
                                                        __vload4_l4i8, __vload4_l4u8, __vload4_l4i16, __vload4_l4u16, __vload4_l4i32, __vload4_l4u32, __vload4_l4i64, __vload4_l4u64, __vload4_lf4, __vload4_ld4,    \
                                                        __vload4_c4i8, __vload4_c4u8, __vload4_c4i16, __vload4_c4u16, __vload4_c4i32, __vload4_c4u32, __vload4_c4i64, __vload4_c4u64, __vload4_cf4, __vload4_cd4    )

#define vload8(X,Y)     __builtin_overload( 2, X, Y,    __vload8_8i8, __vload8_8u8, __vload8_8i16, __vload8_8u16, __vload8_8i32, __vload8_8u32, __vload8_8i64, __vload8_8u64, __vload8_f8, __vload8_d8,             \
                                                        __vload8_g8i8, __vload8_g8u8, __vload8_g8i16, __vload8_g8u16, __vload8_g8i32, __vload8_g8u32, __vload8_g8i64, __vload8_g8u64, __vload8_gf8, __vload8_gd8,    \
                                                        __vload8_l8i8, __vload8_l8u8, __vload8_l8i16, __vload8_l8u16, __vload8_l8i32, __vload8_l8u32, __vload8_l8i64, __vload8_l8u64, __vload8_lf8, __vload8_ld8,    \
                                                        __vload8_c8i8, __vload8_c8u8, __vload8_c8i16, __vload8_c8u16, __vload8_c8i32, __vload8_c8u32, __vload8_c8i64, __vload8_c8u64, __vload8_cf8, __vload8_cd8    )

#define vload16(X,Y)     __builtin_overload( 2, X, Y,    __vload16_16i8, __vload16_16u8, __vload16_16i16, __vload16_16u16, __vload16_16i32, __vload16_16u32, __vload16_16i64, __vload16_16u64, __vload16_f16, __vload16_d16,             \
                                                        __vload16_g16i8, __vload16_g16u8, __vload16_g16i16, __vload16_g16u16, __vload16_g16i32, __vload16_g16u32, __vload16_g16i64, __vload16_g16u64, __vload16_gf16, __vload16_gd16,    \
                                                        __vload16_l16i8, __vload16_l16u8, __vload16_l16i16, __vload16_l16u16, __vload16_l16i32, __vload16_l16u32, __vload16_l16i64, __vload16_l16u64, __vload16_lf16, __vload16_ld16,    \
                                                        __vload16_c16i8, __vload16_c16u8, __vload16_c16i16, __vload16_c16u16, __vload16_c16i32, __vload16_c16u32, __vload16_c16i64, __vload16_c16u64, __vload16_cf16, __vload16_cd16    )


#define vload_half( X, Y )      __builtin_overload( 2, X, Y, __vload_half, __vload_halfg, __vload_halfc, __vload_halfl )
#define vloada_half( X, Y )     __builtin_overload( 2, X, Y, __vloada_half, __vloada_halfg, __vloada_halfc, __vloada_halfl )
#define vload_half2( X, Y )     __builtin_overload( 2, X, Y, __vload_half2, __vload_half2g, __vload_half2c, __vload_half2l )
#define vloada_half2( X, Y )    __builtin_overload( 2, X, Y, __vloada_half2, __vloada_half2g, __vloada_half2c, __vloada_half2l )
#define vload_half4( X, Y )     __builtin_overload( 2, X, Y, __vload_half4, __vload_half4g, __vload_half4c, __vload_half4l )
#define vloada_half4( X, Y )    __builtin_overload( 2, X, Y, __vloada_half4, __vloada_half4g, __vloada_half4c, __vloada_half4l )
#define vload_half8( X, Y )     __builtin_overload( 2, X, Y, __vload_half8, __vload_half8g, __vload_half8c, __vload_half8l )
#define vloada_half8( X, Y )    __builtin_overload( 2, X, Y, __vloada_half8, __vloada_half8g, __vloada_half8c, __vloada_half8l )
#define vload_half16( X, Y )    __builtin_overload( 2, X, Y, __vload_half16, __vload_half16g, __vload_half16c, __vload_half16l )
#define vloada_half16( X, Y )   __builtin_overload( 2, X, Y, __vloada_half16, __vloada_half16g, __vloada_half16c, __vloada_half16l )

#define vstore2( X,Y,Z )    __builtin_overload( 3, X, Y, Z, __vstore2_i8, __vstore2_u8, __vstore2_i16, __vstore2_u16, __vstore2_i32, __vstore2_u32, __vstore2_i64, __vstore2_u64, __vstore2_f, __vstore2_d,                 \
                                                            __vstore2_i8g, __vstore2_u8g, __vstore2_i16g, __vstore2_u16g, __vstore2_i32g, __vstore2_u32g, __vstore2_i64g, __vstore2_u64g, __vstore2_fg, __vstore2_dg,       \
                                                            __vstore2_i8l, __vstore2_u8l, __vstore2_i16l, __vstore2_u16l, __vstore2_i32l, __vstore2_u32l, __vstore2_i64l, __vstore2_u64l, __vstore2_fl, __vstore2_dl )
                                                            
#define vstore4( X,Y,Z )    __builtin_overload( 3, X, Y, Z, __vstore4_i8, __vstore4_u8, __vstore4_i16, __vstore4_u16, __vstore4_i32, __vstore4_u32, __vstore4_i64, __vstore4_u64, __vstore4_f, __vstore4_d,                 \
                                                            __vstore4_i8g, __vstore4_u8g, __vstore4_i16g, __vstore4_u16g, __vstore4_i32g, __vstore4_u32g, __vstore4_i64g, __vstore4_u64g, __vstore4_fg, __vstore4_dg,       \
                                                            __vstore4_i8l, __vstore4_u8l, __vstore4_i16l, __vstore4_u16l, __vstore4_i32l, __vstore4_u32l, __vstore4_i64l, __vstore4_u64l, __vstore4_fl, __vstore4_dl )
                                                            
#define vstore8( X,Y,Z )    __builtin_overload( 3, X, Y, Z, __vstore8_i8, __vstore8_u8, __vstore8_i16, __vstore8_u16, __vstore8_i32, __vstore8_u32, __vstore8_i64, __vstore8_u64, __vstore8_f, __vstore8_d,                 \
                                                            __vstore8_i8g, __vstore8_u8g, __vstore8_i16g, __vstore8_u16g, __vstore8_i32g, __vstore8_u32g, __vstore8_i64g, __vstore8_u64g, __vstore8_fg, __vstore8_dg,       \
                                                            __vstore8_i8l, __vstore8_u8l, __vstore8_i16l, __vstore8_u16l, __vstore8_i32l, __vstore8_u32l, __vstore8_i64l, __vstore8_u64l, __vstore8_fl, __vstore8_dl )
                                                            
#define vstore16( X,Y,Z )    __builtin_overload( 3, X, Y, Z, __vstore16_i8, __vstore16_u8, __vstore16_i16, __vstore16_u16, __vstore16_i32, __vstore16_u32, __vstore16_i64, __vstore16_u64, __vstore16_f, __vstore16_d,            \
                                                            __vstore16_i8g, __vstore16_u8g, __vstore16_i16g, __vstore16_u16g, __vstore16_i32g, __vstore16_u32g, __vstore16_i64g, __vstore16_u64g, __vstore16_fg, __vstore16_dg,   \
                                                            __vstore16_i8l, __vstore16_u8l, __vstore16_i16l, __vstore16_u16l, __vstore16_i32l, __vstore16_u32l, __vstore16_i64l, __vstore16_u64l, __vstore16_fl, __vstore16_dl )

#define vstore_half( X, Y, Z )      __builtin_overload( 3, X, Y, Z, __vstore_half, __vstore_halfg, __vstore_halfl, __vstore_halfd, __vstore_halfdg, __vstore_halfdl )
#define vstore_half2( X, Y, Z )     __builtin_overload( 3, X, Y, Z, __vstore_half2, __vstore_half2g, __vstore_half2l, __vstore_halfd2, __vstore_halfd2g, __vstore_halfd2l )
#define vstore_half4( X, Y, Z )     __builtin_overload( 3, X, Y, Z, __vstore_half4, __vstore_half4g, __vstore_half4l, __vstore_halfd4, __vstore_halfd4g, __vstore_halfd4l )
#define vstore_half8( X, Y, Z )     __builtin_overload( 3, X, Y, Z, __vstore_half8, __vstore_half8g, __vstore_half8l, __vstore_halfd8, __vstore_halfd8g, __vstore_halfd8l )
#define vstore_half16( X, Y, Z )    __builtin_overload( 3, X, Y, Z, __vstore_half16, __vstore_half16g, __vstore_half16l, __vstore_halfd16, __vstore_halfd16g, __vstore_halfd16l )
#define vstorea_half( X, Y, Z )     __builtin_overload( 3, X, Y, Z, __vstorea_half, __vstorea_halfg, __vstorea_halfl, __vstorea_halfd, __vstorea_halfdg, __vstorea_halfdl )
#define vstorea_half2( X, Y, Z )    __builtin_overload( 3, X, Y, Z, __vstorea_half2, __vstorea_half2g, __vstorea_half2l, __vstorea_halfd2, __vstorea_halfd2g, __vstorea_halfd2l )
#define vstorea_half4( X, Y, Z )    __builtin_overload( 3, X, Y, Z, __vstorea_half4, __vstorea_half4g, __vstorea_half4l, __vstorea_halfd4, __vstorea_halfd4g, __vstorea_halfd4l )
#define vstorea_half8( X, Y, Z )    __builtin_overload( 3, X, Y, Z, __vstorea_half8, __vstorea_half8g, __vstorea_half8l, __vstorea_halfd8, __vstorea_halfd8g, __vstorea_halfd8l )
#define vstorea_half16( X, Y, Z )   __builtin_overload( 3, X, Y, Z, __vstorea_half16, __vstorea_half16g, __vstorea_half16l, __vstorea_halfd16, __vstorea_halfd16g, __vstorea_halfd16l )

#define vstore_half_rte( X, Y, Z )      __builtin_overload( 3, X, Y, Z, __vstore_half_rte, __vstore_half_rteg, __vstore_half_rtel, __vstore_halfd_rte, __vstore_halfd_rteg, __vstore_halfd_rtel )
#define vstore_half2_rte( X, Y, Z )     __builtin_overload( 3, X, Y, Z, __vstore_half2_rte, __vstore_half2_rteg, __vstore_half2_rtel, __vstore_halfd2_rte, __vstore_halfd2_rteg, __vstore_halfd2_rtel )
#define vstore_half4_rte( X, Y, Z )     __builtin_overload( 3, X, Y, Z, __vstore_half4_rte, __vstore_half4_rteg, __vstore_half4_rtel, __vstore_halfd4_rte, __vstore_halfd4_rteg, __vstore_halfd4_rtel )
#define vstore_half8_rte( X, Y, Z )     __builtin_overload( 3, X, Y, Z, __vstore_half8_rte, __vstore_half8_rteg, __vstore_half8_rtel, __vstore_halfd8_rte, __vstore_halfd8_rteg, __vstore_halfd8_rtel )
#define vstore_half16_rte( X, Y, Z )    __builtin_overload( 3, X, Y, Z, __vstore_half16_rte, __vstore_half16_rteg, __vstore_half16_rtel, __vstore_halfd16_rte, __vstore_halfd16_rteg, __vstore_halfd16_rtel )
#define vstorea_half_rte( X, Y, Z )     __builtin_overload( 3, X, Y, Z, __vstorea_half_rte, __vstorea_half_rteg, __vstorea_half_rtel, __vstorea_halfd_rte, __vstorea_halfd_rteg, __vstorea_halfd_rtel )
#define vstorea_half2_rte( X, Y, Z )    __builtin_overload( 3, X, Y, Z, __vstorea_half2_rte, __vstorea_half2_rteg, __vstorea_half2_rtel, __vstorea_halfd2_rte, __vstorea_halfd2_rteg, __vstorea_halfd2_rtel )
#define vstorea_half4_rte( X, Y, Z )    __builtin_overload( 3, X, Y, Z, __vstorea_half4_rte, __vstorea_half4_rteg, __vstorea_half4_rtel, __vstorea_halfd4_rte, __vstorea_halfd4_rteg, __vstorea_halfd4_rtel )
#define vstorea_half8_rte( X, Y, Z )    __builtin_overload( 3, X, Y, Z, __vstorea_half8_rte, __vstorea_half8_rteg, __vstorea_half8_rtel, __vstorea_halfd8_rte, __vstorea_halfd8_rteg, __vstorea_halfd8_rtel )
#define vstorea_half16_rte( X, Y, Z )   __builtin_overload( 3, X, Y, Z, __vstorea_half16_rte, __vstorea_half16_rteg, __vstorea_half16_rtel, __vstorea_halfd16_rte, __vstorea_halfd16_rteg, __vstorea_halfd16_rtel )

#define vstore_half_rtz( X, Y, Z )      __builtin_overload( 3, X, Y, Z, __vstore_half_rtz, __vstore_half_rtzg, __vstore_half_rtzl, __vstore_halfd_rtz, __vstore_halfd_rtzg, __vstore_halfd_rtzl )
#define vstore_half2_rtz( X, Y, Z )     __builtin_overload( 3, X, Y, Z, __vstore_half2_rtz, __vstore_half2_rtzg, __vstore_half2_rtzl, __vstore_halfd2_rtz, __vstore_halfd2_rtzg, __vstore_halfd2_rtzl )
#define vstore_half4_rtz( X, Y, Z )     __builtin_overload( 3, X, Y, Z, __vstore_half4_rtz, __vstore_half4_rtzg, __vstore_half4_rtzl, __vstore_halfd4_rtz, __vstore_halfd4_rtzg, __vstore_halfd4_rtzl )
#define vstore_half8_rtz( X, Y, Z )     __builtin_overload( 3, X, Y, Z, __vstore_half8_rtz, __vstore_half8_rtzg, __vstore_half8_rtzl, __vstore_halfd8_rtz, __vstore_halfd8_rtzg, __vstore_halfd8_rtzl )
#define vstore_half16_rtz( X, Y, Z )    __builtin_overload( 3, X, Y, Z, __vstore_half16_rtz, __vstore_half16_rtzg, __vstore_half16_rtzl, __vstore_halfd16_rtz, __vstore_halfd16_rtzg, __vstore_halfd16_rtzl )
#define vstorea_half_rtz( X, Y, Z )     __builtin_overload( 3, X, Y, Z, __vstorea_half_rtz, __vstorea_half_rtzg, __vstorea_half_rtzl, __vstorea_halfd_rtz, __vstorea_halfd_rtzg, __vstorea_halfd_rtzl )
#define vstorea_half2_rtz( X, Y, Z )    __builtin_overload( 3, X, Y, Z, __vstorea_half2_rtz, __vstorea_half2_rtzg, __vstorea_half2_rtzl, __vstorea_halfd2_rtz, __vstorea_halfd2_rtzg, __vstorea_halfd2_rtzl )
#define vstorea_half4_rtz( X, Y, Z )    __builtin_overload( 3, X, Y, Z, __vstorea_half4_rtz, __vstorea_half4_rtzg, __vstorea_half4_rtzl, __vstorea_halfd4_rtz, __vstorea_halfd4_rtzg, __vstorea_halfd4_rtzl )
#define vstorea_half8_rtz( X, Y, Z )    __builtin_overload( 3, X, Y, Z, __vstorea_half8_rtz, __vstorea_half8_rtzg, __vstorea_half8_rtzl, __vstorea_halfd8_rtz, __vstorea_halfd8_rtzg, __vstorea_halfd8_rtzl )
#define vstorea_half16_rtz( X, Y, Z )   __builtin_overload( 3, X, Y, Z, __vstorea_half16_rtz, __vstorea_half16_rtzg, __vstorea_half16_rtzl, __vstorea_halfd16_rtz, __vstorea_halfd16_rtzg, __vstorea_halfd16_rtzl )

#define vstore_half_rtp( X, Y, Z )      __builtin_overload( 3, X, Y, Z, __vstore_half_rtp, __vstore_half_rtpg, __vstore_half_rtpl, __vstore_halfd_rtp, __vstore_halfd_rtpg, __vstore_halfd_rtpl )
#define vstore_half2_rtp( X, Y, Z )     __builtin_overload( 3, X, Y, Z, __vstore_half2_rtp, __vstore_half2_rtpg, __vstore_half2_rtpl, __vstore_halfd2_rtp, __vstore_halfd2_rtpg, __vstore_halfd2_rtpl )
#define vstore_half4_rtp( X, Y, Z )     __builtin_overload( 3, X, Y, Z, __vstore_half4_rtp, __vstore_half4_rtpg, __vstore_half4_rtpl, __vstore_halfd4_rtp, __vstore_halfd4_rtpg, __vstore_halfd4_rtpl )
#define vstore_half8_rtp( X, Y, Z )     __builtin_overload( 3, X, Y, Z, __vstore_half8_rtp, __vstore_half8_rtpg, __vstore_half8_rtpl, __vstore_halfd8_rtp, __vstore_halfd8_rtpg, __vstore_halfd8_rtpl )
#define vstore_half16_rtp( X, Y, Z )    __builtin_overload( 3, X, Y, Z, __vstore_half16_rtp, __vstore_half16_rtpg, __vstore_half16_rtpl, __vstore_halfd16_rtp, __vstore_halfd16_rtpg, __vstore_halfd16_rtpl )
#define vstorea_half_rtp( X, Y, Z )     __builtin_overload( 3, X, Y, Z, __vstorea_half_rtp, __vstorea_half_rtpg, __vstorea_half_rtpl, __vstorea_halfd_rtp, __vstorea_halfd_rtpg, __vstorea_halfd_rtpl )
#define vstorea_half2_rtp( X, Y, Z )    __builtin_overload( 3, X, Y, Z, __vstorea_half2_rtp, __vstorea_half2_rtpg, __vstorea_half2_rtpl, __vstorea_halfd2_rtp, __vstorea_halfd2_rtpg, __vstorea_halfd2_rtpl )
#define vstorea_half4_rtp( X, Y, Z )    __builtin_overload( 3, X, Y, Z, __vstorea_half4_rtp, __vstorea_half4_rtpg, __vstorea_half4_rtpl, __vstorea_halfd4_rtp, __vstorea_halfd4_rtpg, __vstorea_halfd4_rtpl )
#define vstorea_half8_rtp( X, Y, Z )    __builtin_overload( 3, X, Y, Z, __vstorea_half8_rtp, __vstorea_half8_rtpg, __vstorea_half8_rtpl, __vstorea_halfd8_rtp, __vstorea_halfd8_rtpg, __vstorea_halfd8_rtpl )
#define vstorea_half16_rtp( X, Y, Z )   __builtin_overload( 3, X, Y, Z, __vstorea_half16_rtp, __vstorea_half16_rtpg, __vstorea_half16_rtpl, __vstorea_halfd16_rtp, __vstorea_halfd16_rtpg, __vstorea_halfd16_rtpl )

#define vstore_half_rtn( X, Y, Z )      __builtin_overload( 3, X, Y, Z, __vstore_half_rtn, __vstore_half_rtng, __vstore_half_rtnl, __vstore_halfd_rtn, __vstore_halfd_rtng, __vstore_halfd_rtnl )
#define vstore_half2_rtn( X, Y, Z )     __builtin_overload( 3, X, Y, Z, __vstore_half2_rtn, __vstore_half2_rtng, __vstore_half2_rtnl, __vstore_halfd2_rtn, __vstore_halfd2_rtng, __vstore_halfd2_rtnl )
#define vstore_half4_rtn( X, Y, Z )     __builtin_overload( 3, X, Y, Z, __vstore_half4_rtn, __vstore_half4_rtng, __vstore_half4_rtnl, __vstore_halfd4_rtn, __vstore_halfd4_rtng, __vstore_halfd4_rtnl )
#define vstore_half8_rtn( X, Y, Z )     __builtin_overload( 3, X, Y, Z, __vstore_half8_rtn, __vstore_half8_rtng, __vstore_half8_rtnl, __vstore_halfd8_rtn, __vstore_halfd8_rtng, __vstore_halfd8_rtnl )
#define vstore_half16_rtn( X, Y, Z )    __builtin_overload( 3, X, Y, Z, __vstore_half16_rtn, __vstore_half16_rtng, __vstore_half16_rtnl, __vstore_halfd16_rtn, __vstore_halfd16_rtng, __vstore_halfd16_rtnl )
#define vstorea_half_rtn( X, Y, Z )     __builtin_overload( 3, X, Y, Z, __vstorea_half_rtn, __vstorea_half_rtng, __vstorea_half_rtnl, __vstorea_halfd_rtn, __vstorea_halfd_rtng, __vstorea_halfd_rtnl )
#define vstorea_half2_rtn( X, Y, Z )    __builtin_overload( 3, X, Y, Z, __vstorea_half2_rtn, __vstorea_half2_rtng, __vstorea_half2_rtnl, __vstorea_halfd2_rtn, __vstorea_halfd2_rtng, __vstorea_halfd2_rtnl )
#define vstorea_half4_rtn( X, Y, Z )    __builtin_overload( 3, X, Y, Z, __vstorea_half4_rtn, __vstorea_half4_rtng, __vstorea_half4_rtnl, __vstorea_halfd4_rtn, __vstorea_halfd4_rtng, __vstorea_halfd4_rtnl )
#define vstorea_half8_rtn( X, Y, Z )    __builtin_overload( 3, X, Y, Z, __vstorea_half8_rtn, __vstorea_half8_rtng, __vstorea_half8_rtnl, __vstorea_halfd8_rtn, __vstorea_halfd8_rtng, __vstorea_halfd8_rtnl )
#define vstorea_half16_rtn( X, Y, Z )   __builtin_overload( 3, X, Y, Z, __vstorea_half16_rtn, __vstorea_half16_rtng, __vstorea_half16_rtnl, __vstorea_halfd16_rtn, __vstorea_halfd16_rtng, __vstorea_halfd16_rtnl )


// 5.1.4 Vector Components / Constructors
#define make_uchar2(A,B) (uchar2)((A),(B))
#define make_uchar4(A,B,C,D) (uchar4)((A),(B),(C),(D))
#define make_uchar8(A,B,C,D,E,F,G,H) (uchar8)((A),(B),(C),(D),(E),(F),(G),(H))
#define make_uchar16(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P) (uchar16)((A),(B),(C),(D),(E),(F),(G),(H),(I),(J),(K),(L),(M),(N),(O),(P))

#define make_char2(A,B) (char2)((A),(B))
#define make_char4(A,B,C,D) (char4)((A),(B),(C),(D))
#define make_char8(A,B,C,D,E,F,G,H) (char8)((A),(B),(C),(D),(E),(F),(G),(H))
#define make_char16(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P) (char16)((A),(B),(C),(D),(E),(F),(G),(H),(I),(J),(K),(L),(M),(N),(O),(P))

#define make_short2(A,B) (short2)((A),(B))
#define make_short4(A,B,C,D) (short4)((A),(B),(C),(D))
#define make_short8(A,B,C,D,E,F,G,H) (short8)((A),(B),(C),(D),(E),(F),(G),(H))
#define make_short16(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P) (short16)((A),(B),(C),(D),(E),(F),(G),(H),(I),(J),(K),(L),(M),(N),(O),(P))

#define make_ushort2(A,B) (ushort2)((A),(B))
#define make_ushort4(A,B,C,D) (ushort4)((A),(B),(C),(D))
#define make_ushort8(A,B,C,D,E,F,G,H) (ushort8)((A),(B),(C),(D),(E),(F),(G),(H))
#define make_ushort16(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P) (ushort16)((A),(B),(C),(D),(E),(F),(G),(H),(I),(J),(K),(L),(M),(N),(O),(P))

#define make_int2(A,B) (int2)((A),(B))
#define make_int4(A,B,C,D) (int4)((A),(B),(C),(D))
#define make_int8(A,B,C,D,E,F,G,H) (int8)((A),(B),(C),(D),(E),(F),(G),(H))
#define make_int16(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P) (int16)((A),(B),(C),(D),(E),(F),(G),(H),(I),(J),(K),(L),(M),(N),(O),(P))

#define make_uint2(A,B) (uint2)((A),(B))
#define make_uint4(A,B,C,D) (uint4)((A),(B),(C),(D))
#define make_uint8(A,B,C,D,E,F,G,H) (uint8)((A),(B),(C),(D),(E),(F),(G),(H))
#define make_uint16(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P) (uint16)((A),(B),(C),(D),(E),(F),(G),(H),(I),(J),(K),(L),(M),(N),(O),(P))

#define make_long2(A,B) (long2)((A),(B))
#define make_long4(A,B,C,D) (long4)((A),(B),(C),(D))
#define make_long8(A,B,C,D,E,F,G,H) (long8)((A),(B),(C),(D),(E),(F),(G),(H))
#define make_long16(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P) (long16)((A),(B),(C),(D),(E),(F),(G),(H),(I),(J),(K),(L),(M),(N),(O),(P))

#define make_ulong2(A,B) (ulong2)((A),(B))
#define make_ulong4(A,B,C,D) (ulong4)((A),(B),(C),(D))
#define make_ulong8(A,B,C,D,E,F,G,H) (ulong8)((A),(B),(C),(D),(E),(F),(G),(H))
#define make_ulong16(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P) (ulong16)((A),(B),(C),(D),(E),(F),(G),(H),(I),(J),(K),(L),(M),(N),(O),(P))

#define make_float2(A,B) (float2)((A),(B))
#define make_float3_SPI(A,B,C) (__float3_SPI)((A),(B),(C))
#define make_float4(A,B,C,D) (float4)((A),(B),(C),(D))
#define make_float8(A,B,C,D,E,F,G,H) (float8)((A),(B),(C),(D),(E),(F),(G),(H))
#define make_float16(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P) (float16)((A),(B),(C),(D),(E),(F),(G),(H),(I),(J),(K),(L),(M),(N),(O),(P))

#define make_double2(A,B) (double2)((A),(B))
#define make_double4(A,B,C,D) (double4)((A),(B),(C),(D))
#define make_double8(A,B,C,D,E,F,G,H) (double8)((A),(B),(C),(D),(E),(F),(G),(H))
#define make_double16(A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P) (double16)((A),(B),(C),(D),(E),(F),(G),(H),(I),(J),(K),(L),(M),(N),(O),(P))

// 5.2.3 convert_ operators
typedef enum
{
    __kDefaultRoundingMode = 0,
    __kRoundToNearestEven = 1,
    __kRoundTowardNegativeInf = 2,
    __kRoundTowardInf = 3,
    __kRoundTowardZero = 4
}__clRoundingMode;

typedef enum
{
    __kUnsaturated = 0,
    __kSaturated = 1
}__clSaturationMode;

#if defined( __PTX__ )

    //type
    #define convert_char(_X)    __builtin_convert(_X, char, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_uchar(_X)   __builtin_convert(_X, uchar, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_short(_X)   __builtin_convert(_X, short, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_ushort(_X)  __builtin_convert(_X, ushort, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_int(_X)     __builtin_convert(_X, int, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_uint(_X)    __builtin_convert(_X, uint, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_long(_X)    __builtin_convert(_X, long, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_ulong(_X)   __builtin_convert(_X, ulong, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_float(_X)   __builtin_convert(_X, float, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_double(_X)  __builtin_convert(_X, double, __kDefaultRoundingMode, __kUnsaturated )

    #define convert_char_sat(_X)    __builtin_convert(_X, char, __kDefaultRoundingMode, __kSaturated )
    #define convert_uchar_sat(_X)   __builtin_convert(_X, uchar, __kDefaultRoundingMode, __kSaturated )
    #define convert_short_sat(_X)   __builtin_convert(_X, short, __kDefaultRoundingMode, __kSaturated )
    #define convert_ushort_sat(_X)  __builtin_convert(_X, ushort, __kDefaultRoundingMode, __kSaturated )
    #define convert_int_sat(_X)     __builtin_convert(_X, int, __kDefaultRoundingMode, __kSaturated )
    #define convert_uint_sat(_X)    __builtin_convert(_X, uint, __kDefaultRoundingMode, __kSaturated )
    #define convert_long_sat(_X)    __builtin_convert(_X, long, __kDefaultRoundingMode, __kSaturated )
    #define convert_ulong_sat(_X)   __builtin_convert(_X, ulong, __kDefaultRoundingMode, __kSaturated )
    #define convert_float_sat(_X)   __builtin_convert(_X, float, __kDefaultRoundingMode, __kSaturated )
    #define convert_double_sat(_X)  __builtin_convert(_X, double, __kDefaultRoundingMode, __kSaturated )

    #define convert_char_rte(_X)    __builtin_convert(_X, char, __kRoundToNearestEven, __kUnsaturated )
    #define convert_uchar_rte(_X)   __builtin_convert(_X, uchar, __kRoundToNearestEven, __kUnsaturated )
    #define convert_short_rte(_X)   __builtin_convert(_X, short, __kRoundToNearestEven, __kUnsaturated )
    #define convert_ushort_rte(_X)  __builtin_convert(_X, ushort, __kRoundToNearestEven, __kUnsaturated )
    #define convert_int_rte(_X)     __builtin_convert(_X, int, __kRoundToNearestEven, __kUnsaturated )
    #define convert_uint_rte(_X)    __builtin_convert(_X, uint, __kRoundToNearestEven, __kUnsaturated )
    #define convert_long_rte(_X)    __builtin_convert(_X, long, __kRoundToNearestEven, __kUnsaturated )
    #define convert_ulong_rte(_X)   __builtin_convert(_X, ulong, __kRoundToNearestEven, __kUnsaturated )
    #define convert_float_rte(_X)   __builtin_convert(_X, float, __kRoundToNearestEven, __kUnsaturated )
    #define convert_double_rte(_X)  __builtin_convert(_X, double, __kRoundToNearestEven, __kUnsaturated )

    #define convert_char_sat_rte(_X)    __builtin_convert(_X, char, __kRoundToNearestEven, __kSaturated )
    #define convert_uchar_sat_rte(_X)   __builtin_convert(_X, uchar, __kRoundToNearestEven, __kSaturated )
    #define convert_short_sat_rte(_X)   __builtin_convert(_X, short, __kRoundToNearestEven, __kSaturated )
    #define convert_ushort_sat_rte(_X)  __builtin_convert(_X, ushort, __kRoundToNearestEven, __kSaturated )
    #define convert_int_sat_rte(_X)     __builtin_convert(_X, int, __kRoundToNearestEven, __kSaturated )
    #define convert_uint_sat_rte(_X)    __builtin_convert(_X, uint, __kRoundToNearestEven, __kSaturated )
    #define convert_long_sat_rte(_X)    __builtin_convert(_X, long, __kRoundToNearestEven, __kSaturated )
    #define convert_ulong_sat_rte(_X)   __builtin_convert(_X, ulong, __kRoundToNearestEven, __kSaturated )
    #define convert_float_sat_rte(_X)   __builtin_convert(_X, float, __kRoundToNearestEven, __kSaturated )
    #define convert_double_sat_rte(_X)  __builtin_convert(_X, double, __kRoundToNearestEven, __kSaturated )

    #define convert_char_rtn(_X)    __builtin_convert(_X, char, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_uchar_rtn(_X)   __builtin_convert(_X, uchar, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_short_rtn(_X)   __builtin_convert(_X, short, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_ushort_rtn(_X)  __builtin_convert(_X, ushort, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_int_rtn(_X)     __builtin_convert(_X, int, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_uint_rtn(_X)    __builtin_convert(_X, uint, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_long_rtn(_X)    __builtin_convert(_X, long, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_ulong_rtn(_X)   __builtin_convert(_X, ulong, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_float_rtn(_X)   __builtin_convert(_X, float, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_double_rtn(_X)  __builtin_convert(_X, double, __kRoundTowardNegativeInf, __kUnsaturated )

    #define convert_char_sat_rtn(_X)    __builtin_convert(_X, char, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_uchar_sat_rtn(_X)   __builtin_convert(_X, uchar, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_short_sat_rtn(_X)   __builtin_convert(_X, short, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_ushort_sat_rtn(_X)  __builtin_convert(_X, ushort, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_int_sat_rtn(_X)     __builtin_convert(_X, int, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_uint_sat_rtn(_X)    __builtin_convert(_X, uint, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_long_sat_rtn(_X)    __builtin_convert(_X, long, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_ulong_sat_rtn(_X)   __builtin_convert(_X, ulong, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_float_sat_rtn(_X)   __builtin_convert(_X, float, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_double_sat_rtn(_X)  __builtin_convert(_X, double, __kRoundTowardNegativeInf, __kSaturated )

    #define convert_char_rtp(_X)    __builtin_convert(_X, char, __kRoundTowardInf, __kUnsaturated )
    #define convert_uchar_rtp(_X)   __builtin_convert(_X, uchar, __kRoundTowardInf, __kUnsaturated )
    #define convert_short_rtp(_X)   __builtin_convert(_X, short, __kRoundTowardInf, __kUnsaturated )
    #define convert_ushort_rtp(_X)  __builtin_convert(_X, ushort, __kRoundTowardInf, __kUnsaturated )
    #define convert_int_rtp(_X)     __builtin_convert(_X, int, __kRoundTowardInf, __kUnsaturated )
    #define convert_uint_rtp(_X)    __builtin_convert(_X, uint, __kRoundTowardInf, __kUnsaturated )
    #define convert_long_rtp(_X)    __builtin_convert(_X, long, __kRoundTowardInf, __kUnsaturated )
    #define convert_ulong_rtp(_X)   __builtin_convert(_X, ulong, __kRoundTowardInf, __kUnsaturated )
    #define convert_float_rtp(_X)   __builtin_convert(_X, float, __kRoundTowardInf, __kUnsaturated )
    #define convert_double_rtp(_X)  __builtin_convert(_X, double, __kRoundTowardInf, __kUnsaturated )

    #define convert_char_sat_rtp(_X)    __builtin_convert(_X, char, __kRoundTowardInf, __kSaturated )
    #define convert_uchar_sat_rtp(_X)   __builtin_convert(_X, uchar, __kRoundTowardInf, __kSaturated )
    #define convert_short_sat_rtp(_X)   __builtin_convert(_X, short, __kRoundTowardInf, __kSaturated )
    #define convert_ushort_sat_rtp(_X)  __builtin_convert(_X, ushort, __kRoundTowardInf, __kSaturated )
    #define convert_int_sat_rtp(_X)     __builtin_convert(_X, int, __kRoundTowardInf, __kSaturated )
    #define convert_uint_sat_rtp(_X)    __builtin_convert(_X, uint, __kRoundTowardInf, __kSaturated )
    #define convert_long_sat_rtp(_X)    __builtin_convert(_X, long, __kRoundTowardInf, __kSaturated )
    #define convert_ulong_sat_rtp(_X)   __builtin_convert(_X, ulong, __kRoundTowardInf, __kSaturated )
    #define convert_float_sat_rtp(_X)   __builtin_convert(_X, float, __kRoundTowardInf, __kSaturated )
    #define convert_double_sat_rtp(_X)  __builtin_convert(_X, double, __kRoundTowardInf, __kSaturated )

    #define convert_char_rtz(_X)    __builtin_convert(_X, char, __kRoundTowardZero, __kUnsaturated )
    #define convert_uchar_rtz(_X)   __builtin_convert(_X, uchar, __kRoundTowardZero, __kUnsaturated )
    #define convert_short_rtz(_X)   __builtin_convert(_X, short, __kRoundTowardZero, __kUnsaturated )
    #define convert_ushort_rtz(_X)  __builtin_convert(_X, ushort, __kRoundTowardZero, __kUnsaturated )
    #define convert_int_rtz(_X)     __builtin_convert(_X, int, __kRoundTowardZero, __kUnsaturated )
    #define convert_uint_rtz(_X)    __builtin_convert(_X, uint, __kRoundTowardZero, __kUnsaturated )
    #define convert_long_rtz(_X)    __builtin_convert(_X, long, __kRoundTowardZero, __kUnsaturated )
    #define convert_ulong_rtz(_X)   __builtin_convert(_X, ulong, __kRoundTowardZero, __kUnsaturated )
    #define convert_float_rtz(_X)   __builtin_convert(_X, float, __kRoundTowardZero, __kUnsaturated )
    #define convert_double_rtz(_X)  __builtin_convert(_X, double, __kRoundTowardZero, __kUnsaturated )

    #define convert_char_sat_rtz(_X)    __builtin_convert(_X, char, __kRoundTowardZero, __kSaturated )
    #define convert_uchar_sat_rtz(_X)   __builtin_convert(_X, uchar, __kRoundTowardZero, __kSaturated )
    #define convert_short_sat_rtz(_X)   __builtin_convert(_X, short, __kRoundTowardZero, __kSaturated )
    #define convert_ushort_sat_rtz(_X)  __builtin_convert(_X, ushort, __kRoundTowardZero, __kSaturated )
    #define convert_int_sat_rtz(_X)     __builtin_convert(_X, int, __kRoundTowardZero, __kSaturated )
    #define convert_uint_sat_rtz(_X)    __builtin_convert(_X, uint, __kRoundTowardZero, __kSaturated )
    #define convert_long_sat_rtz(_X)    __builtin_convert(_X, long, __kRoundTowardZero, __kSaturated )
    #define convert_ulong_sat_rtz(_X)   __builtin_convert(_X, ulong, __kRoundTowardZero, __kSaturated )
    #define convert_float_sat_rtz(_X)   __builtin_convert(_X, float, __kRoundTowardZero, __kSaturated )
    #define convert_double_sat_rtz(_X)  __builtin_convert(_X, double, __kRoundTowardZero, __kSaturated )

    //type2
    #define convert_char2(_X)    __builtin_convert(_X, char2, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_uchar2(_X)   __builtin_convert(_X, uchar2, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_short2(_X)   __builtin_convert(_X, short2, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_ushort2(_X)  __builtin_convert(_X, ushort2, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_int2(_X)     __builtin_convert(_X, int2, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_uint2(_X)    __builtin_convert(_X, uint2, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_long2(_X)    __builtin_convert(_X, long2, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_ulong2(_X)   __builtin_convert(_X, ulong2, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_float2(_X)   __builtin_convert(_X, float2, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_double2(_X)  __builtin_convert(_X, double2, __kDefaultRoundingMode, __kUnsaturated )

    #define convert_char2_sat(_X)    __builtin_convert(_X, char2, __kDefaultRoundingMode, __kSaturated )
    #define convert_uchar2_sat(_X)   __builtin_convert(_X, uchar2, __kDefaultRoundingMode, __kSaturated )
    #define convert_short2_sat(_X)   __builtin_convert(_X, short2, __kDefaultRoundingMode, __kSaturated )
    #define convert_ushort2_sat(_X)  __builtin_convert(_X, ushort2, __kDefaultRoundingMode, __kSaturated )
    #define convert_int2_sat(_X)     __builtin_convert(_X, int2, __kDefaultRoundingMode, __kSaturated )
    #define convert_uint2_sat(_X)    __builtin_convert(_X, uint2, __kDefaultRoundingMode, __kSaturated )
    #define convert_long2_sat(_X)    __builtin_convert(_X, long2, __kDefaultRoundingMode, __kSaturated )
    #define convert_ulong2_sat(_X)   __builtin_convert(_X, ulong2, __kDefaultRoundingMode, __kSaturated )
    #define convert_float2_sat(_X)   __builtin_convert(_X, float2, __kDefaultRoundingMode, __kSaturated )
    #define convert_double2_sat(_X)  __builtin_convert(_X, double2, __kDefaultRoundingMode, __kSaturated )

    #define convert_char2_rte(_X)    __builtin_convert(_X, char2, __kRoundToNearestEven, __kUnsaturated )
    #define convert_uchar2_rte(_X)   __builtin_convert(_X, uchar2, __kRoundToNearestEven, __kUnsaturated )
    #define convert_short2_rte(_X)   __builtin_convert(_X, short2, __kRoundToNearestEven, __kUnsaturated )
    #define convert_ushort2_rte(_X)  __builtin_convert(_X, ushort2, __kRoundToNearestEven, __kUnsaturated )
    #define convert_int2_rte(_X)     __builtin_convert(_X, int2, __kRoundToNearestEven, __kUnsaturated )
    #define convert_uint2_rte(_X)    __builtin_convert(_X, uint2, __kRoundToNearestEven, __kUnsaturated )
    #define convert_long2_rte(_X)    __builtin_convert(_X, long2, __kRoundToNearestEven, __kUnsaturated )
    #define convert_ulong2_rte(_X)   __builtin_convert(_X, ulong2, __kRoundToNearestEven, __kUnsaturated )
    #define convert_float2_rte(_X)   __builtin_convert(_X, float2, __kRoundToNearestEven, __kUnsaturated )
    #define convert_double2_rte(_X)  __builtin_convert(_X, double2, __kRoundToNearestEven, __kUnsaturated )

    #define convert_char2_sat_rte(_X)    __builtin_convert(_X, char2, __kRoundToNearestEven, __kSaturated )
    #define convert_uchar2_sat_rte(_X)   __builtin_convert(_X, uchar2, __kRoundToNearestEven, __kSaturated )
    #define convert_short2_sat_rte(_X)   __builtin_convert(_X, short2, __kRoundToNearestEven, __kSaturated )
    #define convert_ushort2_sat_rte(_X)  __builtin_convert(_X, ushort2, __kRoundToNearestEven, __kSaturated )
    #define convert_int2_sat_rte(_X)     __builtin_convert(_X, int2, __kRoundToNearestEven, __kSaturated )
    #define convert_uint2_sat_rte(_X)    __builtin_convert(_X, uint2, __kRoundToNearestEven, __kSaturated )
    #define convert_long2_sat_rte(_X)    __builtin_convert(_X, long2, __kRoundToNearestEven, __kSaturated )
    #define convert_ulong2_sat_rte(_X)   __builtin_convert(_X, ulong2, __kRoundToNearestEven, __kSaturated )
    #define convert_float2_sat_rte(_X)   __builtin_convert(_X, float2, __kRoundToNearestEven, __kSaturated )
    #define convert_double2_sat_rte(_X)  __builtin_convert(_X, double2, __kRoundToNearestEven, __kSaturated )

    #define convert_char2_rtn(_X)    __builtin_convert(_X, char2, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_uchar2_rtn(_X)   __builtin_convert(_X, uchar2, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_short2_rtn(_X)   __builtin_convert(_X, short2, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_ushort2_rtn(_X)  __builtin_convert(_X, ushort2, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_int2_rtn(_X)     __builtin_convert(_X, int2, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_uint2_rtn(_X)    __builtin_convert(_X, uint2, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_long2_rtn(_X)    __builtin_convert(_X, long2, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_ulong2_rtn(_X)   __builtin_convert(_X, ulong2, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_float2_rtn(_X)   __builtin_convert(_X, float2, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_double2_rtn(_X)  __builtin_convert(_X, double2, __kRoundTowardNegativeInf, __kUnsaturated )

    #define convert_char2_sat_rtn(_X)    __builtin_convert(_X, char2, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_uchar2_sat_rtn(_X)   __builtin_convert(_X, uchar2, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_short2_sat_rtn(_X)   __builtin_convert(_X, short2, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_ushort2_sat_rtn(_X)  __builtin_convert(_X, ushort2, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_int2_sat_rtn(_X)     __builtin_convert(_X, int2, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_uint2_sat_rtn(_X)    __builtin_convert(_X, uint2, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_long2_sat_rtn(_X)    __builtin_convert(_X, long2, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_ulong2_sat_rtn(_X)   __builtin_convert(_X, ulong2, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_float2_sat_rtn(_X)   __builtin_convert(_X, float2, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_double2_sat_rtn(_X)  __builtin_convert(_X, double2, __kRoundTowardNegativeInf, __kSaturated )

    #define convert_char2_rtp(_X)    __builtin_convert(_X, char2, __kRoundTowardInf, __kUnsaturated )
    #define convert_uchar2_rtp(_X)   __builtin_convert(_X, uchar2, __kRoundTowardInf, __kUnsaturated )
    #define convert_short2_rtp(_X)   __builtin_convert(_X, short2, __kRoundTowardInf, __kUnsaturated )
    #define convert_ushort2_rtp(_X)  __builtin_convert(_X, ushort2, __kRoundTowardInf, __kUnsaturated )
    #define convert_int2_rtp(_X)     __builtin_convert(_X, int2, __kRoundTowardInf, __kUnsaturated )
    #define convert_uint2_rtp(_X)    __builtin_convert(_X, uint2, __kRoundTowardInf, __kUnsaturated )
    #define convert_long2_rtp(_X)    __builtin_convert(_X, long2, __kRoundTowardInf, __kUnsaturated )
    #define convert_ulong2_rtp(_X)   __builtin_convert(_X, ulong2, __kRoundTowardInf, __kUnsaturated )
    #define convert_float2_rtp(_X)   __builtin_convert(_X, float2, __kRoundTowardInf, __kUnsaturated )
    #define convert_double2_rtp(_X)  __builtin_convert(_X, double2, __kRoundTowardInf, __kUnsaturated )

    #define convert_char2_sat_rtp(_X)    __builtin_convert(_X, char2, __kRoundTowardInf, __kSaturated )
    #define convert_uchar2_sat_rtp(_X)   __builtin_convert(_X, uchar2, __kRoundTowardInf, __kSaturated )
    #define convert_short2_sat_rtp(_X)   __builtin_convert(_X, short2, __kRoundTowardInf, __kSaturated )
    #define convert_ushort2_sat_rtp(_X)  __builtin_convert(_X, ushort2, __kRoundTowardInf, __kSaturated )
    #define convert_int2_sat_rtp(_X)     __builtin_convert(_X, int2, __kRoundTowardInf, __kSaturated )
    #define convert_uint2_sat_rtp(_X)    __builtin_convert(_X, uint2, __kRoundTowardInf, __kSaturated )
    #define convert_long2_sat_rtp(_X)    __builtin_convert(_X, long2, __kRoundTowardInf, __kSaturated )
    #define convert_ulong2_sat_rtp(_X)   __builtin_convert(_X, ulong2, __kRoundTowardInf, __kSaturated )
    #define convert_float2_sat_rtp(_X)   __builtin_convert(_X, float2, __kRoundTowardInf, __kSaturated )
    #define convert_double2_sat_rtp(_X)  __builtin_convert(_X, double2, __kRoundTowardInf, __kSaturated )

    #define convert_char2_rtz(_X)    __builtin_convert(_X, char2, __kRoundTowardZero, __kUnsaturated )
    #define convert_uchar2_rtz(_X)   __builtin_convert(_X, uchar2, __kRoundTowardZero, __kUnsaturated )
    #define convert_short2_rtz(_X)   __builtin_convert(_X, short2, __kRoundTowardZero, __kUnsaturated )
    #define convert_ushort2_rtz(_X)  __builtin_convert(_X, ushort2, __kRoundTowardZero, __kUnsaturated )
    #define convert_int2_rtz(_X)     __builtin_convert(_X, int2, __kRoundTowardZero, __kUnsaturated )
    #define convert_uint2_rtz(_X)    __builtin_convert(_X, uint2, __kRoundTowardZero, __kUnsaturated )
    #define convert_long2_rtz(_X)    __builtin_convert(_X, long2, __kRoundTowardZero, __kUnsaturated )
    #define convert_ulong2_rtz(_X)   __builtin_convert(_X, ulong2, __kRoundTowardZero, __kUnsaturated )
    #define convert_float2_rtz(_X)   __builtin_convert(_X, float2, __kRoundTowardZero, __kUnsaturated )
    #define convert_double2_rtz(_X)  __builtin_convert(_X, double2, __kRoundTowardZero, __kUnsaturated )

    #define convert_char2_sat_rtz(_X)    __builtin_convert(_X, char2, __kRoundTowardZero, __kSaturated )
    #define convert_uchar2_sat_rtz(_X)   __builtin_convert(_X, uchar2, __kRoundTowardZero, __kSaturated )
    #define convert_short2_sat_rtz(_X)   __builtin_convert(_X, short2, __kRoundTowardZero, __kSaturated )
    #define convert_ushort2_sat_rtz(_X)  __builtin_convert(_X, ushort2, __kRoundTowardZero, __kSaturated )
    #define convert_int2_sat_rtz(_X)     __builtin_convert(_X, int2, __kRoundTowardZero, __kSaturated )
    #define convert_uint2_sat_rtz(_X)    __builtin_convert(_X, uint2, __kRoundTowardZero, __kSaturated )
    #define convert_long2_sat_rtz(_X)    __builtin_convert(_X, long2, __kRoundTowardZero, __kSaturated )
    #define convert_ulong2_sat_rtz(_X)   __builtin_convert(_X, ulong2, __kRoundTowardZero, __kSaturated )
    #define convert_float2_sat_rtz(_X)   __builtin_convert(_X, float2, __kRoundTowardZero, __kSaturated )
    #define convert_double2_sat_rtz(_X)  __builtin_convert(_X, double2, __kRoundTowardZero, __kSaturated )

    //type4
    #define convert_char4(_X)    __builtin_convert(_X, char4, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_uchar4(_X)   __builtin_convert(_X, uchar4, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_short4(_X)   __builtin_convert(_X, short4, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_ushort4(_X)  __builtin_convert(_X, ushort4, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_int4(_X)     __builtin_convert(_X, int4, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_uint4(_X)    __builtin_convert(_X, uint4, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_long4(_X)    __builtin_convert(_X, long4, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_ulong4(_X)   __builtin_convert(_X, ulong4, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_float4(_X)   __builtin_convert(_X, float4, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_double4(_X)  __builtin_convert(_X, double4, __kDefaultRoundingMode, __kUnsaturated )

    #define convert_char4_sat(_X)    __builtin_convert(_X, char4, __kDefaultRoundingMode, __kSaturated )
    #define convert_uchar4_sat(_X)   __builtin_convert(_X, uchar4, __kDefaultRoundingMode, __kSaturated )
    #define convert_short4_sat(_X)   __builtin_convert(_X, short4, __kDefaultRoundingMode, __kSaturated )
    #define convert_ushort4_sat(_X)  __builtin_convert(_X, ushort4, __kDefaultRoundingMode, __kSaturated )
    #define convert_int4_sat(_X)     __builtin_convert(_X, int4, __kDefaultRoundingMode, __kSaturated )
    #define convert_uint4_sat(_X)    __builtin_convert(_X, uint4, __kDefaultRoundingMode, __kSaturated )
    #define convert_long4_sat(_X)    __builtin_convert(_X, long4, __kDefaultRoundingMode, __kSaturated )
    #define convert_ulong4_sat(_X)   __builtin_convert(_X, ulong4, __kDefaultRoundingMode, __kSaturated )
    #define convert_float4_sat(_X)   __builtin_convert(_X, float4, __kDefaultRoundingMode, __kSaturated )
    #define convert_double4_sat(_X)  __builtin_convert(_X, double4, __kDefaultRoundingMode, __kSaturated )

    #define convert_char4_rte(_X)    __builtin_convert(_X, char4, __kRoundToNearestEven, __kUnsaturated )
    #define convert_uchar4_rte(_X)   __builtin_convert(_X, uchar4, __kRoundToNearestEven, __kUnsaturated )
    #define convert_short4_rte(_X)   __builtin_convert(_X, short4, __kRoundToNearestEven, __kUnsaturated )
    #define convert_ushort4_rte(_X)  __builtin_convert(_X, ushort4, __kRoundToNearestEven, __kUnsaturated )
    #define convert_int4_rte(_X)     __builtin_convert(_X, int4, __kRoundToNearestEven, __kUnsaturated )
    #define convert_uint4_rte(_X)    __builtin_convert(_X, uint4, __kRoundToNearestEven, __kUnsaturated )
    #define convert_long4_rte(_X)    __builtin_convert(_X, long4, __kRoundToNearestEven, __kUnsaturated )
    #define convert_ulong4_rte(_X)   __builtin_convert(_X, ulong4, __kRoundToNearestEven, __kUnsaturated )
    #define convert_float4_rte(_X)   __builtin_convert(_X, float4, __kRoundToNearestEven, __kUnsaturated )
    #define convert_double4_rte(_X)  __builtin_convert(_X, double4, __kRoundToNearestEven, __kUnsaturated )

    #define convert_char4_sat_rte(_X)    __builtin_convert(_X, char4, __kRoundToNearestEven, __kSaturated )
    #define convert_uchar4_sat_rte(_X)   __builtin_convert(_X, uchar4, __kRoundToNearestEven, __kSaturated )
    #define convert_short4_sat_rte(_X)   __builtin_convert(_X, short4, __kRoundToNearestEven, __kSaturated )
    #define convert_ushort4_sat_rte(_X)  __builtin_convert(_X, ushort4, __kRoundToNearestEven, __kSaturated )
    #define convert_int4_sat_rte(_X)     __builtin_convert(_X, int4, __kRoundToNearestEven, __kSaturated )
    #define convert_uint4_sat_rte(_X)    __builtin_convert(_X, uint4, __kRoundToNearestEven, __kSaturated )
    #define convert_long4_sat_rte(_X)    __builtin_convert(_X, long4, __kRoundToNearestEven, __kSaturated )
    #define convert_ulong4_sat_rte(_X)   __builtin_convert(_X, ulong4, __kRoundToNearestEven, __kSaturated )
    #define convert_float4_sat_rte(_X)   __builtin_convert(_X, float4, __kRoundToNearestEven, __kSaturated )
    #define convert_double4_sat_rte(_X)  __builtin_convert(_X, double4, __kRoundToNearestEven, __kSaturated )

    #define convert_char4_rtn(_X)    __builtin_convert(_X, char4, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_uchar4_rtn(_X)   __builtin_convert(_X, uchar4, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_short4_rtn(_X)   __builtin_convert(_X, short4, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_ushort4_rtn(_X)  __builtin_convert(_X, ushort4, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_int4_rtn(_X)     __builtin_convert(_X, int4, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_uint4_rtn(_X)    __builtin_convert(_X, uint4, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_long4_rtn(_X)    __builtin_convert(_X, long4, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_ulong4_rtn(_X)   __builtin_convert(_X, ulong4, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_float4_rtn(_X)   __builtin_convert(_X, float4, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_double4_rtn(_X)  __builtin_convert(_X, double4, __kRoundTowardNegativeInf, __kUnsaturated )

    #define convert_char4_sat_rtn(_X)    __builtin_convert(_X, char4, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_uchar4_sat_rtn(_X)   __builtin_convert(_X, uchar4, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_short4_sat_rtn(_X)   __builtin_convert(_X, short4, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_ushort4_sat_rtn(_X)  __builtin_convert(_X, ushort4, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_int4_sat_rtn(_X)     __builtin_convert(_X, int4, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_uint4_sat_rtn(_X)    __builtin_convert(_X, uint4, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_long4_sat_rtn(_X)    __builtin_convert(_X, long4, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_ulong4_sat_rtn(_X)   __builtin_convert(_X, ulong4, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_float4_sat_rtn(_X)   __builtin_convert(_X, float4, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_double4_sat_rtn(_X)  __builtin_convert(_X, double4, __kRoundTowardNegativeInf, __kSaturated )

    #define convert_char4_rtp(_X)    __builtin_convert(_X, char4, __kRoundTowardInf, __kUnsaturated )
    #define convert_uchar4_rtp(_X)   __builtin_convert(_X, uchar4, __kRoundTowardInf, __kUnsaturated )
    #define convert_short4_rtp(_X)   __builtin_convert(_X, short4, __kRoundTowardInf, __kUnsaturated )
    #define convert_ushort4_rtp(_X)  __builtin_convert(_X, ushort4, __kRoundTowardInf, __kUnsaturated )
    #define convert_int4_rtp(_X)     __builtin_convert(_X, int4, __kRoundTowardInf, __kUnsaturated )
    #define convert_uint4_rtp(_X)    __builtin_convert(_X, uint4, __kRoundTowardInf, __kUnsaturated )
    #define convert_long4_rtp(_X)    __builtin_convert(_X, long4, __kRoundTowardInf, __kUnsaturated )
    #define convert_ulong4_rtp(_X)   __builtin_convert(_X, ulong4, __kRoundTowardInf, __kUnsaturated )
    #define convert_float4_rtp(_X)   __builtin_convert(_X, float4, __kRoundTowardInf, __kUnsaturated )
    #define convert_double4_rtp(_X)  __builtin_convert(_X, double4, __kRoundTowardInf, __kUnsaturated )

    #define convert_char4_sat_rtp(_X)    __builtin_convert(_X, char4, __kRoundTowardInf, __kSaturated )
    #define convert_uchar4_sat_rtp(_X)   __builtin_convert(_X, uchar4, __kRoundTowardInf, __kSaturated )
    #define convert_short4_sat_rtp(_X)   __builtin_convert(_X, short4, __kRoundTowardInf, __kSaturated )
    #define convert_ushort4_sat_rtp(_X)  __builtin_convert(_X, ushort4, __kRoundTowardInf, __kSaturated )
    #define convert_int4_sat_rtp(_X)     __builtin_convert(_X, int4, __kRoundTowardInf, __kSaturated )
    #define convert_uint4_sat_rtp(_X)    __builtin_convert(_X, uint4, __kRoundTowardInf, __kSaturated )
    #define convert_long4_sat_rtp(_X)    __builtin_convert(_X, long4, __kRoundTowardInf, __kSaturated )
    #define convert_ulong4_sat_rtp(_X)   __builtin_convert(_X, ulong4, __kRoundTowardInf, __kSaturated )
    #define convert_float4_sat_rtp(_X)   __builtin_convert(_X, float4, __kRoundTowardInf, __kSaturated )
    #define convert_double4_sat_rtp(_X)  __builtin_convert(_X, double4, __kRoundTowardInf, __kSaturated )

    #define convert_char4_rtz(_X)    __builtin_convert(_X, char4, __kRoundTowardZero, __kUnsaturated )
    #define convert_uchar4_rtz(_X)   __builtin_convert(_X, uchar4, __kRoundTowardZero, __kUnsaturated )
    #define convert_short4_rtz(_X)   __builtin_convert(_X, short4, __kRoundTowardZero, __kUnsaturated )
    #define convert_ushort4_rtz(_X)  __builtin_convert(_X, ushort4, __kRoundTowardZero, __kUnsaturated )
    #define convert_int4_rtz(_X)     __builtin_convert(_X, int4, __kRoundTowardZero, __kUnsaturated )
    #define convert_uint4_rtz(_X)    __builtin_convert(_X, uint4, __kRoundTowardZero, __kUnsaturated )
    #define convert_long4_rtz(_X)    __builtin_convert(_X, long4, __kRoundTowardZero, __kUnsaturated )
    #define convert_ulong4_rtz(_X)   __builtin_convert(_X, ulong4, __kRoundTowardZero, __kUnsaturated )
    #define convert_float4_rtz(_X)   __builtin_convert(_X, float4, __kRoundTowardZero, __kUnsaturated )
    #define convert_double4_rtz(_X)  __builtin_convert(_X, double4, __kRoundTowardZero, __kUnsaturated )

    #define convert_char4_sat_rtz(_X)    __builtin_convert(_X, char4, __kRoundTowardZero, __kSaturated )
    #define convert_uchar4_sat_rtz(_X)   __builtin_convert(_X, uchar4, __kRoundTowardZero, __kSaturated )
    #define convert_short4_sat_rtz(_X)   __builtin_convert(_X, short4, __kRoundTowardZero, __kSaturated )
    #define convert_ushort4_sat_rtz(_X)  __builtin_convert(_X, ushort4, __kRoundTowardZero, __kSaturated )
    #define convert_int4_sat_rtz(_X)     __builtin_convert(_X, int4, __kRoundTowardZero, __kSaturated )
    #define convert_uint4_sat_rtz(_X)    __builtin_convert(_X, uint4, __kRoundTowardZero, __kSaturated )
    #define convert_long4_sat_rtz(_X)    __builtin_convert(_X, long4, __kRoundTowardZero, __kSaturated )
    #define convert_ulong4_sat_rtz(_X)   __builtin_convert(_X, ulong4, __kRoundTowardZero, __kSaturated )
    #define convert_float4_sat_rtz(_X)   __builtin_convert(_X, float4, __kRoundTowardZero, __kSaturated )
    #define convert_double4_sat_rtz(_X)  __builtin_convert(_X, double4, __kRoundTowardZero, __kSaturated )

    //type8
    #define convert_char8(_X)    __builtin_convert(_X, char8, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_uchar8(_X)   __builtin_convert(_X, uchar8, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_short8(_X)   __builtin_convert(_X, short8, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_ushort8(_X)  __builtin_convert(_X, ushort8, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_int8(_X)     __builtin_convert(_X, int8, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_uint8(_X)    __builtin_convert(_X, uint8, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_long8(_X)    __builtin_convert(_X, long8, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_ulong8(_X)   __builtin_convert(_X, ulong8, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_float8(_X)   __builtin_convert(_X, float8, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_double8(_X)  __builtin_convert(_X, double8, __kDefaultRoundingMode, __kUnsaturated )

    #define convert_char8_sat(_X)    __builtin_convert(_X, char8, __kDefaultRoundingMode, __kSaturated )
    #define convert_uchar8_sat(_X)   __builtin_convert(_X, uchar8, __kDefaultRoundingMode, __kSaturated )
    #define convert_short8_sat(_X)   __builtin_convert(_X, short8, __kDefaultRoundingMode, __kSaturated )
    #define convert_ushort8_sat(_X)  __builtin_convert(_X, ushort8, __kDefaultRoundingMode, __kSaturated )
    #define convert_int8_sat(_X)     __builtin_convert(_X, int8, __kDefaultRoundingMode, __kSaturated )
    #define convert_uint8_sat(_X)    __builtin_convert(_X, uint8, __kDefaultRoundingMode, __kSaturated )
    #define convert_long8_sat(_X)    __builtin_convert(_X, long8, __kDefaultRoundingMode, __kSaturated )
    #define convert_ulong8_sat(_X)   __builtin_convert(_X, ulong8, __kDefaultRoundingMode, __kSaturated )
    #define convert_float8_sat(_X)   __builtin_convert(_X, float8, __kDefaultRoundingMode, __kSaturated )
    #define convert_double8_sat(_X)  __builtin_convert(_X, double8, __kDefaultRoundingMode, __kSaturated )

    #define convert_char8_rte(_X)    __builtin_convert(_X, char8, __kRoundToNearestEven, __kUnsaturated )
    #define convert_uchar8_rte(_X)   __builtin_convert(_X, uchar8, __kRoundToNearestEven, __kUnsaturated )
    #define convert_short8_rte(_X)   __builtin_convert(_X, short8, __kRoundToNearestEven, __kUnsaturated )
    #define convert_ushort8_rte(_X)  __builtin_convert(_X, ushort8, __kRoundToNearestEven, __kUnsaturated )
    #define convert_int8_rte(_X)     __builtin_convert(_X, int8, __kRoundToNearestEven, __kUnsaturated )
    #define convert_uint8_rte(_X)    __builtin_convert(_X, uint8, __kRoundToNearestEven, __kUnsaturated )
    #define convert_long8_rte(_X)    __builtin_convert(_X, long8, __kRoundToNearestEven, __kUnsaturated )
    #define convert_ulong8_rte(_X)   __builtin_convert(_X, ulong8, __kRoundToNearestEven, __kUnsaturated )
    #define convert_float8_rte(_X)   __builtin_convert(_X, float8, __kRoundToNearestEven, __kUnsaturated )
    #define convert_double8_rte(_X)  __builtin_convert(_X, double8, __kRoundToNearestEven, __kUnsaturated )

    #define convert_char8_sat_rte(_X)    __builtin_convert(_X, char8, __kRoundToNearestEven, __kSaturated )
    #define convert_uchar8_sat_rte(_X)   __builtin_convert(_X, uchar8, __kRoundToNearestEven, __kSaturated )
    #define convert_short8_sat_rte(_X)   __builtin_convert(_X, short8, __kRoundToNearestEven, __kSaturated )
    #define convert_ushort8_sat_rte(_X)  __builtin_convert(_X, ushort8, __kRoundToNearestEven, __kSaturated )
    #define convert_int8_sat_rte(_X)     __builtin_convert(_X, int8, __kRoundToNearestEven, __kSaturated )
    #define convert_uint8_sat_rte(_X)    __builtin_convert(_X, uint8, __kRoundToNearestEven, __kSaturated )
    #define convert_long8_sat_rte(_X)    __builtin_convert(_X, long8, __kRoundToNearestEven, __kSaturated )
    #define convert_ulong8_sat_rte(_X)   __builtin_convert(_X, ulong8, __kRoundToNearestEven, __kSaturated )
    #define convert_float8_sat_rte(_X)   __builtin_convert(_X, float8, __kRoundToNearestEven, __kSaturated )
    #define convert_double8_sat_rte(_X)  __builtin_convert(_X, double8, __kRoundToNearestEven, __kSaturated )

    #define convert_char8_rtn(_X)    __builtin_convert(_X, char8, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_uchar8_rtn(_X)   __builtin_convert(_X, uchar8, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_short8_rtn(_X)   __builtin_convert(_X, short8, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_ushort8_rtn(_X)  __builtin_convert(_X, ushort8, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_int8_rtn(_X)     __builtin_convert(_X, int8, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_uint8_rtn(_X)    __builtin_convert(_X, uint8, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_long8_rtn(_X)    __builtin_convert(_X, long8, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_ulong8_rtn(_X)   __builtin_convert(_X, ulong8, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_float8_rtn(_X)   __builtin_convert(_X, float8, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_double8_rtn(_X)  __builtin_convert(_X, double8, __kRoundTowardNegativeInf, __kUnsaturated )

    #define convert_char8_sat_rtn(_X)    __builtin_convert(_X, char8, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_uchar8_sat_rtn(_X)   __builtin_convert(_X, uchar8, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_short8_sat_rtn(_X)   __builtin_convert(_X, short8, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_ushort8_sat_rtn(_X)  __builtin_convert(_X, ushort8, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_int8_sat_rtn(_X)     __builtin_convert(_X, int8, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_uint8_sat_rtn(_X)    __builtin_convert(_X, uint8, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_long8_sat_rtn(_X)    __builtin_convert(_X, long8, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_ulong8_sat_rtn(_X)   __builtin_convert(_X, ulong8, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_float8_sat_rtn(_X)   __builtin_convert(_X, float8, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_double8_sat_rtn(_X)  __builtin_convert(_X, double8, __kRoundTowardNegativeInf, __kSaturated )

    #define convert_char8_rtp(_X)    __builtin_convert(_X, char8, __kRoundTowardInf, __kUnsaturated )
    #define convert_uchar8_rtp(_X)   __builtin_convert(_X, uchar8, __kRoundTowardInf, __kUnsaturated )
    #define convert_short8_rtp(_X)   __builtin_convert(_X, short8, __kRoundTowardInf, __kUnsaturated )
    #define convert_ushort8_rtp(_X)  __builtin_convert(_X, ushort8, __kRoundTowardInf, __kUnsaturated )
    #define convert_int8_rtp(_X)     __builtin_convert(_X, int8, __kRoundTowardInf, __kUnsaturated )
    #define convert_uint8_rtp(_X)    __builtin_convert(_X, uint8, __kRoundTowardInf, __kUnsaturated )
    #define convert_long8_rtp(_X)    __builtin_convert(_X, long8, __kRoundTowardInf, __kUnsaturated )
    #define convert_ulong8_rtp(_X)   __builtin_convert(_X, ulong8, __kRoundTowardInf, __kUnsaturated )
    #define convert_float8_rtp(_X)   __builtin_convert(_X, float8, __kRoundTowardInf, __kUnsaturated )
    #define convert_double8_rtp(_X)  __builtin_convert(_X, double8, __kRoundTowardInf, __kUnsaturated )

    #define convert_char8_sat_rtp(_X)    __builtin_convert(_X, char8, __kRoundTowardInf, __kSaturated )
    #define convert_uchar8_sat_rtp(_X)   __builtin_convert(_X, uchar8, __kRoundTowardInf, __kSaturated )
    #define convert_short8_sat_rtp(_X)   __builtin_convert(_X, short8, __kRoundTowardInf, __kSaturated )
    #define convert_ushort8_sat_rtp(_X)  __builtin_convert(_X, ushort8, __kRoundTowardInf, __kSaturated )
    #define convert_int8_sat_rtp(_X)     __builtin_convert(_X, int8, __kRoundTowardInf, __kSaturated )
    #define convert_uint8_sat_rtp(_X)    __builtin_convert(_X, uint8, __kRoundTowardInf, __kSaturated )
    #define convert_long8_sat_rtp(_X)    __builtin_convert(_X, long8, __kRoundTowardInf, __kSaturated )
    #define convert_ulong8_sat_rtp(_X)   __builtin_convert(_X, ulong8, __kRoundTowardInf, __kSaturated )
    #define convert_float8_sat_rtp(_X)   __builtin_convert(_X, float8, __kRoundTowardInf, __kSaturated )
    #define convert_double8_sat_rtp(_X)  __builtin_convert(_X, double8, __kRoundTowardInf, __kSaturated )

    #define convert_char8_rtz(_X)    __builtin_convert(_X, char8, __kRoundTowardZero, __kUnsaturated )
    #define convert_uchar8_rtz(_X)   __builtin_convert(_X, uchar8, __kRoundTowardZero, __kUnsaturated )
    #define convert_short8_rtz(_X)   __builtin_convert(_X, short8, __kRoundTowardZero, __kUnsaturated )
    #define convert_ushort8_rtz(_X)  __builtin_convert(_X, ushort8, __kRoundTowardZero, __kUnsaturated )
    #define convert_int8_rtz(_X)     __builtin_convert(_X, int8, __kRoundTowardZero, __kUnsaturated )
    #define convert_uint8_rtz(_X)    __builtin_convert(_X, uint8, __kRoundTowardZero, __kUnsaturated )
    #define convert_long8_rtz(_X)    __builtin_convert(_X, long8, __kRoundTowardZero, __kUnsaturated )
    #define convert_ulong8_rtz(_X)   __builtin_convert(_X, ulong8, __kRoundTowardZero, __kUnsaturated )
    #define convert_float8_rtz(_X)   __builtin_convert(_X, float8, __kRoundTowardZero, __kUnsaturated )
    #define convert_double8_rtz(_X)  __builtin_convert(_X, double8, __kRoundTowardZero, __kUnsaturated )

    #define convert_char8_sat_rtz(_X)    __builtin_convert(_X, char8, __kRoundTowardZero, __kSaturated )
    #define convert_uchar8_sat_rtz(_X)   __builtin_convert(_X, uchar8, __kRoundTowardZero, __kSaturated )
    #define convert_short8_sat_rtz(_X)   __builtin_convert(_X, short8, __kRoundTowardZero, __kSaturated )
    #define convert_ushort8_sat_rtz(_X)  __builtin_convert(_X, ushort8, __kRoundTowardZero, __kSaturated )
    #define convert_int8_sat_rtz(_X)     __builtin_convert(_X, int8, __kRoundTowardZero, __kSaturated )
    #define convert_uint8_sat_rtz(_X)    __builtin_convert(_X, uint8, __kRoundTowardZero, __kSaturated )
    #define convert_long8_sat_rtz(_X)    __builtin_convert(_X, long8, __kRoundTowardZero, __kSaturated )
    #define convert_ulong8_sat_rtz(_X)   __builtin_convert(_X, ulong8, __kRoundTowardZero, __kSaturated )
    #define convert_float8_sat_rtz(_X)   __builtin_convert(_X, float8, __kRoundTowardZero, __kSaturated )
    #define convert_double8_sat_rtz(_X)  __builtin_convert(_X, double8, __kRoundTowardZero, __kSaturated )

    //type16
    #define convert_char16(_X)    __builtin_convert(_X, char16, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_uchar16(_X)   __builtin_convert(_X, uchar16, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_short16(_X)   __builtin_convert(_X, short16, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_ushort16(_X)  __builtin_convert(_X, ushort16, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_int16(_X)     __builtin_convert(_X, int16, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_uint16(_X)    __builtin_convert(_X, uint16, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_long16(_X)    __builtin_convert(_X, long16, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_ulong16(_X)   __builtin_convert(_X, ulong16, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_float16(_X)   __builtin_convert(_X, float16, __kDefaultRoundingMode, __kUnsaturated )
    #define convert_double16(_X)  __builtin_convert(_X, double16, __kDefaultRoundingMode, __kUnsaturated )

    #define convert_char16_sat(_X)    __builtin_convert(_X, char16, __kDefaultRoundingMode, __kSaturated )
    #define convert_uchar16_sat(_X)   __builtin_convert(_X, uchar16, __kDefaultRoundingMode, __kSaturated )
    #define convert_short16_sat(_X)   __builtin_convert(_X, short16, __kDefaultRoundingMode, __kSaturated )
    #define convert_ushort16_sat(_X)  __builtin_convert(_X, ushort16, __kDefaultRoundingMode, __kSaturated )
    #define convert_int16_sat(_X)     __builtin_convert(_X, int16, __kDefaultRoundingMode, __kSaturated )
    #define convert_uint16_sat(_X)    __builtin_convert(_X, uint16, __kDefaultRoundingMode, __kSaturated )
    #define convert_long16_sat(_X)    __builtin_convert(_X, long16, __kDefaultRoundingMode, __kSaturated )
    #define convert_ulong16_sat(_X)   __builtin_convert(_X, ulong16, __kDefaultRoundingMode, __kSaturated )
    #define convert_float16_sat(_X)   __builtin_convert(_X, float16, __kDefaultRoundingMode, __kSaturated )
    #define convert_double16_sat(_X)  __builtin_convert(_X, double16, __kDefaultRoundingMode, __kSaturated )

    #define convert_char16_rte(_X)    __builtin_convert(_X, char16, __kRoundToNearestEven, __kUnsaturated )
    #define convert_uchar16_rte(_X)   __builtin_convert(_X, uchar16, __kRoundToNearestEven, __kUnsaturated )
    #define convert_short16_rte(_X)   __builtin_convert(_X, short16, __kRoundToNearestEven, __kUnsaturated )
    #define convert_ushort16_rte(_X)  __builtin_convert(_X, ushort16, __kRoundToNearestEven, __kUnsaturated )
    #define convert_int16_rte(_X)     __builtin_convert(_X, int16, __kRoundToNearestEven, __kUnsaturated )
    #define convert_uint16_rte(_X)    __builtin_convert(_X, uint16, __kRoundToNearestEven, __kUnsaturated )
    #define convert_long16_rte(_X)    __builtin_convert(_X, long16, __kRoundToNearestEven, __kUnsaturated )
    #define convert_ulong16_rte(_X)   __builtin_convert(_X, ulong16, __kRoundToNearestEven, __kUnsaturated )
    #define convert_float16_rte(_X)   __builtin_convert(_X, float16, __kRoundToNearestEven, __kUnsaturated )
    #define convert_double16_rte(_X)  __builtin_convert(_X, double16, __kRoundToNearestEven, __kUnsaturated )

    #define convert_char16_sat_rte(_X)    __builtin_convert(_X, char16, __kRoundToNearestEven, __kSaturated )
    #define convert_uchar16_sat_rte(_X)   __builtin_convert(_X, uchar16, __kRoundToNearestEven, __kSaturated )
    #define convert_short16_sat_rte(_X)   __builtin_convert(_X, short16, __kRoundToNearestEven, __kSaturated )
    #define convert_ushort16_sat_rte(_X)  __builtin_convert(_X, ushort16, __kRoundToNearestEven, __kSaturated )
    #define convert_int16_sat_rte(_X)     __builtin_convert(_X, int16, __kRoundToNearestEven, __kSaturated )
    #define convert_uint16_sat_rte(_X)    __builtin_convert(_X, uint16, __kRoundToNearestEven, __kSaturated )
    #define convert_long16_sat_rte(_X)    __builtin_convert(_X, long16, __kRoundToNearestEven, __kSaturated )
    #define convert_ulong16_sat_rte(_X)   __builtin_convert(_X, ulong16, __kRoundToNearestEven, __kSaturated )
    #define convert_float16_sat_rte(_X)   __builtin_convert(_X, float16, __kRoundToNearestEven, __kSaturated )
    #define convert_double16_sat_rte(_X)  __builtin_convert(_X, double16, __kRoundToNearestEven, __kSaturated )

    #define convert_char16_rtn(_X)    __builtin_convert(_X, char16, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_uchar16_rtn(_X)   __builtin_convert(_X, uchar16, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_short16_rtn(_X)   __builtin_convert(_X, short16, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_ushort16_rtn(_X)  __builtin_convert(_X, ushort16, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_int16_rtn(_X)     __builtin_convert(_X, int16, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_uint16_rtn(_X)    __builtin_convert(_X, uint16, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_long16_rtn(_X)    __builtin_convert(_X, long16, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_ulong16_rtn(_X)   __builtin_convert(_X, ulong16, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_float16_rtn(_X)   __builtin_convert(_X, float16, __kRoundTowardNegativeInf, __kUnsaturated )
    #define convert_double16_rtn(_X)  __builtin_convert(_X, double16, __kRoundTowardNegativeInf, __kUnsaturated )

    #define convert_char16_sat_rtn(_X)    __builtin_convert(_X, char16, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_uchar16_sat_rtn(_X)   __builtin_convert(_X, uchar16, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_short16_sat_rtn(_X)   __builtin_convert(_X, short16, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_ushort16_sat_rtn(_X)  __builtin_convert(_X, ushort16, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_int16_sat_rtn(_X)     __builtin_convert(_X, int16, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_uint16_sat_rtn(_X)    __builtin_convert(_X, uint16, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_long16_sat_rtn(_X)    __builtin_convert(_X, long16, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_ulong16_sat_rtn(_X)   __builtin_convert(_X, ulong16, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_float16_sat_rtn(_X)   __builtin_convert(_X, float16, __kRoundTowardNegativeInf, __kSaturated )
    #define convert_double16_sat_rtn(_X)  __builtin_convert(_X, double16, __kRoundTowardNegativeInf, __kSaturated )

    #define convert_char16_rtp(_X)    __builtin_convert(_X, char16, __kRoundTowardInf, __kUnsaturated )
    #define convert_uchar16_rtp(_X)   __builtin_convert(_X, uchar16, __kRoundTowardInf, __kUnsaturated )
    #define convert_short16_rtp(_X)   __builtin_convert(_X, short16, __kRoundTowardInf, __kUnsaturated )
    #define convert_ushort16_rtp(_X)  __builtin_convert(_X, ushort16, __kRoundTowardInf, __kUnsaturated )
    #define convert_int16_rtp(_X)     __builtin_convert(_X, int16, __kRoundTowardInf, __kUnsaturated )
    #define convert_uint16_rtp(_X)    __builtin_convert(_X, uint16, __kRoundTowardInf, __kUnsaturated )
    #define convert_long16_rtp(_X)    __builtin_convert(_X, long16, __kRoundTowardInf, __kUnsaturated )
    #define convert_ulong16_rtp(_X)   __builtin_convert(_X, ulong16, __kRoundTowardInf, __kUnsaturated )
    #define convert_float16_rtp(_X)   __builtin_convert(_X, float16, __kRoundTowardInf, __kUnsaturated )
    #define convert_double16_rtp(_X)  __builtin_convert(_X, double16, __kRoundTowardInf, __kUnsaturated )

    #define convert_char16_sat_rtp(_X)    __builtin_convert(_X, char16, __kRoundTowardInf, __kSaturated )
    #define convert_uchar16_sat_rtp(_X)   __builtin_convert(_X, uchar16, __kRoundTowardInf, __kSaturated )
    #define convert_short16_sat_rtp(_X)   __builtin_convert(_X, short16, __kRoundTowardInf, __kSaturated )
    #define convert_ushort16_sat_rtp(_X)  __builtin_convert(_X, ushort16, __kRoundTowardInf, __kSaturated )
    #define convert_int16_sat_rtp(_X)     __builtin_convert(_X, int16, __kRoundTowardInf, __kSaturated )
    #define convert_uint16_sat_rtp(_X)    __builtin_convert(_X, uint16, __kRoundTowardInf, __kSaturated )
    #define convert_long16_sat_rtp(_X)    __builtin_convert(_X, long16, __kRoundTowardInf, __kSaturated )
    #define convert_ulong16_sat_rtp(_X)   __builtin_convert(_X, ulong16, __kRoundTowardInf, __kSaturated )
    #define convert_float16_sat_rtp(_X)   __builtin_convert(_X, float16, __kRoundTowardInf, __kSaturated )
    #define convert_double16_sat_rtp(_X)  __builtin_convert(_X, double16, __kRoundTowardInf, __kSaturated )

    #define convert_char16_rtz(_X)    __builtin_convert(_X, char16, __kRoundTowardZero, __kUnsaturated )
    #define convert_uchar16_rtz(_X)   __builtin_convert(_X, uchar16, __kRoundTowardZero, __kUnsaturated )
    #define convert_short16_rtz(_X)   __builtin_convert(_X, short16, __kRoundTowardZero, __kUnsaturated )
    #define convert_ushort16_rtz(_X)  __builtin_convert(_X, ushort16, __kRoundTowardZero, __kUnsaturated )
    #define convert_int16_rtz(_X)     __builtin_convert(_X, int16, __kRoundTowardZero, __kUnsaturated )
    #define convert_uint16_rtz(_X)    __builtin_convert(_X, uint16, __kRoundTowardZero, __kUnsaturated )
    #define convert_long16_rtz(_X)    __builtin_convert(_X, long16, __kRoundTowardZero, __kUnsaturated )
    #define convert_ulong16_rtz(_X)   __builtin_convert(_X, ulong16, __kRoundTowardZero, __kUnsaturated )
    #define convert_float16_rtz(_X)   __builtin_convert(_X, float16, __kRoundTowardZero, __kUnsaturated )
    #define convert_double16_rtz(_X)  __builtin_convert(_X, double16, __kRoundTowardZero, __kUnsaturated )

    #define convert_char16_sat_rtz(_X)    __builtin_convert(_X, char16, __kRoundTowardZero, __kSaturated )
    #define convert_uchar16_sat_rtz(_X)   __builtin_convert(_X, uchar16, __kRoundTowardZero, __kSaturated )
    #define convert_short16_sat_rtz(_X)   __builtin_convert(_X, short16, __kRoundTowardZero, __kSaturated )
    #define convert_ushort16_sat_rtz(_X)  __builtin_convert(_X, ushort16, __kRoundTowardZero, __kSaturated )
    #define convert_int16_sat_rtz(_X)     __builtin_convert(_X, int16, __kRoundTowardZero, __kSaturated )
    #define convert_uint16_sat_rtz(_X)    __builtin_convert(_X, uint16, __kRoundTowardZero, __kSaturated )
    #define convert_long16_sat_rtz(_X)    __builtin_convert(_X, long16, __kRoundTowardZero, __kSaturated )
    #define convert_ulong16_sat_rtz(_X)   __builtin_convert(_X, ulong16, __kRoundTowardZero, __kSaturated )
    #define convert_float16_sat_rtz(_X)   __builtin_convert(_X, float16, __kRoundTowardZero, __kSaturated )
    #define convert_double16_sat_rtz(_X)  __builtin_convert(_X, double16, __kRoundTowardZero, __kSaturated )


#else

    #define convert_uchar(_X)    __builtin_overload( 1, _X, __convert_uchar_uchar, __convert_uchar_ushort, __convert_uchar_uint, __convert_uchar_ulong, __convert_uchar_char, __convert_uchar_short, __convert_uchar_int, __convert_uchar_long, __convert_uchar_float, __convert_uchar_double )
    #define convert_uchar2(_X)    __builtin_overload( 1, _X, __convert_uchar2_uchar2, __convert_uchar2_ushort2, __convert_uchar2_uint2, __convert_uchar2_ulong2, __convert_uchar2_char2, __convert_uchar2_short2, __convert_uchar2_int2, __convert_uchar2_long2, __convert_uchar2_float2, __convert_uchar2_double2 )
    #define convert_uchar4(_X)    __builtin_overload( 1, _X, __convert_uchar4_uchar4, __convert_uchar4_ushort4, __convert_uchar4_uint4, __convert_uchar4_ulong4, __convert_uchar4_char4, __convert_uchar4_short4, __convert_uchar4_int4, __convert_uchar4_long4, __convert_uchar4_float4, __convert_uchar4_double4 )
    #define convert_uchar8(_X)    __builtin_overload( 1, _X, __convert_uchar8_uchar8, __convert_uchar8_ushort8, __convert_uchar8_uint8, __convert_uchar8_ulong8, __convert_uchar8_char8, __convert_uchar8_short8, __convert_uchar8_int8, __convert_uchar8_long8, __convert_uchar8_float8, __convert_uchar8_double8 )
    #define convert_uchar16(_X)    __builtin_overload( 1, _X, __convert_uchar16_uchar16, __convert_uchar16_ushort16, __convert_uchar16_uint16, __convert_uchar16_ulong16, __convert_uchar16_char16, __convert_uchar16_short16, __convert_uchar16_int16, __convert_uchar16_long16, __convert_uchar16_float16, __convert_uchar16_double16 )
    #define convert_uchar_rte(_X)    __builtin_overload( 1, _X, __convert_uchar_rte_uchar, __convert_uchar_ushort, __convert_uchar_uint, __convert_uchar_ulong, __convert_uchar_rte_char, __convert_uchar_rte_short, __convert_uchar_rte_int, __convert_uchar_rte_long, __convert_uchar_rte_float, __convert_uchar_rte_double )
    #define convert_uchar2_rte(_X)    __builtin_overload( 1, _X, __convert_uchar2_rte_uchar2, __convert_uchar2_rte_ushort2, __convert_uchar2_rte_uint2, __convert_uchar2_ulong2, __convert_uchar2_rte_char2, __convert_uchar2_rte_short2, __convert_uchar2_rte_int2, __convert_uchar2_rte_long2, __convert_uchar2_rte_float2, __convert_uchar2_rte_double2 )
    #define convert_uchar4_rte(_X)    __builtin_overload( 1, _X, __convert_uchar4_rte_uchar4, __convert_uchar4_rte_ushort4, __convert_uchar4_uint4, __convert_uchar4_ulong4, __convert_uchar4_rte_char4, __convert_uchar4_rte_short4, __convert_uchar4_rte_int4, __convert_uchar4_rte_long4, __convert_uchar4_rte_float4, __convert_uchar4_rte_double4 )
    #define convert_uchar8_rte(_X)    __builtin_overload( 1, _X, __convert_uchar8_rte_uchar8, __convert_uchar8_ushort8, __convert_uchar8_uint8, __convert_uchar8_ulong8, __convert_uchar8_rte_char8, __convert_uchar8_rte_short8, __convert_uchar8_rte_int8, __convert_uchar8_rte_long8, __convert_uchar8_rte_float8, __convert_uchar8_rte_double8 )
    #define convert_uchar16_rte(_X)    __builtin_overload( 1, _X, __convert_uchar16_rte_uchar16, __convert_uchar16_ushort16, __convert_uchar16_uint16, __convert_uchar16_ulong16, __convert_uchar16_rte_char16, __convert_uchar16_rte_short16, __convert_uchar16_rte_int16, __convert_uchar16_rte_long16, __convert_uchar16_rte_float16, __convert_uchar16_rte_double16 )
    #define convert_uchar_rtp(_X)    __builtin_overload( 1, _X, __convert_uchar_rtp_uchar, __convert_uchar_ushort, __convert_uchar_uint, __convert_uchar_ulong, __convert_uchar_rtp_char, __convert_uchar_rtp_short, __convert_uchar_rtp_int, __convert_uchar_rtp_long, __convert_uchar_rtp_float, __convert_uchar_rtp_double )
    #define convert_uchar2_rtp(_X)    __builtin_overload( 1, _X, __convert_uchar2_rtp_uchar2, __convert_uchar2_rtp_ushort2, __convert_uchar2_rtp_uint2, __convert_uchar2_ulong2, __convert_uchar2_rtp_char2, __convert_uchar2_rtp_short2, __convert_uchar2_rtp_int2, __convert_uchar2_rtp_long2, __convert_uchar2_rtp_float2, __convert_uchar2_rtp_double2 )
    #define convert_uchar4_rtp(_X)    __builtin_overload( 1, _X, __convert_uchar4_rtp_uchar4, __convert_uchar4_rtp_ushort4, __convert_uchar4_uint4, __convert_uchar4_ulong4, __convert_uchar4_rtp_char4, __convert_uchar4_rtp_short4, __convert_uchar4_rtp_int4, __convert_uchar4_rtp_long4, __convert_uchar4_rtp_float4, __convert_uchar4_rtp_double4 )
    #define convert_uchar8_rtp(_X)    __builtin_overload( 1, _X, __convert_uchar8_rtp_uchar8, __convert_uchar8_ushort8, __convert_uchar8_uint8, __convert_uchar8_ulong8, __convert_uchar8_rtp_char8, __convert_uchar8_rtp_short8, __convert_uchar8_rtp_int8, __convert_uchar8_rtp_long8, __convert_uchar8_rtp_float8, __convert_uchar8_rtp_double8 )
    #define convert_uchar16_rtp(_X)    __builtin_overload( 1, _X, __convert_uchar16_rtp_uchar16, __convert_uchar16_ushort16, __convert_uchar16_uint16, __convert_uchar16_ulong16, __convert_uchar16_rtp_char16, __convert_uchar16_rtp_short16, __convert_uchar16_rtp_int16, __convert_uchar16_rtp_long16, __convert_uchar16_rtp_float16, __convert_uchar16_rtp_double16 )
    #define convert_uchar_rtn(_X)    __builtin_overload( 1, _X, __convert_uchar_rtn_uchar, __convert_uchar_ushort, __convert_uchar_uint, __convert_uchar_ulong, __convert_uchar_rtn_char, __convert_uchar_rtn_short, __convert_uchar_rtn_int, __convert_uchar_rtn_long, __convert_uchar_rtn_float, __convert_uchar_rtn_double )
    #define convert_uchar2_rtn(_X)    __builtin_overload( 1, _X, __convert_uchar2_rtn_uchar2, __convert_uchar2_rtn_ushort2, __convert_uchar2_rtn_uint2, __convert_uchar2_ulong2, __convert_uchar2_rtn_char2, __convert_uchar2_rtn_short2, __convert_uchar2_rtn_int2, __convert_uchar2_rtn_long2, __convert_uchar2_rtn_float2, __convert_uchar2_rtn_double2 )
    #define convert_uchar4_rtn(_X)    __builtin_overload( 1, _X, __convert_uchar4_rtn_uchar4, __convert_uchar4_rtn_ushort4, __convert_uchar4_uint4, __convert_uchar4_ulong4, __convert_uchar4_rtn_char4, __convert_uchar4_rtn_short4, __convert_uchar4_rtn_int4, __convert_uchar4_rtn_long4, __convert_uchar4_rtn_float4, __convert_uchar4_rtn_double4 )
    #define convert_uchar8_rtn(_X)    __builtin_overload( 1, _X, __convert_uchar8_rtn_uchar8, __convert_uchar8_ushort8, __convert_uchar8_uint8, __convert_uchar8_ulong8, __convert_uchar8_rtn_char8, __convert_uchar8_rtn_short8, __convert_uchar8_rtn_int8, __convert_uchar8_rtn_long8, __convert_uchar8_rtn_float8, __convert_uchar8_rtn_double8 )
    #define convert_uchar16_rtn(_X)    __builtin_overload( 1, _X, __convert_uchar16_rtn_uchar16, __convert_uchar16_ushort16, __convert_uchar16_uint16, __convert_uchar16_ulong16, __convert_uchar16_rtn_char16, __convert_uchar16_rtn_short16, __convert_uchar16_rtn_int16, __convert_uchar16_rtn_long16, __convert_uchar16_rtn_float16, __convert_uchar16_rtn_double16 )
    #define convert_uchar_rtz(_X)    __builtin_overload( 1, _X, __convert_uchar_rtz_uchar, __convert_uchar_ushort, __convert_uchar_uint, __convert_uchar_ulong, __convert_uchar_rtz_char, __convert_uchar_rtz_short, __convert_uchar_rtz_int, __convert_uchar_rtz_long, __convert_uchar_rtz_float, __convert_uchar_rtz_double )
    #define convert_uchar2_rtz(_X)    __builtin_overload( 1, _X, __convert_uchar2_rtz_uchar2, __convert_uchar2_rtz_ushort2, __convert_uchar2_rtz_uint2, __convert_uchar2_ulong2, __convert_uchar2_rtz_char2, __convert_uchar2_rtz_short2, __convert_uchar2_rtz_int2, __convert_uchar2_rtz_long2, __convert_uchar2_rtz_float2, __convert_uchar2_rtz_double2 )
    #define convert_uchar4_rtz(_X)    __builtin_overload( 1, _X, __convert_uchar4_rtz_uchar4, __convert_uchar4_rtz_ushort4, __convert_uchar4_uint4, __convert_uchar4_ulong4, __convert_uchar4_rtz_char4, __convert_uchar4_rtz_short4, __convert_uchar4_rtz_int4, __convert_uchar4_rtz_long4, __convert_uchar4_rtz_float4, __convert_uchar4_rtz_double4 )
    #define convert_uchar8_rtz(_X)    __builtin_overload( 1, _X, __convert_uchar8_rtz_uchar8, __convert_uchar8_ushort8, __convert_uchar8_uint8, __convert_uchar8_ulong8, __convert_uchar8_rtz_char8, __convert_uchar8_rtz_short8, __convert_uchar8_rtz_int8, __convert_uchar8_rtz_long8, __convert_uchar8_rtz_float8, __convert_uchar8_rtz_double8 )
    #define convert_uchar16_rtz(_X)    __builtin_overload( 1, _X, __convert_uchar16_rtz_uchar16, __convert_uchar16_ushort16, __convert_uchar16_uint16, __convert_uchar16_ulong16, __convert_uchar16_rtz_char16, __convert_uchar16_rtz_short16, __convert_uchar16_rtz_int16, __convert_uchar16_rtz_long16, __convert_uchar16_rtz_float16, __convert_uchar16_rtz_double16 )
    #define convert_uchar_sat(_X)    __builtin_overload( 1, _X, __convert_uchar_sat_uchar, __convert_uchar_sat_ushort, __convert_uchar_sat_uint, __convert_uchar_sat_ulong, __convert_uchar_sat_char, __convert_uchar_sat_short, __convert_uchar_sat_int, __convert_uchar_sat_long, __convert_uchar_sat_float, __convert_uchar_sat_double )
    #define convert_uchar2_sat(_X)    __builtin_overload( 1, _X, __convert_uchar2_sat_uchar2, __convert_uchar2_sat_ushort2, __convert_uchar2_sat_uint2, __convert_uchar2_sat_ulong2, __convert_uchar2_sat_char2, __convert_uchar2_sat_short2, __convert_uchar2_sat_int2, __convert_uchar2_sat_long2, __convert_uchar2_sat_float2, __convert_uchar2_sat_double2 )
    #define convert_uchar4_sat(_X)    __builtin_overload( 1, _X, __convert_uchar4_sat_uchar4, __convert_uchar4_sat_ushort4, __convert_uchar4_sat_uint4, __convert_uchar4_sat_ulong4, __convert_uchar4_sat_char4, __convert_uchar4_sat_short4, __convert_uchar4_sat_int4, __convert_uchar4_sat_long4, __convert_uchar4_sat_float4, __convert_uchar4_sat_double4 )
    #define convert_uchar8_sat(_X)    __builtin_overload( 1, _X, __convert_uchar8_sat_uchar8, __convert_uchar8_sat_ushort8, __convert_uchar8_sat_uint8, __convert_uchar8_sat_ulong8, __convert_uchar8_sat_char8, __convert_uchar8_sat_short8, __convert_uchar8_sat_int8, __convert_uchar8_sat_long8, __convert_uchar8_sat_float8, __convert_uchar8_sat_double8 )
    #define convert_uchar16_sat(_X)    __builtin_overload( 1, _X, __convert_uchar16_sat_uchar16, __convert_uchar16_sat_ushort16, __convert_uchar16_sat_uint16, __convert_uchar16_sat_ulong16, __convert_uchar16_sat_char16, __convert_uchar16_sat_short16, __convert_uchar16_sat_int16, __convert_uchar16_sat_long16, __convert_uchar16_sat_float16, __convert_uchar16_sat_double16 )
    #define convert_uchar_sat_rte(_X)    __builtin_overload( 1, _X, __convert_uchar_sat_rte_uchar, __convert_uchar_sat_ushort, __convert_uchar_sat_rte_uint, __convert_uchar_sat_rte_ulong, __convert_uchar_sat_char, __convert_uchar_sat_short, __convert_uchar_sat_rte_int, __convert_uchar_sat_rte_long, __convert_uchar_sat_rte_float, __convert_uchar_sat_rte_double )
    #define convert_uchar2_sat_rte(_X)    __builtin_overload( 1, _X, __convert_uchar2_sat_rte_uchar2, __convert_uchar2_sat_rte_ushort2, __convert_uchar2_sat_rte_uint2, __convert_uchar2_sat_rte_ulong2, __convert_uchar2_sat_rte_char2, __convert_uchar2_sat_rte_short2, __convert_uchar2_sat_rte_int2, __convert_uchar2_sat_rte_long2, __convert_uchar2_sat_rte_float2, __convert_uchar2_sat_rte_double2 )
    #define convert_uchar4_sat_rte(_X)    __builtin_overload( 1, _X, __convert_uchar4_sat_rte_uchar4, __convert_uchar4_sat_rte_ushort4, __convert_uchar4_sat_rte_uint4, __convert_uchar4_sat_rte_ulong4, __convert_uchar4_sat_rte_char4, __convert_uchar4_sat_rte_short4, __convert_uchar4_sat_rte_int4, __convert_uchar4_sat_rte_long4, __convert_uchar4_sat_rte_float4, __convert_uchar4_sat_rte_double4 )
    #define convert_uchar8_sat_rte(_X)    __builtin_overload( 1, _X, __convert_uchar8_sat_rte_uchar8, __convert_uchar8_sat_ushort8, __convert_uchar8_sat_rte_uint8, __convert_uchar8_sat_rte_ulong8, __convert_uchar8_sat_rte_char8, __convert_uchar8_sat_short8, __convert_uchar8_sat_rte_int8, __convert_uchar8_sat_rte_long8, __convert_uchar8_sat_rte_float8, __convert_uchar8_sat_rte_double8 )
    #define convert_uchar16_sat_rte(_X)    __builtin_overload( 1, _X, __convert_uchar16_sat_rte_uchar16, __convert_uchar16_sat_ushort16, __convert_uchar16_sat_rte_uint16, __convert_uchar16_sat_rte_ulong16, __convert_uchar16_sat_char16, __convert_uchar16_sat_short16, __convert_uchar16_sat_rte_int16, __convert_uchar16_sat_rte_long16, __convert_uchar16_sat_rte_float16, __convert_uchar16_sat_rte_double16 )
    #define convert_uchar_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_uchar_sat_rtp_uchar, __convert_uchar_sat_ushort, __convert_uchar_sat_rtp_uint, __convert_uchar_sat_rtp_ulong, __convert_uchar_sat_char, __convert_uchar_sat_short, __convert_uchar_sat_rtp_int, __convert_uchar_sat_rtp_long, __convert_uchar_sat_rtp_float, __convert_uchar_sat_rtp_double )
    #define convert_uchar2_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_uchar2_sat_rtp_uchar2, __convert_uchar2_sat_rtp_ushort2, __convert_uchar2_sat_rtp_uint2, __convert_uchar2_sat_rtp_ulong2, __convert_uchar2_sat_rtp_char2, __convert_uchar2_sat_rtp_short2, __convert_uchar2_sat_rtp_int2, __convert_uchar2_sat_rtp_long2, __convert_uchar2_sat_rtp_float2, __convert_uchar2_sat_rtp_double2 )
    #define convert_uchar4_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_uchar4_sat_rtp_uchar4, __convert_uchar4_sat_rtp_ushort4, __convert_uchar4_sat_rtp_uint4, __convert_uchar4_sat_rtp_ulong4, __convert_uchar4_sat_rtp_char4, __convert_uchar4_sat_rtp_short4, __convert_uchar4_sat_rtp_int4, __convert_uchar4_sat_rtp_long4, __convert_uchar4_sat_rtp_float4, __convert_uchar4_sat_rtp_double4 )
    #define convert_uchar8_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_uchar8_sat_rtp_uchar8, __convert_uchar8_sat_ushort8, __convert_uchar8_sat_rtp_uint8, __convert_uchar8_sat_rtp_ulong8, __convert_uchar8_sat_rtp_char8, __convert_uchar8_sat_short8, __convert_uchar8_sat_rtp_int8, __convert_uchar8_sat_rtp_long8, __convert_uchar8_sat_rtp_float8, __convert_uchar8_sat_rtp_double8 )
    #define convert_uchar16_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_uchar16_sat_rtp_uchar16, __convert_uchar16_sat_ushort16, __convert_uchar16_sat_rtp_uint16, __convert_uchar16_sat_rtp_ulong16, __convert_uchar16_sat_char16, __convert_uchar16_sat_short16, __convert_uchar16_sat_rtp_int16, __convert_uchar16_sat_rtp_long16, __convert_uchar16_sat_rtp_float16, __convert_uchar16_sat_rtp_double16 )
    #define convert_uchar_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_uchar_sat_rtn_uchar, __convert_uchar_sat_ushort, __convert_uchar_sat_rtn_uint, __convert_uchar_sat_rtn_ulong, __convert_uchar_sat_char, __convert_uchar_sat_short, __convert_uchar_sat_rtn_int, __convert_uchar_sat_rtn_long, __convert_uchar_sat_rtn_float, __convert_uchar_sat_rtn_double )
    #define convert_uchar2_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_uchar2_sat_rtn_uchar2, __convert_uchar2_sat_rtn_ushort2, __convert_uchar2_sat_rtn_uint2, __convert_uchar2_sat_rtn_ulong2, __convert_uchar2_sat_rtn_char2, __convert_uchar2_sat_rtn_short2, __convert_uchar2_sat_rtn_int2, __convert_uchar2_sat_rtn_long2, __convert_uchar2_sat_rtn_float2, __convert_uchar2_sat_rtn_double2 )
    #define convert_uchar4_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_uchar4_sat_rtn_uchar4, __convert_uchar4_sat_rtn_ushort4, __convert_uchar4_sat_rtn_uint4, __convert_uchar4_sat_rtn_ulong4, __convert_uchar4_sat_rtn_char4, __convert_uchar4_sat_rtn_short4, __convert_uchar4_sat_rtn_int4, __convert_uchar4_sat_rtn_long4, __convert_uchar4_sat_rtn_float4, __convert_uchar4_sat_rtn_double4 )
    #define convert_uchar8_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_uchar8_sat_rtn_uchar8, __convert_uchar8_sat_ushort8, __convert_uchar8_sat_rtn_uint8, __convert_uchar8_sat_rtn_ulong8, __convert_uchar8_sat_rtn_char8, __convert_uchar8_sat_short8, __convert_uchar8_sat_rtn_int8, __convert_uchar8_sat_rtn_long8, __convert_uchar8_sat_rtn_float8, __convert_uchar8_sat_rtn_double8 )
    #define convert_uchar16_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_uchar16_sat_rtn_uchar16, __convert_uchar16_sat_ushort16, __convert_uchar16_sat_rtn_uint16, __convert_uchar16_sat_rtn_ulong16, __convert_uchar16_sat_char16, __convert_uchar16_sat_short16, __convert_uchar16_sat_rtn_int16, __convert_uchar16_sat_rtn_long16, __convert_uchar16_sat_rtn_float16, __convert_uchar16_sat_rtn_double16 )
    #define convert_uchar_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_uchar_sat_rtz_uchar, __convert_uchar_sat_ushort, __convert_uchar_sat_rtz_uint, __convert_uchar_sat_rtz_ulong, __convert_uchar_sat_char, __convert_uchar_sat_short, __convert_uchar_sat_rtz_int, __convert_uchar_sat_rtz_long, __convert_uchar_sat_rtz_float, __convert_uchar_sat_rtz_double )
    #define convert_uchar2_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_uchar2_sat_rtz_uchar2, __convert_uchar2_sat_rtz_ushort2, __convert_uchar2_sat_rtz_uint2, __convert_uchar2_sat_rtz_ulong2, __convert_uchar2_sat_rtz_char2, __convert_uchar2_sat_rtz_short2, __convert_uchar2_sat_rtz_int2, __convert_uchar2_sat_rtz_long2, __convert_uchar2_sat_rtz_float2, __convert_uchar2_sat_rtz_double2 )
    #define convert_uchar4_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_uchar4_sat_rtz_uchar4, __convert_uchar4_sat_rtz_ushort4, __convert_uchar4_sat_rtz_uint4, __convert_uchar4_sat_rtz_ulong4, __convert_uchar4_sat_rtz_char4, __convert_uchar4_sat_rtz_short4, __convert_uchar4_sat_rtz_int4, __convert_uchar4_sat_rtz_long4, __convert_uchar4_sat_rtz_float4, __convert_uchar4_sat_rtz_double4 )
    #define convert_uchar8_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_uchar8_sat_rtz_uchar8, __convert_uchar8_sat_ushort8, __convert_uchar8_sat_rtz_uint8, __convert_uchar8_sat_rtz_ulong8, __convert_uchar8_sat_rtz_char8, __convert_uchar8_sat_short8, __convert_uchar8_sat_rtz_int8, __convert_uchar8_sat_rtz_long8, __convert_uchar8_sat_rtz_float8, __convert_uchar8_sat_rtz_double8 )
    #define convert_uchar16_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_uchar16_sat_rtz_uchar16, __convert_uchar16_sat_ushort16, __convert_uchar16_sat_rtz_uint16, __convert_uchar16_sat_rtz_ulong16, __convert_uchar16_sat_char16, __convert_uchar16_sat_short16, __convert_uchar16_sat_rtz_int16, __convert_uchar16_sat_rtz_long16, __convert_uchar16_sat_rtz_float16, __convert_uchar16_sat_rtz_double16 )
    #define convert_ushort(_X)    __builtin_overload( 1, _X, __convert_ushort_uchar, __convert_ushort_ushort, __convert_ushort_uint, __convert_ushort_ulong, __convert_ushort_char, __convert_ushort_short, __convert_ushort_int, __convert_ushort_long, __convert_ushort_float, __convert_ushort_double )
    #define convert_ushort2(_X)    __builtin_overload( 1, _X, __convert_ushort2_uchar2, __convert_ushort2_ushort2, __convert_ushort2_uint2, __convert_ushort2_ulong2, __convert_ushort2_char2, __convert_ushort2_short2, __convert_ushort2_int2, __convert_ushort2_long2, __convert_ushort2_float2, __convert_ushort2_double2 )
    #define convert_ushort4(_X)    __builtin_overload( 1, _X, __convert_ushort4_uchar4, __convert_ushort4_ushort4, __convert_ushort4_uint4, __convert_ushort4_ulong4, __convert_ushort4_char4, __convert_ushort4_short4, __convert_ushort4_int4, __convert_ushort4_long4, __convert_ushort4_float4, __convert_ushort4_double4 )
    #define convert_ushort8(_X)    __builtin_overload( 1, _X, __convert_ushort8_uchar8, __convert_ushort8_ushort8, __convert_ushort8_uint8, __convert_ushort8_ulong8, __convert_ushort8_char8, __convert_ushort8_short8, __convert_ushort8_int8, __convert_ushort8_long8, __convert_ushort8_float8, __convert_ushort8_double8 )
    #define convert_ushort16(_X)    __builtin_overload( 1, _X, __convert_ushort16_uchar16, __convert_ushort16_ushort16, __convert_ushort16_uint16, __convert_ushort16_ulong16, __convert_ushort16_char16, __convert_ushort16_short16, __convert_ushort16_int16, __convert_ushort16_long16, __convert_ushort16_float16, __convert_ushort16_double16 )
    #define convert_ushort_rte(_X)    __builtin_overload( 1, _X, __convert_ushort_uchar, __convert_ushort_rte_ushort, __convert_ushort_uint, __convert_ushort_ulong, __convert_ushort_rte_char, __convert_ushort_rte_short, __convert_ushort_rte_int, __convert_ushort_rte_long, __convert_ushort_rte_float, __convert_ushort_rte_double )
    #define convert_ushort2_rte(_X)    __builtin_overload( 1, _X, __convert_ushort2_rte_uchar2, __convert_ushort2_rte_ushort2, __convert_ushort2_rte_uint2, __convert_ushort2_ulong2, __convert_ushort2_rte_char2, __convert_ushort2_rte_short2, __convert_ushort2_rte_int2, __convert_ushort2_rte_long2, __convert_ushort2_rte_float2, __convert_ushort2_rte_double2 )
    #define convert_ushort4_rte(_X)    __builtin_overload( 1, _X, __convert_ushort4_rte_uchar4, __convert_ushort4_rte_ushort4, __convert_ushort4_uint4, __convert_ushort4_ulong4, __convert_ushort4_rte_char4, __convert_ushort4_rte_short4, __convert_ushort4_rte_int4, __convert_ushort4_rte_long4, __convert_ushort4_rte_float4, __convert_ushort4_rte_double4 )
    #define convert_ushort8_rte(_X)    __builtin_overload( 1, _X, __convert_ushort8_uchar8, __convert_ushort8_rte_ushort8, __convert_ushort8_uint8, __convert_ushort8_ulong8, __convert_ushort8_rte_char8, __convert_ushort8_rte_short8, __convert_ushort8_rte_int8, __convert_ushort8_rte_long8, __convert_ushort8_rte_float8, __convert_ushort8_rte_double8 )
    #define convert_ushort16_rte(_X)    __builtin_overload( 1, _X, __convert_ushort16_uchar16, __convert_ushort16_rte_ushort16, __convert_ushort16_rte_uint16, __convert_ushort16_rte_ulong16, __convert_ushort16_rte_char16, __convert_ushort16_rte_short16, __convert_ushort16_rte_int16, __convert_ushort16_rte_long16, __convert_ushort16_rte_float16, __convert_ushort16_rte_double16 )
    #define convert_ushort_rtp(_X)    __builtin_overload( 1, _X, __convert_ushort_uchar, __convert_ushort_rtp_ushort, __convert_ushort_uint, __convert_ushort_ulong, __convert_ushort_rtp_char, __convert_ushort_rtp_short, __convert_ushort_rtp_int, __convert_ushort_rtp_long, __convert_ushort_rtp_float, __convert_ushort_rtp_double )
    #define convert_ushort2_rtp(_X)    __builtin_overload( 1, _X, __convert_ushort2_rtp_uchar2, __convert_ushort2_rtp_ushort2, __convert_ushort2_rtp_uint2, __convert_ushort2_ulong2, __convert_ushort2_rtp_char2, __convert_ushort2_rtp_short2, __convert_ushort2_rtp_int2, __convert_ushort2_rtp_long2, __convert_ushort2_rtp_float2, __convert_ushort2_rtp_double2 )
    #define convert_ushort4_rtp(_X)    __builtin_overload( 1, _X, __convert_ushort4_rtp_uchar4, __convert_ushort4_rtp_ushort4, __convert_ushort4_uint4, __convert_ushort4_ulong4, __convert_ushort4_rtp_char4, __convert_ushort4_rtp_short4, __convert_ushort4_rtp_int4, __convert_ushort4_rtp_long4, __convert_ushort4_rtp_float4, __convert_ushort4_rtp_double4 )
    #define convert_ushort8_rtp(_X)    __builtin_overload( 1, _X, __convert_ushort8_uchar8, __convert_ushort8_rtp_ushort8, __convert_ushort8_uint8, __convert_ushort8_ulong8, __convert_ushort8_rtp_char8, __convert_ushort8_rtp_short8, __convert_ushort8_rtp_int8, __convert_ushort8_rtp_long8, __convert_ushort8_rtp_float8, __convert_ushort8_rtp_double8 )
    #define convert_ushort16_rtp(_X)    __builtin_overload( 1, _X, __convert_ushort16_uchar16, __convert_ushort16_rtp_ushort16, __convert_ushort16_rtp_uint16, __convert_ushort16_rtp_ulong16, __convert_ushort16_rtp_char16, __convert_ushort16_rtp_short16, __convert_ushort16_rtp_int16, __convert_ushort16_rtp_long16, __convert_ushort16_rtp_float16, __convert_ushort16_rtp_double16 )
    #define convert_ushort_rtn(_X)    __builtin_overload( 1, _X, __convert_ushort_uchar, __convert_ushort_rtn_ushort, __convert_ushort_uint, __convert_ushort_ulong, __convert_ushort_rtn_char, __convert_ushort_rtn_short, __convert_ushort_rtn_int, __convert_ushort_rtn_long, __convert_ushort_rtn_float, __convert_ushort_rtn_double )
    #define convert_ushort2_rtn(_X)    __builtin_overload( 1, _X, __convert_ushort2_rtn_uchar2, __convert_ushort2_rtn_ushort2, __convert_ushort2_rtn_uint2, __convert_ushort2_ulong2, __convert_ushort2_rtn_char2, __convert_ushort2_rtn_short2, __convert_ushort2_rtn_int2, __convert_ushort2_rtn_long2, __convert_ushort2_rtn_float2, __convert_ushort2_rtn_double2 )
    #define convert_ushort4_rtn(_X)    __builtin_overload( 1, _X, __convert_ushort4_rtn_uchar4, __convert_ushort4_rtn_ushort4, __convert_ushort4_uint4, __convert_ushort4_ulong4, __convert_ushort4_rtn_char4, __convert_ushort4_rtn_short4, __convert_ushort4_rtn_int4, __convert_ushort4_rtn_long4, __convert_ushort4_rtn_float4, __convert_ushort4_rtn_double4 )
    #define convert_ushort8_rtn(_X)    __builtin_overload( 1, _X, __convert_ushort8_uchar8, __convert_ushort8_rtn_ushort8, __convert_ushort8_uint8, __convert_ushort8_ulong8, __convert_ushort8_rtn_char8, __convert_ushort8_rtn_short8, __convert_ushort8_rtn_int8, __convert_ushort8_rtn_long8, __convert_ushort8_rtn_float8, __convert_ushort8_rtn_double8 )
    #define convert_ushort16_rtn(_X)    __builtin_overload( 1, _X, __convert_ushort16_uchar16, __convert_ushort16_rtn_ushort16, __convert_ushort16_rtn_uint16, __convert_ushort16_rtn_ulong16, __convert_ushort16_rtn_char16, __convert_ushort16_rtn_short16, __convert_ushort16_rtn_int16, __convert_ushort16_rtn_long16, __convert_ushort16_rtn_float16, __convert_ushort16_rtn_double16 )
    #define convert_ushort_rtz(_X)    __builtin_overload( 1, _X, __convert_ushort_uchar, __convert_ushort_rtz_ushort, __convert_ushort_uint, __convert_ushort_ulong, __convert_ushort_rtz_char, __convert_ushort_rtz_short, __convert_ushort_rtz_int, __convert_ushort_rtz_long, __convert_ushort_rtz_float, __convert_ushort_rtz_double )
    #define convert_ushort2_rtz(_X)    __builtin_overload( 1, _X, __convert_ushort2_rtz_uchar2, __convert_ushort2_rtz_ushort2, __convert_ushort2_rtz_uint2, __convert_ushort2_ulong2, __convert_ushort2_rtz_char2, __convert_ushort2_rtz_short2, __convert_ushort2_rtz_int2, __convert_ushort2_rtz_long2, __convert_ushort2_rtz_float2, __convert_ushort2_rtz_double2 )
    #define convert_ushort4_rtz(_X)    __builtin_overload( 1, _X, __convert_ushort4_rtz_uchar4, __convert_ushort4_rtz_ushort4, __convert_ushort4_uint4, __convert_ushort4_ulong4, __convert_ushort4_rtz_char4, __convert_ushort4_rtz_short4, __convert_ushort4_rtz_int4, __convert_ushort4_rtz_long4, __convert_ushort4_rtz_float4, __convert_ushort4_rtz_double4 )
    #define convert_ushort8_rtz(_X)    __builtin_overload( 1, _X, __convert_ushort8_uchar8, __convert_ushort8_rtz_ushort8, __convert_ushort8_uint8, __convert_ushort8_ulong8, __convert_ushort8_rtz_char8, __convert_ushort8_rtz_short8, __convert_ushort8_rtz_int8, __convert_ushort8_rtz_long8, __convert_ushort8_rtz_float8, __convert_ushort8_rtz_double8 )
    #define convert_ushort16_rtz(_X)    __builtin_overload( 1, _X, __convert_ushort16_uchar16, __convert_ushort16_rtz_ushort16, __convert_ushort16_rtz_uint16, __convert_ushort16_rtz_ulong16, __convert_ushort16_rtz_char16, __convert_ushort16_rtz_short16, __convert_ushort16_rtz_int16, __convert_ushort16_rtz_long16, __convert_ushort16_rtz_float16, __convert_ushort16_rtz_double16 )
    #define convert_ushort_sat(_X)    __builtin_overload( 1, _X, __convert_ushort_uchar, __convert_ushort_sat_ushort, __convert_ushort_sat_uint, __convert_ushort_sat_ulong, __convert_ushort_sat_char, __convert_ushort_sat_short, __convert_ushort_sat_int, __convert_ushort_sat_long, __convert_ushort_sat_float, __convert_ushort_sat_double )
    #define convert_ushort2_sat(_X)    __builtin_overload( 1, _X, __convert_ushort2_sat_uchar2, __convert_ushort2_sat_ushort2, __convert_ushort2_sat_uint2, __convert_ushort2_sat_ulong2, __convert_ushort2_sat_char2, __convert_ushort2_sat_short2, __convert_ushort2_sat_int2, __convert_ushort2_sat_long2, __convert_ushort2_sat_float2, __convert_ushort2_sat_double2 )
    #define convert_ushort4_sat(_X)    __builtin_overload( 1, _X, __convert_ushort4_sat_uchar4, __convert_ushort4_sat_ushort4, __convert_ushort4_sat_uint4, __convert_ushort4_sat_ulong4, __convert_ushort4_sat_char4, __convert_ushort4_sat_short4, __convert_ushort4_sat_int4, __convert_ushort4_sat_long4, __convert_ushort4_sat_float4, __convert_ushort4_sat_double4 )
    #define convert_ushort8_sat(_X)    __builtin_overload( 1, _X, __convert_ushort8_uchar8, __convert_ushort8_sat_ushort8, __convert_ushort8_sat_uint8, __convert_ushort8_sat_ulong8, __convert_ushort8_sat_char8, __convert_ushort8_sat_short8, __convert_ushort8_sat_int8, __convert_ushort8_sat_long8, __convert_ushort8_sat_float8, __convert_ushort8_sat_double8 )
    #define convert_ushort16_sat(_X)    __builtin_overload( 1, _X, __convert_ushort16_uchar16, __convert_ushort16_sat_ushort16, __convert_ushort16_sat_uint16, __convert_ushort16_sat_ulong16, __convert_ushort16_sat_char16, __convert_ushort16_sat_short16, __convert_ushort16_sat_int16, __convert_ushort16_sat_long16, __convert_ushort16_sat_float16, __convert_ushort16_sat_double16 )
    #define convert_ushort_sat_rte(_X)    __builtin_overload( 1, _X, __convert_ushort_uchar, __convert_ushort_sat_rte_ushort, __convert_ushort_sat_uint, __convert_ushort_sat_rte_ulong, __convert_ushort_sat_rte_char, __convert_ushort_sat_short, __convert_ushort_sat_int, __convert_ushort_sat_rte_long, __convert_ushort_sat_rte_float, __convert_ushort_sat_rte_double )
    #define convert_ushort2_sat_rte(_X)    __builtin_overload( 1, _X, __convert_ushort2_sat_rte_uchar2, __convert_ushort2_sat_rte_ushort2, __convert_ushort2_sat_rte_uint2, __convert_ushort2_sat_rte_ulong2, __convert_ushort2_sat_rte_char2, __convert_ushort2_sat_rte_short2, __convert_ushort2_sat_rte_int2, __convert_ushort2_sat_rte_long2, __convert_ushort2_sat_rte_float2, __convert_ushort2_sat_rte_double2 )
    #define convert_ushort4_sat_rte(_X)    __builtin_overload( 1, _X, __convert_ushort4_sat_rte_uchar4, __convert_ushort4_sat_rte_ushort4, __convert_ushort4_sat_uint4, __convert_ushort4_sat_rte_ulong4, __convert_ushort4_sat_rte_char4, __convert_ushort4_sat_rte_short4, __convert_ushort4_sat_int4, __convert_ushort4_sat_rte_long4, __convert_ushort4_sat_rte_float4, __convert_ushort4_sat_rte_double4 )
    #define convert_ushort8_sat_rte(_X)    __builtin_overload( 1, _X, __convert_ushort8_uchar8, __convert_ushort8_sat_rte_ushort8, __convert_ushort8_sat_uint8, __convert_ushort8_sat_rte_ulong8, __convert_ushort8_sat_rte_char8, __convert_ushort8_sat_short8, __convert_ushort8_sat_int8, __convert_ushort8_sat_rte_long8, __convert_ushort8_sat_rte_float8, __convert_ushort8_sat_rte_double8 )
    #define convert_ushort16_sat_rte(_X)    __builtin_overload( 1, _X, __convert_ushort16_uchar16, __convert_ushort16_sat_rte_ushort16, __convert_ushort16_sat_rte_uint16, __convert_ushort16_sat_rte_ulong16, __convert_ushort16_sat_rte_char16, __convert_ushort16_sat_rte_short16, __convert_ushort16_sat_rte_int16, __convert_ushort16_sat_rte_long16, __convert_ushort16_sat_rte_float16, __convert_ushort16_sat_rte_double16 )
    #define convert_ushort_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_ushort_uchar, __convert_ushort_sat_rtp_ushort, __convert_ushort_sat_uint, __convert_ushort_sat_rtp_ulong, __convert_ushort_sat_rtp_char, __convert_ushort_sat_short, __convert_ushort_sat_int, __convert_ushort_sat_rtp_long, __convert_ushort_sat_rtp_float, __convert_ushort_sat_rtp_double )
    #define convert_ushort2_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_ushort2_sat_rtp_uchar2, __convert_ushort2_sat_rtp_ushort2, __convert_ushort2_sat_rtp_uint2, __convert_ushort2_sat_rtp_ulong2, __convert_ushort2_sat_rtp_char2, __convert_ushort2_sat_rtp_short2, __convert_ushort2_sat_rtp_int2, __convert_ushort2_sat_rtp_long2, __convert_ushort2_sat_rtp_float2, __convert_ushort2_sat_rtp_double2 )
    #define convert_ushort4_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_ushort4_sat_rtp_uchar4, __convert_ushort4_sat_rtp_ushort4, __convert_ushort4_sat_uint4, __convert_ushort4_sat_rtp_ulong4, __convert_ushort4_sat_rtp_char4, __convert_ushort4_sat_rtp_short4, __convert_ushort4_sat_int4, __convert_ushort4_sat_rtp_long4, __convert_ushort4_sat_rtp_float4, __convert_ushort4_sat_rtp_double4 )
    #define convert_ushort8_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_ushort8_uchar8, __convert_ushort8_sat_rtp_ushort8, __convert_ushort8_sat_uint8, __convert_ushort8_sat_rtp_ulong8, __convert_ushort8_sat_rtp_char8, __convert_ushort8_sat_short8, __convert_ushort8_sat_int8, __convert_ushort8_sat_rtp_long8, __convert_ushort8_sat_rtp_float8, __convert_ushort8_sat_rtp_double8 )
    #define convert_ushort16_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_ushort16_uchar16, __convert_ushort16_sat_rtp_ushort16, __convert_ushort16_sat_rtp_uint16, __convert_ushort16_sat_rtp_ulong16, __convert_ushort16_sat_rtp_char16, __convert_ushort16_sat_rtp_short16, __convert_ushort16_sat_rtp_int16, __convert_ushort16_sat_rtp_long16, __convert_ushort16_sat_rtp_float16, __convert_ushort16_sat_rtp_double16 )
    #define convert_ushort_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_ushort_uchar, __convert_ushort_sat_rtn_ushort, __convert_ushort_sat_uint, __convert_ushort_sat_rtn_ulong, __convert_ushort_sat_rtn_char, __convert_ushort_sat_short, __convert_ushort_sat_int, __convert_ushort_sat_rtn_long, __convert_ushort_sat_rtn_float, __convert_ushort_sat_rtn_double )
    #define convert_ushort2_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_ushort2_sat_rtn_uchar2, __convert_ushort2_sat_rtn_ushort2, __convert_ushort2_sat_rtn_uint2, __convert_ushort2_sat_rtn_ulong2, __convert_ushort2_sat_rtn_char2, __convert_ushort2_sat_rtn_short2, __convert_ushort2_sat_rtn_int2, __convert_ushort2_sat_rtn_long2, __convert_ushort2_sat_rtn_float2, __convert_ushort2_sat_rtn_double2 )
    #define convert_ushort4_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_ushort4_sat_rtn_uchar4, __convert_ushort4_sat_rtn_ushort4, __convert_ushort4_sat_uint4, __convert_ushort4_sat_rtn_ulong4, __convert_ushort4_sat_rtn_char4, __convert_ushort4_sat_rtn_short4, __convert_ushort4_sat_int4, __convert_ushort4_sat_rtn_long4, __convert_ushort4_sat_rtn_float4, __convert_ushort4_sat_rtn_double4 )
    #define convert_ushort8_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_ushort8_uchar8, __convert_ushort8_sat_rtn_ushort8, __convert_ushort8_sat_uint8, __convert_ushort8_sat_rtn_ulong8, __convert_ushort8_sat_rtn_char8, __convert_ushort8_sat_short8, __convert_ushort8_sat_int8, __convert_ushort8_sat_rtn_long8, __convert_ushort8_sat_rtn_float8, __convert_ushort8_sat_rtn_double8 )
    #define convert_ushort16_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_ushort16_uchar16, __convert_ushort16_sat_rtn_ushort16, __convert_ushort16_sat_rtn_uint16, __convert_ushort16_sat_rtn_ulong16, __convert_ushort16_sat_rtn_char16, __convert_ushort16_sat_rtn_short16, __convert_ushort16_sat_rtn_int16, __convert_ushort16_sat_rtn_long16, __convert_ushort16_sat_rtn_float16, __convert_ushort16_sat_rtn_double16 )
    #define convert_ushort_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_ushort_uchar, __convert_ushort_sat_rtz_ushort, __convert_ushort_sat_uint, __convert_ushort_sat_rtz_ulong, __convert_ushort_sat_rtz_char, __convert_ushort_sat_short, __convert_ushort_sat_int, __convert_ushort_sat_rtz_long, __convert_ushort_sat_rtz_float, __convert_ushort_sat_rtz_double )
    #define convert_ushort2_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_ushort2_sat_rtz_uchar2, __convert_ushort2_sat_rtz_ushort2, __convert_ushort2_sat_rtz_uint2, __convert_ushort2_sat_rtz_ulong2, __convert_ushort2_sat_rtz_char2, __convert_ushort2_sat_rtz_short2, __convert_ushort2_sat_rtz_int2, __convert_ushort2_sat_rtz_long2, __convert_ushort2_sat_rtz_float2, __convert_ushort2_sat_rtz_double2 )
    #define convert_ushort4_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_ushort4_sat_rtz_uchar4, __convert_ushort4_sat_rtz_ushort4, __convert_ushort4_sat_uint4, __convert_ushort4_sat_rtz_ulong4, __convert_ushort4_sat_rtz_char4, __convert_ushort4_sat_rtz_short4, __convert_ushort4_sat_int4, __convert_ushort4_sat_rtz_long4, __convert_ushort4_sat_rtz_float4, __convert_ushort4_sat_rtz_double4 )
    #define convert_ushort8_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_ushort8_uchar8, __convert_ushort8_sat_rtz_ushort8, __convert_ushort8_sat_uint8, __convert_ushort8_sat_rtz_ulong8, __convert_ushort8_sat_rtz_char8, __convert_ushort8_sat_short8, __convert_ushort8_sat_int8, __convert_ushort8_sat_rtz_long8, __convert_ushort8_sat_rtz_float8, __convert_ushort8_sat_rtz_double8 )
    #define convert_ushort16_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_ushort16_uchar16, __convert_ushort16_sat_rtz_ushort16, __convert_ushort16_sat_rtz_uint16, __convert_ushort16_sat_rtz_ulong16, __convert_ushort16_sat_rtz_char16, __convert_ushort16_sat_rtz_short16, __convert_ushort16_sat_rtz_int16, __convert_ushort16_sat_rtz_long16, __convert_ushort16_sat_rtz_float16, __convert_ushort16_sat_rtz_double16 )
    #define convert_uint(_X)    __builtin_overload( 1, _X, __convert_uint_uchar, __convert_uint_ushort, __convert_uint_uint, __convert_uint_ulong, __convert_uint_char, __convert_uint_short, __convert_uint_int, __convert_uint_long, __convert_uint_rtz_float, __convert_uint_rtz_double )
    #define convert_uint2(_X)    __builtin_overload( 1, _X, __convert_uint2_uchar2, __convert_uint2_ushort2, __convert_uint2_uint2, __convert_uint2_ulong2, __convert_uint2_char2, __convert_uint2_short2, __convert_uint2_int2, __convert_uint2_long2, __convert_uint2_float2, __convert_uint2_rtz_double2 )
    #define convert_uint4(_X)    __builtin_overload( 1, _X, __convert_uint4_uchar4, __convert_uint4_ushort4, __convert_uint4_uint4, __convert_uint4_ulong4, __convert_uint4_char4, __convert_uint4_short4, __convert_uint4_int4, __convert_uint4_long4, __convert_uint4_rtz_float4, __convert_uint4_rtz_double4 )
    #define convert_uint8(_X)    __builtin_overload( 1, _X, __convert_uint8_uchar8, __convert_uint8_ushort8, __convert_uint8_uint8, __convert_uint8_ulong8, __convert_uint8_char8, __convert_uint8_short8, __convert_uint8_int8, __convert_uint8_long8, __convert_uint8_float8, __convert_uint8_double8 )
    #define convert_uint16(_X)    __builtin_overload( 1, _X, __convert_uint16_uchar16, __convert_uint16_ushort16, __convert_uint16_uint16, __convert_uint16_ulong16, __convert_uint16_char16, __convert_uint16_short16, __convert_uint16_int16, __convert_uint16_long16, __convert_uint16_float16, __convert_uint16_double16 )
    #define convert_uint_rte(_X)    __builtin_overload( 1, _X, __convert_uint_uchar, __convert_uint_ushort, __convert_uint_rte_uint, __convert_uint_ulong, __convert_uint_rte_char, __convert_uint_rte_short, __convert_uint_rte_int, __convert_uint_rte_long, __convert_uint_rte_float, __convert_uint_rte_double )
    #define convert_uint2_rte(_X)    __builtin_overload( 1, _X, __convert_uint2_rte_uchar2, __convert_uint2_rte_ushort2, __convert_uint2_rte_uint2, __convert_uint2_ulong2, __convert_uint2_rte_char2, __convert_uint2_rte_short2, __convert_uint2_rte_int2, __convert_uint2_rte_long2, __convert_uint2_rte_float2, __convert_uint2_rte_double2 )
    #define convert_uint4_rte(_X)    __builtin_overload( 1, _X, __convert_uint4_uchar4, __convert_uint4_ushort4, __convert_uint4_rte_uint4, __convert_uint4_ulong4, __convert_uint4_rte_char4, __convert_uint4_rte_short4, __convert_uint4_rte_int4, __convert_uint4_rte_long4, __convert_uint4_rte_float4, __convert_uint4_rte_double4 )
    #define convert_uint8_rte(_X)    __builtin_overload( 1, _X, __convert_uint8_uchar8, __convert_uint8_ushort8, __convert_uint8_rte_uint8, __convert_uint8_rte_ulong8, __convert_uint8_rte_char8, __convert_uint8_rte_short8, __convert_uint8_rte_int8, __convert_uint8_rte_long8, __convert_uint8_rte_float8, __convert_uint8_rte_double8 )
    #define convert_uint16_rte(_X)    __builtin_overload( 1, _X, __convert_uint16_uchar16, __convert_uint16_rte_ushort16, __convert_uint16_rte_uint16, __convert_uint16_rte_ulong16, __convert_uint16_rte_char16, __convert_uint16_rte_short16, __convert_uint16_rte_int16, __convert_uint16_rte_long16, __convert_uint16_rte_float16, __convert_uint16_rte_double16 )
    #define convert_uint_rtp(_X)    __builtin_overload( 1, _X, __convert_uint_uchar, __convert_uint_ushort, __convert_uint_rtp_uint, __convert_uint_ulong, __convert_uint_rtp_char, __convert_uint_rtp_short, __convert_uint_rtp_int, __convert_uint_rtp_long, __convert_uint_rtp_float, __convert_uint_rtp_double )
    #define convert_uint2_rtp(_X)    __builtin_overload( 1, _X, __convert_uint2_rtp_uchar2, __convert_uint2_rtp_ushort2, __convert_uint2_rtp_uint2, __convert_uint2_ulong2, __convert_uint2_rtp_char2, __convert_uint2_rtp_short2, __convert_uint2_rtp_int2, __convert_uint2_rtp_long2, __convert_uint2_rtp_float2, __convert_uint2_rtp_double2 )
    #define convert_uint4_rtp(_X)    __builtin_overload( 1, _X, __convert_uint4_uchar4, __convert_uint4_ushort4, __convert_uint4_rtp_uint4, __convert_uint4_ulong4, __convert_uint4_rtp_char4, __convert_uint4_rtp_short4, __convert_uint4_rtp_int4, __convert_uint4_rtp_long4, __convert_uint4_rtp_float4, __convert_uint4_rtp_double4 )
    #define convert_uint8_rtp(_X)    __builtin_overload( 1, _X, __convert_uint8_uchar8, __convert_uint8_ushort8, __convert_uint8_rtp_uint8, __convert_uint8_rtp_ulong8, __convert_uint8_rtp_char8, __convert_uint8_rtp_short8, __convert_uint8_rtp_int8, __convert_uint8_rtp_long8, __convert_uint8_rtp_float8, __convert_uint8_rtp_double8 )
    #define convert_uint16_rtp(_X)    __builtin_overload( 1, _X, __convert_uint16_uchar16, __convert_uint16_rtp_ushort16, __convert_uint16_rtp_uint16, __convert_uint16_rtp_ulong16, __convert_uint16_rtp_char16, __convert_uint16_rtp_short16, __convert_uint16_rtp_int16, __convert_uint16_rtp_long16, __convert_uint16_rtp_float16, __convert_uint16_rtp_double16 )
    #define convert_uint_rtn(_X)    __builtin_overload( 1, _X, __convert_uint_uchar, __convert_uint_ushort, __convert_uint_rtn_uint, __convert_uint_ulong, __convert_uint_rtn_char, __convert_uint_rtn_short, __convert_uint_rtn_int, __convert_uint_rtn_long, __convert_uint_rtn_float, __convert_uint_rtn_double )
    #define convert_uint2_rtn(_X)    __builtin_overload( 1, _X, __convert_uint2_rtn_uchar2, __convert_uint2_rtn_ushort2, __convert_uint2_rtn_uint2, __convert_uint2_ulong2, __convert_uint2_rtn_char2, __convert_uint2_rtn_short2, __convert_uint2_rtn_int2, __convert_uint2_rtn_long2, __convert_uint2_rtn_float2, __convert_uint2_rtn_double2 )
    #define convert_uint4_rtn(_X)    __builtin_overload( 1, _X, __convert_uint4_uchar4, __convert_uint4_ushort4, __convert_uint4_rtn_uint4, __convert_uint4_ulong4, __convert_uint4_rtn_char4, __convert_uint4_rtn_short4, __convert_uint4_rtn_int4, __convert_uint4_rtn_long4, __convert_uint4_rtn_float4, __convert_uint4_rtn_double4 )
    #define convert_uint8_rtn(_X)    __builtin_overload( 1, _X, __convert_uint8_uchar8, __convert_uint8_ushort8, __convert_uint8_rtn_uint8, __convert_uint8_rtn_ulong8, __convert_uint8_rtn_char8, __convert_uint8_rtn_short8, __convert_uint8_rtn_int8, __convert_uint8_rtn_long8, __convert_uint8_rtn_float8, __convert_uint8_rtn_double8 )
    #define convert_uint16_rtn(_X)    __builtin_overload( 1, _X, __convert_uint16_uchar16, __convert_uint16_rtn_ushort16, __convert_uint16_rtn_uint16, __convert_uint16_rtn_ulong16, __convert_uint16_rtn_char16, __convert_uint16_rtn_short16, __convert_uint16_rtn_int16, __convert_uint16_rtn_long16, __convert_uint16_rtn_float16, __convert_uint16_rtn_double16 )
    #define convert_uint_rtz(_X)    __builtin_overload( 1, _X, __convert_uint_uchar, __convert_uint_ushort, __convert_uint_rtz_uint, __convert_uint_ulong, __convert_uint_rtz_char, __convert_uint_rtz_short, __convert_uint_rtz_int, __convert_uint_rtz_long, __convert_uint_rtz_float, __convert_uint_rtz_double )
    #define convert_uint2_rtz(_X)    __builtin_overload( 1, _X, __convert_uint2_rtz_uchar2, __convert_uint2_rtz_ushort2, __convert_uint2_rtz_uint2, __convert_uint2_ulong2, __convert_uint2_rtz_char2, __convert_uint2_rtz_short2, __convert_uint2_rtz_int2, __convert_uint2_rtz_long2, __convert_uint2_rtz_float2, __convert_uint2_rtz_double2 )
    #define convert_uint4_rtz(_X)    __builtin_overload( 1, _X, __convert_uint4_uchar4, __convert_uint4_ushort4, __convert_uint4_rtz_uint4, __convert_uint4_ulong4, __convert_uint4_rtz_char4, __convert_uint4_rtz_short4, __convert_uint4_rtz_int4, __convert_uint4_rtz_long4, __convert_uint4_rtz_float4, __convert_uint4_rtz_double4 )
    #define convert_uint8_rtz(_X)    __builtin_overload( 1, _X, __convert_uint8_uchar8, __convert_uint8_ushort8, __convert_uint8_rtz_uint8, __convert_uint8_rtz_ulong8, __convert_uint8_rtz_char8, __convert_uint8_rtz_short8, __convert_uint8_rtz_int8, __convert_uint8_rtz_long8, __convert_uint8_rtz_float8, __convert_uint8_rtz_double8 )
    #define convert_uint16_rtz(_X)    __builtin_overload( 1, _X, __convert_uint16_uchar16, __convert_uint16_rtz_ushort16, __convert_uint16_rtz_uint16, __convert_uint16_rtz_ulong16, __convert_uint16_rtz_char16, __convert_uint16_rtz_short16, __convert_uint16_rtz_int16, __convert_uint16_rtz_long16, __convert_uint16_rtz_float16, __convert_uint16_rtz_double16 )
    #define convert_uint_sat(_X)    __builtin_overload( 1, _X, __convert_uint_uchar, __convert_uint_ushort, __convert_uint_sat_uint, __convert_uint_sat_ulong, __convert_uint_sat_char, __convert_uint_sat_short, __convert_uint_sat_int, __convert_uint_sat_long, __convert_uint_sat_rtz_float, __convert_uint_sat_rtz_double )
    #define convert_uint2_sat(_X)    __builtin_overload( 1, _X, __convert_uint2_sat_uchar2, __convert_uint2_sat_ushort2, __convert_uint2_sat_uint2, __convert_uint2_sat_ulong2, __convert_uint2_sat_char2, __convert_uint2_sat_short2, __convert_uint2_sat_int2, __convert_uint2_sat_long2, __convert_uint2_sat_float2, __convert_uint2_sat_rtz_double2 )
    #define convert_uint4_sat(_X)    __builtin_overload( 1, _X, __convert_uint4_uchar4, __convert_uint4_ushort4, __convert_uint4_sat_uint4, __convert_uint4_sat_ulong4, __convert_uint4_sat_char4, __convert_uint4_sat_short4, __convert_uint4_sat_int4, __convert_uint4_sat_long4, __convert_uint4_sat_rtz_float4, __convert_uint4_sat_rtz_double4 )
    #define convert_uint8_sat(_X)    __builtin_overload( 1, _X, __convert_uint8_uchar8, __convert_uint8_ushort8, __convert_uint8_sat_uint8, __convert_uint8_sat_ulong8, __convert_uint8_sat_char8, __convert_uint8_sat_short8, __convert_uint8_sat_int8, __convert_uint8_sat_long8, __convert_uint8_sat_float8, __convert_uint8_sat_double8 )
    #define convert_uint16_sat(_X)    __builtin_overload( 1, _X, __convert_uint16_uchar16, __convert_uint16_sat_ushort16, __convert_uint16_sat_uint16, __convert_uint16_sat_ulong16, __convert_uint16_sat_char16, __convert_uint16_sat_short16, __convert_uint16_sat_int16, __convert_uint16_sat_long16, __convert_uint16_sat_float16, __convert_uint16_sat_double16 )
    #define convert_uint_sat_rte(_X)    __builtin_overload( 1, _X, __convert_uint_uchar, __convert_uint_ushort, __convert_uint_sat_rte_uint, __convert_uint_sat_ulong, __convert_uint_sat_rte_char, __convert_uint_sat_rte_short, __convert_uint_sat_int, __convert_uint_sat_long, __convert_uint_sat_rte_float, __convert_uint_sat_rte_double )
    #define convert_uint2_sat_rte(_X)    __builtin_overload( 1, _X, __convert_uint2_sat_rte_uchar2, __convert_uint2_sat_rte_ushort2, __convert_uint2_sat_rte_uint2, __convert_uint2_sat_ulong2, __convert_uint2_sat_rte_char2, __convert_uint2_sat_rte_short2, __convert_uint2_sat_rte_int2, __convert_uint2_sat_long2, __convert_uint2_sat_rte_float2, __convert_uint2_sat_rte_double2 )
    #define convert_uint4_sat_rte(_X)    __builtin_overload( 1, _X, __convert_uint4_uchar4, __convert_uint4_ushort4, __convert_uint4_sat_rte_uint4, __convert_uint4_sat_ulong4, __convert_uint4_sat_rte_char4, __convert_uint4_sat_rte_short4, __convert_uint4_sat_int4, __convert_uint4_sat_long4, __convert_uint4_sat_rte_float4, __convert_uint4_sat_rte_double4 )
    #define convert_uint8_sat_rte(_X)    __builtin_overload( 1, _X, __convert_uint8_uchar8, __convert_uint8_ushort8, __convert_uint8_sat_rte_uint8, __convert_uint8_sat_rte_ulong8, __convert_uint8_sat_rte_char8, __convert_uint8_sat_rte_short8, __convert_uint8_sat_rte_int8, __convert_uint8_sat_rte_long8, __convert_uint8_sat_rte_float8, __convert_uint8_sat_rte_double8 )
    #define convert_uint16_sat_rte(_X)    __builtin_overload( 1, _X, __convert_uint16_uchar16, __convert_uint16_sat_rte_ushort16, __convert_uint16_sat_rte_uint16, __convert_uint16_sat_rte_ulong16, __convert_uint16_sat_rte_char16, __convert_uint16_sat_rte_short16, __convert_uint16_sat_rte_int16, __convert_uint16_sat_rte_long16, __convert_uint16_sat_rte_float16, __convert_uint16_sat_rte_double16 )
    #define convert_uint_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_uint_uchar, __convert_uint_ushort, __convert_uint_sat_rtp_uint, __convert_uint_sat_ulong, __convert_uint_sat_rtp_char, __convert_uint_sat_rtp_short, __convert_uint_sat_int, __convert_uint_sat_long, __convert_uint_sat_rtp_float, __convert_uint_sat_rtp_double )
    #define convert_uint2_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_uint2_sat_rtp_uchar2, __convert_uint2_sat_rtp_ushort2, __convert_uint2_sat_rtp_uint2, __convert_uint2_sat_ulong2, __convert_uint2_sat_rtp_char2, __convert_uint2_sat_rtp_short2, __convert_uint2_sat_rtp_int2, __convert_uint2_sat_long2, __convert_uint2_sat_rtp_float2, __convert_uint2_sat_rtp_double2 )
    #define convert_uint4_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_uint4_uchar4, __convert_uint4_ushort4, __convert_uint4_sat_rtp_uint4, __convert_uint4_sat_ulong4, __convert_uint4_sat_rtp_char4, __convert_uint4_sat_rtp_short4, __convert_uint4_sat_int4, __convert_uint4_sat_long4, __convert_uint4_sat_rtp_float4, __convert_uint4_sat_rtp_double4 )
    #define convert_uint8_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_uint8_uchar8, __convert_uint8_ushort8, __convert_uint8_sat_rtp_uint8, __convert_uint8_sat_rtp_ulong8, __convert_uint8_sat_rtp_char8, __convert_uint8_sat_rtp_short8, __convert_uint8_sat_rtp_int8, __convert_uint8_sat_rtp_long8, __convert_uint8_sat_rtp_float8, __convert_uint8_sat_rtp_double8 )
    #define convert_uint16_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_uint16_uchar16, __convert_uint16_sat_rtp_ushort16, __convert_uint16_sat_rtp_uint16, __convert_uint16_sat_rtp_ulong16, __convert_uint16_sat_rtp_char16, __convert_uint16_sat_rtp_short16, __convert_uint16_sat_rtp_int16, __convert_uint16_sat_rtp_long16, __convert_uint16_sat_rtp_float16, __convert_uint16_sat_rtp_double16 )
    #define convert_uint_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_uint_uchar, __convert_uint_ushort, __convert_uint_sat_rtn_uint, __convert_uint_sat_ulong, __convert_uint_sat_rtn_char, __convert_uint_sat_rtn_short, __convert_uint_sat_int, __convert_uint_sat_long, __convert_uint_sat_rtn_float, __convert_uint_sat_rtn_double )
    #define convert_uint2_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_uint2_sat_rtn_uchar2, __convert_uint2_sat_rtn_ushort2, __convert_uint2_sat_rtn_uint2, __convert_uint2_sat_ulong2, __convert_uint2_sat_rtn_char2, __convert_uint2_sat_rtn_short2, __convert_uint2_sat_rtn_int2, __convert_uint2_sat_long2, __convert_uint2_sat_rtn_float2, __convert_uint2_sat_rtn_double2 )
    #define convert_uint4_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_uint4_uchar4, __convert_uint4_ushort4, __convert_uint4_sat_rtn_uint4, __convert_uint4_sat_ulong4, __convert_uint4_sat_rtn_char4, __convert_uint4_sat_rtn_short4, __convert_uint4_sat_int4, __convert_uint4_sat_long4, __convert_uint4_sat_rtn_float4, __convert_uint4_sat_rtn_double4 )
    #define convert_uint8_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_uint8_uchar8, __convert_uint8_ushort8, __convert_uint8_sat_rtn_uint8, __convert_uint8_sat_rtn_ulong8, __convert_uint8_sat_rtn_char8, __convert_uint8_sat_rtn_short8, __convert_uint8_sat_rtn_int8, __convert_uint8_sat_rtn_long8, __convert_uint8_sat_rtn_float8, __convert_uint8_sat_rtn_double8 )
    #define convert_uint16_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_uint16_uchar16, __convert_uint16_sat_rtn_ushort16, __convert_uint16_sat_rtn_uint16, __convert_uint16_sat_rtn_ulong16, __convert_uint16_sat_rtn_char16, __convert_uint16_sat_rtn_short16, __convert_uint16_sat_rtn_int16, __convert_uint16_sat_rtn_long16, __convert_uint16_sat_rtn_float16, __convert_uint16_sat_rtn_double16 )
    #define convert_uint_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_uint_uchar, __convert_uint_ushort, __convert_uint_sat_rtz_uint, __convert_uint_sat_ulong, __convert_uint_sat_rtz_char, __convert_uint_sat_rtz_short, __convert_uint_sat_int, __convert_uint_sat_long, __convert_uint_sat_rtz_float, __convert_uint_sat_rtz_double )
    #define convert_uint2_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_uint2_sat_rtz_uchar2, __convert_uint2_sat_rtz_ushort2, __convert_uint2_sat_rtz_uint2, __convert_uint2_sat_ulong2, __convert_uint2_sat_rtz_char2, __convert_uint2_sat_rtz_short2, __convert_uint2_sat_rtz_int2, __convert_uint2_sat_long2, __convert_uint2_sat_rtz_float2, __convert_uint2_sat_rtz_double2 )
    #define convert_uint4_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_uint4_uchar4, __convert_uint4_ushort4, __convert_uint4_sat_rtz_uint4, __convert_uint4_sat_ulong4, __convert_uint4_sat_rtz_char4, __convert_uint4_sat_rtz_short4, __convert_uint4_sat_int4, __convert_uint4_sat_long4, __convert_uint4_sat_rtz_float4, __convert_uint4_sat_rtz_double4 )
    #define convert_uint8_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_uint8_uchar8, __convert_uint8_ushort8, __convert_uint8_sat_rtz_uint8, __convert_uint8_sat_rtz_ulong8, __convert_uint8_sat_rtz_char8, __convert_uint8_sat_rtz_short8, __convert_uint8_sat_rtz_int8, __convert_uint8_sat_rtz_long8, __convert_uint8_sat_rtz_float8, __convert_uint8_sat_rtz_double8 )
    #define convert_uint16_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_uint16_uchar16, __convert_uint16_sat_rtz_ushort16, __convert_uint16_sat_rtz_uint16, __convert_uint16_sat_rtz_ulong16, __convert_uint16_sat_rtz_char16, __convert_uint16_sat_rtz_short16, __convert_uint16_sat_rtz_int16, __convert_uint16_sat_rtz_long16, __convert_uint16_sat_rtz_float16, __convert_uint16_sat_rtz_double16 )
    #define convert_ulong(_X)    __builtin_overload( 1, _X, __convert_ulong_uchar, __convert_ulong_ushort, __convert_ulong_uint, __convert_ulong_ulong, __convert_ulong_char, __convert_ulong_short, __convert_ulong_int, __convert_ulong_long, __convert_ulong_rtz_float, __convert_ulong_rtz_double )
    #define convert_ulong2(_X)    __builtin_overload( 1, _X, __convert_ulong2_uchar2, __convert_ulong2_ushort2, __convert_ulong2_uint2, __convert_ulong2_ulong2, __convert_ulong2_char2, __convert_ulong2_short2, __convert_ulong2_int2, __convert_ulong2_long2, __convert_ulong2_rtz_float2, __convert_ulong2_rtz_double2 )
    #define convert_ulong4(_X)    __builtin_overload( 1, _X, __convert_ulong4_uchar4, __convert_ulong4_ushort4, __convert_ulong4_uint4, __convert_ulong4_ulong4, __convert_ulong4_char4, __convert_ulong4_short4, __convert_ulong4_int4, __convert_ulong4_long4, __convert_ulong4_rtz_float4, __convert_ulong4_double4 )
    #define convert_ulong8(_X)    __builtin_overload( 1, _X, __convert_ulong8_uchar8, __convert_ulong8_ushort8, __convert_ulong8_uint8, __convert_ulong8_ulong8, __convert_ulong8_char8, __convert_ulong8_short8, __convert_ulong8_int8, __convert_ulong8_long8, __convert_ulong8_float8, __convert_ulong8_double8 )
    #define convert_ulong16(_X)    __builtin_overload( 1, _X, __convert_ulong16_uchar16, __convert_ulong16_ushort16, __convert_ulong16_uint16, __convert_ulong16_ulong16, __convert_ulong16_char16, __convert_ulong16_short16, __convert_ulong16_int16, __convert_ulong16_long16, __convert_ulong16_float16, __convert_ulong16_double16 )
    #define convert_ulong_rte(_X)    __builtin_overload( 1, _X, __convert_ulong_uchar, __convert_ulong_ushort, __convert_ulong_uint, __convert_ulong_rte_ulong, __convert_ulong_rte_char, __convert_ulong_rte_short, __convert_ulong_rte_int, __convert_ulong_rte_long, __convert_ulong_rte_float, __convert_ulong_rte_double )
    #define convert_ulong2_rte(_X)    __builtin_overload( 1, _X, __convert_ulong2_uchar2, __convert_ulong2_ushort2, __convert_ulong2_uint2, __convert_ulong2_rte_ulong2, __convert_ulong2_rte_char2, __convert_ulong2_rte_short2, __convert_ulong2_rte_int2, __convert_ulong2_rte_long2, __convert_ulong2_rte_float2, __convert_ulong2_rte_double2 )
    #define convert_ulong4_rte(_X)    __builtin_overload( 1, _X, __convert_ulong4_uchar4, __convert_ulong4_ushort4, __convert_ulong4_uint4, __convert_ulong4_rte_ulong4, __convert_ulong4_rte_char4, __convert_ulong4_rte_short4, __convert_ulong4_rte_int4, __convert_ulong4_rte_long4, __convert_ulong4_rte_float4, __convert_ulong4_rte_double4 )
    #define convert_ulong8_rte(_X)    __builtin_overload( 1, _X, __convert_ulong8_uchar8, __convert_ulong8_ushort8, __convert_ulong8_rte_uint8, __convert_ulong8_rte_ulong8, __convert_ulong8_rte_char8, __convert_ulong8_rte_short8, __convert_ulong8_rte_int8, __convert_ulong8_rte_long8, __convert_ulong8_rte_float8, __convert_ulong8_rte_double8 )
    #define convert_ulong16_rte(_X)    __builtin_overload( 1, _X, __convert_ulong16_uchar16, __convert_ulong16_rte_ushort16, __convert_ulong16_rte_uint16, __convert_ulong16_rte_ulong16, __convert_ulong16_rte_char16, __convert_ulong16_rte_short16, __convert_ulong16_rte_int16, __convert_ulong16_rte_long16, __convert_ulong16_rte_float16, __convert_ulong16_rte_double16 )
    #define convert_ulong_rtp(_X)    __builtin_overload( 1, _X, __convert_ulong_uchar, __convert_ulong_ushort, __convert_ulong_uint, __convert_ulong_rtp_ulong, __convert_ulong_rtp_char, __convert_ulong_rtp_short, __convert_ulong_rtp_int, __convert_ulong_rtp_long, __convert_ulong_rtp_float, __convert_ulong_rtp_double )
    #define convert_ulong2_rtp(_X)    __builtin_overload( 1, _X, __convert_ulong2_uchar2, __convert_ulong2_ushort2, __convert_ulong2_uint2, __convert_ulong2_rtp_ulong2, __convert_ulong2_rtp_char2, __convert_ulong2_rtp_short2, __convert_ulong2_rtp_int2, __convert_ulong2_rtp_long2, __convert_ulong2_rtp_float2, __convert_ulong2_rtp_double2 )
    #define convert_ulong4_rtp(_X)    __builtin_overload( 1, _X, __convert_ulong4_uchar4, __convert_ulong4_ushort4, __convert_ulong4_uint4, __convert_ulong4_rtp_ulong4, __convert_ulong4_rtp_char4, __convert_ulong4_rtp_short4, __convert_ulong4_rtp_int4, __convert_ulong4_rtp_long4, __convert_ulong4_rtp_float4, __convert_ulong4_rtp_double4 )
    #define convert_ulong8_rtp(_X)    __builtin_overload( 1, _X, __convert_ulong8_uchar8, __convert_ulong8_ushort8, __convert_ulong8_rtp_uint8, __convert_ulong8_rtp_ulong8, __convert_ulong8_rtp_char8, __convert_ulong8_rtp_short8, __convert_ulong8_rtp_int8, __convert_ulong8_rtp_long8, __convert_ulong8_rtp_float8, __convert_ulong8_rtp_double8 )
    #define convert_ulong16_rtp(_X)    __builtin_overload( 1, _X, __convert_ulong16_uchar16, __convert_ulong16_rtp_ushort16, __convert_ulong16_rtp_uint16, __convert_ulong16_rtp_ulong16, __convert_ulong16_rtp_char16, __convert_ulong16_rtp_short16, __convert_ulong16_rtp_int16, __convert_ulong16_rtp_long16, __convert_ulong16_rtp_float16, __convert_ulong16_rtp_double16 )
    #define convert_ulong_rtn(_X)    __builtin_overload( 1, _X, __convert_ulong_uchar, __convert_ulong_ushort, __convert_ulong_uint, __convert_ulong_rtn_ulong, __convert_ulong_rtn_char, __convert_ulong_rtn_short, __convert_ulong_rtn_int, __convert_ulong_rtn_long, __convert_ulong_rtn_float, __convert_ulong_rtn_double )
    #define convert_ulong2_rtn(_X)    __builtin_overload( 1, _X, __convert_ulong2_uchar2, __convert_ulong2_ushort2, __convert_ulong2_uint2, __convert_ulong2_rtn_ulong2, __convert_ulong2_rtn_char2, __convert_ulong2_rtn_short2, __convert_ulong2_rtn_int2, __convert_ulong2_rtn_long2, __convert_ulong2_rtn_float2, __convert_ulong2_rtn_double2 )
    #define convert_ulong4_rtn(_X)    __builtin_overload( 1, _X, __convert_ulong4_uchar4, __convert_ulong4_ushort4, __convert_ulong4_uint4, __convert_ulong4_rtn_ulong4, __convert_ulong4_rtn_char4, __convert_ulong4_rtn_short4, __convert_ulong4_rtn_int4, __convert_ulong4_rtn_long4, __convert_ulong4_rtn_float4, __convert_ulong4_rtn_double4 )
    #define convert_ulong8_rtn(_X)    __builtin_overload( 1, _X, __convert_ulong8_uchar8, __convert_ulong8_ushort8, __convert_ulong8_rtn_uint8, __convert_ulong8_rtn_ulong8, __convert_ulong8_rtn_char8, __convert_ulong8_rtn_short8, __convert_ulong8_rtn_int8, __convert_ulong8_rtn_long8, __convert_ulong8_rtn_float8, __convert_ulong8_rtn_double8 )
    #define convert_ulong16_rtn(_X)    __builtin_overload( 1, _X, __convert_ulong16_uchar16, __convert_ulong16_rtn_ushort16, __convert_ulong16_rtn_uint16, __convert_ulong16_rtn_ulong16, __convert_ulong16_rtn_char16, __convert_ulong16_rtn_short16, __convert_ulong16_rtn_int16, __convert_ulong16_rtn_long16, __convert_ulong16_rtn_float16, __convert_ulong16_rtn_double16 )
    #define convert_ulong_rtz(_X)    __builtin_overload( 1, _X, __convert_ulong_uchar, __convert_ulong_ushort, __convert_ulong_uint, __convert_ulong_rtz_ulong, __convert_ulong_rtz_char, __convert_ulong_rtz_short, __convert_ulong_rtz_int, __convert_ulong_rtz_long, __convert_ulong_rtz_float, __convert_ulong_rtz_double )
    #define convert_ulong2_rtz(_X)    __builtin_overload( 1, _X, __convert_ulong2_uchar2, __convert_ulong2_ushort2, __convert_ulong2_uint2, __convert_ulong2_rtz_ulong2, __convert_ulong2_rtz_char2, __convert_ulong2_rtz_short2, __convert_ulong2_rtz_int2, __convert_ulong2_rtz_long2, __convert_ulong2_rtz_float2, __convert_ulong2_rtz_double2 )
    #define convert_ulong4_rtz(_X)    __builtin_overload( 1, _X, __convert_ulong4_uchar4, __convert_ulong4_ushort4, __convert_ulong4_uint4, __convert_ulong4_rtz_ulong4, __convert_ulong4_rtz_char4, __convert_ulong4_rtz_short4, __convert_ulong4_rtz_int4, __convert_ulong4_rtz_long4, __convert_ulong4_rtz_float4, __convert_ulong4_rtz_double4 )
    #define convert_ulong8_rtz(_X)    __builtin_overload( 1, _X, __convert_ulong8_uchar8, __convert_ulong8_ushort8, __convert_ulong8_rtz_uint8, __convert_ulong8_rtz_ulong8, __convert_ulong8_rtz_char8, __convert_ulong8_rtz_short8, __convert_ulong8_rtz_int8, __convert_ulong8_rtz_long8, __convert_ulong8_rtz_float8, __convert_ulong8_rtz_double8 )
    #define convert_ulong16_rtz(_X)    __builtin_overload( 1, _X, __convert_ulong16_uchar16, __convert_ulong16_rtz_ushort16, __convert_ulong16_rtz_uint16, __convert_ulong16_rtz_ulong16, __convert_ulong16_rtz_char16, __convert_ulong16_rtz_short16, __convert_ulong16_rtz_int16, __convert_ulong16_rtz_long16, __convert_ulong16_rtz_float16, __convert_ulong16_rtz_double16 )
    #define convert_ulong_sat(_X)    __builtin_overload( 1, _X, __convert_ulong_uchar, __convert_ulong_ushort, __convert_ulong_uint, __convert_ulong_sat_ulong, __convert_ulong_sat_char, __convert_ulong_sat_short, __convert_ulong_sat_int, __convert_ulong_sat_long, __convert_ulong_sat_rtz_float, __convert_ulong_sat_rtz_double )
    #define convert_ulong2_sat(_X)    __builtin_overload( 1, _X, __convert_ulong2_uchar2, __convert_ulong2_ushort2, __convert_ulong2_uint2, __convert_ulong2_sat_ulong2, __convert_ulong2_sat_char2, __convert_ulong2_sat_short2, __convert_ulong2_sat_int2, __convert_ulong2_sat_long2, __convert_ulong2_sat_rtz_float2, __convert_ulong2_sat_rtz_double2 )
    #define convert_ulong4_sat(_X)    __builtin_overload( 1, _X, __convert_ulong4_uchar4, __convert_ulong4_ushort4, __convert_ulong4_uint4, __convert_ulong4_sat_ulong4, __convert_ulong4_sat_char4, __convert_ulong4_sat_short4, __convert_ulong4_sat_int4, __convert_ulong4_sat_long4, __convert_ulong4_sat_rtz_float4, __convert_ulong4_sat_double4 )
    #define convert_ulong8_sat(_X)    __builtin_overload( 1, _X, __convert_ulong8_uchar8, __convert_ulong8_ushort8, __convert_ulong8_sat_uint8, __convert_ulong8_sat_ulong8, __convert_ulong8_sat_char8, __convert_ulong8_sat_short8, __convert_ulong8_sat_int8, __convert_ulong8_sat_long8, __convert_ulong8_sat_float8, __convert_ulong8_sat_double8 )
    #define convert_ulong16_sat(_X)    __builtin_overload( 1, _X, __convert_ulong16_uchar16, __convert_ulong16_sat_ushort16, __convert_ulong16_sat_uint16, __convert_ulong16_sat_ulong16, __convert_ulong16_sat_char16, __convert_ulong16_sat_short16, __convert_ulong16_sat_int16, __convert_ulong16_sat_long16, __convert_ulong16_sat_float16, __convert_ulong16_sat_double16 )
    #define convert_ulong_sat_rte(_X)    __builtin_overload( 1, _X, __convert_ulong_uchar, __convert_ulong_ushort, __convert_ulong_uint, __convert_ulong_sat_rte_ulong, __convert_ulong_sat_rte_char, __convert_ulong_sat_rte_short, __convert_ulong_sat_rte_int, __convert_ulong_sat_long, __convert_ulong_sat_rte_float, __convert_ulong_sat_rte_double )
    #define convert_ulong2_sat_rte(_X)    __builtin_overload( 1, _X, __convert_ulong2_uchar2, __convert_ulong2_ushort2, __convert_ulong2_uint2, __convert_ulong2_sat_rte_ulong2, __convert_ulong2_sat_rte_char2, __convert_ulong2_sat_rte_short2, __convert_ulong2_sat_rte_int2, __convert_ulong2_sat_long2, __convert_ulong2_sat_rte_float2, __convert_ulong2_sat_rte_double2 )
    #define convert_ulong4_sat_rte(_X)    __builtin_overload( 1, _X, __convert_ulong4_uchar4, __convert_ulong4_ushort4, __convert_ulong4_uint4, __convert_ulong4_sat_rte_ulong4, __convert_ulong4_sat_rte_char4, __convert_ulong4_sat_rte_short4, __convert_ulong4_sat_rte_int4, __convert_ulong4_sat_rte_long4, __convert_ulong4_sat_rte_float4, __convert_ulong4_sat_rte_double4 )
    #define convert_ulong8_sat_rte(_X)    __builtin_overload( 1, _X, __convert_ulong8_uchar8, __convert_ulong8_ushort8, __convert_ulong8_sat_rte_uint8, __convert_ulong8_sat_rte_ulong8, __convert_ulong8_sat_rte_char8, __convert_ulong8_sat_rte_short8, __convert_ulong8_sat_rte_int8, __convert_ulong8_sat_rte_long8, __convert_ulong8_sat_rte_float8, __convert_ulong8_sat_rte_double8 )
    #define convert_ulong16_sat_rte(_X)    __builtin_overload( 1, _X, __convert_ulong16_uchar16, __convert_ulong16_sat_rte_ushort16, __convert_ulong16_sat_rte_uint16, __convert_ulong16_sat_rte_ulong16, __convert_ulong16_sat_rte_char16, __convert_ulong16_sat_rte_short16, __convert_ulong16_sat_rte_int16, __convert_ulong16_sat_rte_long16, __convert_ulong16_sat_rte_float16, __convert_ulong16_sat_rte_double16 )
    #define convert_ulong_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_ulong_uchar, __convert_ulong_ushort, __convert_ulong_uint, __convert_ulong_sat_rtp_ulong, __convert_ulong_sat_rtp_char, __convert_ulong_sat_rtp_short, __convert_ulong_sat_rtp_int, __convert_ulong_sat_long, __convert_ulong_sat_rtp_float, __convert_ulong_sat_rtp_double )
    #define convert_ulong2_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_ulong2_uchar2, __convert_ulong2_ushort2, __convert_ulong2_uint2, __convert_ulong2_sat_rtp_ulong2, __convert_ulong2_sat_rtp_char2, __convert_ulong2_sat_rtp_short2, __convert_ulong2_sat_rtp_int2, __convert_ulong2_sat_long2, __convert_ulong2_sat_rtp_float2, __convert_ulong2_sat_rtp_double2 )
    #define convert_ulong4_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_ulong4_uchar4, __convert_ulong4_ushort4, __convert_ulong4_uint4, __convert_ulong4_sat_rtp_ulong4, __convert_ulong4_sat_rtp_char4, __convert_ulong4_sat_rtp_short4, __convert_ulong4_sat_rtp_int4, __convert_ulong4_sat_rtp_long4, __convert_ulong4_sat_rtp_float4, __convert_ulong4_sat_rtp_double4 )
    #define convert_ulong8_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_ulong8_uchar8, __convert_ulong8_ushort8, __convert_ulong8_sat_rtp_uint8, __convert_ulong8_sat_rtp_ulong8, __convert_ulong8_sat_rtp_char8, __convert_ulong8_sat_rtp_short8, __convert_ulong8_sat_rtp_int8, __convert_ulong8_sat_rtp_long8, __convert_ulong8_sat_rtp_float8, __convert_ulong8_sat_rtp_double8 )
    #define convert_ulong16_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_ulong16_uchar16, __convert_ulong16_sat_rtp_ushort16, __convert_ulong16_sat_rtp_uint16, __convert_ulong16_sat_rtp_ulong16, __convert_ulong16_sat_rtp_char16, __convert_ulong16_sat_rtp_short16, __convert_ulong16_sat_rtp_int16, __convert_ulong16_sat_rtp_long16, __convert_ulong16_sat_rtp_float16, __convert_ulong16_sat_rtp_double16 )
    #define convert_ulong_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_ulong_uchar, __convert_ulong_ushort, __convert_ulong_uint, __convert_ulong_sat_rtn_ulong, __convert_ulong_sat_rtn_char, __convert_ulong_sat_rtn_short, __convert_ulong_sat_rtn_int, __convert_ulong_sat_long, __convert_ulong_sat_rtn_float, __convert_ulong_sat_rtn_double )
    #define convert_ulong2_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_ulong2_uchar2, __convert_ulong2_ushort2, __convert_ulong2_uint2, __convert_ulong2_sat_rtn_ulong2, __convert_ulong2_sat_rtn_char2, __convert_ulong2_sat_rtn_short2, __convert_ulong2_sat_rtn_int2, __convert_ulong2_sat_long2, __convert_ulong2_sat_rtn_float2, __convert_ulong2_sat_rtn_double2 )
    #define convert_ulong4_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_ulong4_uchar4, __convert_ulong4_ushort4, __convert_ulong4_uint4, __convert_ulong4_sat_rtn_ulong4, __convert_ulong4_sat_rtn_char4, __convert_ulong4_sat_rtn_short4, __convert_ulong4_sat_rtn_int4, __convert_ulong4_sat_rtn_long4, __convert_ulong4_sat_rtn_float4, __convert_ulong4_sat_rtn_double4 )
    #define convert_ulong8_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_ulong8_uchar8, __convert_ulong8_ushort8, __convert_ulong8_sat_rtn_uint8, __convert_ulong8_sat_rtn_ulong8, __convert_ulong8_sat_rtn_char8, __convert_ulong8_sat_rtn_short8, __convert_ulong8_sat_rtn_int8, __convert_ulong8_sat_rtn_long8, __convert_ulong8_sat_rtn_float8, __convert_ulong8_sat_rtn_double8 )
    #define convert_ulong16_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_ulong16_uchar16, __convert_ulong16_sat_rtn_ushort16, __convert_ulong16_sat_rtn_uint16, __convert_ulong16_sat_rtn_ulong16, __convert_ulong16_sat_rtn_char16, __convert_ulong16_sat_rtn_short16, __convert_ulong16_sat_rtn_int16, __convert_ulong16_sat_rtn_long16, __convert_ulong16_sat_rtn_float16, __convert_ulong16_sat_rtn_double16 )
    #define convert_ulong_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_ulong_uchar, __convert_ulong_ushort, __convert_ulong_uint, __convert_ulong_sat_rtz_ulong, __convert_ulong_sat_rtz_char, __convert_ulong_sat_rtz_short, __convert_ulong_sat_rtz_int, __convert_ulong_sat_long, __convert_ulong_sat_rtz_float, __convert_ulong_sat_rtz_double )
    #define convert_ulong2_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_ulong2_uchar2, __convert_ulong2_ushort2, __convert_ulong2_uint2, __convert_ulong2_sat_rtz_ulong2, __convert_ulong2_sat_rtz_char2, __convert_ulong2_sat_rtz_short2, __convert_ulong2_sat_rtz_int2, __convert_ulong2_sat_long2, __convert_ulong2_sat_rtz_float2, __convert_ulong2_sat_rtz_double2 )
    #define convert_ulong4_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_ulong4_uchar4, __convert_ulong4_ushort4, __convert_ulong4_uint4, __convert_ulong4_sat_rtz_ulong4, __convert_ulong4_sat_rtz_char4, __convert_ulong4_sat_rtz_short4, __convert_ulong4_sat_rtz_int4, __convert_ulong4_sat_rtz_long4, __convert_ulong4_sat_rtz_float4, __convert_ulong4_sat_rtz_double4 )
    #define convert_ulong8_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_ulong8_uchar8, __convert_ulong8_ushort8, __convert_ulong8_sat_rtz_uint8, __convert_ulong8_sat_rtz_ulong8, __convert_ulong8_sat_rtz_char8, __convert_ulong8_sat_rtz_short8, __convert_ulong8_sat_rtz_int8, __convert_ulong8_sat_rtz_long8, __convert_ulong8_sat_rtz_float8, __convert_ulong8_sat_rtz_double8 )
    #define convert_ulong16_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_ulong16_uchar16, __convert_ulong16_sat_rtz_ushort16, __convert_ulong16_sat_rtz_uint16, __convert_ulong16_sat_rtz_ulong16, __convert_ulong16_sat_rtz_char16, __convert_ulong16_sat_rtz_short16, __convert_ulong16_sat_rtz_int16, __convert_ulong16_sat_rtz_long16, __convert_ulong16_sat_rtz_float16, __convert_ulong16_sat_rtz_double16 )
    #define convert_char(_X)    __builtin_overload( 1, _X, __convert_char_uchar, __convert_char_ushort, __convert_char_uint, __convert_char_ulong, __convert_char_char, __convert_char_short, __convert_char_int, __convert_char_long, __convert_char_float, __convert_char_double )
    #define convert_char2(_X)    __builtin_overload( 1, _X, __convert_char2_uchar2, __convert_char2_ushort2, __convert_char2_uint2, __convert_char2_ulong2, __convert_char2_char2, __convert_char2_short2, __convert_char2_int2, __convert_char2_long2, __convert_char2_float2, __convert_char2_double2 )
    #define convert_char4(_X)    __builtin_overload( 1, _X, __convert_char4_uchar4, __convert_char4_ushort4, __convert_char4_uint4, __convert_char4_ulong4, __convert_char4_char4, __convert_char4_short4, __convert_char4_int4, __convert_char4_long4, __convert_char4_float4, __convert_char4_double4 )
    #define convert_char8(_X)    __builtin_overload( 1, _X, __convert_char8_uchar8, __convert_char8_ushort8, __convert_char8_uint8, __convert_char8_ulong8, __convert_char8_char8, __convert_char8_short8, __convert_char8_int8, __convert_char8_long8, __convert_char8_float8, __convert_char8_double8 )
    #define convert_char16(_X)    __builtin_overload( 1, _X, __convert_char16_uchar16, __convert_char16_ushort16, __convert_char16_uint16, __convert_char16_ulong16, __convert_char16_char16, __convert_char16_short16, __convert_char16_int16, __convert_char16_long16, __convert_char16_float16, __convert_char16_double16 )
    #define convert_char_rte(_X)    __builtin_overload( 1, _X, __convert_char_rte_uchar, __convert_char_rte_ushort, __convert_char_rte_uint, __convert_char_rte_ulong, __convert_char_rte_char, __convert_char_rte_short, __convert_char_rte_int, __convert_char_rte_long, __convert_char_rte_float, __convert_char_rte_double )
    #define convert_char2_rte(_X)    __builtin_overload( 1, _X, __convert_char2_rte_uchar2, __convert_char2_rte_ushort2, __convert_char2_rte_uint2, __convert_char2_rte_ulong2, __convert_char2_rte_char2, __convert_char2_rte_short2, __convert_char2_rte_int2, __convert_char2_rte_long2, __convert_char2_rte_float2, __convert_char2_rte_double2 )
    #define convert_char4_rte(_X)    __builtin_overload( 1, _X, __convert_char4_rte_uchar4, __convert_char4_rte_ushort4, __convert_char4_rte_uint4, __convert_char4_rte_ulong4, __convert_char4_rte_char4, __convert_char4_rte_short4, __convert_char4_rte_int4, __convert_char4_rte_long4, __convert_char4_rte_float4, __convert_char4_rte_double4 )
    #define convert_char8_rte(_X)    __builtin_overload( 1, _X, __convert_char8_rte_uchar8, __convert_char8_rte_ushort8, __convert_char8_rte_uint8, __convert_char8_rte_ulong8, __convert_char8_rte_char8, __convert_char8_rte_short8, __convert_char8_rte_int8, __convert_char8_rte_long8, __convert_char8_rte_float8, __convert_char8_rte_double8 )
    #define convert_char16_rte(_X)    __builtin_overload( 1, _X, __convert_char16_rte_uchar16, __convert_char16_rte_ushort16, __convert_char16_rte_uint16, __convert_char16_rte_ulong16, __convert_char16_rte_char16, __convert_char16_rte_short16, __convert_char16_rte_int16, __convert_char16_rte_long16, __convert_char16_rte_float16, __convert_char16_rte_double16 )
    #define convert_char_rtp(_X)    __builtin_overload( 1, _X, __convert_char_rtp_uchar, __convert_char_rtp_ushort, __convert_char_rtp_uint, __convert_char_rtp_ulong, __convert_char_rtp_char, __convert_char_rtp_short, __convert_char_rtp_int, __convert_char_rtp_long, __convert_char_rtp_float, __convert_char_rtp_double )
    #define convert_char2_rtp(_X)    __builtin_overload( 1, _X, __convert_char2_rtp_uchar2, __convert_char2_rtp_ushort2, __convert_char2_rtp_uint2, __convert_char2_rtp_ulong2, __convert_char2_rtp_char2, __convert_char2_rtp_short2, __convert_char2_rtp_int2, __convert_char2_rtp_long2, __convert_char2_rtp_float2, __convert_char2_rtp_double2 )
    #define convert_char4_rtp(_X)    __builtin_overload( 1, _X, __convert_char4_rtp_uchar4, __convert_char4_rtp_ushort4, __convert_char4_rtp_uint4, __convert_char4_rtp_ulong4, __convert_char4_rtp_char4, __convert_char4_rtp_short4, __convert_char4_rtp_int4, __convert_char4_rtp_long4, __convert_char4_rtp_float4, __convert_char4_rtp_double4 )
    #define convert_char8_rtp(_X)    __builtin_overload( 1, _X, __convert_char8_rtp_uchar8, __convert_char8_rtp_ushort8, __convert_char8_rtp_uint8, __convert_char8_rtp_ulong8, __convert_char8_rtp_char8, __convert_char8_rtp_short8, __convert_char8_rtp_int8, __convert_char8_rtp_long8, __convert_char8_rtp_float8, __convert_char8_rtp_double8 )
    #define convert_char16_rtp(_X)    __builtin_overload( 1, _X, __convert_char16_rtp_uchar16, __convert_char16_rtp_ushort16, __convert_char16_rtp_uint16, __convert_char16_rtp_ulong16, __convert_char16_rtp_char16, __convert_char16_rtp_short16, __convert_char16_rtp_int16, __convert_char16_rtp_long16, __convert_char16_rtp_float16, __convert_char16_rtp_double16 )
    #define convert_char_rtn(_X)    __builtin_overload( 1, _X, __convert_char_rtn_uchar, __convert_char_rtn_ushort, __convert_char_rtn_uint, __convert_char_rtn_ulong, __convert_char_rtn_char, __convert_char_rtn_short, __convert_char_rtn_int, __convert_char_rtn_long, __convert_char_rtn_float, __convert_char_rtn_double )
    #define convert_char2_rtn(_X)    __builtin_overload( 1, _X, __convert_char2_rtn_uchar2, __convert_char2_rtn_ushort2, __convert_char2_rtn_uint2, __convert_char2_rtn_ulong2, __convert_char2_rtn_char2, __convert_char2_rtn_short2, __convert_char2_rtn_int2, __convert_char2_rtn_long2, __convert_char2_rtn_float2, __convert_char2_rtn_double2 )
    #define convert_char4_rtn(_X)    __builtin_overload( 1, _X, __convert_char4_rtn_uchar4, __convert_char4_rtn_ushort4, __convert_char4_rtn_uint4, __convert_char4_rtn_ulong4, __convert_char4_rtn_char4, __convert_char4_rtn_short4, __convert_char4_rtn_int4, __convert_char4_rtn_long4, __convert_char4_rtn_float4, __convert_char4_rtn_double4 )
    #define convert_char8_rtn(_X)    __builtin_overload( 1, _X, __convert_char8_rtn_uchar8, __convert_char8_rtn_ushort8, __convert_char8_rtn_uint8, __convert_char8_rtn_ulong8, __convert_char8_rtn_char8, __convert_char8_rtn_short8, __convert_char8_rtn_int8, __convert_char8_rtn_long8, __convert_char8_rtn_float8, __convert_char8_rtn_double8 )
    #define convert_char16_rtn(_X)    __builtin_overload( 1, _X, __convert_char16_rtn_uchar16, __convert_char16_rtn_ushort16, __convert_char16_rtn_uint16, __convert_char16_rtn_ulong16, __convert_char16_rtn_char16, __convert_char16_rtn_short16, __convert_char16_rtn_int16, __convert_char16_rtn_long16, __convert_char16_rtn_float16, __convert_char16_rtn_double16 )
    #define convert_char_rtz(_X)    __builtin_overload( 1, _X, __convert_char_rtz_uchar, __convert_char_rtz_ushort, __convert_char_rtz_uint, __convert_char_rtz_ulong, __convert_char_rtz_char, __convert_char_rtz_short, __convert_char_rtz_int, __convert_char_rtz_long, __convert_char_rtz_float, __convert_char_rtz_double )
    #define convert_char2_rtz(_X)    __builtin_overload( 1, _X, __convert_char2_rtz_uchar2, __convert_char2_rtz_ushort2, __convert_char2_rtz_uint2, __convert_char2_rtz_ulong2, __convert_char2_rtz_char2, __convert_char2_rtz_short2, __convert_char2_rtz_int2, __convert_char2_rtz_long2, __convert_char2_rtz_float2, __convert_char2_rtz_double2 )
    #define convert_char4_rtz(_X)    __builtin_overload( 1, _X, __convert_char4_rtz_uchar4, __convert_char4_rtz_ushort4, __convert_char4_rtz_uint4, __convert_char4_rtz_ulong4, __convert_char4_rtz_char4, __convert_char4_rtz_short4, __convert_char4_rtz_int4, __convert_char4_rtz_long4, __convert_char4_rtz_float4, __convert_char4_rtz_double4 )
    #define convert_char8_rtz(_X)    __builtin_overload( 1, _X, __convert_char8_rtz_uchar8, __convert_char8_rtz_ushort8, __convert_char8_rtz_uint8, __convert_char8_rtz_ulong8, __convert_char8_rtz_char8, __convert_char8_rtz_short8, __convert_char8_rtz_int8, __convert_char8_rtz_long8, __convert_char8_rtz_float8, __convert_char8_rtz_double8 )
    #define convert_char16_rtz(_X)    __builtin_overload( 1, _X, __convert_char16_rtz_uchar16, __convert_char16_rtz_ushort16, __convert_char16_rtz_uint16, __convert_char16_rtz_ulong16, __convert_char16_rtz_char16, __convert_char16_rtz_short16, __convert_char16_rtz_int16, __convert_char16_rtz_long16, __convert_char16_rtz_float16, __convert_char16_rtz_double16 )
    #define convert_char_sat(_X)    __builtin_overload( 1, _X, __convert_char_sat_uchar, __convert_char_sat_ushort, __convert_char_sat_uint, __convert_char_sat_ulong, __convert_char_sat_char, __convert_char_sat_short, __convert_char_sat_int, __convert_char_sat_long, __convert_char_sat_float, __convert_char_sat_double )
    #define convert_char2_sat(_X)    __builtin_overload( 1, _X, __convert_char2_sat_uchar2, __convert_char2_sat_ushort2, __convert_char2_sat_uint2, __convert_char2_sat_ulong2, __convert_char2_sat_char2, __convert_char2_sat_short2, __convert_char2_sat_int2, __convert_char2_sat_long2, __convert_char2_sat_float2, __convert_char2_sat_double2 )
    #define convert_char4_sat(_X)    __builtin_overload( 1, _X, __convert_char4_sat_uchar4, __convert_char4_sat_ushort4, __convert_char4_sat_uint4, __convert_char4_sat_ulong4, __convert_char4_sat_char4, __convert_char4_sat_short4, __convert_char4_sat_int4, __convert_char4_sat_long4, __convert_char4_sat_float4, __convert_char4_sat_double4 )
    #define convert_char8_sat(_X)    __builtin_overload( 1, _X, __convert_char8_sat_uchar8, __convert_char8_sat_ushort8, __convert_char8_sat_uint8, __convert_char8_sat_ulong8, __convert_char8_sat_char8, __convert_char8_sat_short8, __convert_char8_sat_int8, __convert_char8_sat_long8, __convert_char8_sat_float8, __convert_char8_sat_double8 )
    #define convert_char16_sat(_X)    __builtin_overload( 1, _X, __convert_char16_sat_uchar16, __convert_char16_sat_ushort16, __convert_char16_sat_uint16, __convert_char16_sat_ulong16, __convert_char16_sat_char16, __convert_char16_sat_short16, __convert_char16_sat_int16, __convert_char16_sat_long16, __convert_char16_sat_float16, __convert_char16_sat_double16 )
    #define convert_char_sat_rte(_X)    __builtin_overload( 1, _X, __convert_char_sat_uchar, __convert_char_sat_ushort, __convert_char_sat_rte_uint, __convert_char_sat_rte_ulong, __convert_char_sat_rte_char, __convert_char_sat_short, __convert_char_sat_rte_int, __convert_char_sat_rte_long, __convert_char_sat_rte_float, __convert_char_sat_rte_double )
    #define convert_char2_sat_rte(_X)    __builtin_overload( 1, _X, __convert_char2_sat_rte_uchar2, __convert_char2_sat_rte_ushort2, __convert_char2_sat_rte_uint2, __convert_char2_sat_rte_ulong2, __convert_char2_sat_rte_char2, __convert_char2_sat_rte_short2, __convert_char2_sat_rte_int2, __convert_char2_sat_rte_long2, __convert_char2_sat_rte_float2, __convert_char2_sat_rte_double2 )
    #define convert_char4_sat_rte(_X)    __builtin_overload( 1, _X, __convert_char4_sat_rte_uchar4, __convert_char4_sat_rte_ushort4, __convert_char4_sat_rte_uint4, __convert_char4_sat_rte_ulong4, __convert_char4_sat_rte_char4, __convert_char4_sat_rte_short4, __convert_char4_sat_rte_int4, __convert_char4_sat_rte_long4, __convert_char4_sat_rte_float4, __convert_char4_sat_rte_double4 )
    #define convert_char8_sat_rte(_X)    __builtin_overload( 1, _X, __convert_char8_sat_rte_uchar8, __convert_char8_sat_ushort8, __convert_char8_sat_rte_uint8, __convert_char8_sat_rte_ulong8, __convert_char8_sat_rte_char8, __convert_char8_sat_short8, __convert_char8_sat_rte_int8, __convert_char8_sat_rte_long8, __convert_char8_sat_rte_float8, __convert_char8_sat_rte_double8 )
    #define convert_char16_sat_rte(_X)    __builtin_overload( 1, _X, __convert_char16_sat_uchar16, __convert_char16_sat_ushort16, __convert_char16_sat_rte_uint16, __convert_char16_sat_rte_ulong16, __convert_char16_sat_rte_char16, __convert_char16_sat_short16, __convert_char16_sat_rte_int16, __convert_char16_sat_rte_long16, __convert_char16_sat_rte_float16, __convert_char16_sat_rte_double16 )
    #define convert_char_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_char_sat_uchar, __convert_char_sat_ushort, __convert_char_sat_rtp_uint, __convert_char_sat_rtp_ulong, __convert_char_sat_rtp_char, __convert_char_sat_short, __convert_char_sat_rtp_int, __convert_char_sat_rtp_long, __convert_char_sat_rtp_float, __convert_char_sat_rtp_double )
    #define convert_char2_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_char2_sat_rtp_uchar2, __convert_char2_sat_rtp_ushort2, __convert_char2_sat_rtp_uint2, __convert_char2_sat_rtp_ulong2, __convert_char2_sat_rtp_char2, __convert_char2_sat_rtp_short2, __convert_char2_sat_rtp_int2, __convert_char2_sat_rtp_long2, __convert_char2_sat_rtp_float2, __convert_char2_sat_rtp_double2 )
    #define convert_char4_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_char4_sat_rtp_uchar4, __convert_char4_sat_rtp_ushort4, __convert_char4_sat_rtp_uint4, __convert_char4_sat_rtp_ulong4, __convert_char4_sat_rtp_char4, __convert_char4_sat_rtp_short4, __convert_char4_sat_rtp_int4, __convert_char4_sat_rtp_long4, __convert_char4_sat_rtp_float4, __convert_char4_sat_rtp_double4 )
    #define convert_char8_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_char8_sat_rtp_uchar8, __convert_char8_sat_ushort8, __convert_char8_sat_rtp_uint8, __convert_char8_sat_rtp_ulong8, __convert_char8_sat_rtp_char8, __convert_char8_sat_short8, __convert_char8_sat_rtp_int8, __convert_char8_sat_rtp_long8, __convert_char8_sat_rtp_float8, __convert_char8_sat_rtp_double8 )
    #define convert_char16_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_char16_sat_uchar16, __convert_char16_sat_ushort16, __convert_char16_sat_rtp_uint16, __convert_char16_sat_rtp_ulong16, __convert_char16_sat_rtp_char16, __convert_char16_sat_short16, __convert_char16_sat_rtp_int16, __convert_char16_sat_rtp_long16, __convert_char16_sat_rtp_float16, __convert_char16_sat_rtp_double16 )
    #define convert_char_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_char_sat_uchar, __convert_char_sat_ushort, __convert_char_sat_rtn_uint, __convert_char_sat_rtn_ulong, __convert_char_sat_rtn_char, __convert_char_sat_short, __convert_char_sat_rtn_int, __convert_char_sat_rtn_long, __convert_char_sat_rtn_float, __convert_char_sat_rtn_double )
    #define convert_char2_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_char2_sat_rtn_uchar2, __convert_char2_sat_rtn_ushort2, __convert_char2_sat_rtn_uint2, __convert_char2_sat_rtn_ulong2, __convert_char2_sat_rtn_char2, __convert_char2_sat_rtn_short2, __convert_char2_sat_rtn_int2, __convert_char2_sat_rtn_long2, __convert_char2_sat_rtn_float2, __convert_char2_sat_rtn_double2 )
    #define convert_char4_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_char4_sat_rtn_uchar4, __convert_char4_sat_rtn_ushort4, __convert_char4_sat_rtn_uint4, __convert_char4_sat_rtn_ulong4, __convert_char4_sat_rtn_char4, __convert_char4_sat_rtn_short4, __convert_char4_sat_rtn_int4, __convert_char4_sat_rtn_long4, __convert_char4_sat_rtn_float4, __convert_char4_sat_rtn_double4 )
    #define convert_char8_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_char8_sat_rtn_uchar8, __convert_char8_sat_ushort8, __convert_char8_sat_rtn_uint8, __convert_char8_sat_rtn_ulong8, __convert_char8_sat_rtn_char8, __convert_char8_sat_short8, __convert_char8_sat_rtn_int8, __convert_char8_sat_rtn_long8, __convert_char8_sat_rtn_float8, __convert_char8_sat_rtn_double8 )
    #define convert_char16_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_char16_sat_uchar16, __convert_char16_sat_ushort16, __convert_char16_sat_rtn_uint16, __convert_char16_sat_rtn_ulong16, __convert_char16_sat_rtn_char16, __convert_char16_sat_short16, __convert_char16_sat_rtn_int16, __convert_char16_sat_rtn_long16, __convert_char16_sat_rtn_float16, __convert_char16_sat_rtn_double16 )
    #define convert_char_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_char_sat_uchar, __convert_char_sat_ushort, __convert_char_sat_rtz_uint, __convert_char_sat_rtz_ulong, __convert_char_sat_rtz_char, __convert_char_sat_short, __convert_char_sat_rtz_int, __convert_char_sat_rtz_long, __convert_char_sat_rtz_float, __convert_char_sat_rtz_double )
    #define convert_char2_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_char2_sat_rtz_uchar2, __convert_char2_sat_rtz_ushort2, __convert_char2_sat_rtz_uint2, __convert_char2_sat_rtz_ulong2, __convert_char2_sat_rtz_char2, __convert_char2_sat_rtz_short2, __convert_char2_sat_rtz_int2, __convert_char2_sat_rtz_long2, __convert_char2_sat_rtz_float2, __convert_char2_sat_rtz_double2 )
    #define convert_char4_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_char4_sat_rtz_uchar4, __convert_char4_sat_rtz_ushort4, __convert_char4_sat_rtz_uint4, __convert_char4_sat_rtz_ulong4, __convert_char4_sat_rtz_char4, __convert_char4_sat_rtz_short4, __convert_char4_sat_rtz_int4, __convert_char4_sat_rtz_long4, __convert_char4_sat_rtz_float4, __convert_char4_sat_rtz_double4 )
    #define convert_char8_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_char8_sat_rtz_uchar8, __convert_char8_sat_ushort8, __convert_char8_sat_rtz_uint8, __convert_char8_sat_rtz_ulong8, __convert_char8_sat_rtz_char8, __convert_char8_sat_short8, __convert_char8_sat_rtz_int8, __convert_char8_sat_rtz_long8, __convert_char8_sat_rtz_float8, __convert_char8_sat_rtz_double8 )
    #define convert_char16_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_char16_sat_uchar16, __convert_char16_sat_ushort16, __convert_char16_sat_rtz_uint16, __convert_char16_sat_rtz_ulong16, __convert_char16_sat_rtz_char16, __convert_char16_sat_short16, __convert_char16_sat_rtz_int16, __convert_char16_sat_rtz_long16, __convert_char16_sat_rtz_float16, __convert_char16_sat_rtz_double16 )
    #define convert_short(_X)    __builtin_overload( 1, _X, __convert_short_uchar, __convert_short_ushort, __convert_short_uint, __convert_short_ulong, __convert_short_char, __convert_short_short, __convert_short_int, __convert_short_long, __convert_short_float, __convert_short_double )
    #define convert_short2(_X)    __builtin_overload( 1, _X, __convert_short2_uchar2, __convert_short2_ushort2, __convert_short2_uint2, __convert_short2_ulong2, __convert_short2_char2, __convert_short2_short2, __convert_short2_int2, __convert_short2_long2, __convert_short2_float2, __convert_short2_double2 )
    #define convert_short4(_X)    __builtin_overload( 1, _X, __convert_short4_uchar4, __convert_short4_ushort4, __convert_short4_uint4, __convert_short4_ulong4, __convert_short4_char4, __convert_short4_short4, __convert_short4_int4, __convert_short4_long4, __convert_short4_float4, __convert_short4_double4 )
    #define convert_short8(_X)    __builtin_overload( 1, _X, __convert_short8_uchar8, __convert_short8_ushort8, __convert_short8_uint8, __convert_short8_ulong8, __convert_short8_char8, __convert_short8_short8, __convert_short8_int8, __convert_short8_long8, __convert_short8_float8, __convert_short8_double8 )
    #define convert_short16(_X)    __builtin_overload( 1, _X, __convert_short16_uchar16, __convert_short16_ushort16, __convert_short16_uint16, __convert_short16_ulong16, __convert_short16_char16, __convert_short16_short16, __convert_short16_int16, __convert_short16_long16, __convert_short16_float16, __convert_short16_double16 )
    #define convert_short_rte(_X)    __builtin_overload( 1, _X, __convert_short_rte_uchar, __convert_short_rte_ushort, __convert_short_rte_uint, __convert_short_rte_ulong, __convert_short_char, __convert_short_rte_short, __convert_short_rte_int, __convert_short_rte_long, __convert_short_rte_float, __convert_short_rte_double )
    #define convert_short2_rte(_X)    __builtin_overload( 1, _X, __convert_short2_rte_uchar2, __convert_short2_rte_ushort2, __convert_short2_rte_uint2, __convert_short2_rte_ulong2, __convert_short2_rte_char2, __convert_short2_rte_short2, __convert_short2_rte_int2, __convert_short2_rte_long2, __convert_short2_rte_float2, __convert_short2_rte_double2 )
    #define convert_short4_rte(_X)    __builtin_overload( 1, _X, __convert_short4_rte_uchar4, __convert_short4_rte_ushort4, __convert_short4_rte_uint4, __convert_short4_rte_ulong4, __convert_short4_rte_char4, __convert_short4_rte_short4, __convert_short4_rte_int4, __convert_short4_rte_long4, __convert_short4_rte_float4, __convert_short4_rte_double4 )
    #define convert_short8_rte(_X)    __builtin_overload( 1, _X, __convert_short8_rte_uchar8, __convert_short8_rte_ushort8, __convert_short8_rte_uint8, __convert_short8_rte_ulong8, __convert_short8_char8, __convert_short8_rte_short8, __convert_short8_rte_int8, __convert_short8_rte_long8, __convert_short8_rte_float8, __convert_short8_rte_double8 )
    #define convert_short16_rte(_X)    __builtin_overload( 1, _X, __convert_short16_rte_uchar16, __convert_short16_rte_ushort16, __convert_short16_rte_uint16, __convert_short16_rte_ulong16, __convert_short16_char16, __convert_short16_rte_short16, __convert_short16_rte_int16, __convert_short16_rte_long16, __convert_short16_rte_float16, __convert_short16_rte_double16 )
    #define convert_short_rtp(_X)    __builtin_overload( 1, _X, __convert_short_rtp_uchar, __convert_short_rtp_ushort, __convert_short_rtp_uint, __convert_short_rtp_ulong, __convert_short_char, __convert_short_rtp_short, __convert_short_rtp_int, __convert_short_rtp_long, __convert_short_rtp_float, __convert_short_rtp_double )
    #define convert_short2_rtp(_X)    __builtin_overload( 1, _X, __convert_short2_rtp_uchar2, __convert_short2_rtp_ushort2, __convert_short2_rtp_uint2, __convert_short2_rtp_ulong2, __convert_short2_rtp_char2, __convert_short2_rtp_short2, __convert_short2_rtp_int2, __convert_short2_rtp_long2, __convert_short2_rtp_float2, __convert_short2_rtp_double2 )
    #define convert_short4_rtp(_X)    __builtin_overload( 1, _X, __convert_short4_rtp_uchar4, __convert_short4_rtp_ushort4, __convert_short4_rtp_uint4, __convert_short4_rtp_ulong4, __convert_short4_rtp_char4, __convert_short4_rtp_short4, __convert_short4_rtp_int4, __convert_short4_rtp_long4, __convert_short4_rtp_float4, __convert_short4_rtp_double4 )
    #define convert_short8_rtp(_X)    __builtin_overload( 1, _X, __convert_short8_rtp_uchar8, __convert_short8_rtp_ushort8, __convert_short8_rtp_uint8, __convert_short8_rtp_ulong8, __convert_short8_char8, __convert_short8_rtp_short8, __convert_short8_rtp_int8, __convert_short8_rtp_long8, __convert_short8_rtp_float8, __convert_short8_rtp_double8 )
    #define convert_short16_rtp(_X)    __builtin_overload( 1, _X, __convert_short16_rtp_uchar16, __convert_short16_rtp_ushort16, __convert_short16_rtp_uint16, __convert_short16_rtp_ulong16, __convert_short16_char16, __convert_short16_rtp_short16, __convert_short16_rtp_int16, __convert_short16_rtp_long16, __convert_short16_rtp_float16, __convert_short16_rtp_double16 )
    #define convert_short_rtn(_X)    __builtin_overload( 1, _X, __convert_short_rtn_uchar, __convert_short_rtn_ushort, __convert_short_rtn_uint, __convert_short_rtn_ulong, __convert_short_char, __convert_short_rtn_short, __convert_short_rtn_int, __convert_short_rtn_long, __convert_short_rtn_float, __convert_short_rtn_double )
    #define convert_short2_rtn(_X)    __builtin_overload( 1, _X, __convert_short2_rtn_uchar2, __convert_short2_rtn_ushort2, __convert_short2_rtn_uint2, __convert_short2_rtn_ulong2, __convert_short2_rtn_char2, __convert_short2_rtn_short2, __convert_short2_rtn_int2, __convert_short2_rtn_long2, __convert_short2_rtn_float2, __convert_short2_rtn_double2 )
    #define convert_short4_rtn(_X)    __builtin_overload( 1, _X, __convert_short4_rtn_uchar4, __convert_short4_rtn_ushort4, __convert_short4_rtn_uint4, __convert_short4_rtn_ulong4, __convert_short4_rtn_char4, __convert_short4_rtn_short4, __convert_short4_rtn_int4, __convert_short4_rtn_long4, __convert_short4_rtn_float4, __convert_short4_rtn_double4 )
    #define convert_short8_rtn(_X)    __builtin_overload( 1, _X, __convert_short8_rtn_uchar8, __convert_short8_rtn_ushort8, __convert_short8_rtn_uint8, __convert_short8_rtn_ulong8, __convert_short8_char8, __convert_short8_rtn_short8, __convert_short8_rtn_int8, __convert_short8_rtn_long8, __convert_short8_rtn_float8, __convert_short8_rtn_double8 )
    #define convert_short16_rtn(_X)    __builtin_overload( 1, _X, __convert_short16_rtn_uchar16, __convert_short16_rtn_ushort16, __convert_short16_rtn_uint16, __convert_short16_rtn_ulong16, __convert_short16_char16, __convert_short16_rtn_short16, __convert_short16_rtn_int16, __convert_short16_rtn_long16, __convert_short16_rtn_float16, __convert_short16_rtn_double16 )
    #define convert_short_rtz(_X)    __builtin_overload( 1, _X, __convert_short_rtz_uchar, __convert_short_rtz_ushort, __convert_short_rtz_uint, __convert_short_rtz_ulong, __convert_short_char, __convert_short_rtz_short, __convert_short_rtz_int, __convert_short_rtz_long, __convert_short_rtz_float, __convert_short_rtz_double )
    #define convert_short2_rtz(_X)    __builtin_overload( 1, _X, __convert_short2_rtz_uchar2, __convert_short2_rtz_ushort2, __convert_short2_rtz_uint2, __convert_short2_rtz_ulong2, __convert_short2_rtz_char2, __convert_short2_rtz_short2, __convert_short2_rtz_int2, __convert_short2_rtz_long2, __convert_short2_rtz_float2, __convert_short2_rtz_double2 )
    #define convert_short4_rtz(_X)    __builtin_overload( 1, _X, __convert_short4_rtz_uchar4, __convert_short4_rtz_ushort4, __convert_short4_rtz_uint4, __convert_short4_rtz_ulong4, __convert_short4_rtz_char4, __convert_short4_rtz_short4, __convert_short4_rtz_int4, __convert_short4_rtz_long4, __convert_short4_rtz_float4, __convert_short4_rtz_double4 )
    #define convert_short8_rtz(_X)    __builtin_overload( 1, _X, __convert_short8_rtz_uchar8, __convert_short8_rtz_ushort8, __convert_short8_rtz_uint8, __convert_short8_rtz_ulong8, __convert_short8_char8, __convert_short8_rtz_short8, __convert_short8_rtz_int8, __convert_short8_rtz_long8, __convert_short8_rtz_float8, __convert_short8_rtz_double8 )
    #define convert_short16_rtz(_X)    __builtin_overload( 1, _X, __convert_short16_rtz_uchar16, __convert_short16_rtz_ushort16, __convert_short16_rtz_uint16, __convert_short16_rtz_ulong16, __convert_short16_char16, __convert_short16_rtz_short16, __convert_short16_rtz_int16, __convert_short16_rtz_long16, __convert_short16_rtz_float16, __convert_short16_rtz_double16 )
    #define convert_short_sat(_X)    __builtin_overload( 1, _X, __convert_short_sat_uchar, __convert_short_sat_ushort, __convert_short_sat_uint, __convert_short_sat_ulong, __convert_short_char, __convert_short_sat_short, __convert_short_sat_int, __convert_short_sat_long, __convert_short_sat_float, __convert_short_sat_double )
    #define convert_short2_sat(_X)    __builtin_overload( 1, _X, __convert_short2_sat_uchar2, __convert_short2_sat_ushort2, __convert_short2_sat_uint2, __convert_short2_sat_ulong2, __convert_short2_sat_char2, __convert_short2_sat_short2, __convert_short2_sat_int2, __convert_short2_sat_long2, __convert_short2_sat_float2, __convert_short2_sat_double2 )
    #define convert_short4_sat(_X)    __builtin_overload( 1, _X, __convert_short4_sat_uchar4, __convert_short4_sat_ushort4, __convert_short4_sat_uint4, __convert_short4_sat_ulong4, __convert_short4_sat_char4, __convert_short4_sat_short4, __convert_short4_sat_int4, __convert_short4_sat_long4, __convert_short4_sat_float4, __convert_short4_sat_double4 )
    #define convert_short8_sat(_X)    __builtin_overload( 1, _X, __convert_short8_sat_uchar8, __convert_short8_sat_ushort8, __convert_short8_sat_uint8, __convert_short8_sat_ulong8, __convert_short8_char8, __convert_short8_sat_short8, __convert_short8_sat_int8, __convert_short8_sat_long8, __convert_short8_sat_float8, __convert_short8_sat_double8 )
    #define convert_short16_sat(_X)    __builtin_overload( 1, _X, __convert_short16_sat_uchar16, __convert_short16_sat_ushort16, __convert_short16_sat_uint16, __convert_short16_sat_ulong16, __convert_short16_char16, __convert_short16_sat_short16, __convert_short16_sat_int16, __convert_short16_sat_long16, __convert_short16_sat_float16, __convert_short16_sat_double16 )
    #define convert_short_sat_rte(_X)    __builtin_overload( 1, _X, __convert_short_sat_rte_uchar, __convert_short_sat_ushort, __convert_short_sat_uint, __convert_short_sat_rte_ulong, __convert_short_char, __convert_short_sat_rte_short, __convert_short_sat_int, __convert_short_sat_rte_long, __convert_short_sat_rte_float, __convert_short_sat_rte_double )
    #define convert_short2_sat_rte(_X)    __builtin_overload( 1, _X, __convert_short2_sat_rte_uchar2, __convert_short2_sat_rte_ushort2, __convert_short2_sat_rte_uint2, __convert_short2_sat_rte_ulong2, __convert_short2_sat_rte_char2, __convert_short2_sat_rte_short2, __convert_short2_sat_rte_int2, __convert_short2_sat_rte_long2, __convert_short2_sat_rte_float2, __convert_short2_sat_rte_double2 )
    #define convert_short4_sat_rte(_X)    __builtin_overload( 1, _X, __convert_short4_sat_rte_uchar4, __convert_short4_sat_rte_ushort4, __convert_short4_sat_uint4, __convert_short4_sat_rte_ulong4, __convert_short4_sat_rte_char4, __convert_short4_sat_rte_short4, __convert_short4_sat_int4, __convert_short4_sat_rte_long4, __convert_short4_sat_rte_float4, __convert_short4_sat_rte_double4 )
    #define convert_short8_sat_rte(_X)    __builtin_overload( 1, _X, __convert_short8_sat_rte_uchar8, __convert_short8_sat_ushort8, __convert_short8_sat_uint8, __convert_short8_sat_rte_ulong8, __convert_short8_char8, __convert_short8_sat_rte_short8, __convert_short8_sat_int8, __convert_short8_sat_rte_long8, __convert_short8_sat_rte_float8, __convert_short8_sat_rte_double8 )
    #define convert_short16_sat_rte(_X)    __builtin_overload( 1, _X, __convert_short16_sat_rte_uchar16, __convert_short16_sat_rte_ushort16, __convert_short16_sat_rte_uint16, __convert_short16_sat_rte_ulong16, __convert_short16_char16, __convert_short16_sat_rte_short16, __convert_short16_sat_rte_int16, __convert_short16_sat_rte_long16, __convert_short16_sat_rte_float16, __convert_short16_sat_rte_double16 )
    #define convert_short_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_short_sat_rtp_uchar, __convert_short_sat_ushort, __convert_short_sat_uint, __convert_short_sat_rtp_ulong, __convert_short_char, __convert_short_sat_rtp_short, __convert_short_sat_int, __convert_short_sat_rtp_long, __convert_short_sat_rtp_float, __convert_short_sat_rtp_double )
    #define convert_short2_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_short2_sat_rtp_uchar2, __convert_short2_sat_rtp_ushort2, __convert_short2_sat_rtp_uint2, __convert_short2_sat_rtp_ulong2, __convert_short2_sat_rtp_char2, __convert_short2_sat_rtp_short2, __convert_short2_sat_rtp_int2, __convert_short2_sat_rtp_long2, __convert_short2_sat_rtp_float2, __convert_short2_sat_rtp_double2 )
    #define convert_short4_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_short4_sat_rtp_uchar4, __convert_short4_sat_rtp_ushort4, __convert_short4_sat_uint4, __convert_short4_sat_rtp_ulong4, __convert_short4_sat_rtp_char4, __convert_short4_sat_rtp_short4, __convert_short4_sat_int4, __convert_short4_sat_rtp_long4, __convert_short4_sat_rtp_float4, __convert_short4_sat_rtp_double4 )
    #define convert_short8_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_short8_sat_rtp_uchar8, __convert_short8_sat_ushort8, __convert_short8_sat_uint8, __convert_short8_sat_rtp_ulong8, __convert_short8_char8, __convert_short8_sat_rtp_short8, __convert_short8_sat_int8, __convert_short8_sat_rtp_long8, __convert_short8_sat_rtp_float8, __convert_short8_sat_rtp_double8 )
    #define convert_short16_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_short16_sat_rtp_uchar16, __convert_short16_sat_rtp_ushort16, __convert_short16_sat_rtp_uint16, __convert_short16_sat_rtp_ulong16, __convert_short16_char16, __convert_short16_sat_rtp_short16, __convert_short16_sat_rtp_int16, __convert_short16_sat_rtp_long16, __convert_short16_sat_rtp_float16, __convert_short16_sat_rtp_double16 )
    #define convert_short_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_short_sat_rtn_uchar, __convert_short_sat_ushort, __convert_short_sat_uint, __convert_short_sat_rtn_ulong, __convert_short_char, __convert_short_sat_rtn_short, __convert_short_sat_int, __convert_short_sat_rtn_long, __convert_short_sat_rtn_float, __convert_short_sat_rtn_double )
    #define convert_short2_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_short2_sat_rtn_uchar2, __convert_short2_sat_rtn_ushort2, __convert_short2_sat_rtn_uint2, __convert_short2_sat_rtn_ulong2, __convert_short2_sat_rtn_char2, __convert_short2_sat_rtn_short2, __convert_short2_sat_rtn_int2, __convert_short2_sat_rtn_long2, __convert_short2_sat_rtn_float2, __convert_short2_sat_rtn_double2 )
    #define convert_short4_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_short4_sat_rtn_uchar4, __convert_short4_sat_rtn_ushort4, __convert_short4_sat_uint4, __convert_short4_sat_rtn_ulong4, __convert_short4_sat_rtn_char4, __convert_short4_sat_rtn_short4, __convert_short4_sat_int4, __convert_short4_sat_rtn_long4, __convert_short4_sat_rtn_float4, __convert_short4_sat_rtn_double4 )
    #define convert_short8_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_short8_sat_rtn_uchar8, __convert_short8_sat_ushort8, __convert_short8_sat_uint8, __convert_short8_sat_rtn_ulong8, __convert_short8_char8, __convert_short8_sat_rtn_short8, __convert_short8_sat_int8, __convert_short8_sat_rtn_long8, __convert_short8_sat_rtn_float8, __convert_short8_sat_rtn_double8 )
    #define convert_short16_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_short16_sat_rtn_uchar16, __convert_short16_sat_rtn_ushort16, __convert_short16_sat_rtn_uint16, __convert_short16_sat_rtn_ulong16, __convert_short16_char16, __convert_short16_sat_rtn_short16, __convert_short16_sat_rtn_int16, __convert_short16_sat_rtn_long16, __convert_short16_sat_rtn_float16, __convert_short16_sat_rtn_double16 )
    #define convert_short_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_short_sat_rtz_uchar, __convert_short_sat_ushort, __convert_short_sat_uint, __convert_short_sat_rtz_ulong, __convert_short_char, __convert_short_sat_rtz_short, __convert_short_sat_int, __convert_short_sat_rtz_long, __convert_short_sat_rtz_float, __convert_short_sat_rtz_double )
    #define convert_short2_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_short2_sat_rtz_uchar2, __convert_short2_sat_rtz_ushort2, __convert_short2_sat_rtz_uint2, __convert_short2_sat_rtz_ulong2, __convert_short2_sat_rtz_char2, __convert_short2_sat_rtz_short2, __convert_short2_sat_rtz_int2, __convert_short2_sat_rtz_long2, __convert_short2_sat_rtz_float2, __convert_short2_sat_rtz_double2 )
    #define convert_short4_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_short4_sat_rtz_uchar4, __convert_short4_sat_rtz_ushort4, __convert_short4_sat_uint4, __convert_short4_sat_rtz_ulong4, __convert_short4_sat_rtz_char4, __convert_short4_sat_rtz_short4, __convert_short4_sat_int4, __convert_short4_sat_rtz_long4, __convert_short4_sat_rtz_float4, __convert_short4_sat_rtz_double4 )
    #define convert_short8_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_short8_sat_rtz_uchar8, __convert_short8_sat_ushort8, __convert_short8_sat_uint8, __convert_short8_sat_rtz_ulong8, __convert_short8_char8, __convert_short8_sat_rtz_short8, __convert_short8_sat_int8, __convert_short8_sat_rtz_long8, __convert_short8_sat_rtz_float8, __convert_short8_sat_rtz_double8 )
    #define convert_short16_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_short16_sat_rtz_uchar16, __convert_short16_sat_rtz_ushort16, __convert_short16_sat_rtz_uint16, __convert_short16_sat_rtz_ulong16, __convert_short16_char16, __convert_short16_sat_rtz_short16, __convert_short16_sat_rtz_int16, __convert_short16_sat_rtz_long16, __convert_short16_sat_rtz_float16, __convert_short16_sat_rtz_double16 )
    #define convert_int(_X)    __builtin_overload( 1, _X, __convert_int_uchar, __convert_int_ushort, __convert_int_uint, __convert_int_ulong, __convert_int_char, __convert_int_short, __convert_int_int, __convert_int_long, __convert_int_rtz_float, __convert_int_rtz_double )
    #define convert_int2(_X)    __builtin_overload( 1, _X, __convert_int2_uchar2, __convert_int2_ushort2, __convert_int2_uint2, __convert_int2_ulong2, __convert_int2_char2, __convert_int2_short2, __convert_int2_int2, __convert_int2_long2, __convert_int2_float2, __convert_int2_rtz_double2 )
    #define convert_int4(_X)    __builtin_overload( 1, _X, __convert_int4_uchar4, __convert_int4_ushort4, __convert_int4_uint4, __convert_int4_ulong4, __convert_int4_char4, __convert_int4_short4, __convert_int4_int4, __convert_int4_long4, __convert_int4_rtz_float4, __convert_int4_rtz_double4 )
    #define convert_int8(_X)    __builtin_overload( 1, _X, __convert_int8_uchar8, __convert_int8_ushort8, __convert_int8_uint8, __convert_int8_ulong8, __convert_int8_char8, __convert_int8_short8, __convert_int8_int8, __convert_int8_long8, __convert_int8_float8, __convert_int8_double8 )
    #define convert_int16(_X)    __builtin_overload( 1, _X, __convert_int16_uchar16, __convert_int16_ushort16, __convert_int16_uint16, __convert_int16_ulong16, __convert_int16_char16, __convert_int16_short16, __convert_int16_int16, __convert_int16_long16, __convert_int16_float16, __convert_int16_double16 )
    #define convert_int_rte(_X)    __builtin_overload( 1, _X, __convert_int_rte_uchar, __convert_int_rte_ushort, __convert_int_rte_uint, __convert_int_rte_ulong, __convert_int_char, __convert_int_short, __convert_int_rte_int, __convert_int_rte_long, __convert_int_rte_float, __convert_int_rte_double )
    #define convert_int2_rte(_X)    __builtin_overload( 1, _X, __convert_int2_rte_uchar2, __convert_int2_rte_ushort2, __convert_int2_rte_uint2, __convert_int2_rte_ulong2, __convert_int2_rte_char2, __convert_int2_rte_short2, __convert_int2_rte_int2, __convert_int2_rte_long2, __convert_int2_rte_float2, __convert_int2_rte_double2 )
    #define convert_int4_rte(_X)    __builtin_overload( 1, _X, __convert_int4_rte_uchar4, __convert_int4_rte_ushort4, __convert_int4_rte_uint4, __convert_int4_rte_ulong4, __convert_int4_char4, __convert_int4_short4, __convert_int4_rte_int4, __convert_int4_rte_long4, __convert_int4_rte_float4, __convert_int4_rte_double4 )
    #define convert_int8_rte(_X)    __builtin_overload( 1, _X, __convert_int8_rte_uchar8, __convert_int8_rte_ushort8, __convert_int8_rte_uint8, __convert_int8_rte_ulong8, __convert_int8_char8, __convert_int8_short8, __convert_int8_rte_int8, __convert_int8_rte_long8, __convert_int8_rte_float8, __convert_int8_rte_double8 )
    #define convert_int16_rte(_X)    __builtin_overload( 1, _X, __convert_int16_rte_uchar16, __convert_int16_rte_ushort16, __convert_int16_rte_uint16, __convert_int16_rte_ulong16, __convert_int16_char16, __convert_int16_rte_short16, __convert_int16_rte_int16, __convert_int16_rte_long16, __convert_int16_rte_float16, __convert_int16_rte_double16 )
    #define convert_int_rtp(_X)    __builtin_overload( 1, _X, __convert_int_rtp_uchar, __convert_int_rtp_ushort, __convert_int_rtp_uint, __convert_int_rtp_ulong, __convert_int_char, __convert_int_short, __convert_int_rtp_int, __convert_int_rtp_long, __convert_int_rtp_float, __convert_int_rtp_double )
    #define convert_int2_rtp(_X)    __builtin_overload( 1, _X, __convert_int2_rtp_uchar2, __convert_int2_rtp_ushort2, __convert_int2_rtp_uint2, __convert_int2_rtp_ulong2, __convert_int2_rtp_char2, __convert_int2_rtp_short2, __convert_int2_rtp_int2, __convert_int2_rtp_long2, __convert_int2_rtp_float2, __convert_int2_rtp_double2 )
    #define convert_int4_rtp(_X)    __builtin_overload( 1, _X, __convert_int4_rtp_uchar4, __convert_int4_rtp_ushort4, __convert_int4_rtp_uint4, __convert_int4_rtp_ulong4, __convert_int4_char4, __convert_int4_short4, __convert_int4_rtp_int4, __convert_int4_rtp_long4, __convert_int4_rtp_float4, __convert_int4_rtp_double4 )
    #define convert_int8_rtp(_X)    __builtin_overload( 1, _X, __convert_int8_rtp_uchar8, __convert_int8_rtp_ushort8, __convert_int8_rtp_uint8, __convert_int8_rtp_ulong8, __convert_int8_char8, __convert_int8_short8, __convert_int8_rtp_int8, __convert_int8_rtp_long8, __convert_int8_rtp_float8, __convert_int8_rtp_double8 )
    #define convert_int16_rtp(_X)    __builtin_overload( 1, _X, __convert_int16_rtp_uchar16, __convert_int16_rtp_ushort16, __convert_int16_rtp_uint16, __convert_int16_rtp_ulong16, __convert_int16_char16, __convert_int16_rtp_short16, __convert_int16_rtp_int16, __convert_int16_rtp_long16, __convert_int16_rtp_float16, __convert_int16_rtp_double16 )
    #define convert_int_rtn(_X)    __builtin_overload( 1, _X, __convert_int_rtn_uchar, __convert_int_rtn_ushort, __convert_int_rtn_uint, __convert_int_rtn_ulong, __convert_int_char, __convert_int_short, __convert_int_rtn_int, __convert_int_rtn_long, __convert_int_rtn_float, __convert_int_rtn_double )
    #define convert_int2_rtn(_X)    __builtin_overload( 1, _X, __convert_int2_rtn_uchar2, __convert_int2_rtn_ushort2, __convert_int2_rtn_uint2, __convert_int2_rtn_ulong2, __convert_int2_rtn_char2, __convert_int2_rtn_short2, __convert_int2_rtn_int2, __convert_int2_rtn_long2, __convert_int2_rtn_float2, __convert_int2_rtn_double2 )
    #define convert_int4_rtn(_X)    __builtin_overload( 1, _X, __convert_int4_rtn_uchar4, __convert_int4_rtn_ushort4, __convert_int4_rtn_uint4, __convert_int4_rtn_ulong4, __convert_int4_char4, __convert_int4_short4, __convert_int4_rtn_int4, __convert_int4_rtn_long4, __convert_int4_rtn_float4, __convert_int4_rtn_double4 )
    #define convert_int8_rtn(_X)    __builtin_overload( 1, _X, __convert_int8_rtn_uchar8, __convert_int8_rtn_ushort8, __convert_int8_rtn_uint8, __convert_int8_rtn_ulong8, __convert_int8_char8, __convert_int8_short8, __convert_int8_rtn_int8, __convert_int8_rtn_long8, __convert_int8_rtn_float8, __convert_int8_rtn_double8 )
    #define convert_int16_rtn(_X)    __builtin_overload( 1, _X, __convert_int16_rtn_uchar16, __convert_int16_rtn_ushort16, __convert_int16_rtn_uint16, __convert_int16_rtn_ulong16, __convert_int16_char16, __convert_int16_rtn_short16, __convert_int16_rtn_int16, __convert_int16_rtn_long16, __convert_int16_rtn_float16, __convert_int16_rtn_double16 )
    #define convert_int_rtz(_X)    __builtin_overload( 1, _X, __convert_int_rtz_uchar, __convert_int_rtz_ushort, __convert_int_rtz_uint, __convert_int_rtz_ulong, __convert_int_char, __convert_int_short, __convert_int_rtz_int, __convert_int_rtz_long, __convert_int_rtz_float, __convert_int_rtz_double )
    #define convert_int2_rtz(_X)    __builtin_overload( 1, _X, __convert_int2_rtz_uchar2, __convert_int2_rtz_ushort2, __convert_int2_rtz_uint2, __convert_int2_rtz_ulong2, __convert_int2_rtz_char2, __convert_int2_rtz_short2, __convert_int2_rtz_int2, __convert_int2_rtz_long2, __convert_int2_rtz_float2, __convert_int2_rtz_double2 )
    #define convert_int4_rtz(_X)    __builtin_overload( 1, _X, __convert_int4_rtz_uchar4, __convert_int4_rtz_ushort4, __convert_int4_rtz_uint4, __convert_int4_rtz_ulong4, __convert_int4_char4, __convert_int4_short4, __convert_int4_rtz_int4, __convert_int4_rtz_long4, __convert_int4_rtz_float4, __convert_int4_rtz_double4 )
    #define convert_int8_rtz(_X)    __builtin_overload( 1, _X, __convert_int8_rtz_uchar8, __convert_int8_rtz_ushort8, __convert_int8_rtz_uint8, __convert_int8_rtz_ulong8, __convert_int8_char8, __convert_int8_short8, __convert_int8_rtz_int8, __convert_int8_rtz_long8, __convert_int8_rtz_float8, __convert_int8_rtz_double8 )
    #define convert_int16_rtz(_X)    __builtin_overload( 1, _X, __convert_int16_rtz_uchar16, __convert_int16_rtz_ushort16, __convert_int16_rtz_uint16, __convert_int16_rtz_ulong16, __convert_int16_char16, __convert_int16_rtz_short16, __convert_int16_rtz_int16, __convert_int16_rtz_long16, __convert_int16_rtz_float16, __convert_int16_rtz_double16 )
    #define convert_int_sat(_X)    __builtin_overload( 1, _X, __convert_int_sat_uchar, __convert_int_sat_ushort, __convert_int_sat_uint, __convert_int_sat_ulong, __convert_int_char, __convert_int_short, __convert_int_sat_int, __convert_int_sat_long, __convert_int_sat_rtz_float, __convert_int_sat_rtz_double )
    #define convert_int2_sat(_X)    __builtin_overload( 1, _X, __convert_int2_sat_uchar2, __convert_int2_sat_ushort2, __convert_int2_sat_uint2, __convert_int2_sat_ulong2, __convert_int2_sat_char2, __convert_int2_sat_short2, __convert_int2_sat_int2, __convert_int2_sat_long2, __convert_int2_sat_float2, __convert_int2_sat_rtz_double2 )
    #define convert_int4_sat(_X)    __builtin_overload( 1, _X, __convert_int4_sat_uchar4, __convert_int4_sat_ushort4, __convert_int4_sat_uint4, __convert_int4_sat_ulong4, __convert_int4_char4, __convert_int4_short4, __convert_int4_sat_int4, __convert_int4_sat_long4, __convert_int4_sat_rtz_float4, __convert_int4_sat_rtz_double4 )
    #define convert_int8_sat(_X)    __builtin_overload( 1, _X, __convert_int8_sat_uchar8, __convert_int8_sat_ushort8, __convert_int8_sat_uint8, __convert_int8_sat_ulong8, __convert_int8_char8, __convert_int8_short8, __convert_int8_sat_int8, __convert_int8_sat_long8, __convert_int8_sat_float8, __convert_int8_sat_double8 )
    #define convert_int16_sat(_X)    __builtin_overload( 1, _X, __convert_int16_sat_uchar16, __convert_int16_sat_ushort16, __convert_int16_sat_uint16, __convert_int16_sat_ulong16, __convert_int16_char16, __convert_int16_sat_short16, __convert_int16_sat_int16, __convert_int16_sat_long16, __convert_int16_sat_float16, __convert_int16_sat_double16 )
    #define convert_int_sat_rte(_X)    __builtin_overload( 1, _X, __convert_int_sat_rte_uchar, __convert_int_sat_rte_ushort, __convert_int_sat_uint, __convert_int_sat_ulong, __convert_int_char, __convert_int_short, __convert_int_sat_rte_int, __convert_int_sat_long, __convert_int_sat_rte_float, __convert_int_sat_rte_double )
    #define convert_int2_sat_rte(_X)    __builtin_overload( 1, _X, __convert_int2_sat_rte_uchar2, __convert_int2_sat_rte_ushort2, __convert_int2_sat_rte_uint2, __convert_int2_sat_ulong2, __convert_int2_sat_rte_char2, __convert_int2_sat_rte_short2, __convert_int2_sat_rte_int2, __convert_int2_sat_long2, __convert_int2_sat_rte_float2, __convert_int2_sat_rte_double2 )
    #define convert_int4_sat_rte(_X)    __builtin_overload( 1, _X, __convert_int4_sat_rte_uchar4, __convert_int4_sat_rte_ushort4, __convert_int4_sat_uint4, __convert_int4_sat_ulong4, __convert_int4_char4, __convert_int4_short4, __convert_int4_sat_rte_int4, __convert_int4_sat_long4, __convert_int4_sat_rte_float4, __convert_int4_sat_rte_double4 )
    #define convert_int8_sat_rte(_X)    __builtin_overload( 1, _X, __convert_int8_sat_rte_uchar8, __convert_int8_sat_rte_ushort8, __convert_int8_sat_rte_uint8, __convert_int8_sat_rte_ulong8, __convert_int8_char8, __convert_int8_short8, __convert_int8_sat_rte_int8, __convert_int8_sat_rte_long8, __convert_int8_sat_rte_float8, __convert_int8_sat_rte_double8 )
    #define convert_int16_sat_rte(_X)    __builtin_overload( 1, _X, __convert_int16_sat_rte_uchar16, __convert_int16_sat_rte_ushort16, __convert_int16_sat_rte_uint16, __convert_int16_sat_rte_ulong16, __convert_int16_char16, __convert_int16_sat_rte_short16, __convert_int16_sat_rte_int16, __convert_int16_sat_rte_long16, __convert_int16_sat_rte_float16, __convert_int16_sat_rte_double16 )
    #define convert_int_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_int_sat_rtp_uchar, __convert_int_sat_rtp_ushort, __convert_int_sat_uint, __convert_int_sat_ulong, __convert_int_char, __convert_int_short, __convert_int_sat_rtp_int, __convert_int_sat_long, __convert_int_sat_rtp_float, __convert_int_sat_rtp_double )
    #define convert_int2_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_int2_sat_rtp_uchar2, __convert_int2_sat_rtp_ushort2, __convert_int2_sat_rtp_uint2, __convert_int2_sat_ulong2, __convert_int2_sat_rtp_char2, __convert_int2_sat_rtp_short2, __convert_int2_sat_rtp_int2, __convert_int2_sat_long2, __convert_int2_sat_rtp_float2, __convert_int2_sat_rtp_double2 )
    #define convert_int4_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_int4_sat_rtp_uchar4, __convert_int4_sat_rtp_ushort4, __convert_int4_sat_uint4, __convert_int4_sat_ulong4, __convert_int4_char4, __convert_int4_short4, __convert_int4_sat_rtp_int4, __convert_int4_sat_long4, __convert_int4_sat_rtp_float4, __convert_int4_sat_rtp_double4 )
    #define convert_int8_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_int8_sat_rtp_uchar8, __convert_int8_sat_rtp_ushort8, __convert_int8_sat_rtp_uint8, __convert_int8_sat_rtp_ulong8, __convert_int8_char8, __convert_int8_short8, __convert_int8_sat_rtp_int8, __convert_int8_sat_rtp_long8, __convert_int8_sat_rtp_float8, __convert_int8_sat_rtp_double8 )
    #define convert_int16_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_int16_sat_rtp_uchar16, __convert_int16_sat_rtp_ushort16, __convert_int16_sat_rtp_uint16, __convert_int16_sat_rtp_ulong16, __convert_int16_char16, __convert_int16_sat_rtp_short16, __convert_int16_sat_rtp_int16, __convert_int16_sat_rtp_long16, __convert_int16_sat_rtp_float16, __convert_int16_sat_rtp_double16 )
    #define convert_int_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_int_sat_rtn_uchar, __convert_int_sat_rtn_ushort, __convert_int_sat_uint, __convert_int_sat_ulong, __convert_int_char, __convert_int_short, __convert_int_sat_rtn_int, __convert_int_sat_long, __convert_int_sat_rtn_float, __convert_int_sat_rtn_double )
    #define convert_int2_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_int2_sat_rtn_uchar2, __convert_int2_sat_rtn_ushort2, __convert_int2_sat_rtn_uint2, __convert_int2_sat_ulong2, __convert_int2_sat_rtn_char2, __convert_int2_sat_rtn_short2, __convert_int2_sat_rtn_int2, __convert_int2_sat_long2, __convert_int2_sat_rtn_float2, __convert_int2_sat_rtn_double2 )
    #define convert_int4_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_int4_sat_rtn_uchar4, __convert_int4_sat_rtn_ushort4, __convert_int4_sat_uint4, __convert_int4_sat_ulong4, __convert_int4_char4, __convert_int4_short4, __convert_int4_sat_rtn_int4, __convert_int4_sat_long4, __convert_int4_sat_rtn_float4, __convert_int4_sat_rtn_double4 )
    #define convert_int8_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_int8_sat_rtn_uchar8, __convert_int8_sat_rtn_ushort8, __convert_int8_sat_rtn_uint8, __convert_int8_sat_rtn_ulong8, __convert_int8_char8, __convert_int8_short8, __convert_int8_sat_rtn_int8, __convert_int8_sat_rtn_long8, __convert_int8_sat_rtn_float8, __convert_int8_sat_rtn_double8 )
    #define convert_int16_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_int16_sat_rtn_uchar16, __convert_int16_sat_rtn_ushort16, __convert_int16_sat_rtn_uint16, __convert_int16_sat_rtn_ulong16, __convert_int16_char16, __convert_int16_sat_rtn_short16, __convert_int16_sat_rtn_int16, __convert_int16_sat_rtn_long16, __convert_int16_sat_rtn_float16, __convert_int16_sat_rtn_double16 )
    #define convert_int_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_int_sat_rtz_uchar, __convert_int_sat_rtz_ushort, __convert_int_sat_uint, __convert_int_sat_ulong, __convert_int_char, __convert_int_short, __convert_int_sat_rtz_int, __convert_int_sat_long, __convert_int_sat_rtz_float, __convert_int_sat_rtz_double )
    #define convert_int2_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_int2_sat_rtz_uchar2, __convert_int2_sat_rtz_ushort2, __convert_int2_sat_rtz_uint2, __convert_int2_sat_ulong2, __convert_int2_sat_rtz_char2, __convert_int2_sat_rtz_short2, __convert_int2_sat_rtz_int2, __convert_int2_sat_long2, __convert_int2_sat_rtz_float2, __convert_int2_sat_rtz_double2 )
    #define convert_int4_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_int4_sat_rtz_uchar4, __convert_int4_sat_rtz_ushort4, __convert_int4_sat_uint4, __convert_int4_sat_ulong4, __convert_int4_char4, __convert_int4_short4, __convert_int4_sat_rtz_int4, __convert_int4_sat_long4, __convert_int4_sat_rtz_float4, __convert_int4_sat_rtz_double4 )
    #define convert_int8_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_int8_sat_rtz_uchar8, __convert_int8_sat_rtz_ushort8, __convert_int8_sat_rtz_uint8, __convert_int8_sat_rtz_ulong8, __convert_int8_char8, __convert_int8_short8, __convert_int8_sat_rtz_int8, __convert_int8_sat_rtz_long8, __convert_int8_sat_rtz_float8, __convert_int8_sat_rtz_double8 )
    #define convert_int16_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_int16_sat_rtz_uchar16, __convert_int16_sat_rtz_ushort16, __convert_int16_sat_rtz_uint16, __convert_int16_sat_rtz_ulong16, __convert_int16_char16, __convert_int16_sat_rtz_short16, __convert_int16_sat_rtz_int16, __convert_int16_sat_rtz_long16, __convert_int16_sat_rtz_float16, __convert_int16_sat_rtz_double16 )
    #define convert_long(_X)    __builtin_overload( 1, _X, __convert_long_uchar, __convert_long_ushort, __convert_long_uint, __convert_long_ulong, __convert_long_char, __convert_long_short, __convert_long_int, __convert_long_long, __convert_long_rtz_float, __convert_long_rtz_double )
    #define convert_long2(_X)    __builtin_overload( 1, _X, __convert_long2_uchar2, __convert_long2_ushort2, __convert_long2_uint2, __convert_long2_ulong2, __convert_long2_char2, __convert_long2_short2, __convert_long2_int2, __convert_long2_long2, __convert_long2_rtz_float2, __convert_long2_rtz_double2 )
    #define convert_long4(_X)    __builtin_overload( 1, _X, __convert_long4_uchar4, __convert_long4_ushort4, __convert_long4_uint4, __convert_long4_ulong4, __convert_long4_char4, __convert_long4_short4, __convert_long4_int4, __convert_long4_long4, __convert_long4_rtz_float4, __convert_long4_double4 )
    #define convert_long8(_X)    __builtin_overload( 1, _X, __convert_long8_uchar8, __convert_long8_ushort8, __convert_long8_uint8, __convert_long8_ulong8, __convert_long8_char8, __convert_long8_short8, __convert_long8_int8, __convert_long8_long8, __convert_long8_float8, __convert_long8_double8 )
    #define convert_long16(_X)    __builtin_overload( 1, _X, __convert_long16_uchar16, __convert_long16_ushort16, __convert_long16_uint16, __convert_long16_ulong16, __convert_long16_char16, __convert_long16_short16, __convert_long16_int16, __convert_long16_long16, __convert_long16_float16, __convert_long16_double16 )
    #define convert_long_rte(_X)    __builtin_overload( 1, _X, __convert_long_rte_uchar, __convert_long_rte_ushort, __convert_long_rte_uint, __convert_long_rte_ulong, __convert_long_char, __convert_long_short, __convert_long_int, __convert_long_rte_long, __convert_long_rte_float, __convert_long_rte_double )
    #define convert_long2_rte(_X)    __builtin_overload( 1, _X, __convert_long2_rte_uchar2, __convert_long2_rte_ushort2, __convert_long2_rte_uint2, __convert_long2_rte_ulong2, __convert_long2_char2, __convert_long2_short2, __convert_long2_int2, __convert_long2_rte_long2, __convert_long2_rte_float2, __convert_long2_rte_double2 )
    #define convert_long4_rte(_X)    __builtin_overload( 1, _X, __convert_long4_rte_uchar4, __convert_long4_rte_ushort4, __convert_long4_rte_uint4, __convert_long4_rte_ulong4, __convert_long4_char4, __convert_long4_short4, __convert_long4_int4, __convert_long4_rte_long4, __convert_long4_rte_float4, __convert_long4_rte_double4 )
    #define convert_long8_rte(_X)    __builtin_overload( 1, _X, __convert_long8_rte_uchar8, __convert_long8_rte_ushort8, __convert_long8_rte_uint8, __convert_long8_rte_ulong8, __convert_long8_char8, __convert_long8_short8, __convert_long8_rte_int8, __convert_long8_rte_long8, __convert_long8_rte_float8, __convert_long8_rte_double8 )
    #define convert_long16_rte(_X)    __builtin_overload( 1, _X, __convert_long16_rte_uchar16, __convert_long16_rte_ushort16, __convert_long16_rte_uint16, __convert_long16_rte_ulong16, __convert_long16_char16, __convert_long16_rte_short16, __convert_long16_rte_int16, __convert_long16_rte_long16, __convert_long16_rte_float16, __convert_long16_rte_double16 )
    #define convert_long_rtp(_X)    __builtin_overload( 1, _X, __convert_long_rtp_uchar, __convert_long_rtp_ushort, __convert_long_rtp_uint, __convert_long_rtp_ulong, __convert_long_char, __convert_long_short, __convert_long_int, __convert_long_rtp_long, __convert_long_rtp_float, __convert_long_rtp_double )
    #define convert_long2_rtp(_X)    __builtin_overload( 1, _X, __convert_long2_rtp_uchar2, __convert_long2_rtp_ushort2, __convert_long2_rtp_uint2, __convert_long2_rtp_ulong2, __convert_long2_char2, __convert_long2_short2, __convert_long2_int2, __convert_long2_rtp_long2, __convert_long2_rtp_float2, __convert_long2_rtp_double2 )
    #define convert_long4_rtp(_X)    __builtin_overload( 1, _X, __convert_long4_rtp_uchar4, __convert_long4_rtp_ushort4, __convert_long4_rtp_uint4, __convert_long4_rtp_ulong4, __convert_long4_char4, __convert_long4_short4, __convert_long4_int4, __convert_long4_rtp_long4, __convert_long4_rtp_float4, __convert_long4_rtp_double4 )
    #define convert_long8_rtp(_X)    __builtin_overload( 1, _X, __convert_long8_rtp_uchar8, __convert_long8_rtp_ushort8, __convert_long8_rtp_uint8, __convert_long8_rtp_ulong8, __convert_long8_char8, __convert_long8_short8, __convert_long8_rtp_int8, __convert_long8_rtp_long8, __convert_long8_rtp_float8, __convert_long8_rtp_double8 )
    #define convert_long16_rtp(_X)    __builtin_overload( 1, _X, __convert_long16_rtp_uchar16, __convert_long16_rtp_ushort16, __convert_long16_rtp_uint16, __convert_long16_rtp_ulong16, __convert_long16_char16, __convert_long16_rtp_short16, __convert_long16_rtp_int16, __convert_long16_rtp_long16, __convert_long16_rtp_float16, __convert_long16_rtp_double16 )
    #define convert_long_rtn(_X)    __builtin_overload( 1, _X, __convert_long_rtn_uchar, __convert_long_rtn_ushort, __convert_long_rtn_uint, __convert_long_rtn_ulong, __convert_long_char, __convert_long_short, __convert_long_int, __convert_long_rtn_long, __convert_long_rtn_float, __convert_long_rtn_double )
    #define convert_long2_rtn(_X)    __builtin_overload( 1, _X, __convert_long2_rtn_uchar2, __convert_long2_rtn_ushort2, __convert_long2_rtn_uint2, __convert_long2_rtn_ulong2, __convert_long2_char2, __convert_long2_short2, __convert_long2_int2, __convert_long2_rtn_long2, __convert_long2_rtn_float2, __convert_long2_rtn_double2 )
    #define convert_long4_rtn(_X)    __builtin_overload( 1, _X, __convert_long4_rtn_uchar4, __convert_long4_rtn_ushort4, __convert_long4_rtn_uint4, __convert_long4_rtn_ulong4, __convert_long4_char4, __convert_long4_short4, __convert_long4_int4, __convert_long4_rtn_long4, __convert_long4_rtn_float4, __convert_long4_rtn_double4 )
    #define convert_long8_rtn(_X)    __builtin_overload( 1, _X, __convert_long8_rtn_uchar8, __convert_long8_rtn_ushort8, __convert_long8_rtn_uint8, __convert_long8_rtn_ulong8, __convert_long8_char8, __convert_long8_short8, __convert_long8_rtn_int8, __convert_long8_rtn_long8, __convert_long8_rtn_float8, __convert_long8_rtn_double8 )
    #define convert_long16_rtn(_X)    __builtin_overload( 1, _X, __convert_long16_rtn_uchar16, __convert_long16_rtn_ushort16, __convert_long16_rtn_uint16, __convert_long16_rtn_ulong16, __convert_long16_char16, __convert_long16_rtn_short16, __convert_long16_rtn_int16, __convert_long16_rtn_long16, __convert_long16_rtn_float16, __convert_long16_rtn_double16 )
    #define convert_long_rtz(_X)    __builtin_overload( 1, _X, __convert_long_rtz_uchar, __convert_long_rtz_ushort, __convert_long_rtz_uint, __convert_long_rtz_ulong, __convert_long_char, __convert_long_short, __convert_long_int, __convert_long_rtz_long, __convert_long_rtz_float, __convert_long_rtz_double )
    #define convert_long2_rtz(_X)    __builtin_overload( 1, _X, __convert_long2_rtz_uchar2, __convert_long2_rtz_ushort2, __convert_long2_rtz_uint2, __convert_long2_rtz_ulong2, __convert_long2_char2, __convert_long2_short2, __convert_long2_int2, __convert_long2_rtz_long2, __convert_long2_rtz_float2, __convert_long2_rtz_double2 )
    #define convert_long4_rtz(_X)    __builtin_overload( 1, _X, __convert_long4_rtz_uchar4, __convert_long4_rtz_ushort4, __convert_long4_rtz_uint4, __convert_long4_rtz_ulong4, __convert_long4_char4, __convert_long4_short4, __convert_long4_int4, __convert_long4_rtz_long4, __convert_long4_rtz_float4, __convert_long4_rtz_double4 )
    #define convert_long8_rtz(_X)    __builtin_overload( 1, _X, __convert_long8_rtz_uchar8, __convert_long8_rtz_ushort8, __convert_long8_rtz_uint8, __convert_long8_rtz_ulong8, __convert_long8_char8, __convert_long8_short8, __convert_long8_rtz_int8, __convert_long8_rtz_long8, __convert_long8_rtz_float8, __convert_long8_rtz_double8 )
    #define convert_long16_rtz(_X)    __builtin_overload( 1, _X, __convert_long16_rtz_uchar16, __convert_long16_rtz_ushort16, __convert_long16_rtz_uint16, __convert_long16_rtz_ulong16, __convert_long16_char16, __convert_long16_rtz_short16, __convert_long16_rtz_int16, __convert_long16_rtz_long16, __convert_long16_rtz_float16, __convert_long16_rtz_double16 )
    #define convert_long_sat(_X)    __builtin_overload( 1, _X, __convert_long_sat_uchar, __convert_long_sat_ushort, __convert_long_sat_uint, __convert_long_sat_ulong, __convert_long_char, __convert_long_short, __convert_long_int, __convert_long_sat_long, __convert_long_sat_rtz_float, __convert_long_sat_rtz_double )
    #define convert_long2_sat(_X)    __builtin_overload( 1, _X, __convert_long2_sat_uchar2, __convert_long2_sat_ushort2, __convert_long2_sat_uint2, __convert_long2_sat_ulong2, __convert_long2_char2, __convert_long2_short2, __convert_long2_int2, __convert_long2_sat_long2, __convert_long2_sat_rtz_float2, __convert_long2_sat_rtz_double2 )
    #define convert_long4_sat(_X)    __builtin_overload( 1, _X, __convert_long4_sat_uchar4, __convert_long4_sat_ushort4, __convert_long4_sat_uint4, __convert_long4_sat_ulong4, __convert_long4_char4, __convert_long4_short4, __convert_long4_int4, __convert_long4_sat_long4, __convert_long4_sat_rtz_float4, __convert_long4_sat_double4 )
    #define convert_long8_sat(_X)    __builtin_overload( 1, _X, __convert_long8_sat_uchar8, __convert_long8_sat_ushort8, __convert_long8_sat_uint8, __convert_long8_sat_ulong8, __convert_long8_char8, __convert_long8_short8, __convert_long8_sat_int8, __convert_long8_sat_long8, __convert_long8_sat_float8, __convert_long8_sat_double8 )
    #define convert_long16_sat(_X)    __builtin_overload( 1, _X, __convert_long16_sat_uchar16, __convert_long16_sat_ushort16, __convert_long16_sat_uint16, __convert_long16_sat_ulong16, __convert_long16_char16, __convert_long16_sat_short16, __convert_long16_sat_int16, __convert_long16_sat_long16, __convert_long16_sat_float16, __convert_long16_sat_double16 )
    #define convert_long_sat_rte(_X)    __builtin_overload( 1, _X, __convert_long_sat_rte_uchar, __convert_long_sat_rte_ushort, __convert_long_sat_rte_uint, __convert_long_sat_ulong, __convert_long_char, __convert_long_short, __convert_long_int, __convert_long_sat_rte_long, __convert_long_sat_rte_float, __convert_long_sat_rte_double )
    #define convert_long2_sat_rte(_X)    __builtin_overload( 1, _X, __convert_long2_sat_rte_uchar2, __convert_long2_sat_rte_ushort2, __convert_long2_sat_rte_uint2, __convert_long2_sat_ulong2, __convert_long2_char2, __convert_long2_short2, __convert_long2_int2, __convert_long2_sat_rte_long2, __convert_long2_sat_rte_float2, __convert_long2_sat_rte_double2 )
    #define convert_long4_sat_rte(_X)    __builtin_overload( 1, _X, __convert_long4_sat_rte_uchar4, __convert_long4_sat_rte_ushort4, __convert_long4_sat_rte_uint4, __convert_long4_sat_rte_ulong4, __convert_long4_char4, __convert_long4_short4, __convert_long4_int4, __convert_long4_sat_rte_long4, __convert_long4_sat_rte_float4, __convert_long4_sat_rte_double4 )
    #define convert_long8_sat_rte(_X)    __builtin_overload( 1, _X, __convert_long8_sat_rte_uchar8, __convert_long8_sat_rte_ushort8, __convert_long8_sat_rte_uint8, __convert_long8_sat_rte_ulong8, __convert_long8_char8, __convert_long8_short8, __convert_long8_sat_rte_int8, __convert_long8_sat_rte_long8, __convert_long8_sat_rte_float8, __convert_long8_sat_rte_double8 )
    #define convert_long16_sat_rte(_X)    __builtin_overload( 1, _X, __convert_long16_sat_rte_uchar16, __convert_long16_sat_rte_ushort16, __convert_long16_sat_rte_uint16, __convert_long16_sat_rte_ulong16, __convert_long16_char16, __convert_long16_sat_rte_short16, __convert_long16_sat_rte_int16, __convert_long16_sat_rte_long16, __convert_long16_sat_rte_float16, __convert_long16_sat_rte_double16 )
    #define convert_long_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_long_sat_rtp_uchar, __convert_long_sat_rtp_ushort, __convert_long_sat_rtp_uint, __convert_long_sat_ulong, __convert_long_char, __convert_long_short, __convert_long_int, __convert_long_sat_rtp_long, __convert_long_sat_rtp_float, __convert_long_sat_rtp_double )
    #define convert_long2_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_long2_sat_rtp_uchar2, __convert_long2_sat_rtp_ushort2, __convert_long2_sat_rtp_uint2, __convert_long2_sat_ulong2, __convert_long2_char2, __convert_long2_short2, __convert_long2_int2, __convert_long2_sat_rtp_long2, __convert_long2_sat_rtp_float2, __convert_long2_sat_rtp_double2 )
    #define convert_long4_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_long4_sat_rtp_uchar4, __convert_long4_sat_rtp_ushort4, __convert_long4_sat_rtp_uint4, __convert_long4_sat_rtp_ulong4, __convert_long4_char4, __convert_long4_short4, __convert_long4_int4, __convert_long4_sat_rtp_long4, __convert_long4_sat_rtp_float4, __convert_long4_sat_rtp_double4 )
    #define convert_long8_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_long8_sat_rtp_uchar8, __convert_long8_sat_rtp_ushort8, __convert_long8_sat_rtp_uint8, __convert_long8_sat_rtp_ulong8, __convert_long8_char8, __convert_long8_short8, __convert_long8_sat_rtp_int8, __convert_long8_sat_rtp_long8, __convert_long8_sat_rtp_float8, __convert_long8_sat_rtp_double8 )
    #define convert_long16_sat_rtp(_X)    __builtin_overload( 1, _X, __convert_long16_sat_rtp_uchar16, __convert_long16_sat_rtp_ushort16, __convert_long16_sat_rtp_uint16, __convert_long16_sat_rtp_ulong16, __convert_long16_char16, __convert_long16_sat_rtp_short16, __convert_long16_sat_rtp_int16, __convert_long16_sat_rtp_long16, __convert_long16_sat_rtp_float16, __convert_long16_sat_rtp_double16 )
    #define convert_long_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_long_sat_rtn_uchar, __convert_long_sat_rtn_ushort, __convert_long_sat_rtn_uint, __convert_long_sat_ulong, __convert_long_char, __convert_long_short, __convert_long_int, __convert_long_sat_rtn_long, __convert_long_sat_rtn_float, __convert_long_sat_rtn_double )
    #define convert_long2_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_long2_sat_rtn_uchar2, __convert_long2_sat_rtn_ushort2, __convert_long2_sat_rtn_uint2, __convert_long2_sat_ulong2, __convert_long2_char2, __convert_long2_short2, __convert_long2_int2, __convert_long2_sat_rtn_long2, __convert_long2_sat_rtn_float2, __convert_long2_sat_rtn_double2 )
    #define convert_long4_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_long4_sat_rtn_uchar4, __convert_long4_sat_rtn_ushort4, __convert_long4_sat_rtn_uint4, __convert_long4_sat_rtn_ulong4, __convert_long4_char4, __convert_long4_short4, __convert_long4_int4, __convert_long4_sat_rtn_long4, __convert_long4_sat_rtn_float4, __convert_long4_sat_rtn_double4 )
    #define convert_long8_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_long8_sat_rtn_uchar8, __convert_long8_sat_rtn_ushort8, __convert_long8_sat_rtn_uint8, __convert_long8_sat_rtn_ulong8, __convert_long8_char8, __convert_long8_short8, __convert_long8_sat_rtn_int8, __convert_long8_sat_rtn_long8, __convert_long8_sat_rtn_float8, __convert_long8_sat_rtn_double8 )
    #define convert_long16_sat_rtn(_X)    __builtin_overload( 1, _X, __convert_long16_sat_rtn_uchar16, __convert_long16_sat_rtn_ushort16, __convert_long16_sat_rtn_uint16, __convert_long16_sat_rtn_ulong16, __convert_long16_char16, __convert_long16_sat_rtn_short16, __convert_long16_sat_rtn_int16, __convert_long16_sat_rtn_long16, __convert_long16_sat_rtn_float16, __convert_long16_sat_rtn_double16 )
    #define convert_long_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_long_sat_rtz_uchar, __convert_long_sat_rtz_ushort, __convert_long_sat_rtz_uint, __convert_long_sat_ulong, __convert_long_char, __convert_long_short, __convert_long_int, __convert_long_sat_rtz_long, __convert_long_sat_rtz_float, __convert_long_sat_rtz_double )
    #define convert_long2_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_long2_sat_rtz_uchar2, __convert_long2_sat_rtz_ushort2, __convert_long2_sat_rtz_uint2, __convert_long2_sat_ulong2, __convert_long2_char2, __convert_long2_short2, __convert_long2_int2, __convert_long2_sat_rtz_long2, __convert_long2_sat_rtz_float2, __convert_long2_sat_rtz_double2 )
    #define convert_long4_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_long4_sat_rtz_uchar4, __convert_long4_sat_rtz_ushort4, __convert_long4_sat_rtz_uint4, __convert_long4_sat_rtz_ulong4, __convert_long4_char4, __convert_long4_short4, __convert_long4_int4, __convert_long4_sat_rtz_long4, __convert_long4_sat_rtz_float4, __convert_long4_sat_rtz_double4 )
    #define convert_long8_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_long8_sat_rtz_uchar8, __convert_long8_sat_rtz_ushort8, __convert_long8_sat_rtz_uint8, __convert_long8_sat_rtz_ulong8, __convert_long8_char8, __convert_long8_short8, __convert_long8_sat_rtz_int8, __convert_long8_sat_rtz_long8, __convert_long8_sat_rtz_float8, __convert_long8_sat_rtz_double8 )
    #define convert_long16_sat_rtz(_X)    __builtin_overload( 1, _X, __convert_long16_sat_rtz_uchar16, __convert_long16_sat_rtz_ushort16, __convert_long16_sat_rtz_uint16, __convert_long16_sat_rtz_ulong16, __convert_long16_char16, __convert_long16_sat_rtz_short16, __convert_long16_sat_rtz_int16, __convert_long16_sat_rtz_long16, __convert_long16_sat_rtz_float16, __convert_long16_sat_rtz_double16 )
    #define convert_float(_X)    __builtin_overload( 1, _X, __convert_float_uchar, __convert_float_ushort, __convert_float_uint, __convert_float_ulong, __convert_float_char, __convert_float_short, __convert_float_int, __convert_float_long, __convert_float_float, __convert_float_double )
    #define convert_float2(_X)    __builtin_overload( 1, _X, __convert_float2_uchar2, __convert_float2_ushort2, __convert_float2_uint2, __convert_float2_ulong2, __convert_float2_char2, __convert_float2_short2, __convert_float2_int2, __convert_float2_long2, __convert_float2_float2, __convert_float2_double2 )
    #define convert_float4(_X)    __builtin_overload( 1, _X, __convert_float4_uchar4, __convert_float4_ushort4, __convert_float4_uint4, __convert_float4_ulong4, __convert_float4_char4, __convert_float4_short4, __convert_float4_int4, __convert_float4_long4, __convert_float4_float4, __convert_float4_double4 )
    #define convert_float8(_X)    __builtin_overload( 1, _X, __convert_float8_uchar8, __convert_float8_ushort8, __convert_float8_uint8, __convert_float8_ulong8, __convert_float8_char8, __convert_float8_short8, __convert_float8_int8, __convert_float8_long8, __convert_float8_float8, __convert_float8_double8 )
    #define convert_float16(_X)    __builtin_overload( 1, _X, __convert_float16_uchar16, __convert_float16_ushort16, __convert_float16_uint16, __convert_float16_ulong16, __convert_float16_char16, __convert_float16_short16, __convert_float16_int16, __convert_float16_long16, __convert_float16_float16, __convert_float16_double16 )
    #define convert_float_rte(_X)    __builtin_overload( 1, _X, __convert_float_uchar, __convert_float_ushort, __convert_float_uint, __convert_float_ulong, __convert_float_char, __convert_float_short, __convert_float_int, __convert_float_long, __convert_float_rte_float, __convert_float_double )
    #define convert_float2_rte(_X)    __builtin_overload( 1, _X, __convert_float2_rte_uchar2, __convert_float2_rte_ushort2, __convert_float2_rte_uint2, __convert_float2_ulong2, __convert_float2_rte_char2, __convert_float2_rte_short2, __convert_float2_rte_int2, __convert_float2_long2, __convert_float2_rte_float2, __convert_float2_double2 )
    #define convert_float4_rte(_X)    __builtin_overload( 1, _X, __convert_float4_uchar4, __convert_float4_ushort4, __convert_float4_uint4, __convert_float4_ulong4, __convert_float4_char4, __convert_float4_short4, __convert_float4_int4, __convert_float4_long4, __convert_float4_rte_float4, __convert_float4_double4 )
    #define convert_float8_rte(_X)    __builtin_overload( 1, _X, __convert_float8_uchar8, __convert_float8_ushort8, __convert_float8_rte_uint8, __convert_float8_rte_ulong8, __convert_float8_char8, __convert_float8_short8, __convert_float8_rte_int8, __convert_float8_rte_long8, __convert_float8_rte_float8, __convert_float8_rte_double8 )
    #define convert_float16_rte(_X)    __builtin_overload( 1, _X, __convert_float16_uchar16, __convert_float16_rte_ushort16, __convert_float16_rte_uint16, __convert_float16_rte_ulong16, __convert_float16_char16, __convert_float16_rte_short16, __convert_float16_rte_int16, __convert_float16_rte_long16, __convert_float16_rte_float16, __convert_float16_rte_double16 )
    #define convert_float_rtp(_X)    __builtin_overload( 1, _X, __convert_float_uchar, __convert_float_ushort, __convert_float_rtp_uint, __convert_float_rtp_ulong, __convert_float_char, __convert_float_short, __convert_float_rtp_int, __convert_float_rtp_long, __convert_float_rtp_float, __convert_float_rtp_double )
    #define convert_float2_rtp(_X)    __builtin_overload( 1, _X, __convert_float2_rtp_uchar2, __convert_float2_rtp_ushort2, __convert_float2_rtp_uint2, __convert_float2_rtp_ulong2, __convert_float2_rtp_char2, __convert_float2_rtp_short2, __convert_float2_rtp_int2, __convert_float2_rtp_long2, __convert_float2_rtp_float2, __convert_float2_rtp_double2 )
    #define convert_float4_rtp(_X)    __builtin_overload( 1, _X, __convert_float4_uchar4, __convert_float4_ushort4, __convert_float4_rtp_uint4, __convert_float4_rtp_ulong4, __convert_float4_char4, __convert_float4_short4, __convert_float4_rtp_int4, __convert_float4_rtp_long4, __convert_float4_rtp_float4, __convert_float4_rtp_double4 )
    #define convert_float8_rtp(_X)    __builtin_overload( 1, _X, __convert_float8_uchar8, __convert_float8_ushort8, __convert_float8_rtp_uint8, __convert_float8_rtp_ulong8, __convert_float8_char8, __convert_float8_short8, __convert_float8_rtp_int8, __convert_float8_rtp_long8, __convert_float8_rtp_float8, __convert_float8_rtp_double8 )
    #define convert_float16_rtp(_X)    __builtin_overload( 1, _X, __convert_float16_uchar16, __convert_float16_rtp_ushort16, __convert_float16_rtp_uint16, __convert_float16_rtp_ulong16, __convert_float16_char16, __convert_float16_rtp_short16, __convert_float16_rtp_int16, __convert_float16_rtp_long16, __convert_float16_rtp_float16, __convert_float16_rtp_double16 )
    #define convert_float_rtn(_X)    __builtin_overload( 1, _X, __convert_float_uchar, __convert_float_ushort, __convert_float_rtn_uint, __convert_float_rtn_ulong, __convert_float_char, __convert_float_short, __convert_float_rtn_int, __convert_float_rtn_long, __convert_float_rtn_float, __convert_float_rtn_double )
    #define convert_float2_rtn(_X)    __builtin_overload( 1, _X, __convert_float2_rtn_uchar2, __convert_float2_rtn_ushort2, __convert_float2_rtn_uint2, __convert_float2_rtn_ulong2, __convert_float2_rtn_char2, __convert_float2_rtn_short2, __convert_float2_rtn_int2, __convert_float2_rtn_long2, __convert_float2_rtn_float2, __convert_float2_rtn_double2 )
    #define convert_float4_rtn(_X)    __builtin_overload( 1, _X, __convert_float4_uchar4, __convert_float4_ushort4, __convert_float4_rtn_uint4, __convert_float4_rtn_ulong4, __convert_float4_char4, __convert_float4_short4, __convert_float4_rtn_int4, __convert_float4_rtn_long4, __convert_float4_rtn_float4, __convert_float4_rtn_double4 )
    #define convert_float8_rtn(_X)    __builtin_overload( 1, _X, __convert_float8_uchar8, __convert_float8_ushort8, __convert_float8_rtn_uint8, __convert_float8_rtn_ulong8, __convert_float8_char8, __convert_float8_short8, __convert_float8_rtn_int8, __convert_float8_rtn_long8, __convert_float8_rtn_float8, __convert_float8_rtn_double8 )
    #define convert_float16_rtn(_X)    __builtin_overload( 1, _X, __convert_float16_uchar16, __convert_float16_rtn_ushort16, __convert_float16_rtn_uint16, __convert_float16_rtn_ulong16, __convert_float16_char16, __convert_float16_rtn_short16, __convert_float16_rtn_int16, __convert_float16_rtn_long16, __convert_float16_rtn_float16, __convert_float16_rtn_double16 )
    #define convert_float_rtz(_X)    __builtin_overload( 1, _X, __convert_float_uchar, __convert_float_ushort, __convert_float_rtz_uint, __convert_float_rtz_ulong, __convert_float_char, __convert_float_short, __convert_float_rtz_int, __convert_float_rtz_long, __convert_float_rtz_float, __convert_float_rtz_double )
    #define convert_float2_rtz(_X)    __builtin_overload( 1, _X, __convert_float2_rtz_uchar2, __convert_float2_rtz_ushort2, __convert_float2_rtz_uint2, __convert_float2_rtz_ulong2, __convert_float2_rtz_char2, __convert_float2_rtz_short2, __convert_float2_rtz_int2, __convert_float2_rtz_long2, __convert_float2_rtz_float2, __convert_float2_rtz_double2 )
    #define convert_float4_rtz(_X)    __builtin_overload( 1, _X, __convert_float4_uchar4, __convert_float4_ushort4, __convert_float4_rtz_uint4, __convert_float4_rtz_ulong4, __convert_float4_char4, __convert_float4_short4, __convert_float4_rtz_int4, __convert_float4_rtz_long4, __convert_float4_rtz_float4, __convert_float4_rtz_double4 )
    #define convert_float8_rtz(_X)    __builtin_overload( 1, _X, __convert_float8_uchar8, __convert_float8_ushort8, __convert_float8_rtz_uint8, __convert_float8_rtz_ulong8, __convert_float8_char8, __convert_float8_short8, __convert_float8_rtz_int8, __convert_float8_rtz_long8, __convert_float8_rtz_float8, __convert_float8_rtz_double8 )
    #define convert_float16_rtz(_X)    __builtin_overload( 1, _X, __convert_float16_uchar16, __convert_float16_rtz_ushort16, __convert_float16_rtz_uint16, __convert_float16_rtz_ulong16, __convert_float16_char16, __convert_float16_rtz_short16, __convert_float16_rtz_int16, __convert_float16_rtz_long16, __convert_float16_rtz_float16, __convert_float16_rtz_double16 )
    #define convert_double(_X)    __builtin_overload( 1, _X, __convert_double_uchar, __convert_double_ushort, __convert_double_uint, __convert_double_ulong, __convert_double_char, __convert_double_short, __convert_double_int, __convert_double_long, __convert_double_float, __convert_double_double )
    #define convert_double2(_X)    __builtin_overload( 1, _X, __convert_double2_uchar2, __convert_double2_ushort2, __convert_double2_uint2, __convert_double2_ulong2, __convert_double2_char2, __convert_double2_short2, __convert_double2_int2, __convert_double2_long2, __convert_double2_float2, __convert_double2_double2 )
    #define convert_double4(_X)    __builtin_overload( 1, _X, __convert_double4_uchar4, __convert_double4_ushort4, __convert_double4_uint4, __convert_double4_ulong4, __convert_double4_char4, __convert_double4_short4, __convert_double4_int4, __convert_double4_long4, __convert_double4_float4, __convert_double4_double4 )
    #define convert_double8(_X)    __builtin_overload( 1, _X, __convert_double8_uchar8, __convert_double8_ushort8, __convert_double8_uint8, __convert_double8_ulong8, __convert_double8_char8, __convert_double8_short8, __convert_double8_int8, __convert_double8_long8, __convert_double8_float8, __convert_double8_double8 )
    #define convert_double16(_X)    __builtin_overload( 1, _X, __convert_double16_uchar16, __convert_double16_ushort16, __convert_double16_uint16, __convert_double16_ulong16, __convert_double16_char16, __convert_double16_short16, __convert_double16_int16, __convert_double16_long16, __convert_double16_float16, __convert_double16_double16 )
    #define convert_double_rte(_X)    __builtin_overload( 1, _X, __convert_double_uchar, __convert_double_ushort, __convert_double_uint, __convert_double_ulong, __convert_double_char, __convert_double_short, __convert_double_int, __convert_double_long, __convert_double_float, __convert_double_rte_double )
    #define convert_double2_rte(_X)    __builtin_overload( 1, _X, __convert_double2_uchar2, __convert_double2_ushort2, __convert_double2_uint2, __convert_double2_ulong2, __convert_double2_char2, __convert_double2_short2, __convert_double2_int2, __convert_double2_long2, __convert_double2_float2, __convert_double2_rte_double2 )
    #define convert_double4_rte(_X)    __builtin_overload( 1, _X, __convert_double4_uchar4, __convert_double4_ushort4, __convert_double4_uint4, __convert_double4_rte_ulong4, __convert_double4_char4, __convert_double4_short4, __convert_double4_int4, __convert_double4_rte_long4, __convert_double4_float4, __convert_double4_rte_double4 )
    #define convert_double8_rte(_X)    __builtin_overload( 1, _X, __convert_double8_uchar8, __convert_double8_ushort8, __convert_double8_rte_uint8, __convert_double8_rte_ulong8, __convert_double8_char8, __convert_double8_short8, __convert_double8_rte_int8, __convert_double8_rte_long8, __convert_double8_rte_float8, __convert_double8_rte_double8 )
    #define convert_double16_rte(_X)    __builtin_overload( 1, _X, __convert_double16_uchar16, __convert_double16_rte_ushort16, __convert_double16_rte_uint16, __convert_double16_rte_ulong16, __convert_double16_char16, __convert_double16_rte_short16, __convert_double16_rte_int16, __convert_double16_rte_long16, __convert_double16_rte_float16, __convert_double16_rte_double16 )
    #define convert_double_rtp(_X)    __builtin_overload( 1, _X, __convert_double_uchar, __convert_double_ushort, __convert_double_uint, __convert_double_rtp_ulong, __convert_double_char, __convert_double_short, __convert_double_int, __convert_double_rtp_long, __convert_double_float, __convert_double_rtp_double )
    #define convert_double2_rtp(_X)    __builtin_overload( 1, _X, __convert_double2_uchar2, __convert_double2_ushort2, __convert_double2_uint2, __convert_double2_rtp_ulong2, __convert_double2_char2, __convert_double2_short2, __convert_double2_int2, __convert_double2_rtp_long2, __convert_double2_float2, __convert_double2_rtp_double2 )
    #define convert_double4_rtp(_X)    __builtin_overload( 1, _X, __convert_double4_uchar4, __convert_double4_ushort4, __convert_double4_uint4, __convert_double4_rtp_ulong4, __convert_double4_char4, __convert_double4_short4, __convert_double4_int4, __convert_double4_rtp_long4, __convert_double4_float4, __convert_double4_rtp_double4 )
    #define convert_double8_rtp(_X)    __builtin_overload( 1, _X, __convert_double8_uchar8, __convert_double8_ushort8, __convert_double8_rtp_uint8, __convert_double8_rtp_ulong8, __convert_double8_char8, __convert_double8_short8, __convert_double8_rtp_int8, __convert_double8_rtp_long8, __convert_double8_rtp_float8, __convert_double8_rtp_double8 )
    #define convert_double16_rtp(_X)    __builtin_overload( 1, _X, __convert_double16_uchar16, __convert_double16_rtp_ushort16, __convert_double16_rtp_uint16, __convert_double16_rtp_ulong16, __convert_double16_char16, __convert_double16_rtp_short16, __convert_double16_rtp_int16, __convert_double16_rtp_long16, __convert_double16_rtp_float16, __convert_double16_rtp_double16 )
    #define convert_double_rtn(_X)    __builtin_overload( 1, _X, __convert_double_uchar, __convert_double_ushort, __convert_double_uint, __convert_double_rtn_ulong, __convert_double_char, __convert_double_short, __convert_double_int, __convert_double_rtn_long, __convert_double_float, __convert_double_rtn_double )
    #define convert_double2_rtn(_X)    __builtin_overload( 1, _X, __convert_double2_uchar2, __convert_double2_ushort2, __convert_double2_uint2, __convert_double2_rtn_ulong2, __convert_double2_char2, __convert_double2_short2, __convert_double2_int2, __convert_double2_rtn_long2, __convert_double2_float2, __convert_double2_rtn_double2 )
    #define convert_double4_rtn(_X)    __builtin_overload( 1, _X, __convert_double4_uchar4, __convert_double4_ushort4, __convert_double4_uint4, __convert_double4_rtn_ulong4, __convert_double4_char4, __convert_double4_short4, __convert_double4_int4, __convert_double4_rtn_long4, __convert_double4_float4, __convert_double4_rtn_double4 )
    #define convert_double8_rtn(_X)    __builtin_overload( 1, _X, __convert_double8_uchar8, __convert_double8_ushort8, __convert_double8_rtn_uint8, __convert_double8_rtn_ulong8, __convert_double8_char8, __convert_double8_short8, __convert_double8_rtn_int8, __convert_double8_rtn_long8, __convert_double8_rtn_float8, __convert_double8_rtn_double8 )
    #define convert_double16_rtn(_X)    __builtin_overload( 1, _X, __convert_double16_uchar16, __convert_double16_rtn_ushort16, __convert_double16_rtn_uint16, __convert_double16_rtn_ulong16, __convert_double16_char16, __convert_double16_rtn_short16, __convert_double16_rtn_int16, __convert_double16_rtn_long16, __convert_double16_rtn_float16, __convert_double16_rtn_double16 )
    #define convert_double_rtz(_X)    __builtin_overload( 1, _X, __convert_double_uchar, __convert_double_ushort, __convert_double_uint, __convert_double_rtz_ulong, __convert_double_char, __convert_double_short, __convert_double_int, __convert_double_rtz_long, __convert_double_float, __convert_double_rtz_double )
    #define convert_double2_rtz(_X)    __builtin_overload( 1, _X, __convert_double2_uchar2, __convert_double2_ushort2, __convert_double2_uint2, __convert_double2_rtz_ulong2, __convert_double2_char2, __convert_double2_short2, __convert_double2_int2, __convert_double2_rtz_long2, __convert_double2_float2, __convert_double2_rtz_double2 )
    #define convert_double4_rtz(_X)    __builtin_overload( 1, _X, __convert_double4_uchar4, __convert_double4_ushort4, __convert_double4_uint4, __convert_double4_rtz_ulong4, __convert_double4_char4, __convert_double4_short4, __convert_double4_int4, __convert_double4_rtz_long4, __convert_double4_float4, __convert_double4_rtz_double4 )
    #define convert_double8_rtz(_X)    __builtin_overload( 1, _X, __convert_double8_uchar8, __convert_double8_ushort8, __convert_double8_rtz_uint8, __convert_double8_rtz_ulong8, __convert_double8_char8, __convert_double8_short8, __convert_double8_rtz_int8, __convert_double8_rtz_long8, __convert_double8_rtz_float8, __convert_double8_rtz_double8 )
    #define convert_double16_rtz(_X)    __builtin_overload( 1, _X, __convert_double16_uchar16, __convert_double16_rtz_ushort16, __convert_double16_rtz_uint16, __convert_double16_rtz_ulong16, __convert_double16_char16, __convert_double16_rtz_short16, __convert_double16_rtz_int16, __convert_double16_rtz_long16, __convert_double16_rtz_float16, __convert_double16_rtz_double16 )

#endif

// 6.2.4.2 as_typen
#define as_char( _x )   __builtin_astype( _x, char )
#define as_uchar( _x )  __builtin_astype( _x, uchar )
#define as_short( _x )  __builtin_astype( _x, short )
#define as_ushort( _x ) __builtin_astype( _x, ushort )
#define as_int( _x )    __builtin_astype( _x, int )
#define as_uint( _x )   __builtin_astype( _x, uint )
#define as_float( _x )  __builtin_astype( _x, float )
#define as_long( _x )   __builtin_astype( _x, long )
#define as_ulong( _x )  __builtin_astype( _x, ulong )
#define as_double( _x ) __builtin_astype( _x, double )

#define as_size_t( _x ) __builtin_astype( _x, size_t )
#define as_intptr_t( _x ) __builtin_astype( _x, intptr_t )
#define as_uintptr_t( _x ) __builtin_astype( _x, uintptr_t )
#define as_ptrdiff_t( _x ) __builtin_astype( _x, ptrdiff_t )

#define as_char2( _x )   __builtin_astype( _x, char2 )
#define as_char4( _x )   __builtin_astype( _x, char4 )
#define as_char8( _x )   __builtin_astype( _x, char8 )
#define as_char16( _x )  __builtin_astype( _x, char16 )

#define as_uchar2( _x )   __builtin_astype( _x, uchar2 )
#define as_uchar4( _x )   __builtin_astype( _x, uchar4 )
#define as_uchar8( _x )   __builtin_astype( _x, uchar8 )
#define as_uchar16( _x )  __builtin_astype( _x, uchar16 )

#define as_short2( _x )   __builtin_astype( _x, short2 )
#define as_short4( _x )   __builtin_astype( _x, short4 )
#define as_short8( _x )   __builtin_astype( _x, short8 )
#define as_short16( _x )  __builtin_astype( _x, short16 )

#define as_ushort2( _x )   __builtin_astype( _x, ushort2 )
#define as_ushort4( _x )   __builtin_astype( _x, ushort4 )
#define as_ushort8( _x )   __builtin_astype( _x, ushort8 )
#define as_ushort16( _x )  __builtin_astype( _x, ushort16 )

#define as_int2( _x )   __builtin_astype( _x, int2 )
#define as_int4( _x )   __builtin_astype( _x, int4 )
#define as_int8( _x )   __builtin_astype( _x, int8 )
#define as_int16( _x )  __builtin_astype( _x, int16 )

#define as_uint2( _x )   __builtin_astype( _x, uint2 )
#define as_uint4( _x )   __builtin_astype( _x, uint4 )
#define as_uint8( _x )   __builtin_astype( _x, uint8 )
#define as_uint16( _x )  __builtin_astype( _x, uint16 )

#define as_float2( _x )   __builtin_astype( _x, float2 )
#define as_float4( _x )   __builtin_astype( _x, float4 )
#define as_float8( _x )   __builtin_astype( _x, float8 )
#define as_float16( _x )  __builtin_astype( _x, float16 )

#define as_long2( _x )   __builtin_astype( _x, long2 )
#define as_long4( _x )   __builtin_astype( _x, long4 )
#define as_long8( _x )   __builtin_astype( _x, long8 )
#define as_long16( _x )  __builtin_astype( _x, long16 )

#define as_ulong2( _x )   __builtin_astype( _x, ulong2 )
#define as_ulong4( _x )   __builtin_astype( _x, ulong4 )
#define as_ulong8( _x )   __builtin_astype( _x, ulong8 )
#define as_ulong16( _x )  __builtin_astype( _x, ulong16 )

#define as_double2( _x )   __builtin_astype( _x, double2 )
#define as_double4( _x )   __builtin_astype( _x, double4 )
#define as_double8( _x )   __builtin_astype( _x, double8 )
#define as_double16( _x )  __builtin_astype( _x, double16 )

// 5.5 Function Qualifiers
#define __kernel __attribute__((annotate("kernel")))
#define kernel __attribute__((annotate("kernel")))

// 5.6 Image Access Qualifiers
#define __rd __attribute__((annotate("__rd")))  
#define __wr __attribute__((annotate("__wr")))  
#define __read_only __attribute__((annotate("__rd")))  
#define read_only __attribute__((annotate("__rd")))  
#define __write_only __attribute__((annotate("__wr")))  
#define write_only __attribute__((annotate("__wr")))  

// 5.9.1 - Work-item Functions
#if defined( __i386__ ) || defined( __i686__ )
/*typedef size_t * __attribute__((address_space(256))) *__gs;
#define get_work_dim()       (unsigned)(((__gs)(0x48))[35][0])
#define get_global_size(X)   (((__gs)(0x48))[32][X])
#define get_global_id(X)     (((__gs)(0x48))[31][X])
#define get_local_size(X)    (1)
#define get_local_id(X)      (0)
#define get_num_groups(X)    (((__gs)(0x48))[34][X])
#define get_group_id(X)      (((__gs)(0x48))[33][X])*/
unsigned int get_work_dim();
size_t get_global_size(unsigned int);
size_t get_global_id(unsigned int);
size_t get_local_size(unsigned int);
size_t get_local_id(unsigned int);
size_t get_num_groups(unsigned int);
size_t get_group_id(unsigned int);
#elif defined( __x86_64__ )
typedef size_t * __attribute__((address_space(256))) *__gs;
#define get_work_dim()       (unsigned)(((__gs)(0x60))[35][0])
#define get_global_size(X)   (((__gs)(0x60))[32][X])
#define get_global_id(X)     (((__gs)(0x60))[31][X])
#define get_local_size(X)    (1)
#define get_local_id(X)      (0)
#define get_num_groups(X)    (((__gs)(0x60))[34][X])
#define get_group_id(X)      (((__gs)(0x60))[33][X])
#else
// These are defined for GPUs by their respective builtin implementations.
uint   get_work_dim(void);
size_t get_global_size(uint dimindx);
size_t get_global_id(uint dimindx);
size_t get_local_size(uint dimindx);
size_t get_local_id (uint dimindx);
size_t get_num_groups(uint dimindx);
size_t get_group_id(uint dimindx);
#endif

// 5.9.2 - Math Defines
#define MAXFLOAT ((float)3.40282346638528860e+38)
#define HUGE_VALF __builtin_huge_valf()
#define INFINITY __builtin_inff()
#define NAN __builtin_nanf("")

/* Section 5.9.2, Table 5.6 */
#define acos(X)     __CLFN_F1(X,acos)
#define acosh(X)    __CLFN_F1(X,acosh)
#define acospi(X)   __CLFN_F1(X,acospi)
#define asin(X)     __CLFN_F1(X,asin)
#define asinh(X)    __CLFN_F1(X,asinh)
#define asinpi(X)   __CLFN_F1(X,asinpi)
#define atan(X)     __CLFN_F1(X,atan)
#define atanh(X)    __CLFN_F1(X,atanh)
#define atanpi(X)    __CLFN_F1(X,atanpi)

#define atan2(X,Y)  __CLFN_F2(X,Y,atan2)
#define atan2pi(X,Y)  __CLFN_F2(X,Y,atan2pi)
#define cbrt(X)     __CLFN_F1(X,cbrt)
#define ceil(X)     __CLFN_F1_SPI(X,ceil)
#define copysign(X,Y) __CLFN_F2(X,Y,copysign)
#define cosh(X)     __CLFN_F1(X,cosh)
#define cospi(X)     __CLFN_F1(X,cospi)
#define erf(X)      __CLFN_F1(X,erf)
#define erfc(X)     __CLFN_F1(X,erfc)
#define exp(X)      __CLFN_F1(X,exp)
#define exp2(X)     __CLFN_F1(X,exp2)
#define exp10(X)    __CLFN_F1(X,exp10)
#define expm1(X)    __CLFN_F1(X,expm1)
#define fabs(X)     __CLFN_F1_SPI(X,fabs)
#define fdim(X,Y)   __CLFN_F2(X,Y,fdim)
#define floor(X)    __CLFN_F1_SPI(X,floor)
#define fma(X,Y,Z)  __builtin_overload( 3, X, Y, Z, __fmaf, __fmaf2, __fmaf4, __fmaf8, __fmaf16,    \
                                                    __fmad, __fmad2, __fmad4, __fmad8, __fmad16     )
                                                    
#define fmax(X,Y) __builtin_overload(2,X,Y, __fmaxf, __fmaxf2, __fmaxf3, __fmaxf4, __fmaxf8, __fmaxf16, \
                                            __fmaxff2, __fmaxff3, __fmaxff4, __fmaxff8, __fmaxff16,     \
                                            fmax, __fmaxd2, __fmaxd4, __fmaxd8, __fmaxd16,              \
                                            __fmaxdd2, __fmaxdd4, __fmaxdd8, __fmaxdd16 )
#define fmin(X,Y) __builtin_overload(2,X,Y, __fminf, __fminf2, __fminf3, __fminf4, __fminf8, __fminf16, \
                                            __fminff2, __fminff3, __fminff4, __fminff8, __fminff16,     \
                                            fmin, __fmind2, __fmind4, __fmind8, __fmind16,              \
                                            __fmindd2, __fmindd4, __fmindd8, __fmindd16 )
                                            
#define fmod(X,Y) __builtin_overload(2,X,Y, __fmodf, __fmodf2, __fmodf4, __fmodf8, __fmodf16,           \
                                            __fmodd, __fmodd2, __fmodd4, __fmodd8, __fmodd16 )
                                            
#define fract(X, Y)    __builtin_overload( 2, X, Y, __fractgf, __fractgf2, __fractgf4, __fractgf8, __fractgf16,     \
                                                __fractlf, __fractlf2, __fractlf4, __fractlf8, __fractlf16,      \
                                                __fractf, __fractf2, __fractf4, __fractf8, __fractf16,           \
                                                __fractgd, __fractgd2, __fractgd4, __fractgd8, __fractgd16,      \
                                                __fractld, __fractld2, __fractld4, __fractld8, __fractld16,      \
                                                fract,   __fractd2, __fractd4, __fractd8, __fractd16             \
                                            )
#define frexp(X,Y)  __builtin_overload( 2, X, Y, __frexpf, __frexpf2, __frexpf4, __frexpf8, __frexpf16,         \
                                                __frexpgf, __frexpgf2, __frexpgf4, __frexpgf8, __frexpgf16,     \
                                                __frexplf, __frexplf2, __frexplf4, __frexplf8, __frexplf16,     \
                                                frexp, __frexpd2, __frexpd4, __frexpd8, __frexpd16,             \
                                                __frexpgd, __frexpgd2, __frexpgd4, __frexpgd8, __frexpgd16,     \
                                                __frexpld, __frexpld2, __frexpld4, __frexpld8, __frexpld16      )
#define hypot(X,Y)  __CLFN_F2(X,Y,hypot)
#define ilogb(X)    __CLFN_F1(X,ilogb)
#define ldexp(X,Y) __CLFN_F2(X,Y,ldexp)

#define lgamma(X)   __CLFN_F1(X,lgamma)
#define lgamma_r(X,Y)   __builtin_overload( 2, X, Y, __lgamma_rf, __lgamma_rf2, __lgamma_rf4, __lgamma_rf8, __lgamma_rf16,         \
                                                     __lgamma_rlf, __lgamma_rlf2, __lgamma_rlf4, __lgamma_rlf8, __lgamma_rlf16,    \
													 __lgamma_rgf, __lgamma_rgf2, __lgamma_rgf4, __lgamma_rgf8, __lgamma_rgf16,    \
													lgamma_r, __lgamma_rd2, __lgamma_rd4, __lgamma_rd8, __lgamma_rd16,            \
                                                    __lgamma_rld, __lgamma_rld2, __lgamma_rld4, __lgamma_rld8, __lgamma_rld16,    \
													__lgamma_rgd, __lgamma_rgd2, __lgamma_rgd4, __lgamma_rgd8, __lgamma_rgd16     )

#define log(X)      __CLFN_F1(X,log)
#define log10(X)    __CLFN_F1(X,log10)
#define log1p(X)    __CLFN_F1(X,log1p)
#define log2(X)     __CLFN_F1(X,log2)
#define logb(X)     __CLFN_F1(X,logb)
#define mad(X,Y,Z)  __builtin_overload(3,X,Y,Z, __madf, __madf2, __madf3, __madf4, __madf8, __madf16,   \
                                                mad, __madd2, __madd4, __madd8, __madd16)

#define modf(X,Y)   __builtin_overload( 2, X, Y, __modfgf, __modfgf2, __modfgf4, __modfgf8, __modfgf16,     \
                                                __modflf, __modflf2, __modflf4, __modflf8, __modflf16,      \
                                                __modff, __modff2, __modff4, __modff8, __modff16,           \
                                                __modfgd, __modfgd2, __modfgd4, __modfgd8, __modfgd16,      \
                                                __modfld, __modfld2, __modfld4, __modfld8, __modfld16,      \
                                                modf,   __modfd2, __modfd4, __modfd8, __modfd16            \
                                            )
#define nan(X)      __builtin_overload( 1, X, __nanf, __nanf2, __nanf4, __nanf8, __nanf16, \
                                              __nand, __nand2, __nand4, __nand8, __nand16 )

#define nextafter(X,Y)  __CLFN_F2(X,Y,nextafter)
#define pow(X,Y)    __CLFN_F2(X,Y,pow)
#define pown(X,Y)   __CLFN_F2(X,Y,pown)
#define powr(X,Y)   __CLFN_F2(X,Y,powr)
#define remainder(X,Y) __CLFN_F2(X,Y,remainder)
#define remquo(X,Y,Z)   __builtin_overload( 3, X, Y, Z, __remquof, __remquof2, __remquof4, __remquof8, __remquof16,         \
                                                        __remquolf, __remquolf2, __remquolf4, __remquolf8, __remquolf16,    \
                                                        __remquogf, __remquogf2, __remquogf4, __remquogf8, __remquogf16,    \
                                                        remquo, __remquod2, __remquod4, __remquod8, __remquod16,            \
                                                        __remquold, __remquold2, __remquold4, __remquold8, __remquold16,    \
                                                        __remquogd, __remquogd2, __remquogd4, __remquogd8, __remquogd16     )
#define rint(X)     __CLFN_F1(X,rint)
#define rootn(X,Y)  __CLFN_F2(X,Y,rootn)
#define round(X)    __CLFN_F1(X,round)
#define rsqrt(X)    __CLFN_F1(X,rsqrt)

#define cos(X)      __builtin_overload(1, X, __cosf, __cosf2, __cosf4, __cosf8, __cosf16, __cosd, __cosd2, __cosd4, __cosd8, __cosd16 )
#define sin(X)      __builtin_overload(1, X, __sinf, __sinf2, __sinf4, __sinf8, __sinf16, __sind, __sind2, __sind4, __sind8, __sind16 )
#define tan(X)      __builtin_overload(1, X, __tanf, __tanf2, __tanf4, __tanf8, __tanf16, __tand, __tand2, __tand4, __tand8, __tand16 )
#define sincos(X,Y) __builtin_overload( 2, X, Y, __sincosgf, __sincosgf2, __sincosgf4, __sincosgf8, __sincosgf16,   \
                                                __sincoslf, __sincoslf2, __sincoslf4, __sincoslf8, __sincoslf16,    \
                                                __sincosf, __sincosf2, __sincosf4, __sincosf8, __sincosf16,    \
                                                __sincosgd, __sincosgd2, __sincosgd4, __sincosgd8, __sincosgd16,   \
                                                __sincosld, __sincosld2, __sincosld4, __sincosld8, __sincosld16,    \
                                                __sincosd, __sincosd2, __sincosd4, __sincosd8, __sincosd16    \
                                                )
#define sinh(X)     __CLFN_F1(X,sinh)
#define sinpi(X)    __CLFN_F1(X,sinpi)
#define sqrt(X)     __CLFN_F1(X,sqrt)
#define tanh(X)     __CLFN_F1(X,tanh)
#define tanpi(X)    __CLFN_F1(X,tanpi)
#define tgamma(X)   __CLFN_F1(X,tgamma)
#define trunc(X)    __CLFN_F1(X,trunc)

/* Sectio 5.9.2, Table 5.7 */
#define half_cos(X) __CLFN_F1_SINGLE(X,half_cos)
#define half_divide(X,Y) __CLFN_F2_SINGLE(X,Y,half_divide)
#define half_exp(X) __CLFN_F1_SINGLE(X,half_exp)
#define half_exp2(X) __CLFN_F1_SINGLE(X,half_exp2)
#define half_exp10(X) __CLFN_F1_SINGLE(X,half_exp10)
#define half_log(X) __CLFN_F1_SINGLE(X,half_log)
#define half_log2(X) __CLFN_F1_SINGLE(X,half_log2)
#define half_log10(X) __CLFN_F1_SINGLE(X,half_log10)
#define half_powr(X,Y) __CLFN_F2_SINGLE(X,Y,half_powr)
#define half_recip(X) __CLFN_F1_SINGLE(X,half_recip)
#define half_rsqrt(X) __CLFN_F1_SINGLE(X,half_rsqrt)
#define half_sin(X) __CLFN_F1_SINGLE(X,half_sin)
#define half_sqrt(X) __CLFN_F1_SINGLE(X,half_sqrt)
#define half_tan(X) __CLFN_F1_SINGLE(X,half_tan)

#define native_cos(X) __CLFN_F1_SINGLE(X,native_cos)
#define native_divide(X,Y) __CLFN_F2_SINGLE(X,Y,native_divide)
#define native_exp(X) __CLFN_F1_SINGLE(X,native_exp)
#define native_exp2(X) __CLFN_F1_SINGLE(X,native_exp2)
#define native_exp10(X) __CLFN_F1_SINGLE(X,native_exp10)
#define native_log(X) __CLFN_F1_SINGLE(X,native_log)
#define native_log2(X) __CLFN_F1_SINGLE(X,native_log2)
#define native_log10(X) __CLFN_F1_SINGLE(X,native_log10)
#define native_powr(X,Y) __CLFN_F2_SINGLE(X,Y,native_powr)
#define native_recip(X) __CLFN_F1_SINGLE(X,native_recip)
#define native_rsqrt(X) __CLFN_F1_SINGLE(X,native_rsqrt)
#define native_sin(X) __CLFN_F1_SINGLE(X,native_sin)
#define native_sqrt(X) __CLFN_F1_SINGLE(X,native_sqrt)
#define native_tan(X) __CLFN_F1_SINGLE(X,native_tan)

/* Section 5.9.3 */
#define __CLFN_ALL1(x,R) __builtin_overload(1, x,   __##R##_1i8, __##R##_2i8, __##R##_4i8, __##R##_8i8, __##R##_16i8,  \
                                                    __##R##_1u8, __##R##_2u8, __##R##_4u8, __##R##_8u8, __##R##_16u8,  \
                                                    __##R##_1i16, __##R##_2i16, __##R##_4i16, __##R##_8i16, __##R##_16i16,  \
                                                    __##R##_1u16, __##R##_2u16, __##R##_4u16, __##R##_8u16, __##R##_16u16,  \
                                                    __##R##_1i32, __##R##_2i32, __##R##_4i32, __##R##_8i32, __##R##_16i32,  \
                                                    __##R##_1u32, __##R##_2u32, __##R##_4u32, __##R##_8u32, __##R##_16u32,  \
                                                    __##R##_1i64, __##R##_2i64, __##R##_4i64, __##R##_8i64, __##R##_16i64,  \
                                                    __##R##_1u64, __##R##_2u64, __##R##_4u64, __##R##_8u64, __##R##_16u64,  \
                                                    __##R##f, __##R##f2, __##R##f4, __##R##f8, __##R##f16,       \
                                                    R, __##R##d2, __##R##d4, __##R##d8, __##R##d16 )
                                                    
#define __CLFN_ALL2(x,y,R) __builtin_overload(2, x, y,  __##R##_1i8, __##R##_2i8, __##R##_4i8, __##R##_8i8, __##R##_16i8,  \
                                                    __##R##_1u8, __##R##_2u8, __##R##_4u8, __##R##_8u8, __##R##_16u8,  \
                                                    __##R##_1i16, __##R##_2i16, __##R##_4i16, __##R##_8i16, __##R##_16i16,  \
                                                    __##R##_1u16, __##R##_2u16, __##R##_4u16, __##R##_8u16, __##R##_16u16,  \
                                                    __##R##_1i32, __##R##_2i32, __##R##_4i32, __##R##_8i32, __##R##_16i32,  \
                                                    __##R##_1u32, __##R##_2u32, __##R##_4u32, __##R##_8u32, __##R##_16u32,  \
                                                    __##R##_1i64, __##R##_2i64, __##R##_4i64, __##R##_8i64, __##R##_16i64,  \
                                                    __##R##_1u64, __##R##_2u64, __##R##_4u64, __##R##_8u64, __##R##_16u64,  \
                                                    __##R##f, __##R##f2, __##R##f4, __##R##f8, __##R##f16,       \
                                                    R, __##R##d2, __##R##d4, __##R##d8, __##R##d16 )

#define __CLFN_ALL2_plus(x,y,R) __builtin_overload(2, x, y,  __##R##_1i8, __##R##_2i8, __##R##_4i8, __##R##_8i8, __##R##_16i8,  \
                                                    __##R##_1u8, __##R##_2u8, __##R##_4u8, __##R##_8u8, __##R##_16u8,  \
                                                    __##R##_1i16, __##R##_2i16, __##R##_4i16, __##R##_8i16, __##R##_16i16,  \
                                                    __##R##_1u16, __##R##_2u16, __##R##_4u16, __##R##_8u16, __##R##_16u16,  \
                                                    __##R##_1i32, __##R##_2i32, __##R##_4i32, __##R##_8i32, __##R##_16i32,  \
                                                    __##R##_1u32, __##R##_2u32, __##R##_4u32, __##R##_8u32, __##R##_16u32,  \
                                                    __##R##_1i64, __##R##_2i64, __##R##_4i64, __##R##_8i64, __##R##_16i64,  \
                                                    __##R##_1u64, __##R##_2u64, __##R##_4u64, __##R##_8u64, __##R##_16u64,  \
                                                    __##R##f, __##R##f2, __##R##f3, __##R##f4, __##R##f8, __##R##f16,       \
                                                    __##R##ff2, __##R##ff4, __##R##ff8, __##R##ff16,       \
                                                    R, __##R##d2, __##R##d4, __##R##d8, __##R##d16,         \
                                                    __##R##dd2, __##R##dd4, __##R##dd8, __##R##dd16 )

                                                    
#define abs(X)          __builtin_overload(1, X,    __abs_1i8,     __abs_2i8,     __abs_4i8,     __abs_8i8,     __abs_16i8,   \
                                                    __abs_1i16,    __abs_2i16,    __abs_4i16,    __abs_8i16,    __abs_16i16,   \
                                                    __abs_1i32,    __abs_2i32,    __abs_4i32,    __abs_8i32,    __abs_16i32,   \
                                                    __abs_1i64,    __abs_2i64,    __abs_4i64,    __abs_8i64,    __abs_16i64 )
#define abs_diff(X,Y)   __CLFN_I2(X,Y,abs_diff)

#define add_sat(X,Y)    __CLFN_I2(X,Y,add_sat)

#define upsample(X,Y)	__CLFN_I2I2(X,Y,upsample)

#define hadd(X,Y)       __CLFN_I2(X,Y,hadd)
#define rhadd(X,Y)      __CLFN_I2(X,Y,rhadd)
#define clz(X)          __builtin_overload(1, X,    __clz_1u8,     __clz_2u8,     __clz_4u8,     __clz_8u8,     __clz_16u8,   \
                                                    __clz_1u16,    __clz_2u16,    __clz_4u16,    __clz_8u16,    __clz_16u16,   \
                                                    __clz_1u32,    __clz_2u32,    __clz_4u32,    __clz_8u32,    __clz_16u32,   \
                                                    __clz_1u64,    __clz_2u64,    __clz_4u64,    __clz_8u64,    __clz_16u64,    \
                                                    __clz_1i8,     __clz_2i8,     __clz_4i8,     __clz_8i8,     __clz_16i8,   \
                                                    __clz_1i16,    __clz_2i16,    __clz_4i16,    __clz_8i16,    __clz_16i16,   \
                                                    __clz_1i32,    __clz_2i32,    __clz_4i32,    __clz_8i32,    __clz_16i32,   \
                                                    __clz_1i64,    __clz_2i64,    __clz_4i64,    __clz_8i64,    __clz_16i64     )
#define mad_hi(X,Y,Z)       __CLFN_I3(X,Y,Z,mad_hi)
#define mad_sat(X,Y,Z)      __CLFN_I3(X,Y,Z,mad_sat)
#define max(X,Y)            __CLFN_ALL2_plus(X,Y,max)
#define min(X,Y)            __CLFN_ALL2_plus(X,Y,min)
#define rotate(X,Y)         __CLFN_I2(X,Y,rotate)
#define mul_hi(X,Y)         __CLFN_I2(X,Y,mul_hi)
#define sub_sat(X,Y)        __CLFN_I2(X,Y,sub_sat)
#define mad24(X,Y,Z)        __builtin_overload(3, X,Y,Z, __mad24_1i32, __mad24_2i32, __mad24_4i32, __mad24_8i32, __mad24_16i32, __mad24_1u32, __mad24_2u32, __mad24_4u32, __mad24_8u32, __mad24_16u32 )
#define mul24(X,Y)         __builtin_overload(2, X,Y, __mul24_1i32, __mul24_2i32, __mul24_4i32, __mul24_8i32, __mul24_16i32, __mul24_1u32, __mul24_2u32, __mul24_4u32, __mul24_8u32, __mul24_16u32 )

/* Section 5.9.4 */
#define clamp(X,Y,Z)    __builtin_overload(3,X,Y,Z, __clampf, __clampf2, __clampf4, __clampf8, __clampf16,      \
                                                    __clampff2, __clampff4, __clampff8, __clampff16,            \
                                                    clamp, __clampd2, __clampd4, __clampd8, __clampd16,         \
                                                    __clampdd2, __clampdd4, __clampdd8, __clampdd16 )
#define cross(X,Y)      __builtin_overload(2,X,Y, __crossf3, __crossf4, __crossd4)
#define degrees(X)      __CLFN_F1(X,degrees)

#define distance(X,Y)   __builtin_overload( 2, X, Y, __distancef, __distancef2, __distancef3, __distancef4, __distanced, __distanced2, __distanced4 )

#define dot(X,Y)   __builtin_overload( 2, X, Y, __dotf, __dotf2, __dotf3, __dotf4, __dotd, __dotd2, __dotd4 )

#define length(X)   __builtin_overload( 1, X, __lengthf, __lengthf2, __lengthf3, __lengthf4, __lengthd, __lengthd2, __lengthd4 )

#define maxmag(X,Y)   __CLFN_F2(X,Y,maxmag)
#define minmag(X,Y)   __CLFN_F2(X,Y,minmag)
#define mix(X,Y,Z)     __builtin_overload(3,X,Y,Z, __mixf, __mixf2, __mixf3, __mixf4, __mixf8, __mixf16,  \
                                                   __mixff2, __mixff3, __mixff4, __mixff8, __mixff16,     \
                                                   mix, __mixd2, __mixd4, __mixd8, __mixd16,              \
                                                   __mixdd2, __mixdd4, __mixdd8, __mixdd16)
#define normalize(X)   __builtin_overload( 1, X, __normalizef, __normalizef2, __normalizef3, __normalizef4, __normalized, __normalized2, __normalized4 )

#define radians(X)    __CLFN_F1(X,radians)
#define step(X,Y)     __builtin_overload(2,X,Y, __stepf, __stepf2, __stepf4, __stepf8, __stepf16,    \
                                                __stepff2, __stepff4, __stepff8, __stepff16,         \
                                                step, __stepd2, __stepd4, __stepd8, __stepd16,       \
                                                __stepdd2, __stepdd4, __stepdd8, __stepdd16 )
#define smoothstep(X,Y,Z)     __builtin_overload(3,X,Y,Z, __smoothstepf, __smoothstepf2, __smoothstepf4, __smoothstepf8, __smoothstepf16, \
                                                          __smoothstepff2, __smoothstepff4, __smoothstepff8, __smoothstepff16,            \
                                                          smoothstep, __smoothstepd2, __smoothstepd4, __smoothstepd8, __smoothstepd16,    \
                                                          __smoothstepdd2, __smoothstepdd4, __smoothstepdd8, __smoothstepdd16)
#define sign(X)                 __CLFN_F1(X,sign)

#define fast_distance(X,Y)          __builtin_overload(2,X,Y, __fast_distancef, __fast_distancef2, __fast_distancef3, __fast_distancef4 )
#define fast_length(X)          __builtin_overload(1,X, __fast_lengthf, __fast_lengthf2, __fast_lengthf3, __fast_lengthf4 )
#define fast_normalize(X)       __builtin_overload(1, X, __fast_normalizef, __fast_normalizef2, __fast_normalizef3, __fast_normalizef4)

/* Section 5.9.5 */
#define isless(X,Y)             ((X) <  (Y))
#define islessequal(X,Y)        ((X) <= (Y))
#define isgreater(X,Y)          ((X) >  (Y))
#define isgreaterequal(X,Y)     ((X) >= (Y))
#define isequal(X,Y)            ((X) == (Y))
#define isnotequal(X,Y)         ((X) != (Y))


#define islessgreater(X,Y)      __CLFN_F2(X,Y,islessgreater)

#define isfinite(X)     __CLFN_F11(X,isfinite)
#define isinf(X)        __CLFN_F11(X,isinf)
#define isnan(X)        __CLFN_F11(X,isnan)
#define isnormal(X)     __CLFN_F11(X,isnormal)
#define isordered(X,Y)     __CLFN_F2(X,Y,isordered)
#define isunordered(X,Y)     __CLFN_F21(X,Y,isunordered)
#define signbit(X)     __CLFN_F11(X,signbit)
#define any(X)      __builtin_overload(1, X,    __any_1i8, __any_2i8, __any_4i8, __any_8i8, __any_16i8, \
                                                __any_1i16, __any_2i16, __any_4i16, __any_8i16, __any_16i16, \
                                                __any_1i32, __any_2i32, __any_4i32, __any_8i32, __any_16i32, \
                                                __any_1i64, __any_2i64, __any_4i64, __any_8i64, __any_16i64 )

#define all(X)      __builtin_overload(1, X,    __all_1i8, __all_2i8, __all_4i8, __all_8i8, __all_16i8, \
												__all_1i16, __all_2i16, __all_4i16, __all_8i16, __all_16i16, \
												__all_1i32, __all_2i32, __all_4i32, __all_8i32, __all_16i32, \
												__all_1i64, __all_2i64, __all_4i64, __all_8i64, __all_16i64)

#define select(X,Y,Z)       __builtin_overload(3,X,Y,Z, __select_1i8, __select_2i8, __select_4i8, __select_8i8, __select_16i8, __select_1u8, __select_2u8, __select_4u8, __select_8u8, __select_16u8,   \
                                                        __select_1i16, __select_2i16, __select_4i16, __select_8i16, __select_16i16, __select_1u16, __select_2u16, __select_4u16, __select_8u16, __select_16u16,   \
                                                        __select_1i32, __select_2i32, __select_4i32, __select_8i32, __select_16i32, __select_1u32, __select_2u32, __select_4u32, __select_8u32, __select_16u32,   \
                                                        __select_1i64, __select_2i64, __select_4i64, __select_8i64, __select_16i64, __select_1u64, __select_2u64, __select_4u64, __select_8u64, __select_16u64,   \
                                                        __select_1i8u, __select_2i8u, __select_4i8u, __select_8i8u, __select_16i8u, __select_1u8u, __select_2u8u, __select_4u8u, __select_8u8u, __select_16u8u,   \
                                                        __select_1i16u, __select_2i16u, __select_4i16u, __select_8i16u, __select_16i16u, __select_1u16u, __select_2u16u, __select_4u16u, __select_8u16u, __select_16u16u,   \
                                                        __select_1i32u, __select_2i32u, __select_4i32u, __select_8i32u, __select_16i32u, __select_1u32u, __select_2u32u, __select_4u32u, __select_8u32u, __select_16u32u,   \
                                                        __select_1i64u, __select_2i64u, __select_4i64u, __select_8i64u, __select_16i64u, __select_1u64u, __select_2u64u, __select_4u64u, __select_8u64u, __select_16u64u,   \
                                                        __select_ffi, __select_ffi2, __select_ffi4, __select_ffi8, __select_ffi16, __select_ffu, __select_ffu2, __select_ffu4, __select_ffu8, __select_ffu16, __select_ffi3, \
                                                        __select_ddi, __select_ddi2, __select_ddi4, __select_ddi8, __select_ddi16, __select_ddu, __select_ddu2, __select_ddu4, __select_ddu8, __select_ddu16    )

#define bitselect(X,Y,Z)       __builtin_overload(3,X,Y,Z, __bitselect_1i8, __bitselect_2i8, __bitselect_4i8, __bitselect_8i8, __bitselect_16i8, __bitselect_1u8, __bitselect_2u8, __bitselect_4u8, __bitselect_8u8, __bitselect_16u8,   \
                                                        __bitselect_1i16, __bitselect_2i16, __bitselect_4i16, __bitselect_8i16, __bitselect_16i16, __bitselect_1u16, __bitselect_2u16, __bitselect_4u16, __bitselect_8u16, __bitselect_16u16,   \
                                                        __bitselect_1i32, __bitselect_2i32, __bitselect_4i32, __bitselect_8i32, __bitselect_16i32, __bitselect_1u32, __bitselect_2u32, __bitselect_4u32, __bitselect_8u32, __bitselect_16u32,   \
                                                        __bitselect_1i64, __bitselect_2i64, __bitselect_4i64, __bitselect_8i64, __bitselect_16i64, __bitselect_1u64, __bitselect_2u64, __bitselect_4u64, __bitselect_8u64, __bitselect_16u64,   \
                                                        __bitselect_f, __bitselect_f2, __bitselect_f4, __bitselect_f8, __bitselect_f16,      \
                                                        __bitselect_d, __bitselect_d2, __bitselect_d4, __bitselect_d8, __bitselect_d16 )

// Include shared types that have to be visible both here and in the framework
#include "cl_kernel_shared.h"

typedef int sampler_t;

// image format description
typedef struct _cl_image_format_t {
  unsigned int num_channels;
  unsigned int channel_order;
  unsigned int channel_data_type;
} cl_image_format_t;

typedef struct  _image2d_t  *image2d_t;
typedef struct  _image3d_t  *image3d_t;

// 5.8.4 - Image Stream Read and Write
// 2D
float4 read_2d_fi(image2d_t image, sampler_t sampler, int2 coord);
float4 read_2d_ff(image2d_t image, sampler_t sampler, float2 coord);
int4   read_2d_ii(image2d_t image, sampler_t sampler, int2 coord);
int4   read_2d_if(image2d_t image, sampler_t sampler, float2 coord);
uint4  read_2d_uii(image2d_t image, sampler_t sampler, int2 coord);
uint4  read_2d_uif(image2d_t image, sampler_t sampler, float2 coord);

// 3D
float4 read_3d_fi(image3d_t image, sampler_t sampler, int4 coord);
float4 read_3d_ff(image3d_t image, sampler_t sampler, float4 coord);
int4   read_3d_ii(image3d_t image, sampler_t sampler, int4 coord);
int4   read_3d_if(image3d_t image, sampler_t sampler, float4 coord);
uint4  read_3d_uii(image3d_t image, sampler_t sampler, int4 coord);
uint4  read_3d_uif(image3d_t image, sampler_t sampler, float4 coord);

void write_imagef(image2d_t image, int2 coord, float4 val);
void write_imagei(image2d_t image, int2 coord, int4 val);
void write_imageui(image2d_t image, int2 coord, uint4 val);
int __get_image_width_2D(image2d_t img);
int __get_image_width_3D(image3d_t img);
#define get_image_width( _img )             __builtin_overload( 1, _img, __get_image_width_2D, __get_image_width_3D )

int __get_image_height_2D(image2d_t img);
int __get_image_height_3D(image3d_t img);
#define get_image_height( _img )             __builtin_overload( 1, _img, __get_image_height_2D, __get_image_height_3D )

int get_image_depth(image3d_t img);

int __get_image_channel_data_type_2D ( image2d_t image);
int __get_image_channel_data_type_3D ( image3d_t image);
#define get_image_channel_data_type( _img )   __builtin_overload( 1, _img, __get_image_channel_data_type_2D, __get_image_channel_data_type_3D )

int __get_image_channel_order_2D ( image2d_t image);
int __get_image_channel_order_3D ( image3d_t image);
#define get_image_channel_order( _img )   __builtin_overload( 1, _img, __get_image_channel_order_2D, __get_image_channel_order_3D )

int2 __get_image_dim_2D ( image2d_t image);
int4 __get_image_dim_3D ( image3d_t image);
#define get_image_dim( _img )   __builtin_overload( 1, _img, __get_image_dim_2D, __get_image_dim_3D )


#define read_imagef(A,B,C) __builtin_overload(3,A,B,C,read_2d_fi,read_2d_ff,read_3d_fi,read_3d_ff)
#define read_imagei(A,B,C) __builtin_overload(3,A,B,C,read_2d_ii,read_2d_if,read_3d_ii,read_3d_if)
#define read_imageui(A,B,C) __builtin_overload(3,A,B,C,read_2d_uii,read_2d_uif,read_3d_uii,read_3d_uif)

#if defined( __i386__ ) || defined( __x86_64__ )
// SPI for CoreImage
void __read_transposed_imagef_resample( __rd image2d_t src, sampler_t smp, float4 x,float4 y, float4 *r,float4 *g,float4 *b,float4 *a);
#define read_transposed_imagef( _src, _smp, _x, _y, _r, _g, _b, _a )   __read_transposed_imagef_resample(_src, _smp, _x, _y, _r, _g, _b, _a )

void __read_transposed_3d_imagef_resample( __rd image3d_t src, sampler_t smp, float4 x,float4 y, float4 z, float4 *r,float4 *g,float4 *b,float4 *a);
#define read_transposed_3d_imagef( _src, _smp, _x, _y, _z, _r, _g, _b, _a )     __read_transposed_imagef_resample( _src, _smp, _x, _y, _z, _r, _g, _b, _a )

void write_transposed_imagef( __wr image2d_t dst, int x, int y, float4 r, float4 g, float4 b, float4 a);

event_t __async_work_group_stream_global_to_image( __wr image2d_t image, size_t x, size_t y, size_t count, 
                                                    const __global float4 *r, const __global float4 *g, 
                                                    const __global float4 *b, const __global float4 *a );
event_t __async_work_group_stream_constant_to_image( __wr image2d_t image, size_t x, size_t y, size_t count, 
                                                    const __constant float4 *r, const __constant float4 *g, 
                                                    const __constant float4 *b, const __constant float4 *a );
event_t __async_work_group_stream_private_to_image( __wr image2d_t image, size_t x, size_t y, size_t count, 
                                                    const __private float4 *r, const __private float4 *g, 
                                                    const __private float4 *b, const __private float4 *a );
event_t __async_work_group_stream_local_to_image( __wr image2d_t image, size_t x, size_t y, size_t count, 
                                                    const __local float4 *r, const __local float4 *g, 
                                                    const __local float4 *b, const __local float4 *a );

void __ci_clamp_toward_fast_path( const __ImageExecInfo *i, float4 *start, float4 *stride, size_t offset, size_t count );


#define __async_work_group_stream_to_image( _image, _x, _y, _count, _r, _g, _b, _a )    \
                    __builtin_overload( 8,  _image, _x, _y, _count, _r, _g, _b, _a,     \
                    __async_work_group_stream_global_to_image, __async_work_group_stream_constant_to_image, \
                    __async_work_group_stream_private_to_image, __async_work_group_stream_local_to_image )
                    
event_t	__async_work_group_stream_private_from_image( __rd image2d_t image, sampler_t sampler, float2 start, float2 stride, size_t count, 
                                                        __private float4 *r,  __private float4 *g,  __private float4 *b,  __private float4 *a );
event_t	__async_work_group_stream_global_from_image( __rd image2d_t image, sampler_t sampler, float2 start, float2 stride, size_t count, 
                                                        __global float4 *r,  __global float4 *g,  __global float4 *b,  __global float4 *a );
event_t	__async_work_group_stream_local_from_image( __rd image2d_t image, sampler_t sampler, float2 start, float2 stride, size_t count, 
                                                        __local float4 *r,  __local float4 *g,  __local float4 *b,  __local float4 *a );

#define __async_work_group_stream_from_image( _image, _sampler, _start, _stride, _count, _r, _g, _b, _a )    \
                    __builtin_overload( 9,  _image, _sampler, _start, _stride, _count, _r, _g, _b, _a,     \
                    __async_work_group_stream_private_from_image, __async_work_group_stream_global_from_image, __async_work_group_stream_local_from_image )


float16  __ci_gamma_SPI( float4 r, float4 g, float4 b, float4 y );
#endif

#define async_work_group_copy( _dst, _src, _numE, _evt )    __builtin_overload( 4, _dst, _src, _numE, _evt,                                          \
        __async_work_group_copy_g2l_1i8,   __async_work_group_copy_g2l_1u8,   __async_work_group_copy_l2g_1i8,    __async_work_group_copy_l2g_1u8,   \
        __async_work_group_copy_g2l_1i16,  __async_work_group_copy_g2l_1u16,  __async_work_group_copy_l2g_1i16,   __async_work_group_copy_l2g_1u16,  \
        __async_work_group_copy_g2l_1i32,  __async_work_group_copy_g2l_1u32,  __async_work_group_copy_l2g_1i32,   __async_work_group_copy_l2g_1u32,  \
        __async_work_group_copy_g2l_1i64,  __async_work_group_copy_g2l_1u64,  __async_work_group_copy_l2g_1i64,   __async_work_group_copy_l2g_1u64,  \
        __async_work_group_copy_g2l_f,     __async_work_group_copy_g2l_d,     __async_work_group_copy_l2g_f,      __async_work_group_copy_l2g_d,     \
        __async_work_group_copy_g2l_2i8,   __async_work_group_copy_g2l_2u8,   __async_work_group_copy_l2g_2i8,    __async_work_group_copy_l2g_2u8,   \
        __async_work_group_copy_g2l_2i16,  __async_work_group_copy_g2l_2u16,  __async_work_group_copy_l2g_2i16,   __async_work_group_copy_l2g_2u16,  \
        __async_work_group_copy_g2l_2i32,  __async_work_group_copy_g2l_2u32,  __async_work_group_copy_l2g_2i32,   __async_work_group_copy_l2g_2u32,  \
        __async_work_group_copy_g2l_2i64,  __async_work_group_copy_g2l_2u64,  __async_work_group_copy_l2g_2i64,   __async_work_group_copy_l2g_2u64,  \
        __async_work_group_copy_g2l_f2,    __async_work_group_copy_g2l_d2,    __async_work_group_copy_l2g_f2,     __async_work_group_copy_l2g_d2,    \
        __async_work_group_copy_g2l_4i8,   __async_work_group_copy_g2l_4u8,    __async_work_group_copy_l2g_4i8,   __async_work_group_copy_l2g_4u8,   \
        __async_work_group_copy_g2l_4i16,  __async_work_group_copy_g2l_4u16,   __async_work_group_copy_l2g_4i16,  __async_work_group_copy_l2g_4u16,  \
        __async_work_group_copy_g2l_4i32,  __async_work_group_copy_g2l_4u32,   __async_work_group_copy_l2g_4i32,  __async_work_group_copy_l2g_4u32,  \
        __async_work_group_copy_g2l_4i64,  __async_work_group_copy_g2l_4u64,   __async_work_group_copy_l2g_4i64,  __async_work_group_copy_l2g_4u64,  \
        __async_work_group_copy_g2l_f4,    __async_work_group_copy_g2l_d4,     __async_work_group_copy_l2g_f4,    __async_work_group_copy_l2g_d4,    \
        __async_work_group_copy_g2l_8i8,   __async_work_group_copy_g2l_8u8,    __async_work_group_copy_l2g_8i8,   __async_work_group_copy_l2g_8u8,   \
        __async_work_group_copy_g2l_8i16,  __async_work_group_copy_g2l_8u16,   __async_work_group_copy_l2g_8i16,  __async_work_group_copy_l2g_8u16,  \
        __async_work_group_copy_g2l_8i32,  __async_work_group_copy_g2l_8u32,   __async_work_group_copy_l2g_8i32,  __async_work_group_copy_l2g_8u32,  \
        __async_work_group_copy_g2l_8i64,  __async_work_group_copy_g2l_8u64,   __async_work_group_copy_l2g_8i64,  __async_work_group_copy_l2g_8u64,  \
        __async_work_group_copy_g2l_f8,    __async_work_group_copy_g2l_d8,     __async_work_group_copy_l2g_f8,    __async_work_group_copy_l2g_d8,    \
        __async_work_group_copy_g2l_16i8,  __async_work_group_copy_g2l_16u8,   __async_work_group_copy_l2g_16i8,  __async_work_group_copy_l2g_16u8,  \
        __async_work_group_copy_g2l_16i16, __async_work_group_copy_g2l_16u16,  __async_work_group_copy_l2g_16i16, __async_work_group_copy_l2g_16u16, \
        __async_work_group_copy_g2l_16i32, __async_work_group_copy_g2l_16u32,  __async_work_group_copy_l2g_16i32, __async_work_group_copy_l2g_16u32, \
        __async_work_group_copy_g2l_16i64, __async_work_group_copy_g2l_16u64,  __async_work_group_copy_l2g_16i64, __async_work_group_copy_l2g_16u64, \
        __async_work_group_copy_g2l_f16,   __async_work_group_copy_g2l_d16,    __async_work_group_copy_l2g_f16,   __async_work_group_copy_l2g_d16 )

void wait_group_events( int, event_t* );

#define prefetch( _p, _size )   __builtin_overload( 2, _p, _size,       \
        __prefetch_1i8,         __prefetch_2i8,     __prefetch_4i8,         __prefetch_8i8,         __prefetch_16i8,        \
        __prefetch_1u8,         __prefetch_2u8,     __prefetch_4u8,         __prefetch_8u8,         __prefetch_16u8,        \
        __prefetch_1i16,        __prefetch_2i16,    __prefetch_4i16,        __prefetch_8i16,        __prefetch_16i16,       \
        __prefetch_1u16,        __prefetch_2u16,    __prefetch_4u16,        __prefetch_8u16,        __prefetch_16u16,       \
        __prefetch_1i32,        __prefetch_2i32,    __prefetch_4i32,        __prefetch_8i32,        __prefetch_16i32,       \
        __prefetch_1u32,        __prefetch_2u32,    __prefetch_4u32,        __prefetch_8u32,        __prefetch_16u32,       \
        __prefetch_1i64,        __prefetch_2i64,    __prefetch_4i64,        __prefetch_8i64,        __prefetch_16i64,       \
        __prefetch_1u64,        __prefetch_2u64,    __prefetch_4u64,        __prefetch_8u64,        __prefetch_16u64,       \
        __prefetch_f,           __prefetch_f2,      __prefetch_f4,          __prefetch_f8,          __prefetch_f16,         \
        __prefetch_d,           __prefetch_d2,      __prefetch_d4,          __prefetch_d8,          __prefetch_d16  )

// 5.9.7 - Synchronization
void barrier(unsigned);

#if defined( __i386__ ) || defined( __i686__ ) || defined( __x86_64__ )
// Define as CPU intrinsics
#define mem_fence(flag)	__builtin_ia32_mfence()
#define read_mem_fence(flag)	__builtin_ia32_lfence()
#define write_mem_fence(flag)	__builtin_ia32_sfence()
#else
void mem_fence(unsigned);
void read_mem_fence(unsigned);
void write_mem_fence(unsigned);
#endif

enum {
  CLK_LOCAL_MEM_FENCE  = 1 << 0,
  CLK_GLOBAL_MEM_FENCE = 1 << 1
};

// 5.9.9 - Atomic Functions
int __atom_add_gi32(__global int *p, int val);
int __atom_sub_gi32(__global int *p, int val);
int __atom_xchg_gi32(__global int *p, int val);
int __atom_min_gi32(__global int *p, int val);
int __atom_max_gi32(__global int *p, int val);
int __atom_inc_gi32(__global int *p);
int __atom_dec_gi32(__global int *p);
int __atom_cmpxchg_gi32(__global int *p, int cmp, int val);
int __atom_and_gi32(__global int *p, int val);
int __atom_or_gi32(__global int *p, int val);
int __atom_xor_gi32(__global int *p, int val);

unsigned __atom_add_gu32(__global unsigned *p, unsigned val);
unsigned __atom_sub_gu32(__global unsigned *p, unsigned val);
unsigned __atom_xchg_gu32(__global unsigned *p, unsigned val);
unsigned __atom_min_gu32(__global unsigned *p, unsigned val);
unsigned __atom_max_gu32(__global unsigned *p, unsigned val);
unsigned __atom_inc_gu32(__global unsigned *p);
unsigned __atom_dec_gu32(__global unsigned *p);
unsigned __atom_cmpxchg_gu32(__global unsigned *p, unsigned cmp, unsigned val);
unsigned __atom_and_gu32(__global unsigned *p, unsigned val);
unsigned __atom_or_gu32(__global unsigned *p, unsigned val);
unsigned __atom_xor_gu32(__global unsigned *p, unsigned val);


int __atom_add_li32(local int *p, int val);
int __atom_sub_li32(local int *p, int val);
int __atom_xchg_li32(local int *p, int val);
int __atom_min_li32(local int *p, int val);
int __atom_max_li32(local int *p, int val);
int __atom_inc_li32(local int *p);
int __atom_dec_li32(local int *p);
int __atom_cmpxchg_li32(local int *p, int cmp, int val);
int __atom_and_li32(local int *p, int val);
int __atom_or_li32(local int *p, int val);
int __atom_xor_li32(local int *p, int val);

unsigned __atom_add_lu32(local unsigned *p, unsigned val);
unsigned __atom_sub_lu32(local unsigned *p, unsigned val);
unsigned __atom_xchg_lu32(local unsigned *p, unsigned val);
unsigned __atom_min_lu32(local unsigned *p, unsigned val);
unsigned __atom_max_lu32(local unsigned *p, unsigned val);
unsigned __atom_inc_lu32(local unsigned *p);
unsigned __atom_dec_lu32(local unsigned *p);
unsigned __atom_cmpxchg_lu32(local unsigned *p, unsigned cmp, unsigned val);
unsigned __atom_and_lu32(local unsigned *p, unsigned val);
unsigned __atom_or_lu32(local unsigned *p, unsigned val);
unsigned __atom_xor_lu32(local unsigned *p, unsigned val);


#define __CLFN_A1(x,R) __builtin_overload(1, x, \
	__atom_##R##_gi32, __atom_##R##_li32,  __atom_##R##_gu32, __atom_##R##_lu32)

#define __CLFN_A2(x,y,R) __builtin_overload(2, x, y, \
	__atom_##R##_gi32, __atom_##R##_li32, __atom_##R##_gu32, __atom_##R##_lu32)

#define __CLFN_A3(x,y,z,R) __builtin_overload(3, x, y, z, \
	__atom_##R##_gi32, __atom_##R##_li32, __atom_##R##_gu32, __atom_##R##_lu32)

#define atom_add(X,Y) __CLFN_A2(X, Y, add)
#define atom_sub(X,Y) __CLFN_A2(X, Y, sub)
#define atom_xchg(X,Y) __CLFN_A2(X, Y, xchg) 
#define atom_min(X,Y) __CLFN_A2(X, Y, min)
#define atom_max(X,Y) __CLFN_A2(X, Y, max)
#define atom_inc(X)  __CLFN_A1(X, inc)
#define atom_dec(X) __CLFN_A1(X, dec)
#define atom_cmpxchg(X,Y,Z) __CLFN_A3(X,Y,Z, cmpxchg) 
#define atom_and(X,Y) __CLFN_A2(X, Y, and)
#define atom_or(X,Y)  __CLFN_A2(X, Y, or)
#define atom_xor(X,Y) __CLFN_A2(X, Y, xor)

// Debug print
void dbg_print(const char* fmt, ...);

#endif  // __CL_KERNEL_H
