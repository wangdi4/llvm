//AVX2
/// STYLE NOTE: Comments at same line of code are arranged to start at column 85 at the output final file

// ****************************************************************************
//                                 char4x8
// ****************************************************************************
OclBuiltinImpl transpose_load_char4x8_avx2 = OclBuiltinImpl<__ocl_load_transpose4x8, [v4i8],  0, [{
    // We load "int8", meaning we load the full matrix in a single load
    int* tmpLoadAdd = (int*)$Arg0VarName;
    ocl_char32 Arg0 = (ocl_char32) vload8(0, tmpLoadAdd);                             // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
    $Arg1Type Arg1 = $Arg1VarName;
    $Arg2Type Arg2 = $Arg2VarName;
    $Arg3Type Arg3 = $Arg3VarName;
    $Arg4Type Arg4 = $Arg4VarName;

    //__ocl_load_transpose_char4x8_common_avx2(ocl_char32 Arg0, char8* Arg1, char8* Arg2, char8* Arg3, char8* Arg4)
    #{__ocl_load_transpose_char4x8_common_avx2}#
  }]>;

OclBuiltinImpl transpose_store_char4x8_avx2 = OclBuiltinImpl<__ocl_transpose_store4x8, [v4i8],  0, [{

    ocl_char32 xyzw;
    ocl_char32* Arg0 = &xyzw;
    $Arg1Type Arg1 = $Arg1VarName;
    $Arg2Type Arg2 = $Arg2VarName;
    $Arg3Type Arg3 = $Arg3VarName;
    $Arg4Type Arg4 = $Arg4VarName;

    //__ocl_transpose_store_char4x8_common_avx2(ocl_char32* Arg0, char8 Arg1, char8 Arg2, char8 Arg3, char8 Arg4)
    #{__ocl_transpose_store_char4x8_common_avx2}#
    // We store "int8", meaning we store the full matrix in a single store
    int* tmpStoreAdd = (int*)$Arg0VarName;
    vstore8(as_int8(xyzw), 0, tmpStoreAdd);
  }]>;

OclBuiltinImpl transpose_gather_char4x8_avx2 = OclBuiltinImpl<__ocl_gather_transpose4x8, [v4i8],  0, [{
    // Broadcast the loaded values, all but the first element in the register which will be moved there
    int8 xyzw1 = *((int*)$Arg1VarName);                                                  // x1 y1 z1 w1 x1 y1 z1 w1 x1 y1 z1 w1 x1 y1 z1 w1
    int8 xyzw2 = *((int*)$Arg2VarName);                                                  // x2 y2 z2 w2 x2 y2 z2 w2 x2 y2 z2 w2 x2 y2 z2 w2
    int8 xyzw3 = *((int*)$Arg3VarName);                                                  // x3 y3 z3 w3 x3 y3 z3 w3 x3 y3 z3 w3 x3 y3 z3 w3
    int8 xyzw4 = *((int*)$Arg4VarName);                                                  // x4 y4 z4 w4 x4 y4 z4 w4 x4 y4 z4 w4 x4 y4 z4 w4
    int8 xyzw5 = *((int*)$Arg5VarName);                                                  // x5 y5 z5 w5 x5 y5 z5 w5 x5 y5 z5 w5 x5 y5 z5 w5
    int8 xyzw6 = *((int*)$Arg6VarName);                                                  // x6 y6 z6 w6 x6 y6 z6 w6 x6 y6 z6 w6 x6 y6 z6 w6
    int8 xyzw7 = *((int*)$Arg7VarName);                                                  // x7 y7 z7 w7 x7 y7 z7 w7 x7 y7 z7 w7 x7 y7 z7 w7

    int8 xyzw = 0;
    // TODO : Replace this blend built-in with ?: when clang bug will be fixed
    xyzw.s0 = *((int*)$Arg0VarName);                                                  // x0 y0 z0 w0  D  D  D  D  D  D  D  D  D  D  D  D |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D
    xyzw = as_int8(_mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw1, 0x2));           // x0 y0 z0 w0 x1 y1 z1 w1  D  D  D  D  D  D  D  D |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D
    xyzw = as_int8(_mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw2, 0x4));           // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2  D  D  D  D |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D
    xyzw = as_int8(_mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw3, 0x8));           // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 |  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D  D
    xyzw = as_int8(_mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw4, 0x10));          // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4  D  D  D  D  D  D  D  D  D  D  D  D
    xyzw = as_int8(_mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw5, 0x20));          // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5  D  D  D  D  D  D  D  D
    xyzw = as_int8(_mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw6, 0x40));          // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6  D  D  D  D
    xyzw = as_int8(_mm256_blend_epi32((__m256i)xyzw, (__m256i)xyzw7, 0x80));          // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3 | x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
    ocl_char32 Arg0 = (ocl_char32)xyzw;
    $Arg8Type Arg1 = $Arg8VarName;
    $Arg9Type Arg2 = $Arg9VarName;
    $Arg10Type Arg3 = $Arg10VarName;
    $Arg11Type Arg4 = $Arg11VarName;

    //__ocl_load_transpose_char4x8_common_avx2(ocl_char32 Arg0, char8* Arg1, char8* Arg2, char8* Arg3, char8* Arg4)
    #{__ocl_load_transpose_char4x8_common_avx2}#
  }]>;

OclBuiltinImpl transpose_scatter_char4x8_avx2 = OclBuiltinImpl<__ocl_transpose_scatter4x8, [v4i8],  0, [{
    int8 xyzw = 0;
    ocl_char32* Arg0 = (ocl_char32*)&xyzw;
    $Arg8Type Arg1 = $Arg8VarName;
    $Arg9Type Arg2 = $Arg9VarName;
    $Arg10Type Arg3 = $Arg10VarName;
    $Arg11Type Arg4 = $Arg11VarName;

    //__ocl_transpose_store_char4x8_common_avx2(ocl_char32* Arg0, char8 Arg1, char8 Arg2, char8 Arg3, char8 Arg4)
    #{__ocl_transpose_store_char4x8_common_avx2}#
    *((int*)$Arg0VarName) = xyzw.s0;
    *((int*)$Arg1VarName) = xyzw.s1;
    *((int*)$Arg2VarName) = xyzw.s2;
    *((int*)$Arg3VarName) = xyzw.s3;
    *((int*)$Arg4VarName) = xyzw.s4;
    *((int*)$Arg5VarName) = xyzw.s5;
    *((int*)$Arg6VarName) = xyzw.s6;
    *((int*)$Arg7VarName) = xyzw.s7;
  }]>;

// ****************************************************************************
//                                 short4x8
// ****************************************************************************
OclBuiltinImpl transpose_load_short4x8_avx2 = OclBuiltinImpl<__ocl_load_transpose4x8, [v4i16],  0, [{
    short16 Arg0 = vload16(0, (short*)$Arg0VarName);                                      // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3
    short16 Arg1 = vload16(1, (short*)$Arg0VarName);                                      // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
    $Arg1Type Arg2 = $Arg1VarName;
    $Arg2Type Arg3 = $Arg2VarName;
    $Arg3Type Arg4 = $Arg3VarName;
    $Arg4Type Arg5 = $Arg4VarName;

    //__ocl_load_transpose_common_short4x8_avx2(short16 Arg0, short16 Arg1, short8* Arg2, short8* Arg3, short8* Arg4, short8* Arg5)
    #{__ocl_load_transpose_common_short4x8_avx2}#
  }]>;

OclBuiltinImpl transpose_store_short4x8_avx2 = OclBuiltinImpl<__ocl_transpose_store4x8, [v4i16],  0, [{
    short16 xyzw0, xyzw1;
    short16* Arg0 = &xyzw0;
    short16* Arg1 = &xyzw1;
    $Arg1Type Arg2 = $Arg1VarName;
    $Arg2Type Arg3 = $Arg2VarName;
    $Arg3Type Arg4 = $Arg3VarName;
    $Arg4Type Arg5 = $Arg4VarName;

    //__ocl_transpose_store_common_short4x8_avx2(short16* Arg0, short16* Arg1, short8 Arg2, short8 Arg3, short8 Arg4, short8 Arg5)
    #{__ocl_transpose_store_common_short4x8_avx2}#
    vstore16(xyzw0, 0, (short*)$Arg0VarName);
    vstore16(xyzw1, 1, (short*)$Arg0VarName);
  }]>;

OclBuiltinImpl transpose_gather_short4x8_avx2 = OclBuiltinImpl<__ocl_gather_transpose4x8, [v4i16],  0, [{
    long4 xyzw1 = *((long*)$Arg1VarName);                                                // x1 y1 z1 w1 x1 y1 z1 w1 x1 y1 z1 w1 x1 y1 z1 w1
    long4 xyzw2 = *((long*)$Arg2VarName);                                                // x2 y2 z2 w2 x2 y2 z2 w2 x2 y2 z2 w2 x2 y2 z2 w2
    long4 xyzw3 = *((long*)$Arg3VarName);                                                // x3 y3 z3 w3 x3 y3 z3 w3 x3 y3 z3 w3 x3 y3 z3 w3
    long4 xyzw5 = *((long*)$Arg5VarName);                                                // x5 y5 z5 w5 x5 y5 z5 w5 x5 y5 z5 w5 x5 y5 z5 w5
    long4 xyzw6 = *((long*)$Arg6VarName);                                                // x6 y6 z6 w6 x6 y6 z6 w6 x6 y6 z6 w6 x6 y6 z6 w6
    long4 xyzw7 = *((long*)$Arg7VarName);                                                // x7 y7 z7 w7 x7 y7 z7 w7 x7 y7 z7 w7 x7 y7 z7 w7

    long4 xyzw0123;
    xyzw0123.s0 = *((long*)$Arg0VarName);                                                // x0 y0 z0 w0  D  D  D  D  D  D  D  D  D  D  D  D
    xyzw0123 = as_long4(_mm256_blend_epi32((__m256i)xyzw0123, (__m256i)xyzw1, 0xC));  // x0 y0 z0 w0 x1 y1 z1 w1  D  D  D  D  D  D  D  D
    xyzw0123 = as_long4(_mm256_blend_epi32((__m256i)xyzw0123, (__m256i)xyzw2, 0x30)); // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2  D  D  D  D
    xyzw0123 = as_long4(_mm256_blend_epi32((__m256i)xyzw0123, (__m256i)xyzw3, 0xC0)); // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3

    long4 xyzw4567;
    xyzw4567.s0 = *((long*)$Arg4VarName);                                                // x4 y4 z4 w4  D  D  D  D  D  D  D  D  D  D  D  D
    xyzw4567 = as_long4(_mm256_blend_epi32((__m256i)xyzw4567, (__m256i)xyzw5, 0xC));  // x4 y4 z4 w4 x5 y5 z5 w5  D  D  D  D  D  D  D  D
    xyzw4567 = as_long4(_mm256_blend_epi32((__m256i)xyzw4567, (__m256i)xyzw6, 0x30)); // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6  D  D  D  D
    xyzw4567 = as_long4(_mm256_blend_epi32((__m256i)xyzw4567, (__m256i)xyzw7, 0xC0)); // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
    short16 Arg0 = as_short16(xyzw0123);
    short16 Arg1 = as_short16(xyzw4567);
    $Arg8Type Arg2 = $Arg8VarName;
    $Arg9Type Arg3 = $Arg9VarName;
    $Arg10Type Arg4 = $Arg10VarName;
    $Arg11Type Arg5 = $Arg11VarName;

    //__ocl_load_transpose_common_short4x8_avx2(short16 Arg0, short16 Arg1, short8* Arg2, short8* Arg3, short8* Arg4, short8* Arg5)
    #{__ocl_load_transpose_common_short4x8_avx2}#
  }]>;

OclBuiltinImpl transpose_scatter_short4x8_avx2 = OclBuiltinImpl<__ocl_transpose_scatter4x8, [v4i16],  0, [{
    short16 xyzw0, xyzw1;
    short16* Arg0 = &xyzw0;
    short16* Arg1 = &xyzw1;
    $Arg8Type Arg2 = $Arg8VarName;
    $Arg9Type Arg3 = $Arg9VarName;
    $Arg10Type Arg4 = $Arg10VarName;
    $Arg11Type Arg5 = $Arg11VarName;

    //__ocl_transpose_store_common_short4x8_avx2(short16* Arg0, short16* Arg1, short8 Arg2, short8 Arg3, short8 Arg4, short8 Arg5)
    #{__ocl_transpose_store_common_short4x8_avx2}#
    *((long*)$Arg0VarName) = as_long4(xyzw0).s0;                                        // x0 y0 z0 w0
    *((long*)$Arg1VarName) = as_long4(xyzw0).s1;                                        // x1 y1 z1 w1
    *((long*)$Arg2VarName) = as_long4(xyzw0).s2;                                        // x2 y2 z2 w2
    *((long*)$Arg3VarName) = as_long4(xyzw0).s3;                                        // x3 y3 z3 w3
    *((long*)$Arg4VarName) = as_long4(xyzw1).s0;                                        // x4 y4 z4 w4
    *((long*)$Arg5VarName) = as_long4(xyzw1).s1;                                        // x5 y5 z5 w5
    *((long*)$Arg6VarName) = as_long4(xyzw1).s2;                                        // x6 y6 z6 w6
    *((long*)$Arg7VarName) = as_long4(xyzw1).s3;                                        // x7 y7 z7 w7
  }]>;

// ****************************************************************************
//                                 int4x4
// ****************************************************************************
OclBuiltinImpl transpose_load_int4x4_avx2 = OclBuiltinImpl<__ocl_load_transpose4x4, [v4i32],  0, [{
    // We load "int8", meaning we load the full matrix in 2 loads
    int* tmpLoadAdd = (int*)$Arg0VarName;
    int8 Arg0 = vload8(0, tmpLoadAdd);                                                // x0 y0 z0 w0 x1 y1 z1 w1 xyzw01
    int8 Arg1 = vload8(1, tmpLoadAdd);                                                // x2 y2 z2 w2 x3 y3 z3 w3 xyzw23
    int8* Arg2;
    int8* Arg3;
    bool Arg4;
    $Arg1Type Arg5 = $Arg1VarName;
    $Arg2Type Arg6 = $Arg2VarName;
    $Arg3Type Arg7 = $Arg3VarName;
    $Arg4Type Arg8 = $Arg4VarName;

    //__ocl_load_transpose_common_int4x4_avx2(int8 Arg0, int8 Arg1, bool Arg4, int4* Arg5, int4* Arg6, int4* Arg7, int4* Arg8)
    #{__ocl_load_transpose_common_int4x4_avx2}#
  }]>;

OclBuiltinImpl transpose_store_int4x4_avx2 = OclBuiltinImpl<__ocl_transpose_store4x4, [v4i32],  0, [{
    int8 Arg0 = {$Arg1VarName, $Arg2VarName};                                                           // x0 x1 x2 x3 y0 y1 y2 y3 xy
    int8 Arg1 = {$Arg3VarName, $Arg4VarName};                                                           // z0 z1 z2 z3 w0 w1 w2 w3 zw
    int8 xyzw01;
    int8 xyzw23;

    int8* Arg2 = &xyzw01;
    int8* Arg3 = &xyzw23;
    bool Arg4 = false;

    //__ocl_transpose_common4_int4x4_avx2(int8 Arg0, int8 Arg1, int8* Arg2, int8* Arg3, bool Arg4)
    #{__ocl_transpose_common4_int4x4_avx2}#
    // We store "int8", meaning we store the full matrix in 2 stores
    int* tmpStoreAdd = (int*)$Arg0VarName;
    vstore8(xyzw01, 0, tmpStoreAdd);
    vstore8(xyzw23, 1, tmpStoreAdd);
  }]>;

OclBuiltinImpl transpose_gather_int4x4_avx2 = OclBuiltinImpl<__ocl_gather_transpose4x4, [v4i32],  0, [{
    int8 Arg0 = {*$Arg0VarName, *$Arg1VarName};                                               // x0 y0 z0 w0 x1 y1 z1 w1
    int8 Arg1 = {*$Arg2VarName, *$Arg3VarName};                                               // x2 y2 z2 w2 x3 y3 z3 w3
    int8* Arg2;
    int8* Arg3;
    bool Arg4;
    $Arg4Type Arg5 = $Arg4VarName;
    $Arg5Type Arg6 = $Arg5VarName;
    $Arg6Type Arg7 = $Arg6VarName;
    $Arg7Type Arg8 = $Arg7VarName;

    //__ocl_load_transpose_common_int4x4_avx2(int8 Arg0, int8 Arg1, bool Arg4, int4* Arg5, int4* Arg6, int4* Arg7, int4* Arg8)
    #{__ocl_load_transpose_common_int4x4_avx2}#
  }]>;

OclBuiltinImpl transpose_scatter_int4x4_avx2 = OclBuiltinImpl<__ocl_transpose_scatter4x4, [v4i32],  0, [{
    int8 Arg0 = {$Arg4VarName, $Arg5VarName};                                                           // x0 x1 x2 x3 y0 y1 y2 y3
    int8 Arg1 = {$Arg6VarName, $Arg7VarName};                                                           // z0 z1 z2 z3 w0 w1 w2 w3

    int8 xyzw01;
    int8 xyzw23;
    int8* Arg2 = &xyzw01;
    int8* Arg3 =  &xyzw23;
    bool Arg4 = false;

    //__ocl_transpose_common4_int4x4_avx2(int8 Arg0, int8 Arg1, int8* Arg2, int8* Arg3, bool Arg4)
    #{__ocl_transpose_common4_int4x4_avx2}#
    *$Arg0VarName = xyzw01.lo;
    *$Arg1VarName = xyzw01.hi;
    *$Arg2VarName = xyzw01.lo;
    *$Arg3VarName = xyzw01.hi;
  }]>;

// ****************************************************************************
//                                 int4x8
// ****************************************************************************
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
    $Arg8Type Arg4 = $Arg8VarName;
    $Arg9Type Arg5 = $Arg9VarName;
    $Arg10Type Arg6 = $Arg10VarName;
    $Arg11Type Arg7 = $Arg11VarName;

    //__ocl_transpose_common_int4x8_avx2(int8 Arg0, int8 Arg1, int8 Arg2, int8 Arg3, int8* Arg4, int8* Arg5, int8* Arg6, int8* Arg7)
    #{__ocl_transpose_common_int4x8_avx2}#
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
    $Arg8Type Arg0 = $Arg8VarName;
    $Arg9Type Arg1 = $Arg9VarName;
    $Arg10Type Arg2 = $Arg10VarName;
    $Arg11Type Arg3 = $Arg11VarName;
    int8* Arg4 = &xyzw04;
    int8* Arg5 = &xyzw15;
    int8* Arg6 = &xyzw26;
    int8* Arg7 = &xyzw37;

    //__ocl_transpose_common_int4x8_avx2(int8 Arg0, int8 Arg1, int8 Arg2, int8 Arg3, int8* Arg4, int8* Arg5, int8* Arg6, int8* Arg7)
    #{__ocl_transpose_common_int4x8_avx2}#
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
//                                 float4x4
// ****************************************************************************
OclBuiltinImpl transpose_load_float4x4_avx2 = OclBuiltinImpl<__ocl_load_transpose4x4, [v4f32],  0, [{
    // We load "float8", meaning we load the full matrix in 2 loads
    float* tmpLoadAdd = (float*)$Arg0VarName;
    float8 Arg0 = vload8(0, tmpLoadAdd);                                              // x0 y0 z0 w0 x1 y1 z1 w1
    float8 Arg1 = vload8(1, tmpLoadAdd);                                              // x2 y2 z2 w2 x3 y3 z3 w3
    $Arg1Type Arg5 = $Arg1VarName;
    $Arg2Type Arg6 = $Arg2VarName;
    $Arg3Type Arg7 = $Arg3VarName;
    $Arg4Type Arg8 = $Arg4VarName;

    //__ocl_load_transpose_common_float4x4_avx2(float8 Arg0,float8  Arg1,float4* Arg5,float4*  Arg6,float4*  Arg7,float4*  Arg8)
    #{__ocl_load_transpose_common_float4x4_avx2}#
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
    float8* Arg2 = &xyzw01;
    float8* Arg3 = &xyzw23;
    bool Arg4 = false;

    //__ocl_transpose_common4_float4x4_avx2(float8 Arg0, float8 Arg1, float8* Arg2, float8* Arg3, bool Arg4)
    #{__ocl_transpose_common4_float4x4_avx2}#
    // We store "float8", meaning we store the full matrix in 2 stores
    float* tmpStoreAdd = (float*)$Arg0VarName;
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

    Arg0.lo = *$Arg0VarName;                                                             // x0 y0 z0 w0 D  D  D  D
    Arg0.hi = *$Arg1VarName;                                                             // x0 y0 z0 w0 x4 y4 z4 w4
    Arg1.lo = *$Arg2VarName;                                                             // x0 y0 z0 w0 D  D  D  D
    Arg1.hi = *$Arg3VarName;                                                             // x0 y0 z0 w0 x4 y4 z4 w4
    $Arg4Type Arg5 = $Arg4VarName;
    $Arg5Type Arg6 = $Arg5VarName;
    $Arg6Type Arg7 = $Arg6VarName;
    $Arg7Type Arg8 = $Arg7VarName;

    //__ocl_load_transpose_common_float4x4_avx2(float8 Arg0,float8  Arg1,float4* Arg5,float4*  Arg6,float4*  Arg7,float4*  Arg8)
    #{__ocl_load_transpose_common_float4x4_avx2}#
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
    float8* Arg2 = &xyzw01;
    float8* Arg3 = &xyzw23;
    bool Arg4 = false;

    //__ocl_transpose_common4_float4x4_avx2(float8 Arg0, float8 Arg1, float8* Arg2, float8* Arg3, bool Arg4)
    #{__ocl_transpose_common4_float4x4_avx2}#
    *$Arg0VarName = xyzw01.lo;
    *$Arg1VarName = xyzw01.hi;
    *$Arg2VarName = xyzw23.lo;
    *$Arg3VarName = xyzw23.hi;
  }]>;
