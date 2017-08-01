// vim:ts=2:sw=2:et:

// degrees

OclBuiltinImpl degrees_f64_generic_not_shared = OclBuiltinImpl<degrees,
  [v3f64] , 0, [{
    return ($Arg0Type)generic_inv_pi_180 * $Arg0VarName;
  }]>;

// mix
OclBuiltinImpl mix_generic_not_shared = OclBuiltinImpl<mix, 
  [v3f64], 0, [{
    return $Arg0VarName + ($Arg1VarName - $Arg0VarName) * $Arg2VarName;
  }]>;

// radians

OclBuiltinImpl radians_f64_generic_not_shared = OclBuiltinImpl<radians,
  [v3f64] , 0, [{
    return ($Arg0Type)generic_pi_180 * $Arg0VarName;
  }]>;

// step
OclBuiltinImpl step_f32_generic_not_shared = OclBuiltinImpl<step,
  [v8f32, v16f32] , 0, [{
    return ($Arg1VarName < $Arg0VarName) ? ($ReturnType)0.0f : ($ReturnType)1.0f;
  }]>;
OclBuiltinImpl step_f64_generic_not_shared = OclBuiltinImpl<step,
  [v3f64, v4f64, v8f64] , 0, [{
    return ($Arg1VarName < $Arg0VarName) ? ($ReturnType)0.0 : ($ReturnType)1.0;
  }]>;

// sign
OclBuiltinImpl sign_f32_generic_not_shared = OclBuiltinImpl<sign,
  [v8f32, v16f32] , 0, [{
    return isnan($Arg0VarName) ? ($ReturnType)0.0f :
           $Arg0VarName > 0.0f ? ($ReturnType)1.0f :
           $Arg0VarName < 0.0f ? ($ReturnType)-1.0f : $Arg0VarName;
  }]>;
OclBuiltinImpl sign_f64_generic_not_shared = OclBuiltinImpl<sign,
  [v3f64, v4f64, v8f64] , 0, [{
    return isnan($Arg0VarName) ? ($ReturnType)0.0 :
           $Arg0VarName > 0.0 ?  ($ReturnType)1.0 :
           $Arg0VarName < 0.0 ? ($ReturnType)-1.0 : $Arg0VarName;
  }]>;
