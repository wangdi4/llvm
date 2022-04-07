code masked_load4x4 = [{
    if ($Arg2VarName.s0)
      $Arg1VarName[0] = $Arg0VarName[0];
    else $Arg1VarName[0] = 0;
    if ($Arg2VarName.s1)
      $Arg1VarName[1] = $Arg0VarName[1];
    else $Arg1VarName[1] = 0;
    if ($Arg2VarName.s2)
      $Arg1VarName[2] = $Arg0VarName[2];
    else $Arg1VarName[2] = 0;
    if ($Arg2VarName.s3)
      $Arg1VarName[3] = $Arg0VarName[3];
    else $Arg1VarName[3] = 0;
  }];
code masked_store4x4 = [{
    if ($Arg2VarName.s0)
      $Arg0VarName[0] = $Arg1VarName[0];
    if ($Arg2VarName.s1)
      $Arg0VarName[1] = $Arg1VarName[1];
    if ($Arg2VarName.s2)
      $Arg0VarName[2] = $Arg1VarName[2];
    if ($Arg2VarName.s3)
      $Arg0VarName[3] = $Arg1VarName[3];
  }];
code masked_load4x8 = [{
    if ($Arg2VarName.s0)
      $Arg1VarName[0] = $Arg0VarName[0];
    else $Arg1VarName[0] = 0;
    if ($Arg2VarName.s1)
      $Arg1VarName[1] = $Arg0VarName[1];
    else $Arg1VarName[1] = 0;
    if ($Arg2VarName.s2)
      $Arg1VarName[2] = $Arg0VarName[2];
    else $Arg1VarName[2] = 0;
    if ($Arg2VarName.s3)
      $Arg1VarName[3] = $Arg0VarName[3];
    else $Arg1VarName[3] = 0;
    if ($Arg2VarName.s4)
      $Arg1VarName[4] = $Arg0VarName[4];
    else $Arg1VarName[4] = 0;
    if ($Arg2VarName.s5)
      $Arg1VarName[5] = $Arg0VarName[5];
    else $Arg1VarName[5] = 0;
    if ($Arg2VarName.s6)
      $Arg1VarName[6] = $Arg0VarName[6];
    else $Arg1VarName[6] = 0;
    if ($Arg2VarName.s7)
      $Arg1VarName[7] = $Arg0VarName[7];
    else $Arg1VarName[7] = 0;
  }];
code masked_store4x8 = [{
    if ($Arg2VarName.s0)
      $Arg0VarName[0] = $Arg1VarName[0];
    if ($Arg2VarName.s1)
      $Arg0VarName[1] = $Arg1VarName[1];
    if ($Arg2VarName.s2)
      $Arg0VarName[2] = $Arg1VarName[2];
    if ($Arg2VarName.s3)
      $Arg0VarName[3] = $Arg1VarName[3];
    if ($Arg2VarName.s4)
      $Arg0VarName[4] = $Arg1VarName[4];
    if ($Arg2VarName.s5)
      $Arg0VarName[5] = $Arg1VarName[5];
    if ($Arg2VarName.s6)
      $Arg0VarName[6] = $Arg1VarName[6];
    if ($Arg2VarName.s7)
      $Arg0VarName[7] = $Arg1VarName[7];
  }];
code masked_load4 = [{
    $ReturnType ret;
    $Arg0BaseType* tmp$Arg0VarName = ($Arg0BaseType*)$Arg0VarName;
    if ($Arg1VarName.s0)
      ret.s0 = tmp$Arg0VarName[0];
    else ret.s0 = 0;
    if ($Arg1VarName.s1)
      ret.s1 = tmp$Arg0VarName[1];
    else ret.s1 = 0;
    if ($Arg1VarName.s2)
      ret.s2 = tmp$Arg0VarName[2];
    else ret.s2 = 0;
    if ($Arg1VarName.s3)
      ret.s3 = tmp$Arg0VarName[3];
    else ret.s3 = 0;
    return ret;
  }];
code masked_store4 = [{
    $Arg0BaseType* tmp$Arg0VarName = ($Arg0BaseType*)$Arg0VarName;
    if ($Arg2VarName.s0)
      tmp$Arg0VarName[0] = $Arg1VarName.s0;
    if ($Arg2VarName.s1)
      tmp$Arg0VarName[1] = $Arg1VarName.s1;
    if ($Arg2VarName.s2)
      tmp$Arg0VarName[2] = $Arg1VarName.s2;
    if ($Arg2VarName.s3)
      tmp$Arg0VarName[3] = $Arg1VarName.s3;
  }];
code masked_load8 = [{
    $ReturnType ret;
    $Arg0BaseType* tmp$Arg0VarName = ($Arg0BaseType*)$Arg0VarName;
    if ($Arg1VarName.s0)
      ret.s0 = tmp$Arg0VarName[0];
    else ret.s0 = 0;
    if ($Arg1VarName.s1)
      ret.s1 = tmp$Arg0VarName[1];
    else ret.s1 = 0;
    if ($Arg1VarName.s2)
      ret.s2 = tmp$Arg0VarName[2];
    else ret.s2 = 0;
    if ($Arg1VarName.s3)
      ret.s3 = tmp$Arg0VarName[3];
    else ret.s3 = 0;
    if ($Arg1VarName.s4)
      ret.s4 = tmp$Arg0VarName[4];
    else ret.s4 = 0;
    if ($Arg1VarName.s5)
      ret.s5 = tmp$Arg0VarName[5];
    else ret.s5 = 0;
    if ($Arg1VarName.s6)
      ret.s6 = tmp$Arg0VarName[6];
    else ret.s6 = 0;
    if ($Arg1VarName.s7)
      ret.s7 = tmp$Arg0VarName[7];
    else ret.s7 = 0;
    return ret;
  }];
code masked_store8 = [{
    $Arg0BaseType* tmp$Arg0VarName = ($Arg0BaseType*)$Arg0VarName;
    if ($Arg2VarName.s0)
      tmp$Arg0VarName[0] = $Arg1VarName.s0;
    if ($Arg2VarName.s1)
      tmp$Arg0VarName[1] = $Arg1VarName.s1;
    if ($Arg2VarName.s2)
      tmp$Arg0VarName[2] = $Arg1VarName.s2;
    if ($Arg2VarName.s3)
      tmp$Arg0VarName[3] = $Arg1VarName.s3;
    if ($Arg2VarName.s4)
      tmp$Arg0VarName[4] = $Arg1VarName.s4;
    if ($Arg2VarName.s5)
      tmp$Arg0VarName[5] = $Arg1VarName.s5;
    if ($Arg2VarName.s6)
      tmp$Arg0VarName[6] = $Arg1VarName.s6;
    if ($Arg2VarName.s7)
      tmp$Arg0VarName[7] = $Arg1VarName.s7;
  }];
// ****************************************************************************
//                                 4x4
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_4x4_sse3 = OclBuiltinImpl<__ocl_masked_load4x4, [v4i8, v4i16, v4i32, v4f32],  0, masked_load4x4>;

OclBuiltinImpl __ocl_masked_store_4x4_sse3 = OclBuiltinImpl<__ocl_masked_store4x4, [v4i8, v4i16, v4i32, v4f32],  0, masked_store4x4>;
// ****************************************************************************
//                                 4x8
// ****************************************************************************
OclBuiltinImpl __ocl_masked_load_4x8_sse3 = OclBuiltinImpl<__ocl_masked_load4x8, [v4i8, v4i16, v4i32, v4f32],  0, masked_load4x8>;

OclBuiltinImpl __ocl_masked_store_4x8_sse3 = OclBuiltinImpl<__ocl_masked_store4x8, [v4i8, v4i16, v4i32, v4f32],  0, masked_store4x8>;
// ****************************************************************************
//                                 4
// ****************************************************************************

OclBuiltinImpl masked_load4_sse3 = OclBuiltinImpl<__ocl_masked_load4, [v4i32, v4i64, v4f32, v4f64],  0, masked_load4>;

OclBuiltinImpl masked_store4_sse3 = OclBuiltinImpl<__ocl_masked_store4, [v4i32, v4i64, v4f32, v4f64],  0, masked_store4>;

// ****************************************************************************
//                                 8
// ****************************************************************************

OclBuiltinImpl masked_load8_sse3 = OclBuiltinImpl<__ocl_masked_load8, [v8i32, v8i64, v8f32, v8f64],  0, masked_load8>;

OclBuiltinImpl masked_store8_sse3 = OclBuiltinImpl<__ocl_masked_store8, [v8i32, v8i64, v8f32, v8f64],  0, masked_store8>;

