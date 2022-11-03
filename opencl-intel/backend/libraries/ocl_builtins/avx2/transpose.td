// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

//AVX2
/// STYLE NOTE: Comments at same line of code are arranged to start at column 85 at the output final file

// ****************************************************************************
//                                 char4x8
// ****************************************************************************

/// @brief  Receives char4x8 matrix as char32, transposes it and outputs the rows
///        of the transposed matrix
/// @param Arg0   - ocl_char32 - char4x8 matrix to be transposed
/// @param Arg1-4 - char8*     - Rows 0-3 of the transposed matrix
OclBuiltinImpl transpose_load_char4x8_avx2_helper = OclBuiltinImpl<__ocl_load_transpose_char4x8_avx2_helper, [v4i8], 0, [{
    ocl_char32 xyzwIn = $Arg0VarName;                                               // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
    ocl_char32 dummy;
    // The reason we use here __builtin_shufflevector and not shuffle(), shuffle2() OpenCL functions
    // is because the input typeses here are char32, which are not legal OpenCL built-in types.
    // We do not want to sue __builtin_shufflevector everywhere because it's not a legal OpenCL function,
    // it's kind of a hack that makes Clang create LLVM shufflevector instructions.
    xyzwIn = __builtin_shufflevector (xyzwIn, dummy,                                // x0 x1 x2 x3 y0 y1 y2 y3 z0 z1 z2 z3 w0 w1 w2 w3 | x4 x5 x6 x7 y4 y5 y6 y7 z4 z5 z6 z7 w4 w5 w6 w7\n
          0, 4, 8, 12, 1, 5, 9, 13, 2, 6, 10, 14, 3, 7, 11, 15,
          16, 20, 24, 28, 17, 21, 25, 29, 18, 22, 26, 30, 19, 23, 27, 31);
    xyzwIn = as_ocl_char32((__builtin_astype(xyzwIn, int8)).s04152637);             // x0 x1 x2 x3 x4 x5 x6 x7 y0 y1 y2 y3 y4 y5 y6 y7 | z0 z1 z2 z3 z4 z5 z6 z7 w0 w1 w2 w3 w4 w5 w6 w7
    ocl_char32 xz = __builtin_shufflevector (xyzwIn, dummy,                         // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D | z0  D z1  D z2  D z3  D z4  D z5  D z6  D z7  D
          0, 32, 1, 33, 2, 34, 3, 35, 4, 36, 5, 37, 6, 38, 7, 39,
          16, 48, 17, 49, 18, 50, 19, 51, 20, 52, 21, 53, 22, 54, 23, 55);
    ocl_char32 yw = __builtin_shufflevector (xyzwIn, dummy,                         // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D | w0  D w1  D w2  D w3  D w4  D w5  D w6  D w7  D
         8, 40, 9, 41, 10, 42, 11, 43, 12, 44, 13, 45, 14, 46, 15, 47,
         24, 56, 25, 57, 26, 58, 27, 59, 28, 60, 29, 61, 30, 62, 31, 63);
    char16 x = as_char16((__builtin_astype(xz, int8)).lo);                          // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D
    char16 y = as_char16((__builtin_astype(yw, int8)).lo);                          // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D
    char16 z = as_char16((__builtin_astype(xz, int8)).hi);                          // z0  D z1  D z2  D z3  D z4  D z5  D z6  D z7  D
    char16 w = as_char16((__builtin_astype(yw, int8)).hi);                          // w0  D w1  D w2  D w3  D w4  D w5  D w6  D w7  D
    *$Arg1VarName = x.s02468ACE;
    *$Arg2VarName = y.s02468ACE;
    *$Arg3VarName = z.s02468ACE;
    *$Arg4VarName = w.s02468ACE;
  }]>;

/// @brief Receives char4x8 matrix as 4 matrix rows, transposes it and outputs the matrix
///        as a whole using char32
/// @param Arg0   - ocl_char32* - This parameter will contain the transposed char4x8 matrix
/// @param Arg1-4 - char8       - Rows 0-3 of the matrix to be transposed
OclBuiltinImpl transpose_store_char4x8_avx2_helper = OclBuiltinImpl<__ocl_transpose_store_char4x8_avx2_helper, [v4i8], 0, [{
    char16 x;
    char16 y;
    char16 z;
    char16 w;
    x.s02468ACE = $Arg1VarName;                                                     // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D
    y.s02468ACE = $Arg2VarName;                                                     // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D
    z.s02468ACE = $Arg3VarName;                                                     // z0  D z1  D z2  D z3  D z4  D z5  D z6  D z7  D
    w.s02468ACE = $Arg4VarName;                                                     // w0  D w1  D w2  D w3  D w4  D w5  D w6  D w7  D
    ocl_char32 xz;
    ocl_char32 yw;
    (*(__private int8*)&xz).lo = as_int4(x);                                        // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D
    (*(__private int8*)&yw).lo = as_int4(y);                                        // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D
    (*(__private int8*)&xz).hi = as_int4(z);                                        // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D | z0  D z1  D z2  D z3  D z4  D z5  D z6  D z7  D
    (*(__private int8*)&yw).hi = as_int4(w);                                        // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D | w0  D w1  D w2  D w3  D w4  D w5  D w6  D w7  D
    // The reason we use here __builtin_shufflevector and not shuffle(), shuffle2() OpenCL functions
    // is because the input typeses here are char32, which are not legal OpenCL built-in types.
    // We do not want to sue __builtin_shufflevector everywhere because it's not a legal OpenCL function,
    // it's kind of a hack that makes Clang create LLVM shufflevector instructions.
    ocl_char32 dummy;
    xz = __builtin_shufflevector (xz, dummy,                                        // x0 x1 x2 x3 x4 x5 x6 x7  D  D  D  D  D  D  D  D | z0 z1 z2 z3 z4 z5 z6 z7  D  D  D  D  D  D  D  D
          0, 2, 4, 6, 8, 10, 12, 14, 0, 0, 0, 0, 0, 0, 0, 0,
          16, 18, 20, 22, 24, 26, 28, 30, 0, 0, 0, 0, 0, 0, 0, 0);
    yw = __builtin_shufflevector (yw, dummy,                                        // y0 y1 y2 y3 y4 y5 y6 y7  D  D  D  D  D  D  D  D | w0 w1 w2 w3 w4 w5 w6 w7  D  D  D  D  D  D  D  D
          0, 2, 4, 6, 8, 10, 12, 14, 0, 0, 0, 0, 0, 0, 0, 0,
          16, 18, 20, 22, 24, 26, 28, 30, 0, 0, 0, 0, 0, 0, 0, 0);
    ocl_char32 txyzw = __builtin_shufflevector (xz, yw,                             // x0 y0 x1 y1 x2 y2 x3 y3 x4 y4 x5 y5 x6 y6 x7 y7 | z0 w0 z1 w1 z2 w2 z3 w3 z4 w4 z5 w5 z6 w6 z7 w7
          0, 32, 1, 33, 2, 34, 3, 35, 4, 36, 5, 37, 6, 38, 7, 39,
          16, 48, 17, 49, 18, 50, 19, 51, 20, 52, 21, 53, 22, 54, 23, 55);
    txyzw = as_ocl_char32((__builtin_astype(txyzw, int8)).s01452367);               // x0 y0 x1 y1 x2 y2 x3 y3 z0 w0 z1 w1 z2 w2 z3 w3 | x4 y4 x5 y5 x6 y6 x7 y7 z4 w4 z5 w5 z6 w6 z7 w7
    txyzw = __builtin_shufflevector (txyzw, dummy,                                  // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
          0, 1, 8, 9, 2, 3, 10, 11, 4, 5, 12, 13, 6, 7, 14, 15,
          16, 17, 24, 25, 18, 19, 26, 27, 20, 21, 28, 29, 22, 23, 30, 31);
    *$Arg0VarName = txyzw;
  }]>;
OclBuiltinImpl transpose_load_char4x8_avx2 = OclBuiltinImpl<__ocl_load_transpose4x8, [v4i8],  0, [{
    // We load "int8", meaning we load the full matrix in a single load
    const __private int* tmpLoadAdd = (const __private int*)$Arg0VarName;
    ocl_char32 vLoaded = (ocl_char32)vload8(0, tmpLoadAdd);                            // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7

    __ocl_load_transpose_char4x8_avx2_helper(vLoaded, $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName);
  }]>;

OclBuiltinImpl transpose_store_char4x8_avx2 = OclBuiltinImpl<__ocl_transpose_store4x8, [v4i8],  0, [{
    ocl_char32 xyzw;
    __private ocl_char32* pStore = &xyzw;

    __ocl_transpose_store_char4x8_avx2_helper(pStore, $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName);

    // We store "int8", meaning we store the full matrix in a single store
    __private int* tmpStoreAdd = (__private int*)$Arg0VarName;
    vstore8(__builtin_astype(xyzw, int8), 0, tmpStoreAdd);
  }]>;

OclBuiltinImpl transpose_gather_char4x8_avx2 = OclBuiltinImpl<__ocl_gather_transpose4x8, [v4i8],  0, [{
    // Broadcast the loaded values, all but the first element in the register which will be moved there
    int8 xyzw1 = *((__private int*)$Arg1VarName);                                     // x1 y1 z1 w1 x1 y1 z1 w1 x1 y1 z1 w1 x1 y1 z1 w1
    int8 xyzw2 = *((__private int*)$Arg2VarName);                                     // x2 y2 z2 w2 x2 y2 z2 w2 x2 y2 z2 w2 x2 y2 z2 w2
    int8 xyzw3 = *((__private int*)$Arg3VarName);                                     // x3 y3 z3 w3 x3 y3 z3 w3 x3 y3 z3 w3 x3 y3 z3 w3
    int8 xyzw4 = *((__private int*)$Arg4VarName);                                     // x4 y4 z4 w4 x4 y4 z4 w4 x4 y4 z4 w4 x4 y4 z4 w4
    int8 xyzw5 = *((__private int*)$Arg5VarName);                                     // x5 y5 z5 w5 x5 y5 z5 w5 x5 y5 z5 w5 x5 y5 z5 w5
    int8 xyzw6 = *((__private int*)$Arg6VarName);                                     // x6 y6 z6 w6 x6 y6 z6 w6 x6 y6 z6 w6 x6 y6 z6 w6
    int8 xyzw7 = *((__private int*)$Arg7VarName);                                     // x7 y7 z7 w7 x7 y7 z7 w7 x7 y7 z7 w7 x7 y7 z7 w7

    int8 xyzw = 0;
    // TODO : Replace this blend built-in with ?: when clang bug will be fixed
    xyzw.s0 = *((__private int*)$Arg0VarName);                                        // x0 y0 z0 w0  D  D  D  D  D  D  D  D  D  D  D  D |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D
    xyzw = as_int8(_mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw1, 0x2));           // x0 y0 z0 w0 x1 y1 z1 w1  D  D  D  D  D  D  D  D |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D
    xyzw = as_int8(_mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw2, 0x4));           // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2  D  D  D  D |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D
    xyzw = as_int8(_mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw3, 0x8));           // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D
    xyzw = as_int8(_mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw4, 0x10));          // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4  D  D  D  D  D  D  D  D  D  D  D  D
    xyzw = as_int8(_mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw5, 0x20));          // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5  D  D  D  D  D  D  D  D
    xyzw = as_int8(_mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw6, 0x40));          // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6  D  D  D  D
    xyzw = as_int8(_mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw7, 0x80));          // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
    ocl_char32 vLoaded = (ocl_char32)xyzw;

    __ocl_load_transpose_char4x8_avx2_helper(vLoaded, $Arg8VarName, $Arg9VarName, $Arg10VarName, $Arg11VarName);
  }]>;

OclBuiltinImpl transpose_scatter_char4x8_avx2 = OclBuiltinImpl<__ocl_transpose_scatter4x8, [v4i8],  0, [{
    int8 xyzw = 0;
    __private ocl_char32* pStore = (__private ocl_char32*)&xyzw;
    __ocl_transpose_store_char4x8_avx2_helper(pStore, $Arg8VarName, $Arg9VarName, $Arg10VarName, $Arg11VarName);

    *((__private int*)$Arg0VarName) = xyzw.s0;
    *((__private int*)$Arg1VarName) = xyzw.s1;
    *((__private int*)$Arg2VarName) = xyzw.s2;
    *((__private int*)$Arg3VarName) = xyzw.s3;
    *((__private int*)$Arg4VarName) = xyzw.s4;
    *((__private int*)$Arg5VarName) = xyzw.s5;
    *((__private int*)$Arg6VarName) = xyzw.s6;
    *((__private int*)$Arg7VarName) = xyzw.s7;
  }]>;

// ****************************************************************************
//                                 short4x8
// ****************************************************************************

OclBuiltinImpl transpose_gather_short4x8_avx2 = OclBuiltinImpl<__ocl_gather_transpose4x8, [v4i16],  0, [{
    long4 xyzw1 = *((__private long*)$Arg1VarName);                                   // x1 y1 z1 w1 x1 y1 z1 w1 x1 y1 z1 w1 x1 y1 z1 w1
    long4 xyzw2 = *((__private long*)$Arg2VarName);                                   // x2 y2 z2 w2 x2 y2 z2 w2 x2 y2 z2 w2 x2 y2 z2 w2
    long4 xyzw3 = *((__private long*)$Arg3VarName);                                   // x3 y3 z3 w3 x3 y3 z3 w3 x3 y3 z3 w3 x3 y3 z3 w3
    long4 xyzw5 = *((__private long*)$Arg5VarName);                                   // x5 y5 z5 w5 x5 y5 z5 w5 x5 y5 z5 w5 x5 y5 z5 w5
    long4 xyzw6 = *((__private long*)$Arg6VarName);                                   // x6 y6 z6 w6 x6 y6 z6 w6 x6 y6 z6 w6 x6 y6 z6 w6
    long4 xyzw7 = *((__private long*)$Arg7VarName);                                   // x7 y7 z7 w7 x7 y7 z7 w7 x7 y7 z7 w7 x7 y7 z7 w7

    long4 xyzw0123;
    xyzw0123.s0 = *((__private long*)$Arg0VarName);                                   // x0 y0 z0 w0  D  D  D  D  D  D  D  D  D  D  D  D
    xyzw0123 = as_long4(_mm256_blend_epi32((__m256i)xyzw0123, (__m256i)xyzw1, 0xC));  // x0 y0 z0 w0 x1 y1 z1 w1  D  D  D  D  D  D  D  D
    xyzw0123 = as_long4(_mm256_blend_epi32((__m256i)xyzw0123, (__m256i)xyzw2, 0x30)); // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2  D  D  D  D
    xyzw0123 = as_long4(_mm256_blend_epi32((__m256i)xyzw0123, (__m256i)xyzw3, 0xC0)); // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3

    long4 xyzw4567;
    xyzw4567.s0 = *((__private long*)$Arg4VarName);                                                // x4 y4 z4 w4  D  D  D  D  D  D  D  D  D  D  D  D
    xyzw4567 = as_long4(_mm256_blend_epi32((__m256i)xyzw4567, (__m256i)xyzw5, 0xC));  // x4 y4 z4 w4 x5 y5 z5 w5  D  D  D  D  D  D  D  D
    xyzw4567 = as_long4(_mm256_blend_epi32((__m256i)xyzw4567, (__m256i)xyzw6, 0x30)); // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6  D  D  D  D
    xyzw4567 = as_long4(_mm256_blend_epi32((__m256i)xyzw4567, (__m256i)xyzw7, 0xC0)); // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
    short16 vLoaded0 = as_short16(xyzw0123);
    short16 vLoaded1 = as_short16(xyzw4567);

    __ocl_load_transpose4x8_helper(vLoaded0, vLoaded1, $Arg8VarName, $Arg9VarName, $Arg10VarName, $Arg11VarName);
  }]>;

OclBuiltinImpl transpose_scatter_short4x8_avx2 = OclBuiltinImpl<__ocl_transpose_scatter4x8, [v4i16],  0, [{
    short16 xyzw0, xyzw1;
    __private short16* pStore0 = &xyzw0;
    __private short16* pStore1 = &xyzw1;

    __ocl_transpose_store4x8_helper(pStore0, pStore1, $Arg8VarName, $Arg9VarName, $Arg10VarName, $Arg11VarName);

    *((__private long*)$Arg0VarName) = as_long4(xyzw0).s0;                             // x0 y0 z0 w0
    *((__private long*)$Arg1VarName) = as_long4(xyzw0).s1;                             // x1 y1 z1 w1
    *((__private long*)$Arg2VarName) = as_long4(xyzw0).s2;                             // x2 y2 z2 w2
    *((__private long*)$Arg3VarName) = as_long4(xyzw0).s3;                             // x3 y3 z3 w3
    *((__private long*)$Arg4VarName) = as_long4(xyzw1).s0;                             // x4 y4 z4 w4
    *((__private long*)$Arg5VarName) = as_long4(xyzw1).s1;                             // x5 y5 z5 w5
    *((__private long*)$Arg6VarName) = as_long4(xyzw1).s2;                             // x6 y6 z6 w6
    *((__private long*)$Arg7VarName) = as_long4(xyzw1).s3;                             // x7 y7 z7 w7
  }]>;

// ****************************************************************************
//                                 int4x4
// ****************************************************************************
/// @brief Receives int4x4 matrix as 2 halfs (2 int2x4 matrixes) using 2 int8,
///        transposes it and outputs the rows of the transposed matrix.
/// @param Arg0   - int8  - Upper part of the int2x4 matrix to be transposed
/// @param Arg1   - int8  - Lower part of the int2x4 matrix to be transposed
/// @param Arg2-5 - int4* - Rows 0-3 of the transposed matrix
OclBuiltinImpl transpose_load_int4x4_avx2_helper = OclBuiltinImpl<__ocl_load_transpose4x4_avx2_helper, [v4i32], 0, [{
    $Arg0VarName = $Arg0VarName.s04152637;                                                           // x0 x1 y0 y1 z0 z1 w0 w1
    $Arg1VarName = $Arg1VarName.s15043726;                                                           // y2 y3 x2 x3 w2 w3 z2 z3

    // TODO : Replace this blend built-in with ?: when clang bug will be fixed
    // int8 cond = {0, 0, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0, 0xFFFFFFFF, 0xFFFFFFFF};
    // int8 xz = cond ? Arg1 : Arg2;
    int8 xz = as_int8(_mm256_blend_epi32((__m256i)$Arg0VarName, (__m256i)$Arg1VarName, 0xCC)); // x0 x1 x2 x3 z0 z1 z2 z3
    uint8 ywIndices = {2, 3, 8, 9, 6, 7, 12, 13};
    int8 yw = __ocl_helper_shuffle2($Arg0VarName, $Arg1VarName, ywIndices);                    // y0 y1 y2 y3 w0 w1 w2 w3

    // x0 x1 x2 x3 z0 z1 z2 z3 = xz
    // y0 y1 y2 y3 w0 w1 w2 w3 = yw
    *$Arg2VarName = xz.lo;                                                                           // x0 x1 x2 x3
    *$Arg3VarName = yw.lo;                                                                           // y0 y1 y2 y3
    *$Arg4VarName = xz.hi;                                                                           // z0 z1 z2 z3
    *$Arg5VarName = yw.hi;                                                                           // w0 w1 w2 w3
  }]>;

/// @brief Receives int4x4 matrix as 2 halfs (2 int2x4 matrixes) using 2 int8,
///        transposes it and outputs the rows of the transposed matrix.
/// @param Arg0 - int8  - Part of the int2x4 matrix to be transposed
/// @param Arg1 - int8  - Part of the int2x4 matrix to be transposed
/// @param Arg2 - int8* - Part of the transposed int4x2 matrix.
/// @param Arg3 - int8* - Part of the transposed int4x2 matrix
OclBuiltinImpl transpose_store_int4x4_avx2_helper = OclBuiltinImpl<__ocl_transpose_store4x4_avx2_helper, [v4i32], 0, [{
    $Arg0VarName = $Arg0VarName.s04261537;                                                           // x0 y0 x2 y2 x1 y1 x3 y3\n"#
    $Arg1VarName = $Arg1VarName.s26043715;                                                           // z2 w2 z0 w0 z3 w3 z1 w1\n"#

    // TODO : Replace this blend built-in with ?: when clang bug will be fixed
    // int8 cond = {0, 0, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0, 0xFFFFFFFF, 0xFFFFFFFF};
    // int8 xz = cond ? Arg1 : Arg2;
    *$Arg2VarName = as_int8(_mm256_blend_epi32((__m256i)$Arg0VarName, (__m256i)$Arg1VarName, 0xCC)); // x0 y0 z0 w0 x1 y1 z1 w1
    uint8 ywIndices = {2, 3, 8, 9, 6, 7, 12, 13};
    *$Arg3VarName = __ocl_helper_shuffle2($Arg0VarName, $Arg1VarName, ywIndices);                    // x2 y2 z2 w2 x3 y3 z3 w3
  }]>;

OclBuiltinImpl transpose_load_int4x4_avx2 = OclBuiltinImpl<__ocl_load_transpose4x4, [v4i32], 0, [{
    // We load "int8", meaning we load the full matrix in 2 loads
    const __private int* tmpLoadAdd = (const __private int*)$Arg0VarName;
    int8 vLoaded0 = vload8(0, tmpLoadAdd);                                                           // x0 y0 z0 w0 x1 y1 z1 w1 xyzw01
    int8 vLoaded1 = vload8(1, tmpLoadAdd);                                                           // x2 y2 z2 w2 x3 y3 z3 w3 xyzw23

    __ocl_load_transpose4x4_avx2_helper(vLoaded0, vLoaded1, $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName);
  }]>;

OclBuiltinImpl transpose_store_int4x4_avx2 = OclBuiltinImpl<__ocl_transpose_store4x4, [v4i32],  0, [{
    int8 Arg0 = {$Arg1VarName, $Arg2VarName};                                                        // x0 x1 x2 x3 y0 y1 y2 y3 xy
    int8 Arg1 = {$Arg3VarName, $Arg4VarName};                                                        // z0 z1 z2 z3 w0 w1 w2 w3 zw
    int8 xyzw01;
    int8 xyzw23;
    __private int8* pStore0 = &xyzw01;
    __private int8* pStore1 = &xyzw23;

    __ocl_transpose_store4x4_avx2_helper(Arg0, Arg1, pStore0, pStore1);

    // We store "int8", meaning we store the full matrix in 2 stores
    __private int* tmpStoreAdd = (__private int*)$Arg0VarName;
    vstore8(xyzw01, 0, tmpStoreAdd);
    vstore8(xyzw23, 1, tmpStoreAdd);
  }]>;

OclBuiltinImpl transpose_gather_int4x4_avx2 = OclBuiltinImpl<__ocl_gather_transpose4x4, [v4i32],  0, [{
    int8 vLoaded0 = {*$Arg0VarName, *$Arg1VarName};                                               // x0 y0 z0 w0 x1 y1 z1 w1
    int8 vLoaded1 = {*$Arg2VarName, *$Arg3VarName};                                               // x2 y2 z2 w2 x3 y3 z3 w3

    __ocl_load_transpose4x4_avx2_helper(vLoaded0, vLoaded1, $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName);
  }]>;

OclBuiltinImpl transpose_scatter_int4x4_avx2 = OclBuiltinImpl<__ocl_transpose_scatter4x4, [v4i32],  0, [{
    int8 Arg0 = {$Arg4VarName, $Arg5VarName};                                                           // x0 x1 x2 x3 y0 y1 y2 y3
    int8 Arg1 = {$Arg6VarName, $Arg7VarName};                                                           // z0 z1 z2 z3 w0 w1 w2 w3

    int8 xyzw01;
    int8 xyzw23;
    __private int8* pStore0 = &xyzw01;
    __private int8* pStore1 =  &xyzw23;

    __ocl_transpose_store4x4_avx2_helper(Arg0, Arg1, pStore0, pStore1);

    *$Arg0VarName = xyzw01.lo;
    *$Arg1VarName = xyzw01.hi;
    *$Arg2VarName = xyzw01.lo;
    *$Arg3VarName = xyzw01.hi;
  }]>;

// ****************************************************************************
//                                 int4x8
// ****************************************************************************

/// @brief Receives int8x4 matrix as 4 double rows (4 int2x4 matrixes) using 4 int8,
///        transposes it and outputs the rows of the transposed matrix.
/// @param Arg0    - int8  - Rows 0,4 of the matrix to be transposed
/// @param Arg1    - int8  - Rows 1,5 of the matrix to be transposed
/// @param Arg2    - int8  - Rows 2,6 og the matrix to be transposed
/// @param Arg3    - int8  - Rows 3,7 matrix to be transposed
/// @param Arg4-7  - int8* - Rows 0-3 of the transposed matrix
OclBuiltinImpl transpose_common_int4x8_avx2_helper = OclBuiltinImpl<__ocl_common_transpose4x8_avx2_helper, [v4i32], 0, [{
    uint8 lowLane8 = {0, 8, 1, 9, 4, 12, 5, 13};
    uint8 highLane8 = {2, 10, 3, 11, 6, 14, 7, 15};
    int8 xy0246 = __ocl_helper_shuffle2($Arg0VarName, $Arg2VarName, lowLane8);               // x0 x2 y0 y2 x4 x6 y4 y6
    int8 zw0246 = __ocl_helper_shuffle2($Arg0VarName, $Arg2VarName, highLane8);              // z0 z2 w0 w2 z4 z6 w4 w6
    int8 xy1357 = __ocl_helper_shuffle2($Arg1VarName, $Arg3VarName, lowLane8);               // x1 x3 y1 y3 x5 x7 y5 y7
    int8 zw1357 = __ocl_helper_shuffle2($Arg1VarName, $Arg3VarName, highLane8);              // z1 z3 w1 w3 z5 z7 w5 w7
    *$Arg4VarName = __ocl_helper_shuffle2(xy0246, xy1357, lowLane8);                         // x0 x1 x2 x3 x4 x5 x6 x7
    *$Arg5VarName = __ocl_helper_shuffle2(xy0246, xy1357, highLane8);                        // y0 y1 y2 y3 y4 y5 y6 y7
    *$Arg6VarName = __ocl_helper_shuffle2(zw0246, zw1357, lowLane8);                         // z0 z1 z2 z3 z4 z5 z6 z7
    *$Arg7VarName = __ocl_helper_shuffle2(zw0246, zw1357, highLane8);                        // w0 w1 w2 w3 w4 w5 w6 w7
  }]>;

OclBuiltinImpl transpose_load_int4x8_avx2 = OclBuiltinImpl<__ocl_load_transpose4x8, [v4i32],  0, [{
    __ocl_gather_transpose_int_4x8( &($Arg0VarName[0]), &($Arg0VarName[1]), &($Arg0VarName[2]), &($Arg0VarName[3]),
                                    &($Arg0VarName[4]), &($Arg0VarName[5]), &($Arg0VarName[6]), &($Arg0VarName[7]),
                                    $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName);
  }]>;

OclBuiltinImpl transpose_store_int4x8_avx2 = OclBuiltinImpl<__ocl_transpose_store4x8, [v4i32],  0, [{
    __ocl_transpose_scatter_int_4x8(&($Arg0VarName[0]), &($Arg0VarName[1]), &($Arg0VarName[2]), &($Arg0VarName[3]),
                                    &($Arg0VarName[4]), &($Arg0VarName[5]), &($Arg0VarName[6]), &($Arg0VarName[7]),
                                    $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName);
  }]>;

OclBuiltinImpl transpose_gather_int4x8_avx2 = OclBuiltinImpl<__ocl_gather_transpose4x8, [v4i32],  0, [{

    // TODO : Replace this shuffle with { pLoadAdd0,  pLoadAdd4} instead of shuffle builtin
    // when clang bug will be fixed
    uint8 concatVectors4 = {0, 1, 2, 3, 4, 5, 6, 7};
    int8 Arg0 = __ocl_helper_shuffle2(*$Arg0VarName, *$Arg4VarName, concatVectors4);        // x0 y0 z0 w0 x4 y4 z4 w4
    int8 Arg1 = __ocl_helper_shuffle2(*$Arg1VarName, *$Arg5VarName, concatVectors4);        // x1 y1 z1 w1 x5 y5 z5 w5
    int8 Arg2 = __ocl_helper_shuffle2(*$Arg2VarName, *$Arg6VarName, concatVectors4);        // x2 y2 z2 w2 x6 y6 z6 w6
    int8 Arg3 = __ocl_helper_shuffle2(*$Arg3VarName, *$Arg7VarName, concatVectors4);        // x3 y3 z3 w3 x7 y7 z7 w7

    __ocl_common_transpose4x8_avx2_helper(Arg0, Arg1, Arg2, Arg3, $Arg8VarName, $Arg9VarName, $Arg10VarName, $Arg11VarName);
                                                                                            // xOut = x0 x1 x2 x3 x4 x5 x6 x7
                                                                                            // yOut = y0 y1 y2 y3 y4 y5 y6 y7
                                                                                            // zOut = z0 z1 z2 z3 z4 z5 z6 z7
                                                                                            // wOut = w0 w1 w2 w3 w4 w5 w6 w7
  }]>;

OclBuiltinImpl transpose_scatter_int4x8_avx2 = OclBuiltinImpl<__ocl_transpose_scatter4x8, [v4i32],  0, [{
    int8 xyzw04;
    int8 xyzw15;
    int8 xyzw26;
    int8 xyzw37;
    __private int8* Arg4 = &xyzw04;
    __private int8* Arg5 = &xyzw15;
    __private int8* Arg6 = &xyzw26;
    __private int8* Arg7 = &xyzw37;

    __ocl_common_transpose4x8_avx2_helper($Arg8VarName, $Arg9VarName, $Arg10VarName, $Arg11VarName, Arg4, Arg5, Arg6, Arg7);
                                                                                      // xyzw04 = x0 y0 z0 w0 x4 y4 z4 w4
                                                                                      // xyzw15 = x1 y1 z1 w1 x5 y5 z5 w5
                                                                                      // xyzw26 = x2 y2 z2 w2 x6 y6 z6 w6
                                                                                      // xyzw37 = x3 y3 z3 w3 x7 y7 z7 w7

    *$Arg0VarName = xyzw04.lo;                                                          // x0 y0 z0 w0
    *$Arg1VarName = xyzw15.lo;                                                          // x1 y1 z1 w1
    *$Arg2VarName = xyzw26.lo;                                                          // x2 y2 z2 w2
    *$Arg3VarName = xyzw37.lo;                                                          // x3 y3 z3 w3
    *$Arg4VarName = xyzw04.hi;                                                          // x4 y4 z4 w4
    *$Arg5VarName = xyzw15.hi;                                                          // x5 y5 z5 w5
    *$Arg6VarName = xyzw26.hi;                                                          // x6 y6 z6 w6
    *$Arg7VarName = xyzw37.hi;                                                          // x7 y7 z7 w7
  }]>;

// ****************************************************************************
//                                 int2x8
// ****************************************************************************
OclBuiltinImpl transpose_load_int2x8_avx2 = OclBuiltinImpl<__ocl_load_transpose2x8, [v2i32],  0, [{

    const __private int* tmpLoadAdd = (const __private int*)$Arg0VarName;
    int8 Arg0 = vload8(0, tmpLoadAdd);
    int8 Arg1 = vload8(1, tmpLoadAdd);


    uint8 firstArray = {0, 2, 4, 6, 8, 10, 12, 14};
    uint8 secondArray = {1, 3, 5, 7, 9, 11, 13, 15};
    *$Arg1VarName = __ocl_helper_shuffle2(Arg0, Arg1, firstArray);
    *$Arg2VarName = __ocl_helper_shuffle2(Arg0, Arg1, secondArray);


  }]>;

OclBuiltinImpl transpose_masked_load_int2x8_avx = OclBuiltinImpl<__ocl_masked_load_transpose2x8, [v2i32], 0, [{
    __private int2 xy[8] = {0};
    __ocl_masked_load_int_2x8($Arg0VarName, xy, $Arg3VarName);
    __ocl_load_transpose_$Arg0BaseType_2x8(xy, $Arg1VarName, $Arg2VarName);
  }]>;

// ****************************************************************************
//                                 float4x4
// ****************************************************************************

/// @brief Receives float4x4 matrix as 2 halfs (2 float2x4 matrixes) using 2 float8,
///        transposes it and outputs the rows of the transposed matrix.
/// @param Arg0    - float8  - Upper part of the float2x4 matrix to be transposed
/// @param Arg1    - float8  - Lower part of the float2x4 matrix to be transposed
/// @param Arg2-5  - float4* - Rows 0-3 of the transposed matrix
OclBuiltinImpl transpose_load_float4x4_avx2_helper = OclBuiltinImpl<__ocl_load_transpose4x4_avx2_helper, [v4f32], 0, [{
    $Arg0VarName = $Arg0VarName.s04152637;                                                     // x0 x1 y0 y1 z0 z1 w0 w1
    $Arg1VarName = $Arg1VarName.s15043726;                                                     // y2 y3 x2 x3 w2 w3 z2 z3

    // TODO : Replace this blend built-in with ?: when clang bug will be fixed
    //int8 cond = {0, 0, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0, 0xFFFFFFFF, 0xFFFFFFFF};
    //int8 xz = cond ? Arg0 : Arg1;
    float8 xz = as_float8(_mm256_blend_ps((__m256)$Arg0VarName, (__m256)$Arg1VarName, 0xCC));  // x0 x1 x2 x3 z0 z1 z2 z3
    uint8 ywShuffle = {2, 3, 8, 9, 6, 7, 12, 13};
    float8 yw = __ocl_helper_shuffle2($Arg0VarName, $Arg1VarName, ywShuffle);                  // y0 y1 y2 y3 w0 w1 w2 w3
    *$Arg2VarName = xz.lo;
    *$Arg3VarName = yw.lo;
    *$Arg4VarName = xz.hi;
    *$Arg5VarName = yw.hi;
  }]>;

/// @brief Receives float4x4 matrix as 2 halfs (2 float2x4 matrixes) using 2 float8,
///        transposes it and outputs the rows of the transposed matrix.
/// @param Arg0  - float8  - Part of the float2x4 matrix to be transposed
/// @param Arg1  - float8  - Part of the float2x4 matrix to be transposed
/// @param Arg2  - float8* - Part of the transposed float4x2 matrix.
/// @param Arg3  - float8* - Part of the transposed int4x2 matrix
OclBuiltinImpl transpose_store_float4x4_avx2_helper = OclBuiltinImpl<__ocl_transpose_store4x4_avx2_helper, [v4f32], 0, [{
    // Arg0 = x0 x1 x2 x3 y0 y1 y2 y3
    // Arg1 = z0 z1 z2 z3 w0 w1 w2 w3
    $Arg0VarName = $Arg0VarName.s04261537;                                                          // x0 y0 x2 y2 x1 y1 x3 y3
    $Arg1VarName = $Arg1VarName.s26043715;                                                          // z2 w2 z0 w0 z3 w3 z1 w1

    // TODO : Replace this blend built-in with ?: when clang bug will be fixed
    //int8 cond = {0, 0, 0xFFFFFFFF, 0xFFFFFFFF, 0, 0, 0xFFFFFFFF, 0xFFFFFFFF};
    //int8 xz = cond ? Arg0 : Arg1;
    *$Arg2VarName = as_float8(_mm256_blend_ps((__m256)$Arg0VarName, (__m256)$Arg1VarName, 0xCC));   // x0 y0 z0 w0 x1 y1 z1 w1
    uint8 ywShuffle = {2, 3, 8, 9, 6, 7, 12, 13};
    *$Arg3VarName = __ocl_helper_shuffle2($Arg0VarName, $Arg1VarName, ywShuffle);                   // x2 y2 z2 w2 x3 y3 z3 w3
  }]>;

OclBuiltinImpl transpose_load_float4x4_avx2 = OclBuiltinImpl<__ocl_load_transpose4x4, [v4f32],  0, [{
    // We load "float8", meaning we load the full matrix in 2 loads
    const __private float* tmpLoadAdd = (const __private float*)$Arg0VarName;
    float8 vLoaded0 = vload8(0, tmpLoadAdd);                                              // x0 y0 z0 w0 x1 y1 z1 w1
    float8 vLoaded1 = vload8(1, tmpLoadAdd);                                              // x2 y2 z2 w2 x3 y3 z3 w3

    __ocl_load_transpose4x4_avx2_helper(vLoaded0, vLoaded1, $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName);
  }]>;

OclBuiltinImpl transpose_store_float4x4_avx2 = OclBuiltinImpl<__ocl_transpose_store4x4, [v4f32],  0, [{
    //float8 xy = {xIn, yIn};                                                         // x0 x1 x2 x3 y0 y1 y2 y3
    //float8 zw = {zIn, wIn};                                                         // z0 z1 z2 z3 w0 w1 w2 w3
    // TODO : Replace this shuffle with  {*pLoadAdd0, *pLoadAdd1} instead of shuffle builtin
    // when clang bug will be fixed
    float8 Arg0;
    float8 Arg1;

    Arg0.lo = $Arg1VarName;                                                                    // x0 x1 x2 x3 D  D  D  D
    Arg0.hi = $Arg2VarName;                                                                    // x0 x1 x2 x3 y0 y1 y2 y3
    Arg1.lo = $Arg3VarName;                                                                    // z0 z1 z2 z3 D  D  D  D
    Arg1.hi = $Arg4VarName;                                                                    // z0 z1 z2 z3 w0 w1 w2 w3

    float8 xyzw01;
    float8 xyzw23;
    __private float8* pStore0 = &xyzw01;
    __private float8* pStore1 = &xyzw23;

    __ocl_transpose_store4x4_avx2_helper(Arg0, Arg1, pStore0, pStore1);

    // We store "float8", meaning we store the full matrix in 2 stores
    __private float* tmpStoreAdd = (__private float*)$Arg0VarName;
    vstore8(xyzw01, 0, tmpStoreAdd);
    vstore8(xyzw23, 1, tmpStoreAdd);
  }]>;

OclBuiltinImpl transpose_gather_float4x4_avx2 = OclBuiltinImpl<__ocl_gather_transpose4x4, [v4f32],  0, [{
    //float8 Arg0 = {*pLoadAdd0, *pLoadAdd1};                                         // x0 y0 z0 w0 x1 y1 z1 w1
    //float8 Arg1 = {*pLoadAdd2, *pLoadAdd3};                                         // x2 y2 z2 w2 x3 y3 z3 w3
    // TODO : Replace this shuffle with  {*pLoadAdd0, *pLoadAdd1} instead of shuffle builtin
    // when clang bug will be fixed
    float8 Arg0;
    float8 Arg1;

    Arg0.lo = *$Arg0VarName;                                                        // x0 y0 z0 w0 D  D  D  D
    Arg0.hi = *$Arg1VarName;                                                        // x0 y0 z0 w0 x4 y4 z4 w4
    Arg1.lo = *$Arg2VarName;                                                        // x0 y0 z0 w0 D  D  D  D
    Arg1.hi = *$Arg3VarName;                                                        // x0 y0 z0 w0 x4 y4 z4 w4

    __ocl_load_transpose4x4_avx2_helper(Arg0, Arg1, $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName);
  }]>;

OclBuiltinImpl transpose_scatter_float4x4_avx2 = OclBuiltinImpl<__ocl_transpose_scatter4x4, [v4f32],  0, [{
    //float8 Arg0 = {xIn, yIn};                                                       // x0 x1 x2 x3 y0 y1 y2 y3
    //float8 Arg1 = {zIn, wIn};                                                       // z0 z1 z2 z3 w0 w1 w2 w3
    // TODO : Replace this shuffle with  {*pLoadAdd0, *pLoadAdd1} instead of shuffle builtin
    // when clang bug will be fixed
    float8 Arg0;
    float8 Arg1;

    Arg0.lo = $Arg4VarName;                                                             // x0 x1 x2 x3 D  D  D  D
    Arg0.hi = $Arg5VarName;                                                             // x0 x1 x2 x3 y0 y1 y2 y3
    Arg1.lo = $Arg6VarName;                                                             // z0 z1 z2 z3 D  D  D  D
    Arg1.hi = $Arg7VarName;                                                             // z0 z1 z2 z3 w0 w1 w2 w3

    float8 xyzw01;
    float8 xyzw23;
    __private float8* pStore0 = &xyzw01;
    __private float8* pStore1 = &xyzw23;

    __ocl_transpose_store4x4_avx2_helper(Arg0, Arg1, pStore0, pStore1);

    *$Arg0VarName = xyzw01.lo;
    *$Arg1VarName = xyzw01.hi;
    *$Arg2VarName = xyzw23.lo;
    *$Arg3VarName = xyzw23.hi;
  }]>;

