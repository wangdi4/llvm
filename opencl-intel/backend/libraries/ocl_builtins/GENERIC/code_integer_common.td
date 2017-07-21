// vim:ts=2:sw=2:et:
// NOTE: common code used in integer built-ins

code abs_diff_igeneric_code = [{
    return as_$ReturnType(($Arg0Type)(max($Arg0VarName, $Arg1VarName) - min($Arg0VarName, $Arg1VarName)));
  }];

code add_sat_svector_code = [{
    $Arg0BaseType zero = 0;
    $ReturnType r = $Arg0VarName + $Arg1VarName;
    $ReturnType p = (($Arg0VarName | $Arg1VarName) > zero) & (r < zero);
    $ReturnType n = (($Arg0VarName & $Arg1VarName) < zero) & (r >= zero);
    return  (n == zero) ?
            (p == zero) ? r : ($ReturnType)generic_max_$Arg0BaseType :
            ($ReturnType)generic_min_$Arg0BaseType;
  }];

code add_sat_unsigned_code = [{
    $ReturnType r = $Arg0VarName + $Arg1VarName;
    return (r < $Arg0VarName) ? ($ReturnType)generic_max_$Arg0BaseType : r;
  }];

code rhadd_igeneric_code = [{
    $ReturnType s, c;
    s = $Arg0VarName ^ $Arg1VarName;
    c = $Arg0VarName & $Arg1VarName;
    return (s >> (($ReturnType)1)) + (c | (s & (($ReturnType)1)));
  }];

code max_ivector_code = [{
    return ($Arg0VarName > $Arg1VarName) ? $Arg0VarName : $Arg1VarName;
  }];

code min_ivector_code = [{
    return ($Arg0VarName < $Arg1VarName) ? $Arg0VarName : $Arg1VarName;
  }];

code sub_sat_svector_code = [{
    $Arg0BaseType zero = 0;
    $ReturnType r = $Arg0VarName - $Arg1VarName;
    $ReturnType p8 = ($Arg1VarName < zero) & (r < $Arg0VarName);
    $ReturnType n8 = ($Arg1VarName > zero) & (r > $Arg0VarName);
    return  (n8 == zero) ?
            (p8 == zero) ? r : ($ReturnType)generic_max_$Arg0BaseType :
            ($ReturnType)generic_min_$Arg0BaseType;
  }];

code sub_sat_unsigned_code = [{
    $ReturnType r = $Arg0VarName - $Arg1VarName;
    return ($Arg0VarName < r) ? ($Arg0BaseType)0 : r;
  }];
