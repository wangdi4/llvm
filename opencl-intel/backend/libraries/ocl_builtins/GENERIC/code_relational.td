// Copyright (C) 2022 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

defvar isfinite_f32_code = [{
    //calculate abs($Arg0VarName)
    //Remove sign bit of the float
    $Arg0VarName = __builtin_astype(__builtin_astype($Arg0VarName, $Arg0igentype) & ($Arg0igentype)abs_inf, $Arg0Type);
    return $Arg0VarName < __builtin_astype(($Arg0igentype)fnan_min, $Arg0Type);
  }];

defvar isfinite_f64_code = [{
    $Arg0VarName = __builtin_astype(__builtin_astype($Arg0VarName, $Arg0igentype) & ($Arg0igentype)dnan_min, $Arg0Type);
    $Arg0Type dRes = __builtin_astype(__builtin_astype($Arg0VarName, $Arg0igentype) != ($Arg0igentype)dnan_min, $Arg0Type);
    $ReturnType res = __builtin_astype(dRes, $ReturnType);
    return res;
  }];

defvar isfinite_v1widen_code = [{
    return ($ReturnType)0 - convert_$ReturnType(isfinite($Arg0VarName));
  }];

defvar isinf_f32_code = [{
    //calculate abs($Arg0VarName)
    //Remove sign bit of the float
    $Arg0VarName = __builtin_astype(__builtin_astype($Arg0VarName, $Arg0igentype) & ($Arg0igentype)abs_inf, $Arg0Type);
    return $Arg0VarName >= __builtin_astype(($Arg0igentype)fnan_min, $Arg0Type);
  }];

defvar isinf_f64_code = [{
    //calculate abs($Arg0VarName)
    //Remove sign bit of the float
    $Arg0VarName = __builtin_astype(__builtin_astype($Arg0VarName, $Arg0igentype) & ($Arg0igentype)dnan_max, $Arg0Type);
    $Arg0Type dRes = __builtin_astype(__builtin_astype($Arg0VarName, $Arg0igentype) == ($Arg0igentype)dnan_min, $Arg0Type);
    return __builtin_astype(dRes, $ReturnType);
  }];

defvar isinf_v1widen_code = [{
    return ($ReturnType)0 - convert_$ReturnType(isinf($Arg0VarName));
  }];

defvar isnormal_f32_code = [{
    $Arg0VarName = __builtin_astype(__builtin_astype($Arg0VarName, $Arg0igentype) & ($Arg0igentype)abs_inf, $Arg0Type);
    $Arg0igentype res = __builtin_astype($Arg0VarName < __builtin_astype(($Arg0igentype)fnan_min, $Arg0Type), $Arg0igentype); //$Arg0VarName<INFINITY
    $Arg0igentype res1 = __builtin_astype($Arg0VarName >= ($Arg0Type)fltm, $Arg0igentype); //$Arg0VarName >= FLT_MIN
    return __builtin_astype(res & res1, $ReturnType);
  }];

defvar isnormal_f64_code = [{
    $Arg0igentype exp = __builtin_astype($Arg0VarName, $Arg0igentype) & ($Arg0igentype)dexp_mask;
    $ReturnType NaNorINF = __builtin_astype(exp == ($Arg0igentype)dexp_mask, $ReturnType);
    $ReturnType denom = __builtin_astype(exp == ($Arg0igentype)0, $ReturnType);
    $ReturnType res = (NaNorINF | denom);
    return (res ^ ($Arg0igentype)FF);
  }];

defvar isnormal_v1widen_code = [{
    return ($ReturnType)0 - convert_$ReturnType(isnormal($Arg0VarName));
  }];

// masked
// Please refer to masked_math.td for naming convention.

defvar masked_impl_code_I32_G_M = [{
    $Arg0VecType m$Arg0VarName = select(($Arg0VecType)($DEF0), $Arg0VarName, $Arg1VarName);
    return $Func(m$Arg0VarName);
  }];

defvar masked_impl_code_I32_G2_M = [{
    $Arg0VecType m$Arg0VarName = select(($Arg0VecType)($DEF0), $Arg0VarName, $Arg2VarName);
    $Arg1VecType m$Arg1VarName = select(($Arg1VecType)($DEF1), $Arg1VarName, $Arg2VarName);
    return $Func(m$Arg0VarName, m$Arg1VarName);
  }];

defvar masked_impl_code_G_G3_M_select = [{
    $Arg0igentype cvt_$Arg3VarName = convert_$Arg0igentype($Arg3VarName);
    $Arg0VecType m$Arg0VarName = select(($Arg0VecType)($DEF0), $Arg0VarName, cvt_$Arg3VarName);
    $Arg1VecType m$Arg1VarName = select(($Arg1VecType)($DEF1), $Arg1VarName, cvt_$Arg3VarName);
    $Arg2VecType m$Arg2VarName = select(($Arg2VecType)($DEF2), $Arg2VarName, cvt_$Arg3VarName);
    return $Func(m$Arg0VarName, m$Arg1VarName, m$Arg2VarName);
  }];

defvar masked_impl_code_G_G2_IG_M = [{
    $Arg0igentype cvt_$Arg3VarName = convert_$Arg0igentype($Arg3VarName);
    $Arg0VecType m$Arg0VarName = select(($Arg0VecType)($DEF0), $Arg0VarName, cvt_$Arg3VarName);
    $Arg1VecType m$Arg1VarName = select(($Arg1VecType)($DEF1), $Arg1VarName, cvt_$Arg3VarName);
    $Arg2VecType m$Arg2VarName = select(($Arg2VecType)($DEF2), $Arg2VarName, cvt_$Arg3VarName);
    return $Func(m$Arg0VarName, m$Arg1VarName, m$Arg2VarName);
  }];
