// Copyright (c) 2006-2012 Intel Corporation
//

// This file contains the builtin-defines used in the CPU-optimized builtins

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// integers masks
const constant char char_MSB_mask =       0x80;
const constant int  int_MSB_mask  = 0x80000000;
const constant char LSB_mask      =          1;

const constant long long_even_mask = 0x00000000FFFFFFFF;

// "magic numbers" for popcount parallel algorithm
const constant int  magic_num_S[] = {1, 2, 4, 8, 16, 32};
const constant long magic_num_B[] = {0x5555555555555555, 0x3333333333333333, 0x0F0F0F0F0F0F0F0F,
                                     0x00FF00FF00FF00FF, 0x0000FFFF0000FFFF, 0x00000000FFFFFFFF};


// shuffle and shuffle2
const constant uchar16 _shuffle_epi16_smask = (uchar16)(0, 0, 2, 2, 4, 4, 6, 6, 8, 8, 10, 10, 12, 12, 14, 14);
const constant uchar16 _shuffle_epi16_amask = (uchar16)(0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1);

const constant uchar16 _shuffle_epi32_smask = (uchar16)(0, 0, 0, 0, 4, 4, 4, 4, 8, 8, 8, 8, 12, 12, 12, 12);
const constant uchar16 _shuffle_epi32_amask = (uchar16)(0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3, 0, 1, 2, 3);

const constant uchar16 _shuffle_epi64_smask = (uchar16)(0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 8);
const constant uchar16 _shuffle_epi64_amask = (uchar16)(0, 1, 2, 3, 4, 5, 6, 7, 0, 1, 2, 3, 4, 5, 6, 7);


uchar16 _MM_SHUFFLE_EPI8(uchar16 x, uchar16 mask)
{
   return as_uchar16( _mm_shuffle_epi8((__m128i)x, (__m128i)mask) );
}

ushort8 _MM_SHUFFLE_EPI16(ushort8 x, ushort8 mask)
{
    mask = as_ushort8( _MM_SHUFFLE_EPI8(as_uchar16(mask), _shuffle_epi16_smask) );
    mask = as_ushort8( _mm_slli_epi16((__m128i)mask, 1) );
    mask = as_ushort8( _mm_adds_epu8((__m128i)mask, (__m128i)_shuffle_epi16_amask) );
    return as_ushort8( _MM_SHUFFLE_EPI8(as_uchar16(x), as_uchar16(mask)) );
}

uint4 _MM_SHUFFLE_EPI32(uint4 x, uint4 mask)
{
    mask = as_uint4( _MM_SHUFFLE_EPI8(as_uchar16(mask), _shuffle_epi32_smask) );
    mask = as_uint4( _mm_slli_epi16((__m128i)mask, 2) );
    mask = as_uint4( _mm_adds_epu8((__m128i)mask, (__m128i)_shuffle_epi32_amask) );
    return as_uint4( _MM_SHUFFLE_EPI8(as_uchar16(x), as_uchar16(mask)) );
}

ulong2 _MM_SHUFFLE_EPI64(ulong2 x, ulong2 mask)
{
    mask = as_ulong2( _MM_SHUFFLE_EPI8(as_uchar16(mask), _shuffle_epi64_smask) );
    mask = as_ulong2( _mm_slli_epi16((__m128i)mask, 3) );
    mask = as_ulong2( _mm_adds_epu8((__m128i)mask, (__m128i)_shuffle_epi64_amask) );
    return as_ulong2( _MM_SHUFFLE_EPI8(as_uchar16(x), as_uchar16(mask)) );
}

// conversion sat
const constant float as_float_min_char    =  -128.0f;
const constant float as_float_max_char    =   127.0f;
const constant float as_float_min_uchar   =     0.0f;
const constant float as_float_max_uchar   =   255.0f;

// cpu conversion define // TODO: what these should be called
const constant int minInt32 = 0xcf000000;
const constant int maxInt32 = 0x4f000000;

// TODO: remove this as its the same as generic_min_int
const constant int minIntVal32 = 0x80000000; //-2147483648.0
const constant int maxIntVal32 = 0x7FFFFFFF; //2147483647.0f

//vloadstore
void* memcpy(void*, const void*, size_t);

const constant short  Fvec8Float16ExponentMask = 0x7C00;//Fvec8Float16ExponentMask,Fvec4Float16NaNExpMask
const constant short  Fvec8Float16MantissaMask = 0x03FF;
const constant short  Fvec8Float16SignMask = 0x8000;

const constant int Fvec4Float32ExponentMask = 0x7F800000;
const constant int Fvec4Float32NanMask = 0x7FC00000;
const constant int FVec4Float16Implicit1Mask = (1<<10);
const constant int Fvec4Float16ExpMin = (1<<10);
const constant int Fvec4Float16BiasDiffDenorm = ((127 - 15 - 10) << 23);
const constant int Fvec4Float16ExpBiasDifference = ((127 - 15) << 10);
const constant int Fvec4Float16NaNExpMask = 0x7C00;

const constant int x7bff = 0x7bff;
const constant int x8000 = 0x8000;
const constant int x7fff = 0x7fff;
const constant int x0200 = 0x0200;
const constant int x7c00 = 0x7c00;
const constant int xfbff = 0xfbff;
const constant int xfc00 = 0xfc00;
const constant int x8001 = 0x8001;

const constant int x7fffffff = 0x7fffffff;
const constant int x7f800000 = 0x7f800000;
const constant int x47800000 = 0x47800000;
const constant int x33800000 = 0x33800000;
const constant int x38800000 = 0x38800000;
const constant int x4b800000 = 0x4b800000;
const constant int xffffe000 = 0xffffe000;
const constant int x38000000 = 0x38000000;
const constant int x477ff000 = 0x477ff000;
const constant int xc7800000 = 0xc7800000;
const constant int xff800000 = 0xff800000;
const constant int xc77fe000 = 0xc77fe000;
const constant int x00002000 = 0x00002000;
const constant int x33000000 = 0x33000000;
const constant int x33c00000 = 0x33c00000;
const constant int x01000000 = 0x01000000;
const constant int x46000000 = 0x46000000;
const constant int x07800000 = 0x07800000;

const constant long x7fffffffffffffff = 0x7fffffffffffffff;
const constant long x7ff0000000000000 = 0x7ff0000000000000;
const constant long x40f0000000000000 = 0x40f0000000000000;
const constant long x3e70000000000000 = 0x3e70000000000000;
const constant long x3f10000000000000 = 0x3f10000000000000;
const constant long x4170000000000000 = 0x4170000000000000;
const constant long xFFFFFC0000000000 = 0xFFFFFC0000000000;
const constant long x3F00000000000000 = 0x3F00000000000000;
const constant long x40effe0000000000 = 0x40effe0000000000;
const constant long x40effc0000000000 = 0x40effc0000000000;
const constant long x00f0000000000000 = 0x00f0000000000000;
const constant long x4290000000000000 = 0x4290000000000000;
const constant long x0000000001000000 = 0x0000000001000000;
const constant long x3e78000000000000 = 0x3e78000000000000;
const constant long x3e60000000000000 = 0x3e60000000000000;
const constant long x0000040000000000 = 0x0000040000000000;
const constant long xc0effc0000000000 = 0xc0effc0000000000;
const constant long xfff0000000000000 = 0xfff0000000000000;
const constant long xc0f0000000000000 = 0xc0f0000000000000;
const constant int conversion_ones = 1;
const constant long dones = 1;
const constant char16 g_vls_4x32to4x16 = {0,1, 4,5,  8,9, 12,13, 0, 0, 0, 0, 0,0,0,0};
const constant char16 g_vls_2x64to2x16 = {0,1, 8,9,0,0, 0, 0, 0, 0, 0, 0,0,0,0,0};							
#ifdef __cplusplus
}
#endif
