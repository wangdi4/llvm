// Copyright (c) 2006-20011 Intel Corporation
// All rights reserved.
#ifndef MIC_DEFINES__H
#define MIC_DEFINES__H
// This file provides implementation for KNF/KNC only
#define MIC_SIMD_BYTES 64
#define MIC_SIMD_BYTES_MASK (MIC_SIMD_BYTES - 1)
#define MIC_SIMD_INT (MIC_SIMD_BYTES >> 2)

// Difference 
# define MIC_ALIGN(decl) decl __attribute__((aligned(MIC_SIMD_BYTES)))
# define ALIGN16 __attribute__((aligned(16)))
# define ALIGN64 __attribute__((aligned(64)))

typedef unsigned long ulong;

typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned int uint;

#define cast_ireg(x) (__m512i)(x) 
#define cast_dreg(x) (__m512d)(x)
#define cast_reg(x)  (__m512)(x)
#define pcast_ireg(x) (*(__m512i*)(x))
#define pcast_dreg(x) (*(__m512d*)(x))
#define pcast_reg(x)  (*(__m512*)(x))
#define vcast(x, type)  (*(type*)(&x))
#define void_const_cast(x) (void const *)(&x)
#define const_void_cast(x) (const void *)(&x)
#define void_cast(x)  (void *)(&x)
#define fp2int(x) as_int(x)
#define dp2int(x) as_long(x)

// Macro for possible intrinsic extension for load/store
#define trunc_to_s8(source, result)  _mm512_stored(void_cast(result), cast_reg(source), _MM_DOWNC_SINT8I, _MM_SUBSET32_16, _MM_HINT_NONE)
#define trunc_to_s16(source, result)  _mm512_stored(void_cast(result), cast_reg(source), _MM_DOWNC_SINT16I, _MM_SUBSET32_16, _MM_HINT_NONE)
#define trunc_to_u8(source, result)  _mm512_stored(void_cast(result), cast_reg(source), _MM_DOWNC_UINT8I, _MM_SUBSET32_16, _MM_HINT_NONE)
#define trunc_to_u16(source, result)  _mm512_stored(void_cast(result), cast_reg(source), _MM_DOWNC_UINT16I, _MM_SUBSET32_16, _MM_HINT_NONE)
#define ext_from_s8(source) _mm512_loadd((void const *)(&source), _MM_FULLUPC_SINT8I, _MM_BROADCAST32_NONE, _MM_HINT_NONE)
#define ext_from_s16(source) _mm512_loadd((void const *)(&source), _MM_FULLUPC_SINT16I, _MM_BROADCAST32_NONE, _MM_HINT_NONE)
#define ext_from_u8(source) _mm512_loadd((void const *)(&source), _MM_FULLUPC_UINT8I, _MM_BROADCAST32_NONE, _MM_HINT_NONE)
#define ext_from_u16(source) _mm512_loadd((void const *)(&source), _MM_FULLUPC_UINT16I, _MM_BROADCAST32_NONE, _MM_HINT_NONE)

#endif // MIC_DEFINES__H

