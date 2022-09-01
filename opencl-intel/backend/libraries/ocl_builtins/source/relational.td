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

defvar relational_scalar_types = [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f16, v1f32, v1f64];
defvar relational_all_types = ExpandTypesByVFAndFlatten<relational_scalar_types, [1, 2, 3, 4, 8, 16, 32, 64]>.Tout;
defvar relational_vector_types = ExpandTypesByVFAndFlatten<relational_scalar_types, [2, 3, 4, 8, 16, 32, 64]>.Tout;
defvar relational_widen_types = ExpandTypesByVFAndFlatten<relational_scalar_types, [4, 8, 16, 32, 64]>.Tout;
defvar relational_declare_only_types = ExpandTypeByVF<v1f16, [32, 64]>.Tout;

// --- Relational functions ---
// https://registry.khronos.org/OpenCL/specs/3.0-unified/html/OpenCL_C.html#relational-functions
//
// Prototypes
let Attrs = [CONSTFUNC, OVERLOADABLE], NeedForwardDecl = true in {
let Types = relational_all_types in {
  // - bitselect -
  // Each bit of the result is the corresponding bit of a if the corresponding
  // bit of c is 0. Otherwise it is the corresponding bit of b.
  def bitselect: OclBuiltin<"bitselect", (outs gentype:$ret), (ins gentype:$a, gentype:$b, gentype:$c)>;

  // - select -
  // For each component of a vector type,
  // result[i] = if MSB of c[i] is set ? b[i] : a[i].
  // For a scalar type, result = c ? b : a.
  // igentype and ugentype must have the same number of elements and bits as gentype.
  def iselect: OclBuiltin<"select", (outs gentype:$ret), (ins gentype:$a, gentype:$b, igentype:$c)>;
  def uselect: OclBuiltin<"select", (outs gentype:$ret), (ins gentype:$a, gentype:$b, ugentype:$c)>;
}
let Types = relational_widen_types in {
  def bitselect_mask: OclBuiltin<"bitselect", (outs gentype:$ret), (ins gentype:$a, gentype:$b, gentype:$c, gentype:$mask)>;

  // Scalar select() checks zero-ness of `c` while vector select() defined by
  // the SPEC checks MSB of `c` -- so we need dedicated widened versions for
  // scalar select().
  def iselect_v1widen: OclBuiltin<"select_v1widen", (outs gentype:$ret), (ins gentype:$a, gentype:$b, igentype:$c)>;
  def uselect_v1widen: OclBuiltin<"select_v1widen", (outs gentype:$ret), (ins gentype:$a, gentype:$b, ugentype:$c)>;
  def iselect_v1widen_mask: OclBuiltin<"select_v1widen", (outs gentype:$ret), (ins gentype:$a, gentype:$b, igentype:$c, gentype:$mask)>;
  def uselect_v1widen_mask: OclBuiltin<"select_v1widen", (outs gentype:$ret), (ins gentype:$a, gentype:$b, ugentype:$c, gentype:$mask)>;
}
// Emit forward declarations so that they can be invoked by other builtins
let Types = relational_declare_only_types in {
  def : OclBuiltin<"isnan", (outs rf2igentype:$ret), (ins gentype:$x)>;
}
}

// Implementations
foreach bi = [bitselect, bitselect_mask] in {
  def : OclBuiltinImpl<bi, relational_all_types, /*isDeclOnly*/false, [{
    // Bitcast all args to int types
    $Arg0igentype _a = __builtin_astype($Arg0VarName, $Arg0igentype);
    $Arg0igentype _b = __builtin_astype($Arg1VarName, $Arg0igentype);
    $Arg0igentype _c = __builtin_astype($Arg2VarName, $Arg0igentype);
    // Take bit of a if corresponding bit of c is 0
    _a &= ~_c;
    // Take bit of b if corresponding bit of c is 1
    _b &= _c;
    _a |= _b;
    return __builtin_astype(_a, $ReturnType);
  }]>, SHARED;
}
foreach bi = [iselect, uselect] in {
  def : OclBuiltinImpl<bi, relational_vector_types, /*isDeclOnly*/false, [{
    $Arg2igentype _c = __builtin_astype($Arg2VarName, $Arg2igentype);
    // Check MSB
    _c = _c < ($Arg2igentype)0;
    return bitselect($Arg0VarName, $Arg1VarName, __builtin_astype(_c, $Arg0Type));
  }]>, SHARED;
  def : OclBuiltinImpl<bi, relational_scalar_types, /*isDeclOnly*/false, [{
    return $Arg2VarName ? $Arg1VarName : $Arg0VarName;
  }]>, SHARED;
}
foreach bi = [iselect_v1widen, iselect_v1widen_mask, uselect_v1widen, uselect_v1widen_mask] in {
  def : OclBuiltinImpl<bi, bi.Types, /*isDeclOnly*/false, [{
    $Arg2igentype _c = __builtin_astype($Arg2VarName, $Arg2igentype);
    // Check zero-ness, each element of the result is either -1 (all bits set) or 0.
    _c = _c != ($Arg2igentype)0;
    return bitselect($Arg0VarName, $Arg1VarName, __builtin_astype(_c, $Arg0Type));
  }]>, SHARED;
}

// VectInfos
let Types = relational_scalar_types in {
  def : VectInfo<[], bitselect, bitselect, bitselect, bitselect, bitselect, bitselect>;
  let HandleAlias = false in
  def : VectInfo<[], bitselect, bitselect_mask, bitselect_mask, bitselect_mask, bitselect_mask, bitselect_mask>;

  foreach bi = [iselect, uselect] in {
    defvar v1widen = !cast<OclBuiltin>(bi # "_v1widen");
    defvar v1widen_mask = !cast<OclBuiltin>(bi # "_v1widen_mask");
    def : VectInfo<[], bi, v1widen, v1widen, v1widen, v1widen, v1widen>;
    let HandleAlias = false in
    def : VectInfo<[], bi, v1widen_mask, v1widen_mask, v1widen_mask, v1widen_mask, v1widen_mask>;
  }
}
