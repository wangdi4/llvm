// Copyright (C) 2013 Intel Corporation
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

// Work-group functions and their vectorized versions
let NeedForwardDecl = 1, VectorAttrs = [KERNEL_CALL_ONCE] in {
  let Types = [ v1i32, v4i32, v8i32, v16i32, v32i32, v64i32 ],
      Attrs = [OVERLOADABLE] in {
    def work_group_all : OclBuiltin<"work_group_all", (outs gentype:$ret), (ins gentype:$predicate), [], [], 1, 0>;
    def work_group_any : OclBuiltin<"work_group_any", (outs gentype:$ret), (ins gentype:$predicate), [], [], 1, 0>;
  }

  let Types = [ v4i32,  v8i32,  v16i32, v32i32, v64i32 ],
      Attrs = [OVERLOADABLE] in {
    def work_group_all_mask : OclBuiltin<"work_group_all", (outs gentype:$ret), (ins gentype:$predicate, gentype:$mask), [], [], 1, 0>;
    def work_group_any_mask : OclBuiltin<"work_group_any", (outs gentype:$ret), (ins gentype:$predicate, gentype:$mask), [], [], 1, 0>;
  }

  let Types = ExpandTypesByVFAndFlatten<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f16, v1f32, v1f64], [1, 4, 8, 16, 32, 64]>.Tout,
      Attrs = [OVERLOADABLE] in {
    def work_group_broadcast1d        : OclBuiltin<"work_group_broadcast",     (outs gentype:$ret), (ins gentype:$src, size_t:$local_id), [], [], 1, 0>;
    def work_group_broadcast2d        : OclBuiltin<"work_group_broadcast",     (outs gentype:$ret), (ins gentype:$src, size_t:$local_id_x, size_t:$local_id_y), [], [], 1, 0>;
    def work_group_broadcast3d        : OclBuiltin<"work_group_broadcast",     (outs gentype:$ret), (ins gentype:$src, size_t:$local_id_x, size_t:$local_id_y, size_t:$local_id_z), [], [], 1, 0>;
    def work_group_reduce_add         : OclBuiltin<"work_group_reduce_add",    (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_reduce_min         : OclBuiltin<"work_group_reduce_min",    (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_reduce_max         : OclBuiltin<"work_group_reduce_max",    (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_reduce_mul         : OclBuiltin<"work_group_reduce_mul",    (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_scan_exclusive_add : OclBuiltin<"work_group_scan_exclusive_add", (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_scan_exclusive_min : OclBuiltin<"work_group_scan_exclusive_min", (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_scan_exclusive_max : OclBuiltin<"work_group_scan_exclusive_max", (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_scan_exclusive_mul : OclBuiltin<"work_group_scan_exclusive_mul", (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_scan_inclusive_add : OclBuiltin<"work_group_scan_inclusive_add", (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_scan_inclusive_min : OclBuiltin<"work_group_scan_inclusive_min", (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_scan_inclusive_max : OclBuiltin<"work_group_scan_inclusive_max", (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_scan_inclusive_mul : OclBuiltin<"work_group_scan_inclusive_mul", (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
  }

  let Types = ExpandTypesByVFAndFlatten<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64], [1, 4, 8, 16, 32, 64]>.Tout,
      Attrs = [OVERLOADABLE] in {
    def work_group_reduce_bitwise_and         : OclBuiltin<"work_group_reduce_bitwise_and",    (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_reduce_bitwise_or         : OclBuiltin<"work_group_reduce_bitwise_or",    (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_reduce_bitwise_xor         : OclBuiltin<"work_group_reduce_bitwise_xor",    (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_scan_exclusive_bitwise_and : OclBuiltin<"work_group_scan_exclusive_bitwise_and",    (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_scan_exclusive_bitwise_or  : OclBuiltin<"work_group_scan_exclusive_bitwise_or",    (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_scan_exclusive_bitwise_xor : OclBuiltin<"work_group_scan_exclusive_bitwise_xor",    (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_scan_inclusive_bitwise_and : OclBuiltin<"work_group_scan_inclusive_bitwise_and",    (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_scan_inclusive_bitwise_or  : OclBuiltin<"work_group_scan_inclusive_bitwise_or",    (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_scan_inclusive_bitwise_xor : OclBuiltin<"work_group_scan_inclusive_bitwise_xor",    (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
  }

  let Types = ExpandTypesByVFAndFlatten<[v1i32], [1, 4, 8, 16, 32, 64]>.Tout,
      Attrs = [OVERLOADABLE] in {
    def work_group_reduce_logical_and         : OclBuiltin<"work_group_reduce_logical_and",    (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_reduce_logical_or         : OclBuiltin<"work_group_reduce_logical_or",    (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_reduce_logical_xor         : OclBuiltin<"work_group_reduce_logical_xor",    (outs gentype:$ret), (ins gentype:$src), [], [], 1, 0>;
    def work_group_reduce_logical_and_mask : OclBuiltin<"work_group_reduce_logical_and",    (outs gentype:$ret), (ins gentype:$src, gentype:$mask), [], [], 1, 0>;
    def work_group_reduce_logical_or_mask : OclBuiltin<"work_group_reduce_logical_or", (outs gentype:$ret), (ins gentype:$src, gentype:$mask), [], [], 1, 0>;
    def work_group_reduce_logical_xor_mask : OclBuiltin<"work_group_reduce_logical_xor", (outs gentype:$ret), (ins gentype:$src, gentype:$mask), [], [], 1, 0>;
  }

  let Types = ExpandTypesByVFAndFlatten<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f16, v1f32, v1f64], [4, 8, 16, 32, 64]>.Tout,
      Attrs = [OVERLOADABLE] in {
    def work_group_broadcast1d_mask        : OclBuiltin<"__work_group_broadcast",     (outs gentype:$ret), (ins gentype:$src, size_t:$local_id, gentype:$mask), [], [], 1, 0>;
    def work_group_broadcast2d_mask        : OclBuiltin<"__work_group_broadcast",     (outs gentype:$ret), (ins gentype:$src, size_t:$local_id_x, size_t:$local_id_y, gentype:$mask), [], [], 1, 0>;
    def work_group_broadcast3d_mask        : OclBuiltin<"__work_group_broadcast",     (outs gentype:$ret), (ins gentype:$src, size_t:$local_id_x, size_t:$local_id_y, size_t:$local_id_z, gentype:$mask), [], [], 1, 0>;
    def work_group_broadcast1d_assume_uniform : OclBuiltin<"work_group_broadcast", (outs gentype:$ret), (ins gentype:$src, u64gentype:$local_id), [], [], 1, 0>;
    def work_group_broadcast2d_assume_uniform : OclBuiltin<"work_group_broadcast", (outs gentype:$ret), (ins gentype:$src, u64gentype:$local_id_x, u64gentype:$local_id_y), [], [], 1, 0>;
    def work_group_broadcast3d_assume_uniform : OclBuiltin<"work_group_broadcast", (outs gentype:$ret), (ins gentype:$src, u64gentype:$local_id_x, u64gentype:$local_id_y, gentype:$local_id_z), [], [], 1, 0>;
    // Force using u32 mask (different with local id types) to differentiate the masked variant.
    def work_group_broadcast1d_assume_uniform_mask : OclBuiltin<"__work_group_broadcast", (outs gentype:$ret), (ins gentype:$src, u64gentype:$local_id, gentype:$mask), [], [], 1, 0>;
    def work_group_broadcast2d_assume_uniform_mask : OclBuiltin<"__work_group_broadcast", (outs gentype:$ret), (ins gentype:$src, u64gentype:$local_id_x, u64gentype:$local_id_y, gentype:$mask), [], [], 1, 0>;
    def work_group_broadcast3d_assume_uniform_mask : OclBuiltin<"__work_group_broadcast", (outs gentype:$ret), (ins gentype:$src, u64gentype:$local_id_x, u64gentype:$local_id_y, u64gentype:$local_id_z, gentype:$mask), [], [], 1, 0>;
    def work_group_reduce_add_mask         : OclBuiltin<"work_group_reduce_add",    (outs gentype:$ret), (ins gentype:$src, gentype:$mask), [], [], 1, 0>;
    def work_group_reduce_min_mask         : OclBuiltin<"work_group_reduce_min",    (outs gentype:$ret), (ins gentype:$src, gentype:$mask), [], [], 1, 0>;
    def work_group_reduce_max_mask         : OclBuiltin<"work_group_reduce_max",    (outs gentype:$ret), (ins gentype:$src, gentype:$mask), [], [], 1, 0>;
    def work_group_reduce_mul_mask         : OclBuiltin<"work_group_reduce_mul",    (outs gentype:$ret), (ins gentype:$src, gentype:$mask), [], [], 1, 0>;
    def work_group_scan_exclusive_add_mask : OclBuiltin<"work_group_scan_exclusive_add", (outs gentype:$ret), (ins gentype:$src, gentype:$mask), [], [], 1, 0>;
    def work_group_scan_exclusive_min_mask : OclBuiltin<"work_group_scan_exclusive_min", (outs gentype:$ret), (ins gentype:$src, gentype:$mask), [], [], 1, 0>;
    def work_group_scan_exclusive_max_mask : OclBuiltin<"work_group_scan_exclusive_max", (outs gentype:$ret), (ins gentype:$src, gentype:$mask), [], [], 1, 0>;
    def work_group_scan_exclusive_mul_mask : OclBuiltin<"work_group_scan_exclusive_mul", (outs gentype:$ret), (ins gentype:$src, gentype:$mask), [], [], 1, 0>;
    def work_group_scan_inclusive_add_mask : OclBuiltin<"work_group_scan_inclusive_add", (outs gentype:$ret), (ins gentype:$src, gentype:$mask), [], [], 1, 0>;
    def work_group_scan_inclusive_min_mask : OclBuiltin<"work_group_scan_inclusive_min", (outs gentype:$ret), (ins gentype:$src, gentype:$mask), [], [], 1, 0>;
    def work_group_scan_inclusive_max_mask : OclBuiltin<"work_group_scan_inclusive_max", (outs gentype:$ret), (ins gentype:$src, gentype:$mask), [], [], 1, 0>;
    def work_group_scan_inclusive_mul_mask : OclBuiltin<"work_group_scan_inclusive_mul", (outs gentype:$ret), (ins gentype:$src, gentype:$mask), [], [], 1, 0>;
  }
  let Types = ExpandTypesByVFAndFlatten<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64], [1, 4, 8, 16, 32, 64]>.Tout,
      Attrs = [OVERLOADABLE] in {
    def work_group_reduce_bitwise_and_mask : OclBuiltin<"work_group_reduce_bitwise_and", (outs gentype:$ret), (ins gentype:$src, gentype:$mask), [], [], 1, 0>;
    def work_group_reduce_bitwise_or_mask : OclBuiltin<"work_group_reduce_bitwise_or", (outs gentype:$ret), (ins gentype:$src, gentype:$mask), [], [], 1, 0>;
    def work_group_reduce_bitwise_xor_mask : OclBuiltin<"work_group_reduce_bitwise_xor", (outs gentype:$ret), (ins gentype:$src, gentype:$mask), [], [], 1, 0>;
    def work_group_scan_exclusive_bitwise_and_mask : OclBuiltin<"work_group_scan_exclusive_bitwise_and",    (outs gentype:$ret), (ins gentype:$src, gentype:$mask), [], [], 1, 0>;
    def work_group_scan_exclusive_bitwise_or_mask  : OclBuiltin<"work_group_scan_exclusive_bitwise_or",    (outs gentype:$ret), (ins gentype:$src, gentype:$mask), [], [], 1, 0>;
    def work_group_scan_exclusive_bitwise_xor_mask : OclBuiltin<"work_group_scan_exclusive_bitwise_xor",    (outs gentype:$ret), (ins gentype:$src, gentype:$mask), [], [], 1, 0>;
    def work_group_scan_inclusive_bitwise_and_mask : OclBuiltin<"work_group_scan_inclusive_bitwise_and",    (outs gentype:$ret), (ins gentype:$src, gentype:$mask), [], [], 1, 0>;
    def work_group_scan_inclusive_bitwise_or_mask  : OclBuiltin<"work_group_scan_inclusive_bitwise_or",    (outs gentype:$ret), (ins gentype:$src, gentype:$mask), [], [], 1, 0>;
    def work_group_scan_inclusive_bitwise_xor_mask : OclBuiltin<"work_group_scan_inclusive_bitwise_xor",    (outs gentype:$ret), (ins gentype:$src, gentype:$mask), [], [], 1, 0>;
  }
}

//
// VectInfo for SYCLKernelVecClone
//
def work_group_all_vect_info : VectInfo<[v1i32], work_group_all, work_group_all, work_group_all, work_group_all, work_group_all, work_group_all>;
def work_group_all_mask_vect_info : VectInfo<[v1i32], work_group_all, work_group_all_mask, work_group_all_mask, work_group_all_mask, work_group_all_mask, work_group_all_mask>;
def work_group_any_vect_info : VectInfo<[v1i32], work_group_any, work_group_any, work_group_any, work_group_any, work_group_any, work_group_any>;
def work_group_any_mask_vect_info : VectInfo<[v1i32], work_group_any, work_group_any_mask, work_group_any_mask, work_group_any_mask, work_group_any_mask, work_group_any_mask>;
let Types = [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f16, v1f32, v1f64] in {
  foreach func = ["reduce", "scan_inclusive", "scan_exclusive"] in {
    foreach op = ["add", "min", "max", "mul"] in {
     defvar vector_builtin = !cast<OclBuiltin>("work_group_"#func#_#op);
     def work_group_#func#_#op#_vect_info : VectInfo<[], !cast<OclBuiltin>("work_group_"#func#_#op), vector_builtin, vector_builtin, vector_builtin, vector_builtin, vector_builtin>;
     defvar mask_builtin = !cast<OclBuiltin>("work_group_"#func#_#op#"_mask");
     def work_group_#func#_#op#_mask_vect_info : VectInfo<[], !cast<OclBuiltin>("work_group_"#func#_#op), mask_builtin, mask_builtin, mask_builtin, mask_builtin, mask_builtin>;
    }
  }
}
let Types = [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64] in {
  foreach func = ["scan_inclusive", "scan_exclusive"] in {
    foreach op = ["bitwise_and", "bitwise_or", "bitwise_xor"] in {
     defvar vector_builtin = !cast<OclBuiltin>("work_group_"#func#_#op);
     def work_group_#func#_#op#_vect_info : VectInfo<[], !cast<OclBuiltin>("work_group_"#func#_#op), vector_builtin, vector_builtin, vector_builtin, vector_builtin, vector_builtin>;
     defvar mask_builtin = !cast<OclBuiltin>("work_group_"#func#_#op#"_mask");
     def work_group_#func#_#op#_mask_vect_info : VectInfo<[], !cast<OclBuiltin>("work_group_"#func#_#op), mask_builtin, mask_builtin, mask_builtin, mask_builtin, mask_builtin>;
    }
  }
}
let Types = [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64] in {
  def work_group_broadcast1d_vect_info : VectInfo<[], work_group_broadcast1d, work_group_broadcast1d, work_group_broadcast1d, work_group_broadcast1d, work_group_broadcast1d, work_group_broadcast1d>;
  def work_group_broadcast2d_vect_info : VectInfo<[], work_group_broadcast2d, work_group_broadcast2d, work_group_broadcast2d, work_group_broadcast2d, work_group_broadcast2d, work_group_broadcast2d>;
  def work_group_broadcast3d_vect_info : VectInfo<[], work_group_broadcast3d, work_group_broadcast3d, work_group_broadcast3d, work_group_broadcast3d, work_group_broadcast3d, work_group_broadcast3d>;
  def work_group_broadcast1d_mask_vect_info : VectInfo<[], work_group_broadcast1d, work_group_broadcast1d_mask, work_group_broadcast1d_mask, work_group_broadcast1d_mask, work_group_broadcast1d_mask, work_group_broadcast1d_mask>;
  def work_group_broadcast2d_mask_vect_info : VectInfo<[], work_group_broadcast2d, work_group_broadcast2d_mask, work_group_broadcast2d_mask, work_group_broadcast2d_mask, work_group_broadcast2d_mask, work_group_broadcast2d_mask>;
  def work_group_broadcast3d_mask_vect_info : VectInfo<[], work_group_broadcast3d, work_group_broadcast3d_mask, work_group_broadcast3d_mask, work_group_broadcast3d_mask, work_group_broadcast3d_mask, work_group_broadcast3d_mask>;
  // Assume uniform variants
  def : VectInfo<[], work_group_broadcast1d, work_group_broadcast1d_assume_uniform, work_group_broadcast1d_assume_uniform, work_group_broadcast1d_assume_uniform, work_group_broadcast1d_assume_uniform, work_group_broadcast1d_assume_uniform>;
  def : VectInfo<[], work_group_broadcast2d, work_group_broadcast2d_assume_uniform, work_group_broadcast2d_assume_uniform, work_group_broadcast2d_assume_uniform, work_group_broadcast2d_assume_uniform, work_group_broadcast2d_assume_uniform>;
  def : VectInfo<[], work_group_broadcast3d, work_group_broadcast3d_assume_uniform, work_group_broadcast3d_assume_uniform, work_group_broadcast3d_assume_uniform, work_group_broadcast3d_assume_uniform, work_group_broadcast3d_assume_uniform>;
  // Assume uniform with mask
  def : VectInfo<[], work_group_broadcast1d, work_group_broadcast1d_assume_uniform_mask, work_group_broadcast1d_assume_uniform_mask, work_group_broadcast1d_assume_uniform_mask, work_group_broadcast1d_assume_uniform_mask, work_group_broadcast1d_assume_uniform_mask>;
  def : VectInfo<[], work_group_broadcast2d, work_group_broadcast2d_assume_uniform_mask, work_group_broadcast2d_assume_uniform_mask, work_group_broadcast2d_assume_uniform_mask, work_group_broadcast2d_assume_uniform_mask, work_group_broadcast2d_assume_uniform_mask>;
  def : VectInfo<[], work_group_broadcast3d, work_group_broadcast3d_assume_uniform_mask, work_group_broadcast3d_assume_uniform_mask, work_group_broadcast3d_assume_uniform_mask, work_group_broadcast3d_assume_uniform_mask, work_group_broadcast3d_assume_uniform_mask>;
}

let Types = [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64] in {
  foreach op = ["bitwise_and", "bitwise_or", "bitwise_xor"] in {
    def : VectInfo<[], !cast<OclBuiltin>("work_group_reduce_"#op), !cast<OclBuiltin>("work_group_reduce_"#op), !cast<OclBuiltin>("work_group_reduce_"#op), !cast<OclBuiltin>("work_group_reduce_"#op), !cast<OclBuiltin>("work_group_reduce_"#op), !cast<OclBuiltin>("work_group_reduce_"#op)>;
    defvar mask_builtin = !cast<OclBuiltin>("work_group_reduce_"#op#"_mask");
    def : VectInfo<[], !cast<OclBuiltin>("work_group_reduce_"#op), mask_builtin, mask_builtin, mask_builtin, mask_builtin, mask_builtin>;
  }
}

let Types = [v1i32] in {
  foreach op = ["logical_and", "logical_or", "logical_xor"] in {
    def : VectInfo<[], !cast<OclBuiltin>("work_group_reduce_"#op), !cast<OclBuiltin>("work_group_reduce_"#op), !cast<OclBuiltin>("work_group_reduce_"#op), !cast<OclBuiltin>("work_group_reduce_"#op), !cast<OclBuiltin>("work_group_reduce_"#op), !cast<OclBuiltin>("work_group_reduce_"#op)>;
    defvar mask_builtin = !cast<OclBuiltin>("work_group_reduce_"#op#"_mask");
    def work_group_#func#_#op#_mask_vect_info : VectInfo<[], !cast<OclBuiltin>("work_group_reduce_"#op), mask_builtin, mask_builtin, mask_builtin, mask_builtin, mask_builtin>;
  }
}

// -------------------------------------------------------------------------------------------------------------------------------
// Per-WI versions of work-group functions. These functions have the following arguments:
//   1. WI (scalar or vectorized packet) argument of the same type as corresponding WG function
//   2. Pointer to accumulator. It is the most recent return value (for previous WI).
//      It is of the same type and width as WI argument.
//      Thus, - for uniform functions it contains the intermediate result ad hoc;
//            - for scan function it contains the result for previous WI;
//            - for broadcast function it contains broadcasted value or arbitrary value
//              (until the broadcasted value will be encountered)
//   3. For broadcast function ONLY: linear form of linear_id(x,y,z) argument of corresponding WG function, and linearID of the WI
//  Return value: intermediate result of calculation (for uniform and broadcast functions) or per-WI result (for scan functions)
// -------------------------------------------------------------------------------------------------------------------------------

// Forward definitions of utility add/all/any functions used by other helpers
let NeedForwardDecl = 1 in { /* work around the missing prototype in opencl.h */
  let Types = ExpandTypesByVFAndFlatten<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f16, v1f32, v1f64], [1, 4, 8, 16, 32, 64]>.Tout,
      Attrs = [OVERLOADABLE] in {
    def work_group_add_helper_util  : OclBuiltin<"work_group_add_util",  (outs gentype:$ret), (ins gentype:$src, gentype:$accum), [], [], 1, 0>;
    def work_group_mul_helper_util  : OclBuiltin<"work_group_mul_util",  (outs gentype:$ret), (ins gentype:$src, gentype:$accum), [], [], 1, 0>;
  }
  let Types = [ v1i32, v4i32, v8i32, v16i32, v32i32, v64i32 ],
      Attrs = [OVERLOADABLE] in {
    def work_group_all_helper_util  : OclBuiltin<"work_group_all_util",  (outs gentype:$ret), (ins gentype:$src, gentype:$accum), [], [], 1, 0>;
    def work_group_any_helper_util  : OclBuiltin<"work_group_any_util",  (outs gentype:$ret), (ins gentype:$src, gentype:$accum), [], [], 1, 0>;
  }
  let Types = ExpandTypesByVFAndFlatten<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64], [1, 4, 8, 16, 32, 64]>.Tout,
      Attrs = [OVERLOADABLE] in {
    def work_group_bitwise_and_helper_util  : OclBuiltin<"work_group_bitwise_and_util",  (outs gentype:$ret), (ins gentype:$src, gentype:$accum), [], [], 1, 0>;
    def work_group_bitwise_or_helper_util  : OclBuiltin<"work_group_bitwise_or_util",  (outs gentype:$ret), (ins gentype:$src, gentype:$accum), [], [], 1, 0>;
    def work_group_bitwise_xor_helper_util  : OclBuiltin<"work_group_bitwise_xor_util",  (outs gentype:$ret), (ins gentype:$src, gentype:$accum), [], [], 1, 0>;
  }
  let Types = ExpandTypesByVFAndFlatten<[v1i32], [1, 4, 8, 16, 32, 64]>.Tout,
      Attrs = [OVERLOADABLE] in {
    def work_group_logical_and_helper_util  : OclBuiltin<"work_group_logical_and_util",  (outs gentype:$ret), (ins gentype:$src, gentype:$accum), [], [], 1, 0>;
    def work_group_logical_or_helper_util  : OclBuiltin<"work_group_logical_or_util",  (outs gentype:$ret), (ins gentype:$src, gentype:$accum), [], [], 1, 0>;
    def work_group_logical_xor_helper_util  : OclBuiltin<"work_group_logical_xor_util",  (outs gentype:$ret), (ins gentype:$src, gentype:$accum), [], [], 1, 0>;
  }
}


let Types = [ v1i32, v4i32, v8i32, v16i32, v32i32, v64i32 ],
    Attrs = [OVERLOADABLE] in {
  def work_group_all_helper : OclBuiltin<"work_group_all", (outs gentype:$ret), (ins gentype:$predicate, ppgentype:$accum), [], [], 1, 0>;
  def work_group_any_helper : OclBuiltin<"work_group_any", (outs gentype:$ret), (ins gentype:$predicate, ppgentype:$accum), [], [], 1, 0>;
}

let Types = [ v4i32, v8i32, v16i32, v32i32, v64i32 ],
    Attrs = [OVERLOADABLE] in {
  def work_group_all_helper_mask : OclBuiltin<"work_group_all", (outs gentype:$ret), (ins gentype:$predicate, gentype:$mask, ppgentype:$accum), [], [], 1, 0>;
  def work_group_any_helper_mask : OclBuiltin<"work_group_any", (outs gentype:$ret), (ins gentype:$predicate, gentype:$mask, ppgentype:$accum), [], [], 1, 0>;
}

let Types = [ v1i8, v4i8, v8i8, v16i8, v32i8, v64i8,
              v1u8, v4u8, v8u8, v16u8, v32u8, v64u8,
              v1i16, v4i16, v8i16, v16i16, v32i16, v64i16,
              v1u16, v4u16, v8u16, v16u16, v32u16, v64u16,
              v1i32, v4i32, v8i32, v16i32, v32i32, v64i32,
              v1u32, v4u32, v8u32, v16u32, v32u32, v64u32,
              v1i64, v4i64, v8i64, v16i64, v32i64, v64i64,
              v1u64, v4u64, v8u64, v16u64, v32u64, v64u64,
              v1f32, v4f32, v8f32, v16f32, v32f32, v64f32,
              v1f64, v4f64, v8f64, v16f64, v32f64, v64f64],
    Attrs = [OVERLOADABLE] in {
  def work_group_broadcast_helper : OclBuiltin<"work_group_broadcast", (outs gentype:$ret), (ins gentype:$src, size_t:$linear_local_id, size_t:$linear_id, ppgentype:$accum), [], [], 1, 0>;
}

let Types = [ v1i8, v4i8, v8i8, v16i8, v32i8, v64i8,
              v1u8, v4u8, v8u8, v16u8, v32u8, v64u8,
              v1i16, v4i16, v8i16, v16i16, v32i16, v64i16,
              v1u16, v4u16, v8u16, v16u16, v32u16, v64u16,
              v4i32, v8i32, v16i32, v32i32, v64i32,
              v4u32, v8u32, v16u32, v32u32, v64u32,
              v4i64, v8i64, v16i64, v32i64, v64i64,
              v4u64, v8u64, v16u64, v32u64, v64u64,
              v4f32, v8f32, v16f32, v32f32, v64f32,
              v4f64, v8f64, v16f64, v32f64, v64f64],
    Attrs = [OVERLOADABLE] in {
  def work_group_broadcast_helper_mask : OclBuiltin<"__work_group_broadcast", (outs gentype:$ret), (ins gentype:$src, size_t:$linear_local_id, size_t:$linear_id, gentype:$mask, ppgentype:$accum), [], [], 1, 0>;
}

foreach func = ["reduce", "scan_exclusive", "scan_inclusive"] in {
  foreach op = ["add", "min", "max", "mul"] in {
    let Types = ExpandTypesByVFAndFlatten<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f16, v1f32, v1f64], [1, 4, 8, 16, 32, 64]>.Tout,
        Attrs = [OVERLOADABLE] in {
      def work_group_#func#_#op#_helper :
                OclBuiltin<"work_group_"#func#"_"#op, (outs gentype:$ret), (ins gentype:$src, ppgentype:$accum), [], [], 1, 0>;
    }
  }
}

foreach func = ["reduce", "scan_exclusive", "scan_inclusive"] in {
  foreach op = ["bitwise_and", "bitwise_or", "bitwise_xor"] in {
    let Types = ExpandTypesByVFAndFlatten<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64], [1, 4, 8, 16, 32, 64]>.Tout,
        Attrs = [OVERLOADABLE] in {
      def work_group_#func#_#op#_helper :
                OclBuiltin<"work_group_"#func#"_"#op, (outs gentype:$ret), (ins gentype:$src, ppgentype:$accum), [], [], 1, 0>;
    }
  }
}

foreach op = ["logical_and", "logical_or", "logical_xor"] in {
  let Types = ExpandTypesByVFAndFlatten<[v1i32], [1, 4, 8, 16, 32, 64]>.Tout,
      Attrs = [OVERLOADABLE] in {
    def work_group_reduce_#op#_helper :
      OclBuiltin<"work_group_reduce_"#op, (outs gentype:$ret), (ins gentype:$src, ppgentype:$accum), [], [], 1, 0>;
    def work_group_reduce_#op#_helper_mask :
      OclBuiltin<"work_group_reduce_"#op, (outs gentype:$ret), (ins gentype:$src, gentype:$mask, ppgentype:$accum), [], [], 1, 0>;
  }
}

foreach func = ["reduce", "scan_exclusive", "scan_inclusive"] in {
  foreach op = ["add", "min", "max", "mul"] in {
    let Types = ExpandTypesByVFAndFlatten<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f16, v1f32, v1f64], [1, 4, 8, 16, 32, 64]>.Tout,
        Attrs = [OVERLOADABLE] in {
      def work_group_#func#_#op#_helper_mask :
                OclBuiltin<"work_group_"#func#"_"#op, (outs gentype:$ret), (ins gentype:$src, gentype:$mask, ppgentype:$accum), [], [], 1, 0>;
    }
  }
}

foreach func = ["reduce", "scan_exclusive", "scan_inclusive"] in {
  foreach op = ["bitwise_and", "bitwise_or", "bitwise_xor"] in {
    let Types = ExpandTypesByVFAndFlatten<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64], [1, 4, 8, 16, 32, 64]>.Tout,
        Attrs = [OVERLOADABLE] in {
      def work_group_#func#_#op#_helper_mask :
                OclBuiltin<"work_group_"#func#"_"#op, (outs gentype:$ret), (ins gentype:$src, gentype:$mask, ppgentype:$accum), [], [], 1, 0>;
    }
  }
}

// -----------------------------------------------------------------------------------------------------------------------
// Finalization functions for uniform non-broadcast vectorized WG functions. These functions have the following argument:
// 1. Value of the last (accumulated) return value (vectorized packet) of per-WI call
// Return value: uniform vector result of all WI calls - to be used by function call usages
// -----------------------------------------------------------------------------------------------------------------------

let Types = [ v4i32, v8i32, v16i32, v32i32, v64i32 ],
    Attrs = [OVERLOADABLE] in {
  def work_group_all_finalize_helper : OclBuiltin<"__finalize_work_group_all", (outs gentype:$ret), (ins gentype:$accum), [], [], 1, 0>;
  def work_group_any_finalize_helper : OclBuiltin<"__finalize_work_group_any", (outs gentype:$ret), (ins gentype:$accum), [], [], 1, 0>;
  def work_group_all_finalize_helper_mask : OclBuiltin<"__finalize_work_group_all", (outs gentype:$ret), (ins gentype:$accum, gentype:$dummy_mask), [], [], 1, 0>;
  def work_group_any_finalize_helper_mask : OclBuiltin<"__finalize_work_group_any", (outs gentype:$ret), (ins gentype:$accum, gentype:$dummy_mask), [], [], 1, 0>;
}

let Types = ExpandTypesByVFAndFlatten<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f16, v1f32, v1f64], [4, 8, 16, 32, 64]>.Tout,
    Attrs = [OVERLOADABLE] in {
  def work_group_reduce_add_finalize_helper : OclBuiltin<"__finalize_work_group_reduce_add", (outs gentype:$ret), (ins gentype:$accum), [], [], 1, 0>;
  def work_group_reduce_min_finalize_helper : OclBuiltin<"__finalize_work_group_reduce_min", (outs gentype:$ret), (ins gentype:$accum), [], [], 1, 0>;
  def work_group_reduce_max_finalize_helper : OclBuiltin<"__finalize_work_group_reduce_max", (outs gentype:$ret), (ins gentype:$accum), [], [], 1, 0>;
  def work_group_reduce_mul_finalize_helper : OclBuiltin<"__finalize_work_group_reduce_mul", (outs gentype:$ret), (ins gentype:$accum), [], [], 1, 0>;
  def work_group_reduce_add_finalize_helper_mask : OclBuiltin<"__finalize_work_group_reduce_add", (outs gentype:$ret), (ins gentype:$accum, gentype:$dummy_mask), [], [], 1, 0>;
  def work_group_reduce_min_finalize_helper_mask : OclBuiltin<"__finalize_work_group_reduce_min", (outs gentype:$ret), (ins gentype:$accum, gentype:$dummy_mask), [], [], 1, 0>;
  def work_group_reduce_max_finalize_helper_mask : OclBuiltin<"__finalize_work_group_reduce_max", (outs gentype:$ret), (ins gentype:$accum, gentype:$dummy_mask), [], [], 1, 0>;
  def work_group_reduce_mul_finalize_helper_mask : OclBuiltin<"__finalize_work_group_reduce_mul", (outs gentype:$ret), (ins gentype:$accum, gentype:$dummy_mask), [], [], 1, 0>;
}

let Types = ExpandTypesByVFAndFlatten<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64], [1, 4, 8, 16, 32, 64]>.Tout,
    Attrs = [OVERLOADABLE] in {
  def work_group_reduce_bitwise_and_finalize_helper : OclBuiltin<"__finalize_work_group_reduce_bitwise_and", (outs gentype:$ret), (ins gentype:$accum), [], [], 1, 0>;
  def work_group_reduce_bitwise_or_finalize_helper : OclBuiltin<"__finalize_work_group_reduce_bitwise_or", (outs gentype:$ret), (ins gentype:$accum), [], [], 1, 0>;
  def work_group_reduce_bitwise_xor_finalize_helper : OclBuiltin<"__finalize_work_group_reduce_bitwise_xor", (outs gentype:$ret), (ins gentype:$accum), [], [], 1, 0>;
  def work_group_reduce_bitwise_and_finalize_helper_mask : OclBuiltin<"__finalize_work_group_reduce_bitwise_and", (outs gentype:$ret), (ins gentype:$accum, gentype:$dummy_mask), [], [], 1, 0>;
  def work_group_reduce_bitwise_or_finalize_helper_mask : OclBuiltin<"__finalize_work_group_reduce_bitwise_or", (outs gentype:$ret), (ins gentype:$accum, gentype:$dummy_mask), [], [], 1, 0>;
  def work_group_reduce_bitwise_xor_finalize_helper_mask : OclBuiltin<"__finalize_work_group_reduce_bitwise_xor", (outs gentype:$ret), (ins gentype:$accum, gentype:$dummy_mask), [], [], 1, 0>;
}

let Types = ExpandTypesByVFAndFlatten<[v1i32], [1, 4, 8, 16, 32, 64]>.Tout,
    Attrs = [OVERLOADABLE] in {
  def work_group_reduce_logical_and_finalize_helper : OclBuiltin<"__finalize_work_group_reduce_logical_and", (outs gentype:$ret), (ins gentype:$accum), [], [], 1, 0>;
  def work_group_reduce_logical_or_finalize_helper : OclBuiltin<"__finalize_work_group_reduce_logical_or", (outs gentype:$ret), (ins gentype:$accum), [], [], 1, 0>;
  def work_group_reduce_logical_xor_finalize_helper : OclBuiltin<"__finalize_work_group_reduce_logical_xor", (outs gentype:$ret), (ins gentype:$accum), [], [], 1, 0>;
  def work_group_reduce_logical_and_finalize_helper_mask : OclBuiltin<"__finalize_work_group_reduce_logical_and", (outs gentype:$ret), (ins gentype:$accum, gentype:$dummy_mask), [], [], 1, 0>;
  def work_group_reduce_logical_or_finalize_helper_mask : OclBuiltin<"__finalize_work_group_reduce_logical_or", (outs gentype:$ret), (ins gentype:$accum, gentype:$dummy_mask), [], [], 1, 0>;
  def work_group_reduce_logical_xor_finalize_helper_mask : OclBuiltin<"__finalize_work_group_reduce_logical_xor", (outs gentype:$ret), (ins gentype:$accum, gentype:$dummy_mask), [], [], 1, 0>;
}
