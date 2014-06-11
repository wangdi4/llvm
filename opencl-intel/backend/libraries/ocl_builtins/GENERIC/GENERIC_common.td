// vim:ts=2:sw=2:et:
// NOTE
// Purporse of the file GENERIC_common.td is to share code between CPU and MIC
// during CPU built ins transition to tblgen

  include "GENERIC/code_expand_promote_common.td"

  // clamp_s. for float and double
  // NOTE: integer and common BIs has clamp, min, max BIs.
  OclBuiltinImpl clamp_s_generic = OclBuiltinImpl<clamp_s, clamp_s.Types, 0, [{
    $Arg1VecType vec$Arg1VarName = ($Arg1VecType)$Arg1VarName;
    $Arg2VecType vec$Arg2VarName = ($Arg2VecType)$Arg2VarName;
    return $Func($Arg0VarName, vec$Arg1VarName, vec$Arg2VarName);
  }]>;

  // max_s
  OclBuiltinImpl max_s_generic = OclBuiltinImpl<max_s, max_s.Types, 0, [{
    $Arg1VecType vec$Arg1VarName = ($Arg1VecType)$Arg1VarName;
    return $Func($Arg0VarName, vec$Arg1VarName);
  }]>;

  // min_s
  OclBuiltinImpl min_s_generic = OclBuiltinImpl<min_s, min_s.Types, 0, [{
    $Arg1VecType vec$Arg1VarName = ($Arg1VecType)$Arg1VarName;
    return $Func($Arg0VarName, vec$Arg1VarName);
  }]>;

  // mix_s
  OclBuiltinImpl mix_s_generic = OclBuiltinImpl<mix_s, mix_s.Types, 0, [{
    $Arg2VecType vec$Arg2VarName = ($Arg2VecType)$Arg2VarName;
    return $Func($Arg0VarName, $Arg1VarName, vec$Arg2VarName);
  }]>;

  // step_s
  OclBuiltinImpl step_s_generic = OclBuiltinImpl<step_s, step_s.Types, 0, [{
    $Arg0VecType vec$Arg0VarName = ($Arg0VecType)$Arg0VarName;
    return $Func(vec$Arg0VarName, $Arg1VarName);
  }]>;

  // smoothstep_s
  OclBuiltinImpl smoothstep_s_generic = OclBuiltinImpl<smoothstep_s, smoothstep_s.Types, 0, [{
    $Arg0VecType vec$Arg0VarName = ($Arg0VecType)$Arg0VarName;
    $Arg1VecType vec$Arg1VarName = ($Arg1VecType)$Arg1VarName;
    return $Func(vec$Arg0VarName, vec$Arg1VarName, $Arg2VarName);
  }]>;


include "GENERIC/common.td"
include "GENERIC/conversion.td"
include "GENERIC/conversion_sat.td"
include "GENERIC/geometric.td"
include "GENERIC/integer.td"
include "GENERIC/math_not_shared.td"
include "GENERIC/misc.td"
include "GENERIC/relational.td"
include "GENERIC/soa_relational.td"
include "GENERIC/group_builtin.td"
