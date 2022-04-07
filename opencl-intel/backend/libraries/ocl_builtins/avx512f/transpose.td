// ****************************************************************************
//                        Generic AVX512 implementation
// ****************************************************************************

OclBuiltinImpl transpose_load_4x16_512 = OclBuiltinImpl<__ocl_load_transpose4x16, [v4i8],  0, [{
    __ocl_gather_transpose_$Arg0BaseType_4x16( &($Arg0VarName[0]), &($Arg0VarName[1]), &($Arg0VarName[2]), &($Arg0VarName[3]),
                                    &($Arg0VarName[4]), &($Arg0VarName[5]), &($Arg0VarName[6]), &($Arg0VarName[7]),
                                    &($Arg0VarName[8]), &($Arg0VarName[9]), &($Arg0VarName[10]), &($Arg0VarName[11]),
                                    &($Arg0VarName[12]), &($Arg0VarName[13]), &($Arg0VarName[14]), &($Arg0VarName[15]),
                                    $Arg1VarName, $Arg2VarName, $Arg3VarName, $Arg4VarName);
  }]>;

OclBuiltinImpl transpose_masked_gather_4x16_512 = OclBuiltinImpl<__ocl_masked_gather_transpose4x16,  [v4i8, v4i16], 0, [{
    // get mask as bits in int
    const int rescmp = intel_movemask(mask);
    // ALL elements in mask are -1
    if(rescmp == 0xF){
      __ocl_gather_transpose_$Arg0BaseType_4x16($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName,
                                $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName, $Arg8VarName,
                                $Arg9VarName, $Arg10VarName, $Arg11VarName, $Arg12VarName, $Arg13VarName,
                                $Arg14VarName, $Arg15VarName, $Arg16VarName, $Arg17VarName, $Arg18VarName, $Arg19VarName);
      return;
    }
    // ALL elements in mask are zero
    if(rescmp == 0){
        return;
    }
    // mask addresses to stub variable
    $Arg0BaseType#4 stub;
    $Arg0VarName = mask.s0 ? $Arg0VarName : &stub;
    $Arg1VarName = mask.s1 ? $Arg1VarName : &stub;
    $Arg2VarName = mask.s2 ? $Arg2VarName : &stub;
    $Arg3VarName = mask.s3 ? $Arg3VarName : &stub;
    $Arg4VarName = mask.s4 ? $Arg4VarName : &stub;
    $Arg5VarName = mask.s5 ? $Arg5VarName : &stub;
    $Arg6VarName = mask.s6 ? $Arg6VarName : &stub;
    $Arg7VarName = mask.s7 ? $Arg7VarName : &stub;
    $Arg8VarName = mask.s8 ? $Arg8VarName : &stub;
    $Arg9VarName = mask.s9 ? $Arg9VarName : &stub;
    $Arg10VarName = mask.sA ? $Arg10VarName : &stub;
    $Arg11VarName = mask.sB ? $Arg11VarName : &stub;
    $Arg12VarName = mask.sC ? $Arg12VarName : &stub;
    $Arg13VarName = mask.sD ? $Arg13VarName : &stub;
    $Arg14VarName = mask.sE ? $Arg14VarName : &stub;
    $Arg15VarName = mask.sF ? $Arg15VarName : &stub;

    __ocl_gather_transpose_$Arg0BaseType_4x16($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName,
                                $Arg4VarName, $Arg5VarName, $Arg6VarName, $Arg7VarName, $Arg8VarName,
                                $Arg9VarName, $Arg10VarName, $Arg11VarName, $Arg12VarName, $Arg13VarName,
                                $Arg14VarName, $Arg15VarName, $Arg16VarName, $Arg17VarName, $Arg18VarName, $Arg19VarName);
  }]>;

OclBuiltinImpl transpose_gather_char4x16_512 = OclBuiltinImpl<__ocl_gather_transpose4x16, [v4i8, v4i16],  0, [{
    u$Arg0BaseType#8 concatVectors4 = {0, 1, 2, 3, 4, 5, 6, 7};
    u$Arg0BaseType#16 concatVectors8 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};

    $Arg0BaseType#8 v8tmp0 = __ocl_helper_shuffle2(*$Arg0VarName, *$Arg4VarName, concatVectors4);
    $Arg0BaseType#8 v8tmp1 = __ocl_helper_shuffle2(*$Arg1VarName, *$Arg5VarName, concatVectors4);
    $Arg0BaseType#8 v8tmp2 = __ocl_helper_shuffle2(*$Arg2VarName, *$Arg6VarName, concatVectors4);
    $Arg0BaseType#8 v8tmp3 = __ocl_helper_shuffle2(*$Arg3VarName, *$Arg7VarName, concatVectors4);
    $Arg0BaseType#8 v8tmp4 = __ocl_helper_shuffle2(*$Arg8VarName, *$Arg12VarName, concatVectors4);
    $Arg0BaseType#8 v8tmp5 = __ocl_helper_shuffle2(*$Arg9VarName, *$Arg13VarName, concatVectors4);
    $Arg0BaseType#8 v8tmp6 = __ocl_helper_shuffle2(*$Arg10VarName, *$Arg14VarName, concatVectors4);
    $Arg0BaseType#8 v8tmp7 = __ocl_helper_shuffle2(*$Arg11VarName, *$Arg15VarName, concatVectors4);
    $Arg0BaseType#16 v16tmp0 = __ocl_helper_shuffle2(v8tmp0, v8tmp4, concatVectors8);
    $Arg0BaseType#16 v16tmp1 = __ocl_helper_shuffle2(v8tmp1, v8tmp5, concatVectors8);
    $Arg0BaseType#16 v16tmp2 = __ocl_helper_shuffle2(v8tmp2, v8tmp6, concatVectors8);
    $Arg0BaseType#16 v16tmp3 = __ocl_helper_shuffle2(v8tmp3, v8tmp7, concatVectors8);

    u$Arg0BaseType#16 lowLane16 = {0, 16, 1, 17, 4, 20, 5, 21, 8, 24, 9, 25, 12, 28, 13, 29};
    u$Arg0BaseType#16 highLane16 = {2, 18, 3, 19, 6, 22, 7, 23, 10, 26, 11, 27, 14, 30, 15, 31};
    $Arg0BaseType#16 xy0246 = __ocl_helper_shuffle2(v16tmp0, v16tmp2, lowLane16);              // x0 x2 y0 y2 x4 x6 y4 y6 x8 x10 y8 y10 x12 x14 y12 y14
    $Arg0BaseType#16 zw0246 = __ocl_helper_shuffle2(v16tmp0, v16tmp2, highLane16);             // z0 z2 w0 w2 z4 z6 w4 w6 z8 z10 w8 w10 z12 z14 w12 w14
    $Arg0BaseType#16 xy1357 = __ocl_helper_shuffle2(v16tmp1, v16tmp3, lowLane16);              // x1 x3 y1 y3 x5 x7 y5 y7 x9 x11 y9 y11 x13 x15 y13 y15
    $Arg0BaseType#16 zw1357 = __ocl_helper_shuffle2(v16tmp1, v16tmp3, highLane16);             // z1 z3 w1 w3 z5 z7 w5 w7 z9 z11 w9 w11 z13 z15 w13 w15
    *$Arg16VarName = __ocl_helper_shuffle2(xy0246, xy1357, lowLane16);                         // x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 x10 x11 x12 x13 x14 x15
    *$Arg17VarName = __ocl_helper_shuffle2(xy0246, xy1357, highLane16);                        // y0 y1 y2 y3 y4 y5 y6 y7 y8 y9 y10 y11 y12 y13 y14 y15
    *$Arg18VarName = __ocl_helper_shuffle2(zw0246, zw1357, lowLane16);                         // z0 z1 z2 z3 z4 z5 z6 z7 z8 z9 z10 z11 z12 z13 z14 z15
    *$Arg19VarName = __ocl_helper_shuffle2(zw0246, zw1357, highLane16);                        // w0 w1 w2 w3 w4 w5 w6 w7 w8 w9 w10 w11 w12 w13 w14 w15
  }]>;

// ****************************************************************************
//                                char4x16
// ****************************************************************************
OclBuiltinImpl transpose_scatter_char4x16_512 = OclBuiltinImpl<__ocl_transpose_scatter4x16, [v4i8],  0, [{
    uchar16 low16 = {0, 16, 1, 17, 2, 18, 3, 19, 4, 20, 5, 21, 6, 22, 7, 23};
    uchar16 hi16 = {8, 24, 9, 25, 10, 26, 11, 27, 12, 28, 13, 29, 14, 30, 15, 31};
    char16 xylow = as_char16(__ocl_helper_shuffle2($Arg16VarName, $Arg17VarName, low16));         // x0 y0 x1 y1 x2 y2 x3 y3 x4 y4 x5 y5 x6 y6 x7 y7
    char16 xyhi = as_char16(__ocl_helper_shuffle2($Arg16VarName, $Arg17VarName, hi16));           // x8 y9 x10 y10...
    char16 zwlow = as_char16(__ocl_helper_shuffle2($Arg18VarName, $Arg19VarName, low16));         // z0 w0 z1 w1 z2 w2 z3 w3 z4 w4 z5 w5 z6 w6 z7 w7
    char16 zwhi = as_char16(__ocl_helper_shuffle2($Arg18VarName, $Arg19VarName, hi16));           // z8 w8 z10 w10...

    uchar16 low8  = {0, 1, 16, 17, 2, 3, 18, 19, 4, 5, 20, 21, 6, 7, 22, 23};
    uchar16 high8 = {8, 9, 24, 25, 10, 11, 26, 27, 12, 13, 28, 29, 14, 15, 30, 31};
    char16 xyzw0 = as_char16(__ocl_helper_shuffle2(xylow, zwlow, low8));                          // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3
    char16 xyzw1 = as_char16(__ocl_helper_shuffle2(xylow, zwlow, high8));                         // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7
    char16 xyzw2 = as_char16(__ocl_helper_shuffle2(xyhi, zwhi, low8));                            // x0 y0 z0 w0 x1 y1 z1 w1 x2 y2 z2 w2 x3 y3 z3 w3
    char16 xyzw3 = as_char16(__ocl_helper_shuffle2(xyhi, zwhi, high8));                           // x4 y4 z4 w4 x5 y5 z5 w5 x6 y6 z6 w6 x7 y7 z7 w7

    *$Arg0VarName = xyzw0.s0123;
    *$Arg1VarName = xyzw0.s4567;
    *$Arg2VarName = xyzw0.s89AB;
    *$Arg3VarName = xyzw0.sCDEF;
    *$Arg4VarName = xyzw1.s0123;
    *$Arg5VarName = xyzw1.s4567;
    *$Arg6VarName = xyzw1.s89AB;
    *$Arg7VarName = xyzw1.sCDEF;
    *$Arg8VarName = xyzw2.s0123;
    *$Arg9VarName = xyzw2.s4567;
    *$Arg10VarName = xyzw2.s89AB;
    *$Arg11VarName = xyzw2.sCDEF;
    *$Arg12VarName = xyzw3.s0123;
    *$Arg13VarName = xyzw3.s4567;
    *$Arg14VarName = xyzw3.s89AB;
    *$Arg15VarName = xyzw3.sCDEF;
 }]>;

// ****************************************************************************
//                                 int4x16
// ****************************************************************************
OclBuiltinImpl _shuffle_pairs_int16_512 = OclBuiltinImpl<__ocl_helper_shuffle_pairs, [v16i32], 0, [{
    int16 t0 = as_$ReturnType(_mm512_permutex2var_epi32(__builtin_astype($Arg0VarName, __m512i),
                                                        __builtin_astype($Arg4VarName, __m512i),
                                                        __builtin_astype($Arg1VarName, __m512i)));

    int16 t1 = as_$ReturnType(_mm512_permutex2var_epi32(__builtin_astype($Arg2VarName, __m512i),
                                                        __builtin_astype($Arg4VarName, __m512i),
                                                        __builtin_astype($Arg3VarName, __m512i)));
    t0.hi = t1.lo;
    return t0;
  }]>;


OclBuiltinImpl transpose_load_int4x16_512 = OclBuiltinImpl<__ocl_load_transpose4x16, [v4i32],  0, [{
   int16 t0 = *(int16 *)&($Arg0VarName[0]);
   int16 t1 = *(int16 *)&($Arg0VarName[4]);
   int16 t2 = *(int16 *)&($Arg0VarName[8]);
   int16 t3 = *(int16 *)&($Arg0VarName[12]);
   int16 mask0 = {0,4,8,12,16,20,24,28, -1,-1, -1, -1, -1, -1, -1, -1};
   *$Arg1VarName = __ocl_helper_shuffle_pairs_int16(t0,t1,t2,t3,mask0);
   int16 mask1 = {1,5,9,13,17,21,25,29,-1,-1,-1, -1, -1, -1, -1, -1};
   *$Arg2VarName = __ocl_helper_shuffle_pairs_int16(t0,t1,t2,t3,mask1);
   int16 mask2 = {2,6,10,14,18,22,26,30,-1,-1,-1, -1, -1, -1, -1, -1};
   *$Arg3VarName = __ocl_helper_shuffle_pairs_int16(t0,t1,t2,t3,mask2);
   int16 mask3 = {3,7,11,15,19,23,27,31,-1,-1,-1, -1, -1, -1, -1, -1};
   *$Arg4VarName = __ocl_helper_shuffle_pairs_int16(t0,t1,t2,t3,mask3);
  }]>;

// ****************************************************************************
//                                 float4x16
// ****************************************************************************
OclBuiltinImpl _shuffle_pairs_float16_512 = OclBuiltinImpl<__ocl_helper_shuffle_pairs, [v16f32], 0, [{
    float16 t0 = as_$ReturnType(_mm512_permutex2var_ps(__builtin_astype($Arg0VarName, __m512),
                                                        __builtin_astype($Arg4VarName, __m512i),
                                                        __builtin_astype($Arg1VarName, __m512)));

    float16 t1 = as_$ReturnType(_mm512_permutex2var_ps(__builtin_astype($Arg2VarName, __m512),
                                                        __builtin_astype($Arg4VarName, __m512i),
                                                        __builtin_astype($Arg3VarName, __m512)));
    t0.hi = t1.lo;
    return t0;
  }]>;

OclBuiltinImpl transpose_load_float4x16_512 = OclBuiltinImpl<__ocl_load_transpose4x16, [v4f32],  0, [{
   float16 t0 = *(float16 *)&($Arg0VarName[0]);
   float16 t1 = *(float16 *)&($Arg0VarName[4]);
   float16 t2 = *(float16 *)&($Arg0VarName[8]);
   float16 t3 = *(float16 *)&($Arg0VarName[12]);
   int16 mask0 = {0,4,8,12,16,20,24,28, -1,-1, -1, -1, -1, -1, -1, -1};
   *$Arg1VarName = __ocl_helper_shuffle_pairs_float16(t0,t1,t2,t3,mask0);
   int16 mask1 = {1,5,9,13,17,21,25,29,-1,-1,-1, -1, -1, -1, -1, -1};
   *$Arg2VarName = __ocl_helper_shuffle_pairs_float16(t0,t1,t2,t3,mask1);
   int16 mask2 = {2,6,10,14,18,22,26,30,-1,-1,-1, -1, -1, -1, -1, -1};
   *$Arg3VarName = __ocl_helper_shuffle_pairs_float16(t0,t1,t2,t3,mask2);
   int16 mask3 = {3,7,11,15,19,23,27,31,-1,-1,-1, -1, -1, -1, -1, -1};
   *$Arg4VarName = __ocl_helper_shuffle_pairs_float16(t0,t1,t2,t3,mask3);
  }]>;

OclBuiltinImpl transpose_masked_load_float4x16_512 = OclBuiltinImpl<__ocl_masked_load_transpose4x16, [v4f32],  0, [{
   ushort LoadMask = (ushort)$Arg5VarName;
   ushort LoadMask0, LoadMask1, LoadMask2, LoadMask3;
   __ocl_expand_mask_4x16(LoadMask, &LoadMask0, &LoadMask1, &LoadMask2, &LoadMask3);
   float16 t0 = __ocl_imasked_load_float16((__private float16*)(float16*)&($Arg0VarName[0]), LoadMask0);
   float16 t1 = __ocl_imasked_load_float16((__private float16*)(float16*)&($Arg0VarName[4]), LoadMask1);
   float16 t2 = __ocl_imasked_load_float16((__private float16*)(float16*)&($Arg0VarName[8]), LoadMask2);
   float16 t3 = __ocl_imasked_load_float16((__private float16*)(float16*)&($Arg0VarName[12]), LoadMask3);
   int16 mask0 = {0,4,8,12,16,20,24,28, -1,-1, -1, -1, -1, -1, -1, -1};
   *$Arg1VarName = __ocl_helper_shuffle_pairs_float16(t0,t1,t2,t3,mask0);
   int16 mask1 = {1,5,9,13,17,21,25,29,-1,-1,-1, -1, -1, -1, -1, -1};
   *$Arg2VarName = __ocl_helper_shuffle_pairs_float16(t0,t1,t2,t3,mask1);
   int16 mask2 = {2,6,10,14,18,22,26,30,-1,-1,-1, -1, -1, -1, -1, -1};
   *$Arg3VarName = __ocl_helper_shuffle_pairs_float16(t0,t1,t2,t3,mask2);
   int16 mask3 = {3,7,11,15,19,23,27,31,-1,-1,-1, -1, -1, -1, -1, -1};
   *$Arg4VarName = __ocl_helper_shuffle_pairs_float16(t0,t1,t2,t3,mask3);
  }]>;
