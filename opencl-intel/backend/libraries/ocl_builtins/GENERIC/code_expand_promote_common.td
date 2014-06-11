// vim:ts=2:sw=2:et:
// NOTE: common code used in OclBuiltinImpl to expand or promote built-in

  code Prolog = [{}];
  code Epilog = [{}];

  code Expand1 = [{
    $ReturnType $ReturnVarName;
    $ReturnVarName$ExpandLoPattern = $ExpandLoFunc($Arg0VarName$ExpandLoPattern);
    $ReturnVarName$ExpandHiPattern = $ExpandHiFunc($Arg0VarName$ExpandHiPattern);
    return $ReturnVarName;
  }];

  code Expand2 = [{
    $ReturnType $ReturnVarName;
    $ReturnVarName$ExpandLoPattern = $ExpandLoFunc($Arg0VarName$ExpandLoPattern, $Arg1VarName$ExpandLoPattern);
    $ReturnVarName$ExpandHiPattern = $ExpandHiFunc($Arg0VarName$ExpandHiPattern, $Arg1VarName$ExpandHiPattern);
    return $ReturnVarName;
  }];

  code Expand3 = [{
    $ReturnType $ReturnVarName;
    $ReturnVarName$ExpandLoPattern = $ExpandLoFunc($Arg0VarName$ExpandLoPattern, $Arg1VarName$ExpandLoPattern, $Arg2VarName$ExpandLoPattern);
    $ReturnVarName$ExpandHiPattern = $ExpandHiFunc($Arg0VarName$ExpandHiPattern, $Arg1VarName$ExpandHiPattern, $Arg2VarName$ExpandHiPattern);
    return $ReturnVarName;
  }];

  code Promote1 = [{
    $NativeArg0Type n$Arg0VarName = ($NativeArg0Type)0;
    n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeReturnType $ReturnVarName = $NativeFunc(n$Arg0VarName);
    return $ReturnVarName$Pattern;
  }];

  code Promote2 = [{
    $NativeArg0Type n$Arg0VarName = ($NativeArg0Type)0;
    n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName = ($NativeArg1Type)0;
    n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeReturnType $ReturnVarName = $NativeFunc(n$Arg0VarName, n$Arg1VarName);
    return $ReturnVarName$Pattern;
  }];

   code Promote3 = [{
    $NativeArg0Type n$Arg0VarName = ($NativeArg0Type)0;
    n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName = ($NativeArg1Type)0;
    n$Arg1VarName$Pattern = $Arg1VarName;
    $NativeArg2Type n$Arg2VarName = ($NativeArg2Type)0;
    n$Arg2VarName$Pattern = $Arg2VarName;
    $NativeReturnType $ReturnVarName = $NativeFunc(n$Arg0VarName, n$Arg1VarName,n$Arg2VarName);
    return $ReturnVarName$Pattern;
  }];

  code Promote1_scalar = [{
    $NativeArg0Type n$Arg0VarName = ($NativeArg0Type)0;
    n$Arg0VarName$Pattern = $Arg0VarName;
    return $NativeFunc(n$Arg0VarName);
  }];

  code Promote2_scalar = [{
    $NativeArg0Type n$Arg0VarName = ($NativeArg0Type)0;
    n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeArg1Type n$Arg1VarName = ($NativeArg1Type)0;
    n$Arg1VarName$Pattern = $Arg1VarName;
    return $NativeFunc(n$Arg0VarName, n$Arg1VarName);
  }];

  code Promote_fract = [{
    $NativeArg0Type n$Arg0VarName;
    $NativeArg0Type p$Arg1VarName;
    n$Arg0VarName$Pattern = $Arg0VarName;
    $NativeReturnType $ReturnVarName = $NativeFunc(n$Arg0VarName, &p$Arg1VarName);
    *$Arg1VarName = p$Arg1VarName$Pattern;
    return $ReturnVarName$Pattern;
  }];

  // expand fract, modf, sincos
  code Expand_2Ptr = [{
    $ReturnType $ReturnVarName;
    $ExpandLoReturnType nlo$Arg1VarName;
    $ExpandHiReturnType nhi$Arg1VarName;
    $ReturnVarName$ExpandLoPattern = $ExpandLoFunc($Arg0VarName$ExpandLoPattern, &nlo$Arg1VarName);
    $Arg1VarName$ExpandLoPatternPtr = nlo$Arg1VarName;
    $ReturnVarName$ExpandHiPattern = $ExpandHiFunc($Arg0VarName$ExpandHiPattern, &nhi$Arg1VarName);
    $Arg1VarName$ExpandHiPatternPtr = nhi$Arg1VarName;
    return $ReturnVarName;
  }];

  // Expand frexp
  code Expand_frexp_lgamma = [{
    $ReturnType $ReturnVarName;
    int$ExpandLoSuffix nlo$Arg1VarName;
    int$ExpandHiSuffix nhi$Arg1VarName;
    $ReturnVarName$ExpandLoPattern = $ExpandLoFunc($Arg0VarName$ExpandLoPattern, &nlo$Arg1VarName);
    $Arg1VarName$ExpandLoPatternPtr = nlo$Arg1VarName;
    $ReturnVarName$ExpandHiPattern = $ExpandHiFunc($Arg0VarName$ExpandHiPattern, &nhi$Arg1VarName);
    $Arg1VarName$ExpandHiPatternPtr = nhi$Arg1VarName;
    return $ReturnVarName;
  }];

  // expand remquo
  code Expand_remquo = [{
    $ReturnType $ReturnVarName;
    int$ExpandLoSuffix nlo$Arg2VarName;
    int$ExpandHiSuffix nhi$Arg2VarName;
    $ReturnVarName$ExpandLoPattern = $ExpandLoFunc($Arg0VarName$ExpandLoPattern, $Arg1VarName$ExpandLoPattern, &nlo$Arg2VarName);
    $Arg2VarName$ExpandLoPatternPtr = nlo$Arg2VarName;
    $ReturnVarName$ExpandHiPattern = $ExpandHiFunc($Arg0VarName$ExpandHiPattern, $Arg1VarName$ExpandHiPattern, &nhi$Arg2VarName);
    $Arg2VarName$ExpandHiPatternPtr = nhi$Arg2VarName;
    return $ReturnVarName;
  }];