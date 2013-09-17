// sse4
/// STYLE NOTE: Comments at same line of code are arranged to start at column 85 at the output final file

// ****************************************************************************
//                                 char4x4
// ****************************************************************************
OclBuiltinImpl transpose_load_char4x4_sse4 = OclBuiltinImpl<__ocl_load_transpose4x4,  [v4i8],  0, [{
    // We load "char16", meaning we load the full matrix in a single load
    char* tmpLoadAdd = (char*)$Arg0VarName;
    char16 Arg0 = vload16(0, tmpLoadAdd); // unaligned load
    $Arg1Type Arg1 = $Arg1VarName;
    $Arg2Type Arg2 = $Arg2VarName;
    $Arg3Type Arg3 = $Arg3VarName;
    $Arg4Type Arg4 = $Arg4VarName;

    //__ocl_load_transpose_char4x4common_sse4(char16 Arg0, char4* Arg1, char4* Arg2, char4* Arg3, char4* Arg4)
    #{__ocl_load_transpose_char4x4common_sse4}#
  }]>;

OclBuiltinImpl transpose_store_char4x4_sse4 = OclBuiltinImpl<__ocl_transpose_store4x4,  [v4i8],  0, [{
    char16 xyzw;
    char16* Arg0 = &xyzw;
    $Arg1Type Arg1 = $Arg1VarName;
    $Arg2Type Arg2 = $Arg2VarName;
    $Arg3Type Arg3 = $Arg3VarName;
    $Arg4Type Arg4 = $Arg4VarName;

    //__ocl_transpose_store_char4x4common_sse4(char16* Arg0, char4 Arg1, char4 Arg2, char4 Arg3, char4 Arg4)
    #{__ocl_transpose_store_char4x4common_sse4}#
    // We store "char16", meaning we store the full matrix in a single store
    char *tmpStoreAdd = (char*)$Arg0VarName;
    vstore16(xyzw, 0, tmpStoreAdd);  // unaligned store
  }]>;

OclBuiltinImpl transpose_gather_char4x4_sse4 = OclBuiltinImpl<__ocl_gather_transpose4x4,  [v4i8],  0, [{
    // Broadcast the loaded values, all but the first element in the register which will be moved there
    int4 xyzw1 = *((int*)$Arg1VarName);                                                  // x1 y1 z1 w1 x1 y1 z1 w1 x1 y1 z1 w1 x1 y1 z1 w1
    int4 xyzw2 = *((int*)$Arg2VarName);                                                  // x2 y2 z2 w2 x2 y2 z2 w2 x2 y2 z2 w2 x2 y2 z2 w2
    int4 xyzw3 = *((int*)$Arg3VarName);                                                  // x3 y3 z3 w3 x3 y3 z3 w3 x3 y3 z3 w3 x3 y3 z3 w3

    int4 xyzw;
    // TODO : Replace this blend built-in with ?: when clang bug will be fixed
    // There's no blendd in AVX, so we use blendps
    xyzw.s0 = *((int*)$Arg0VarName);                                                     // x0 y0 z0 w0  D  D  D  D  D  D  D  D  D  D  D  D
    xyzw = as_int4(_mm_blend_ps((__m128)xyzw, (__m128)xyzw1, 0x2));                   // x0 y0 z0 w0 x1 y1 z1 w1  D  D  D  D  D  D  D  D
    xyzw = as_int4(_mm_blend_ps((__m128)xyzw, (__m128)xyzw2, 0x4));                   // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2  D  D  D  D
    xyzw = as_int4(_mm_blend_ps((__m128)xyzw, (__m128)xyzw3, 0x8));                   // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3
    char16 Arg0 = as_char16(xyzw);
    $Arg4Type Arg1 = $Arg4VarName;
    $Arg5Type Arg2 = $Arg5VarName;
    $Arg6Type Arg3 = $Arg6VarName;
    $Arg7Type Arg4 = $Arg7VarName;

    //__ocl_load_transpose_char4x4common_sse4(char16 Arg0, char4* Arg1, char4* Arg2, char4* Arg3, char4* Arg4)
    #{__ocl_load_transpose_char4x4common_sse4}#
  }]>;

OclBuiltinImpl transpose_scatter_char4x4_sse4 = OclBuiltinImpl<__ocl_transpose_scatter4x4,  [v4i8],  0, [{
    int4 xyzw = 0;
    char16* Arg0 = (char16*)&xyzw;
    $Arg4Type Arg1 = $Arg4VarName;
    $Arg5Type Arg2 = $Arg5VarName;
    $Arg6Type Arg3 = $Arg6VarName;
    $Arg7Type Arg4 = $Arg7VarName;

    //__ocl_transpose_store_char4x4common_sse4(char16* Arg0, char4 Arg1, char4 Arg2, char4 Arg3, char4 Arg4)
    #{__ocl_transpose_store_char4x4common_sse4}#
    *((int*)$Arg0VarName) = xyzw.s0;
    *((int*)$Arg1VarName) = xyzw.s1;
    *((int*)$Arg2VarName) = xyzw.s2;
    *((int*)$Arg3VarName) = xyzw.s3;
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
OclBuiltinImpl transpose_load_char4x8_sse4 = OclBuiltinImpl<__ocl_load_transpose4x8, [v4i8],  0, [{
    // We load "char16", meaning we load the full matrix in a 2 loads
    char* tmpLoadAdd = (char*)$Arg0VarName;
    char16 Arg0 = vload16(0, tmpLoadAdd);                                             // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3
    char16 Arg1 = vload16(1, tmpLoadAdd);                                             // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
    $Arg1Type Arg2 = $Arg1VarName;
    $Arg2Type Arg3 = $Arg2VarName;
    $Arg3Type Arg4 = $Arg3VarName;
    $Arg4Type Arg5 = $Arg4VarName;

    //__ocl_load_transpose_char4x8_common_sse4(char16 Arg0, char16  Arg1, char8* Arg2, char8* Arg3, char8* Arg4, char8* Arg5)
    #{__ocl_load_transpose_char4x8_common_sse4}#
  }]>;

OclBuiltinImpl transpose_store_char4x8_sse4 = OclBuiltinImpl<__ocl_transpose_store4x8, [v4i8],  0, [{
    char16 xyzw0;
    char16 xyzw1;
    char16* Arg0 = &xyzw0;
    char16* Arg1 = &xyzw1;
    $Arg1Type Arg2 = $Arg1VarName;
    $Arg2Type Arg3 = $Arg2VarName;
    $Arg3Type Arg4 = $Arg3VarName;
    $Arg4Type Arg5 = $Arg4VarName;

    //__ocl_transpose_store_char4x8_common_sse4(char16* Arg0, char16* Arg1, char8 Arg2, char8 Arg3, char8 Arg4, char8 Arg5)
     #{__ocl_transpose_store_char4x8_common_sse4}#
    // We store "char16", meaning we store the full matrix in a 2 stores
    char* tmpStoreAdd = (char*)$Arg0VarName;
    vstore16(xyzw0, 0, tmpStoreAdd);
    vstore16(xyzw1, 1, tmpStoreAdd);
  }]>;

OclBuiltinImpl transpose_gather_char4x8_sse4 = OclBuiltinImpl<__ocl_gather_transpose4x8, [v4i8],  0, [{
    // Broadcast the loaded values, all but the first element in the register which will be moved there
    int4 xyzw1 = *((int*)$Arg1VarName);                                                  // x1 y1 z1 w1 x1 y1 z1 w1 x1 y1 z1 w1 x1 y1 z1 w1
    int4 xyzw2 = *((int*)$Arg2VarName);                                                  // x2 y2 z2 w2 x2 y2 z2 w2 x2 y2 z2 w2 x2 y2 z2 w2
    int4 xyzw3 = *((int*)$Arg3VarName);                                                  // x3 y3 z3 w3 x3 y3 z3 w3 x3 y3 z3 w3 x3 y3 z3 w3

    int4 xyzw5 = *((int*)$Arg5VarName);                                                  // x5 y5 z5 w5 x5 y5 z5 w5 x5 y5 z5 w5 x5 y5 z5 w5
    int4 xyzw6 = *((int*)$Arg6VarName);                                                  // x6 y6 z6 w6 x6 y6 z6 w6 x6 y6 z6 w6 x6 y6 z6 w6
    int4 xyzw7 = *((int*)$Arg7VarName);                                                  // x7 y7 z7 w7 x7 y7 z7 w7 x7 y7 z7 w7 x7 y7 z7 w7

    int4 xyzwIn0;
    int4 xyzwIn1;
    // TODO : Replace this blend built-in with ?: when clang bug will be fixed
    // We don't have blendd in AVX, so we use blendps
    xyzwIn0.s0 = *((int*)$Arg0VarName);                                                  // x0 y0 z0 w0  D  D  D  D  D  D  D  D  D  D  D  D
    xyzwIn0 = as_int4(_mm_blend_ps((__m128)xyzwIn0, (__m128)xyzw1, 0x2));             // x0 y0 z0 w0 x1 y1 z1 w1  D  D  D  D  D  D  D  D
    xyzwIn0 = as_int4(_mm_blend_ps((__m128)xyzwIn0, (__m128)xyzw2, 0x4));             // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2  D  D  D  D
    xyzwIn0 = as_int4(_mm_blend_ps((__m128)xyzwIn0, (__m128)xyzw3, 0x8));             // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3

    xyzwIn1.s0 = *((int*)$Arg4VarName);                                                  // x4 y4 z4 w4  D  D  D  D  D  D  D  D  D  D  D  D
    xyzwIn1 = as_int4(_mm_blend_ps((__m128)xyzwIn1, (__m128)xyzw5, 0x2));             // x4 y4 z4 w4 x5 y5 z5 w5  D  D  D  D  D  D  D  D
    xyzwIn1 = as_int4(_mm_blend_ps((__m128)xyzwIn1, (__m128)xyzw6, 0x4));             // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6  D  D  D  D
    xyzwIn1 = as_int4(_mm_blend_ps((__m128)xyzwIn1, (__m128)xyzw7, 0x8));             // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
    char16 Arg0 = as_char16(xyzwIn0);
    char16 Arg1 = as_char16(xyzwIn1);
    $Arg8Type Arg2 = $Arg8VarName;
    $Arg9Type Arg3 = $Arg9VarName;
    $Arg10Type Arg4 = $Arg10VarName;
    $Arg11Type Arg5 = $Arg11VarName;

    //__ocl_load_transpose_char4x8_common_sse4(char16 Arg0, char16  Arg1, char8* Arg2, char8* Arg3, char8* Arg4, char8* Arg5)
    #{__ocl_load_transpose_char4x8_common_sse4}#
  }]>;

OclBuiltinImpl transpose_scatter_char4x8_sse4 = OclBuiltinImpl<__ocl_transpose_scatter4x8, [v4i8],  0, [{
    int4 xyzw0 = 0;
    int4 xyzw1 = 0;
    char16* Arg0 = (char16*)&xyzw0;
    char16* Arg1 = (char16*)&xyzw1;
    $Arg8Type Arg2 = $Arg8VarName;
    $Arg9Type Arg3 = $Arg9VarName;
    $Arg10Type Arg4 = $Arg10VarName;
    $Arg11Type Arg5 = $Arg11VarName;

    //__ocl_transpose_store_char4x8_common_sse4(char16* Arg0, char16* Arg1, char8 Arg2, char8 Arg3, char8 Arg4, char8 Arg5)
    #{__ocl_transpose_store_char4x8_common_sse4}#
    *((int*)$Arg0VarName) = xyzw0.s0;
    *((int*)$Arg1VarName) = xyzw0.s1;
    *((int*)$Arg2VarName) = xyzw0.s2;
    *((int*)$Arg3VarName) = xyzw0.s3;
    *((int*)$Arg4VarName) = xyzw1.s0;
    *((int*)$Arg5VarName) = xyzw1.s1;
    *((int*)$Arg6VarName) = xyzw1.s2;
    *((int*)$Arg7VarName) = xyzw1.s3;
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
    xy02 = __ocl_helper_shuffle2($Arg0VarName, $Arg2VarName, low4);                                 // x0 x2 y0 y2
    zw02 = __ocl_helper_shuffle2($Arg0VarName, $Arg2VarName, high4);                                // z0 z2 w0 w2
    xy13 = __ocl_helper_shuffle2($Arg1VarName, $Arg3VarName, low4);                                 // x1 x3 y1 y3
    zw13 = __ocl_helper_shuffle2($Arg1VarName, $Arg3VarName, high4);                                // z1 z3 w1 w3

    *$Arg4VarName = __ocl_helper_shuffle2(xy02, xy13, low4);                                  // x0 x1 x2 x3
    *$Arg5VarName = __ocl_helper_shuffle2(xy02, xy13, high4);                                 // y0 y1 y2 y3
    *$Arg6VarName = __ocl_helper_shuffle2(zw02, zw13, low4);                                  // z0 z1 z2 z3
    *$Arg7VarName = __ocl_helper_shuffle2(zw02, zw13, high4);                                 // w0 w1 w2 w3
  }]>;