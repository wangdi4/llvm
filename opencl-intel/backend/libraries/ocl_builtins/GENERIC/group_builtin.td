// Copyright (c) 2006-2012 Intel Corporation
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
code AsyncWorkGroupCopyCode_l2g = [{
    __builtin_memcpy((__private char*)(char*)$Arg0VarName, (const __private char*)(const char*)$Arg1VarName, $Arg2VarName*sizeof(*$Arg0VarName));
    return $Arg3VarName;
  }];
code AsyncWorkGroupCopyCode_g2l = [{
    __builtin_memcpy((__private char*)(char*)$Arg0VarName, (const __private char*)(const char*)$Arg1VarName, $Arg2VarName*sizeof(*$Arg0VarName));
    return $Arg3VarName;
  }];
code AsyncWorkGroupStridedCopyCode_l2g = [{
    for(size_t i=0, j=0; i<$Arg2VarName; i++, j+=$Arg3VarName) {
      $Arg0VarName[j] = $Arg1VarName[i];
    }
    return $Arg4VarName;
  }];
code AsyncWorkGroupStridedCopyCode_g2l = [{
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
list<OclType> work_group_types_all = [v1i32, v4i32, v8i32, v16i32,
                                      v1u32, v4u32, v8u32, v16u32,
                                      v1i64, v4i64, v8i64, v16i64,
                                      v1u64, v4u64, v8u64, v16u64,
                                      v1f32, v4f32, v8f32, v16f32,
                                      v1f64, v4f64, v8f64, v16f64];
list<OclType> work_group_types_int = [v1i32, v4i32, v8i32, v16i32,
                                      v1u32, v4u32, v8u32, v16u32,
                                      v1i64, v4i64, v8i64, v16i64,
                                      v1u64, v4u64, v8u64, v16u64];
list<OclType> work_group_types_float = [v1f32, v4f32, v8f32, v16f32,
                                        v1f64, v4f64, v8f64, v16f64];
// Scalar types
list<OclType> work_group_scalar_types_all = [v1i32, v1u32, v1i64, v1u64, v1f32, v1f64];
list<OclType> work_group_scalar_types_int = [v1i32, v1u32, v1i64, v1u64];
list<OclType> work_group_scalar_types_float = [v1f32, v1f64];
// Vector types
list<OclType> work_group_vector_types_all = [v4i32, v8i32, v16i32,
                                             v4u32, v8u32, v16u32,
                                             v4i64, v8i64, v16i64,
                                             v4u64, v8u64, v16u64,
                                             v4f32, v8f32, v16f32,
                                             v4f64, v8f64, v16f64];
list<OclType> work_group_vector_types_all4 =  [v4i32, v4u32, v4i64, v4u64, v4f32, v4f64];
list<OclType> work_group_vector_types_all8 =  [v8i32, v8u32, v8i64, v8u64, v8f32, v8f64];
list<OclType> work_group_vector_types_all16 = [v16i32, v16u32, v16i64, v16u64, v16f32, v16f64];
list<OclType> work_group_vector_types_int = [v4i32, v8i32, v16i32,
                                             v4u32, v8u32, v16u32,
                                             v4i64, v8i64, v16i64,
                                             v4u64, v8u64, v16u64];
list<OclType> work_group_vector_types_int4 =  [v4i32, v4u32, v4i64, v4u64];
list<OclType> work_group_vector_types_int8 =  [v8i32, v8u32, v8i64, v8u64];
list<OclType> work_group_vector_types_int16 = [v16i32, v16u32, v16i64, v16u64];
list<OclType> work_group_vector_types_float = [v4f32, v8f32, v16f32,
                                               v4f64, v8f64, v16f64];
list<OclType> work_group_vector_types_float4 =  [v4f32, v4f64];
list<OclType> work_group_vector_types_float8 =  [v8f32, v8f64];
list<OclType> work_group_vector_types_float16 = [v16f32, v16f64];

// Fake implementations of WG functions - in order to keep their declarations in BI LLVM module
OclBuiltinImpl work_group_all_fake = OclBuiltinImpl<work_group_all, [ v1i32,  v4i32,  v8i32,  v16i32 ], 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_any_fake = OclBuiltinImpl<work_group_any, [ v1i32,  v4i32,  v8i32,  v16i32 ], 0,
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
OclBuiltinImpl work_group_reduce_add_fake = OclBuiltinImpl<work_group_reduce_add, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_reduce_min_fake = OclBuiltinImpl<work_group_reduce_min, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_reduce_max_fake = OclBuiltinImpl<work_group_reduce_max, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_scan_exclusive_add_fake = OclBuiltinImpl<work_group_scan_exclusive_add, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_scan_exclusive_min_fake = OclBuiltinImpl<work_group_scan_exclusive_min, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_scan_exclusive_max_fake = OclBuiltinImpl<work_group_scan_exclusive_max, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_scan_inclusive_add_fake = OclBuiltinImpl<work_group_scan_inclusive_add, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_scan_inclusive_min_fake = OclBuiltinImpl<work_group_scan_inclusive_min, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_scan_inclusive_max_fake = OclBuiltinImpl<work_group_scan_inclusive_max, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;

OclBuiltinImpl work_group_all_mask_fake = OclBuiltinImpl<work_group_all_mask, [ v1i32,  v4i32,  v8i32,  v16i32 ], 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_any_mask_fake = OclBuiltinImpl<work_group_any_mask, [ v1i32,  v4i32,  v8i32,  v16i32 ], 0,
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
OclBuiltinImpl work_group_reduce_add_mask_fake = OclBuiltinImpl<work_group_reduce_add_mask, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_reduce_min_mask_fake = OclBuiltinImpl<work_group_reduce_min_mask, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_reduce_max_mask_fake = OclBuiltinImpl<work_group_reduce_max_mask, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_scan_exclusive_add_mask_fake = OclBuiltinImpl<work_group_scan_exclusive_add_mask, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_fake = OclBuiltinImpl<work_group_scan_exclusive_min_mask, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_fake = OclBuiltinImpl<work_group_scan_exclusive_max_mask, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_scan_inclusive_add_mask_fake = OclBuiltinImpl<work_group_scan_inclusive_add_mask, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_fake = OclBuiltinImpl<work_group_scan_inclusive_min_mask, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_fake = OclBuiltinImpl<work_group_scan_inclusive_max_mask, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;

// Functions for addition/and/or operations (in order to have all per-WI helper functions uniformal)
OclBuiltinImpl work_group_add_helper_util__generic = OclBuiltinImpl<work_group_add_helper_util, work_group_types_all, 0,
  [{
    return $Arg0VarName + $Arg1VarName;
  }]>;
OclBuiltinImpl work_group_all_helper_util_int = OclBuiltinImpl<work_group_all_helper_util, [v1i32, v4i32, v8i32, v16i32], 0,
  [{
    return $Arg0VarName & $Arg1VarName;
  }]>;
OclBuiltinImpl work_group_any_helper_util_int = OclBuiltinImpl<work_group_any_helper_util, [v1i32, v4i32, v8i32, v16i32], 0,
  [{
    return $Arg0VarName | $Arg1VarName;
  }]>;


//-----------------------------------------------
// 1. Uniform result
//-----------------------------------------------

code work_group_inclusive_function_op_vector = [{
    $Arg0Type acc = $FUNC($Arg0VarName, *$Arg1VarName);
    *$Arg1VarName = acc;
    return acc;
  }];

code work_group_inclusive_function_op_scalar = [{
    $Arg0Type acc = $FUNC($Arg0VarName, *$Arg1VarName);
    *$Arg1VarName = acc;
    return acc;
  }];

code work_group_inclusive_function_op_mask = [{
    $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
    $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
    arg0 &= arg1;
    arg1 = ~arg1 & ($MaskCastTy)$VAL;
    arg0 |= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0Type acc = $FUNC($Arg0VarName, *$Arg2VarName);
    *$Arg2VarName = acc;
    return acc;
  }];

// work_group_all
OclBuiltinImpl work_group_all_vector_helper_int = OclBuiltinImpl<work_group_all_helper, [v4i32, v8i32, v16i32], 0, work_group_inclusive_function_op_vector, [["$FUNC", "work_group_all_util"]]>;
OclBuiltinImpl work_group_all_scalar_helper_int = OclBuiltinImpl<work_group_all_helper, [v1i32], 0, work_group_inclusive_function_op_scalar, [["$FUNC", "work_group_all_util"]]>;
OclBuiltinImpl work_group_all_mask_helper_int = OclBuiltinImpl<work_group_all_helper_mask, [v4i32, v8i32, v16i32], 0, work_group_inclusive_function_op_mask, [["$FUNC", "work_group_all_util"], ["$VAL", "1"]]>;

// work_group_any
OclBuiltinImpl work_group_any_vector_helper_int = OclBuiltinImpl<work_group_any_helper, [v4i32, v8i32, v16i32], 0, work_group_inclusive_function_op_vector, [["$FUNC", "work_group_any_util"]]>;
OclBuiltinImpl work_group_any_scalar_helper_int = OclBuiltinImpl<work_group_any_helper, [v1i32], 0, work_group_inclusive_function_op_scalar, [["$FUNC", "work_group_any_util"]]>;
OclBuiltinImpl work_group_any_mask_helper_int = OclBuiltinImpl<work_group_any_helper_mask, [v4i32, v8i32, v16i32], 0, work_group_inclusive_function_op_mask, [["$FUNC", "work_group_any_util"], ["$VAL", "0"]]>;

// work_group_reduce_add
OclBuiltinImpl work_group_reduce_add_vector_helper_generic = OclBuiltinImpl<work_group_reduce_add_helper, work_group_vector_types_all, 0, work_group_inclusive_function_op_vector, [["$FUNC", "work_group_add_util"]]>;
OclBuiltinImpl work_group_reduce_add_scalar_helper_generic = OclBuiltinImpl<work_group_reduce_add_helper, work_group_scalar_types_all, 0, work_group_inclusive_function_op_scalar, [["$FUNC", "work_group_add_util"]]>;

// work_group_reduce_max/min
OclBuiltinImpl work_group_reduce_min_vector_helper_int = OclBuiltinImpl<work_group_reduce_min_helper, work_group_vector_types_int, 0, work_group_inclusive_function_op_vector, [["$FUNC", "min"]]>;
OclBuiltinImpl work_group_reduce_min_scalar_helper_int = OclBuiltinImpl<work_group_reduce_min_helper, work_group_scalar_types_int, 0, work_group_inclusive_function_op_scalar, [["$FUNC", "min"]]>;
OclBuiltinImpl work_group_reduce_min_vector_helper_float = OclBuiltinImpl<work_group_reduce_min_helper, work_group_vector_types_float, 0, work_group_inclusive_function_op_vector, [["$FUNC", "fmin"]]>;
OclBuiltinImpl work_group_reduce_min_scalar_helper_float = OclBuiltinImpl<work_group_reduce_min_helper, work_group_scalar_types_float, 0, work_group_inclusive_function_op_scalar, [["$FUNC", "fmin"]]>;
OclBuiltinImpl work_group_reduce_max_vector_helper_int = OclBuiltinImpl<work_group_reduce_max_helper, work_group_vector_types_int, 0, work_group_inclusive_function_op_vector, [["$FUNC", "max"]]>;
OclBuiltinImpl work_group_reduce_max_scalar_helper_int = OclBuiltinImpl<work_group_reduce_max_helper, work_group_scalar_types_int, 0, work_group_inclusive_function_op_scalar, [["$FUNC", "max"]]>;
OclBuiltinImpl work_group_reduce_max_vector_helper_float = OclBuiltinImpl<work_group_reduce_max_helper, work_group_vector_types_float, 0, work_group_inclusive_function_op_vector, [["$FUNC", "fmax"]]>;
OclBuiltinImpl work_group_reduce_max_scalar_helper_float = OclBuiltinImpl<work_group_reduce_max_helper, work_group_scalar_types_float, 0, work_group_inclusive_function_op_scalar, [["$FUNC", "fmax"]]>;

// Masked work group reduce add
OclBuiltinImpl work_group_reduce_add_mask_helper_generic = OclBuiltinImpl<work_group_reduce_add_helper_mask, work_group_vector_types_all, 0, work_group_inclusive_function_op_mask, [["$FUNC", "work_group_add_util"], ["$VAL", "0"]]>;

// Masked work group reduce max/min
OclBuiltinImpl work_group_reduce_min_mask_helper_i32 = OclBuiltinImpl<work_group_reduce_min_helper_mask, [v4i32, v8i32, v16i32], 0, work_group_inclusive_function_op_mask, [["$FUNC", "min"],["$VAL", "INT_MAX"]]>;
OclBuiltinImpl work_group_reduce_min_mask_helper_u32 = OclBuiltinImpl<work_group_reduce_min_helper_mask, [v4u32, v8u32, v16u32], 0, work_group_inclusive_function_op_mask, [["$FUNC", "min"],["$VAL", "UINT_MAX"]]>;
OclBuiltinImpl work_group_reduce_min_mask_helper_i64 = OclBuiltinImpl<work_group_reduce_min_helper_mask, [v4i64, v8i64, v16i64], 0, work_group_inclusive_function_op_mask, [["$FUNC", "min"],["$VAL", "LONG_MAX"]]>;
OclBuiltinImpl work_group_reduce_min_mask_helper_u64 = OclBuiltinImpl<work_group_reduce_min_helper_mask, [v4u64, v8u64, v16u64], 0, work_group_inclusive_function_op_mask, [["$FUNC", "min"],["$VAL", "ULONG_MAX"]]>;
OclBuiltinImpl work_group_reduce_min_mask_helper_f32 = OclBuiltinImpl<work_group_reduce_min_helper_mask, [v4f32, v8f32, v16f32], 0, work_group_inclusive_function_op_mask, [["$FUNC", "fmin"], ["$VAL", "INFINITY"]]>;
OclBuiltinImpl work_group_reduce_min_mask_helper_f64 = OclBuiltinImpl<work_group_reduce_min_helper_mask, [v4f64, v8f64, v16f64], 0, work_group_inclusive_function_op_mask, [["$FUNC", "fmin"], ["$VAL", "INFINITY"]]>;

OclBuiltinImpl work_group_reduce_max_mask_helper_i32 = OclBuiltinImpl<work_group_reduce_max_helper_mask, [v4i32, v8i32, v16i32], 0, work_group_inclusive_function_op_mask, [["$FUNC", "max"],["$VAL", "INT_MIN"]]>;
OclBuiltinImpl work_group_reduce_max_mask_helper_u32 = OclBuiltinImpl<work_group_reduce_max_helper_mask, [v4u32, v8u32, v16u32], 0, work_group_inclusive_function_op_mask, [["$FUNC", "max"],["$VAL", "0"]]>;
OclBuiltinImpl work_group_reduce_max_mask_helper_i64 = OclBuiltinImpl<work_group_reduce_max_helper_mask, [v4i64, v8i64, v16i64], 0, work_group_inclusive_function_op_mask, [["$FUNC", "max"],["$VAL", "LONG_MIN"]]>;
OclBuiltinImpl work_group_reduce_max_mask_helper_u64 = OclBuiltinImpl<work_group_reduce_max_helper_mask, [v4u64, v8u64, v16u64], 0, work_group_inclusive_function_op_mask, [["$FUNC", "max"],["$VAL", "0"]]>;
OclBuiltinImpl work_group_reduce_max_mask_helper_f32 = OclBuiltinImpl<work_group_reduce_max_helper_mask, [v4f32, v8f32, v16f32], 0, work_group_inclusive_function_op_mask, [["$FUNC", "fmax"], ["$VAL", "-INFINITY"]]>;
OclBuiltinImpl work_group_reduce_max_mask_helper_f64 = OclBuiltinImpl<work_group_reduce_max_helper_mask, [v4f64, v8f64, v16f64], 0, work_group_inclusive_function_op_mask, [["$FUNC", "fmax"], ["$VAL", "-INFINITY"]]>;


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
OclBuiltinImpl work_group_broadcast_vector_helper = OclBuiltinImpl<work_group_broadcast_helper, work_group_vector_types_all, 0, [{
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

// Masked work_group_broadcast_vector
OclBuiltinImpl work_group_broadcast_mask_helper = OclBuiltinImpl<work_group_broadcast_helper_mask, work_group_vector_types_all, 0, [{
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

//-----------------------------------------------
// 2. Non-uniform result
//-----------------------------------------------

code work_group_scan_exclusive_function_op_vector4 = [{
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg1VarName;
    ret.s0 = acc.s3;
    ret.s1 = acc.s0 = $FUNC($Arg0VarName.s0, ret.s0);
    ret.s2 = acc.s1 = $FUNC($Arg0VarName.s1, ret.s1);
    ret.s3 = acc.s2 = $FUNC($Arg0VarName.s2, ret.s2);
    acc.s3 = $FUNC($Arg0VarName.s3, ret.s3);
    *$Arg1VarName = acc;
    return ret;
  }];

code work_group_scan_exclusive_function_op_vector8 = [{
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg1VarName;
    ret.s0 = acc.s7;
    ret.s1 = acc.s0 = $FUNC($Arg0VarName.s0, ret.s0);
    ret.s2 = acc.s1 = $FUNC($Arg0VarName.s1, ret.s1);
    ret.s3 = acc.s2 = $FUNC($Arg0VarName.s2, ret.s2);
    ret.s4 = acc.s3 = $FUNC($Arg0VarName.s3, ret.s3);
    ret.s5 = acc.s4 = $FUNC($Arg0VarName.s4, ret.s4);
    ret.s6 = acc.s5 = $FUNC($Arg0VarName.s5, ret.s5);
    ret.s7 = acc.s6 = $FUNC($Arg0VarName.s6, ret.s6);
    acc.s7 = $FUNC($Arg0VarName.s7, ret.s7);
    *$Arg1VarName = acc;
    return ret;
  }];

code work_group_scan_exclusive_function_op_vector16 = [{
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg1VarName;
    ret.s0 = acc.sF;
    ret.s1 = acc.s0 = $FUNC($Arg0VarName.s0, ret.s0);
    ret.s2 = acc.s1 = $FUNC($Arg0VarName.s1, ret.s1);
    ret.s3 = acc.s2 = $FUNC($Arg0VarName.s2, ret.s2);
    ret.s4 = acc.s3 = $FUNC($Arg0VarName.s3, ret.s3);
    ret.s5 = acc.s4 = $FUNC($Arg0VarName.s4, ret.s4);
    ret.s6 = acc.s5 = $FUNC($Arg0VarName.s5, ret.s5);
    ret.s7 = acc.s6 = $FUNC($Arg0VarName.s6, ret.s6);
    ret.s8 = acc.s7 = $FUNC($Arg0VarName.s7, ret.s7);
    ret.s9 = acc.s8 = $FUNC($Arg0VarName.s8, ret.s8);
    ret.sA = acc.s9 = $FUNC($Arg0VarName.s9, ret.s9);
    ret.sB = acc.sA = $FUNC($Arg0VarName.sA, ret.sA);
    ret.sC = acc.sB = $FUNC($Arg0VarName.sB, ret.sB);
    ret.sD = acc.sC = $FUNC($Arg0VarName.sC, ret.sC);
    ret.sE = acc.sD = $FUNC($Arg0VarName.sD, ret.sD);
    ret.sF = acc.sE = $FUNC($Arg0VarName.sE, ret.sE);
    acc.sF = $FUNC($Arg0VarName.sF, ret.sF);
    *$Arg1VarName = acc;
    return ret;
  }];

code work_group_scan_inclusive_function_op_vector4 = [{
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg1VarName;
    ret.s0 = $FUNC($Arg0VarName.s0, acc.s3);
    ret.s1 = $FUNC($Arg0VarName.s1, ret.s0);
    ret.s2 = $FUNC($Arg0VarName.s2, ret.s1);
    ret.s3 = $FUNC($Arg0VarName.s3, ret.s2);
    *$Arg1VarName = ret;
    return ret;
  }];

code work_group_scan_inclusive_function_op_vector8 = [{
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg1VarName;
    ret.s0 = $FUNC($Arg0VarName.s0, acc.s7);
    ret.s1 = $FUNC($Arg0VarName.s1, ret.s0);
    ret.s2 = $FUNC($Arg0VarName.s2, ret.s1);
    ret.s3 = $FUNC($Arg0VarName.s3, ret.s2);
    ret.s4 = $FUNC($Arg0VarName.s4, ret.s3);
    ret.s5 = $FUNC($Arg0VarName.s5, ret.s4);
    ret.s6 = $FUNC($Arg0VarName.s6, ret.s5);
    ret.s7 = $FUNC($Arg0VarName.s7, ret.s6);
    *$Arg1VarName = ret;
    return ret;
  }];

code work_group_scan_inclusive_function_op_vector16 = [{
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg1VarName;
    ret.s0 = $FUNC($Arg0VarName.s0, acc.sF);
    ret.s1 = $FUNC($Arg0VarName.s1, ret.s0);
    ret.s2 = $FUNC($Arg0VarName.s2, ret.s1);
    ret.s3 = $FUNC($Arg0VarName.s3, ret.s2);
    ret.s4 = $FUNC($Arg0VarName.s4, ret.s3);
    ret.s5 = $FUNC($Arg0VarName.s5, ret.s4);
    ret.s6 = $FUNC($Arg0VarName.s6, ret.s5);
    ret.s7 = $FUNC($Arg0VarName.s7, ret.s6);
    ret.s8 = $FUNC($Arg0VarName.s8, ret.s7);
    ret.s9 = $FUNC($Arg0VarName.s9, ret.s8);
    ret.sA = $FUNC($Arg0VarName.sA, ret.s9);
    ret.sB = $FUNC($Arg0VarName.sB, ret.sA);
    ret.sC = $FUNC($Arg0VarName.sC, ret.sB);
    ret.sD = $FUNC($Arg0VarName.sD, ret.sC);
    ret.sE = $FUNC($Arg0VarName.sE, ret.sD);
    ret.sF = $FUNC($Arg0VarName.sF, ret.sE);
    *$Arg1VarName = ret;
    return ret;
  }];


code work_group_scan_exclusive_function_op_mask4 = [{

    $MaskCastTy#4 arg0 = as_$MaskCastTy#4($Arg0VarName);
    $MaskCastTy#4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), $MaskCastTy#4);
    arg0 &= arg1;
    arg1 = ~arg1 & ($MaskCastTy)$VAL;
    arg0 |= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);

    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg2VarName;
    ret.s0 = acc.s3;
    ret.s1 = acc.s0 = $FUNC($Arg0VarName.s0, ret.s0);
    ret.s2 = acc.s1 = $FUNC($Arg0VarName.s1, ret.s1);
    ret.s3 = acc.s2 = $FUNC($Arg0VarName.s2, ret.s2);
    acc.s3 = $FUNC($Arg0VarName.s3, ret.s3);
    *$Arg2VarName = acc;
    return ret;
  }];

code work_group_scan_exclusive_function_op_mask8 = [{
    $MaskCastTy#8 arg0 = as_$MaskCastTy#8($Arg0VarName);
    $MaskCastTy#8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), $MaskCastTy#8);
    arg0 &= arg1;
    arg1 = ~arg1 & ($MaskCastTy)$VAL;
    arg0 |= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg2VarName;
    ret.s0 = acc.s7;
    ret.s1 = acc.s0 = $FUNC($Arg0VarName.s0, ret.s0);
    ret.s2 = acc.s1 = $FUNC($Arg0VarName.s1, ret.s1);
    ret.s3 = acc.s2 = $FUNC($Arg0VarName.s2, ret.s2);
    ret.s4 = acc.s3 = $FUNC($Arg0VarName.s3, ret.s3);
    ret.s5 = acc.s4 = $FUNC($Arg0VarName.s4, ret.s4);
    ret.s6 = acc.s5 = $FUNC($Arg0VarName.s5, ret.s5);
    ret.s7 = acc.s6 = $FUNC($Arg0VarName.s6, ret.s6);
    acc.s7 = $FUNC($Arg0VarName.s7, ret.s7);
    *$Arg2VarName = acc;
    return ret;
  }];

code work_group_scan_exclusive_function_op_mask16 = [{
    $MaskCastTy#16 arg0 = as_$MaskCastTy#16($Arg0VarName);
    $MaskCastTy#16 arg1 = __builtin_convertvector(as_int16($Arg1VarName), $MaskCastTy#16);
    arg0 &= arg1;
    arg1 = ~arg1 & ($MaskCastTy)$VAL;
    arg0 |= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg2VarName;
    ret.s0 = acc.sF;
    ret.s1 = acc.s0 = $FUNC($Arg0VarName.s0, ret.s0);
    ret.s2 = acc.s1 = $FUNC($Arg0VarName.s1, ret.s1);
    ret.s3 = acc.s2 = $FUNC($Arg0VarName.s2, ret.s2);
    ret.s4 = acc.s3 = $FUNC($Arg0VarName.s3, ret.s3);
    ret.s5 = acc.s4 = $FUNC($Arg0VarName.s4, ret.s4);
    ret.s6 = acc.s5 = $FUNC($Arg0VarName.s5, ret.s5);
    ret.s7 = acc.s6 = $FUNC($Arg0VarName.s6, ret.s6);
    ret.s8 = acc.s7 = $FUNC($Arg0VarName.s7, ret.s7);
    ret.s9 = acc.s8 = $FUNC($Arg0VarName.s8, ret.s8);
    ret.sA = acc.s9 = $FUNC($Arg0VarName.s9, ret.s9);
    ret.sB = acc.sA = $FUNC($Arg0VarName.sA, ret.sA);
    ret.sC = acc.sB = $FUNC($Arg0VarName.sB, ret.sB);
    ret.sD = acc.sC = $FUNC($Arg0VarName.sC, ret.sC);
    ret.sE = acc.sD = $FUNC($Arg0VarName.sD, ret.sD);
    ret.sF = acc.sE = $FUNC($Arg0VarName.sE, ret.sE);
    acc.sF = $FUNC($Arg0VarName.sF, ret.sF);
    *$Arg2VarName = acc;
    return ret;
  }];

code work_group_scan_inclusive_function_op_mask4 = [{
    $MaskCastTy#4 arg0 = as_$MaskCastTy#4($Arg0VarName);
    $MaskCastTy#4 arg1 = __builtin_convertvector(as_int4($Arg1VarName), $MaskCastTy#4);
    arg0 &= arg1;
    arg1 = ~arg1 & ($MaskCastTy)$VAL;
    arg0 |= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg2VarName;
    ret.s0 = $FUNC($Arg0VarName.s0, acc.s3);
    ret.s1 = $FUNC($Arg0VarName.s1, ret.s0);
    ret.s2 = $FUNC($Arg0VarName.s2, ret.s1);
    ret.s3 = $FUNC($Arg0VarName.s3, ret.s2);
    *$Arg2VarName = ret;
    return ret;
  }];

code work_group_scan_inclusive_function_op_mask8 = [{
    $MaskCastTy#8 arg0 = as_$MaskCastTy#8($Arg0VarName);
    $MaskCastTy#8 arg1 = __builtin_convertvector(as_int8($Arg1VarName), $MaskCastTy#8);
    arg0 &= arg1;
    arg1 = ~arg1 & ($MaskCastTy)$VAL;
    arg0 |= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg2VarName;
    ret.s0 = $FUNC($Arg0VarName.s0, acc.s7);
    ret.s1 = $FUNC($Arg0VarName.s1, ret.s0);
    ret.s2 = $FUNC($Arg0VarName.s2, ret.s1);
    ret.s3 = $FUNC($Arg0VarName.s3, ret.s2);
    ret.s4 = $FUNC($Arg0VarName.s4, ret.s3);
    ret.s5 = $FUNC($Arg0VarName.s5, ret.s4);
    ret.s6 = $FUNC($Arg0VarName.s6, ret.s5);
    ret.s7 = $FUNC($Arg0VarName.s7, ret.s6);
    *$Arg2VarName = ret;
    return ret;
  }];

code work_group_scan_inclusive_function_op_mask16 = [{
    $MaskCastTy#16 arg0 = as_$MaskCastTy#16($Arg0VarName);
    $MaskCastTy#16 arg1 = __builtin_convertvector(as_int16($Arg1VarName), $MaskCastTy#16);
    arg0 &= arg1;
    arg1 = ~arg1 & ($MaskCastTy)$VAL;
    arg0 |= arg1;
    $Arg0VarName = as_$Arg0Type(arg0);
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg2VarName;
    ret.s0 = $FUNC($Arg0VarName.s0, acc.sF);
    ret.s1 = $FUNC($Arg0VarName.s1, ret.s0);
    ret.s2 = $FUNC($Arg0VarName.s2, ret.s1);
    ret.s3 = $FUNC($Arg0VarName.s3, ret.s2);
    ret.s4 = $FUNC($Arg0VarName.s4, ret.s3);
    ret.s5 = $FUNC($Arg0VarName.s5, ret.s4);
    ret.s6 = $FUNC($Arg0VarName.s6, ret.s5);
    ret.s7 = $FUNC($Arg0VarName.s7, ret.s6);
    ret.s8 = $FUNC($Arg0VarName.s8, ret.s7);
    ret.s9 = $FUNC($Arg0VarName.s9, ret.s8);
    ret.sA = $FUNC($Arg0VarName.sA, ret.s9);
    ret.sB = $FUNC($Arg0VarName.sB, ret.sA);
    ret.sC = $FUNC($Arg0VarName.sC, ret.sB);
    ret.sD = $FUNC($Arg0VarName.sD, ret.sC);
    ret.sE = $FUNC($Arg0VarName.sE, ret.sD);
    ret.sF = $FUNC($Arg0VarName.sF, ret.sE);
    *$Arg2VarName = ret;
    return ret;
  }];

// Vector versions of Scan_add/min/max functions
OclBuiltinImpl work_group_scan_exclusive_add_vector_helper_gen4 =
      OclBuiltinImpl<work_group_scan_exclusive_add_helper, work_group_vector_types_all4,    0, work_group_scan_exclusive_function_op_vector4, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_scan_exclusive_add_vector_helper_gen8 =
      OclBuiltinImpl<work_group_scan_exclusive_add_helper, work_group_vector_types_all8,    0, work_group_scan_exclusive_function_op_vector8, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_scan_exclusive_add_vector_helper_gen16 =
      OclBuiltinImpl<work_group_scan_exclusive_add_helper, work_group_vector_types_all16,   0, work_group_scan_exclusive_function_op_vector16, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_vector_helper_int4 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_vector_types_int4,    0, work_group_scan_exclusive_function_op_vector4, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_vector_helper_int8 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_vector_types_int8,    0, work_group_scan_exclusive_function_op_vector8, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_vector_helper_int16 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_vector_types_int16,   0, work_group_scan_exclusive_function_op_vector16, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_vector_helper_float4 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_vector_types_float4,  0, work_group_scan_exclusive_function_op_vector4, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_vector_helper_float8 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_vector_types_float8,  0, work_group_scan_exclusive_function_op_vector8, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_vector_helper_float16 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_vector_types_float16, 0, work_group_scan_exclusive_function_op_vector16, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_vector_helper_int4 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_int4,    0, work_group_scan_exclusive_function_op_vector4, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_vector_helper_int8 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_int8,    0, work_group_scan_exclusive_function_op_vector8, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_vector_helper_int16 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_int16,   0, work_group_scan_exclusive_function_op_vector16, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_vector_helper_float4 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_float4,  0, work_group_scan_exclusive_function_op_vector4, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_vector_helper_float8 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_float8,  0, work_group_scan_exclusive_function_op_vector8, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_vector_helper_float16 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_float16, 0, work_group_scan_exclusive_function_op_vector16, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_scan_inclusive_add_vector_helper_gen4 =
      OclBuiltinImpl<work_group_scan_inclusive_add_helper, work_group_vector_types_all4,    0, work_group_scan_inclusive_function_op_vector4, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_scan_inclusive_add_vector_helper_gen8 =
      OclBuiltinImpl<work_group_scan_inclusive_add_helper, work_group_vector_types_all8,    0, work_group_scan_inclusive_function_op_vector8, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_scan_inclusive_add_vector_helper_gen16 =
      OclBuiltinImpl<work_group_scan_inclusive_add_helper, work_group_vector_types_all16,   0, work_group_scan_inclusive_function_op_vector16, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_vector_helper_int4 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_vector_types_int4,    0, work_group_scan_inclusive_function_op_vector4, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_vector_helper_int8 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_vector_types_int8,    0, work_group_scan_inclusive_function_op_vector8, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_vector_helper_int16 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_vector_types_int16,   0, work_group_scan_inclusive_function_op_vector16, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_vector_helper_float4 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_vector_types_float4,  0, work_group_scan_inclusive_function_op_vector4, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_vector_helper_float8 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_vector_types_float8,  0, work_group_scan_inclusive_function_op_vector8, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_vector_helper_float16 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_vector_types_float16, 0, work_group_scan_inclusive_function_op_vector16, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_vector_helper_int4 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_int4,    0, work_group_scan_inclusive_function_op_vector4, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_vector_helper_int8 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_int8,    0, work_group_scan_inclusive_function_op_vector8, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_vector_helper_int16 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_int16,   0, work_group_scan_inclusive_function_op_vector16, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_vector_helper_float4 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_float4,  0, work_group_scan_inclusive_function_op_vector4, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_vector_helper_float8 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_float8,  0, work_group_scan_inclusive_function_op_vector8, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_vector_helper_float16 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_float16, 0, work_group_scan_inclusive_function_op_vector16, [["$FUNC","fmax"]]>;

// Vector versions of Scan_add/min/max functions
OclBuiltinImpl work_group_scan_exclusive_add_mask_helper_gen4 =
      OclBuiltinImpl<work_group_scan_exclusive_add_helper_mask, work_group_vector_types_all4,    0, work_group_scan_exclusive_function_op_mask4, [["$FUNC","work_group_add_util"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_exclusive_add_mask_helper_gen8 =
      OclBuiltinImpl<work_group_scan_exclusive_add_helper_mask, work_group_vector_types_all8,    0, work_group_scan_exclusive_function_op_mask8, [["$FUNC","work_group_add_util"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_exclusive_add_mask_helper_gen16 =
      OclBuiltinImpl<work_group_scan_exclusive_add_helper_mask, work_group_vector_types_all16,   0, work_group_scan_exclusive_function_op_mask16, [["$FUNC","work_group_add_util"], ["$VAL", "0"]]>;

OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_int4 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v4i32], 0, work_group_scan_exclusive_function_op_mask4, [["$FUNC","min"], ["$VAL", "INT_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_int8 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v8i32], 0, work_group_scan_exclusive_function_op_mask8, [["$FUNC","min"], ["$VAL", "INT_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_int16 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v16i32], 0, work_group_scan_exclusive_function_op_mask16, [["$FUNC","min"], ["$VAL", "INT_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_uint4 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v4u32], 0, work_group_scan_exclusive_function_op_mask4, [["$FUNC","min"], ["$VAL", "UINT_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_uint8 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v8u32], 0, work_group_scan_exclusive_function_op_mask8, [["$FUNC","min"], ["$VAL", "UINT_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_uint16 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v16u32], 0, work_group_scan_exclusive_function_op_mask16, [["$FUNC","min"], ["$VAL", "UINT_MAX"]]>;

OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_long4 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v4i64], 0, work_group_scan_exclusive_function_op_mask4, [["$FUNC","min"], ["$VAL", "LONG_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_long8 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v8i64], 0, work_group_scan_exclusive_function_op_mask8, [["$FUNC","min"], ["$VAL", "LONG_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_long16 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v16i64], 0, work_group_scan_exclusive_function_op_mask16, [["$FUNC","min"], ["$VAL", "LONG_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_ulong4 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v4u64], 0, work_group_scan_exclusive_function_op_mask4, [["$FUNC","min"], ["$VAL", "ULONG_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_ulong8 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v8u64], 0, work_group_scan_exclusive_function_op_mask8, [["$FUNC","min"], ["$VAL", "ULONG_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_ulong16 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v16u64], 0, work_group_scan_exclusive_function_op_mask16, [["$FUNC","min"], ["$VAL", "ULONG_MAX"]]>;

OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_float4 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, work_group_vector_types_float4,  0, work_group_scan_exclusive_function_op_mask4, [["$FUNC","fmin"], ["$VAL", "INFINITY"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_float8 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, work_group_vector_types_float8,  0, work_group_scan_exclusive_function_op_mask8, [["$FUNC","fmin"], ["$VAL", "INFINITY"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_float16 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, work_group_vector_types_float16, 0, work_group_scan_exclusive_function_op_mask16, [["$FUNC","fmin"], ["$VAL", "INFINITY"]]>;


OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_int4 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v4i32], 0, work_group_scan_exclusive_function_op_mask4, [["$FUNC","max"], ["$VAL", "INT_MIN"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_int8 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v8i32], 0, work_group_scan_exclusive_function_op_mask8, [["$FUNC","max"], ["$VAL", "INT_MIN"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_int16 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v16i32], 0, work_group_scan_exclusive_function_op_mask16, [["$FUNC","max"], ["$VAL", "INT_MIN"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_uint4 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v4u32], 0, work_group_scan_exclusive_function_op_mask4, [["$FUNC","max"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_uint8 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v8u32], 0, work_group_scan_exclusive_function_op_mask8, [["$FUNC","max"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_uint16 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v16u32], 0, work_group_scan_exclusive_function_op_mask16, [["$FUNC","max"], ["$VAL", "0"]]>;

OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_long4 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v4i64], 0, work_group_scan_exclusive_function_op_mask4, [["$FUNC","max"], ["$VAL", "LONG_MIN"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_long8 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v8i64], 0, work_group_scan_exclusive_function_op_mask8, [["$FUNC","max"], ["$VAL", "LONG_MIN"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_long16 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v16i64], 0, work_group_scan_exclusive_function_op_mask16, [["$FUNC","max"], ["$VAL", "LONG_MIN"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_ulong4 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v4u64], 0, work_group_scan_exclusive_function_op_mask4, [["$FUNC","max"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_ulong8 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v8u64], 0, work_group_scan_exclusive_function_op_mask8, [["$FUNC","max"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_ulong16 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v16u64], 0, work_group_scan_exclusive_function_op_mask16, [["$FUNC","max"], ["$VAL", "0"]]>;

OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_float4 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_float4,  0, work_group_scan_exclusive_function_op_mask4, [["$FUNC","fmax"], ["$VAL", "-INFINITY"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_float8 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_float8,  0, work_group_scan_exclusive_function_op_mask8, [["$FUNC","fmax"], ["$VAL", "-INFINITY"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_float16 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_float16, 0, work_group_scan_exclusive_function_op_mask16, [["$FUNC","fmax"], ["$VAL", "-INFINITY"]]>;


OclBuiltinImpl work_group_scan_inclusive_add_mask_helper_gen4 =
      OclBuiltinImpl<work_group_scan_inclusive_add_helper_mask, work_group_vector_types_all4,    0, work_group_scan_inclusive_function_op_mask4, [["$FUNC","work_group_add_util"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_inclusive_add_mask_helper_gen8 =
      OclBuiltinImpl<work_group_scan_inclusive_add_helper_mask, work_group_vector_types_all8,    0, work_group_scan_inclusive_function_op_mask8, [["$FUNC","work_group_add_util"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_inclusive_add_mask_helper_gen16 =
      OclBuiltinImpl<work_group_scan_inclusive_add_helper_mask, work_group_vector_types_all16,   0, work_group_scan_inclusive_function_op_mask16, [["$FUNC","work_group_add_util"], ["$VAL", "0"]]>;

OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_int4 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v4i32], 0, work_group_scan_inclusive_function_op_mask4, [["$FUNC","min"], ["$VAL", "INT_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_int8 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v8i32], 0, work_group_scan_inclusive_function_op_mask8, [["$FUNC","min"], ["$VAL", "INT_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_int16 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v16i32], 0, work_group_scan_inclusive_function_op_mask16, [["$FUNC","min"], ["$VAL", "INT_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_uint4 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v4u32], 0, work_group_scan_inclusive_function_op_mask4, [["$FUNC","min"], ["$VAL", "UINT_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_uint8 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v8u32], 0, work_group_scan_inclusive_function_op_mask8, [["$FUNC","min"], ["$VAL", "UINT_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_uint16 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v16u32], 0, work_group_scan_inclusive_function_op_mask16, [["$FUNC","min"], ["$VAL", "UINT_MAX"]]>;

OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_long4 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v4i64], 0, work_group_scan_inclusive_function_op_mask4, [["$FUNC","min"], ["$VAL", "LONG_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_long8 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v8i64], 0, work_group_scan_inclusive_function_op_mask8, [["$FUNC","min"], ["$VAL", "LONG_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_long16 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v16i64], 0, work_group_scan_inclusive_function_op_mask16, [["$FUNC","min"], ["$VAL", "LONG_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_ulong4 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v4u64], 0, work_group_scan_inclusive_function_op_mask4, [["$FUNC","min"], ["$VAL", "ULONG_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_ulong8 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v8u64], 0, work_group_scan_inclusive_function_op_mask8, [["$FUNC","min"], ["$VAL", "ULONG_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_ulong16 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v16u64], 0, work_group_scan_inclusive_function_op_mask16, [["$FUNC","min"], ["$VAL", "ULONG_MAX"]]>;

OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_float4 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, work_group_vector_types_float4,  0, work_group_scan_inclusive_function_op_mask4, [["$FUNC","fmin"], ["$VAL", "INFINITY"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_float8 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, work_group_vector_types_float8,  0, work_group_scan_inclusive_function_op_mask8, [["$FUNC","fmin"], ["$VAL", "INFINITY"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_float16 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, work_group_vector_types_float16, 0, work_group_scan_inclusive_function_op_mask16, [["$FUNC","fmin"], ["$VAL", "INFINITY"]]>;


OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_int4 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v4i32], 0, work_group_scan_inclusive_function_op_mask4, [["$FUNC","max"], ["$VAL", "INT_MIN"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_int8 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v8i32], 0, work_group_scan_inclusive_function_op_mask8, [["$FUNC","max"], ["$VAL", "INT_MIN"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_int16 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v16i32], 0, work_group_scan_inclusive_function_op_mask16, [["$FUNC","max"], ["$VAL", "INT_MIN"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_uint4 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v4u32], 0, work_group_scan_inclusive_function_op_mask4, [["$FUNC","max"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_uint8 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v8u32], 0, work_group_scan_inclusive_function_op_mask8, [["$FUNC","max"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_uint16 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v16u32], 0, work_group_scan_inclusive_function_op_mask16, [["$FUNC","max"], ["$VAL", "0"]]>;

OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_long4 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v4i64], 0, work_group_scan_inclusive_function_op_mask4, [["$FUNC","max"], ["$VAL", "LONG_MIN"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_long8 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v8i64], 0, work_group_scan_inclusive_function_op_mask8, [["$FUNC","max"], ["$VAL", "LONG_MIN"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_long16 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v16i64], 0, work_group_scan_inclusive_function_op_mask16, [["$FUNC","max"], ["$VAL", "LONG_MIN"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_ulong4 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v4u64], 0, work_group_scan_inclusive_function_op_mask4, [["$FUNC","max"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_ulong8 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v8u64], 0, work_group_scan_inclusive_function_op_mask8, [["$FUNC","max"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_ulong16 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v16u64], 0, work_group_scan_inclusive_function_op_mask16, [["$FUNC","max"], ["$VAL", "0"]]>;

OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_float4 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_float4,  0, work_group_scan_inclusive_function_op_mask4, [["$FUNC","fmax"], ["$VAL", "-INFINITY"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_float8 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_float8,  0, work_group_scan_inclusive_function_op_mask8, [["$FUNC","fmax"], ["$VAL", "-INFINITY"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_float16 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_float16, 0, work_group_scan_inclusive_function_op_mask16, [["$FUNC","fmax"], ["$VAL", "-INFINITY"]]>;




// Scalar versions of Scan_add/min/max functions
code work_group_scan_exclusive_function_op_scalar = [{
    $Arg0Type acc = *$Arg1VarName;
    *$Arg1VarName = $FUNC($Arg0VarName, *$Arg1VarName);
    return acc;
  }];

OclBuiltinImpl work_group_scan_exclusive_add_scalar_helper_gen =
      OclBuiltinImpl<work_group_scan_exclusive_add_helper, work_group_scalar_types_all,     0, work_group_scan_exclusive_function_op_scalar, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_scalar_helper_int =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_scalar_types_int,     0, work_group_scan_exclusive_function_op_scalar, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_scalar_helper_float =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_scalar_types_float,   0, work_group_scan_exclusive_function_op_scalar, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_scalar_helper_int =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_scalar_types_int,     0, work_group_scan_exclusive_function_op_scalar, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_scalar_helper_float =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_scalar_types_float,   0, work_group_scan_exclusive_function_op_scalar, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_scan_inclusive_add_scalar_helper_gen =
      OclBuiltinImpl<work_group_scan_inclusive_add_helper, work_group_scalar_types_all,     0, work_group_inclusive_function_op_scalar, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_scalar_helper_int =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_scalar_types_int,     0, work_group_inclusive_function_op_scalar, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_scalar_helper_float =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_scalar_types_float,   0, work_group_inclusive_function_op_scalar, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_scalar_helper_int =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_scalar_types_int,     0, work_group_inclusive_function_op_scalar, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_scalar_helper_float =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_scalar_types_float,   0, work_group_inclusive_function_op_scalar, [["$FUNC","fmax"]]>;


//-----------------------------------------------
// 3. Finalization of uniform result
//-----------------------------------------------

code work_group_finalize_function_op4 = [{
    $Arg0VecType ret;
    ret.s0 = $Arg0VarName.s0;
    ret.s0 = $FUNC($Arg0VarName.s1, ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s2, ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s3, ret.s0);
    return ($Arg0VecType)(ret.s0);
  }];

code work_group_finalize_function_op8 = [{
    $Arg0VecType ret;
    ret.s0 = $Arg0VarName.s0;
    ret.s0 = $FUNC($Arg0VarName.s1, ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s2, ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s3, ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s4, ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s5, ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s6, ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s7, ret.s0);
    return ($Arg0VecType)(ret.s0);
  }];

code work_group_finalize_function_op16 = [{
    $Arg0VecType ret;
    ret.s0 = $Arg0VarName.s0;
    ret.s0 = $FUNC($Arg0VarName.s1,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s2,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s3,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s4,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s5,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s6,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s7,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s8,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s9,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.sA,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.sB,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.sC,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.sD,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.sE,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.sF,  ret.s0);
    return ($Arg0VecType)(ret.s0);
  }];

// Definitions of finalization functions
OclBuiltinImpl work_group_all_finalize_helper_int4 =  OclBuiltinImpl<work_group_all_finalize_helper,   [v4i32],   0, work_group_finalize_function_op4, [["$FUNC","work_group_all_util"]]>;
OclBuiltinImpl work_group_all_finalize_helper_int8 =  OclBuiltinImpl<work_group_all_finalize_helper,   [v8i32],   0, work_group_finalize_function_op8, [["$FUNC","work_group_all_util"]]>;
OclBuiltinImpl work_group_all_finalize_helper_int16 = OclBuiltinImpl<work_group_all_finalize_helper,   [v16i32],  0, work_group_finalize_function_op16, [["$FUNC","work_group_all_util"]]>;
OclBuiltinImpl work_group_any_finalize_helper_int4 =  OclBuiltinImpl<work_group_any_finalize_helper,   [v4i32],   0, work_group_finalize_function_op4, [["$FUNC","work_group_any_util"]]>;
OclBuiltinImpl work_group_any_finalize_helper_int8 =  OclBuiltinImpl<work_group_any_finalize_helper,   [v8i32],   0, work_group_finalize_function_op8, [["$FUNC","work_group_any_util"]]>;
OclBuiltinImpl work_group_any_finalize_helper_int16 = OclBuiltinImpl<work_group_any_finalize_helper,   [v16i32],  0, work_group_finalize_function_op16, [["$FUNC","work_group_any_util"]]>;
OclBuiltinImpl work_group_reduce_add_finalize_helper_gen4 =
      OclBuiltinImpl<work_group_reduce_add_finalize_helper, work_group_vector_types_all4,    0, work_group_finalize_function_op4, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_reduce_add_finalize_helper_gen8 =
      OclBuiltinImpl<work_group_reduce_add_finalize_helper, work_group_vector_types_all8,    0, work_group_finalize_function_op8, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_reduce_add_finalize_helper_gen16 =
      OclBuiltinImpl<work_group_reduce_add_finalize_helper, work_group_vector_types_all16,   0, work_group_finalize_function_op16, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_int4 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper, work_group_vector_types_int4,    0, work_group_finalize_function_op4, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_int8 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper, work_group_vector_types_int8,    0, work_group_finalize_function_op8, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_int16 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper, work_group_vector_types_int16,   0, work_group_finalize_function_op16, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_float4 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper, work_group_vector_types_float4,  0, work_group_finalize_function_op4, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_float8 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper, work_group_vector_types_float8,  0, work_group_finalize_function_op8, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_float16 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper, work_group_vector_types_float16, 0, work_group_finalize_function_op16, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_int4 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper, work_group_vector_types_int4,    0, work_group_finalize_function_op4, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_int8 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper, work_group_vector_types_int8,    0, work_group_finalize_function_op8, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_int16 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper, work_group_vector_types_int16,   0, work_group_finalize_function_op16, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_float4 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper, work_group_vector_types_float4,  0, work_group_finalize_function_op4, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_float8 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper, work_group_vector_types_float8,  0, work_group_finalize_function_op8, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_float16 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper, work_group_vector_types_float16, 0, work_group_finalize_function_op16, [["$FUNC","fmax"]]>;

// Definitions of finalization functions with a dummy mask.
OclBuiltinImpl work_group_all_finalize_helper_mask_int4 =  OclBuiltinImpl<work_group_all_finalize_helper_mask, [v4i32],   0, work_group_finalize_function_op4, [["$FUNC","work_group_all_util"]]>;
OclBuiltinImpl work_group_all_finalize_helper_mask_int8 =  OclBuiltinImpl<work_group_all_finalize_helper_mask, [v8i32],   0, work_group_finalize_function_op8, [["$FUNC","work_group_all_util"]]>;
OclBuiltinImpl work_group_all_finalize_helper_mask_int16 = OclBuiltinImpl<work_group_all_finalize_helper_mask, [v16i32],  0, work_group_finalize_function_op16, [["$FUNC","work_group_all_util"]]>;
OclBuiltinImpl work_group_any_finalize_helper_mask_int4 =  OclBuiltinImpl<work_group_any_finalize_helper_mask, [v4i32],   0, work_group_finalize_function_op4, [["$FUNC","work_group_any_util"]]>;
OclBuiltinImpl work_group_any_finalize_helper_mask_int8 =  OclBuiltinImpl<work_group_any_finalize_helper_mask, [v8i32],   0, work_group_finalize_function_op8, [["$FUNC","work_group_any_util"]]>;
OclBuiltinImpl work_group_any_finalize_helper_mask_int16 = OclBuiltinImpl<work_group_any_finalize_helper_mask, [v16i32],  0, work_group_finalize_function_op16, [["$FUNC","work_group_any_util"]]>;
OclBuiltinImpl work_group_reduce_add_finalize_helper_mask_gen4 =
      OclBuiltinImpl<work_group_reduce_add_finalize_helper_mask, work_group_vector_types_all4,    0, work_group_finalize_function_op4, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_reduce_add_finalize_helper_mask_gen8 =
      OclBuiltinImpl<work_group_reduce_add_finalize_helper_mask, work_group_vector_types_all8,    0, work_group_finalize_function_op8, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_reduce_add_finalize_helper_mask_gen16 =
      OclBuiltinImpl<work_group_reduce_add_finalize_helper_mask, work_group_vector_types_all16,   0, work_group_finalize_function_op16, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_mask_int4 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper_mask, work_group_vector_types_int4,    0, work_group_finalize_function_op4, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_mask_int8 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper_mask, work_group_vector_types_int8,    0, work_group_finalize_function_op8, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_mask_int16 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper_mask, work_group_vector_types_int16,   0, work_group_finalize_function_op16, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_mask_float4 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper_mask, work_group_vector_types_float4,  0, work_group_finalize_function_op4, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_mask_float8 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper_mask, work_group_vector_types_float8,  0, work_group_finalize_function_op8, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_mask_float16 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper_mask, work_group_vector_types_float16, 0, work_group_finalize_function_op16, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_mask_int4 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper_mask, work_group_vector_types_int4,    0, work_group_finalize_function_op4, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_mask_int8 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper_mask, work_group_vector_types_int8,    0, work_group_finalize_function_op8, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_mask_int16 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper_mask, work_group_vector_types_int16,   0, work_group_finalize_function_op16, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_mask_float4 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper_mask, work_group_vector_types_float4,  0, work_group_finalize_function_op4, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_mask_float8 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper_mask, work_group_vector_types_float8,  0, work_group_finalize_function_op8, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_mask_float16 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper_mask, work_group_vector_types_float16, 0, work_group_finalize_function_op16, [["$FUNC","fmax"]]>;
