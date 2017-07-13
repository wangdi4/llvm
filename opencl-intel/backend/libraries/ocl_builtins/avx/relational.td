OclBuiltinImpl isfinite_promote       =    OclBuiltinImpl    <isfinite,       [v3f64],                  0, Promote1>;
OclBuiltinImpl isinf_promote          =    OclBuiltinImpl    <isinf,          [v3f64],                  0, Promote1>;
OclBuiltinImpl isnan_promote          =    OclBuiltinImpl    <isnan,          [v3f64],                  0, Promote1>;
OclBuiltinImpl isnormal_promote       =    OclBuiltinImpl    <isnormal,       [v3f64],                  0, Promote1>;
OclBuiltinImpl isordered_promote      =    OclBuiltinImpl    <isordered,      [v3f64],                  0, Promote2>;
OclBuiltinImpl isunordered_promote    =    OclBuiltinImpl    <isunordered,    [v3f64],                  0, Promote2>;
OclBuiltinImpl signbit_promote        =    OclBuiltinImpl    <signbit,        [v3f64],                  0, Promote1>;
//OclBuiltinImpl any_promote          =    OclBuiltinImpl    <any,            [v3i64],                  0, Promote1>;
//OclBuiltinImpl all_promote          =    OclBuiltinImpl    <all,            [v3i64],                  0, Promote1>;
OclBuiltinImpl bitselect_promote      =    OclBuiltinImpl    <bitselect,      [v3f64, v3i64, v3u64],    0, Promote3>;
OclBuiltinImpl iselect_promote        =    OclBuiltinImpl    <iselect,        [v3f64, v3i64, v3u64],    0, Promote3>;
OclBuiltinImpl uselect_promote        =    OclBuiltinImpl    <uselect,        [v3f64, v3i64, v3u64],    0, Promote3>;
OclBuiltinImpl islessgreater2         =    OclBuiltinImpl    <islessgreater,  [v3f64, v16f32],          0, Promote2>;
OclBuiltinImpl islessgreater2         =    OclBuiltinImpl    <islessgreater,  [v4f64, v8f64],           0, [{
    return (($Arg0VarName < $Arg1VarName) | ($Arg0VarName > $Arg1VarName)) ;
  }]>;
//==============select===============
code select_vector_v8f = [{ 
    return $Arg2VarName ? $Arg1VarName : $Arg0VarName;
  }];

code select_vector_v4d = [{ 
    return $Arg2VarName ? $Arg1VarName : $Arg0VarName;
  }];

code select_vector_v8i = [{ 
    return $Arg2VarName ? $Arg1VarName : $Arg0VarName;
  }];

code select_vector_v4l = [{
    return $Arg2VarName ? $Arg1VarName : $Arg0VarName;
  }];

//======v8f32 and v4f64 implementation=============================== 

OclBuiltinImpl isnan_v8f32 = OclBuiltinImpl<isnan, [v8f32], 0, [{
    return as_$ReturnType(_mm256_cmp_ps($Arg0VarName, $Arg0VarName, _CMP_UNORD_Q));
  }]>;

OclBuiltinImpl isnan_v4f64 = OclBuiltinImpl<isnan, [v4f64], 0, [{
    return as_$ReturnType(_mm256_cmp_pd($Arg0VarName, $Arg0VarName, _CMP_UNORD_Q));
  }]>;

OclBuiltinImpl isordered_v8f32 = OclBuiltinImpl<isordered, [v8f32], 0, [{
    return as_$ReturnType(_mm256_cmp_ps($Arg0VarName, $Arg1VarName, _CMP_ORD_Q));
  }]>;

OclBuiltinImpl isordered_v4f64 = OclBuiltinImpl<isordered, [v4f64], 0, [{
    return  as_$ReturnType(_mm256_cmp_pd($Arg0VarName, $Arg1VarName, _CMP_ORD_Q));
  }]>;

OclBuiltinImpl isunordered_v8f32 = OclBuiltinImpl<isunordered, [v8f32], 0, [{
    return as_$ReturnType(_mm256_cmp_ps($Arg0VarName,$Arg1VarName, _CMP_UNORD_S));
  }]>;

OclBuiltinImpl isunordered_v4f64 = OclBuiltinImpl<isunordered, [v4f64], 0, [{
    return  as_$ReturnType(_mm256_cmp_pd($Arg0VarName, $Arg1VarName,_CMP_UNORD_S));
  }]>;

OclBuiltinImpl signbit_v8f32 = OclBuiltinImpl<signbit, [v8f32], 0, [{
    $Arg0VarName = as_$Arg0Type(as_$Arg0igentype($Arg0VarName) & ($Arg0igentype)fsign_mask);
    return as_$ReturnType(select($Arg0VarName, as_$Arg0Type(($Arg0igentype)FFFMask8),  as_$Arg0igentype($Arg0VarName)));
  }]>;

OclBuiltinImpl signbit_v4f64 = OclBuiltinImpl<signbit, [v4f64], 0, [{
    $Arg0VarName = as_$Arg0Type(as_$Arg0igentype($Arg0VarName) & ($Arg0igentype)dsign_mask);
    return as_$ReturnType(select($Arg0VarName,  as_$Arg0Type(($Arg0igentype)FF), as_$Arg0igentype($Arg0VarName)));
  }]>;

OclBuiltinImpl bitselect_v8f32 = OclBuiltinImpl<bitselect, [v8f32, v4f64, v8i32, v8u32], 0, [{
    $Arg0VarName = as_$Arg0Type( ~as_$Arg0igentype($Arg2VarName) & as_$Arg0igentype($Arg0VarName)); //res1 is $Arg0VarName if $Arg2VarName is 0
    $Arg1VarName = as_$Arg0Type( as_$Arg0igentype($Arg2VarName) & as_$Arg0igentype($Arg1VarName)); //res2 bit is $Arg1VarName if $Arg2VarName is 1
    return as_$ReturnType(as_$Arg0igentype($Arg0VarName) | as_$Arg0igentype($Arg1VarName));
  }]>;

OclBuiltinImpl iselect_v8f32 = OclBuiltinImpl<iselect, [v8f32], 0, select_vector_v8f>;

OclBuiltinImpl iselect_v4f64 = OclBuiltinImpl<iselect, [v4f64], 0, select_vector_v4d>;

OclBuiltinImpl uselect_v8f32 = OclBuiltinImpl<uselect, [v8f32], 0, select_vector_v8f>;

OclBuiltinImpl uselect_v4f64 = OclBuiltinImpl<uselect, [v4f64], 0, select_vector_v4d>;


//======v8i32\v8u32 and v4i64\v4u64 implementation=============================== 
OclBuiltinImpl any_v8i32 = OclBuiltinImpl<any, [v8i32], 0, [{
    int mask = _mm256_movemask_ps(__builtin_astype($Arg0VarName,__m256));
    return as_$ReturnType(((mask & 0xFF) != 0));
  }]>;

OclBuiltinImpl any_v4i64 = OclBuiltinImpl<any, [v4i64], 0, [{
    int mask = _mm256_movemask_ps(__builtin_astype($Arg0VarName,__m256));
    return as_$ReturnType(((mask & 0xAA) != 0));
  }]>;

OclBuiltinImpl all_v8i32 = OclBuiltinImpl<all, [v8i32], 0, [{
    int mask = _mm256_movemask_ps(__builtin_astype($Arg0VarName,__m256));
    return as_$ReturnType(((mask & 0xFF) == 0xFF));
  }]>;

OclBuiltinImpl all_v4i64 = OclBuiltinImpl<all, [v4i64], 0, [{
    int mask = _mm256_movemask_ps(__builtin_astype($Arg0VarName,__m256));
    return as_$ReturnType(((mask & 0xAA) == 0xAA));
  }]>;

OclBuiltinImpl bitselect_v8i32 = OclBuiltinImpl<bitselect, [v8i32, v8u32, v16i16, v16u16], 0, [{
    $Arg0igentype res1 = (~ as_$Arg0igentype($Arg2VarName) & as_$Arg0igentype($Arg0VarName));
    $Arg0igentype res2 = ( as_$Arg0igentype($Arg2VarName) & as_$Arg0igentype($Arg1VarName));
    return as_$ReturnType(res1 | res2);
  }]>;

OclBuiltinImpl bitselect_v4i64 = OclBuiltinImpl<bitselect, [v4i64], 0, [{
    return as_$ReturnType(bitselect(as_int8($Arg0VarName), as_int8($Arg1VarName), as_int8($Arg2VarName)));
  }]>;

OclBuiltinImpl bitselect_v4i64 = OclBuiltinImpl<bitselect, [v4u64], 0, [{
    return as_$ReturnType(bitselect(as_uint8($Arg0VarName), as_uint8($Arg1VarName), as_uint8($Arg2VarName)));
  }]>;

OclBuiltinImpl iselect_v8 = OclBuiltinImpl<iselect, [v8i32, v8u32], 0, select_vector_v8i>;

OclBuiltinImpl iselect_v16 = OclBuiltinImpl<iselect, [v4i64, v4u64], 0, select_vector_v4l>;

OclBuiltinImpl uselect_v8 = OclBuiltinImpl<uselect, [v8i32, v8u32], 0, select_vector_v8i>;

OclBuiltinImpl uselect_v16 = OclBuiltinImpl<uselect, [v4i64, v4u64], 0, select_vector_v4l>;

