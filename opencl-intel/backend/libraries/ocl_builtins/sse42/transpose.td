// sse4
/// STYLE NOTE: Comments at same line of code are arranged to start at column 85 at the output final file

// ****************************************************************************
//                                 char4x4
// ****************************************************************************

/// @brief Recieves a single pointer to char16 and outputs transposed matrix as 4 matrix rows
/// @param Arg0    - char16 - char4x4 matrix to be transposed
/// @param Arg1-4  - char4* - Row 0-3 of the transposed matrix
OclBuiltinImpl transpose_load_char4x4_sse4_helper = OclBuiltinImpl<__ocl_load_transpose4x4_helper, [v4i8], 0, [{
    char16 txyzw = $Arg0VarName;                                                    // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3
    *$Arg1VarName = txyzw.s048C;                                                    // x0  D  D  D x1  D  D  D x2  D  D  D x3  D  D  D
    txyzw = as_char16(_mm_srli_si128((__m128i)txyzw, 1));                           // y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3  D
    *$Arg2VarName = txyzw.s048C;                                                    // y0  D  D  D y1  D  D  D y2  D  D  D y3  D  D  D
    txyzw = as_char16(_mm_srli_si128((__m128i)txyzw, 1));                           // z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3  D  D
    *$Arg3VarName = txyzw.s048C;                                                    // z0  D  D  D z1  D  D  D z2  D  D  D z3  D  D  D
    txyzw = as_char16(_mm_srli_si128((__m128i)txyzw, 1));                           // w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3  D  D  D
    *$Arg4VarName = txyzw.s048C;
  }]>;

/// @brief Receives char4x4 matrix as 4 matrix rows, transposes it and outputs the matrix as a whole using char16
/// @param Arg0    - char16* - This parameter will contain the transposed char4x4 matrix
/// @param Arg1-4  - char4   - Row 0-3 of the matrix to be transposed
OclBuiltinImpl transpose_store_char4x4_sse4_helper = OclBuiltinImpl<__ocl_transpose_store4x4_helper, [v4i8], 0, [{
    char16 x;
    char16 y;
    char16 z;
    char16 w;
    x.s0123 = $Arg1VarName;                                                         // x0 x1 x2 x3  D  D  D  D  D  D  D  D  D  D  D  D
    y.s0123 = $Arg2VarName;                                                         // y0 y1 y2 y3  D  D  D  D  D  D  D  D  D  D  D  D
    z.s0123 = $Arg3VarName;                                                         // z0 z1 z2 z3  D  D  D  D  D  D  D  D  D  D  D  D
    w.s0123 = $Arg4VarName;                                                         // w0 w1 w2 w3  D  D  D  D  D  D  D  D  D  D  D  D
    uchar16 low16 = {0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23};
    short8 xy = as_short8(__ocl_helper_shuffle2(x, y, low16));                      // x0 y0 x1 y1 x2 y2 x3 y3  D  D  D  D  D  D  D  D
    short8 zw = as_short8(__ocl_helper_shuffle2(z, w, low16));                      // z0 w0 z1 w1 z2 w2 z3 w3  D  D  D  D  D  D  D  D
    ushort8 low8 = {0, 8, 1, 9, 2, 10, 3, 11};
    *$Arg0VarName = as_char16(__ocl_helper_shuffle2(xy, zw, low8));                 // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3
  }]>;

OclBuiltinImpl transpose_load_char4x4_sse4 = OclBuiltinImpl<__ocl_load_transpose4x4, [v4i8], 0, [{
    // We load "char16", meaning we load the full matrix in a single load
    const __private char* tmpLoadAdd = (const __private char*)$Arg0VarName;
    char16 vLoaded = vload16(0, tmpLoadAdd); // unaligned load

    __ocl_load_transpose4x4_helper(vLoaded, $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName);
  }]>;

OclBuiltinImpl transpose_store_char4x4_sse4 = OclBuiltinImpl<__ocl_transpose_store4x4,  [v4i8],  0, [{
    char16 xyzw;
    __private char16* pStore = &xyzw;

    __ocl_transpose_store4x4_helper(pStore, $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName);

    // We store "char16", meaning we store the full matrix in a single store
    __private char *tmpStoreAdd = (__private char*)$Arg0VarName;
    vstore16(xyzw, 0, tmpStoreAdd);  // unaligned store
  }]>;

OclBuiltinImpl transpose_gather_char4x4_sse4 = OclBuiltinImpl<__ocl_gather_transpose4x4,  [v4i8],  0, [{
    // Broadcast the loaded values, all but the first element in the register which will be moved there
    int4 xyzw1 = *((__private int*)$Arg1VarName);                                                  // x1 y1 z1 w1 x1 y1 z1 w1 x1 y1 z1 w1 x1 y1 z1 w1
    int4 xyzw2 = *((__private int*)$Arg2VarName);                                                  // x2 y2 z2 w2 x2 y2 z2 w2 x2 y2 z2 w2 x2 y2 z2 w2
    int4 xyzw3 = *((__private int*)$Arg3VarName);                                                  // x3 y3 z3 w3 x3 y3 z3 w3 x3 y3 z3 w3 x3 y3 z3 w3

    int4 xyzw;
    // TODO : Replace this blend built-in with ?: when clang bug will be fixed
    // There's no blendd in AVX, so we use blendps
    xyzw.s0 = *((__private int*)$Arg0VarName);                                      // x0 y0 z0 w0  D  D  D  D  D  D  D  D  D  D  D  D
    xyzw = as_int4(_mm_blend_ps((__m128)xyzw, (__m128)xyzw1, 0x2));                 // x0 y0 z0 w0 x1 y1 z1 w1  D  D  D  D  D  D  D  D
    xyzw = as_int4(_mm_blend_ps((__m128)xyzw, (__m128)xyzw2, 0x4));                 // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2  D  D  D  D
    xyzw = as_int4(_mm_blend_ps((__m128)xyzw, (__m128)xyzw3, 0x8));                 // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3
    char16 vGathered = as_char16(xyzw);

    __ocl_load_transpose4x4_helper(vGathered, $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName);
  }]>;

OclBuiltinImpl transpose_scatter_char4x4_sse4 = OclBuiltinImpl<__ocl_transpose_scatter4x4,  [v4i8],  0, [{
    int4 xyzw = 0;
    __private char16* pStore = (__private char16*)&xyzw;

    __ocl_transpose_store4x4_helper(pStore, $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName);

    *((__private int*)$Arg0VarName) = xyzw.s0;
    *((__private int*)$Arg1VarName) = xyzw.s1;
    *((__private int*)$Arg2VarName) = xyzw.s2;
    *((__private int*)$Arg3VarName) = xyzw.s3;
  }]>;

OclBuiltinImpl transpose_masked_gather_char4x4_sse4 = OclBuiltinImpl<__ocl_masked_gather_transpose4x4,  [v4i8], 0, [{
    // get mask as bits in int
    const int rescmp = intel_movemask(mask);
    // ALL 4 elements in mask are -1
    if(rescmp == 0xF){
      __ocl_gather_transpose_$Arg0BaseType_4x4($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName,
                                $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName);
      return;
    }
    // ALL elements in mask are zero
    if(rescmp == 0){
        return;
    }
    // mask addresses to stub variable
    char4 stub;
    $Arg0VarName = mask.s0 ? $Arg0VarName : &stub;
    $Arg1VarName = mask.s1 ? $Arg1VarName : &stub;
    $Arg2VarName = mask.s2 ? $Arg2VarName : &stub;
    $Arg3VarName = mask.s3 ? $Arg3VarName : &stub;

    __ocl_gather_transpose_$Arg0BaseType_4x4($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName,
                                $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName);
  }]>;

// ****************************************************************************
//                                 char4x8
// ****************************************************************************

/// @brief Receives char8x4 matrix as 2 halfs (2 char4x4 matrixes) using 2 char16,
///        transposes it and outputs the rows of the transposed matrix.
/// @param Arg0    - char16 - Upper part of the char8x4 matrix to be transposed
/// @param Arg1    - char16 - Lower part of the char8x4 matrix to be transposed
/// @param Arg2-5  - char8* - Rows 0-3 of the transposed matrix
OclBuiltinImpl transpose_load_char4x8_sse4_helper = OclBuiltinImpl<__ocl_load_transpose4x8_helper, [v4i8], 0, [{
    $Arg0VarName = $Arg0VarName.s048C159D26AE37BF;                                                    // x0 x1 x2 x3 y0 y1 y2 y3 z0 z1 z2 z3 w0 w1 w2 w3
    $Arg1VarName = $Arg1VarName.s048C159D26AE37BF;                                                    // x4 x5 x6 x7 y4 y5 y6 y7 z4 z5 z6 z7 w4 w5 w6 w7
    uint4 low4 = {0, 4, 1, 5};
    uint4 high4 = {2, 6, 3, 7};
    char16 xy = as_char16(__ocl_helper_shuffle2(as_int4($Arg0VarName), as_int4($Arg1VarName), low4)); // x0 x1 x2 x3 x4 x5 x6 x7 y0 y1 y2 y3 y4 y5 y6 y7
    char16 zw = as_char16(__ocl_helper_shuffle2(as_int4($Arg0VarName), as_int4($Arg1VarName), high4));// z0 z1 z2 z3 z4 z5 z6 z7 w0 w1 w2 w3 w4 w5 w6 w7
    *$Arg2VarName = xy.lo;                                                                            // x0  D x1  D x2  D x3  D x4  D x5  D x6  D x7  D
    *$Arg3VarName = xy.hi;                                                                            // y0  D y1  D y2  D y3  D y4  D y5  D y6  D y7  D
    *$Arg4VarName = zw.lo;                                                                            // z0  D z1  D z2  D z3  D z4  D z5  D z6  D z7  D
    *$Arg5VarName = zw.hi;                                                                            // w0  D w1  D w2  D w3  D w4  D w5  D w6  D w7  D
  }]>;

/// @brief Receives char4x8 matrix as 4 matrix rows, transposes it and outputs the matrix
///        2 halfs (2 char4x4 matrixes) using 2 char16, which create one char4x8 matrix
/// @param Arg0    - char16* - Upper part of the transposed char4x8
/// @param Arg1    - char16* - Lower part of the transposed char4x8
/// @param Arg2-5  - char8   - Rows 0-3 of the matrix to be transposed
OclBuiltinImpl transpose_store_char4x8_sse4_helper = OclBuiltinImpl<__ocl_transpose_store4x8_helper, [v4i8], 0, [{
    char16 x;
    char16 y;
    char16 z;
    char16 w;
    x.lo = $Arg2VarName;                                                            // x0 x1 x2 x3 x4 x5 x6 x7  D  D  D  D  D  D  D  D
    y.lo = $Arg3VarName;                                                            // y0 y1 y2 y3 y4 y5 y6 y7  D  D  D  D  D  D  D  D
    z.lo = $Arg4VarName;                                                            // z0 z1 z2 z3 z4 z5 z6 z7  D  D  D  D  D  D  D  D
    w.lo = $Arg5VarName;                                                            // w0 w1 w2 w3 w4 w5 w6 w7  D  D  D  D  D  D  D  D
    uchar16 low16 = {0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23};
    short8 xy = as_short8(__ocl_helper_shuffle2(x, y, low16));                      // x0 y0 x1 y1 x2 y2 x3 y3 x4 y4 x5 y5 x6 y6 x7 y7
    short8 zw = as_short8(__ocl_helper_shuffle2(z, w, low16));                      // z0 w0 z1 w1 z2 w2 z3 w3 z4 w4 z5 w5 z6 w6 z7 w7
    ushort8 low8 = {0, 8, 1, 9, 2, 10, 3, 11};
    ushort8 high8 = {4, 12, 5, 13, 6, 14, 7, 15};
    *$Arg0VarName = as_char16(__ocl_helper_shuffle2(xy, zw, low8));                 // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3
    *$Arg1VarName = as_char16(__ocl_helper_shuffle2(xy, zw, high8));                // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
  }]>;

OclBuiltinImpl transpose_load_char4x8_sse4 = OclBuiltinImpl<__ocl_load_transpose4x8, [v4i8], 0, [{
    // We load "char16", meaning we load the full matrix in a 2 loads
    const __private char* tmpLoadAdd = (const __private char*)$Arg0VarName;
    char16 vLoaded0 = vload16(0, tmpLoadAdd);                                       // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3
    char16 vLoaded1 = vload16(1, tmpLoadAdd);                                       // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7

    __ocl_load_transpose4x8_helper(vLoaded0, vLoaded1, $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName);
  }]>;

OclBuiltinImpl transpose_store_char4x8_sse4 = OclBuiltinImpl<__ocl_transpose_store4x8, [v4i8],  0, [{
    char16 xyzw0;
    char16 xyzw1;
    __private char16* pStore0 = &xyzw0;
    __private char16* pStore1 = &xyzw1;

    __ocl_transpose_store4x8_helper(pStore0, pStore1, $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName);

    // We store "char16", meaning we store the full matrix in a 2 stores
    __private char* tmpStoreAdd = (__private char*)$Arg0VarName;
    vstore16(xyzw0, 0, tmpStoreAdd);
    vstore16(xyzw1, 1, tmpStoreAdd);
  }]>;

OclBuiltinImpl transpose_gather_char4x8_sse4 = OclBuiltinImpl<__ocl_gather_transpose4x8, [v4i8],  0, [{
    // Broadcast the loaded values, all but the first element in the register which will be moved there
    int4 xyzw1 = *((__private int*)$Arg1VarName);                                                  // x1 y1 z1 w1 x1 y1 z1 w1 x1 y1 z1 w1 x1 y1 z1 w1
    int4 xyzw2 = *((__private int*)$Arg2VarName);                                                  // x2 y2 z2 w2 x2 y2 z2 w2 x2 y2 z2 w2 x2 y2 z2 w2
    int4 xyzw3 = *((__private int*)$Arg3VarName);                                                  // x3 y3 z3 w3 x3 y3 z3 w3 x3 y3 z3 w3 x3 y3 z3 w3

    int4 xyzw5 = *((__private int*)$Arg5VarName);                                                  // x5 y5 z5 w5 x5 y5 z5 w5 x5 y5 z5 w5 x5 y5 z5 w5
    int4 xyzw6 = *((__private int*)$Arg6VarName);                                                  // x6 y6 z6 w6 x6 y6 z6 w6 x6 y6 z6 w6 x6 y6 z6 w6
    int4 xyzw7 = *((__private int*)$Arg7VarName);                                                  // x7 y7 z7 w7 x7 y7 z7 w7 x7 y7 z7 w7 x7 y7 z7 w7

    int4 xyzwIn0;
    int4 xyzwIn1;
    // TODO : Replace this blend built-in with ?: when clang bug will be fixed
    // We don't have blendd in AVX, so we use blendps
    xyzwIn0.s0 = *((__private int*)$Arg0VarName);                                   // x0 y0 z0 w0  D  D  D  D  D  D  D  D  D  D  D  D
    xyzwIn0 = as_int4(_mm_blend_ps((__m128)xyzwIn0, (__m128)xyzw1, 0x2));           // x0 y0 z0 w0 x1 y1 z1 w1  D  D  D  D  D  D  D  D
    xyzwIn0 = as_int4(_mm_blend_ps((__m128)xyzwIn0, (__m128)xyzw2, 0x4));           // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2  D  D  D  D
    xyzwIn0 = as_int4(_mm_blend_ps((__m128)xyzwIn0, (__m128)xyzw3, 0x8));           // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3

    xyzwIn1.s0 = *((__private int*)$Arg4VarName);                                   // x4 y4 z4 w4  D  D  D  D  D  D  D  D  D  D  D  D
    xyzwIn1 = as_int4(_mm_blend_ps((__m128)xyzwIn1, (__m128)xyzw5, 0x2));           // x4 y4 z4 w4 x5 y5 z5 w5  D  D  D  D  D  D  D  D
    xyzwIn1 = as_int4(_mm_blend_ps((__m128)xyzwIn1, (__m128)xyzw6, 0x4));           // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6  D  D  D  D
    xyzwIn1 = as_int4(_mm_blend_ps((__m128)xyzwIn1, (__m128)xyzw7, 0x8));           // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
    char16 vLoaded0 = as_char16(xyzwIn0);
    char16 vLoaded1 = as_char16(xyzwIn1);

    __ocl_load_transpose4x8_helper(vLoaded0, vLoaded1, $Arg8VarName, $Arg9VarName, $Arg10VarName, $Arg11VarName);
  }]>;

OclBuiltinImpl transpose_scatter_char4x8_sse4 = OclBuiltinImpl<__ocl_transpose_scatter4x8, [v4i8],  0, [{
    int4 xyzw0 = 0;
    int4 xyzw1 = 0;
    __private char16* pStore0 = (__private char16*)&xyzw0;
    __private char16* pStore1 = (__private char16*)&xyzw1;

    __ocl_transpose_store4x8_helper(pStore0, pStore1, $Arg8VarName, $Arg9VarName, $Arg10VarName, $Arg11VarName);

    *((__private int*)$Arg0VarName) = xyzw0.s0;
    *((__private int*)$Arg1VarName) = xyzw0.s1;
    *((__private int*)$Arg2VarName) = xyzw0.s2;
    *((__private int*)$Arg3VarName) = xyzw0.s3;
    *((__private int*)$Arg4VarName) = xyzw1.s0;
    *((__private int*)$Arg5VarName) = xyzw1.s1;
    *((__private int*)$Arg6VarName) = xyzw1.s2;
    *((__private int*)$Arg7VarName) = xyzw1.s3;
  }]>;

// ****************************************************************************
//                                 helper function
// ****************************************************************************
OclBuiltinImpl __ocl_transpose4x4_sse4 =  OclBuiltinImpl<__ocl_transpose4x4, [v4i32, v4f32], 0, [{
    // xyzw0 = x0 y0 z0 w0
    // xyzw1 = x1 y1 z1 w1
    // xyzw2 = x2 y2 z2 w2
    // xyzw3 = x3 y3 z3 w3
    uint4 low4 = {0, 4, 1, 5};
    uint4 high4 = {2, 6, 3, 7};

    $Arg0Type xy02, zw02, xy13, zw13;
    xy02 = __ocl_helper_shuffle2($Arg0VarName, $Arg2VarName, low4);                           // x0 x2 y0 y2
    zw02 = __ocl_helper_shuffle2($Arg0VarName, $Arg2VarName, high4);                          // z0 z2 w0 w2
    xy13 = __ocl_helper_shuffle2($Arg1VarName, $Arg3VarName, low4);                           // x1 x3 y1 y3
    zw13 = __ocl_helper_shuffle2($Arg1VarName, $Arg3VarName, high4);                          // z1 z3 w1 w3

    *$Arg4VarName = __ocl_helper_shuffle2(xy02, xy13, low4);                                  // x0 x1 x2 x3
    *$Arg5VarName = __ocl_helper_shuffle2(xy02, xy13, high4);                                 // y0 y1 y2 y3
    *$Arg6VarName = __ocl_helper_shuffle2(zw02, zw13, low4);                                  // z0 z1 z2 z3
    *$Arg7VarName = __ocl_helper_shuffle2(zw02, zw13, high4);                                 // w0 w1 w2 w3
  }]>;
