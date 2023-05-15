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

// Finalize synchronization helper for uniform workgroup functions.
// A workgroup function is uniform if it returns same value for all work items.
// e.g. work_group_broadcast, work_group_reduce_*
//
// The finalize helper is inserted after barrier sync point by GroupBuiltin
// pass, to make sure the result is used after barrier synchronization.
//
// Prototypes
let Types = ExpandTypesByVFAndFlatten<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64,
                                       v1f16, v1f32, v1f64], [1, 4, 8, 16, 32, 64]>.Tout,
    Attrs = [OVERLOADABLE] in {
  // identity
  // This finalize helper is a special function, which returns its arguments as is, 
  // and is used for work_group_broadcast and all scalar work_group functions
  def work_group_identity_finalize_helper : OclBuiltin<"__finalize_work_group_identity", (outs gentype:$ret), (ins gentype:$accum)>;
}

// Implementations
def : OclBuiltinImpl<work_group_identity_finalize_helper, work_group_identity_finalize_helper.Types, /*isDeclOnly*/false, ret_arg0_code>, SHARED;

// VectInfos (not needed for finalize helpers)

// --- work_group sort builtins ---
//
// https://github.com/andreyfe1/llvm/blob/design_group_sort/sycl/doc/GroupSort.md
//
// for default sorting algorithm
// void __devicelib_default_work_group_joint_sort_ascending_<encoded_param_types>(T* first, uint n, byte* scratch);
// void __devicelib_default_work_group_joint_sort_descending_<encoded_param_types>(T* first, uint n, byte* scratch);
//
// for fixed-size arrays
// void __devicelib_default_work_group_private_sort_close_ascending_<encoded_param_types>(T* first, uint n, byte* scratch);
// void __devicelib_default_work_group_private_sort_close_descending_<encoded_param_types>(T* first, uint n, byte* scratch);
// void __devicelib_default_work_group_private_sort_spread_ascending_<encoded_param_types>(T* first, uint n, byte* scratch);
// void __devicelib_default_work_group_private_sort_spread_descending_<encoded_param_types>(T* first, uint n, byte* scratch);
//
// for key value sorting using the default algorithm
// void __devicelib_default_work_group_joint_sort_ascending_<encoded_param_types>(T* keys_first, U* values_first, uint n, byte* scratch); 
// void __devicelib_default_work_group_joint_sort_descending_<encoded_param_types>(T* keys_first, U* values_first, uint n, byte* scratch);
//
// for key value sorting using fixed-size arrays
// void __devicelib_default_work_group_private_sort_close_ascending_<encoded_param_types>(T* keys_first, U* values_first, uint n, byte* scratch);
// void __devicelib_default_work_group_private_sort_close_descending_<encoded_param_types>(T* keys_first, U* values_first, uint n, byte* scratch); 
// void __devicelib_default_work_group_private_sort_spread_ascending_<encoded_param_types>(T* keys_first, U* values_first, uint n, byte* scratch); 
// void __devicelib_default_work_group_private_sort_spread_descending_<encoded_param_types>(T* keys_first, U* values_first, uint n, byte* scratch);
//
// T is from the following list i8, i16, i32, i64, u8, u16, u32, u64, f16, f32, f64
// p1 for global/private address space.
// p3 for shared local memory

// To reduce RTL size, cast all pointer type params to generic address space type,
// then p1 and p3 type will have the same implementation.
// so we just support p1 type here and handle p3 type in Pass

defvar wg_sort_type_name_list = ["i8", "i16", "i32", "i64", "u8", "u16", "u32", "u64", "f16", "f32", "f64"];

// Prototypes
let Attrs = [OVERLOADABLE], NeedForwardDecl = true in {
  foreach func = ["ascending", "descending"] in {
    foreach method = ["joint_sort", "private_sort_close", "private_sort_spread"] in {
      // scalar
      foreach type_name = wg_sort_type_name_list in {
        defvar type = !cast<OclType>("v1"#type_name);
        // key only sorting
        let Types = [type] in
          def work_group_#method#_#func#_#type_name : OclBuiltin<
            "__devicelib_default_work_group_"#method#"_"#func#"_p1"#type_name#"_u32_p1i8",
            (outs void:$ret), (ins pgentype:$first, uint:$n, ptr_char:$scratch)>;
        // key value sorting, scalar
        foreach value_type_name = wg_sort_type_name_list in {
          let Types = [type] in {
            defvar value_type = !cast<OclType>("pv1"#value_type_name);
            def work_group_#method#_#func#_#type_name#_#value_type_name : OclBuiltin<
              "__devicelib_default_work_group_"#method#"_"#func#"_p1"#type_name#"_p1"#value_type_name#"_u32_p1i8",
              (outs void:$ret), (ins pgentype:$key, value_type:$value, uint:$n, ptr_char:$scratch)>;
          }
        }
      }
    }
  }
}

defvar joint_sort_code =
[{
  __ocl_sort($Arg0VarName, $Arg1VarName, $Mode);
}];

defvar private_sort_close_code = 
[{
  $Arg0Type data = ($Arg0Type)$Arg2VarName;
  __ocl_sort(data, $Arg1VarName, $Mode);
}];

defvar private_sort_spread_code = private_sort_close_code;

defvar joint_sort_key_value_code =
[{
  __ocl_sort($Arg0VarName, $Arg1VarName, $Arg2VarName, $Mode);
}];

defvar private_sort_close_key_value_code = 
[{
  $Arg0Type key_data = ($Arg0Type)$Arg3VarName;
  $Arg1Type value_data = ($Arg1Type)($Arg3VarName + $Arg2VarName * $KeyBitWidth / 8);
  __ocl_sort(key_data, value_data, $Arg2VarName, $Mode);
}];

defvar private_sort_spread_key_value_code = private_sort_close_key_value_code;

// Implementations
foreach func = ["ascending", "descending"] in {
  defvar mode = !if(!eq(func, "ascending"), "0", "1");
  foreach sort_type = ["joint_sort", "private_sort_close", "private_sort_spread"] in { 
    defvar codes = !cond(
      !eq(sort_type, "joint_sort"): joint_sort_code,
      !eq(sort_type, "private_sort_close"): private_sort_close_code,
      !eq(sort_type, "private_sort_spread"): private_sort_spread_code);
    defvar kv_codes = !cond(
      !eq(sort_type, "joint_sort"): joint_sort_key_value_code,
      !eq(sort_type, "private_sort_close"): private_sort_close_key_value_code,
      !eq(sort_type, "private_sort_spread"): private_sort_spread_key_value_code);
    foreach type_name = wg_sort_type_name_list in {
      // scalar
      defvar builtin1 = !cast<OclBuiltin>("work_group_"#sort_type#"_"#func#"_"#type_name);
      // key-only
      def : OclBuiltinImpl<builtin1, builtin1.Types, false, codes, [["$Mode", !cast<string>(mode)]]>, SHARED;
      defvar key_type = !cast<OclType>("v1"#type_name);
      // key-value
      foreach value_type_name = wg_sort_type_name_list in {
        defvar builtin4 = !cast<OclBuiltin>("work_group_"#sort_type#"_"#func#"_"#type_name#"_"#value_type_name);
        def : OclBuiltinImpl<builtin4, builtin4.Types, false, kv_codes, [["$Mode", !cast<string>(mode)], ["$KeyBitWidth", !cast<string>(key_type.BitWidth)]]>, SHARED;
      }
    }
  }
}

// ---- work group private sort spread sort, copy scratch to output memory helper
// Prototypes
let Attrs = [OVERLOADABLE], NeedForwardDecl = true in {
  foreach type_name = wg_sort_type_name_list in {
    // key only sorting, copy memory
    defvar type = !cast<OclType>("v1"#type_name);
    defvar int_type = !cast<OclType>("v1i64");
    let Types = [type] in
      def work_group_private_sort_spread_copy_#type : OclBuiltin<
        "__devicelib_default_work_group_private_spread_sort_copy",
        (outs void:$ret), (ins pgentype:$first, uint:$n, ptr_char:$scratch,
                            int_type:$local_id, int_type:$local_size, bool:$direction)>;
    // key value sorting, copy memory
    foreach value_type_name = wg_sort_type_name_list in {
      let Types = [type] in {
        defvar value_type = !cast<OclType>("pv1"#value_type_name);
        def work_group_private_sort_spread_copy_#type#_#value_type : OclBuiltin<
          "__devicelib_default_work_group_private_spread_sort_copy", 
          (outs void:$ret), (ins pgentype:$key, value_type:$value, uint:$n, 
                             ptr_char:$scratch, int_type:$local_id, 
                             int_type:$local_size, bool:$direction)>;
      }
    }
  }
}

defvar private_spread_sort_copy_helper_code =
[{
  $Arg0Type data = ($Arg0Type)$Arg2VarName;
  for (uint i = 0; i < $Arg1VarName; ++i) {
    $Arg0VarName[i] = data[$Arg4VarName * i + $Arg3VarName];
  }
}];

defvar private_spread_sort_key_value_copy_helper_code =
[{
  $Arg0Type key_data = ($Arg0Type)$Arg3VarName;
  $Arg1Type value_data = ($Arg1Type)(($Arg0Type)$Arg3VarName + $Arg5VarName * $Arg2VarName);
  for (uint i = 0; i < $Arg2VarName; ++i) {
    $Arg0VarName[i] = key_data[$Arg5VarName * i + $Arg4VarName];
    $Arg1VarName[i] = value_data[$Arg5VarName * i + $Arg4VarName];
  }
}];

// Implementations
foreach func = ["ascending", "descending"] in {
  foreach type_name = wg_sort_type_name_list in {
    defvar type = !cast<OclType>("v1"#type_name);
    defvar builtin = !cast<OclBuiltin>("work_group_private_sort_spread_copy_"#type);
    def : OclBuiltinImpl<builtin, builtin.Types, false, private_spread_sort_copy_helper_code>, SHARED;
    foreach value_type_name = wg_sort_type_name_list in {
      defvar value_type = !cast<OclType>("pv1"#value_type_name);
      defvar builtin_kv = !cast<OclBuiltin>("work_group_private_sort_spread_copy_"#type#"_"#value_type);
      def : OclBuiltinImpl<builtin_kv, builtin_kv.Types, false, private_spread_sort_key_value_copy_helper_code>, SHARED;
    }
  }
}
