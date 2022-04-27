code Expand_any = [{  
    $ReturnType res1, res2;
    res1 = any($Arg0VarName.lo);
    res2 = any($Arg0VarName.hi);
    return (res1 || res2);
  }];
code Expand_all = [{
    $ReturnType res1, res2;
    res1 = all($Arg0VarName.lo);
    res2 = all($Arg0VarName.hi);
    return (res1 && res2);
  }];
code Expand_any_3i8_3i16 = [{
    $ReturnType res1, res2, res3;
    res1 = any($Arg0VarName.s0);
    res2 = any($Arg0VarName.s1);
    res3 = any($Arg0VarName.s2);
    return (res1 || res2 || res3);
  }];
code Expand_all_3i8_3i16 = [{
    $ReturnType res1, res2, res3;
    res1 = all($Arg0VarName.s0);
    res2 = all($Arg0VarName.s1);
    res3 = all($Arg0VarName.s2);
    return (res1 && res2 && res3);
  }];
code Expand_any_all_char8 = [{
    char16 temp;
    temp.lo = $Arg0VarName;
    temp.hi = $Arg0VarName;
    return $Func(temp);
  }];
code Expand_any_all_char4 = [{
    char16 temp;
    temp.lo.lo = $Arg0VarName;
    temp.lo.hi = $Arg0VarName;
    temp.hi.lo = $Arg0VarName;
    temp.hi.hi = $Arg0VarName;
    return $Func(temp);
  }];
code Expand_4i16 = [{
    short8 temp;
    temp.lo = $Arg0VarName;
    temp.hi = $Arg0VarName;
    return $Func(temp);
  }];
code Expand_2i32 = [{
    int4 temp;
    temp.lo = $Arg0VarName;
    temp.hi = $Arg0VarName;
    return $Func(temp);
  }];
code Expand_3i32 = [{
    int4 temp;
    temp.s012 = $Arg0VarName;
    temp.s3 = $Arg0VarName.s0;
    return $Func(temp);
  }];

code v3_double_1arg = [{
    double4 tmp;
    long4 ret;
    tmp.s012 = $Arg0VarName;
    ret = $Func(tmp);
    return ret.s012;
  }];
code v3_double_2arg = [{
    double4 tmp1;
    double4 tmp2;
    long4 ret;
    tmp1.s012 = $Arg0VarName;
    tmp2.s012 = $Arg1VarName;
    ret = $Func(tmp1, tmp2);
    return ret.s012;
  }];

code any_all_scalar8 = [{ 
    return as_$ReturnType(($Arg0VarName & 0x80) != 0);
  }];

code any_all_scalar16 = [{ 
    return as_$ReturnType(($Arg0VarName & 0x8000) != 0);
  }];

code any_all_scalar32 = [{ 
    return as_$ReturnType(($Arg0VarName & 0x80000000L) != 0);
  }];

code any_all_scalar64 = [{ 
    return as_$ReturnType(($Arg0VarName & 0x8000000000000000L) != 0);
  }];