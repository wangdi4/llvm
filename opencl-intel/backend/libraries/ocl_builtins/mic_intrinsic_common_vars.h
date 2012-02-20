// Copyright (c) 2006-2011 Intel Corporation
// All rights reserved.
#ifndef MIC_INTRINSIC_COMMON_VARS__H
#define MIC_INTRINSIC_COMMON_VARS__H

// Constant definitions
const unsigned char const_char_msb = 0x80;
const unsigned short const_short_msb = 0x8000;
const unsigned short const_mask_0x5555 = 0x5555;
const unsigned short const_mask_0xaaaa = 0xaaaa;
const unsigned int const_fp_mantissa = 0x7fffff;
const unsigned int const_fp_exp = 0x7f800000;
const unsigned int const_no_sign = 0x7fffffff;
const unsigned int const_msb = 0x80000000;
const ulong const_long_msb = 0x8000000000000000L;
MIC_ALIGN(const int const_vector_0_1_x8[MIC_SIMD_INT]) = { 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1};
MIC_ALIGN(const int const_vector_zeros[MIC_SIMD_INT]) ={ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
MIC_ALIGN(const int const_vector_unpack[MIC_SIMD_INT]) = { 0, 0, 1, 1, 2, 2, 3, 3, 7, 7, 5, 5, 6, 6, 7, 7};
MIC_ALIGN(const int const_vector_1x16[MIC_SIMD_INT]) = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
MIC_ALIGN(const int const_vector_2x16[MIC_SIMD_INT]) = { 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
MIC_ALIGN(const int const_vector_3x16[MIC_SIMD_INT]) = { 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3};
MIC_ALIGN(const int const_vector_4x16[MIC_SIMD_INT]) = { 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4};
MIC_ALIGN(const int const_vector_7x16[MIC_SIMD_INT]) = { 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7};
MIC_ALIGN(const int const_vector_8x16[MIC_SIMD_INT]) = { 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8};
MIC_ALIGN(const int const_vector_15x16[MIC_SIMD_INT]) = { 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15};
MIC_ALIGN(const int const_vector_16x16[MIC_SIMD_INT]) = { 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16};
MIC_ALIGN(const int const_vector_31x16[MIC_SIMD_INT]) = { 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31};
MIC_ALIGN(const int const_vector_1x8[MIC_SIMD_INT]) = { 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0};
MIC_ALIGN(const int const_vector_3x8[MIC_SIMD_INT]) = { 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0, 3, 0};
MIC_ALIGN(const int const_vector_7x8[MIC_SIMD_INT]) = { 7, 0, 7, 0, 7, 0, 7, 0, 7, 0, 7, 0, 7, 0, 7, 0};
MIC_ALIGN(const int const_vector_15x8[MIC_SIMD_INT]) = { 15, 0, 15, 0, 15, 0, 15, 0, 15, 0, 15, 0, 15, 0, 15, 0};
MIC_ALIGN(const int const_vector_31x8[MIC_SIMD_INT]) = { 31, 0, 31, 0, 31, 0, 31, 0, 31, 0, 31, 0, 31, 0, 31, 0};
MIC_ALIGN(const int const_vector_ones[MIC_SIMD_INT]) = { 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff,
                                                         0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
                                                         0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 
                                                         0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff};
MIC_ALIGN(const int const_vector_msb[MIC_SIMD_INT]) = { 0x80000000, 0x80000000, 0x80000000, 0x80000000,
                                                        0x80000000, 0x80000000, 0x80000000, 0x80000000, 
                                                        0x80000000, 0x80000000, 0x80000000, 0x80000000, 
                                                        0x80000000, 0x80000000, 0x80000000, 0x80000000};
MIC_ALIGN(const int const_vector_char_msb[MIC_SIMD_INT]) = { 0x80808080, 0x80808080, 0x80808080, 0x80808080,
                                                        0x80808080, 0x80808080, 0x80808080, 0x80808080, 
                                                        0x80808080, 0x80808080, 0x80808080, 0x80808080, 
                                                        0x80808080, 0x80808080, 0x80808080, 0x80808080};
MIC_ALIGN(const int const_vector_short_msb[MIC_SIMD_INT]) = { 0x80008000, 0x80008000, 0x80008000, 0x80008000,
                                                        0x80008000, 0x80008000, 0x80008000, 0x80008000, 
                                                        0x80008000, 0x80008000, 0x80008000, 0x80008000, 
                                                        0x80008000, 0x80008000, 0x80008000, 0x80008000};
MIC_ALIGN(const int const_vector3_msb[MIC_SIMD_INT]) = { 0x80000000, 0x80000000, 0x80000000, 0x00000000,
                                                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                                         0x00000000, 0x00000000, 0x00000000, 0x00000000};
MIC_ALIGN(const int const_vector7_msb[MIC_SIMD_INT]) = { 0x80000000, 0x80000000, 0x80000000, 0x80000000,
                                                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                                         0x00000000, 0x00000000, 0x00000000, 0x00000000};
MIC_ALIGN(const int const_vector15_msb[MIC_SIMD_INT]) = { 0x80000000, 0x80000000, 0x80000000, 0x80000000,
                                                         0x80000000, 0x80000000, 0x80000000, 0x80000000,
                                                         0x00000000, 0x00000000, 0x00000000, 0x00000000,
                                                         0x00000000, 0x00000000, 0x00000000, 0x00000000};
MIC_ALIGN(const int const_vector_nosign[MIC_SIMD_INT]) = { 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff,
                                                           0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff,
                                                           0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff,
                                                           0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff};

MIC_ALIGN(const int const_vector_inf[MIC_SIMD_INT]) = { 0x7f000000, 0x7f000000, 0x7f000000, 0x7f000000,
                                                        0x7f000000, 0x7f000000, 0x7f000000, 0x7f000000, 
                                                        0x7f000000, 0x7f000000, 0x7f000000, 0x7f000000, 
                                                        0x7f000000, 0x7f000000, 0x7f000000, 0x7f000000};

MIC_ALIGN(const int const_vector_exp[MIC_SIMD_INT]) = { 0x7f800000, 0x7f800000, 0x7f800000, 0x7f800000,
                                                        0x7f800000, 0x7f800000, 0x7f800000, 0x7f800000, 
                                                        0x7f800000, 0x7f800000, 0x7f800000, 0x7f800000, 
                                                        0x7f800000, 0x7f800000, 0x7f800000, 0x7f800000};
MIC_ALIGN(const int const_vector_man[MIC_SIMD_INT]) = { 0x007fffff, 0x007fffff, 0x007fffff, 0x007fffff,
                                                        0x007fffff, 0x007fffff, 0x007fffff, 0x007fffff, 
                                                        0x007fffff, 0x007fffff, 0x007fffff, 0x007fffff, 
                                                        0x007fffff, 0x007fffff, 0x007fffff, 0x007fffff};
MIC_ALIGN(const int const_vector_long_msb[MIC_SIMD_INT]) = { 0x0, 0x80000000, 0x0, 0x80000000,
                                                             0x0, 0x80000000, 0x0, 0x80000000,
                                                             0x0, 0x80000000, 0x0, 0x80000000,
                                                             0x0, 0x80000000, 0x0, 0x80000000};
MIC_ALIGN(const int const_vector3_long_msb[MIC_SIMD_INT]) = { 0x0, 0x80000000, 0x0, 0x80000000,
                                                              0x0, 0x80000000, 0x0, 0x00000000,
                                                              0x0, 0x00000000, 0x0, 0x00000000,
                                                              0x0, 0x00000000, 0x0, 0x00000000};
MIC_ALIGN(const int const_vector7_long_msb[MIC_SIMD_INT]) = { 0x0, 0x80000000, 0x0, 0x80000000,
                                                              0x0, 0x80000000, 0x0, 0x80000000,
                                                              0x0, 0x80000000, 0x0, 0x80000000,
                                                              0x0, 0x80000000, 0x0, 0x00000000};
#endif // MIC_INTRINSIC_COMMON_VARS__H
