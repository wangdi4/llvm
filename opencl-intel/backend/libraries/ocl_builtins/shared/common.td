// vim:ts=2:sw=2:et:

//promote and expand

  OclBuiltinImpl step_promote = OclBuiltinImpl <step, [v2f32, v3f32], 0, Promote2>;
  OclBuiltinImpl step_expand  = OclBuiltinImpl <step, [v16f64], 0, Expand2>;
  OclBuiltinImpl sign_promote = OclBuiltinImpl <sign, [v2f32, v3f32], 0, Promote1>;
  OclBuiltinImpl sign_expand  = OclBuiltinImpl <sign, [v16f64], 0, Expand1>;

// ======GENERIC CPU specific implementation=======================
//  clamp
//
//  Originally fmin/fmax usage in clamp is defined by spec.
//  fmin/fmax treats NaNs differently than min/max.
//  If one argument is a NaN, fmin() returns the other argument.
//  So on CPU fmin checks for NaN in second argument.
//  Luckily x86 instruction minps, maxps treats NaN is 1st argument correctly
//  according to OpenCL 1.2 spec.
//  They (minps/maxps) can be used as in original code before tblgen port

OclBuiltinImpl clamp_fgeneric = OclBuiltinImpl<clamp,
  [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
   v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, [{
    return min(max($Arg0VarName, $Arg1VarName), $Arg2VarName);
  }]>;

// vector, scalar BI generation
//======v4f32 and v2f64 implementation===============================

// step
OclBuiltinImpl step_v4f32 = OclBuiltinImpl<step,
  [v4f32] , 0, [{
    $ReturnType $ReturnVarName;
    $ReturnVarName = _mm_cmpge_ps($Arg1VarName,$Arg0VarName);
    $ReturnVarName = _mm_and_ps($ReturnVarName, f4const_oneStorage);
    return $ReturnVarName;
  }]>;

OclBuiltinImpl step_v2f64 = OclBuiltinImpl<step,
  [v2f64] , 0, [{
    $ReturnType $ReturnVarName;
    $ReturnVarName = _mm_cmpge_pd($Arg1VarName,$Arg0VarName);
    $ReturnVarName = _mm_and_pd($ReturnVarName, d2const_oneStorage);
    return $ReturnVarName;
  }]>;


// sign
OclBuiltinImpl sign_v4f32 = OclBuiltinImpl<sign,[v4f32] , 0, [{
    //if X=nan -> x=0
    $ReturnType f4res = _mm_cmpord_ps($Arg0VarName, $Arg0VarName); // f4res = 0 if x = nan
    $Arg0VarName = _mm_and_ps(f4res, $Arg0VarName);//if f4res=0 (x=nan) x=0

    // Get sign flags
    $ReturnType signFlag = _mm_and_ps($Arg0VarName, f4const_minusZeroStorage);

    //if x>0 return res=1
    $ReturnType res = _mm_cmpgt_ps($Arg0VarName, _mm_setzero_ps());
    res = _mm_and_ps(res, f4const_oneStorage); //res = 1 if x>0

    //if x<-0 return res1=-1
    $ReturnType res1 = _mm_cmplt_ps($Arg0VarName, f4const_minusZeroStorage); ////res1 = -1 if x<-0
    res1 = _mm_and_ps(res1, f4const_minusOneStorage);
    res = _mm_or_ps(res, res1);
    //if x=0 res=0 (if x=0 res0=res1=0) - no need to do anything

    // Set res=-0 if x=-0
    res = _mm_or_ps(res, signFlag);
    return res;
  }]>;

OclBuiltinImpl sign_v2f64 = OclBuiltinImpl<sign,[v2f64] , 0, [{
    //if X=nan -> x=0
    $ReturnType d2res = _mm_cmpord_pd($Arg0VarName, $Arg0VarName); // d2res = 0 if x = nan
    $Arg0VarName = _mm_and_pd(d2res, $Arg0VarName);//if d2res=0 (x=nan) x=0

    // Get sign flags
    $ReturnType signFlag = _mm_and_pd($Arg0VarName, d2const_minusZeroStorage);

    //if x>0 return res=1
    $ReturnType res = _mm_cmpgt_pd($Arg0VarName, _mm_setzero_pd());
    res = _mm_and_pd(res, d2const_oneStorage); //res = 1 if x>0

    //if x<-0 return res1=-1
    $ReturnType res1 = _mm_cmplt_pd($Arg0VarName, d2const_minusZeroStorage); ////res1 = -1 if x<-0
    res1 = _mm_and_pd(res1, d2const_minusOneStorage);
    res = _mm_or_pd(res, res1);
    //if x=0 res=0 (if x=0 res0=res1=0) - no need to do anything

    // Set res=-0 if x=-0
    res = _mm_or_pd(res, signFlag);
    return res;
  }]>;

