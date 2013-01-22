OclBuiltinImpl HalfToFloatA2 = OclBuiltinImpl<HalfToFloat, [v1f16], 0, [{
	half8 x;
	x.s0 = $Arg0VarName;
	return Half4ToFloat4(as_short8(x)).s0;
}]>;  
OclBuiltinImpl Half4ToFloat4A2 = OclBuiltinImpl<Half4ToFloat4, [v8i16], 0, [{
	return _mm_cvtph_ps(__builtin_astype($Arg0VarName, __m128i));
}]>; 
OclBuiltinImpl Half8ToFloat8A2 = OclBuiltinImpl<Half8ToFloat8, [v8i16], 0, [{
	return _mm256_cvtph_ps(__builtin_astype($Arg0VarName, __m128i));
}]>;
OclBuiltinImpl _ocl_float2half_rteA2 = OclBuiltinImpl<_ocl_float2half_rte, [v4f32], 0, [{
  	return as_$ReturnType(_mm_cvtps_ph($Arg0VarName, _MM_FROUND_TO_NEAREST_INT));
}]>;
OclBuiltinImpl _ocl_float2half_rtzA2 = OclBuiltinImpl<_ocl_float2half_rtz, [v4f32], 0, [{
 	return as_$ReturnType(_mm_cvtps_ph($Arg0VarName, _MM_FROUND_TO_ZERO));
}]>;
OclBuiltinImpl _ocl_float2half_rtpA2 = OclBuiltinImpl<_ocl_float2half_rtp, [v4f32], 0, [{
 	return as_$ReturnType(_mm_cvtps_ph($Arg0VarName, _MM_FROUND_TO_POS_INF));
}]>;
OclBuiltinImpl _ocl_float2half_rtnA2 = OclBuiltinImpl<_ocl_float2half_rtn, [v4f32], 0, [{
 	return as_$ReturnType(_mm_cvtps_ph($Arg0VarName, _MM_FROUND_TO_NEG_INF));
}]>;
OclBuiltinImpl _ocl_float2half8_rteA2 = OclBuiltinImpl<_ocl_float2half8_rte, [v8f32], 0, [{
    return as_$ReturnType(_mm256_cvtps_ph($Arg0VarName, _MM_FROUND_TO_NEAREST_INT));
}]>;
OclBuiltinImpl _ocl_float2half8_rtzA2 = OclBuiltinImpl<_ocl_float2half8_rtz, [v8f32], 0, [{
    return as_$ReturnType(_mm256_cvtps_ph($Arg0VarName, _MM_FROUND_TO_ZERO));
}]>;
OclBuiltinImpl _ocl_float2half8_rtnA2 = OclBuiltinImpl<_ocl_float2half8_rtn, [v8f32], 0, [{
    return as_$ReturnType(_mm256_cvtps_ph($Arg0VarName, _MM_FROUND_TO_NEG_INF));
}]>;
OclBuiltinImpl _ocl_float2half8_rtpA2 = OclBuiltinImpl<_ocl_float2half8_rtp, [v8f32], 0, [{
    return as_$ReturnType(_mm256_cvtps_ph($Arg0VarName, _MM_FROUND_TO_POS_INF));
}]>;
OclBuiltinImpl _ocl_double2ToHalf2_rteA2 = OclBuiltinImpl<_ocl_double2ToHalf2_rte, [v2f64], 0, [{
	typedef union { double2 d; long2 ix; } double2_t;
	double2_t x,j;
	float2 f;
	float4 res;
	half2 h;
	x.d = $Arg0VarName;
	// The key point  is to set the 29 bit to 1 if any of bits 28:0 are 1
	j.ix = x.ix + 0x1fffffff;
	j.ix &= 0x20000000;
	x.ix |= j.ix;
	x.ix &= ~0x1fffffff;
	f = convert_float2_rte(x.d);
	vstore_half2_rte(f, 0, (half*)&h);
	res.s0 = as_float(h);
	return res;	
}]>;
OclBuiltinImpl _ocl_double4ToHalf4_rteA2 = OclBuiltinImpl<_ocl_double4ToHalf4_rte, [v4f64], 0, [{
	typedef union { double4 d; long4 ix; } double4_t;
	double4_t x,j;
	float4 f,res;
	half4 h;
	x.d = $Arg0VarName;
	// The key point  is to set the 29 bit to 1 if any of bits 28:0 are 1
	j.ix = x.ix + 0x1fffffff;
	j.ix &= 0x20000000;
	x.ix |= j.ix;
	x.ix &= ~0x1fffffff;
	f = convert_float4_rte(x.d);
	vstore_half4_rte(f, 0, (half*)&h);
	res.s01 = as_float2(h);
	return res;	
}]>;