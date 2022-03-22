defvar frexp_svml = [{
    return __ocl_svml_$Target_frexp$SVMLSuffix($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];

defvar lgamma_r_svml = [{
    return __ocl_svml_$Target_lgammar$SVMLSuffix($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];

defvar modf_svml = [{
    return __ocl_svml_$Target_modf$SVMLSuffix($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];

defvar modf_ret2ptr = [{
    $Arg0Type res = modf($Arg0VarName, $Arg2VarName);
    *$Arg1VarName = res;
  }];

defvar remquo_svml = [{
    return __ocl_svml_$Target_remquo$SVMLSuffix($Arg0VarName, $Arg1VarName, ($Arg2NoASType)$Arg2VarName);
  }];

defvar sincos_svml = [{
    return __ocl_svml_$Target_sincos$SVMLSuffix($Arg0VarName, ($Arg1NoASType)$Arg1VarName);
  }];

defvar sincos_ret2ptr = [{
    $Arg0Type res = sincos($Arg0VarName, $Arg2VarName);
    *$Arg1VarName = res;
  }];
