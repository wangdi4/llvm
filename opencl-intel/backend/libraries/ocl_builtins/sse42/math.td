// vim:ts=2:sw=2:et:


//////////////////////////////////////////////////////////////////////////////////////////////
//                Float f32 section
//////////////////////////////////////////////////////////////////////////////////////////////

// ceil
OclBuiltinImpl ceil_f32_impl = OclBuiltinImpl<ceil, [v4f32], 0, [{
    return _mm_ceil_ps($Arg0VarName);
  }]>;
// leave only implementaion for float4. others sizes are promoted and expanded to float4 version
OclBuiltinImpl ceil_f32_promote = OclBuiltinImpl<ceil, [v1f32, v2f32, v3f32], 0, Promote1>;
OclBuiltinImpl ceil_f32_expand = OclBuiltinImpl<ceil, [v8f32, v16f32], 0, Expand1>;

// floor
OclBuiltinImpl floor_f32_impl = OclBuiltinImpl<floor, [v4f32], 0, [{
    return _mm_floor_ps($Arg0VarName);
  }]>;
// leave only implementaion for float4. others sizes are promoted and expanded to float4 version

// trunc
OclBuiltinImpl trunc_f32_impl = OclBuiltinImpl<trunc, [v4f32], 0, [{
    return _mm_round_ps($Arg0VarName, _MM_FROUND_TO_ZERO);
  }]>;

// round
OclBuiltinImpl round_f32_impl = OclBuiltinImpl<round, [v4f32], 0, [{
    const int mth_signMask = 0x7FFFFFFF;
    float4 xAbs = fabs($Arg0VarName);
    int4 xSign = as_int4($Arg0VarName) & ~mth_signMask;
    float4 xAbsRounded = _mm_round_ps(xAbs, _MM_FROUND_TO_NEAREST_INT);
    float4 xDiff = xAbs - xAbsRounded;
    xDiff = as_float4(xDiff == 0.5f);
    xDiff = as_float4(as_int4(xDiff) & as_int(1.0f));
    $Arg0VarName = xAbsRounded + xDiff;
    $Arg0VarName = as_float4(as_int4($Arg0VarName) | xSign);
    return $Arg0VarName;
  }]>;


// rint
OclBuiltinImpl rint_f64_impl = OclBuiltinImpl<rint, [v2f64], 0, [{
    return _mm_round_pd($Arg0VarName, _MM_FROUND_TO_NEAREST_INT);
  }]>;
OclBuiltinImpl rint_f32_impl = OclBuiltinImpl<rint, [v4f32], 0, [{
    return _mm_round_ps($Arg0VarName, _MM_FROUND_TO_NEAREST_INT);
  }]>;
// leave only implementaion for float4. others sizes are promoted and expanded to float4 version

//fract
// leave only implementaion for float4. float, float2, float3 are promoted to float4
OclBuiltinImpl fract_pas_f32_promote = OclBuiltinImpl<fract_pas, [v1f32, v2f32, v3f32], 0, Promote_fract>;
OclBuiltinImpl fract_las_f32_promote = OclBuiltinImpl<fract_las, [v1f32, v2f32, v3f32], 0, Promote_fract>;
OclBuiltinImpl fract_gas_f32_promote = OclBuiltinImpl<fract_gas, [v1f32, v2f32, v3f32], 0, Promote_fract>;
OclBuiltinImpl fract_gen_f32_promote = OclBuiltinImpl<fract_gen, [v1f32, v2f32, v3f32], 0, Promote_fract>;


//////////////////////////////////////////////////////////////////////////////////////////////
//                Double f64 section
//////////////////////////////////////////////////////////////////////////////////////////////
// ceil
// leave only implementaion to double2. others sizes are calling double2 version
OclBuiltinImpl ceil_f64_impl = OclBuiltinImpl<ceil, [v2f64], 0, [{
    return _mm_ceil_pd($Arg0VarName);
  }]>;
OclBuiltinImpl ceil_f64_promote = OclBuiltinImpl<ceil, [v1f64], 0, Promote1>;
OclBuiltinImpl ceil_f64_expand = OclBuiltinImpl<ceil, [v3f64, v4f64, v8f64, v16f64], 0, Expand1>;

// floor
// leave only implementaion to double2. others sizes are calling double2 version
OclBuiltinImpl floor_f64_impl = OclBuiltinImpl<floor, [v2f64], 0, [{
    return _mm_floor_pd($Arg0VarName);
  }]>;

// trunc
// leave only implementaion to double2. others sizes are calling double2 version
OclBuiltinImpl trunc_f64_impl = OclBuiltinImpl<trunc, [v2f64], 0, [{
    return _mm_round_pd($Arg0VarName, _MM_FROUND_TO_ZERO);
  }]>;

// round
// leave only implementaion to double2. others sizes are calling double2 version
OclBuiltinImpl round_f64_impl = OclBuiltinImpl<round, [v2f64], 0, [{
    const long mth_signMask = 0x7FFFFFFFFFFFFFFF;
    double2 xAbs = fabs($Arg0VarName);
    long2 xSign = as_long2($Arg0VarName) & ~mth_signMask;
    double2 xAbsRounded = _mm_round_pd(xAbs, _MM_FROUND_TO_NEAREST_INT);
    double2 xDiff = xAbs - xAbsRounded;
    xDiff = as_double2(xDiff == 0.5);
    xDiff = as_double2(as_long2(xDiff) & as_long(1.0));
    $Arg0VarName = xAbsRounded + xDiff;
    $Arg0VarName = as_double2(as_long2($Arg0VarName) | xSign);
    return $Arg0VarName;
  }]>;

// rint
// leave only implementaion to double2. others sizes are calling double2 version
OclBuiltinImpl rint_f64_impl = OclBuiltinImpl<rint, [v2f64], 0, [{
    return _mm_round_pd($Arg0VarName, _MM_FROUND_TO_NEAREST_INT);
  }]>;

