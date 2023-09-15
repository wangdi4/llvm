// Copyright (C) 2014 Intel Corporation
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

defvar clamp_s_code = [{
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

// masked

// ugentype = f(gentype, mask)
defvar masked_impl_code_UG_G_M = [{
    $Arg0VecType m$Arg0VarName = select(($Arg0VecType)($DEF0), $Arg0VarName, $Arg1VarName);
    return $Func(m$Arg0VarName);
  }];

// ugentype = f(gentype, gentype, mask)
defvar masked_impl_code_UG_G2_M = [{
    $Arg0VecType m$Arg0VarName = select(($Arg0VecType)($DEF0), $Arg0VarName, $Arg2VarName);
    $Arg1VecType m$Arg1VarName = select(($Arg1VecType)($DEF0), $Arg1VarName, $Arg2VarName);
    return $Func(m$Arg0VarName, m$Arg1VarName);
  }];

/// Common functions
defvar clamp_f_code = [{
    return fmax(fmin($Arg0VarName, $Arg2VarName), $Arg1VarName);
  }];
