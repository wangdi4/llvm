code masked_generic_float_impl_code = [{
    int$VecLength n$Arg1VarName = as_int$VecLength($Arg1VarName);

    $Arg0VecType _safein_ = ($Arg0VecType)($DEFAULT);
    $Arg0VecType m$Arg0VarName = select(_safein_, $Arg0VarName, n$Arg1VarName);

    $ReturnType _ret_ = $Func(m$Arg0VarName);
    $ReturnType _safeout_ = ($ReturnType)(0);
    return select(_safeout_, _ret_, n$Arg1VarName);
  }];

code masked_generic_double_impl_code = [{
    long$VecLength n$Arg1VarName = as_long$VecLength($Arg1VarName);

    $Arg0VecType _safein_ = ($Arg0VecType)($DEFAULT);
    $Arg0VecType m$Arg0VarName = select(_safein_, $Arg0VarName, n$Arg1VarName);

    $ReturnType _ret_ = $Func(m$Arg0VarName);
    $ReturnType _safeout_ = ($ReturnType)(0);
    return select(_safeout_, _ret_, n$Arg1VarName);
  }];

list<OclType> math_float_types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32];
list<OclType> math_double_types = [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64];

OclBuiltinImpl acos_svml_float_mask = OclBuiltinImpl<acos_mask, math_float_types, 0, masked_generic_float_impl_code, [["$DEFAULT", "0"]]>;
OclBuiltinImpl acos_svml_double_mask = OclBuiltinImpl<acos_mask, math_double_types, 0, masked_generic_double_impl_code, [["$DEFAULT", "0"]]>;
