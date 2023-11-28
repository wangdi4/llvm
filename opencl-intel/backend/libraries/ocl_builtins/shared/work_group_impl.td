//
// Copyright (C) 2023 Intel Corporation
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

defvar work_group_v1_types_i = [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64];
defvar work_group_v1_types_f = [v1f16, v1f32, v1f64];
defvar work_group_v1_types = [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f16, v1f32, v1f64];

// Functions for add/mul operations (in order to have all per-WI helper functions uniformal)

foreach name = ["add", "mul", "bitwise_and", "bitwise_or", "bitwise_xor"] in {
  defvar types = ExpandTypesByVFAndFlatten<work_group_v1_types, [1, 4, 8, 16, 32, 64]>.Tout;
  defvar op = !cond(!eq(name, "add") : "+",
                    !eq(name, "mul") : "*",
                    !eq(name, "bitwise_and") : "&",
                    !eq(name, "bitwise_or") : "|",
                    !eq(name, "bitwise_xor") : "^");
  def wg_#name#_helper_util_impl : OclBuiltinImpl<!cast<OclBuiltin>("work_group_" # name # "_helper_util"), types, 0, binary_op_code, [["$Op", op]]>;
}

// v1 work_group_reduce functions

foreach name = ["add", "mul"] in {
  def wg_reduce_#name#_scalar_impl : OclBuiltinImpl<!cast<OclBuiltin>("work_group_reduce_" # name # "_helper"), work_group_v1_types, 0, work_group_inclusive_function_op_scalar, [["$FUNC", "work_group_" # name # "_util"]]>;
  foreach clusive = ["exclusive", "inclusive"] in {
    defvar scalar_code = !cond(!eq(clusive, "exclusive") : work_group_scan_exclusive_function_op_scalar, true : work_group_inclusive_function_op_scalar);
    def wg_scan_#clusive#_#name#_scalar_impl : OclBuiltinImpl<!cast<OclBuiltin>("work_group_scan_" # clusive # "_" # name # "_helper"), work_group_v1_types, 0, scalar_code, [["$FUNC", "work_group_" # name # "_util"]]>;
  }
}

foreach name = ["bitwise_and", "bitwise_or", "bitwise_xor"] in {
  def wg_reduce_#name#_scalar_impl : OclBuiltinImpl<!cast<OclBuiltin>("work_group_reduce_" # name # "_helper"), work_group_v1_types_i, 0, work_group_inclusive_function_op_scalar, [["$FUNC", "work_group_" # name # "_util"]]>;
  foreach clusive = ["exclusive", "inclusive"] in {
    defvar scalar_code = !cond(!eq(clusive, "exclusive") : work_group_scan_exclusive_function_op_scalar, true : work_group_inclusive_function_op_scalar);
    def wg_scan_#clusive#_#name#_scalar_impl : OclBuiltinImpl<!cast<OclBuiltin>("work_group_scan_" # clusive # "_" # name # "_helper"), work_group_v1_types, 0, scalar_code, [["$FUNC", "work_group_" # name # "_util"]]>;
    def wg_scan_#clusive#_#name#_fake : OclBuiltinImpl<!cast<OclBuiltin>("work_group_scan_" # clusive # "_" # name), work_group_v1_types, 0, ret_arg0_code>;
    def wg_scan_#clusive#_#name#_mask_fake : OclBuiltinImpl<!cast<OclBuiltin>("work_group_scan_" # clusive # "_" # name # "_mask"), work_group_v1_types, 0, ret_arg0_code>;
  }
}

foreach name = ["min", "max"] in {
  defvar reduce_helper = !cast<OclBuiltin>("work_group_reduce_" # name # "_helper");
  def wg_reduce_#name#_scalar_impl   : OclBuiltinImpl<reduce_helper, work_group_v1_types_i, 0, work_group_inclusive_function_op_scalar, [["$FUNC", name]]>;
  def wg_reduce_#name#_scalar_f_impl : OclBuiltinImpl<reduce_helper, work_group_v1_types_f, 0, work_group_inclusive_function_op_scalar, [["$FUNC", "f" # name]]>;
  foreach clusive = ["exclusive", "inclusive"] in {
    defvar scan_helper = !cast<OclBuiltin>("work_group_scan_" # clusive # "_" # name # "_helper");
    defvar scalar_code = !cond(!eq(clusive, "exclusive") : work_group_scan_exclusive_function_op_scalar, true : work_group_inclusive_function_op_scalar);
    def wg_scan_#clusive#_#name#_scalar_impl   : OclBuiltinImpl<scan_helper, work_group_v1_types_i, 0, scalar_code, [["$FUNC", name]]>;
    def wg_scan_#clusive#_#name#_scalar_f_impl : OclBuiltinImpl<scan_helper, work_group_v1_types_f, 0, scalar_code, [["$FUNC", "f" # name]]>;
  }
}

foreach name = ["add", "min", "max", "mul"] in {
  def wg_reduce_#name#_fake : OclBuiltinImpl<!cast<OclBuiltin>("work_group_reduce_" # name), work_group_v1_types, 0, ret_arg0_code>;
  def wg_reduce_#name#_mask_fake : OclBuiltinImpl<!cast<OclBuiltin>("work_group_reduce_" # name # "_mask"), work_group_v1_types, 0, ret_arg0_code>;
  foreach clusive = ["exclusive", "inclusive"] in {
    def wg_scan_#clusive#_#name#_fake : OclBuiltinImpl<!cast<OclBuiltin>("work_group_scan_" # clusive # "_" # name), work_group_v1_types, 0, ret_arg0_code>;
    def wg_scan_#clusive#_#name#_mask_fake : OclBuiltinImpl<!cast<OclBuiltin>("work_group_scan_" # clusive # "_" # name # "_mask"), work_group_v1_types, 0, ret_arg0_code>;
  }
}

// vector work_group helper and finalize_helper functions.

// work_grup_reduce

foreach vf = [4, 8, 16, 32, 64] in {
  defvar vector_code = !cond(!eq(vf, 64) : work_group_inclusive_function_op_vector64,
                             !eq(vf, 32) : work_group_inclusive_function_op_vector32,
                             true : work_group_inclusive_function_op_vector);
  defvar mask_code = !cond(!eq(vf, 64) : work_group_inclusive_function_op_mask64,
                           !eq(vf, 32) : work_group_inclusive_function_op_mask32,
                           true : work_group_inclusive_function_op_mask);
  defvar finalize_code = !cond(!eq(vf, 4) : work_group_finalize_function_op4,
                               !eq(vf, 8) : work_group_finalize_function_op8,
                               !eq(vf, 16) : work_group_finalize_function_op16,
                               !eq(vf, 32) : work_group_finalize_function_op32,
                               !eq(vf, 64) : work_group_finalize_function_op64);
  foreach name = ["add", "mul"] in {
    defvar types = ExpandTypesByVFAndFlatten<work_group_v1_types, [vf]>.Tout;
    defvar func = "work_group_" # name # "_util";

    def wg_reduce_#name#vf#_impl : OclBuiltinImpl<!cast<OclBuiltin>("work_group_reduce_" # name # "_helper"), types, 0, vector_code, [["$FUNC", "work_group_" # name # "_util"]]>;
    defvar val = !cond(!eq(name, "add") : "0", true : "1");
    def wg_reduce_#name#vf#_mask_impl : OclBuiltinImpl<!cast<OclBuiltin>("work_group_reduce_" # name # "_helper_mask"), types, 0, mask_code, [["$FUNC", "work_group_" # name # "_util"], ["$VAL", val]]>;

    def wg_reduce_#name#vf#_finalizer_impl : OclBuiltinImpl<!cast<OclBuiltin>("work_group_reduce_" # name # "_finalize_helper"), types, 0, finalize_code, [["$FUNC", func]]>;
    def wg_reduce_#name#vf#_finalizer_mask_impl : OclBuiltinImpl<!cast<OclBuiltin>("work_group_reduce_" # name # "_finalize_helper_mask"), types, 0, finalize_code, [["$FUNC", func]]>;
  }

  foreach name = ["min", "max"] in {
    defvar types_i = ExpandTypesByVFAndFlatten<work_group_v1_types_i, [vf]>.Tout;
    defvar types_f = ExpandTypesByVFAndFlatten<work_group_v1_types_f, [vf]>.Tout;

    defvar helper = !cast<OclBuiltin>("work_group_reduce_" # name # "_helper");
    def wg_reduce_#name#vf#_impl : OclBuiltinImpl<helper, types_i, 0, vector_code, [["$FUNC", name]]>;
    def wg_reduce_#name#vf#_f_impl : OclBuiltinImpl<helper, types_f, 0, vector_code, [["$FUNC", "f" # name]]>;

    defvar helper_mask = !cast<OclBuiltin>("work_group_reduce_" # name # "_helper_mask");
    foreach ty = ["i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64"] in {
      def wg_reduce_#name#vf#_mask_#ty#_impl : OclBuiltinImpl<helper_mask, [!cast<OclType>("v" # vf # ty)], 0, mask_code, [["$FUNC", name], ["$VAL", !cond(!eq(name, "min") : "$MaxVal", true : "$MinVal")]]>;
    }
    def wg_reduce_#name#vf#_mask_f_impl : OclBuiltinImpl<helper_mask, types_f, 0, mask_code, [["$FUNC", "f" # name], ["$VAL", !cond(!eq(name, "min") : "INFINITY", true : "-INFINITY")]]>;

    defvar finalize_helper = !cast<OclBuiltin>("work_group_reduce_" # name # "_finalize_helper");
    def wg_reduce_#name#vf#_finalizer_impl : OclBuiltinImpl<finalize_helper, types_i, 0, finalize_code, [["$FUNC", name]]>;
    def wg_reduce_#name#vf#_finalizer_f_impl : OclBuiltinImpl<finalize_helper, types_f, 0, finalize_code, [["$FUNC", "f" # name]]>;

    defvar finalize_helper_mask = !cast<OclBuiltin>("work_group_reduce_" # name # "_finalize_helper_mask");
    def wg_reduce_#name#vf#_finalizer_mask_impl : OclBuiltinImpl<finalize_helper_mask, types_i, 0, finalize_code, [["$FUNC", name]]>;
    def wg_reduce_#name#vf#_finalizer_mask_f_impl : OclBuiltinImpl<finalize_helper_mask, types_f, 0, finalize_code, [["$FUNC", "f" # name]]>;
  }

  foreach name = ["bitwise_and", "bitwise_or", "bitwise_xor"] in {
    defvar types = ExpandTypesByVFAndFlatten<work_group_v1_types, [vf]>.Tout;
    defvar func = "work_group_" # name # "_util";
    defvar bitwise_mask_code = !cond(!eq(vf, 64) : work_group_inclusive_bitwise_op_mask64,
                               !eq(vf, 32) : work_group_inclusive_bitwise_op_mask32,
                               true : work_group_inclusive_bitwise_op_mask);
    defvar val = !cond(!eq(name, "bitwise_and") : "$UMaxVal", true : "0");

    def : OclBuiltinImpl<!cast<OclBuiltin>("work_group_reduce_" # name # "_helper"), types, 0, vector_code, [["$FUNC", func]]>;
    def : OclBuiltinImpl<!cast<OclBuiltin>("work_group_reduce_" # name # "_helper_mask"), types, 0, bitwise_mask_code, [["$FUNC", func], ["$VAL", val]]>;
    def : OclBuiltinImpl<!cast<OclBuiltin>("work_group_reduce_" # name # "_finalize_helper"), types, 0, finalize_code, [["$FUNC", func]]>;
    def : OclBuiltinImpl<!cast<OclBuiltin>("work_group_reduce_" # name # "_finalize_helper_mask"), types, 0, finalize_code, [["$FUNC", func]]>;
  }

  foreach name = ["logical_and", "logical_or", "logical_xor"] in {
    defvar types = ExpandTypesByVFAndFlatten<[v1i32], [vf]>.Tout;
    defvar func = "work_group_" # name # "_util";

    def : OclBuiltinImpl<!cast<OclBuiltin>("work_group_reduce_" # name # "_helper"), types, 0, vector_code, [["$FUNC", "work_group_" # name # "_util"]]>;
    defvar val = !cond(!eq(name, "logical_and") : "1", true : "0");
    def : OclBuiltinImpl<!cast<OclBuiltin>("work_group_reduce_" # name # "_helper_mask"), types, 0, mask_code, [["$FUNC", "work_group_" # name # "_util"], ["$VAL", val]]>;

    def : OclBuiltinImpl<!cast<OclBuiltin>("work_group_reduce_" # name # "_finalize_helper"), types, 0, finalize_code, [["$FUNC", func]]>;
    def : OclBuiltinImpl<!cast<OclBuiltin>("work_group_reduce_" # name # "_finalize_helper_mask"), types, 0, finalize_code, [["$FUNC", func]]>;
  }
}

// work_group_scan

foreach vf = [4, 8, 16, 32, 64] in {
  defvar exclusive_vector_code = !cond(!eq(vf, 64) : work_group_scan_exclusive_function_op_vector64,
                                 !eq(vf, 32) : work_group_scan_exclusive_function_op_vector32,
                                 !eq(vf, 16) : work_group_scan_exclusive_function_op_vector16,
                                 !eq(vf, 8) : work_group_scan_exclusive_function_op_vector8,
                                 true : work_group_scan_exclusive_function_op_vector4);
  defvar inclusive_vector_code = !cond(!eq(vf, 64) : work_group_scan_inclusive_function_op_vector64,
                                 !eq(vf, 32) : work_group_scan_inclusive_function_op_vector32,
                                 !eq(vf, 16) : work_group_scan_inclusive_function_op_vector16,
                                 !eq(vf, 8) : work_group_scan_inclusive_function_op_vector8,
                                 true : work_group_scan_inclusive_function_op_vector4);
  defvar exclusive_mask_code = !cond(!eq(vf, 64) : work_group_scan_exclusive_function_op_mask64,
                                     !eq(vf, 32) : work_group_scan_exclusive_function_op_mask32,
                                     !eq(vf, 16) : work_group_scan_exclusive_function_op_mask16,
                                     !eq(vf, 8) : work_group_scan_exclusive_function_op_mask8,
                                     true : work_group_scan_exclusive_function_op_mask4);
  defvar inclusive_mask_code = !cond(!eq(vf, 64) : work_group_scan_inclusive_function_op_mask64,
                                     !eq(vf, 32) : work_group_scan_inclusive_function_op_mask32,
                                     !eq(vf, 16) : work_group_scan_inclusive_function_op_mask16,
                                     !eq(vf, 8) : work_group_scan_inclusive_function_op_mask8,
                                     true : work_group_scan_inclusive_function_op_mask4);
  foreach clusive = ["exclusive", "inclusive"] in {
    defvar vector_code = !cond(!eq(clusive, "exclusive") : exclusive_vector_code, true : inclusive_vector_code);
    defvar mask_code = !cond(!eq(clusive, "exclusive") : exclusive_mask_code, true : inclusive_mask_code);
    foreach name = ["add", "mul", "bitwise_and", "bitwise_or", "bitwise_xor"] in {
      defvar helper = "work_group_scan_" # clusive # "_" # name # "_helper";
      defvar types = ExpandTypesByVFAndFlatten<work_group_v1_types, [vf]>.Tout;

      def wg_scan_#clusive#_#name#vf#_impl : OclBuiltinImpl<!cast<OclBuiltin>(helper), types, 0, vector_code, [["$FUNC", "work_group_" # name # "_util"]]>;
      defvar val = !cond(!eq(name, "mul") : "1",
                         !eq(name, "bitwise_and") : "1",
                         true : "0");
      def wg_scan_#clusive#_#name#vf#_mask_impl : OclBuiltinImpl<!cast<OclBuiltin>(helper # "_mask"), types, 0, mask_code, [["$FUNC", "work_group_" # name # "_util"], ["$VAL", val]]>;
    }

    foreach name = ["min", "max"] in {
      defvar types_i = ExpandTypesByVFAndFlatten<work_group_v1_types_i, [vf]>.Tout;
      defvar types_f = ExpandTypesByVFAndFlatten<work_group_v1_types_f, [vf]>.Tout;

      defvar helper_name = "work_group_scan_" # clusive # "_" # name # "_helper";
      defvar helper = !cast<OclBuiltin>(helper_name);
      def wg_scan_#clusive#_#name#vf#_impl : OclBuiltinImpl<helper, types_i, 0, vector_code, [["$FUNC", name]]>;
      def wg_scan_#clusive#_#name#vf#_f_impl : OclBuiltinImpl<helper, types_f, 0, vector_code, [["$FUNC", "f" # name]]>;

      defvar helper_mask = !cast<OclBuiltin>(helper_name # "_mask");
      foreach ty = ["i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64"] in {
        def wg_scan_#clusive#_#name#vf#_mask_#ty#_impl : OclBuiltinImpl<helper_mask, [!cast<OclType>("v" # vf # ty)], 0, mask_code, [["$FUNC", name], ["$VAL", !cond(!eq(name, "min") : "$MaxVal", true : "$MinVal")]]>;
      }
      def wg_scan_#clusive#_#name#vf#_mask_f_impl : OclBuiltinImpl<helper_mask, types_f, 0, mask_code, [["$FUNC", "f" # name], ["$VAL", !cond(!eq(name, "min") : "INFINITY", true : "-INFINITY")]]>;
    }
  }
}
