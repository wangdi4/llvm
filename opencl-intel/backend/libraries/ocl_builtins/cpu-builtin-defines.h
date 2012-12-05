// Copyright (c) 2006-2012 Intel Corporation
//

// This file contains the builtin-defines used in the CPU-optimized builtins

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// integers masks
const char char_MSB_mask =       0x80;
const int  int_MSB_mask  = 0x80000000;
const char LSB_mask      =          1;

const long long_even_mask = 0x00000000FFFFFFFF;

// "magic numbers" for popcount parallel algorithm
const int  magic_num_S[] = {1, 2, 4, 8, 16, 32};
const long magic_num_B[] = {0x5555555555555555, 0x3333333333333333, 0x0F0F0F0F0F0F0F0F,
                            0x00FF00FF00FF00FF, 0x0000FFFF0000FFFF, 0x00000000FFFFFFFF};

#ifdef __cplusplus
}
#endif
