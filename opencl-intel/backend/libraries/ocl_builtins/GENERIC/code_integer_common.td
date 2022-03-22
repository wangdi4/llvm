// vim:ts=2:sw=2:et:
// NOTE: common code used in integer built-ins

defvar abs_diff_igeneric_code = [{
    return as_$ReturnType(($Arg0Type)(max($Arg0VarName, $Arg1VarName) - min($Arg0VarName, $Arg1VarName)));
  }];

defvar add_sat_svector_code = [{
    $Arg0BaseType zero = 0;
    $ReturnType r = $Arg0VarName + $Arg1VarName;
    $ReturnType p = (($Arg0VarName | $Arg1VarName) > zero) & (r < zero);
    $ReturnType n = (($Arg0VarName & $Arg1VarName) < zero) & (r >= zero);
    return  (n == zero) ?
            (p == zero) ? r : ($ReturnType)generic_max_$Arg0BaseType :
            ($ReturnType)generic_min_$Arg0BaseType;
  }];

defvar add_sat_unsigned_code = [{
    $ReturnType r = $Arg0VarName + $Arg1VarName;
    return (r < $Arg0VarName) ? ($ReturnType)generic_max_$Arg0BaseType : r;
  }];

defvar clamp_s_impl = [{
    $Arg1VecType vec$Arg1VarName = ($Arg1VecType)$Arg1VarName;
    $Arg2VecType vec$Arg2VarName = ($Arg2VecType)$Arg2VarName;
    return $Func($Arg0VarName, vec$Arg1VarName, vec$Arg2VarName);
  }];

defvar rhadd_igeneric_code = [{
    $ReturnType s, c;
    s = $Arg0VarName ^ $Arg1VarName;
    c = $Arg0VarName & $Arg1VarName;
    return (s >> (($ReturnType)1)) + (c | (s & (($ReturnType)1)));
  }];

defvar max_ivector_code = [{
    return ($Arg0VarName > $Arg1VarName) ? $Arg0VarName : $Arg1VarName;
  }];

defvar max_s_impl =[{
    $Arg1VecType vec$Arg1VarName = ($Arg1VecType)$Arg1VarName;
    return $Func($Arg0VarName, vec$Arg1VarName);
  }];

defvar min_ivector_code = [{
    return ($Arg0VarName < $Arg1VarName) ? $Arg0VarName : $Arg1VarName;
  }];

defvar min_s_impl = [{
    $Arg1VecType vec$Arg1VarName = ($Arg1VecType)$Arg1VarName;
    return $Func($Arg0VarName, vec$Arg1VarName);
  }];

defvar sub_sat_svector_code = [{
    $Arg0BaseType zero = 0;
    $ReturnType r = $Arg0VarName - $Arg1VarName;
    $ReturnType p8 = ($Arg1VarName < zero) & (r < $Arg0VarName);
    $ReturnType n8 = ($Arg1VarName > zero) & (r > $Arg0VarName);
    return  (n8 == zero) ?
            (p8 == zero) ? r : ($ReturnType)generic_max_$Arg0BaseType :
            ($ReturnType)generic_min_$Arg0BaseType;
  }];

defvar sub_sat_unsigned_code = [{
    $ReturnType r = $Arg0VarName - $Arg1VarName;
    return ($Arg0VarName < r) ? ($Arg0BaseType)0 : r;
  }];

/// Common functions
defvar clamp_f_code = [{
    return fmax(fmin($Arg0VarName, $Arg2VarName), $Arg1VarName);
  }];

defvar max_f_code = [{
    // !!! order of arguments in comparison is important. DO NOT CHANGE
    // Reason is how NaN is treated. "y < x ? y : x" sequence generates max_ps x86 intrinsics
    // max_ps treats NaN differently in case NaN is in 1st argument or in 2nd argument
    return ($Arg1VarName < $Arg0VarName) ? $Arg0VarName : $Arg1VarName;
  }];

defvar min_f_code = [{
    return ($Arg0VarName < $Arg1VarName) ? $Arg0VarName : $Arg1VarName;
  }];
