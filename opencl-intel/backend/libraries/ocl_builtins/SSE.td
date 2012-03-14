// vim:ts=2:sw=2:et:

include "GENERIC.td"

class SSE : Generic {

  list<OclType> NativeTypes = [v1f32, v4f32, v1f64, v2f64];

  code Promote2 = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName; n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeReturnType $ReturnVarName = $NativeFunc(n$Arg0VarName, n$Arg1VarName);
    return $ReturnVarName$Pattern;
  }];

  OclBuiltinImpl fmax_v1 = OclBuiltinImpl<fmax, [v1f32], 0, [{
    return _mm_max_ss($Arg0VarName, $Arg1VarName);
  }]>;
  OclBuiltinImpl fmax_v4 = OclBuiltinImpl<fmax, [v4f32], 0, [{
    return _mm_max_ps($Arg0VarName, $Arg1VarName);
  }]>;
  OclBuiltinImpl fmax_promote = OclBuiltinImpl<fmax, [v2f32, v3f32], 0, Promote2>;
  OclBuiltinImpl fmax_expand  = OclBuiltinImpl<fmax, [v8f32, v16f32], 0, Expand2>;
}
