//
//Scalar relational
//
code v1_relational = [{
  $ReturnType ret = $RemovePrefixFunc($Arg0VarName, $Arg1VarName);
  return ret * ($ReturnType)-1;
}];

code v_relational_d = [{
  $ReturnType ret = downConvert($RemovePrefixFunc($Arg0VarName, $Arg1VarName));
  return ret & ($ReturnType)1;
}];

code v_relational_f = [{
  $ReturnType ret = $RemovePrefixFunc($Arg0VarName, $Arg1VarName);
  return ret & ($ReturnType)1;
}];

code downCvt4 = [{
  return convert_int4($Arg0VarName);
}];

code downCvt8 = [{
  return convert_int8($Arg0VarName);
}];

code downCvt16 = [{
  return convert_int16($Arg0VarName);
}];
//
//downConvert utilty
//
OclBuiltinImpl downConvertImpl4 = OclBuiltinImpl<downConvert, [v4i64], 0, downCvt4>;
OclBuiltinImpl downConvertImpl8 = OclBuiltinImpl<downConvert, [v8i64], 0, downCvt8>;
OclBuiltinImpl downConvertImpl16= OclBuiltinImpl<downConvert, [v16i64], 0, downCvt16>;
//
//Scalar
//
OclBuiltinImpl isless1impl = OclBuiltinImpl<scalar_isless, [v1f32, v1f64]
, 0, v1_relational>;
OclBuiltinImpl isgreater1impl = OclBuiltinImpl<scalar_isgreater, [v1f32, v1f64]
  , 0, v1_relational>;
OclBuiltinImpl isequal1impl = OclBuiltinImpl<scalar_isequal, [v1f32, v1f64]
  , 0, v1_relational>;
OclBuiltinImpl isgreaterequal1impl = OclBuiltinImpl<scalar_isgreaterequal, [v1f32, v1f64]
  , 0, v1_relational>;
OclBuiltinImpl islessequal1impl = OclBuiltinImpl<scalar_islessequal, [v1f32, v1f64]
  , 0, v1_relational>;
OclBuiltinImpl islessgreater1impl = OclBuiltinImpl<scalar_islessgreater, [v1f32, v1f64]
  , 0, v1_relational>;
OclBuiltinImpl isnotequal1impl = OclBuiltinImpl<scalar_isnotequal, [v1f32, v1f64]
  , 0, v1_relational>;
//
//Vectors
//
OclBuiltinImpl isless1impl_f = OclBuiltinImpl<v_isless, [v4f32, v8f32, v16f32]
, 0, v_relational_f>;
OclBuiltinImpl isless1impl_d = OclBuiltinImpl<v_isless, [v4f64, v8f64, v16f64]
, 0, v_relational_d>;
OclBuiltinImpl isgreater1impl_f = OclBuiltinImpl<v_isgreater, [v4f32, v8f32, v16f32]
  , 0, v_relational_f>;
OclBuiltinImpl isgreater1impl_d = OclBuiltinImpl<v_isgreater, [v4f64, v8f64, v16f64]
  , 0, v_relational_d>;
OclBuiltinImpl isequal1impl_f = OclBuiltinImpl<v_isequal, [v4f32, v8f32, v16f32]
  , 0, v_relational_f>;
OclBuiltinImpl isequal1impl_d = OclBuiltinImpl<v_isequal, [v4f64, v8f64, v16f64]
  , 0, v_relational_d>;
OclBuiltinImpl isgreaterequal1impl_f = OclBuiltinImpl<v_isgreaterequal, [v4f32, v8f32, v16f32]
  , 0, v_relational_f>;
OclBuiltinImpl isgreaterequal1impl_d = OclBuiltinImpl<v_isgreaterequal, [v4f64, v8f64, v16f64]
  , 0, v_relational_d>;
OclBuiltinImpl islessequal1impl_f = OclBuiltinImpl<v_islessequal, [v4f32, v8f32, v16f32]
  , 0, v_relational_f>;
OclBuiltinImpl islessequal1impl_d = OclBuiltinImpl<v_islessequal, [v4f64, v8f64, v16f64]
  , 0, v_relational_d>;
OclBuiltinImpl islessgreater1impl_f = OclBuiltinImpl<v_islessgreater, [v4f32, v8f32, v16f32]
  , 0, v_relational_f>;
OclBuiltinImpl islessgreater1impl_d = OclBuiltinImpl<v_islessgreater, [v4f64, v8f64, v16f64]
  , 0, v_relational_d>;
OclBuiltinImpl isnotequal1impl_f = OclBuiltinImpl<v_isnotequal, [v4f32, v8f32, v16f32]
  , 0, v_relational_f>;
OclBuiltinImpl isnotequal1impl_d = OclBuiltinImpl<v_isnotequal, [v4f64, v8f64, v16f64]
  , 0, v_relational_d>;

