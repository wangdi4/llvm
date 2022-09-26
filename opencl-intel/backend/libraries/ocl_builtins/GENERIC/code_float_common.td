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

defvar degrees_f32_code = [{
    return ($Arg0Type)generic_inv_pi_180f * $Arg0VarName;
  }];
defvar degrees_f64_code = [{
    return ($Arg0Type)generic_inv_pi_180 * $Arg0VarName;
  }];

defvar radians_f32_code = [{
    return ($Arg0Type)generic_pi_180f * $Arg0VarName;
  }];
defvar radians_f64_code = [{
    return ($Arg0Type)generic_pi_180 * $Arg0VarName;
  }];

defvar mix_code = [{
    return $Arg0VarName + ($Arg1VarName - $Arg0VarName) * $Arg2VarName;
  }];

defvar mix_s_code = [{
    return $Func($Arg0VarName, $Arg1VarName, ($Arg2VecType)$Arg2VarName);
  }];

defvar step_s_code = [{
    return $Func(($Arg0VecType)$Arg0VarName, $Arg1VarName);
  }];

defvar smoothstep_f32_code = [{
    $ReturnType t = clamp(($Arg2VarName - $Arg0VarName) / ($Arg1VarName - $Arg0VarName), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
  }];
defvar smoothstep_f64_code = [{
    $ReturnType t = clamp(($Arg2VarName - $Arg0VarName) / ($Arg1VarName - $Arg0VarName), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
  }];

defvar smoothstep_s_code = [{
    return $Func(($Arg0VecType)$Arg0VarName, ($Arg1VecType)$Arg1VarName, $Arg2VarName);
  }];
