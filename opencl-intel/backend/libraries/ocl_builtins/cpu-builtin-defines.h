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


#ifdef __cplusplus
}
#endif
