//
// Broadcast
//

OclBuiltinImpl sub_group_broadcast_vector = OclBuiltinImpl<sub_group_broadcast_vec, [v4i32, v8i32, v16i32,
                                                                                     v4u32, v8u32, v16u32,
                                                                                     v4i64, v8i64, v16i64,
                                                                                     v4u64, v8u64, v16u64,
                                                                                     v4f32, v8f32, v16f32,
                                                                                     v4f64, v8f64, v16f64],  0,
  [{
    return $Arg0VarName[$Arg1VarName];
  }]>;

OclBuiltinImpl sub_group_broadcast_vector_cs = OclBuiltinImpl<sub_group_broadcast_vec_cs, [v4i8 , v8i8 , v16i8 ,
                                                                                           v4u8 , v8u8 , v16u8 ,
                                                                                           v4i16, v8i16, v16i16,
                                                                                           v4u16, v8u16, v16u16], 0,
  [{
    return $Arg0VarName[$Arg1VarName];
  }]>;

OclBuiltinImpl sub_group_shuffle_avx512v48gentype = OclBuiltinImpl<sub_group_shuffle_avx512, [v48i8, v48u8, v48i16, v48u16, v48f32,
                                                                                              v48i32, v48u32, v48i64, v48u64], 0,
  [{
     $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x16($Arg0VarName);
     $Arg0BaseType#16* arg0 = ($Arg0BaseType#16*)&$Arg0VarName;

     $Arg0BaseType#16 tmp1 = intel_sub_group_shuffle(arg0[0], $Arg1VarName, $Arg2VarName);
     $Arg0BaseType#16 tmp2 = intel_sub_group_shuffle(arg0[1], $Arg1VarName, $Arg2VarName);
     $Arg0BaseType#16 tmp3 = intel_sub_group_shuffle(arg0[2], $Arg1VarName, $Arg2VarName);

     $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_16x3(($Arg0Type)(tmp1, tmp2, tmp3));
     return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_shuffle_avx2v24gentype = OclBuiltinImpl<sub_group_shuffle_avx2, [v24i8, v24u8, v24i16, v24u16, v24f32,
                                                                                        v24i32, v24u32, v24i64, v24u64], 0,
  [{
     $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x8($Arg0VarName);
     $Arg0BaseType#8* arg0 = ($Arg0BaseType#8*)&$Arg0VarName;

     $Arg0BaseType#8 tmp1 = intel_sub_group_shuffle(arg0[0], $Arg1VarName, $Arg2VarName);
     $Arg0BaseType#8 tmp2 = intel_sub_group_shuffle(arg0[1], $Arg1VarName, $Arg2VarName);
     $Arg0BaseType#8 tmp3 = intel_sub_group_shuffle(arg0[2], $Arg1VarName, $Arg2VarName);

     $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_8x3(($Arg0Type)(tmp1, tmp2, tmp3));
     return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_shuffle_avxv12gentype = OclBuiltinImpl<sub_group_shuffle_avx, [v12i8, v12u8, v12i16, v12u16, v12f32,
                                                                                        v12i32, v12u32, v12i64, v12u64], 0,
  [{
     $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x4($Arg0VarName);
     $Arg0BaseType#4* arg0 = ($Arg0BaseType#4*)&$Arg0VarName;

     $Arg0BaseType#4 tmp1 = intel_sub_group_shuffle(arg0[0], $Arg1VarName, $Arg2VarName);
     $Arg0BaseType#4 tmp2 = intel_sub_group_shuffle(arg0[1], $Arg1VarName, $Arg2VarName);
     $Arg0BaseType#4 tmp3 = intel_sub_group_shuffle(arg0[2], $Arg1VarName, $Arg2VarName);

     $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_4x3(($Arg0Type)(tmp1, tmp2, tmp3));
     return $Arg0VarName;
  }]>;

// shuffle up
OclBuiltinImpl sub_group_shuffle_up_avx512v48gentype = OclBuiltinImpl<sub_group_shuffle_up_avx512, [v48i8, v48u8, v48i16, v48u16, v48f32,
                                                                                                    v48i32, v48u32, v48i64, v48u64], 0,
  [{
    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x16($Arg0VarName);
    $Arg1VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x16($Arg1VarName);

    $Arg0BaseType#16* arg0 = ($Arg0BaseType#16*)&$Arg0VarName;
    $Arg0BaseType#16* arg1 = ($Arg0BaseType#16*)&$Arg1VarName;

    $Arg0BaseType#16 tmp1 = intel_sub_group_shuffle_up(arg0[0], arg1[0], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#16 tmp2 = intel_sub_group_shuffle_up(arg0[1], arg1[1], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#16 tmp3 = intel_sub_group_shuffle_up(arg0[2], arg1[2], $Arg2VarName, $Arg3VarName);

    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_16x3(($Arg0Type)(tmp1, tmp2, tmp3));
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_shuffle_up_avx2v24gentype = OclBuiltinImpl<sub_group_shuffle_up_avx2, [v24i8, v24u8, v24i16, v24u16, v24f32,
                                                                                                v24i32, v24u32, v24i64, v24u64], 0,
  [{
    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x8($Arg0VarName);
    $Arg1VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x8($Arg1VarName);

    $Arg0BaseType#8* arg0 = ($Arg0BaseType#8*)&$Arg0VarName;
    $Arg0BaseType#8* arg1 = ($Arg0BaseType#8*)&$Arg1VarName;

    $Arg0BaseType#8 tmp1 = intel_sub_group_shuffle_up(arg0[0], arg1[0], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#8 tmp2 = intel_sub_group_shuffle_up(arg0[1], arg1[1], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#8 tmp3 = intel_sub_group_shuffle_up(arg0[2], arg1[2], $Arg2VarName, $Arg3VarName);

    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_8x3(($Arg0Type)(tmp1, tmp2, tmp3));
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_shuffle_up_avxv12gentype = OclBuiltinImpl<sub_group_shuffle_up_avx, [v12i8, v12u8, v12i16, v12u16, v12f32,
                                                                                              v12i32, v12u32, v12i64, v12u64], 0,
  [{
    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x4($Arg0VarName);
    $Arg1VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x4($Arg1VarName);

    $Arg0BaseType#4* arg0 = ($Arg0BaseType#4*)&$Arg0VarName;
    $Arg0BaseType#4* arg1 = ($Arg0BaseType#4*)&$Arg1VarName;

    $Arg0BaseType#4 tmp1 = intel_sub_group_shuffle_up(arg0[0], arg1[0], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#4 tmp2 = intel_sub_group_shuffle_up(arg0[1], arg1[1], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#4 tmp3 = intel_sub_group_shuffle_up(arg0[2], arg1[2], $Arg2VarName, $Arg3VarName);

    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_4x3(($Arg0Type)(tmp1, tmp2, tmp3));
    return $Arg0VarName;
  }]>;

// shuffle down
OclBuiltinImpl sub_group_shuffle_down_avx512v48gentype = OclBuiltinImpl<sub_group_shuffle_down_avx512, [v48i8, v48u8, v48i16, v48u16, v48f32,
                                                                                                        v48i32, v48u32, v48i64, v48u64], 0,
  [{
    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x16($Arg0VarName);
    $Arg1VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x16($Arg1VarName);

    $Arg0BaseType#16* arg0 = ($Arg0BaseType#16*)&$Arg0VarName;
    $Arg0BaseType#16* arg1 = ($Arg0BaseType#16*)&$Arg1VarName;

    $Arg0BaseType#16 tmp1 = intel_sub_group_shuffle_down(arg0[0], arg1[0], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#16 tmp2 = intel_sub_group_shuffle_down(arg0[1], arg1[1], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#16 tmp3 = intel_sub_group_shuffle_down(arg0[2], arg1[2], $Arg2VarName, $Arg3VarName);

    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_16x3(($Arg0Type)(tmp1, tmp2, tmp3));
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_shuffle_down_avx2v24gentype = OclBuiltinImpl<sub_group_shuffle_down_avx2, [v24i8, v24u8, v24i16, v24u16, v24f32,
                                                                                                    v24i32, v24u32, v24i64, v24u64], 0,
  [{
    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x8($Arg0VarName);
    $Arg1VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x8($Arg1VarName);

    $Arg0BaseType#8* arg0 = ($Arg0BaseType#8*)&$Arg0VarName;
    $Arg0BaseType#8* arg1 = ($Arg0BaseType#8*)&$Arg1VarName;

    $Arg0BaseType#8 tmp1 = intel_sub_group_shuffle_down(arg0[0], arg1[0], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#8 tmp2 = intel_sub_group_shuffle_down(arg0[1], arg1[1], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#8 tmp3 = intel_sub_group_shuffle_down(arg0[2], arg1[2], $Arg2VarName, $Arg3VarName);

    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_8x3(($Arg0Type)(tmp1, tmp2, tmp3));
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_shuffle_down_avxv12gentype = OclBuiltinImpl<sub_group_shuffle_down_avx, [v12i8, v12u8, v12i16, v12u16, v12f32,
                                                                                                  v12i32, v12u32, v12i64, v12u64], 0,
  [{
    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x4($Arg0VarName);
    $Arg1VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x4($Arg1VarName);

    $Arg0BaseType#4* arg0 = ($Arg0BaseType#4*)&$Arg0VarName;
    $Arg0BaseType#4* arg1 = ($Arg0BaseType#4*)&$Arg1VarName;

    $Arg0BaseType#4 tmp1 = intel_sub_group_shuffle_down(arg0[0], arg1[0], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#4 tmp2 = intel_sub_group_shuffle_down(arg0[1], arg1[1], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#4 tmp3 = intel_sub_group_shuffle_down(arg0[2], arg1[2], $Arg2VarName, $Arg3VarName);

    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_4x3(($Arg0Type)(tmp1, tmp2, tmp3));
    return $Arg0VarName;
  }]>;

// shuffle xor

OclBuiltinImpl sub_group_shuffle_xor_avx512v48gentype = OclBuiltinImpl<sub_group_shuffle_xor_avx512, [v48i8, v48u8, v48i16, v48u16, v48f32,
                                                                                                      v48i32, v48u32, v48i64, v48u64], 0,
  [{
    uint16 indexes = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    indexes = indexes ^ $Arg1VarName;
    return intel_sub_group_shuffle($Arg0VarName, indexes, $Arg2VarName);
  }]>;

OclBuiltinImpl sub_group_shuffle_xor_avx2v24gentype = OclBuiltinImpl<sub_group_shuffle_xor_avx2, [v24i8, v24u8, v24i16, v24u16, v24f32,
                                                                                                  v24i32, v24u32, v24i64, v24u64], 0,
  [{
    uint8 indexes = {0, 1, 2, 3, 4, 5, 6, 7};
    indexes = indexes ^ $Arg1VarName;
    return intel_sub_group_shuffle($Arg0VarName, indexes, $Arg2VarName);
  }]>;

OclBuiltinImpl sub_group_shuffle_xor_avxv12gentype = OclBuiltinImpl<sub_group_shuffle_xor_avx, [v12i8, v12u8, v12i16, v12u16, v12f32,
                                                                                                v12i32, v12u32, v12i64, v12u64], 0,
  [{
    uint4 indexes = {0, 1, 2, 3};
    indexes = indexes ^ $Arg1VarName;
    return intel_sub_group_shuffle($Arg0VarName, indexes, $Arg2VarName);
  }]>;
