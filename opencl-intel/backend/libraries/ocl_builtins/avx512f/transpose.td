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

// ****************************************************************************
//                                char4x16
// ****************************************************************************

OclBuiltinImpl transpose_gather_char4x16_512 = OclBuiltinImpl<__ocl_gather_transpose4x16, [v4i8],  0, [{
    uchar8 concatVectors4 = {0, 1, 2, 3, 4, 5, 6, 7};
    uchar16 concatVectors8 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    char8 tmp0 = __ocl_helper_shuffle2(*$Arg0VarName, *$Arg4VarName, concatVectors4);
    char8 tmp1 = __ocl_helper_shuffle2(*$Arg1VarName, *$Arg5VarName, concatVectors4);
    char8 tmp2 = __ocl_helper_shuffle2(*$Arg2VarName, *$Arg6VarName, concatVectors4);
    char8 tmp3 = __ocl_helper_shuffle2(*$Arg3VarName, *$Arg7VarName, concatVectors4);
    char8 tmp4 = __ocl_helper_shuffle2(*$Arg8VarName, *$Arg12VarName, concatVectors4);
    char8 tmp5 = __ocl_helper_shuffle2(*$Arg9VarName, *$Arg13VarName, concatVectors4);
    char8 tmp6 = __ocl_helper_shuffle2(*$Arg10VarName, *$Arg14VarName, concatVectors4);
    char8 tmp7 = __ocl_helper_shuffle2(*$Arg11VarName, *$Arg15VarName, concatVectors4);
    char16 Arg0 = __ocl_helper_shuffle2(tmp0, tmp4, concatVectors8);
    char16 Arg1 = __ocl_helper_shuffle2(tmp1, tmp5, concatVectors8);
    char16 Arg2 = __ocl_helper_shuffle2(tmp2, tmp6, concatVectors8);
    char16 Arg3 = __ocl_helper_shuffle2(tmp3, tmp7, concatVectors8);
    $Arg16Type Arg4 = $Arg16VarName;
    $Arg17Type Arg5 = $Arg17VarName;
    $Arg18Type Arg6 = $Arg18VarName;
    $Arg19Type Arg7 = $Arg19VarName;

    //__ocl_transpose_common_char4x16_512(char16 Arg0, char16 Arg1, char16 Arg2, char16 Arg3, __private char16* Arg4, __private char16* Arg5, __private char16* Arg6, __private char16* Arg7)
    #{__ocl_transpose_common_char4x16_512}#
  }]>;

OclBuiltinImpl transpose_scatter_char4x16_512 = OclBuiltinImpl<__ocl_transpose_scatter4x16, [v4i8],  0, [{
   char16 xyzw0, xyzw1, xyzw2, xyzw3;
    __private char16* Arg0 = &xyzw0;
    __private char16* Arg1 = &xyzw1;
    __private char16* Arg2 = &xyzw2;
    __private char16* Arg3 = &xyzw3;
    $Arg16Type Arg4 = $Arg16VarName;
    $Arg17Type Arg5 = $Arg17VarName;
    $Arg18Type Arg6 = $Arg18VarName;
    $Arg19Type Arg7 = $Arg19VarName;

    //__ocl_transpose_store_common_char4x16_512(__private char16* Arg0, __private char16* Arg1, __private char16* Arg2, __private char16* Arg3, char16 Arg4, char16 Arg5, char16 Arg6, char16 Arg7)
    #{__ocl_transpose_store_common_char4x16_512}#

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
//                                short4x16
// ****************************************************************************

OclBuiltinImpl transpose_gather_short4x16_512 = OclBuiltinImpl<__ocl_gather_transpose4x16, [v4i16],  0, [{
    ushort8 concatVectors4 = {0, 1, 2, 3, 4, 5, 6, 7};
    ushort16 concatVectors8 = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    short8 tmp0 = __ocl_helper_shuffle2(*$Arg0VarName, *$Arg4VarName, concatVectors4);
    short8 tmp1 = __ocl_helper_shuffle2(*$Arg1VarName, *$Arg5VarName, concatVectors4);
    short8 tmp2 = __ocl_helper_shuffle2(*$Arg2VarName, *$Arg6VarName, concatVectors4);
    short8 tmp3 = __ocl_helper_shuffle2(*$Arg3VarName, *$Arg7VarName, concatVectors4);
    short8 tmp4 = __ocl_helper_shuffle2(*$Arg8VarName, *$Arg12VarName, concatVectors4);
    short8 tmp5 = __ocl_helper_shuffle2(*$Arg9VarName, *$Arg13VarName, concatVectors4);
    short8 tmp6 = __ocl_helper_shuffle2(*$Arg10VarName, *$Arg14VarName, concatVectors4);
    short8 tmp7 = __ocl_helper_shuffle2(*$Arg11VarName, *$Arg15VarName, concatVectors4);
    short16 Arg0 = __ocl_helper_shuffle2(tmp0, tmp4, concatVectors8);
    short16 Arg1 = __ocl_helper_shuffle2(tmp1, tmp5, concatVectors8);
    short16 Arg2 = __ocl_helper_shuffle2(tmp2, tmp6, concatVectors8);
    short16 Arg3 = __ocl_helper_shuffle2(tmp3, tmp7, concatVectors8);
    $Arg16Type Arg4 = $Arg16VarName;
    $Arg17Type Arg5 = $Arg17VarName;
    $Arg18Type Arg6 = $Arg18VarName;
    $Arg19Type Arg7 = $Arg19VarName;

    //__ocl_transpose_common_short4x16_512(short16 Arg0, short16 Arg1, short16 Arg2, short16 Arg3, __private short16* Arg4, __private short16* Arg5, __private short16* Arg6, __private short16* Arg7)
    #{__ocl_transpose_common_short4x16_512}#
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
