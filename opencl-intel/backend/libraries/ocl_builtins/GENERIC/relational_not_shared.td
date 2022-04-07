// vim:ts=2:sw=2:et:
// Copyright (c) 2006-2012 Intel Corporation
// Relational
// OpenCL 1.2 rev 15, Table 6.14

// isnan

OclBuiltinImpl isnan_f64_promote_not_shared = OclBuiltinImpl<isnan, [v3f64, v4f64], 0, Promote1>;

OclBuiltinImpl isnan_f64_v8 = OclBuiltinImpl<isnan, [v8f64], 0, [{
    return $Arg0VarName != $Arg0VarName;
  }]>;
OclBuiltinImpl isnormalal_f64_promote_not_shared = OclBuiltinImpl<isnormal, [v3f64], 0, Promote1>;


