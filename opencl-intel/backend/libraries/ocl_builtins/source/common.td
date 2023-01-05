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

defvar common_f16_types = ExpandTypeByVF<v1f16, [1, 2, 3, 4, 8, 16, 32, 64]>.Tout;
defvar common_scalar_types = [v1i8, v1u8, v1i16, v1u16, v1f16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64];
defvar common_types = ExpandTypesByVFAndFlatten<common_scalar_types, [1, 2, 3, 4, 8, 16, 32, 64]>.Tout;

// --- Common Functions ---
// https://registry.khronos.org/OpenCL/specs/3.0-unified/html/OpenCL_C.html#common-functions
//
// Prototypes
let Types = common_f16_types,
    Attrs = [CONSTFUNC, OVERLOADABLE],
    NeedForwardDecl = true in {
// Builtins that use vector version as vector variants.
defset list<OclBuiltin> common_builtins_with_general_vect_info = {
  def clamp_f16 : OclBuiltin<"clamp", (outs gentype:$ret), (ins gentype:$x, gentype:$minval, gentype:$maxval)>;
  def degrees_f16 : OclBuiltin<"degrees", (outs gentype:$ret), (ins gentype:$radians)>;
  def mix_f16 : OclBuiltin<"mix", (outs gentype:$ret), (ins gentype:$x, gentype:$y, gentype:$a)>;
  def radians_f16 : OclBuiltin<"radians", (outs gentype:$ret), (ins gentype:$degrees)>;
  def step_f16 : OclBuiltin<"step", (outs gentype:$ret), (ins gentype:$edge, gentype:$x)>;
  def smoothstep_f16 : OclBuiltin<"smoothstep", (outs gentype:$ret), (ins gentype:$edge0, gentype:$edge1, gentype:$x)>;
  def sign_f16 : OclBuiltin<"sign", (outs gentype:$ret), (ins gentype:$x)>;
} // defset common_builtins_with_general_vect_info

  def clamp_s_f16 : OclBuiltin<"clamp", (outs gentype:$ret), (ins gentype:$x, sgentype:$minval, sgentype:$maxval)>, Except<[v1f16]>;
  def mix_s_f16 : OclBuiltin<"mix", (outs gentype:$ret), (ins gentype:$x, gentype:$y, sgentype:$a)>, Except<[v1f16]>;
  def step_s_f16 : OclBuiltin<"step", (outs gentype:$ret), (ins sgentype:$edge, gentype:$x)>, Except<[v1f16]>;
  def smoothstep_s_f16 : OclBuiltin<"smoothstep", (outs gentype:$ret), (ins sgentype:$edge0, sgentype:$edge1, gentype:$x)>, Except<[v1f16]>;
}

let Types = common_types,
    Attrs = [CONSTFUNC, OVERLOADABLE],
    NeedForwardDecl = true in {
// Builtins that use vector version as vector variants.
defset list<OclBuiltin> common_all_type_builtins_with_general_vect_info = {
  def max_all_type : OclBuiltin<"max", (outs gentype:$ret), (ins gentype:$x, gentype:$y)>;
  def min_all_type : OclBuiltin<"min", (outs gentype:$ret), (ins gentype:$x, gentype:$y)>;
} // defset common_all_type_builtins_with_general_vect_info

  def max_s_all_type : OclBuiltin<"max", (outs gentype:$ret), (ins gentype:$x, sgentype:$y)>, Except<common_scalar_types>;
  def min_s_all_type : OclBuiltin<"min", (outs gentype:$ret), (ins gentype:$x, sgentype:$y)>, Except<common_scalar_types>;
}

// Implementations
foreach bi = [clamp_f16, clamp_s_f16] in {
  def : OclBuiltinImpl<bi, bi.Types, false, [{
    return min(max($Arg0VarName, $Arg1VarName), $Arg2VarName);
  }]>, SHARED;
}

def : OclBuiltinImpl<degrees_f16, degrees_f16.Types, false, [{
  return 180 * M_1_PI_H * $Arg0VarName;
}]>, SHARED;

// !!! order of arguments in comparison is important. DO NOT CHANGE
// Reason is fmax implement rely on max implement
// and will affect how fmax NaN input is treated.
// order should be "y < x ? y : x" sequence
// fmax treats NaN differently in case NaN is in 1st argument or in 2nd argument
foreach bi = [max_all_type, max_s_all_type] in {
  def : OclBuiltinImpl<bi, bi.Types, false, [{
    return select($Arg1VarName, $Arg0VarName, ($Arg0igentype)($Arg1VarName < $Arg0VarName));
  }]>, SHARED;
}

// !!! order of arguments in comparison is important. DO NOT CHANGE
// Reason is fmin implement rely on min implement
// and will affect how fmin NaN input is treated.
// order should be "x < y ? x : y" sequence
// fmin treats NaN differently in case NaN is in 1st argument or in 2nd argument
foreach bi = [min_all_type, min_s_all_type] in {
  def : OclBuiltinImpl<bi, bi.Types, false, [{
    return select($Arg1VarName, $Arg0VarName, ($Arg0igentype)($Arg0VarName < $Arg1VarName));
  }]>, SHARED;
}

foreach bi = [mix_f16, mix_s_f16] in {
  def : OclBuiltinImpl<bi, bi.Types, false, [{
    return $Arg0VarName + ($Arg1VarName - $Arg0VarName) * $Arg2VarName;
  }]>, SHARED;
}

def : OclBuiltinImpl<radians_f16, radians_f16.Types, false, [{
  return M_PI_H / 180 * $Arg0VarName;
}]>, SHARED;

foreach bi = [step_f16, step_s_f16] in {
  def : OclBuiltinImpl<bi, bi.Types, false, [{
    return select(($ReturnType)1.0, ($ReturnType)0.0, ($Arg1igentype)($Arg1VarName < $Arg0VarName));
  }]>, SHARED;
}

foreach bi = [smoothstep_f16, smoothstep_s_f16] in {
  def : OclBuiltinImpl<bi, bi.Types, false, [{
    $ReturnType _t = clamp(($Arg2VarName - $Arg0VarName) / ($Arg1VarName - $Arg0VarName), ($ReturnType)0.0, ($ReturnType)1.0);
    return _t * _t * (3 - 2 * _t);
  }]>, SHARED;
}

defvar sign_code = [{
  $ReturnType _res = copysign(($Arg0Type)1.0, $Arg0VarName);
  _res = select(_res, $Arg0VarName, ($Arg0igentype)(fabs($Arg0VarName) == ($Arg0Type)0.0));
  _res = select(_res, ($ReturnType)0.0, ($Arg0igentype)isnan($Arg0VarName));
  return _res;
}];
def : OclBuiltinImpl<sign_f16, sign_f16.Types, false, sign_code>, SHARED;

// VectInfos
foreach bi = common_builtins_with_general_vect_info # common_all_type_builtins_with_general_vect_info in {
  def : VectInfo<[v1f16], bi, bi, bi, bi, bi, bi>;
}
