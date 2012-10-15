// vim:ts=2:sw=2:et:
// Copyright (c) 2006-2012 Intel Corporation
// Relational
// OpenCL 1.2 rev 15, Table 6.14

// isnan
OclBuiltinImpl isnan_f64_v1_v8 = OclBuiltinImpl<isnan, [v1f64, v8f64], 0, [{
    return $Arg0VarName != $Arg0VarName;
  }]>;
OclBuiltinImpl isnan_f64_promote = OclBuiltinImpl<isnan, [v2f64, v3f64, v4f64], 0, Promote1>;
OclBuiltinImpl isnan_f64_expand = OclBuiltinImpl<isnan, [v16f64], 0, Expand1>;

// isnormal
OclBuiltinImpl isnormal_f64_v1 = OclBuiltinImpl<isnormal, [v1f64], 0, [{
    return ( ((as_long($Arg0VarName) & double_const_expMask)!=0) && ( (as_long($Arg0VarName) & double_const_expMask) != double_const_expMask) )?1:0;
  }]>;
OclBuiltinImpl isnormal_f64_v8 = OclBuiltinImpl<isnormal, [v8f64], 0, [{
    return ( ((as_long8($Arg0VarName) & double_const_expMask)!=0) && ( (as_long8($Arg0VarName) & double_const_expMask) != double_const_expMask) )?(long8)-1:(long8)0;
  }]>;
OclBuiltinImpl isnormalal_f64_promote = OclBuiltinImpl<isnormal, [v2f64, v3f64, v4f64], 0, Promote1>;
OclBuiltinImpl isnormal_f64_expand = OclBuiltinImpl<isnormal, [v16f64], 0, Expand1>;
