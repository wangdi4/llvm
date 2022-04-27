//AVX
/// STYLE NOTE: Comments at same line of code are arranged to start at column 85 at the output final file

// ****************************************************************************
//                                 char4x8
// ****************************************************************************
OclBuiltinImpl transpose_masked_load_char4x8_avx = OclBuiltinImpl<__ocl_masked_load_transpose4x8, [v4i8], 0, [{
    ocl_char32 xyzw;
    __ocl_masked_load_char_4x8($Arg0VarName, (__private char4*)&xyzw, $Arg5VarName);
    __ocl_load_transpose_char_4x8((__private char4*)&xyzw, $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName);
  }]>;

OclBuiltinImpl transpose_masked_store_char4x8_avx = OclBuiltinImpl<__ocl_masked_transpose_store4x8, [v4i8], 0, [{
    ocl_char32 xyzw;
    __ocl_transpose_store_char_4x8((__private char4*)&xyzw, $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName);
    __ocl_masked_store_char_4x8($Arg0VarName, (__private char4*)&xyzw, $Arg5VarName);
  }]>;

// ****************************************************************************
//                                 short4x8
// ****************************************************************************

/// @brief Receives short8x4 matrix as 2 halfs (2 short4x4 matrixes) using 2 short16,
///        transposes it and outputs the rows of the transposed matrix.
/// @param Arg0    - short16 - Upper part of the short8x4 matrix to be transposed
/// @param Arg1    - short16 - Lower part of the short8x4 matrix to be transposed
/// @param Arg2-5  - short8* - Rows 0-3 of the transposed matrix
OclBuiltinImpl transpose_load_short4x8_avx_helper = OclBuiltinImpl<__ocl_load_transpose4x8_helper, [v4i16], 0, [{
    ushort16 maskUnpackLo = {0, 16, 1, 17, 2, 18, 3, 19,  8, 24,  9, 25, 10, 26, 11, 27};
    ushort16 maskUnpackHi = {4, 20, 5, 21, 6, 22, 7, 23, 12, 28, 13, 29, 14, 30, 15, 31};
    uint8 maskPermute = {0, 4, 1, 5, 2, 6, 3, 7};
    short16 tmp1 = __ocl_helper_shuffle2($Arg0VarName, $Arg1VarName, maskUnpackLo); // x0 x4 y0 y4 z0 z4 w0 w4 x2 x6 y2 y6 z2 z6 w2 w6
    short16 tmp2 = __ocl_helper_shuffle2($Arg0VarName, $Arg1VarName, maskUnpackHi); // x1 x5 y1 y5 z1 z5 w1 w5 x3 x7 y3 y7 z3 z7 w3 w7
    short16 tmp3 = __ocl_helper_shuffle2(tmp1, tmp2, maskUnpackLo);                 // x0 x1 x4 x5 y0 y1 y4 y5 x2 x3 x6 x7 y2 y3 y6 y7
    short16 tmp4 = __ocl_helper_shuffle2(tmp1, tmp2, maskUnpackHi);                 // z0 z1 z4 z5 w0 w1 w4 w5 z2 z3 z6 z7 w2 w3 w6 w7
    short16 tmp5 = as_short16(__ocl_helper_shuffle(as_float8(tmp3), maskPermute));  // x0 x1 x2 x3 x4 x5 x6 x7 y0 y1 y2 y3 y4 y5 y6 y7
    short16 tmp6 = as_short16(__ocl_helper_shuffle(as_float8(tmp4), maskPermute));  // z0 z1 z2 z3 z4 z5 z6 z7 w0 w1 w2 w3 w4 w5 w6 w7
    *$Arg2VarName = tmp5.lo;                                                        // x0 x1 x2 x3 x4 x5 x6 x7
    *$Arg3VarName = tmp5.hi;                                                        // y0 y1 y2 y3 y4 y5 y6 y7
    *$Arg4VarName = tmp6.lo;                                                        // z0 z1 z2 z3 z4 z5 z6 z7
    *$Arg5VarName = tmp6.hi;                                                        // w0 w1 w2 w3 w4 w5 w6 w7
  }]>;

/// @brief Receives short4x8 matrix as 4 matrix rows, transposes it and outputs the matrix
///        2 halfs (2 short4x4 matrixes) using 2 short16, which create one short4x8 matrix
/// @param Arg0    - short16* - Upper part of the transposed short4x8
/// @param Arg1    - short16* - Lower part of the transposed short4x8
/// @param Arg2-5  - short8   - Rows 0-3 of the matrix to be transposed
OclBuiltinImpl transpose_store_short4x8_sse4_helper = OclBuiltinImpl<__ocl_transpose_store4x8_helper, [v4i16], 0, [{
    ushort16 low16 = {0, 8, 1, 9, 2, 10, 3, 11, 4, 12, 5, 13, 6, 14, 7, 15};
    float8 xy = as_float8(__ocl_helper_shuffle2($Arg2VarName, $Arg3VarName, low16)); // x0 y0 x1 y1 x2 y2 x3 y3 x4 y4 x5 y5 x6 y6 x7 y7
    float8 zw = as_float8(__ocl_helper_shuffle2($Arg4VarName, $Arg5VarName, low16)); // z0 w0 z1 w1 z2 w2 z3 w3 z4 w4 z5 w5 z6 w6 z7 w7
    uint8 low8  = {0, 8, 1, 9, 2, 10, 3, 11};
    uint8 high8 = {4, 12, 5, 13, 6, 14, 7, 15};
    *$Arg0VarName = as_short16(__ocl_helper_shuffle2(xy, zw, low8));                 // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3
    *$Arg1VarName = as_short16(__ocl_helper_shuffle2(xy, zw, high8));                // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
  }]>;

OclBuiltinImpl transpose_load_short4x8_avx = OclBuiltinImpl<__ocl_load_transpose4x8, [v4i16], 0, [{
    short16 vLoaded0 = vload16(0, (const __private short*)$Arg0VarName);            // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3
    short16 vLoaded1 = vload16(1, (const __private short*)$Arg0VarName);            // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7

    __ocl_load_transpose4x8_helper(vLoaded0, vLoaded1, $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName);
  }]>;

OclBuiltinImpl transpose_store_short4x8_avx = OclBuiltinImpl<__ocl_transpose_store4x8, [v4i16], 0, [{
    short16 xyzw0, xyzw1;
    __private short16* pStore0 = &xyzw0;
    __private short16* pStore1 = &xyzw1;

    __ocl_transpose_store4x8_helper(pStore0, pStore1, $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName);

    vstore16(xyzw0, 0, (__private short*)$Arg0VarName);
    vstore16(xyzw1, 1, (__private short*)$Arg0VarName);
  }]>;

OclBuiltinImpl transpose_gather_short4x8_avx = OclBuiltinImpl<__ocl_gather_transpose4x8, [v4i16], 0, [{
    double4 xyzw1 = *((__private double*)$Arg1VarName);                              // x1 y1 z1 w1 x1 y1 z1 w1 x1 y1 z1 w1 x1 y1 z1 w1
    double4 xyzw2 = *((__private double*)$Arg2VarName);                              // x2 y2 z2 w2 x2 y2 z2 w2 x2 y2 z2 w2 x2 y2 z2 w2
    double4 xyzw3 = *((__private double*)$Arg3VarName);                              // x3 y3 z3 w3 x3 y3 z3 w3 x3 y3 z3 w3 x3 y3 z3 w3
    double4 xyzw5 = *((__private double*)$Arg5VarName);                              // x5 y5 z5 w5 x5 y5 z5 w5 x5 y5 z5 w5 x5 y5 z5 w5
    double4 xyzw6 = *((__private double*)$Arg6VarName);                              // x6 y6 z6 w6 x6 y6 z6 w6 x6 y6 z6 w6 x6 y6 z6 w6
    double4 xyzw7 = *((__private double*)$Arg7VarName);                              // x7 y7 z7 w7 x7 y7 z7 w7 x7 y7 z7 w7 x7 y7 z7 w7

    double4 xyzw0123;
    xyzw0123.s0 = *((__private double*)$Arg0VarName);                                 // x0 y0 z0 w0  D  D  D  D  D  D  D  D  D  D  D  D
    xyzw0123 = as_double4(_mm256_blend_pd((__m256d)xyzw0123, (__m256d)xyzw1, 0x2));   // x0 y0 z0 w0 x1 y1 z1 w1  D  D  D  D  D  D  D  D
    xyzw0123 = as_double4(_mm256_blend_pd((__m256d)xyzw0123, (__m256d)xyzw2, 0x4));   // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2  D  D  D  D
    xyzw0123 = as_double4(_mm256_blend_pd((__m256d)xyzw0123, (__m256d)xyzw3, 0x8));   // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3

    double4 xyzw4567;
    xyzw4567.s0 = *((__private double*)$Arg4VarName);                                 // x4 y4 z4 w4  D  D  D  D  D  D  D  D  D  D  D  D
    xyzw4567 = as_double4(_mm256_blend_pd((__m256d)xyzw4567, (__m256d)xyzw5, 0x2));   // x4 y4 z4 w4 x5 y5 z5 w5  D  D  D  D  D  D  D  D
    xyzw4567 = as_double4(_mm256_blend_pd((__m256d)xyzw4567, (__m256d)xyzw6, 0x4));   // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6  D  D  D  D
    xyzw4567 = as_double4(_mm256_blend_pd((__m256d)xyzw4567, (__m256d)xyzw7, 0x8));   // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
    short16 vLoaded0 = as_short16(xyzw0123);
    short16 vLoaded1 = as_short16(xyzw4567);

    __ocl_load_transpose4x8_helper(vLoaded0, vLoaded1, $Arg8VarName, $Arg9VarName, $Arg10VarName, $Arg11VarName);
  }]>;

OclBuiltinImpl transpose_scatter_short4x8_avx = OclBuiltinImpl<__ocl_transpose_scatter4x8, [v4i16], 0, [{
    short16 xyzw0, xyzw1;
    __private short16* pStore0 = &xyzw0;
    __private short16* pStore1 = &xyzw1;

    __ocl_transpose_store4x8_helper(pStore0, pStore1, $Arg8VarName, $Arg9VarName, $Arg10VarName, $Arg11VarName);

    *((__private double*)$Arg0VarName) = as_double4(xyzw0).s0;                         // x0 y0 z0 w0
    *((__private double*)$Arg1VarName) = as_double4(xyzw0).s1;                         // x1 y1 z1 w1
    *((__private double*)$Arg2VarName) = as_double4(xyzw0).s2;                         // x2 y2 z2 w2
    *((__private double*)$Arg3VarName) = as_double4(xyzw0).s3;                         // x3 y3 z3 w3
    *((__private double*)$Arg4VarName) = as_double4(xyzw1).s0;                         // x4 y4 z4 w4
    *((__private double*)$Arg5VarName) = as_double4(xyzw1).s1;                         // x5 y5 z5 w5
    *((__private double*)$Arg6VarName) = as_double4(xyzw1).s2;                         // x6 y6 z6 w6
    *((__private double*)$Arg7VarName) = as_double4(xyzw1).s3;                         // x7 y7 z7 w7
  }]>;

// ****************************************************************************
//                                 int4x8
// ****************************************************************************
OclBuiltinImpl transpose_load_int4x8_avx = OclBuiltinImpl<__ocl_load_transpose4x8, [v4i32], 0, [{
    int4 x0;
    int4 y0;
    int4 z0;
    int4 w0;

    __ocl_load_transpose_int_4x4($Arg0VarName,
                                 &x0, &y0, &z0, &w0);                                 // x0 x1 x2 x3
                                                                                      // y0 y1 y2 y3
                                                                                      // z0 z1 z2 z3
                                                                                      // w0 w1 w2 w3

    int4 x1;
    int4 y1;
    int4 z1;
    int4 w1;

    __private int4* pLoadAdd1 = &($Arg0VarName[4]);

    __ocl_load_transpose_int_4x4(pLoadAdd1,
                                 &x1, &y1, &z1, &w1);                                 // x4 x5 x6 x7
                                                                                      // y4 y5 y6 y7
                                                                                      // z4 z5 z6 z7
                                                                                      // w4 w5 w6 w7

    // TODO : Replace these shuffle builtins with *xOut = {x0, x1} when clang bug will be fixed
    uint8 concatVectors4 = {0, 1, 2, 3, 4, 5, 6, 7};
    *$Arg1VarName = __ocl_helper_shuffle2(x0, x1, concatVectors4);                    // x0 x1 x2 x3 x4 x5 x6 x7
    *$Arg2VarName = __ocl_helper_shuffle2(y0, y1, concatVectors4);                    // y0 y1 y2 y3 y4 y5 y6 y7
    *$Arg3VarName = __ocl_helper_shuffle2(z0, z1, concatVectors4);                    // z0 z1 z2 z3 z4 z5 z6 z7
    *$Arg4VarName = __ocl_helper_shuffle2(w0, w1, concatVectors4);                    // w0 w1 w2 w3 w4 w5 w6 w7
  }]>;

OclBuiltinImpl transpose_store_int4x8_avx = OclBuiltinImpl<__ocl_transpose_store4x8, [v4i32], 0, [{
    int4 x0 = $Arg1VarName.lo;
    int4 y0 = $Arg2VarName.lo;
    int4 z0 = $Arg3VarName.lo;
    int4 w0 = $Arg4VarName.lo;

    __ocl_transpose_store_int_4x4($Arg0VarName,
                                  x0, y0, z0, w0);                                    // x0 y0 z0 w0
                                                                                      // x1 y1 z1 w1
                                                                                      // x2 y2 z2 w2
                                                                                      // x3 y3 z3 w3
    int4 x1 = $Arg1VarName.hi;
    int4 y1 = $Arg2VarName.hi;
    int4 z1 = $Arg3VarName.hi;
    int4 w1 = $Arg4VarName.hi;

    __private int4* pStoreAdd1 = &($Arg0VarName[4]);

    __ocl_transpose_store_int_4x4(pStoreAdd1,
                                  x1, y1, z1, w1);                                    // x4 y4 z4 w4
                                                                                      // x5 y5 z5 w5
                                                                                      // x6 y6 z6 w6
                                                                                      // x7 y7 z7 w7
  }]>;

OclBuiltinImpl transpose_gather_int4x8_avx = OclBuiltinImpl<__ocl_gather_transpose4x8, [v4i32], 0, [{
    int4 x0;
    int4 y0;
    int4 z0;
    int4 w0;

    __ocl_gather_transpose_int_4x4($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName,
                                   &x0, &y0, &z0, &w0);                               // x0 x1 x2 x3
                                                                                      // y0 y1 y2 y3
                                                                                      // z0 z1 z2 z3
                                                                                      // w0 w1 w2 w3
    int4 x1;
    int4 y1;
    int4 z1;
    int4 w1;

    __ocl_gather_transpose_int_4x4($Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName,
                                   &x1, &y1, &z1, &w1);                               // x4 x5 x6 x7
                                                                                      // y4 y5 y6 y7
                                                                                      // z4 z5 z6 z7
                                                                                      // w4 w5 w6 w7

    // TODO : Replace these shuffle builtins with *xOut = {x0, x1} when clang bug will be fixed
    uint8 concatVectors4 = {0, 1, 2, 3, 4, 5, 6, 7};
    *$Arg8VarName = __ocl_helper_shuffle2(x0, x1, concatVectors4);                    // x0 x1 x2 x3 x4 x5 x6 x7
    *$Arg9VarName = __ocl_helper_shuffle2(y0, y1, concatVectors4);                    // y0 y1 y2 y3 y4 y5 y6 y7
    *$Arg10VarName = __ocl_helper_shuffle2(z0, z1, concatVectors4);                   // z0 z1 z2 z3 z4 z5 z6 z7
    *$Arg11VarName = __ocl_helper_shuffle2(w0, w1, concatVectors4);                   // w0 w1 w2 w3 w4 w5 w6 w7
  }]>;

OclBuiltinImpl transpose_scatter_int4x8_avx = OclBuiltinImpl<__ocl_transpose_scatter4x8, [v4i32], 0, [{
    int4 x0 = $Arg8VarName.s0123;
    int4 y0 = $Arg9VarName.s0123;
    int4 z0 = $Arg10VarName.s0123;
    int4 w0 = $Arg11VarName.s0123;

    __ocl_transpose_scatter_int_4x4($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName,
                                    x0, y0, z0, w0);                                  // x0 y0 z0 w0
                                                                                      // x1 y1 z1 w1
                                                                                      // x2 y2 z2 w2
                                                                                      // x3 y3 z3 w3

    int4 x1 = $Arg8VarName.s4567;
    int4 y1 = $Arg9VarName.s4567;
    int4 z1 = $Arg10VarName.s4567;
    int4 w1 = $Arg11VarName.s4567;

    __ocl_transpose_scatter_int_4x4($Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName,
                                    x1, y1, z1, w1);                                  // x4 y4 z4 w4
                                                                                      // x5 y5 z5 w5
                                                                                      // x6 y6 z6 w6
                                                                                      // x7 y7 z7 w7
  }]>;

// ****************************************************************************
//                                 float2x8
// ****************************************************************************
OclBuiltinImpl transpose_load_float2x8_avx = OclBuiltinImpl<__ocl_load_transpose2x8, [v2f32],  0, [{
    const __private float* tmpLoadAdd = (const __private float*)$Arg0VarName;
    float8 Arg0 = vload8(0, tmpLoadAdd);
    float8 Arg1 = vload8(1, tmpLoadAdd);

    uint8 firstArray = {0, 2, 4, 6, 8, 10, 12, 14};
    uint8 secondArray = {1, 3, 5, 7, 9, 11, 13, 15};
    *$Arg1VarName = __ocl_helper_shuffle2(Arg0, Arg1, firstArray);
    *$Arg2VarName = __ocl_helper_shuffle2(Arg0, Arg1, secondArray);
  }]>;

// ****************************************************************************
//                                 float4x8
// ****************************************************************************
OclBuiltinImpl transpose_load_float4x8_avx = OclBuiltinImpl<__ocl_load_transpose4x8, [v4f32], 0, [{
    __ocl_transpose4x8( $Arg0VarName[0], $Arg0VarName[1], $Arg0VarName[2], $Arg0VarName[3],
                        $Arg0VarName[4], $Arg0VarName[5], $Arg0VarName[6], $Arg0VarName[7],
                        $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName);
  }]>;

OclBuiltinImpl transpose_store_float4x8_avx = OclBuiltinImpl<__ocl_transpose_store4x8, [v4f32], 0, [{
    __ocl_transpose8x4( $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName,
                        &($Arg0VarName[0]), &($Arg0VarName[1]), &($Arg0VarName[2]), &($Arg0VarName[3]),
                        &($Arg0VarName[4]), &($Arg0VarName[5]), &($Arg0VarName[6]), &($Arg0VarName[7]));
  }]>;

OclBuiltinImpl transpose_gather_float4x8_avx = OclBuiltinImpl<__ocl_gather_transpose4x8, [v4f32], 0, [{
    __ocl_transpose4x8( *$Arg0VarName, *$Arg1VarName, *$Arg2VarName, *$Arg3VarName, *$Arg4VarName, *$Arg5VarName, *$Arg6VarName, *$Arg7VarName,
                        $Arg8VarName, $Arg9VarName, $Arg10VarName, $Arg11VarName);
  }]>;

OclBuiltinImpl transpose_scatter_float4x8_avx = OclBuiltinImpl<__ocl_transpose_scatter4x8, [v4f32], 0, [{
    __ocl_transpose8x4( $Arg8VarName, $Arg9VarName, $Arg10VarName, $Arg11VarName,
                        $Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName,
                        $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName);
  }]>;

OclBuiltinImpl transpose_masked_load_float4x8_avx = OclBuiltinImpl<__ocl_masked_load_transpose4x8, [v4f32], 0, [{
    __private float4 xyzw[8] = {0};
    __ocl_masked_load_float_4x8($Arg0VarName, xyzw, $Arg5VarName);
    __ocl_load_transpose_$Arg0BaseType_4x8(xyzw, $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName);
  }]>;

OclBuiltinImpl transpose_masked_store_float4x8_avx = OclBuiltinImpl<__ocl_masked_transpose_store4x8, [v4f32], 0, [{
    __private float4 xyzw[8];
    __ocl_transpose_store_$Arg0BaseType_4x8(xyzw, $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName);
    __ocl_masked_store_float_4x8($Arg0VarName, xyzw, $Arg5VarName);
  }]>;


// ****************************************************************************
//                                 helper function
// ****************************************************************************
OclBuiltinImpl __ocl_transpose4x8_avx =    OclBuiltinImpl    <__ocl_transpose4x8, [v4f32], 0, [{
    // TODO : Replace this shuffle with { xyzw0, xyzw4} instead of shuffle builtin
    // when clang bug will be fixed
    uint8 concatVectors4 = {0, 1, 2, 3, 4, 5, 6, 7};
    float8 xyzw04 = __ocl_helper_shuffle2($Arg0VarName, $Arg4VarName, concatVectors4);              // x0 y0 z0 w0 x4 y4 z4 w4
    float8 xyzw15 = __ocl_helper_shuffle2($Arg1VarName, $Arg5VarName, concatVectors4);              // x1 y1 z1 w1 x5 y5 z5 w5
    float8 xyzw26 = __ocl_helper_shuffle2($Arg2VarName, $Arg6VarName, concatVectors4);              // x2 y2 z2 w2 x6 y6 z6 w6
    float8 xyzw37 = __ocl_helper_shuffle2($Arg3VarName, $Arg7VarName, concatVectors4);              // x3 y3 z3 w3 x7 y7 z7 w7

    uint8 lowLane8 = {0, 8, 1, 9, 4, 12, 5, 13};
    uint8 highLane8 = {2, 10, 3, 11, 6, 14, 7, 15};

    float8 xy0246 = __ocl_helper_shuffle2(xyzw04, xyzw26, lowLane8);                  // x0 x2 y0 y2 x4 x6 y4 y6
    float8 zw0246 = __ocl_helper_shuffle2(xyzw04, xyzw26, highLane8);                 // z0 z2 w0 w2 z4 z6 w4 w6
    float8 xy1357 = __ocl_helper_shuffle2(xyzw15, xyzw37, lowLane8);                  // x1 x3 y1 y3 x5 x7 y5 y7
    float8 zw1357 = __ocl_helper_shuffle2(xyzw15, xyzw37, highLane8);                 // z1 z3 w1 w3 z5 z7 w5 w7

    *$Arg8VarName = __ocl_helper_shuffle2(xy0246, xy1357, lowLane8);                          // x0 x1 x2 x3 x4 x5 x6 x7
    *$Arg9VarName = __ocl_helper_shuffle2(xy0246, xy1357, highLane8);                         // y0 y1 y2 y3 y4 y5 y6 y7
    *$Arg10VarName = __ocl_helper_shuffle2(zw0246, zw1357, lowLane8);                          // z0 z1 z2 z3 z4 z5 z6 z7
    *$Arg11VarName = __ocl_helper_shuffle2(zw0246, zw1357, highLane8);                         // w0 w1 w2 w3 w4 w5 w6 w7
  }]>;

OclBuiltinImpl __ocl_transpose8x4_avx =    OclBuiltinImpl    <__ocl_transpose8x4, [v4f32], 0, [{
    uint8 lowLane8 = {0, 8, 1, 9, 4, 12, 5, 13};
    uint8 highLane8 = {2, 10, 3, 11, 6, 14, 7, 15};

    float8 xy0145 = __ocl_helper_shuffle2($Arg0VarName, $Arg1VarName, lowLane8);                      // x0 y0 x1 y1 x4 y4 x5 y5
    float8 xy2367 = __ocl_helper_shuffle2($Arg0VarName, $Arg1VarName, highLane8);                     // x2 y2 x3 y3 x6 y6 x7 y7
    float8 zw0145 = __ocl_helper_shuffle2($Arg2VarName, $Arg3VarName, lowLane8);                      // z0 w0 z1 w1 z4 w4 z5 w5
    float8 zw2367 = __ocl_helper_shuffle2($Arg2VarName, $Arg3VarName, highLane8);                     // z2 w2 z3 w3 z6 w6 z7 w7


    ulong4 lowLane4 = {0, 4, 2, 6};
    ulong4 highLane4 = {1, 5, 3, 7};
    float8 xyzw04 = as_float8(__ocl_helper_shuffle2(as_double4(xy0145),
                                              as_double4(zw0145), lowLane4));         // x0 y0 z0 w0 x4 y4 z4 w4
    float8 xyzw15 = as_float8(__ocl_helper_shuffle2(as_double4(xy0145),
                                              as_double4(zw0145), highLane4));        // x1 y1 z1 w1 x5 y5 z5 w5
    float8 xyzw26 = as_float8(__ocl_helper_shuffle2(as_double4(xy2367),
                                              as_double4(zw2367), lowLane4));         // x2 y2 z2 w2 x6 y6 z6 w6
    float8 xyzw37 = as_float8(__ocl_helper_shuffle2(as_double4(xy2367),
                                              as_double4(zw2367), highLane4));        // x3 y3 z3 w3 x7 y7 z7 w7

    *$Arg4VarName = xyzw04.lo;                                                               // x0 y0 z0 w0
    *$Arg5VarName = xyzw15.lo;                                                               // x1 y1 z1 w1
    *$Arg6VarName = xyzw26.lo;                                                               // x2 y2 z2 w2
    *$Arg7VarName = xyzw37.lo;                                                               // x3 y3 z3 w3
    *$Arg8VarName = xyzw04.hi;                                                               // x4 y4 z4 w4
    *$Arg9VarName = xyzw15.hi;                                                               // x5 y5 z5 w5
    *$Arg10VarName = xyzw26.hi;                                                               // x6 y6 z6 w6
    *$Arg11VarName = xyzw37.hi;                                                               // x7 y7 z7 w7
  }]>;

