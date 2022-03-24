// vim:ts=2:sw=2:et:
// NOTE: common code used in in conversion_sat built-ins

defvar _int2int_rounding = [{
    return convert_$ReturnType($Arg0VarName);
  }];

defvar _int2int_rounding_sat = [{
    return convert_$ReturnType_sat($Arg0VarName);
  }];

defvar _2fp_sat = [{
    return convert_$ReturnType($Arg0VarName);
  }];

defvar _2fp_sat_rte = [{
    return convert_$ReturnType_rte($Arg0VarName);
  }];

defvar _2fp_sat_rtz = [{
    return convert_$ReturnType_rtz($Arg0VarName);
  }];

defvar _2fp_sat_rtp = [{
    return convert_$ReturnType_rtp($Arg0VarName);
  }];

defvar _2fp_sat_rtn = [{
    return convert_$ReturnType_rtn($Arg0VarName);
  }];

// destType = sourceType
defvar convertSameType = [{
    return $Arg0VarName;
  }];
// u2uUP, u2sUP
defvar unsigned_up_sat = [{
    return as_$ReturnType(__ocl_zext_$Arg0Symigentype_$ReturnSymigentype(as_$Arg0igentype($Arg0VarName)));
  }];
// unsigned-to-signed/singed-to-unsigned flat conversion
defvar u2s_flat_sat = [{
    return as_$ReturnType(min($Arg0VarName, ($Arg0Type)generic_max_$ReturnBaseType));
  }];

defvar s2u_flat_sat = [{
    return as_$ReturnType(max($Arg0VarName, ($Arg0Type)0));
  }];

// signed-to-unsinged UP conversion with saturation
defvar s2u_up_sat = [{
    return as_$ReturnType(__ocl_zext_$Arg0Symigentype_$ReturnSymigentype(
                          as_$Arg0igentype(
                            max($Arg0VarName, ($Arg0Type)0))));
  }];
// signed-to-signed UP conversion with saturation
defvar s2s_up_sat = [{
    return as_$ReturnType(__ocl_sext_$Arg0Symigentype_$ReturnSymigentype(as_$Arg0igentype($Arg0VarName)));
  }];

// signed-to-unsigned/signed-to-signed down conversion
defvar s2u_s2s_down_sat = [{
    return as_$ReturnType(__ocl_trunc_$Arg0Symigentype_$ReturnSymigentype(
                          as_$Arg0igentype(
                            clamp($Arg0VarName, ($Arg0Type)generic_min_$ReturnBaseType, ($Arg0Type)generic_max_$ReturnBaseType))));
  }];

// unsigned-signed down conversion
defvar u2s_u2u_down_sat = [{
    return as_$ReturnType(__ocl_trunc_$Arg0Symigentype_$ReturnSymigentype(
                          as_$Arg0igentype(
                            min($Arg0VarName, ($Arg0Type)generic_max_$ReturnBaseType))));
  }];

// f16-to-X conversion
defvar fp16_sat = [{
    return convert_$ReturnType_sat(__ocl_fpext_$Arg0Symgentype_$Arg0Symf32gentype($Arg0VarName));
  }];
defvar fp16_sat_rte = [{
    return convert_$ReturnType_sat_rte(__ocl_fpext_$Arg0Symgentype_$Arg0Symf32gentype($Arg0VarName));
  }];
defvar fp16_sat_rtz = [{
    return convert_$ReturnType_sat_rtz(__ocl_fpext_$Arg0Symgentype_$Arg0Symf32gentype($Arg0VarName));
  }];
defvar fp16_sat_rtp = [{
    return convert_$ReturnType_sat_rtp(__ocl_fpext_$Arg0Symgentype_$Arg0Symf32gentype($Arg0VarName));
  }];
defvar fp16_sat_rtn = [{
    return convert_$ReturnType_sat_rtn(__ocl_fpext_$Arg0Symgentype_$Arg0Symf32gentype($Arg0VarName));
  }];

// f32-to-X conversion
defvar fp2signed_flat_sat = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptoi32$rtzsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];
defvar fp2signed_flat_sat_rte = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptoi32$rtnsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];
defvar fp2signed_flat_sat_rtz = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptoi32$rtzsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];
defvar fp2signed_flat_sat_rtp = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptoi32$upsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];
defvar fp2signed_flat_sat_rtn = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptoi32$downsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];

defvar fp2unsigned_flat_sat = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptou32$rtzsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];
defvar fp2unsigned_flat_sat_rte = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptou32$rtnsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];
defvar fp2unsigned_flat_sat_rtz = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptou32$rtzsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];
defvar fp2unsigned_flat_sat_rtp = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptou32$upsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];
defvar fp2unsigned_flat_sat_rtn = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptou32$downsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];

defvar fp2signed_down_sat = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symigentype_$ReturnSymigentype(
              clamp(as_$Arg0igentype( __ocl_svml_$Target_cvtfptoi32$rtzsat$SVMLSuffix($Arg0VarName_with_NaN)),
                    ($Arg0igentype)generic_min_$ReturnBaseType, ($Arg0igentype)generic_max_$ReturnBaseType)));
  }];
defvar fp2signed_down_sat_rte = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symigentype_$ReturnSymigentype(
              clamp(as_$Arg0igentype( __ocl_svml_$Target_cvtfptoi32$rtnsat$SVMLSuffix($Arg0VarName_with_NaN)),
                    ($Arg0igentype)generic_min_$ReturnBaseType, ($Arg0igentype)generic_max_$ReturnBaseType)));
  }];
defvar fp2signed_down_sat_rtz = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symigentype_$ReturnSymigentype(
              clamp(as_$Arg0igentype( __ocl_svml_$Target_cvtfptoi32$rtzsat$SVMLSuffix($Arg0VarName_with_NaN)),
                    ($Arg0igentype)generic_min_$ReturnBaseType, ($Arg0igentype)generic_max_$ReturnBaseType)));
  }];
defvar fp2signed_down_sat_rtp = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symigentype_$ReturnSymigentype(
              clamp(as_$Arg0igentype( __ocl_svml_$Target_cvtfptoi32$upsat$SVMLSuffix($Arg0VarName_with_NaN)),
                    ($Arg0igentype)generic_min_$ReturnBaseType, ($Arg0igentype)generic_max_$ReturnBaseType)));
  }];
defvar fp2signed_down_sat_rtn = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symigentype_$ReturnSymigentype(
              clamp(as_$Arg0igentype( __ocl_svml_$Target_cvtfptoi32$downsat$SVMLSuffix($Arg0VarName_with_NaN)),
                    ($Arg0igentype)generic_min_$ReturnBaseType, ($Arg0igentype)generic_max_$ReturnBaseType)));
  }];

defvar fp2unsigned_down_sat = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symigentype_$ReturnSymigentype(
              as_$Arg0igentype(
                clamp(as_$Arg0ugentype( __ocl_svml_$Target_cvtfptou32$rtzsat$SVMLSuffix($Arg0VarName_with_NaN)),
                  ($Arg0ugentype)generic_min_$ReturnBaseType, ($Arg0ugentype)generic_max_$ReturnBaseType))));
  }];
defvar fp2unsigned_down_sat_rte = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symigentype_$ReturnSymigentype(
              as_$Arg0igentype(
                clamp(as_$Arg0ugentype( __ocl_svml_$Target_cvtfptou32$rtnsat$SVMLSuffix($Arg0VarName_with_NaN)),
                    ($Arg0ugentype)generic_min_$ReturnBaseType, ($Arg0ugentype)generic_max_$ReturnBaseType))));
  }];
defvar fp2unsigned_down_sat_rtz = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symigentype_$ReturnSymigentype(
              as_$Arg0igentype(
                clamp(as_$Arg0ugentype( __ocl_svml_$Target_cvtfptou32$rtzsat$SVMLSuffix($Arg0VarName_with_NaN)),
                    ($Arg0ugentype)generic_min_$ReturnBaseType, ($Arg0ugentype)generic_max_$ReturnBaseType))));
  }];
defvar fp2unsigned_down_sat_rtp = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symigentype_$ReturnSymigentype(
              as_$Arg0igentype(
                clamp(as_$Arg0ugentype( __ocl_svml_$Target_cvtfptou32$upsat$SVMLSuffix($Arg0VarName_with_NaN)),
                    ($Arg0ugentype)generic_min_$ReturnBaseType, ($Arg0ugentype)generic_max_$ReturnBaseType))));
  }];
defvar fp2unsigned_down_sat_rtn = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symigentype_$ReturnSymigentype(
              as_$Arg0igentype(
                clamp(as_$Arg0ugentype( __ocl_svml_$Target_cvtfptou32$downsat$SVMLSuffix($Arg0VarName_with_NaN)),
                     ($Arg0ugentype)generic_min_$ReturnBaseType, ($Arg0ugentype)generic_max_$ReturnBaseType))));
  }];

defvar fp2unsigned_up_sat = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptou64$rtzsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];
defvar fp2unsigned_up_sat_rte = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptou64$rtnsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];
defvar fp2unsigned_up_sat_rtz = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptou64$rtzsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];
defvar fp2unsigned_up_sat_rtp = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptou64$upsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];
defvar fp2unsigned_up_sat_rtn = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptou64$downsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];

defvar fp2signed_up_sat = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptoi64$rtzsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];
defvar fp2signed_up_sat_rte = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptoi64$rtnsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];
defvar fp2signed_up_sat_rtz = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptoi64$rtzsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];
defvar fp2signed_up_sat_rtp = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptoi64$upsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];
defvar fp2signed_up_sat_rtn = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptoi64$downsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];

// f64-to-X conversion
defvar dp2signed_flat_sat = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptoi64$rtzsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];
defvar dp2signed_flat_sat_rte = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptoi64$rtnsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];
defvar dp2signed_flat_sat_rtz = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptoi64$rtzsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];
defvar dp2signed_flat_sat_rtp = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptoi64$upsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];
defvar dp2signed_flat_sat_rtn = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptoi64$downsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];

defvar dp2unsigned_flat_sat = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptou64$rtzsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];
defvar dp2unsigned_flat_sat_rte = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptou64$rtnsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];
defvar dp2unsigned_flat_sat_rtz = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptou64$rtzsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];
defvar dp2unsigned_flat_sat_rtp = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptou64$upsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];
defvar dp2unsigned_flat_sat_rtn = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return __ocl_svml_$Target_cvtfptou64$downsat$SVMLSuffix($Arg0VarName_with_NaN);
  }];

defvar dp2signed_down_sat = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symigentype_$ReturnSymigentype(
              clamp(as_$Arg0igentype( __ocl_svml_$Target_cvtfptoi64$rtzsat$SVMLSuffix($Arg0VarName_with_NaN)),
                    ($Arg0igentype)generic_min_$ReturnBaseType, ($Arg0igentype)generic_max_$ReturnBaseType)));
  }];
defvar dp2signed_down_sat_rte = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symigentype_$ReturnSymigentype(
              clamp(as_$Arg0igentype( __ocl_svml_$Target_cvtfptoi64$rtnsat$SVMLSuffix($Arg0VarName_with_NaN)),
                    ($Arg0igentype)generic_min_$ReturnBaseType, ($Arg0igentype)generic_max_$ReturnBaseType)));
  }];
defvar dp2signed_down_sat_rtz = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symigentype_$ReturnSymigentype(
              clamp(as_$Arg0igentype( __ocl_svml_$Target_cvtfptoi64$rtzsat$SVMLSuffix($Arg0VarName_with_NaN)),
                    ($Arg0igentype)generic_min_$ReturnBaseType, ($Arg0igentype)generic_max_$ReturnBaseType)));
  }];
defvar dp2signed_down_sat_rtp = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symigentype_$ReturnSymigentype(
              clamp(as_$Arg0igentype( __ocl_svml_$Target_cvtfptoi64$upsat$SVMLSuffix($Arg0VarName_with_NaN)),
                    ($Arg0igentype)generic_min_$ReturnBaseType, ($Arg0igentype)generic_max_$ReturnBaseType)));
  }];
defvar dp2signed_down_sat_rtn = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symigentype_$ReturnSymigentype(
              clamp(as_$Arg0igentype( __ocl_svml_$Target_cvtfptoi64$downsat$SVMLSuffix($Arg0VarName_with_NaN)),
                    ($Arg0igentype)generic_min_$ReturnBaseType, ($Arg0igentype)generic_max_$ReturnBaseType)));
  }];

defvar dp2unsigned_down_sat = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symigentype_$ReturnSymigentype(
              as_$Arg0igentype(
                clamp(as_$Arg0ugentype( __ocl_svml_$Target_cvtfptou64$rtzsat$SVMLSuffix($Arg0VarName_with_NaN)),
                      ($Arg0ugentype)generic_min_$ReturnBaseType, ($Arg0ugentype)generic_max_$ReturnBaseType))));
  }];
defvar dp2unsigned_down_sat_rte = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symigentype_$ReturnSymigentype(
              as_$Arg0igentype(
                clamp(as_$Arg0ugentype( __ocl_svml_$Target_cvtfptou64$rtnsat$SVMLSuffix($Arg0VarName_with_NaN)),
                      ($Arg0ugentype)generic_min_$ReturnBaseType, ($Arg0ugentype)generic_max_$ReturnBaseType))));
  }];
defvar dp2unsigned_down_sat_rtz = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symigentype_$ReturnSymigentype(
              as_$Arg0igentype(
                clamp(as_$Arg0ugentype( __ocl_svml_$Target_cvtfptou64$rtzsat$SVMLSuffix($Arg0VarName_with_NaN)),
                      ($Arg0ugentype)generic_min_$ReturnBaseType, ($Arg0ugentype)generic_max_$ReturnBaseType))));
  }];
defvar dp2unsigned_down_sat_rtp = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symigentype_$ReturnSymigentype(
              as_$Arg0igentype(
                clamp(as_$Arg0ugentype( __ocl_svml_$Target_cvtfptou64$upsat$SVMLSuffix($Arg0VarName_with_NaN)),
                      ($Arg0ugentype)generic_min_$ReturnBaseType, ($Arg0ugentype)generic_max_$ReturnBaseType))));
  }];
defvar dp2unsigned_down_sat_rtn = [{
    const $Arg0igentype mask = isnan($Arg0VarName);
    const $Arg0Type zeros = ($Arg0Type)0.0f;
    $Arg0Type $Arg0VarName_with_NaN = select($Arg0VarName, zeros, mask);
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symigentype_$ReturnSymigentype(
              as_$Arg0igentype(
                clamp(as_$Arg0ugentype( __ocl_svml_$Target_cvtfptou64$downsat$SVMLSuffix($Arg0VarName_with_NaN)),
                      ($Arg0ugentype)generic_min_$ReturnBaseType, ($Arg0ugentype)generic_max_$ReturnBaseType))));
  }];

//def convert_u8_sat_rte  : OclBuiltin<"convert_$ReturnType_sat_rte", (outs  u8gentype:$ret), (ins gentype:$x), [], []>;
//def convert_u8_sat_rtz  : OclBuiltin<"convert_$ReturnType_sat_rtz", (outs  u8gentype:$ret), (ins gentype:$x), [], []>;
//def convert_u8_sat_rtp  : OclBuiltin<"convert_$ReturnType_sat_rtp", (outs  u8gentype:$ret), (ins gentype:$x), [], []>;
//def convert_u8_sat_rtn  : OclBuiltin<"convert_$ReturnType_sat_rtn", (outs  u8gentype:$ret), (ins gentype:$x), [], []>;
