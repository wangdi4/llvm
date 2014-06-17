// OpenCL 2.0 -cl-fast-relaxed-math functions
// cos_rm
OclBuiltinImpl cos_svml_rm = OclBuiltinImpl<cos_rm, cos_rm.Types, 0, [{
    return __ocl_svml_$Target_cos$SVMLSuffix_rm($Arg0VarName);
  }]>;

// exp_rm
OclBuiltinImpl exp_svml_rm = OclBuiltinImpl<exp_rm, exp_rm.Types, 0, [{
    return __ocl_svml_$Target_exp$SVMLSuffix_rm($Arg0VarName);
  }]>;

// exp2_rm
OclBuiltinImpl exp2_svml_rm = OclBuiltinImpl<exp2_rm, exp2_rm.Types, 0, [{
    return __ocl_svml_$Target_exp2$SVMLSuffix_rm($Arg0VarName);
  }]>;

// exp10_rm
OclBuiltinImpl exp10_svml_rm = OclBuiltinImpl<exp10_rm, exp10_rm.Types, 0, [{
    return __ocl_svml_$Target_exp10$SVMLSuffix_rm($Arg0VarName);
  }]>;

// log_rm
OclBuiltinImpl log_svml_rm = OclBuiltinImpl<log_rm, log_rm.Types, 0, [{
    return __ocl_svml_$Target_log$SVMLSuffix_rm($Arg0VarName);
  }]>;

// log2_rm
OclBuiltinImpl log2_svml_rm = OclBuiltinImpl<log2_rm, log2_rm.Types, 0, [{
    return __ocl_svml_$Target_log2$SVMLSuffix_rm($Arg0VarName);
  }]>;

// pow_rm
OclBuiltinImpl pow_svml_rm = OclBuiltinImpl<pow_rm, pow_rm.Types, 0, [{
    return __ocl_svml_$Target_pow$SVMLSuffix_rm($Arg0VarName, $Arg1VarName);
  }]>;

// sin_rm
OclBuiltinImpl sin_svml_rm = OclBuiltinImpl<sin_rm, sin_rm.Types, 0, [{
    return __ocl_svml_$Target_sin$SVMLSuffix_rm($Arg0VarName);
  }]>;

// tan_rm
OclBuiltinImpl tan_svml_rm = OclBuiltinImpl<tan_rm, tan_rm.Types, 0, [{
    return __ocl_svml_$Target_tan$SVMLSuffix_rm($Arg0VarName);
  }]>;
