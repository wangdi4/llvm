defvar bitselect_f_code = [{
    // res1 is $Arg0VarName if $Arg2VarName is 0
    $Arg0VarName = __builtin_astype((~__builtin_astype(($Arg2VarName), $Arg0igentype) & __builtin_astype(($Arg0VarName), $Arg0igentype)), $Arg0Type);
    // res2 bit is $Arg1VarName if $Arg2VarName is 1
    $Arg1VarName = __builtin_astype((__builtin_astype(($Arg2VarName), $Arg0igentype) & __builtin_astype(($Arg1VarName), $Arg0igentype)), $Arg0Type);
    return __builtin_astype((__builtin_astype(($Arg0VarName), $Arg0igentype) | __builtin_astype(($Arg1VarName), $Arg0igentype)), $ReturnType);
  }];

defvar bitselect_i_code = [{
    $Arg0VarName = __builtin_astype(((~ $Arg2VarName) &  $Arg0VarName), $Arg0Type);
    $Arg1VarName = __builtin_astype(($Arg2VarName &  $Arg1VarName), $Arg0Type);
    return __builtin_astype(($Arg0VarName |  $Arg1VarName), $ReturnType);
  }];
