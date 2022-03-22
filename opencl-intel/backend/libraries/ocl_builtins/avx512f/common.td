// clamp on floating
OclBuiltinImpl clamp_fgeneric = OclBuiltinImpl<clamp,
  ExpandTypesByVFAndFlatten<[v1f32, v1f64], [32, 64]>.Tout, 0, clamp_f_code>;

// max
OclBuiltinImpl max_fgeneric = OclBuiltinImpl<max,
  ExpandTypesByVFAndFlatten<[v1f32, v1f64], [32, 64]>.Tout, 0, max_f_code>;

// min
OclBuiltinImpl min_fgeneric = OclBuiltinImpl<min,
  ExpandTypesByVFAndFlatten<[v1f32, v1f64], [32, 64]>.Tout, 0, min_f_code>;

list<OclType> common_types_v32_to_v64 = ExpandTypesByVFAndFlatten<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64], [32, 64]>.Tout;

// clamp_s
OclBuiltinImpl clamp_s_generic = OclBuiltinImpl<clamp_s, common_types_v32_to_v64, 0, clamp_s_impl>;

// max_s
OclBuiltinImpl max_s_generic = OclBuiltinImpl<max_s, common_types_v32_to_v64, 0, max_s_impl>;

// min_s
OclBuiltinImpl min_s_generic = OclBuiltinImpl<min_s, common_types_v32_to_v64, 0, min_s_impl>;

// step
OclBuiltinImpl step_v16f32 = OclBuiltinImpl<step, [v16f32] , 0, [{
    // According to the specification this built-in should return
    // 0.0 if $Arg1VarName < $Arg0VarName, else 1.0

    __mmask16 cmpMask = _mm512_cmp_ps_mask($Arg1VarName, $Arg0VarName, _CMP_GE_OS);
    return _mm512_maskz_mov_ps(cmpMask, _mm512_set1_ps(1.0f));
  }]>;

OclBuiltinImpl step_v8f64 = OclBuiltinImpl<step, [v8f64] , 0, [{
    // According to the specification this built-in should return
    // 0.0 if $Arg1VarName < $Arg0VarName, else 1.0,

    __mmask8 cmpMask = _mm512_cmp_pd_mask($Arg1VarName, $Arg0VarName, _CMP_GE_OS);
    return _mm512_maskz_mov_pd(cmpMask, _mm512_set1_pd(1.0));
  }]>;
