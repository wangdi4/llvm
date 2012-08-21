// vim:ts=2:sw=2:et:

// lprefetch
OclBuiltinImpl _lprefetch = OclBuiltinImpl<lprefetch, [v1i8], 0, [{
    size_t totalLines = (($Arg1VarName * $Arg2VarName) + MIC_DEV_DCU_LINE_SIZE - 1) / MIC_DEV_DCU_LINE_SIZE;
    for(size_t i = 0; i < totalLines; ++i) {
      _mm_prefetch($Arg0VarName, _MM_HINT_T1);
      $Arg0VarName += MIC_DEV_DCU_LINE_SIZE;
    }
    return ;
  }]>;

// max
OclBuiltinImpl max_f32_v16 = OclBuiltinImpl<max, [v16f32], 0, [{
    return _mm512_max_ps($Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl mask_max_f32_v16 = OclBuiltinImpl<mask_max, [v16f32], 0, [{
    return _mm512_mask_max_ps($Arg1VarName, $Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;
OclBuiltinImpl max_f32_promote = OclBuiltinImpl<max, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote2>;

OclBuiltinImpl max_f64_v8 = OclBuiltinImpl<max, [v8f64], 0, [{
    return _mm512_max_pd($Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl mask_max_f64_v8 = OclBuiltinImpl<mask_max, [v8f64], 0, [{
    return _mm512_mask_max_pd($Arg1VarName, $Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;
OclBuiltinImpl max_f64_promote = OclBuiltinImpl<max, [v1f64, v2f64, v3f64, v4f64], 0, MaskPromote2>;
OclBuiltinImpl max_f64_expand = OclBuiltinImpl<max, [v16f64], 0, Expand2>;

// min
OclBuiltinImpl min_f32_v16 = OclBuiltinImpl<min, [v16f32], 0, [{
    return _mm512_min_ps($Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl mask_min_f32_v16 = OclBuiltinImpl<mask_min, [v16f32], 0, [{
    return _mm512_mask_min_ps($Arg1VarName, $Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;
OclBuiltinImpl min_f32_promote = OclBuiltinImpl<min, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote2>;

OclBuiltinImpl min_f64_v8 = OclBuiltinImpl<min, [v8f64], 0, [{
    return _mm512_min_pd($Arg0VarName, $Arg1VarName);
  }]>;
OclBuiltinImpl mask_min_f64_v8 = OclBuiltinImpl<mask_min, [v8f64], 0, [{
    return _mm512_mask_min_pd($Arg1VarName, $Arg0VarName, $Arg1VarName, $Arg2VarName);
  }]>;
OclBuiltinImpl min_f64_promote = OclBuiltinImpl<min, [v1f64, v2f64, v3f64, v4f64], 0, MaskPromote2>;
OclBuiltinImpl min_f64_expand = OclBuiltinImpl<min, [v16f64], 0, Expand2>;

// mix
OclBuiltinImpl mix_f32_v16 = OclBuiltinImpl<mix, [v16f32], 0, [{
    return _mm512_madd231_ps($Arg0VarName, $Arg2VarName,
                             _mm512_sub_ps($Arg1VarName, $Arg0VarName));
  }]>;
OclBuiltinImpl mask_mix_f32_v16 = OclBuiltinImpl<mask_mix, [v16f32], 0, [{
    return _mm512_mask_madd231_ps($Arg1VarName, $Arg0VarName, $Arg3VarName,
                                  _mm512_mask_sub_ps($Arg2VarName, $Arg0VarName, $Arg2VarName, $Arg1VarName));
  }]>;
OclBuiltinImpl mix_f32_promote = OclBuiltinImpl<mix, [v1f32, v2f32, v3f32, v4f32, v8f32], 0, MaskPromote3>;

OclBuiltinImpl mix_f64_v8 = OclBuiltinImpl<mix, [v8f64], 0, [{
    return _mm512_madd231_pd($Arg0VarName, $Arg2VarName,
                             _mm512_sub_pd($Arg1VarName, $Arg0VarName));
  }]>;
OclBuiltinImpl mask_mix_f64_v8 = OclBuiltinImpl<mask_mix, [v8f64], 0, [{
    return _mm512_mask_madd231_pd($Arg1VarName, $Arg0VarName, $Arg3VarName,
                                  _mm512_mask_sub_pd($Arg2VarName, $Arg0VarName, $Arg2VarName, $Arg1VarName));
  }]>;
OclBuiltinImpl mix_f64_promote = OclBuiltinImpl<mix, [v1f64, v2f64, v3f64, v4f64], 0, MaskPromote3>;
OclBuiltinImpl mix_f64_expand = OclBuiltinImpl<mix, [v16f64], 0, Expand3>;
