/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  oclbuiltin_d.cl


\*****************************************************************************/

#define IN_VARS_A                                                              \
  double a_in;                                                                 \
  double2 a2_in;                                                               \
  double3 a3_in;                                                               \
  double4 a4_in;                                                               \
  double8 a8_in;                                                               \
  double16 a16_in;
#define IN_VARS_B                                                              \
  double b_in;                                                                 \
  double2 b2_in;                                                               \
  double3 b3_in;                                                               \
  double4 b4_in;                                                               \
  double8 b8_in;                                                               \
  double16 b16_in;
#define IN_VARS_C                                                              \
  double c_in;                                                                 \
  double2 c2_in;                                                               \
  double3 c3_in;                                                               \
  double4 c4_in;                                                               \
  double8 c8_in;                                                               \
  double16 c16_in;

#define OUT_VARS                                                               \
  double a_out;                                                                \
  double2 a2_out;                                                              \
  double3 a3_out;                                                              \
  double4 a4_out;                                                              \
  double8 a8_out;                                                              \
  double16 a16_out;

#define SET_IN_ONEARG(_idx)                                                    \
  a_in = input[_idx];                                                          \
  a2_in.s0 = a_in;                                                             \
  a2_in.s1 = input[_idx + 1];                                                  \
  a3_in.s01 = a2_in;                                                           \
  a3_in.s2 = input[_idx + 2];                                                  \
  a4_in.s012 = a3_in;                                                          \
  a4_in.s3 = input[_idx + 3];                                                  \
  a8_in.lo = a4_in;                                                            \
  a8_in.s4 = input[_idx + 4];                                                  \
  a8_in.s5 = input[_idx + 5];                                                  \
  a8_in.s6 = input[_idx + 6];                                                  \
  a8_in.s7 = input[_idx + 7];                                                  \
  a16_in.lo = a8_in;                                                           \
  a16_in.s8 = input[_idx + 8];                                                 \
  a16_in.s9 = input[_idx + 9];                                                 \
  a16_in.sA = input[_idx + 10];                                                \
  a16_in.sB = input[_idx + 11];                                                \
  a16_in.sC = input[_idx + 12];                                                \
  a16_in.sD = input[_idx + 13];                                                \
  a16_in.sE = input[_idx + 14];                                                \
  a16_in.sF = input[_idx + 15];

#define CALL_BI_ONEARG(_func)                                                  \
  a_out = _func(a_in);                                                         \
  a2_out = _func(a2_in);                                                       \
  a3_out = _func(a3_in);                                                       \
  a4_out = _func(a4_in);                                                       \
  a8_out = _func(a8_in);                                                       \
  a16_out = _func(a16_in);

#define OUTPUT_ONE_VEC_FLOAT_UPTO_4(_idx)                                      \
  output[_idx] = a_out;                                                        \
  output[_idx + 1] = a2_out.s0;                                                \
  output[_idx + 2] = a2_out.s1;                                                \
  output[_idx + 3] = a3_out.s0;                                                \
  output[_idx + 4] = a3_out.s1;                                                \
  output[_idx + 5] = a3_out.s2;                                                \
  output[_idx + 6] = a4_out.s0;                                                \
  output[_idx + 7] = a4_out.s1;                                                \
  output[_idx + 8] = a4_out.s2;                                                \
  output[_idx + 9] = a4_out.s3;

#define OUTPUT_ONE_VEC_FLOAT(_idx)                                             \
  OUTPUT_ONE_VEC_FLOAT_UPTO_4(_idx)                                            \
  output[_idx + 10] = a8_out.s0;                                               \
  output[_idx + 11] = a8_out.s1;                                               \
  output[_idx + 12] = a8_out.s2;                                               \
  output[_idx + 13] = a8_out.s3;                                               \
  output[_idx + 14] = a8_out.s4;                                               \
  output[_idx + 15] = a8_out.s5;                                               \
  output[_idx + 16] = a8_out.s6;                                               \
  output[_idx + 17] = a8_out.s7;                                               \
  output[_idx + 18] = a16_out.s0;                                              \
  output[_idx + 19] = a16_out.s1;                                              \
  output[_idx + 20] = a16_out.s2;                                              \
  output[_idx + 21] = a16_out.s3;                                              \
  output[_idx + 22] = a16_out.s4;                                              \
  output[_idx + 23] = a16_out.s5;                                              \
  output[_idx + 24] = a16_out.s6;                                              \
  output[_idx + 25] = a16_out.s7;                                              \
  output[_idx + 26] = a16_out.s8;                                              \
  output[_idx + 27] = a16_out.s9;                                              \
  output[_idx + 28] = a16_out.sA;                                              \
  output[_idx + 29] = a16_out.sB;                                              \
  output[_idx + 30] = a16_out.sC;                                              \
  output[_idx + 31] = a16_out.sD;                                              \
  output[_idx + 32] = a16_out.sE;                                              \
  output[_idx + 33] = a16_out.sF;

#define KERNEL_BI_ONEARG(_func)                                                \
  __kernel void _func##_d(__global double *input, __global int *input_int,     \
                          __global double *output, __global double *output2) { \
    IN_VARS_A                                                                  \
    OUT_VARS                                                                   \
    uint tid = 0;                                                              \
    SET_IN_ONEARG(tid)                                                         \
    CALL_BI_ONEARG(_func)                                                      \
    OUTPUT_ONE_VEC_FLOAT(tid)                                                  \
  }

// the second input argument of function is a copy of the first argument
#define SET_IN_TWOARGS(_idx)                                                   \
  SET_IN_ONEARG(_idx);                                                         \
  b_in = a_in;                                                                 \
  b2_in = a2_in;                                                               \
  b3_in = a3_in;                                                               \
  b4_in = a4_in;                                                               \
  b8_in = a8_in;                                                               \
  b16_in = a16_in;

#define CALL_BI_TWOARGS(_func)                                                 \
  a_out = _func(a_in, b_in);                                                   \
  a2_out = _func(a2_in, b2_in);                                                \
  a3_out = _func(a3_in, b3_in);                                                \
  a4_out = _func(a4_in, b4_in);                                                \
  a8_out = _func(a8_in, b8_in);                                                \
  a16_out = _func(a16_in, b16_in);

#define KERNEL_BI_TWOARGS(_func)                                               \
  __kernel void _func##_d(__global double *input, __global int *input_int,     \
                          __global double *output, __global double *output2) { \
    IN_VARS_A                                                                  \
    IN_VARS_B                                                                  \
    OUT_VARS                                                                   \
    uint tid = 0;                                                              \
    SET_IN_TWOARGS(tid)                                                        \
    CALL_BI_TWOARGS(_func)                                                     \
    OUTPUT_ONE_VEC_FLOAT(tid)                                                  \
  }

// the second input argument of function is a copy of the first argument
#define SET_IN_THREEARGS(_idx)                                                 \
  SET_IN_TWOARGS(_idx);                                                        \
  c_in = a_in;                                                                 \
  c2_in = a2_in;                                                               \
  c3_in = a3_in;                                                               \
  c4_in = a4_in;                                                               \
  c8_in = a8_in;                                                               \
  c16_in = a16_in;

#define CALL_BI_THREEARGS(_func)                                               \
  a_out = _func(a_in, b_in, c_in);                                             \
  a2_out = _func(a2_in, b2_in, c2_in);                                         \
  a3_out = _func(a3_in, b3_in, c3_in);                                         \
  a4_out = _func(a4_in, b4_in, c4_in);                                         \
  a8_out = _func(a8_in, b8_in, c8_in);                                         \
  a16_out = _func(a16_in, b16_in, c16_in);

#define KERNEL_BI_THREEARGS(_func)                                             \
  __kernel void _func##_d(__global double *input, __global int *input_int,     \
                          __global double *output, __global double *output2) { \
    IN_VARS_A                                                                  \
    IN_VARS_B                                                                  \
    IN_VARS_C                                                                  \
    OUT_VARS                                                                   \
    uint tid = 0;                                                              \
    SET_IN_THREEARGS(tid)                                                      \
    CALL_BI_THREEARGS(_func)                                                   \
    OUTPUT_ONE_VEC_FLOAT(tid)                                                  \
  }

#define KERNEL_BI_MINMAX(_func)                                                \
  __kernel void _func##_s_d(__global double *input, __global int *input_int,   \
                            __global double *output,                           \
                            __global double *output2) {                        \
    IN_VARS_A                                                                  \
    OUT_VARS                                                                   \
    double b_in = 0.5;                                                         \
    uint tid = 0;                                                              \
    SET_IN_ONEARG(tid)                                                         \
    a2_out = _func(a2_in, b_in);                                               \
    a3_out = _func(a3_in, b_in);                                               \
    a4_out = _func(a4_in, b_in);                                               \
    a8_out = _func(a8_in, b_in);                                               \
    a16_out = _func(a16_in, b_in);                                             \
    OUTPUT_ONE_VEC_FLOAT(tid)                                                  \
  }

#define KERNEL_BI_GEOM_ONEARG(_func)                                           \
  __kernel void _func##_d(__global double *input, __global int *input_int,     \
                          __global double *output, __global double *output2) { \
    double a_in;                                                               \
    double2 a2_in;                                                             \
    double3 a3_in;                                                             \
    double4 a4_in;                                                             \
    uint tid = 0;                                                              \
    a_in = input[tid];                                                         \
    a2_in.s0 = a_in;                                                           \
    a2_in.s1 = input[tid + 1];                                                 \
    a3_in.s01 = a2_in;                                                         \
    a3_in.s2 = input[tid + 2];                                                 \
    a4_in.s012 = a3_in;                                                        \
    a4_in.s3 = input[tid + 3];                                                 \
    output[tid] = _func(a_in);                                                 \
    output[tid + 1] = _func(a2_in);                                            \
    output[tid + 2] = _func(a3_in);                                            \
    output[tid + 3] = _func(a4_in);                                            \
  }

#define KERNEL_BI_GEOM_TWOARGS(_func)                                          \
  __kernel void _func##_d(__global double *input, __global int *input_int,     \
                          __global double *output, __global double *output2) { \
    double a_in;                                                               \
    double2 a2_in;                                                             \
    double3 a3_in;                                                             \
    double4 a4_in;                                                             \
    double b_in;                                                               \
    double2 b2_in;                                                             \
    double3 b3_in;                                                             \
    double4 b4_in;                                                             \
    uint tid = 0;                                                              \
    a_in = input[tid];                                                         \
    a2_in.s0 = a_in;                                                           \
    a2_in.s1 = input[tid + 1];                                                 \
    a3_in.s01 = a2_in;                                                         \
    a3_in.s2 = input[tid + 2];                                                 \
    a4_in.s012 = a3_in;                                                        \
    a4_in.s3 = input[tid + 3];                                                 \
    b_in = a_in;                                                               \
    b2_in = a2_in;                                                             \
    b3_in = a3_in;                                                             \
    b4_in = a4_in;                                                             \
    output[tid] = _func(a_in, b_in);                                           \
    output[tid + 1] = _func(a2_in, b2_in);                                     \
    output[tid + 2] = _func(a3_in, b3_in);                                     \
    output[tid + 3] = _func(a4_in, b4_in);                                     \
  }

#define KERNEL_BI_NORMALIZE(_func)                                             \
  __kernel void _func##_d(__global double *input, __global int *input_int,     \
                          __global double *output, __global double *output2) { \
    double a_in;                                                               \
    double2 a2_in;                                                             \
    double3 a3_in;                                                             \
    double4 a4_in;                                                             \
    double a_out;                                                              \
    double2 a2_out;                                                            \
    double3 a3_out;                                                            \
    double4 a4_out;                                                            \
    uint tid = 0;                                                              \
    a_in = input[tid];                                                         \
    a2_in.s0 = a_in;                                                           \
    a2_in.s1 = input[tid + 1];                                                 \
    a3_in.s01 = a2_in;                                                         \
    a3_in.s2 = input[tid + 2];                                                 \
    a4_in.s012 = a3_in;                                                        \
    a4_in.s3 = input[tid + 3];                                                 \
    a_out = _func(a_in);                                                       \
    a2_out = _func(a2_in);                                                     \
    a3_out = _func(a3_in);                                                     \
    a4_out = _func(a4_in);                                                     \
    OUTPUT_ONE_VEC_FLOAT_UPTO_4(tid)                                           \
  }

#define KERNEL_BI_SINGLE_POW(_func)                                            \
  __kernel void _func##_s_d(__global double *input, __global int *input_int,   \
                            __global double *output,                           \
                            __global double *output2) {                        \
    IN_VARS_A                                                                  \
    int i_in = 3;                                                              \
    int2 i2_in = 3;                                                            \
    int3 i3_in = 3;                                                            \
    int4 i4_in = 3;                                                            \
    int8 i8_in = 3;                                                            \
    int16 i16_in = 3;                                                          \
    OUT_VARS                                                                   \
    uint tid = 0;                                                              \
    SET_IN_ONEARG(tid)                                                         \
    a_out = _func(a_in, i_in);                                                 \
    a2_out = _func(a2_in, i2_in);                                              \
    a3_out = _func(a3_in, i3_in);                                              \
    a4_out = _func(a4_in, i4_in);                                              \
    a8_out = _func(a8_in, i8_in);                                              \
    a16_out = _func(a16_in, i16_in);                                           \
    OUTPUT_ONE_VEC_FLOAT(tid)                                                  \
  }

#define KERNEL_BI_SINGLE_LDEXP(_func)                                          \
  __kernel void _func##_s_d(__global double *input, __global int *input_int,   \
                            __global double *output,                           \
                            __global double *output2) {                        \
    IN_VARS_A                                                                  \
    int i_in = 3;                                                              \
    OUT_VARS                                                                   \
    uint tid = 0;                                                              \
    SET_IN_ONEARG(tid)                                                         \
    a_out = _func(a_in, i_in);                                                 \
    a2_out = _func(a2_in, i_in);                                               \
    a3_out = _func(a3_in, i_in);                                               \
    a4_out = _func(a4_in, i_in);                                               \
    a8_out = _func(a8_in, i_in);                                               \
    a16_out = _func(a16_in, i_in);                                             \
    OUTPUT_ONE_VEC_FLOAT(tid)                                                  \
  }

#define KERNEL_BI_FOUT_FIN_IIN(_func)                                          \
  __kernel void _func##_d(__global double *input, __global int *input_int,     \
                          __global double *output, __global double *output2) { \
    IN_VARS_A                                                                  \
    int i_in;                                                                  \
    int2 i2_in;                                                                \
    int3 i3_in;                                                                \
    int4 i4_in;                                                                \
    int8 i8_in;                                                                \
    int16 i16_in;                                                              \
    OUT_VARS                                                                   \
    uint tid = 0;                                                              \
    SET_IN_ONEARG(tid)                                                         \
    i_in = input_int[tid];                                                     \
    i2_in.s0 = i_in;                                                           \
    i2_in.s1 = input_int[tid + 1];                                             \
    i3_in.s01 = i2_in;                                                         \
    i3_in.s2 = input_int[tid + 2];                                             \
    i4_in.s012 = i3_in;                                                        \
    i4_in.s3 = input_int[tid + 3];                                             \
    i8_in.lo = i4_in;                                                          \
    i8_in.s4 = input_int[tid + 4];                                             \
    i8_in.s5 = input_int[tid + 5];                                             \
    i8_in.s6 = input_int[tid + 6];                                             \
    i8_in.s7 = input_int[tid + 7];                                             \
    i16_in.lo = i8_in;                                                         \
    i16_in.s8 = input_int[tid + 8];                                            \
    i16_in.s9 = input_int[tid + 9];                                            \
    i16_in.sA = input_int[tid + 10];                                           \
    i16_in.sB = input_int[tid + 11];                                           \
    i16_in.sC = input_int[tid + 12];                                           \
    i16_in.sD = input_int[tid + 13];                                           \
    i16_in.sE = input_int[tid + 14];                                           \
    i16_in.sF = input_int[tid + 15];                                           \
    a_out = _func(a_in, i_in);                                                 \
    a2_out = _func(a2_in, i2_in);                                              \
    a3_out = _func(a3_in, i3_in);                                              \
    a4_out = _func(a4_in, i4_in);                                              \
    a8_out = _func(a8_in, i8_in);                                              \
    a16_out = _func(a16_in, i16_in);                                           \
    OUTPUT_ONE_VEC_FLOAT(tid)                                                  \
  }

#define KERNEL_BI_TWOOUTARGS(_func)                                            \
  __kernel void _func##_d(__global double *input, __global int *input_int,     \
                          __global double *output, __global double *output2) { \
    IN_VARS_A                                                                  \
    OUT_VARS                                                                   \
    double b_out;                                                              \
    double2 b2_out;                                                            \
    double3 b3_out;                                                            \
    double4 b4_out;                                                            \
    double8 b8_out;                                                            \
    double16 b16_out;                                                          \
    uint tid = 0;                                                              \
    SET_IN_ONEARG(tid)                                                         \
    a_out = _func(a_in, &b_out);                                               \
    a2_out = _func(a2_in, &b2_out);                                            \
    a3_out = _func(a3_in, &b3_out);                                            \
    a4_out = _func(a4_in, &b4_out);                                            \
    a8_out = _func(a8_in, &b8_out);                                            \
    a16_out = _func(a16_in, &b16_out);                                         \
    OUTPUT_ONE_VEC_FLOAT(tid)                                                  \
    output2[tid] = b_out;                                                      \
    output2[tid + 1] = b2_out.s0;                                              \
    output2[tid + 2] = b2_out.s1;                                              \
    output2[tid + 3] = b3_out.s0;                                              \
    output2[tid + 4] = b3_out.s1;                                              \
    output2[tid + 5] = b3_out.s2;                                              \
    output2[tid + 6] = b4_out.s0;                                              \
    output2[tid + 7] = b4_out.s1;                                              \
    output2[tid + 8] = b4_out.s2;                                              \
    output2[tid + 9] = b4_out.s3;                                              \
    output2[tid + 10] = b8_out.s0;                                             \
    output2[tid + 11] = b8_out.s1;                                             \
    output2[tid + 12] = b8_out.s2;                                             \
    output2[tid + 13] = b8_out.s3;                                             \
    output2[tid + 14] = b8_out.s4;                                             \
    output2[tid + 15] = b8_out.s5;                                             \
    output2[tid + 16] = b8_out.s6;                                             \
    output2[tid + 17] = b8_out.s7;                                             \
    output2[tid + 18] = b16_out.s0;                                            \
    output2[tid + 19] = b16_out.s1;                                            \
    output2[tid + 20] = b16_out.s2;                                            \
    output2[tid + 21] = b16_out.s3;                                            \
    output2[tid + 22] = b16_out.s4;                                            \
    output2[tid + 23] = b16_out.s5;                                            \
    output2[tid + 24] = b16_out.s6;                                            \
    output2[tid + 25] = b16_out.s7;                                            \
    output2[tid + 26] = b16_out.s8;                                            \
    output2[tid + 27] = b16_out.s9;                                            \
    output2[tid + 28] = b16_out.sA;                                            \
    output2[tid + 29] = b16_out.sB;                                            \
    output2[tid + 30] = b16_out.sC;                                            \
    output2[tid + 31] = b16_out.sD;                                            \
    output2[tid + 32] = b16_out.sE;                                            \
    output2[tid + 33] = b16_out.sF;                                            \
  }

#define KERNEL_BI_FREXP(_func)                                                 \
  __kernel void _func##_d(__global double *input, __global int *input_int,     \
                          __global double *output, __global double *output2) { \
    IN_VARS_A                                                                  \
    OUT_VARS                                                                   \
    int i_out;                                                                 \
    int2 i2_out;                                                               \
    int3 i3_out;                                                               \
    int4 i4_out;                                                               \
    int8 i8_out;                                                               \
    int16 i16_out;                                                             \
    uint tid = 0;                                                              \
    SET_IN_ONEARG(tid)                                                         \
    a_out = _func(a_in, &i_out);                                               \
    a2_out = _func(a2_in, &i2_out);                                            \
    a3_out = _func(a3_in, &i3_out);                                            \
    a4_out = _func(a4_in, &i4_out);                                            \
    a8_out = _func(a8_in, &i8_out);                                            \
    a16_out = _func(a16_in, &i16_out);                                         \
    OUTPUT_ONE_VEC_FLOAT(tid)                                                  \
  }

// used for temporary disable functions
#define KERNEL_DUMMY(_func)                                                    \
  __kernel void _func##_d(__global double *input, __global int *input_int,     \
                          __global double *output, __global double *output2) { \
  }

KERNEL_BI_ONEARG(acos)
KERNEL_BI_ONEARG(acospi)
KERNEL_BI_ONEARG(asin)
KERNEL_BI_ONEARG(asinpi)
KERNEL_BI_ONEARG(atan)
KERNEL_BI_TWOARGS(atan2)
KERNEL_BI_TWOARGS(atan2pi)
KERNEL_BI_ONEARG(atanpi)
KERNEL_BI_ONEARG(cos)
KERNEL_BI_ONEARG(cosh)
KERNEL_BI_ONEARG(cospi)
KERNEL_BI_ONEARG(exp)
KERNEL_BI_ONEARG(exp2)
KERNEL_BI_ONEARG(exp10)
KERNEL_BI_ONEARG(expm1)
KERNEL_BI_ONEARG(log)
KERNEL_BI_ONEARG(log2)
KERNEL_BI_ONEARG(log10)
KERNEL_BI_ONEARG(log1p)
KERNEL_BI_ONEARG(logb)

KERNEL_BI_ONEARG(ceil)
KERNEL_BI_TWOARGS(pow)

KERNEL_BI_THREEARGS(
    clamp) // gentype clamp (gentype x, gentype minval, gentype maxval)

// gentype clamp (gentype x, sgentype minval, sgentype maxval)
__kernel void clamp_s_d(__global double *input, __global int *input_int,
                        __global double *output, __global double *output2) {
  IN_VARS_A
  double b_in, c_in;
  OUT_VARS
  uint tid = 0;
  SET_IN_ONEARG(tid)
  b_in = 0.25; // set minval
  c_in = 0.55; // set maxval
  a_out = clamp(a_in, b_in, c_in);
  a2_out = clamp(a2_in, b_in, c_in);
  a3_out = clamp(a3_in, b_in, c_in);
  a4_out = clamp(a4_in, b_in, c_in);
  a8_out = clamp(a8_in, b_in, c_in);
  a16_out = clamp(a16_in, b_in, c_in);
  OUTPUT_ONE_VEC_FLOAT(tid)
}

KERNEL_BI_ONEARG(sinh)
KERNEL_BI_ONEARG(sin)
KERNEL_BI_ONEARG(sinpi)
KERNEL_BI_ONEARG(sqrt)
KERNEL_BI_ONEARG(rsqrt)
KERNEL_BI_ONEARG(tan)
KERNEL_BI_ONEARG(tanh)
KERNEL_BI_ONEARG(tanpi)
KERNEL_BI_ONEARG(fabs)

KERNEL_BI_ONEARG(asinh)
KERNEL_BI_ONEARG(acosh)
KERNEL_BI_ONEARG(atanh)

__kernel void vload_d(__global double *input, __global int *input_int,
                      __global double *output, __global double *output2) {
  OUT_VARS
  uint tid = 0;
  a2_out = vload2(0, input);
  a3_out = vload3(0, input);
  a4_out = vload4(0, input);
  a8_out = vload8(0, input);
  a16_out = vload16(0, input);
  OUTPUT_ONE_VEC_FLOAT(tid)
}

__kernel void vstore_d(__global double *input, __global int *input_int,
                       __global double *output, __global double *output2) {
  IN_VARS_A
  OUT_VARS
  uint tid = 0;
  SET_IN_ONEARG(tid)
  vstore2(a2_in, 0, (double *)&a2_out);
  vstore3(a3_in, 0, (double *)&a3_out);
  vstore4(a4_in, 0, (double *)&a4_out);
  vstore8(a8_in, 0, (double *)&a8_out);
  vstore16(a16_in, 0, (double *)&a16_out);
  OUTPUT_ONE_VEC_FLOAT(tid)
}

KERNEL_BI_TWOARGS(min)
KERNEL_BI_MINMAX(min)
KERNEL_BI_TWOARGS(max)
KERNEL_BI_MINMAX(max)
KERNEL_BI_TWOARGS(hypot)

KERNEL_BI_TWOARGS(step) // gentype step (gentype edge, gentype x)

__kernel void
step_s_d(__global double *input, __global int *input_int,
         __global double *output,
         __global double *output2) // gentypef step (double edge, gentypef x)
{
  IN_VARS_A
  OUT_VARS
  uint tid = 0;
  double edge = 0.5;
  SET_IN_ONEARG(tid)
  a_out = step(edge, a_in);
  a2_out = step(edge, a2_in);
  a3_out = step(edge, a3_in);
  a4_out = step(edge, a4_in);
  a8_out = step(edge, a8_in);
  a16_out = step(edge, a16_in);
  OUTPUT_ONE_VEC_FLOAT(tid)
}
KERNEL_BI_THREEARGS(
    smoothstep) // gentype smoothstep (gentype edge0,gentype edge1,gentype x)

__kernel void smoothstep_s_d(
    __global double *input, __global int *input_int, __global double *output,
    __global double
        *output2) // gentype smoothstep (gentype edge0,gentype edge1,gentype x)
{
  IN_VARS_A
  OUT_VARS
  uint tid = 0;
  double edge0 = 0.25;
  double edge1 = 0.55;
  SET_IN_ONEARG(tid)
  a_out = smoothstep(edge0, edge1, a_in);
  a2_out = smoothstep(edge0, edge1, a2_in);
  a3_out = smoothstep(edge0, edge1, a3_in);
  a4_out = smoothstep(edge0, edge1, a4_in);
  a8_out = smoothstep(edge0, edge1, a8_in);
  a16_out = smoothstep(edge0, edge1, a16_in);
  OUTPUT_ONE_VEC_FLOAT(tid)
}

KERNEL_BI_ONEARG(radians)
KERNEL_BI_ONEARG(degrees)
KERNEL_BI_ONEARG(sign)
KERNEL_BI_ONEARG(floor)
KERNEL_BI_GEOM_TWOARGS(dot)
KERNEL_BI_THREEARGS(mix) // gentype mix (gentype x,gentype y, gentype a)

__kernel void mix_s_d(
    __global double *input, __global int *input_int, __global double *output,
    __global double *output2) // gentypef mix (gentypef x,gentypef y, double a)
{
  IN_VARS_A
  IN_VARS_B
  double c_in = 0.5;
  OUT_VARS
  uint tid = 0;
  a2_out = mix(a2_in, b2_in, c_in);
  a3_out = mix(a3_in, b3_in, c_in);
  a4_out = mix(a4_in, b4_in, c_in);
  a8_out = mix(a8_in, b8_in, c_in);
  a16_out = mix(a16_in, b16_in, c_in);
  OUTPUT_ONE_VEC_FLOAT(tid)
}

KERNEL_BI_NORMALIZE(normalize)

__kernel void
cross_d(__global double *input, __global int *input_int,
        __global double *output,
        __global double *output2) // gentypef step (double edge, gentypef x)
{
  double3 a3_in, b3_in, a3_out;
  double4 a4_in, b4_in, a4_out;
  uint tid = 0;
  a3_in.s0 = input[tid];
  a3_in.s1 = input[tid + 1];
  a3_in.s2 = input[tid + 2];
  a4_in.s012 = a3_in;
  a4_in.s3 = input[tid + 3];
  b3_in.s0 = input[tid + 4];
  b3_in.s1 = input[tid + 5];
  b3_in.s2 = input[tid + 6];
  b4_in.s012 = b3_in;
  b4_in.s3 = input[tid + 7];
  a3_out = cross(a3_in, b3_in);
  a4_out = cross(a4_in, b4_in);
  output[tid] = a3_out.s0;
  output[tid + 1] = a3_out.s1;
  output[tid + 2] = a3_out.s2;
  output[tid + 3] = a4_out.s0;
  output[tid + 4] = a4_out.s1;
  output[tid + 5] = a4_out.s2;
  output[tid + 6] = a4_out.s3;
}

KERNEL_BI_GEOM_ONEARG(length)
KERNEL_BI_GEOM_TWOARGS(distance)
/*
disabled until CSSD100014650 will be fixed
__kernel void convert_double_ulong_d(__global double * input, __global int *
input_int,
                                   __global double * output, __global double *
output2)
{
    OUT_VARS
    uint tid = 0;
    ulong in = 3;
    a_out = convert_double(in); a2_out = convert_double2((ulong2)in);
    a3_out = convert_double3((ulong3)in); a4_out = convert_double4((ulong4)in);
    a8_out = convert_double8((ulong8)in);  a16_out =
convert_double16((ulong16)in);

    OUTPUT_ONE_VEC_FLOAT(tid)
}

__kernel void convert_double_long_d(__global double * input, __global int *
input_int,
                                  __global double * output, __global double *
output2)
{
    OUT_VARS
    uint tid = 0;
    long in = 3;
    a_out = convert_double(in); a2_out = convert_double2((long2)in);
    a3_out = convert_double3((long3)in); a4_out = convert_double4((long4)in);
    a8_out = convert_double8((long8)in); a16_out = convert_double16((long16)in);

    OUTPUT_ONE_VEC_FLOAT(tid)
}
*/

KERNEL_BI_FOUT_FIN_IIN(rootn)
KERNEL_BI_FOUT_FIN_IIN(ldexp) // doublen ldexp (doublen x, intn k)
KERNEL_BI_SINGLE_LDEXP(ldexp) // doublen ldexp (doublen x, int k)

KERNEL_BI_TWOOUTARGS(modf)
KERNEL_BI_FREXP(frexp)

KERNEL_BI_TWOARGS(maxmag)
KERNEL_BI_TWOARGS(minmag)
KERNEL_BI_TWOARGS(copysign)
KERNEL_BI_TWOARGS(nextafter)
KERNEL_BI_TWOARGS(fdim)
KERNEL_BI_THREEARGS(fma)
KERNEL_BI_THREEARGS(mad)
KERNEL_BI_ONEARG(rint)
KERNEL_BI_ONEARG(round)
KERNEL_BI_ONEARG(trunc)
KERNEL_BI_ONEARG(cbrt)
KERNEL_BI_TWOARGS(powr)
KERNEL_BI_TWOARGS(fmod)
KERNEL_BI_TWOARGS(fmin) // gentype fmin (gentype x, gentype y)
KERNEL_BI_TWOARGS(fmax) // gentype fmax (gentype x, gentype y)
KERNEL_BI_MINMAX(fmin)  // gentype fmax (gentype x, double y)
KERNEL_BI_MINMAX(fmax)  // gentype fmax (gentype x, double y)
KERNEL_BI_SINGLE_POW(pown)

__kernel void ilogb_d(__global double *input, __global int *input_int,
                      __global double *output, __global double *output2) {
  IN_VARS_A
  int i_out;
  int2 i2_out;
  int3 i3_out;
  int4 i4_out;
  int8 i8_out;
  int16 i16_out;
  uint tid = 0;
  SET_IN_ONEARG(tid)
  i_out = ilogb(a_in);
  i2_out = ilogb(a2_in);
  i3_out = ilogb(a3_in);
  i4_out = ilogb(a4_in);
  i8_out = ilogb(a8_in);
  i16_out = ilogb(a16_in);
}

__kernel void nan_d(__global double *input, __global int *input_int,
                    __global double *output, __global double *output2) {
  ulong ui_in = 0;
  ulong2 ui2_in = 0;
  ulong3 ui3_in = 0;
  ulong4 ui4_in = 0;
  ulong8 ui8_in = 0;
  ulong16 ui16_in = 0;
  uint tid = 0;
  OUT_VARS
  a_out = nan(ui_in);
  a2_out = nan(ui2_in);
  a3_out = nan(ui3_in);
  a4_out = nan(ui4_in);
  a8_out = nan(ui8_in);
  a16_out = nan(ui16_in);
  OUTPUT_ONE_VEC_FLOAT(tid)
}

KERNEL_BI_TWOOUTARGS(fract)
KERNEL_BI_ONEARG(lgamma)
KERNEL_BI_FREXP(lgamma_r)

KERNEL_BI_THREEARGS(bitselect)

__kernel void select_d(__global double *input, __global int *input_int,
                       __global double *output, __global double *output2) {
  IN_VARS_A
  IN_VARS_B
  OUT_VARS
  uint tid = 0;
  SET_IN_TWOARGS(tid)

  long l_in = 0;
  long2 l2_in = 0;
  long3 l3_in = 0;
  long4 l4_in = 0;
  long8 l8_in = 0;
  long16 l16_in = 0;
  ulong ul_in = 0;
  ulong2 ul2_in = 0;
  ulong3 ul3_in = 0;
  ulong4 ul4_in = 0;
  ulong8 ul8_in = 0;
  ulong16 ul16_in = 0;

  a_out = select(a_in, b_in, l_in);
  a2_out = select(a2_in, b2_in, l2_in);
  a3_out = select(a3_in, b3_in, l3_in);
  a4_out = select(a4_in, b4_in, l4_in);
  a8_out = select(a8_in, b8_in, l8_in);
  a16_out = select(a16_in, b16_in, l16_in);

  a_out = select(a_in, b_in, ul_in);
  a2_out = select(a2_in, b2_in, ul2_in);
  a3_out = select(a3_in, b3_in, ul3_in);
  a4_out = select(a4_in, b4_in, ul4_in);
  a8_out = select(a8_in, b8_in, ul8_in);
  a16_out = select(a16_in, b16_in, ul16_in);

  OUTPUT_ONE_VEC_FLOAT(tid)
}

KERNEL_BI_TWOARGS(remainder)

__kernel void remquo_d(__global double *input, __global int *input_int,
                       __global double *output, __global double *output2) {
  IN_VARS_A
  IN_VARS_B
  OUT_VARS
  uint tid = 0;
  SET_IN_TWOARGS(tid)
  int i_out;
  int2 i2_out;
  int3 i3_out;
  int4 i4_out;
  int8 i8_out;
  int16 i16_out;
  a_out = remquo(a_in, b_in, &i_out);
  a2_out = remquo(a2_in, b2_in, &i2_out);
  a3_out = remquo(a3_in, b3_in, &i3_out);
  a4_out = remquo(a4_in, b4_in, &i4_out);
  a8_out = remquo(a8_in, b8_in, &i8_out);
  a16_out = remquo(a16_in, b16_in, &i16_out);
  OUTPUT_ONE_VEC_FLOAT(tid)
}

// possible data types for suffle and shuffle2 built-ins are vectors{2|4|8|16}
__kernel void shuffle_d(__global double *input, __global int *input_int,
                        __global double *output, __global double *output2) {
  double2 a2_in;
  double4 a4_in;
  double8 a8_in;
  double16 a16_in;
  double2 a2_out;
  double4 a4_out;
  double8 a8_out;
  double16 a16_out;

  ulong2 ui2_in = {1, 0};
  ulong4 ui4_in = {1, 0, 1, 0};
  ulong8 ui8_in = {1, 0, 1, 0, 1, 0, 1, 0};
  ulong16 ui16_in = {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0};
  uint tid = 0;

  a2_in.s0 = input[tid];
  a2_in.s1 = input[tid + 1];
  a4_in.s01 = a2_in;
  a4_in.s2 = input[tid + 2];
  a4_in.s3 = input[tid + 3];
  a8_in.lo = a4_in;
  a8_in.s4 = input[tid + 4];
  a8_in.s5 = input[tid + 5];
  a8_in.s6 = input[tid + 6];
  a8_in.s7 = input[tid + 7];
  a16_in.lo = a8_in;
  a16_in.s8 = input[tid + 8];
  a16_in.s9 = input[tid + 9];
  a16_in.sA = input[tid + 10];
  a16_in.sB = input[tid + 11];
  a16_in.sC = input[tid + 12];
  a16_in.sD = input[tid + 13];
  a16_in.sE = input[tid + 14];
  a16_in.sF = input[tid + 15];

  a2_out = shuffle(a2_in, ui2_in);
  a2_out = shuffle(a4_in, ui2_in);
  a2_out = shuffle(a8_in, ui2_in);
  a2_out = shuffle(a16_in, ui2_in);
  a4_out = shuffle(a2_in, ui4_in);
  a4_out = shuffle(a4_in, ui4_in);
  a4_out = shuffle(a8_in, ui4_in);
  a4_out = shuffle(a16_in, ui4_in);
  a8_out = shuffle(a2_in, ui8_in);
  a8_out = shuffle(a4_in, ui8_in);
  a8_out = shuffle(a8_in, ui8_in);
  a8_out = shuffle(a16_in, ui8_in);
  a16_out = shuffle(a2_in, ui16_in);
  a16_out = shuffle(a4_in, ui16_in);
  a16_out = shuffle(a8_in, ui16_in);
  a16_out = shuffle(a16_in, ui16_in);

  output[tid + 1] = a2_out.s0;
  output[tid + 2] = a2_out.s1;
  output[tid + 6] = a4_out.s0;
  output[tid + 7] = a4_out.s1;
  output[tid + 8] = a4_out.s2;
  output[tid + 9] = a4_out.s3;
  output[tid + 10] = a8_out.s0;
  output[tid + 11] = a8_out.s1;
  output[tid + 12] = a8_out.s2;
  output[tid + 13] = a8_out.s3;
  output[tid + 14] = a8_out.s4;
  output[tid + 15] = a8_out.s5;
  output[tid + 16] = a8_out.s6;
  output[tid + 17] = a8_out.s7;
  output[tid + 18] = a16_out.s0;
  output[tid + 19] = a16_out.s1;
  output[tid + 20] = a16_out.s2;
  output[tid + 21] = a16_out.s3;
  output[tid + 22] = a16_out.s4;
  output[tid + 23] = a16_out.s5;
  output[tid + 24] = a16_out.s6;
  output[tid + 25] = a16_out.s7;
  output[tid + 26] = a16_out.s8;
  output[tid + 27] = a16_out.s9;
  output[tid + 28] = a16_out.sA;
  output[tid + 29] = a16_out.sB;
  output[tid + 30] = a16_out.sC;
  output[tid + 31] = a16_out.sD;
  output[tid + 32] = a16_out.sE;
  output[tid + 33] = a16_out.sF;
}

__kernel void shuffle2_d(__global double *input, __global int *input_int,
                         __global double *output, __global double *output2) {
  double2 b2_in;
  double4 b4_in;
  double8 b8_in;
  double16 b16_in;
  double2 a2_in;
  double4 a4_in;
  double8 a8_in;
  double16 a16_in;
  double2 a2_out;
  double4 a4_out;
  double8 a8_out;
  double16 a16_out;

  ulong2 ui2_in = {1, 0};
  ulong4 ui4_in = {1, 0, 1, 0};
  ulong8 ui8_in = {1, 0, 1, 0, 1, 0, 1, 0};
  ulong16 ui16_in = {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0};
  ulong tid = 0;

  a2_in.s0 = input[tid];
  a2_in.s1 = input[tid + 1];
  a4_in.s01 = a2_in;
  a4_in.s2 = input[tid + 2];
  a4_in.s3 = input[tid + 3];
  a8_in.lo = a4_in;
  a8_in.s4 = input[tid + 4];
  a8_in.s5 = input[tid + 5];
  a8_in.s6 = input[tid + 6];
  a8_in.s7 = input[tid + 7];
  a16_in.lo = a8_in;
  a16_in.s8 = input[tid + 8];
  a16_in.s9 = input[tid + 9];
  a16_in.sA = input[tid + 10];
  a16_in.sB = input[tid + 11];
  a16_in.sC = input[tid + 12];
  a16_in.sD = input[tid + 13];
  a16_in.sE = input[tid + 14];
  a16_in.sF = input[tid + 15];

  b2_in = a2_in;
  b4_in = a4_in;
  b8_in = a8_in;
  b16_in = a16_in;

  a2_out = shuffle2(a2_in, b2_in, ui2_in);
  a2_out = shuffle2(a4_in, b4_in, ui2_in);
  a2_out = shuffle2(a8_in, b8_in, ui2_in);
  a2_out = shuffle2(a16_in, b16_in, ui2_in);
  a4_out = shuffle2(a2_in, b2_in, ui4_in);
  a4_out = shuffle2(a4_in, b4_in, ui4_in);
  a4_out = shuffle2(a8_in, b8_in, ui4_in);
  a4_out = shuffle2(a16_in, b16_in, ui4_in);
  a8_out = shuffle2(a2_in, b2_in, ui8_in);
  a8_out = shuffle2(a4_in, b4_in, ui8_in);
  a8_out = shuffle2(a8_in, b8_in, ui8_in);
  a8_out = shuffle2(a16_in, b16_in, ui8_in);
  a16_out = shuffle2(a2_in, b2_in, ui16_in);
  a16_out = shuffle2(a4_in, b4_in, ui16_in);
  a16_out = shuffle2(a8_in, b8_in, ui16_in);
  a16_out = shuffle2(a16_in, b16_in, ui16_in);

  output[tid + 1] = a2_out.s0;
  output[tid + 2] = a2_out.s1;
  output[tid + 6] = a4_out.s0;
  output[tid + 7] = a4_out.s1;
  output[tid + 8] = a4_out.s2;
  output[tid + 9] = a4_out.s3;
  output[tid + 10] = a8_out.s0;
  output[tid + 11] = a8_out.s1;
  output[tid + 12] = a8_out.s2;
  output[tid + 13] = a8_out.s3;
  output[tid + 14] = a8_out.s4;
  output[tid + 15] = a8_out.s5;
  output[tid + 16] = a8_out.s6;
  output[tid + 17] = a8_out.s7;
  output[tid + 18] = a16_out.s0;
  output[tid + 19] = a16_out.s1;
  output[tid + 20] = a16_out.s2;
  output[tid + 21] = a16_out.s3;
  output[tid + 22] = a16_out.s4;
  output[tid + 23] = a16_out.s5;
  output[tid + 24] = a16_out.s6;
  output[tid + 25] = a16_out.s7;
  output[tid + 26] = a16_out.s8;
  output[tid + 27] = a16_out.s9;
  output[tid + 28] = a16_out.sA;
  output[tid + 29] = a16_out.sB;
  output[tid + 30] = a16_out.sC;
  output[tid + 31] = a16_out.sD;
  output[tid + 32] = a16_out.sE;
  output[tid + 33] = a16_out.sF;
}
