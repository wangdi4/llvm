
OclBuiltinImpl _shuffle_pairs_int16_knl = OclBuiltinImpl<__ocl_helper_shuffle_pairs_knl, [v16i32], 0, [{
    int16 t0 = as_$ReturnType(_mm512_permutex2var_epi32(__builtin_astype($Arg0VarName, __m512i),
                                                        __builtin_astype($Arg4VarName, __m512i),
                                                        __builtin_astype($Arg1VarName, __m512i)));

    int16 t1 = as_$ReturnType(_mm512_permutex2var_epi32(__builtin_astype($Arg2VarName, __m512i),
                                                        __builtin_astype($Arg4VarName, __m512i),
                                                        __builtin_astype($Arg3VarName, __m512i)));
    t0.hi = t1.lo;
    return t0;
  }]>;

OclBuiltinImpl transpose_load_int4x16_knl = OclBuiltinImpl<__ocl_load_transpose4x16, [v4i32],  0, [{
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

OclBuiltinImpl _shuffle_pairs_float16_knl = OclBuiltinImpl<__ocl_helper_shuffle_pairs_knl, [v16f32], 0, [{
    float16 t0 = as_$ReturnType(_mm512_permutex2var_ps(__builtin_astype($Arg0VarName, __m512),
                                                        __builtin_astype($Arg4VarName, __m512i),
                                                        __builtin_astype($Arg1VarName, __m512)));

    float16 t1 = as_$ReturnType(_mm512_permutex2var_ps(__builtin_astype($Arg2VarName, __m512),
                                                        __builtin_astype($Arg4VarName, __m512i),
                                                        __builtin_astype($Arg3VarName, __m512)));
    t0.hi = t1.lo;
    return t0;
  }]>;

OclBuiltinImpl transpose_load_float4x16_knl = OclBuiltinImpl<__ocl_load_transpose4x16, [v4f32],  0, [{
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

OclBuiltinImpl transpose_masked_load_float4x16_knl = OclBuiltinImpl<__ocl_masked_load_transpose4x16, [v4f32],  0, [{
   ushort LoadMask = (ushort)$Arg5VarName;
   ushort LoadMask0, LoadMask1, LoadMask2, LoadMask3;
   __ocl_expand_mask_4x16(LoadMask, &LoadMask0, &LoadMask1, &LoadMask2, &LoadMask3);
   float16 t0 = __ocl_imasked_load_float16((float16*)&($Arg0VarName[0]), LoadMask0);
   float16 t1 = __ocl_imasked_load_float16((float16*)&($Arg0VarName[4]), LoadMask1);
   float16 t2 = __ocl_imasked_load_float16((float16*)&($Arg0VarName[8]), LoadMask2);
   float16 t3 = __ocl_imasked_load_float16((float16*)&($Arg0VarName[12]), LoadMask3);
   int16 mask0 = {0,4,8,12,16,20,24,28, -1,-1, -1, -1, -1, -1, -1, -1};
   *$Arg1VarName = __ocl_helper_shuffle_pairs_float16(t0,t1,t2,t3,mask0);
   int16 mask1 = {1,5,9,13,17,21,25,29,-1,-1,-1, -1, -1, -1, -1, -1};
   *$Arg2VarName = __ocl_helper_shuffle_pairs_float16(t0,t1,t2,t3,mask1);
   int16 mask2 = {2,6,10,14,18,22,26,30,-1,-1,-1, -1, -1, -1, -1, -1};
   *$Arg3VarName = __ocl_helper_shuffle_pairs_float16(t0,t1,t2,t3,mask2);
   int16 mask3 = {3,7,11,15,19,23,27,31,-1,-1,-1, -1, -1, -1, -1, -1};
   *$Arg4VarName = __ocl_helper_shuffle_pairs_float16(t0,t1,t2,t3,mask3);
  }]>;

