// vim:ts=2:sw=2:et:

//promote and expand
// NOTE: all the promote for v2f32 and v3f32 were done in "corei7/common.td"
//     all the expand also done in "corei7/common.td"/
//  double3 (v3f64) will promote to use double4 version

  OclBuiltinImpl degrees_promote    = OclBuiltinImpl  <degrees,    [v3f64], 0, Promote1>;
  OclBuiltinImpl radians_promote    = OclBuiltinImpl  <radians,    [v3f64], 0, Promote1>;
  OclBuiltinImpl step_promote       = OclBuiltinImpl  <step,       [v3f64], 0, Promote2>;
  OclBuiltinImpl sign_promote       = OclBuiltinImpl  <sign,       [v3f64], 0, Promote1>;
  OclBuiltinImpl smoothstep_promote = OclBuiltinImpl  <smoothstep, [v3f64], 0, Promote3>;
  
// vector, scalar BI generation 
// also done in "corei7/common.td"

//======v8f32 and v4f64 implementation===============================

// step
OclBuiltinImpl step_v8f32 = OclBuiltinImpl<step, [v8f32] , 0, [{
    $ReturnType $ReturnVarName;
    //if x>=edge return -1 else (x<edge) return 0
    $ReturnVarName = _mm256_cmp_ps($Arg1VarName,$Arg0VarName, _CMP_GE_OS);
    //Convert -1 to 1
    $ReturnVarName = _mm256_and_ps($ReturnVarName, f8const_oneStorage);
    return $ReturnVarName;
  }]>;
  
OclBuiltinImpl step_v4f64 = OclBuiltinImpl<step, [v4f64] , 0, [{
    $ReturnType $ReturnVarName;
    //if x>=edge return -1 else (x<edge) return 0
    $ReturnVarName = _mm256_cmp_pd($Arg1VarName,$Arg0VarName, _CMP_GE_OS);
    //Convert -1 to 1
    $ReturnVarName = _mm256_and_pd($ReturnVarName, d4const_oneStorage);
    return $ReturnVarName;
  }]>;
   
 
// sign
OclBuiltinImpl sign_v8f32 = OclBuiltinImpl<sign,[v8f32] , 0, [{
    //if x=nan return res=0
    $ReturnType f8res = _mm256_cmp_ps($Arg0VarName, $Arg0VarName, _CMP_ORD_Q); // f8res = 0 if x = nan
    $Arg0VarName = _mm256_and_ps(f8res, $Arg0VarName); //if f8res=0 (x=nan) x=0

    // Get sign flags
    $ReturnType signFlag = _mm256_and_ps($Arg0VarName, f8const_minusZeroStorage);

    //if x>0 return res=1
    $ReturnType res = _mm256_cmp_ps($Arg0VarName, _mm256_setzero_ps(), _CMP_GT_OS);
    res = _mm256_and_ps(res, f8const_oneStorage); //res = 1 if x>0

    //if x<-0 return res1=-1
    $ReturnType res1 = _mm256_cmp_ps($Arg0VarName, f8const_minusZeroStorage, _CMP_LT_OS); //res1 = -1 if x<-0
    res1 = _mm256_and_ps(res1, f8const_minusOneStorage);
    res = _mm256_or_ps(res, res1);
    //if x=0 res=0 (if x=0 res0=res1=0) - no need to do anything

    // Set res=-0 if x=-0
    res = _mm256_or_ps(res, signFlag);
    return res;
  }]>;

OclBuiltinImpl sign_v4f64 = OclBuiltinImpl<sign,[v4f64] , 0, [{
    //if X=nan -> x=0
    $ReturnType d4res = _mm256_cmp_pd($Arg0VarName, $Arg0VarName, _CMP_ORD_Q); // d4res = 0 if x = nan
    $Arg0VarName = _mm256_and_pd(d4res, $Arg0VarName); //if d4res=0 (x=nan) x=0

    // Get sign flags
    $ReturnType signFlag = _mm256_and_pd($Arg0VarName, d4const_minusZeroStorage);

    //if x>0 return res=1
    $ReturnType res = _mm256_cmp_pd($Arg0VarName, _mm256_setzero_pd(), _CMP_GT_OS);
    res = _mm256_and_pd(res, d4const_oneStorage); //res = 1 if x>0

    //if x<-0 return res1=-1
    $ReturnType res1 = _mm256_cmp_pd($Arg0VarName, d4const_minusZeroStorage,_CMP_LT_OS); //res1 = -1 if x<-0
    res1 = _mm256_and_pd(res1, d4const_minusOneStorage);
    res = _mm256_or_pd(res, res1);
    //if x=0 res=0 (if x=0 res0=res1=0) - no need to do anything

    // Set res=-0 if x=-0
    res = _mm256_or_pd(res, signFlag);
    return res;
  }]>;
