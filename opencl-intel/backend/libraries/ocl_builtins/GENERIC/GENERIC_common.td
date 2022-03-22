// vim:ts=2:sw=2:et:
// NOTE
// Purporse of the file GENERIC_common.td is to share code between CPU and MIC
// during CPU built ins transition to tblgen

  list<OclType> common_types_v1_to_v16 = ExpandTypesByVFAndFlatten<
      [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64],
      [1, 2, 3, 4, 8, 16]>.Tout;

  // clamp_s. for float and double
  // NOTE: integer and common BIs has clamp, min, max BIs.
  OclBuiltinImpl clamp_s_generic = OclBuiltinImpl<clamp_s, common_types_v1_to_v16, 0, clamp_s_impl>;

  // max_s
  OclBuiltinImpl max_s_generic = OclBuiltinImpl<max_s, common_types_v1_to_v16, 0, max_s_impl>;

  // min_s
  OclBuiltinImpl min_s_generic = OclBuiltinImpl<min_s, common_types_v1_to_v16, 0, min_s_impl>;

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


include "GENERIC/conversion_not_shared.td"
include "GENERIC/conversion_sat_not_shared.td"
include "GENERIC/integer_not_shared.td"
include "GENERIC/math_not_shared.td"
include "GENERIC/misc.td"
include "GENERIC/common_not_shared.td"
include "GENERIC/relational_not_shared.td"
include "GENERIC/soa_relational.td"
include "GENERIC/group_builtin.td"
include "GENERIC/sub_group_builtin_fake_impl.td"
