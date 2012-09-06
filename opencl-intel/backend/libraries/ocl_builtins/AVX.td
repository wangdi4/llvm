// vim:ts=2:sw=2:et:

include "SSE.td"

class AVX : SSE {
  list<OclType> NativeTypes = [v1f32, v4f32, v8f32, v1f64, v2f64, v4f64];

  OclBuiltinImpl fmax_v8 = OclBuiltinImpl<fmax, [v8f32], 0, [{
    return _mm256_max_ps($Arg0VarName, $Arg1VarName);
  }]>;
}
