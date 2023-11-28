// Copyright (c) 2006 Intel Corporation
// Group Built-In Functions
// Generic

// According to OpenCL 1.2 rev 15 (ch. 6.12.10, Table 6.18)
// Async Copies from Global to Local Memory, Local to Global Memory.
/*****************************************************************\
 async_work_group_copy:                                          *
 *   event_t async_work_group_copy(__global gentype *dst,        *
                                   const __local gentype *src,   *
                                   size_t  num_elements,         *
                                   event_t event)                *
*   event_t async_work_group_copy(__local gentype *dst,          *
                                   const __global gentype *src,  *
                                   size_t  num_elements,         *
                                   event_t event)                *
 async_work_group_srided_copy:                                   *
 *   event_t async_work_group_srided_copy(__global gentype *dst, *
                                   const __local gentype *src,   *
                                   size_t  num_elements,         *
                                   event_t event)                *
*   event_t async_work_group_srided_copy(__local gentype *dst,   *
                                   const __global gentype *src,  *
                                   size_t  num_elements,         *
                                   event_t event)                *
wait_group_events:                                               *
*   void wait_group_events (int num_events,  event_t *event_list)*
\*****************************************************************/
defvar AsyncWorkGroupCopyCode_l2g = [{
    __builtin_memcpy((__private char*)(char*)$Arg0VarName, (const __private char*)(const char*)$Arg1VarName, $Arg2VarName*sizeof(*$Arg0VarName));
    return $Arg3VarName;
  }];
defvar AsyncWorkGroupCopyCode_g2l = [{
    __builtin_memcpy((__private char*)(char*)$Arg0VarName, (const __private char*)(const char*)$Arg1VarName, $Arg2VarName*sizeof(*$Arg0VarName));
    return $Arg3VarName;
  }];
defvar AsyncWorkGroupStridedCopyCode_l2g = [{
    for(size_t i=0, j=0; i<$Arg2VarName; i++, j+=$Arg3VarName) {
      $Arg0VarName[j] = $Arg1VarName[i];
    }
    return $Arg4VarName;
  }];
defvar AsyncWorkGroupStridedCopyCode_g2l = [{
    for(size_t i=0, j=0; i<$Arg2VarName; i++, j+=$Arg3VarName) {
      $Arg0VarName[i] = $Arg1VarName[j];
    }
    return $Arg4VarName;
  }];
OclBuiltinImpl async_work_group_copy_l2g_impl  = OclBuiltinImpl<async_work_group_copy_l2g,  async_work_group_copy_l2g.Types, 0, AsyncWorkGroupCopyCode_l2g>;
OclBuiltinImpl async_work_group_copy_g2l_impl  = OclBuiltinImpl<async_work_group_copy_g2l,  async_work_group_copy_g2l.Types, 0, AsyncWorkGroupCopyCode_g2l>;
OclBuiltinImpl async_work_group_strided_copy_l2g_impl  = OclBuiltinImpl<async_work_group_strided_copy_l2g,  async_work_group_strided_copy_l2g.Types, 0, AsyncWorkGroupStridedCopyCode_l2g>;
OclBuiltinImpl async_work_group_strided_copy_g2l_impl = OclBuiltinImpl<async_work_group_strided_copy_g2l, async_work_group_strided_copy_g2l.Types, 0, AsyncWorkGroupStridedCopyCode_g2l>;
OclBuiltinImpl wait_group_events_generic_impl = OclBuiltinImpl<wait_group_events_generic, wait_group_events_generic.Types, 0, [{ }]>;
OclBuiltinImpl wait_group_events_global_impl  = OclBuiltinImpl<wait_group_events_global, wait_group_events_global.Types, 0, [{ }]>;
OclBuiltinImpl wait_group_events_local_impl   = OclBuiltinImpl<wait_group_events_local, wait_group_events_local.Types, 0, [{ }]>;
OclBuiltinImpl wait_group_events_private_impl = OclBuiltinImpl<wait_group_events_private, wait_group_events_private.Types, 0, [{ }]>;


// Helpers for implementation of OpenCL 2.0 work-group functions.
// The OCL optimizer converts calls to these functions to WG-wide loops
// with calls to per-WI variants of these functions.
// The functions below are per-WI variants of work-group functions.

// Type sets used by different per-WI helpers
// All types
list<OclType> work_group_types_all = [v1i8, v4i8, v8i8, v16i8, v32i8, v64i8,
                                      v1u8, v4u8, v8u8, v16u8, v32u8, v64u8,
                                      v1i16, v4i16, v8i16, v16i16, v32i16, v64i16,
                                      v1u16, v4u16, v8u16, v16u16, v32u16, v64u16,
                                      v1i32, v4i32, v8i32, v16i32, v32i32, v64i32,
                                      v1u32, v4u32, v8u32, v16u32, v32u32, v64u32,
                                      v1i64, v4i64, v8i64, v16i64, v32i64, v64i64,
                                      v1u64, v4u64, v8u64, v16u64, v32u64, v64u64,
                                      v1f32, v4f32, v8f32, v16f32, v32f32, v64f32,
                                      v1f64, v4f64, v8f64, v16f64, v32f64, v64f64];
list<OclType> work_group_types_int = [v1i32, v4i32, v8i32, v16i32, v32i32, v64i32,
                                      v1u32, v4u32, v8u32, v16u32, v32u32, v64u32,
                                      v1i64, v4i64, v8i64, v16i64, v32i64, v64i64,
                                      v1u64, v4u64, v8u64, v16u64, v32u64, v64u64];
list<OclType> work_group_types_all_integer = ExpandTypesByVFAndFlatten<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64], [1, 4, 8, 16, 32, 64]>.Tout;
list<OclType> work_group_types_float = [v1f32, v4f32, v8f32, v16f32, v32f32, v64f32,
                                        v1f64, v4f64, v8f64, v16f64, v32f64, v64f64];
list<OclType> work_group_types_i32 = ExpandTypesByVFAndFlatten<[v1i32], [1, 4, 8, 16, 32, 64]>.Tout;
// Scalar types
list<OclType> work_group_scalar_types_all = [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64];
list<OclType> work_group_scalar_types_int = [v1i32, v1u32, v1i64, v1u64];
list<OclType> work_group_scalar_types_float = [v1f32, v1f64];
list<OclType> work_group_scalar_types_all_integer = [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64];
// Vector types
list<OclType> work_group_vector_types_all = [v4i8, v8i8, v16i8, v32i8, v64i8,
                                             v4u8, v8u8, v16u8, v32u8, v64u8,
                                             v4i16, v8i16, v16i16, v32i16, v64i16,
                                             v4u16, v8u16, v16u16, v32u16, v64u16,
                                             v4i32, v8i32, v16i32, v32i32, v64i32,
                                             v4u32, v8u32, v16u32, v32u32, v64u32,
                                             v4i64, v8i64, v16i64, v32i64, v64i64,
                                             v4u64, v8u64, v16u64, v32u64, v64u64,
                                             v4f32, v8f32, v16f32, v32f32, v64f32,
                                             v4f64, v8f64, v16f64, v32f64, v64f64];
list<OclType> work_group_vector_types_all4 =  [v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4i64, v4u64, v4f32, v4f64];
list<OclType> work_group_vector_types_all8 =  [v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8i64, v8u64, v8f32, v8f64];
list<OclType> work_group_vector_types_all16 = [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16i64, v16u64, v16f32, v16f64];
list<OclType> work_group_vector_types_all32 = [v32i8, v32u8, v32i16, v32u16, v32i32, v32u32, v32i64, v32u64, v32f32, v32f64];
list<OclType> work_group_vector_types_all64 = [v64i8, v64u8, v64i16, v64u16, v64i32, v64u32, v64i64, v64u64, v64f32, v64f64];

list<OclType> work_group_vector_types_int = [v4i32, v8i32, v16i32, v32i32, v64i32,
                                             v4u32, v8u32, v16u32, v32u32, v64u32,
                                             v4i64, v8i64, v16i64, v32i64, v64i64,
                                             v4u64, v8u64, v16u64, v32f64, v64f64];
list<OclType> work_group_vector_types_int4 =  [v4i32, v4u32, v4i64, v4u64];
list<OclType> work_group_vector_types_int8 =  [v8i32, v8u32, v8i64, v8u64];
list<OclType> work_group_vector_types_int16 = [v16i32, v16u32, v16i64, v16u64];
list<OclType> work_group_vector_types_int32 = [v32i32, v32u32, v32i64, v32u64];
list<OclType> work_group_vector_types_int64 = [v64i32, v64u32, v64i64, v64u64];

list<OclType> work_group_vector_types_float = [v4f32, v8f32, v16f32,
                                               v4f64, v8f64, v16f64];
list<OclType> work_group_vector_types_float4 =  [v4f32, v4f64];
list<OclType> work_group_vector_types_float8 =  [v8f32, v8f64];
list<OclType> work_group_vector_types_float16 = [v16f32, v16f64];
list<OclType> work_group_vector_types_float32 = [v32f32, v32f64];
list<OclType> work_group_vector_types_float64 = [v64f32, v64f64];

list<OclType> work_group_vector_types_all_i32 = ExpandTypesByVFAndFlatten<[v1i32], [4, 8, 16, 32, 64]>.Tout;
list<OclType> work_group_vector_types_all_integer4 =  [v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4i64, v4u64];
list<OclType> work_group_vector_types_all_integer8 =  [v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8i64, v8u64];
list<OclType> work_group_vector_types_all_integer16 = [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16i64, v16u64];
list<OclType> work_group_vector_types_all_integer32 = [v32i8, v32u8, v32i16, v32u16, v32i32, v32u32, v32i64, v32u64];
list<OclType> work_group_vector_types_all_integer64 = [v64i8, v64u8, v64i16, v64u16, v64i32, v64u32, v64i64, v64u64];
list<OclType> work_group_vector_types_all_integer = ExpandTypesByVFAndFlatten<[v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64], [4, 8, 16, 32, 64]>.Tout;

// Fake implementations of WG functions - in order to keep their declarations in BI LLVM module
OclBuiltinImpl work_group_all_fake = OclBuiltinImpl<work_group_all, [ v1i32, v4i32, v8i32, v16i32, v32i32, v64i32 ], 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_any_fake = OclBuiltinImpl<work_group_any, [ v1i32, v4i32, v8i32, v16i32, v32i32, v64i32 ], 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_broadcast1d_fake = OclBuiltinImpl<work_group_broadcast1d, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_broadcast2d_fake = OclBuiltinImpl<work_group_broadcast2d, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_broadcast3d_fake = OclBuiltinImpl<work_group_broadcast3d, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_reduce_bitwise_and_fake = OclBuiltinImpl<work_group_reduce_bitwise_and, work_group_types_all_integer, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_reduce_bitwise_or_fake = OclBuiltinImpl<work_group_reduce_bitwise_or, work_group_types_all_integer, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_reduce_bitwise_xor_fake = OclBuiltinImpl<work_group_reduce_bitwise_xor, work_group_types_all_integer, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_reduce_logical_and_fake = OclBuiltinImpl<work_group_reduce_logical_and, work_group_types_i32, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_reduce_logical_or_fake = OclBuiltinImpl<work_group_reduce_logical_or, work_group_types_i32, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_reduce_logical_xor_fake = OclBuiltinImpl<work_group_reduce_logical_xor, work_group_types_i32, 0,
  [{
    return $Arg0VarName;
  }]>;

OclBuiltinImpl work_group_all_mask_fake = OclBuiltinImpl<work_group_all_mask, [ v1i32, v4i32, v8i32, v16i32, v32i32, v64i32 ], 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_any_mask_fake = OclBuiltinImpl<work_group_any_mask, [ v1i32, v4i32, v8i32, v16i32, v32i32, v64i32 ], 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_broadcast1d_mask_fake = OclBuiltinImpl<work_group_broadcast1d_mask, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_broadcast2d_mask_fake = OclBuiltinImpl<work_group_broadcast2d_mask, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_broadcast3d_mask_fake = OclBuiltinImpl<work_group_broadcast3d_mask, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;

// Functions for and/or operations (in order to have all per-WI helper functions uniformal)
OclBuiltinImpl work_group_bitwise_and_helper_util__generic = OclBuiltinImpl<work_group_bitwise_and_helper_util, work_group_types_all_integer, 0,
  [{
    return $Arg0VarName & $Arg1VarName;
  }]>;
OclBuiltinImpl work_group_bitwise_or_helper_util__generic = OclBuiltinImpl<work_group_bitwise_or_helper_util, work_group_types_all_integer, 0,
  [{
    return $Arg0VarName | $Arg1VarName;
  }]>;
OclBuiltinImpl work_group_bitwise_xor_helper_util__generic = OclBuiltinImpl<work_group_bitwise_xor_helper_util, work_group_types_all_integer, 0,
  [{
    return $Arg0VarName ^ $Arg1VarName;
  }]>;
OclBuiltinImpl work_group_logical_and_helper_util__generic = OclBuiltinImpl<work_group_logical_and_helper_util, work_group_types_i32, 0,
  [{
    return $Arg0VarName && $Arg1VarName;
  }]>;
OclBuiltinImpl work_group_logical_or_helper_util__generic = OclBuiltinImpl<work_group_logical_or_helper_util, work_group_types_i32, 0,
  [{
    return $Arg0VarName || $Arg1VarName;
  }]>;
OclBuiltinImpl work_group_logical_xor_helper_util__generic = OclBuiltinImpl<work_group_logical_xor_helper_util, work_group_types_i32, 0,
  [{
    return $Arg0VarName != $Arg1VarName;
  }]>;
OclBuiltinImpl work_group_all_helper_util_int = OclBuiltinImpl<work_group_all_helper_util, [v1i32, v4i32, v8i32, v16i32, v32i32, v64i32], 0,
  [{
    return $Arg0VarName & $Arg1VarName;
  }]>;
OclBuiltinImpl work_group_any_helper_util_int = OclBuiltinImpl<work_group_any_helper_util, [v1i32, v4i32, v8i32, v16i32, v32i32, v64i32], 0,
  [{
    return $Arg0VarName | $Arg1VarName;
  }]>;


//-----------------------------------------------
// 1. Uniform result
//-----------------------------------------------

// work_group_all
OclBuiltinImpl work_group_all_vector_helper_int = OclBuiltinImpl<work_group_all_helper, [v4i32, v8i32, v16i32, v32i32, v64i32], 0, work_group_inclusive_function_op_vector, [["$FUNC", "work_group_all_util"]]>;
OclBuiltinImpl work_group_all_scalar_helper_int = OclBuiltinImpl<work_group_all_helper, [v1i32], 0, work_group_inclusive_function_op_scalar, [["$FUNC", "work_group_all_util"]]>;
OclBuiltinImpl work_group_all_mask_helper_int = OclBuiltinImpl<work_group_all_helper_mask, [v4i32, v8i32, v16i32, v32i32, v64i32], 0, work_group_inclusive_function_op_mask, [["$FUNC", "work_group_all_util"], ["$VAL", "1"]]>;

// work_group_any
OclBuiltinImpl work_group_any_vector_helper_int = OclBuiltinImpl<work_group_any_helper, [v4i32, v8i32, v16i32, v32i32, v64i32], 0, work_group_inclusive_function_op_vector, [["$FUNC", "work_group_any_util"]]>;
OclBuiltinImpl work_group_any_scalar_helper_int = OclBuiltinImpl<work_group_any_helper, [v1i32], 0, work_group_inclusive_function_op_scalar, [["$FUNC", "work_group_any_util"]]>;
OclBuiltinImpl work_group_any_mask_helper_int = OclBuiltinImpl<work_group_any_helper_mask, [v4i32, v8i32, v16i32, v32i32, v64i32], 0, work_group_inclusive_function_op_mask, [["$FUNC", "work_group_any_util"], ["$VAL", "0"]]>;

// work_group_reduce_bitwise_and
OclBuiltinImpl work_group_reduce_bitwise_and_vector_helper_generic = OclBuiltinImpl<work_group_reduce_bitwise_and_helper, work_group_vector_types_all_integer, 0, work_group_inclusive_function_op_vector, [["$FUNC", "work_group_bitwise_and_util"]]>;
OclBuiltinImpl work_group_reduce_bitwise_and_scalar_helper_generic = OclBuiltinImpl<work_group_reduce_bitwise_and_helper, work_group_scalar_types_all_integer, 0, work_group_inclusive_function_op_scalar, [["$FUNC", "work_group_bitwise_and_util"]]>;

// work_group_reduce_bitwise_or
OclBuiltinImpl work_group_reduce_bitwise_or_vector_helper_generic = OclBuiltinImpl<work_group_reduce_bitwise_or_helper, work_group_vector_types_all_integer, 0, work_group_inclusive_function_op_vector, [["$FUNC", "work_group_bitwise_or_util"]]>;
OclBuiltinImpl work_group_reduce_bitwise_or_scalar_helper_generic = OclBuiltinImpl<work_group_reduce_bitwise_or_helper, work_group_scalar_types_all_integer, 0, work_group_inclusive_function_op_scalar, [["$FUNC", "work_group_bitwise_or_util"]]>;

// work_group_reduce_bitwise_xor
OclBuiltinImpl work_group_reduce_bitwise_xor_vector_helper_generic = OclBuiltinImpl<work_group_reduce_bitwise_xor_helper, work_group_vector_types_all_integer, 0, work_group_inclusive_function_op_vector, [["$FUNC", "work_group_bitwise_xor_util"]]>;
OclBuiltinImpl work_group_reduce_bitwise_xor_scalar_helper_generic = OclBuiltinImpl<work_group_reduce_bitwise_xor_helper, work_group_scalar_types_all_integer, 0, work_group_inclusive_function_op_scalar, [["$FUNC", "work_group_bitwise_xor_util"]]>;

// work_group_reduce_logical_and
OclBuiltinImpl work_group_reduce_logical_and_vector_helper_generic = OclBuiltinImpl<work_group_reduce_logical_and_helper, work_group_vector_types_all_i32, 0, work_group_inclusive_function_op_vector, [["$FUNC", "work_group_logical_and_util"]]>;
OclBuiltinImpl work_group_reduce_logical_and_scalar_helper_generic = OclBuiltinImpl<work_group_reduce_logical_and_helper, [v1i32], 0, work_group_inclusive_function_op_scalar, [["$FUNC", "work_group_logical_and_util"]]>;

// work_group_reduce_logical_or
OclBuiltinImpl work_group_reduce_logical_or_vector_helper_generic = OclBuiltinImpl<work_group_reduce_logical_or_helper, work_group_vector_types_all_i32, 0, work_group_inclusive_function_op_vector, [["$FUNC", "work_group_logical_or_util"]]>;
OclBuiltinImpl work_group_reduce_logical_or_scalar_helper_generic = OclBuiltinImpl<work_group_reduce_logical_or_helper, [v1i32], 0, work_group_inclusive_function_op_scalar, [["$FUNC", "work_group_logical_or_util"]]>;

// work_group_reduce_logical_xor
OclBuiltinImpl work_group_reduce_logical_xor_vector_helper_generic = OclBuiltinImpl<work_group_reduce_logical_xor_helper, work_group_vector_types_all_i32, 0, work_group_inclusive_function_op_vector, [["$FUNC", "work_group_logical_xor_util"]]>;
OclBuiltinImpl work_group_reduce_logical_xor_scalar_helper_generic = OclBuiltinImpl<work_group_reduce_logical_xor_helper, [v1i32], 0, work_group_inclusive_function_op_scalar, [["$FUNC", "work_group_logical_xor_util"]]>;

// work_group_broadcast_scalar
OclBuiltinImpl work_group_broadcast_scalar_helper = OclBuiltinImpl<work_group_broadcast_helper, work_group_scalar_types_all, 0, [{
    if ($Arg2VarName == $Arg1VarName) {
      *$Arg3VarName = $Arg0VarName;
      return $Arg0VarName;
    } else {
      return *$Arg3VarName;
    }
  }]>;

// work_group_broadcast_vector
OclBuiltinImpl work_group_broadcast_vector_helper = OclBuiltinImpl<work_group_broadcast_helper, [v4i8, v8i8, v16i8, v4u8, v8u8, v16u8, v4i16, v8i16, v16i16, v4u16, v8u16, v16u16, v4i32, v8i32, v16i32, v4u32, v8u32, v16u32, v4i64, v8i64, v16i64, v4u64, v8u64, v16u64, v4f32, v8f32, v16f32, v4f64, v8f64, v16f64], 0, [{
   if ($Arg2VarName <= $Arg1VarName && $Arg1VarName < $Arg2VarName + vec_step($Arg0VarName)) {
      uint offset = $Arg1VarName - $Arg2VarName;
      $Arg0ugentype _mask = ($Arg0ugentype)(offset);
      $Arg0VecType acc = shuffle($Arg0VarName, _mask);
      *$Arg3VarName = acc;
      return acc;
    } else {
      return *$Arg3VarName;
    }
  }]>;

OclBuiltinImpl work_group_broadcast_vector_helper_32 = OclBuiltinImpl<work_group_broadcast_helper, work_group_vector_types_all32, 0, [{
   if ($Arg2VarName <= $Arg1VarName && $Arg1VarName < $Arg2VarName + vec_step($Arg0VarName)) {
      uint offset = $Arg1VarName - $Arg2VarName;
      $Arg0ugentype _mask = ($Arg0ugentype)(offset);
      $Arg0VecType acc;
      acc.lo = shuffle($Arg0VarName.lo, _mask.lo);
      acc.hi = shuffle($Arg0VarName.hi, _mask.hi);
      *$Arg3VarName = acc;
      return acc;
    } else {
      return *$Arg3VarName;
    }
  }]>;

OclBuiltinImpl work_group_broadcast_vector_helper_64 = OclBuiltinImpl<work_group_broadcast_helper, work_group_vector_types_all64, 0, [{
   if ($Arg2VarName <= $Arg1VarName && $Arg1VarName < $Arg2VarName + vec_step($Arg0VarName)) {
      uint offset = $Arg1VarName - $Arg2VarName;
      $Arg0ugentype _mask = ($Arg0ugentype)(offset);
      $Arg0VecType acc;
      acc.lo.lo = shuffle($Arg0VarName.lo.lo, _mask.lo.lo);
      acc.lo.hi = shuffle($Arg0VarName.lo.hi, _mask.lo.hi);
      acc.hi.lo = shuffle($Arg0VarName.hi.lo, _mask.hi.lo);
      acc.hi.hi = shuffle($Arg0VarName.hi.hi, _mask.hi.hi);

      *$Arg3VarName = acc;
      return acc;
    } else {
      return *$Arg3VarName;
    }
  }]>;

// Masked work_group_broadcast_vector
OclBuiltinImpl work_group_broadcast_mask_helper = OclBuiltinImpl<work_group_broadcast_helper_mask, [v4i8, v8i8, v16i8, v4u8, v8u8, v16u8, v4i16, v8i16, v16i16, v4u16, v8u16, v16u16, v4i32, v8i32, v16i32, v4u32, v8u32, v16u32, v4i64, v8i64, v16i64, v4u64, v8u64, v16u64, v4f32, v8f32, v16f32, v4f64, v8f64, v16f64], 0, [{
   // No need to check the mask here
   if ($Arg2VarName <= $Arg1VarName && $Arg1VarName < $Arg2VarName + vec_step($Arg0VarName)) {
      uint offset = $Arg1VarName - $Arg2VarName;
      $Arg0ugentype _mask = ($Arg0ugentype)(offset);
      $Arg0VecType acc = shuffle($Arg0VarName, _mask);
      *$Arg4VarName = acc;
      return acc;
    } else {
      return *$Arg4VarName;
    }
  }]>;

OclBuiltinImpl work_group_broadcast_mask_helper_32 = OclBuiltinImpl<work_group_broadcast_helper_mask, work_group_vector_types_all32, 0, [{
   if ($Arg2VarName <= $Arg1VarName && $Arg1VarName < $Arg2VarName + vec_step($Arg0VarName)) {
      uint offset = $Arg1VarName - $Arg2VarName;
      $Arg0ugentype _mask = ($Arg0ugentype)(offset);
      $Arg0VecType acc;
      acc.lo = shuffle($Arg0VarName.lo, _mask.lo);
      acc.hi = shuffle($Arg0VarName.hi, _mask.hi);
      *$Arg4VarName = acc;
      return acc;
    } else {
      return *$Arg4VarName;
    }
  }]>;

OclBuiltinImpl work_group_broadcast_mask_helper_64 = OclBuiltinImpl<work_group_broadcast_helper_mask, work_group_vector_types_all64, 0, [{
   if ($Arg2VarName <= $Arg1VarName && $Arg1VarName < $Arg2VarName + vec_step($Arg0VarName)) {
      uint offset = $Arg1VarName - $Arg2VarName;
      $Arg0ugentype _mask = ($Arg0ugentype)(offset);
      $Arg0VecType acc;
      acc.lo.lo = shuffle($Arg0VarName.lo.lo, _mask.lo.lo);
      acc.lo.hi = shuffle($Arg0VarName.lo.hi, _mask.lo.hi);
      acc.hi.lo = shuffle($Arg0VarName.hi.lo, _mask.hi.lo);
      acc.hi.hi = shuffle($Arg0VarName.hi.hi, _mask.hi.hi);

      *$Arg4VarName = acc;
      return acc;
    } else {
      return *$Arg4VarName;
    }
  }]>;

// Definitions of finalization functions
OclBuiltinImpl work_group_all_finalize_helper_int4 =  OclBuiltinImpl<work_group_all_finalize_helper,   [v4i32],   0, work_group_finalize_function_op4, [["$FUNC","work_group_all_util"]]>;
OclBuiltinImpl work_group_all_finalize_helper_int8 =  OclBuiltinImpl<work_group_all_finalize_helper,   [v8i32],   0, work_group_finalize_function_op8, [["$FUNC","work_group_all_util"]]>;
OclBuiltinImpl work_group_all_finalize_helper_int16 = OclBuiltinImpl<work_group_all_finalize_helper,   [v16i32],  0, work_group_finalize_function_op16, [["$FUNC","work_group_all_util"]]>;
OclBuiltinImpl work_group_all_finalize_helper_int32 = OclBuiltinImpl<work_group_all_finalize_helper,   [v32i32],  0, work_group_finalize_function_op32, [["$FUNC","work_group_all_util"]]>;
OclBuiltinImpl work_group_all_finalize_helper_int64 = OclBuiltinImpl<work_group_all_finalize_helper,   [v64i32],  0, work_group_finalize_function_op64, [["$FUNC","work_group_all_util"]]>;

OclBuiltinImpl work_group_any_finalize_helper_int4 =  OclBuiltinImpl<work_group_any_finalize_helper,   [v4i32],   0, work_group_finalize_function_op4, [["$FUNC","work_group_any_util"]]>;
OclBuiltinImpl work_group_any_finalize_helper_int8 =  OclBuiltinImpl<work_group_any_finalize_helper,   [v8i32],   0, work_group_finalize_function_op8, [["$FUNC","work_group_any_util"]]>;
OclBuiltinImpl work_group_any_finalize_helper_int16 = OclBuiltinImpl<work_group_any_finalize_helper,   [v16i32],  0, work_group_finalize_function_op16, [["$FUNC","work_group_any_util"]]>;
OclBuiltinImpl work_group_any_finalize_helper_int32 = OclBuiltinImpl<work_group_any_finalize_helper,   [v32i32],  0, work_group_finalize_function_op32, [["$FUNC","work_group_any_util"]]>;
OclBuiltinImpl work_group_any_finalize_helper_int64 = OclBuiltinImpl<work_group_any_finalize_helper,   [v64i32],  0, work_group_finalize_function_op64, [["$FUNC","work_group_any_util"]]>;

// Definitions of finalization functions with a dummy mask.
OclBuiltinImpl work_group_all_finalize_helper_mask_int4 =  OclBuiltinImpl<work_group_all_finalize_helper_mask, [v4i32],   0, work_group_finalize_function_op4, [["$FUNC","work_group_all_util"]]>;
OclBuiltinImpl work_group_all_finalize_helper_mask_int8 =  OclBuiltinImpl<work_group_all_finalize_helper_mask, [v8i32],   0, work_group_finalize_function_op8, [["$FUNC","work_group_all_util"]]>;
OclBuiltinImpl work_group_all_finalize_helper_mask_int16 = OclBuiltinImpl<work_group_all_finalize_helper_mask, [v16i32],  0, work_group_finalize_function_op16, [["$FUNC","work_group_all_util"]]>;
OclBuiltinImpl work_group_all_finalize_helper_mask_int32 = OclBuiltinImpl<work_group_all_finalize_helper_mask, [v32i32],  0, work_group_finalize_function_op32, [["$FUNC","work_group_all_util"]]>;
OclBuiltinImpl work_group_all_finalize_helper_mask_int64 = OclBuiltinImpl<work_group_all_finalize_helper_mask, [v64i32],  0, work_group_finalize_function_op64, [["$FUNC","work_group_all_util"]]>;

OclBuiltinImpl work_group_any_finalize_helper_mask_int4 =  OclBuiltinImpl<work_group_any_finalize_helper_mask, [v4i32],   0, work_group_finalize_function_op4, [["$FUNC","work_group_any_util"]]>;
OclBuiltinImpl work_group_any_finalize_helper_mask_int8 =  OclBuiltinImpl<work_group_any_finalize_helper_mask, [v8i32],   0, work_group_finalize_function_op8, [["$FUNC","work_group_any_util"]]>;
OclBuiltinImpl work_group_any_finalize_helper_mask_int16 = OclBuiltinImpl<work_group_any_finalize_helper_mask, [v16i32],  0, work_group_finalize_function_op16, [["$FUNC","work_group_any_util"]]>;
OclBuiltinImpl work_group_any_finalize_helper_mask_int32 = OclBuiltinImpl<work_group_any_finalize_helper_mask, [v32i32],  0, work_group_finalize_function_op32, [["$FUNC","work_group_any_util"]]>;
OclBuiltinImpl work_group_any_finalize_helper_mask_int64 = OclBuiltinImpl<work_group_any_finalize_helper_mask, [v64i32],  0, work_group_finalize_function_op64, [["$FUNC","work_group_any_util"]]>;
