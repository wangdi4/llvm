// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

// This file contains the builtin-defines used in the CPU-optimized builtins

#pragma once

#pragma OPENCL EXTENSION cl_khr_int64_base_atomics : enable
#pragma OPENCL EXTENSION cl_khr_int64_extended_atomics : enable
// enable cl_khr_fp16 extension to generate atomic_[max, min] functions with
// half type returned
#pragma OPENCL EXTENSION cl_khr_fp16 : enable

#ifndef CLK_CHANNEL_MEM_FENCE
/**
 * Queue a memory fence to ensure correct
 * ordering of memory operations to channel
 **/
#define CLK_CHANNEL_MEM_FENCE 0x04
#endif

// integers masks
extern const constant char char_MSB_mask;
extern const constant int int_MSB_mask;
extern const constant char LSB_mask;

extern const constant long long_even_mask;

// "magic numbers" for popcount parallel algorithm
extern const constant int magic_num_S[6];
extern const constant long magic_num_B[6];
// relational
extern const constant short h_nan_max;
extern const constant int f_nan_max;
extern const constant long d_nan_max;
extern const constant short h_exp_mask;
extern const constant int f_exp_mask;
extern const constant long d_exp_mask;
extern const constant float fltm;
extern const constant int fsign_mask;
extern const constant long dsign_mask;
extern const constant short h_mask;
extern const constant int f_mask;
extern const constant long d_mask;

// shuffle and shuffle2
extern const constant uchar16 _shuffle_epi16_smask;
extern const constant uchar16 _shuffle_epi16_amask;

extern const constant uchar16 _shuffle_epi32_smask;
extern const constant uchar16 _shuffle_epi32_amask;

extern const constant uchar16 _shuffle_epi64_smask;
extern const constant uchar16 _shuffle_epi64_amask;

// conversion sat
extern const constant float as_float_min_char;
extern const constant float as_float_max_char;
extern const constant float as_float_min_uchar;
extern const constant float as_float_max_uchar;

// cpu conversion define // TODO: what these should be called
extern const constant int minInt32;
extern const constant int maxInt32;

// TODO: remove this as its the same as generic_min_int
extern const constant int minIntVal32; //-2147483648.0
extern const constant int maxIntVal32; // 2147483647.0f

// vloadstore
void *memcpy(void *, const void *, size_t);

extern const constant short
    Fvec8Float16ExponentMask; // Fvec8Float16ExponentMask,Fvec4Float16NaNExpMask
extern const constant short Fvec8Float16MantissaMask;
extern const constant short Fvec8Float16SignMask;

extern const constant int Fvec4Float32ExponentMask;
extern const constant int Fvec4Float32NanMask;
extern const constant int FVec4Float16Implicit1Mask;
extern const constant int Fvec4Float16ExpMin;
extern const constant int Fvec4Float16BiasDiffDenorm;
extern const constant int Fvec4Float16ExpBiasDifference;
extern const constant int Fvec4Float16NaNExpMask;

extern const constant int x7bff;
extern const constant int x8000;
extern const constant int x7fff;
extern const constant int x0200;
extern const constant int x7c00;
extern const constant int xfbff;
extern const constant int xfc00;
extern const constant int x8001;

extern const constant int x7fffffff;
extern const constant int x7f800000;
extern const constant int x47800000;
extern const constant int x33800000;
extern const constant int x38800000;
extern const constant int x4b800000;
extern const constant int xffffe000;
extern const constant int x38000000;
extern const constant int x477ff000;
extern const constant int xc7800000;
extern const constant int xff800000;
extern const constant int xc77fe000;
extern const constant int x00002000;
extern const constant int x33000000;
extern const constant int x33c00000;
extern const constant int x01000000;
extern const constant int x46000000;
extern const constant int x07800000;

extern const constant long x7fffffffffffffff;
extern const constant long x7ff0000000000000;
extern const constant long x40f0000000000000;
extern const constant long x3e70000000000000;
extern const constant long x3f10000000000000;
extern const constant long x4170000000000000;
extern const constant long xFFFFFC0000000000;
extern const constant long x3F00000000000000;
extern const constant long x40effe0000000000;
extern const constant long x40effc0000000000;
extern const constant long x00f0000000000000;
extern const constant long x4290000000000000;
extern const constant long x0000000001000000;
extern const constant long x3e78000000000000;
extern const constant long x3e60000000000000;
extern const constant long x0000040000000000;
extern const constant long xc0effc0000000000;
extern const constant long xfff0000000000000;
extern const constant long xc0f0000000000000;
extern const constant int conversion_ones;
extern const constant long dones;
extern const constant char16 g_vls_4x32to4x16;
extern const constant char16 g_vls_2x64to2x16;
