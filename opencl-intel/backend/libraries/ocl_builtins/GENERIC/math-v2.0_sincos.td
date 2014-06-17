// sincos_rm
code sincos_svml_rm = [{
    return __ocl_svml_$Target_sincos$SVMLSuffix_rm($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];
OclBuiltinImpl sincos_pas_svml_rm = OclBuiltinImpl<sincos_pas_rm, sincos_pas_rm.Types, 0, sincos_svml_rm>;
OclBuiltinImpl sincos_las_svml_rm = OclBuiltinImpl<sincos_las_rm, sincos_las_rm.Types, 0, sincos_svml_rm>;
OclBuiltinImpl sincos_gas_svml_rm = OclBuiltinImpl<sincos_gas_rm, sincos_gas_rm.Types, 0, sincos_svml_rm>;
