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
OclBuiltinImpl work_group_reduce_add_fake = OclBuiltinImpl<work_group_reduce_add, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl work_group_reduce_mul_fake = OclBuiltinImpl<work_group_reduce_mul, work_group_types_all, 0,
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
OclBuiltinImpl work_group_scan_exclusive_mul_fake = OclBuiltinImpl<work_group_scan_exclusive_mul, work_group_types_all, 0,
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
OclBuiltinImpl work_group_scan_inclusive_mul_fake = OclBuiltinImpl<work_group_scan_inclusive_mul, work_group_types_all, 0,
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
OclBuiltinImpl work_group_reduce_mul_mask_fake = OclBuiltinImpl<work_group_reduce_mul_mask, work_group_types_all, 0,
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
OclBuiltinImpl work_group_scan_exclusive_mul_mask_fake = OclBuiltinImpl<work_group_scan_exclusive_mul_mask, work_group_types_all, 0,
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
OclBuiltinImpl work_group_scan_inclusive_mul_mask_fake = OclBuiltinImpl<work_group_scan_inclusive_mul_mask, work_group_types_all, 0,
  [{
    return $Arg0VarName;
  }]>;

// Functions for addition/and/or operations (in order to have all per-WI helper functions uniformal)
OclBuiltinImpl work_group_add_helper_util__generic = OclBuiltinImpl<work_group_add_helper_util, work_group_types_all, 0,
  [{
    return $Arg0VarName + $Arg1VarName;
  }]>;
OclBuiltinImpl work_group_mul_helper_util__generic = OclBuiltinImpl<work_group_mul_helper_util, work_group_types_all, 0,
  [{
    return $Arg0VarName * $Arg1VarName;
  }]>;
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

defvar work_group_inclusive_function_op_vector = [{
    $Arg0Type acc = $FUNC($Arg0VarName, *$Arg1VarName);
    *$Arg1VarName = acc;
    return acc;
  }];

defvar work_group_inclusive_function_op_vector32 = [{
    $Arg0Type acc;
    acc.lo = $FUNC($Arg0VarName.lo, (*$Arg1VarName).lo);
    acc.hi = $FUNC($Arg0VarName.hi, (*$Arg1VarName).hi);
    *$Arg1VarName = acc;
    return acc;
  }];

defvar work_group_inclusive_function_op_vector64 = [{
    $Arg0Type acc;
    acc.lo.lo = $FUNC($Arg0VarName.lo.lo, (*$Arg1VarName).lo.lo);
    acc.lo.hi = $FUNC($Arg0VarName.lo.hi, (*$Arg1VarName).lo.hi);
    acc.hi.lo = $FUNC($Arg0VarName.hi.lo, (*$Arg1VarName).hi.lo);
    acc.hi.hi = $FUNC($Arg0VarName.hi.hi, (*$Arg1VarName).hi.hi);
    *$Arg1VarName = acc;
    return acc;
  }];


defvar work_group_inclusive_function_op_scalar = [{
    $Arg0Type acc = $FUNC($Arg0VarName, *$Arg1VarName);
    *$Arg1VarName = acc;
    return acc;
  }];

defvar work_group_inclusive_function_op_mask = [{
    $MaskCastTy$VecLength arg0 = __builtin_astype($Arg0VarName, $MaskCastTy$VecLength);
    $MaskCastTy$VecLength arg1 = __builtin_convertvector(__builtin_astype($Arg1VarName, int$VecLength), $MaskCastTy$VecLength);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);
    $Arg0Type acc = $FUNC($Arg0VarName, *$Arg2VarName);
    *$Arg2VarName = acc;
    return acc;
  }];

defvar work_group_inclusive_function_op_mask32 = [{
    $MaskCastTy$VecLength arg0 = __builtin_astype($Arg0VarName, $MaskCastTy$VecLength);
    $MaskCastTy$VecLength arg1 = __builtin_convertvector(__builtin_astype($Arg1VarName, int$VecLength), $MaskCastTy$VecLength);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);
    $Arg0Type acc;
    acc.lo = $FUNC($Arg0VarName.lo, (*$Arg2VarName).lo);
    acc.hi = $FUNC($Arg0VarName.hi, (*$Arg2VarName).hi);
    *$Arg2VarName = acc;
    return acc;
  }];

defvar work_group_inclusive_function_op_mask64 = [{
    $MaskCastTy$VecLength arg0 = __builtin_astype($Arg0VarName, $MaskCastTy$VecLength);
    $MaskCastTy$VecLength arg1 = __builtin_convertvector(__builtin_astype($Arg1VarName, int$VecLength), $MaskCastTy$VecLength);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);
    $Arg0Type acc;
    acc.lo.lo = $FUNC($Arg0VarName.lo.lo, (*$Arg2VarName).lo.lo);
    acc.lo.hi = $FUNC($Arg0VarName.lo.hi, (*$Arg2VarName).lo.hi);
    acc.hi.lo = $FUNC($Arg0VarName.hi.lo, (*$Arg2VarName).hi.lo);
    acc.hi.hi = $FUNC($Arg0VarName.hi.hi, (*$Arg2VarName).hi.hi);
    *$Arg2VarName = acc;
    return acc;
  }];


// work_group_all
OclBuiltinImpl work_group_all_vector_helper_int = OclBuiltinImpl<work_group_all_helper, [v4i32, v8i32, v16i32, v32i32, v64i32], 0, work_group_inclusive_function_op_vector, [["$FUNC", "work_group_all_util"]]>;
OclBuiltinImpl work_group_all_scalar_helper_int = OclBuiltinImpl<work_group_all_helper, [v1i32], 0, work_group_inclusive_function_op_scalar, [["$FUNC", "work_group_all_util"]]>;
OclBuiltinImpl work_group_all_mask_helper_int = OclBuiltinImpl<work_group_all_helper_mask, [v4i32, v8i32, v16i32, v32i32, v64i32], 0, work_group_inclusive_function_op_mask, [["$FUNC", "work_group_all_util"], ["$VAL", "1"]]>;

// work_group_any
OclBuiltinImpl work_group_any_vector_helper_int = OclBuiltinImpl<work_group_any_helper, [v4i32, v8i32, v16i32, v32i32, v64i32], 0, work_group_inclusive_function_op_vector, [["$FUNC", "work_group_any_util"]]>;
OclBuiltinImpl work_group_any_scalar_helper_int = OclBuiltinImpl<work_group_any_helper, [v1i32], 0, work_group_inclusive_function_op_scalar, [["$FUNC", "work_group_any_util"]]>;
OclBuiltinImpl work_group_any_mask_helper_int = OclBuiltinImpl<work_group_any_helper_mask, [v4i32, v8i32, v16i32, v32i32, v64i32], 0, work_group_inclusive_function_op_mask, [["$FUNC", "work_group_any_util"], ["$VAL", "0"]]>;

// work_group_reduce_add
OclBuiltinImpl work_group_reduce_add_vector_helper_generic = OclBuiltinImpl<work_group_reduce_add_helper, work_group_vector_types_all, 0, work_group_inclusive_function_op_vector, [["$FUNC", "work_group_add_util"]]>;
OclBuiltinImpl work_group_reduce_add_scalar_helper_generic = OclBuiltinImpl<work_group_reduce_add_helper, work_group_scalar_types_all, 0, work_group_inclusive_function_op_scalar, [["$FUNC", "work_group_add_util"]]>;

// work_group_reduce_mul
OclBuiltinImpl work_group_reduce_mul_vector_helper_generic = OclBuiltinImpl<work_group_reduce_mul_helper, work_group_vector_types_all, 0, work_group_inclusive_function_op_vector, [["$FUNC", "work_group_mul_util"]]>;
OclBuiltinImpl work_group_reduce_mul_scalar_helper_generic = OclBuiltinImpl<work_group_reduce_mul_helper, work_group_scalar_types_all, 0, work_group_inclusive_function_op_scalar, [["$FUNC", "work_group_mul_util"]]>;

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

// work_group_reduce_max/min
OclBuiltinImpl work_group_reduce_min_vector_helper_int4 = OclBuiltinImpl<work_group_reduce_min_helper, work_group_vector_types_int4, 0, work_group_inclusive_function_op_vector, [["$FUNC", "min"]]>;
OclBuiltinImpl work_group_reduce_min_vector_helper_int8 = OclBuiltinImpl<work_group_reduce_min_helper, work_group_vector_types_int8, 0, work_group_inclusive_function_op_vector, [["$FUNC", "min"]]>;
OclBuiltinImpl work_group_reduce_min_vector_helper_int16 = OclBuiltinImpl<work_group_reduce_min_helper, work_group_vector_types_int16, 0, work_group_inclusive_function_op_vector, [["$FUNC", "min"]]>;
OclBuiltinImpl work_group_reduce_min_vector_helper_int32 = OclBuiltinImpl<work_group_reduce_min_helper, work_group_vector_types_int32, 0, work_group_inclusive_function_op_vector32, [["$FUNC", "min"]]>;
OclBuiltinImpl work_group_reduce_min_vector_helper_int64 = OclBuiltinImpl<work_group_reduce_min_helper, work_group_vector_types_int64, 0, work_group_inclusive_function_op_vector64, [["$FUNC", "min"]]>;
OclBuiltinImpl work_group_reduce_min_scalar_helper_int = OclBuiltinImpl<work_group_reduce_min_helper, work_group_scalar_types_int, 0, work_group_inclusive_function_op_scalar, [["$FUNC", "min"]]>;

OclBuiltinImpl work_group_reduce_min_vector_helper_float4 = OclBuiltinImpl<work_group_reduce_min_helper, work_group_vector_types_float4, 0, work_group_inclusive_function_op_vector, [["$FUNC", "fmin"]]>;
OclBuiltinImpl work_group_reduce_min_vector_helper_float8 = OclBuiltinImpl<work_group_reduce_min_helper, work_group_vector_types_float8, 0, work_group_inclusive_function_op_vector, [["$FUNC", "fmin"]]>;
OclBuiltinImpl work_group_reduce_min_vector_helper_float16  = OclBuiltinImpl<work_group_reduce_min_helper, work_group_vector_types_float16, 0, work_group_inclusive_function_op_vector, [["$FUNC", "fmin"]]>;
OclBuiltinImpl work_group_reduce_min_vector_helper_float32 = OclBuiltinImpl<work_group_reduce_min_helper, work_group_vector_types_float32, 0, work_group_inclusive_function_op_vector32, [["$FUNC", "fmin"]]>;
OclBuiltinImpl work_group_reduce_min_vector_helper_float64 = OclBuiltinImpl<work_group_reduce_min_helper, work_group_vector_types_float64, 0, work_group_inclusive_function_op_vector64, [["$FUNC", "fmin"]]>;

OclBuiltinImpl work_group_reduce_min_scalar_helper_float = OclBuiltinImpl<work_group_reduce_min_helper, work_group_scalar_types_float, 0, work_group_inclusive_function_op_scalar, [["$FUNC", "fmin"]]>;
OclBuiltinImpl work_group_reduce_max_vector_helper_int4 = OclBuiltinImpl<work_group_reduce_max_helper, work_group_vector_types_int4, 0, work_group_inclusive_function_op_vector, [["$FUNC", "max"]]>;
OclBuiltinImpl work_group_reduce_max_vector_helper_int8 = OclBuiltinImpl<work_group_reduce_max_helper, work_group_vector_types_int8, 0, work_group_inclusive_function_op_vector, [["$FUNC", "max"]]>;
OclBuiltinImpl work_group_reduce_max_vector_helper_int16 = OclBuiltinImpl<work_group_reduce_max_helper, work_group_vector_types_int16, 0, work_group_inclusive_function_op_vector, [["$FUNC", "max"]]>;
OclBuiltinImpl work_group_reduce_max_vector_helper_int32 = OclBuiltinImpl<work_group_reduce_max_helper, work_group_vector_types_int32, 0, work_group_inclusive_function_op_vector32, [["$FUNC", "max"]]>;
OclBuiltinImpl work_group_reduce_max_vector_helper_int64 = OclBuiltinImpl<work_group_reduce_max_helper, work_group_vector_types_int64, 0, work_group_inclusive_function_op_vector64, [["$FUNC", "max"]]>;

OclBuiltinImpl work_group_reduce_max_scalar_helper_int = OclBuiltinImpl<work_group_reduce_max_helper, work_group_scalar_types_int, 0, work_group_inclusive_function_op_scalar, [["$FUNC", "max"]]>;
OclBuiltinImpl work_group_reduce_max_vector_helper_float4 = OclBuiltinImpl<work_group_reduce_max_helper, work_group_vector_types_float4, 0, work_group_inclusive_function_op_vector, [["$FUNC", "fmax"]]>;
OclBuiltinImpl work_group_reduce_max_vector_helper_float8 = OclBuiltinImpl<work_group_reduce_max_helper, work_group_vector_types_float8, 0, work_group_inclusive_function_op_vector, [["$FUNC", "fmax"]]>;
OclBuiltinImpl work_group_reduce_max_vector_helper_float16 = OclBuiltinImpl<work_group_reduce_max_helper, work_group_vector_types_float16, 0, work_group_inclusive_function_op_vector, [["$FUNC", "fmax"]]>;
OclBuiltinImpl work_group_reduce_max_vector_helper_float32 = OclBuiltinImpl<work_group_reduce_max_helper, work_group_vector_types_float32, 0, work_group_inclusive_function_op_vector32, [["$FUNC", "fmax"]]>;
OclBuiltinImpl work_group_reduce_max_vector_helper_float64 = OclBuiltinImpl<work_group_reduce_max_helper, work_group_vector_types_float64, 0, work_group_inclusive_function_op_vector64, [["$FUNC", "fmax"]]>;

OclBuiltinImpl work_group_reduce_max_scalar_helper_float = OclBuiltinImpl<work_group_reduce_max_helper, work_group_scalar_types_float, 0, work_group_inclusive_function_op_scalar, [["$FUNC", "fmax"]]>;

// Masked work group reduce add
//OclBuiltinImpl work_group_reduce_add_mask_helper_generic = OclBuiltinImpl<work_group_reduce_add_helper_mask, work_group_vector_types_all, 0, work_group_inclusive_function_op_mask, [["$FUNC", "work_group_add_util"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_reduce_add_mask_helper_gen4 = OclBuiltinImpl<work_group_reduce_add_helper_mask, work_group_vector_types_all4, 0, work_group_inclusive_function_op_mask, [["$FUNC", "work_group_add_util"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_reduce_add_mask_helper_gen8 = OclBuiltinImpl<work_group_reduce_add_helper_mask, work_group_vector_types_all8, 0, work_group_inclusive_function_op_mask, [["$FUNC", "work_group_add_util"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_reduce_add_mask_helper_gen16 = OclBuiltinImpl<work_group_reduce_add_helper_mask, work_group_vector_types_all16, 0, work_group_inclusive_function_op_mask, [["$FUNC", "work_group_add_util"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_reduce_add_mask_helper_gen32 = OclBuiltinImpl<work_group_reduce_add_helper_mask, work_group_vector_types_all32, 0, work_group_inclusive_function_op_mask, [["$FUNC", "work_group_add_util"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_reduce_add_mask_helper_gen64 = OclBuiltinImpl<work_group_reduce_add_helper_mask, work_group_vector_types_all64, 0, work_group_inclusive_function_op_mask, [["$FUNC", "work_group_add_util"], ["$VAL", "0"]]>;

OclBuiltinImpl work_group_reduce_mul_mask_helper_gen4 = OclBuiltinImpl<work_group_reduce_mul_helper_mask, work_group_vector_types_all4, 0, work_group_inclusive_function_op_mask, [["$FUNC", "work_group_mul_util"], ["$VAL", "1"]]>;
OclBuiltinImpl work_group_reduce_mul_mask_helper_gen8 = OclBuiltinImpl<work_group_reduce_mul_helper_mask, work_group_vector_types_all8, 0, work_group_inclusive_function_op_mask, [["$FUNC", "work_group_mul_util"], ["$VAL", "1"]]>;
OclBuiltinImpl work_group_reduce_mul_mask_helper_gen16 = OclBuiltinImpl<work_group_reduce_mul_helper_mask, work_group_vector_types_all16, 0, work_group_inclusive_function_op_mask, [["$FUNC", "work_group_mul_util"], ["$VAL", "1"]]>;
OclBuiltinImpl work_group_reduce_mul_mask_helper_gen32 = OclBuiltinImpl<work_group_reduce_mul_helper_mask, work_group_vector_types_all32, 0, work_group_inclusive_function_op_mask, [["$FUNC", "work_group_mul_util"], ["$VAL", "1"]]>;
OclBuiltinImpl work_group_reduce_mul_mask_helper_gen64 = OclBuiltinImpl<work_group_reduce_mul_helper_mask, work_group_vector_types_all64, 0, work_group_inclusive_function_op_mask, [["$FUNC", "work_group_mul_util"], ["$VAL", "1"]]>;

// Masked work group reduce max/min
OclBuiltinImpl work_group_reduce_min_mask_helper_i32 = OclBuiltinImpl<work_group_reduce_min_helper_mask, [v4i32, v8i32, v16i32], 0, work_group_inclusive_function_op_mask, [["$FUNC", "min"],["$VAL", "INT_MAX"]]>;
OclBuiltinImpl work_group_reduce_min_mask_helper_i32_v32 = OclBuiltinImpl<work_group_reduce_min_helper_mask, [v32i32], 0, work_group_inclusive_function_op_mask32, [["$FUNC", "min"],["$VAL", "INT_MAX"]]>;
OclBuiltinImpl work_group_reduce_min_mask_helper_i32_v64 = OclBuiltinImpl<work_group_reduce_min_helper_mask, [v64i32], 0, work_group_inclusive_function_op_mask64, [["$FUNC", "min"],["$VAL", "INT_MAX"]]>;

OclBuiltinImpl work_group_reduce_min_mask_helper_u32 = OclBuiltinImpl<work_group_reduce_min_helper_mask, [v4u32, v8u32, v16u32], 0, work_group_inclusive_function_op_mask, [["$FUNC", "min"],["$VAL", "UINT_MAX"]]>;
OclBuiltinImpl work_group_reduce_min_mask_helper_u32_v32 = OclBuiltinImpl<work_group_reduce_min_helper_mask, [v32u32], 0, work_group_inclusive_function_op_mask32, [["$FUNC", "min"],["$VAL", "UINT_MAX"]]>;
OclBuiltinImpl work_group_reduce_min_mask_helper_u32_v64 = OclBuiltinImpl<work_group_reduce_min_helper_mask, [v64u32], 0, work_group_inclusive_function_op_mask64, [["$FUNC", "min"],["$VAL", "UINT_MAX"]]>;

OclBuiltinImpl work_group_reduce_min_mask_helper_i64 = OclBuiltinImpl<work_group_reduce_min_helper_mask, [v4i64, v8i64, v16i64], 0, work_group_inclusive_function_op_mask, [["$FUNC", "min"],["$VAL", "LONG_MAX"]]>;
OclBuiltinImpl work_group_reduce_min_mask_helper_i64_v32 = OclBuiltinImpl<work_group_reduce_min_helper_mask, [v32i64], 0, work_group_inclusive_function_op_mask32, [["$FUNC", "min"],["$VAL", "LONG_MAX"]]>;
OclBuiltinImpl work_group_reduce_min_mask_helper_i64_v64 = OclBuiltinImpl<work_group_reduce_min_helper_mask, [v64i64], 0, work_group_inclusive_function_op_mask64, [["$FUNC", "min"],["$VAL", "LONG_MAX"]]>;

OclBuiltinImpl work_group_reduce_min_mask_helper_u64 = OclBuiltinImpl<work_group_reduce_min_helper_mask, [v4u64, v8u64, v16u64], 0, work_group_inclusive_function_op_mask, [["$FUNC", "min"],["$VAL", "ULONG_MAX"]]>;
OclBuiltinImpl work_group_reduce_min_mask_helper_u64_v32 = OclBuiltinImpl<work_group_reduce_min_helper_mask, [v32u64], 0, work_group_inclusive_function_op_mask32, [["$FUNC", "min"],["$VAL", "ULONG_MAX"]]>;
OclBuiltinImpl work_group_reduce_min_mask_helper_u64_v64 = OclBuiltinImpl<work_group_reduce_min_helper_mask, [v64u64], 0, work_group_inclusive_function_op_mask64, [["$FUNC", "min"],["$VAL", "ULONG_MAX"]]>;

OclBuiltinImpl work_group_reduce_min_mask_helper_f32 = OclBuiltinImpl<work_group_reduce_min_helper_mask, [v4f32, v8f32, v16f32], 0, work_group_inclusive_function_op_mask, [["$FUNC", "fmin"], ["$VAL", "INFINITY"]]>;
OclBuiltinImpl work_group_reduce_min_mask_helper_f32_v32 = OclBuiltinImpl<work_group_reduce_min_helper_mask, [v32f32], 0, work_group_inclusive_function_op_mask32, [["$FUNC", "fmin"], ["$VAL", "INFINITY"]]>;
OclBuiltinImpl work_group_reduce_min_mask_helper_f32_v64 = OclBuiltinImpl<work_group_reduce_min_helper_mask, [v64f32], 0, work_group_inclusive_function_op_mask64, [["$FUNC", "fmin"], ["$VAL", "INFINITY"]]>;

OclBuiltinImpl work_group_reduce_min_mask_helper_f64 = OclBuiltinImpl<work_group_reduce_min_helper_mask, [v4f64, v8f64, v16f64], 0, work_group_inclusive_function_op_mask, [["$FUNC", "fmin"], ["$VAL", "INFINITY"]]>;
OclBuiltinImpl work_group_reduce_min_mask_helper_f64_v32 = OclBuiltinImpl<work_group_reduce_min_helper_mask, [v32f64], 0, work_group_inclusive_function_op_mask32, [["$FUNC", "fmin"], ["$VAL", "INFINITY"]]>;
OclBuiltinImpl work_group_reduce_min_mask_helper_f64_v64 = OclBuiltinImpl<work_group_reduce_min_helper_mask, [v64f64], 0, work_group_inclusive_function_op_mask64, [["$FUNC", "fmin"], ["$VAL", "INFINITY"]]>;

OclBuiltinImpl work_group_reduce_max_mask_helper_i32 = OclBuiltinImpl<work_group_reduce_max_helper_mask, [v4i32, v8i32, v16i32], 0, work_group_inclusive_function_op_mask, [["$FUNC", "max"],["$VAL", "INT_MIN"]]>;
OclBuiltinImpl work_group_reduce_max_mask_helper_i32_v32 = OclBuiltinImpl<work_group_reduce_max_helper_mask, [v32i32], 0, work_group_inclusive_function_op_mask32, [["$FUNC", "max"],["$VAL", "INT_MIN"]]>;
OclBuiltinImpl work_group_reduce_max_mask_helper_i32_v64 = OclBuiltinImpl<work_group_reduce_max_helper_mask, [v64i32], 0, work_group_inclusive_function_op_mask64, [["$FUNC", "max"],["$VAL", "INT_MIN"]]>;

OclBuiltinImpl work_group_reduce_max_mask_helper_u32 = OclBuiltinImpl<work_group_reduce_max_helper_mask, [v4u32, v8u32, v16u32], 0, work_group_inclusive_function_op_mask, [["$FUNC", "max"],["$VAL", "0"]]>;
OclBuiltinImpl work_group_reduce_max_mask_helper_u32_v32 = OclBuiltinImpl<work_group_reduce_max_helper_mask, [v32u32], 0, work_group_inclusive_function_op_mask32, [["$FUNC", "max"],["$VAL", "0"]]>;
OclBuiltinImpl work_group_reduce_max_mask_helper_u32_v64 = OclBuiltinImpl<work_group_reduce_max_helper_mask, [v64u32], 0, work_group_inclusive_function_op_mask64, [["$FUNC", "max"],["$VAL", "0"]]>;

OclBuiltinImpl work_group_reduce_max_mask_helper_i64 = OclBuiltinImpl<work_group_reduce_max_helper_mask, [v4i64, v8i64, v16i64], 0, work_group_inclusive_function_op_mask, [["$FUNC", "max"],["$VAL", "LONG_MIN"]]>;
OclBuiltinImpl work_group_reduce_max_mask_helper_i64_v32 = OclBuiltinImpl<work_group_reduce_max_helper_mask, [v32i64], 0, work_group_inclusive_function_op_mask32, [["$FUNC", "max"],["$VAL", "LONG_MIN"]]>;
OclBuiltinImpl work_group_reduce_max_mask_helper_i64_v64 = OclBuiltinImpl<work_group_reduce_max_helper_mask, [v64i64], 0, work_group_inclusive_function_op_mask64, [["$FUNC", "max"],["$VAL", "LONG_MIN"]]>;

OclBuiltinImpl work_group_reduce_max_mask_helper_u64 = OclBuiltinImpl<work_group_reduce_max_helper_mask, [v4u64, v8u64, v16u64], 0, work_group_inclusive_function_op_mask, [["$FUNC", "max"],["$VAL", "0"]]>;
OclBuiltinImpl work_group_reduce_max_mask_helper_u64_v32 = OclBuiltinImpl<work_group_reduce_max_helper_mask, [v32u64], 0, work_group_inclusive_function_op_mask32, [["$FUNC", "max"],["$VAL", "0"]]>;
OclBuiltinImpl work_group_reduce_max_mask_helper_u64_v64 = OclBuiltinImpl<work_group_reduce_max_helper_mask, [v64u64], 0, work_group_inclusive_function_op_mask64, [["$FUNC", "max"],["$VAL", "0"]]>;

OclBuiltinImpl work_group_reduce_max_mask_helper_f32 = OclBuiltinImpl<work_group_reduce_max_helper_mask, [v4f32, v8f32, v16f32], 0, work_group_inclusive_function_op_mask, [["$FUNC", "fmax"], ["$VAL", "-INFINITY"]]>;
OclBuiltinImpl work_group_reduce_max_mask_helper_f32_v32 = OclBuiltinImpl<work_group_reduce_max_helper_mask, [v32f32], 0, work_group_inclusive_function_op_mask32, [["$FUNC", "fmax"], ["$VAL", "-INFINITY"]]>;
OclBuiltinImpl work_group_reduce_max_mask_helper_f32_v64 = OclBuiltinImpl<work_group_reduce_max_helper_mask, [v64f32], 0, work_group_inclusive_function_op_mask64, [["$FUNC", "fmax"], ["$VAL", "-INFINITY"]]>;

OclBuiltinImpl work_group_reduce_max_mask_helper_f64 = OclBuiltinImpl<work_group_reduce_max_helper_mask, [v4f64, v8f64, v16f64], 0, work_group_inclusive_function_op_mask, [["$FUNC", "fmax"], ["$VAL", "-INFINITY"]]>;
OclBuiltinImpl work_group_reduce_max_mask_helper_f64_v32 = OclBuiltinImpl<work_group_reduce_max_helper_mask, [v32f64], 0, work_group_inclusive_function_op_mask32, [["$FUNC", "fmax"], ["$VAL", "-INFINITY"]]>;
OclBuiltinImpl work_group_reduce_max_mask_helper_f64_v64 = OclBuiltinImpl<work_group_reduce_max_helper_mask, [v64f64], 0, work_group_inclusive_function_op_mask64, [["$FUNC", "fmax"], ["$VAL", "-INFINITY"]]>;

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

//-----------------------------------------------
// 2. Non-uniform result
//-----------------------------------------------

defvar work_group_scan_exclusive_function_op_vector4 = [{
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

defvar work_group_scan_exclusive_function_op_vector8 = [{
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

defvar work_group_scan_exclusive_function_op_vector16 = [{
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

defvar work_group_scan_exclusive_function_op_vector32 = [{
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg1VarName;
    ret.lo.s0 = acc.hi.sF;
    ret.lo.s1 = acc.lo.s0 = $FUNC($Arg0VarName.lo.s0, ret.lo.s0);
    ret.lo.s2 = acc.lo.s1 = $FUNC($Arg0VarName.lo.s1, ret.lo.s1);
    ret.lo.s3 = acc.lo.s2 = $FUNC($Arg0VarName.lo.s2, ret.lo.s2);
    ret.lo.s4 = acc.lo.s3 = $FUNC($Arg0VarName.lo.s3, ret.lo.s3);
    ret.lo.s5 = acc.lo.s4 = $FUNC($Arg0VarName.lo.s4, ret.lo.s4);
    ret.lo.s6 = acc.lo.s5 = $FUNC($Arg0VarName.lo.s5, ret.lo.s5);
    ret.lo.s7 = acc.lo.s6 = $FUNC($Arg0VarName.lo.s6, ret.lo.s6);
    ret.lo.s8 = acc.lo.s7 = $FUNC($Arg0VarName.lo.s7, ret.lo.s7);
    ret.lo.s9 = acc.lo.s8 = $FUNC($Arg0VarName.lo.s8, ret.lo.s8);
    ret.lo.sA = acc.lo.s9 = $FUNC($Arg0VarName.lo.s9, ret.lo.s9);
    ret.lo.sB = acc.lo.sA = $FUNC($Arg0VarName.lo.sA, ret.lo.sA);
    ret.lo.sC = acc.lo.sB = $FUNC($Arg0VarName.lo.sB, ret.lo.sB);
    ret.lo.sD = acc.lo.sC = $FUNC($Arg0VarName.lo.sC, ret.lo.sC);
    ret.lo.sE = acc.lo.sD = $FUNC($Arg0VarName.lo.sD, ret.lo.sD);
    ret.lo.sF = acc.lo.sE = $FUNC($Arg0VarName.lo.sE, ret.lo.sE);

    ret.hi.s0 = acc.lo.sF = $FUNC($Arg0VarName.lo.sF, ret.lo.sF);
    ret.hi.s1 = acc.hi.s0 = $FUNC($Arg0VarName.hi.s0, ret.hi.s0);
    ret.hi.s2 = acc.hi.s1 = $FUNC($Arg0VarName.hi.s1, ret.hi.s1);
    ret.hi.s3 = acc.hi.s2 = $FUNC($Arg0VarName.hi.s2, ret.hi.s2);
    ret.hi.s4 = acc.hi.s3 = $FUNC($Arg0VarName.hi.s3, ret.hi.s3);
    ret.hi.s5 = acc.hi.s4 = $FUNC($Arg0VarName.hi.s4, ret.hi.s4);
    ret.hi.s6 = acc.hi.s5 = $FUNC($Arg0VarName.hi.s5, ret.hi.s5);
    ret.hi.s7 = acc.hi.s6 = $FUNC($Arg0VarName.hi.s6, ret.hi.s6);
    ret.hi.s8 = acc.hi.s7 = $FUNC($Arg0VarName.hi.s7, ret.hi.s7);
    ret.hi.s9 = acc.hi.s8 = $FUNC($Arg0VarName.hi.s8, ret.hi.s8);
    ret.hi.sA = acc.hi.s9 = $FUNC($Arg0VarName.hi.s9, ret.hi.s9);
    ret.hi.sB = acc.hi.sA = $FUNC($Arg0VarName.hi.sA, ret.hi.sA);
    ret.hi.sC = acc.hi.sB = $FUNC($Arg0VarName.hi.sB, ret.hi.sB);
    ret.hi.sD = acc.hi.sC = $FUNC($Arg0VarName.hi.sC, ret.hi.sC);
    ret.hi.sE = acc.hi.sD = $FUNC($Arg0VarName.hi.sD, ret.hi.sD);
    ret.hi.sF = acc.hi.sE = $FUNC($Arg0VarName.hi.sE, ret.hi.sE);
    acc.hi.sF = $FUNC($Arg0VarName.hi.sF, ret.hi.sF);

    *$Arg1VarName = acc;
    return ret;
  }];

defvar work_group_scan_exclusive_function_op_vector64 = [{
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg1VarName;
    ret.lo.lo.s0 = acc.hi.hi.sF;
    ret.lo.lo.s1 = acc.lo.lo.s0 = $FUNC($Arg0VarName.lo.lo.s0, ret.lo.lo.s0);
    ret.lo.lo.s2 = acc.lo.lo.s1 = $FUNC($Arg0VarName.lo.lo.s1, ret.lo.lo.s1);
    ret.lo.lo.s3 = acc.lo.lo.s2 = $FUNC($Arg0VarName.lo.lo.s2, ret.lo.lo.s2);
    ret.lo.lo.s4 = acc.lo.lo.s3 = $FUNC($Arg0VarName.lo.lo.s3, ret.lo.lo.s3);
    ret.lo.lo.s5 = acc.lo.lo.s4 = $FUNC($Arg0VarName.lo.lo.s4, ret.lo.lo.s4);
    ret.lo.lo.s6 = acc.lo.lo.s5 = $FUNC($Arg0VarName.lo.lo.s5, ret.lo.lo.s5);
    ret.lo.lo.s7 = acc.lo.lo.s6 = $FUNC($Arg0VarName.lo.lo.s6, ret.lo.lo.s6);
    ret.lo.lo.s8 = acc.lo.lo.s7 = $FUNC($Arg0VarName.lo.lo.s7, ret.lo.lo.s7);
    ret.lo.lo.s9 = acc.lo.lo.s8 = $FUNC($Arg0VarName.lo.lo.s8, ret.lo.lo.s8);
    ret.lo.lo.sA = acc.lo.lo.s9 = $FUNC($Arg0VarName.lo.lo.s9, ret.lo.lo.s9);
    ret.lo.lo.sB = acc.lo.lo.sA = $FUNC($Arg0VarName.lo.lo.sA, ret.lo.lo.sA);
    ret.lo.lo.sC = acc.lo.lo.sB = $FUNC($Arg0VarName.lo.lo.sB, ret.lo.lo.sB);
    ret.lo.lo.sD = acc.lo.lo.sC = $FUNC($Arg0VarName.lo.lo.sC, ret.lo.lo.sC);
    ret.lo.lo.sE = acc.lo.lo.sD = $FUNC($Arg0VarName.lo.lo.sD, ret.lo.lo.sD);
    ret.lo.lo.sF = acc.lo.lo.sE = $FUNC($Arg0VarName.lo.lo.sE, ret.lo.lo.sE);

    ret.lo.hi.s0 = acc.lo.lo.sF = $FUNC($Arg0VarName.lo.lo.sF, ret.lo.lo.sF);
    ret.lo.hi.s1 = acc.lo.hi.s0 = $FUNC($Arg0VarName.lo.hi.s0, ret.lo.hi.s0);
    ret.lo.hi.s2 = acc.lo.hi.s1 = $FUNC($Arg0VarName.lo.hi.s1, ret.lo.hi.s1);
    ret.lo.hi.s3 = acc.lo.hi.s2 = $FUNC($Arg0VarName.lo.hi.s2, ret.lo.hi.s2);
    ret.lo.hi.s4 = acc.lo.hi.s3 = $FUNC($Arg0VarName.lo.hi.s3, ret.lo.hi.s3);
    ret.lo.hi.s5 = acc.lo.hi.s4 = $FUNC($Arg0VarName.lo.hi.s4, ret.lo.hi.s4);
    ret.lo.hi.s6 = acc.lo.hi.s5 = $FUNC($Arg0VarName.lo.hi.s5, ret.lo.hi.s5);
    ret.lo.hi.s7 = acc.lo.hi.s6 = $FUNC($Arg0VarName.lo.hi.s6, ret.lo.hi.s6);
    ret.lo.hi.s8 = acc.lo.hi.s7 = $FUNC($Arg0VarName.lo.hi.s7, ret.lo.hi.s7);
    ret.lo.hi.s9 = acc.lo.hi.s8 = $FUNC($Arg0VarName.lo.hi.s8, ret.lo.hi.s8);
    ret.lo.hi.sA = acc.lo.hi.s9 = $FUNC($Arg0VarName.lo.hi.s9, ret.lo.hi.s9);
    ret.lo.hi.sB = acc.lo.hi.sA = $FUNC($Arg0VarName.lo.hi.sA, ret.lo.hi.sA);
    ret.lo.hi.sC = acc.lo.hi.sB = $FUNC($Arg0VarName.lo.hi.sB, ret.lo.hi.sB);
    ret.lo.hi.sD = acc.lo.hi.sC = $FUNC($Arg0VarName.lo.hi.sC, ret.lo.hi.sC);
    ret.lo.hi.sE = acc.lo.hi.sD = $FUNC($Arg0VarName.lo.hi.sD, ret.lo.hi.sD);
    ret.lo.hi.sF = acc.lo.hi.sE = $FUNC($Arg0VarName.lo.hi.sE, ret.lo.hi.sE);

    ret.hi.lo.s0 = acc.lo.hi.sF = $FUNC($Arg0VarName.lo.hi.sF, ret.lo.hi.sF);
    ret.hi.lo.s1 = acc.hi.lo.s0 = $FUNC($Arg0VarName.hi.lo.s0, ret.hi.lo.s0);
    ret.hi.lo.s2 = acc.hi.lo.s1 = $FUNC($Arg0VarName.hi.lo.s1, ret.hi.lo.s1);
    ret.hi.lo.s3 = acc.hi.lo.s2 = $FUNC($Arg0VarName.hi.lo.s2, ret.hi.lo.s2);
    ret.hi.lo.s4 = acc.hi.lo.s3 = $FUNC($Arg0VarName.hi.lo.s3, ret.hi.lo.s3);
    ret.hi.lo.s5 = acc.hi.lo.s4 = $FUNC($Arg0VarName.hi.lo.s4, ret.hi.lo.s4);
    ret.hi.lo.s6 = acc.hi.lo.s5 = $FUNC($Arg0VarName.hi.lo.s5, ret.hi.lo.s5);
    ret.hi.lo.s7 = acc.hi.lo.s6 = $FUNC($Arg0VarName.hi.lo.s6, ret.hi.lo.s6);
    ret.hi.lo.s8 = acc.hi.lo.s7 = $FUNC($Arg0VarName.hi.lo.s7, ret.hi.lo.s7);
    ret.hi.lo.s9 = acc.hi.lo.s8 = $FUNC($Arg0VarName.hi.lo.s8, ret.hi.lo.s8);
    ret.hi.lo.sA = acc.hi.lo.s9 = $FUNC($Arg0VarName.hi.lo.s9, ret.hi.lo.s9);
    ret.hi.lo.sB = acc.hi.lo.sA = $FUNC($Arg0VarName.hi.lo.sA, ret.hi.lo.sA);
    ret.hi.lo.sC = acc.hi.lo.sB = $FUNC($Arg0VarName.hi.lo.sB, ret.hi.lo.sB);
    ret.hi.lo.sD = acc.hi.lo.sC = $FUNC($Arg0VarName.hi.lo.sC, ret.hi.lo.sC);
    ret.hi.lo.sE = acc.hi.lo.sD = $FUNC($Arg0VarName.hi.lo.sD, ret.hi.lo.sD);
    ret.hi.lo.sF = acc.hi.lo.sE = $FUNC($Arg0VarName.hi.lo.sE, ret.hi.lo.sE);

    ret.hi.hi.s0 = acc.hi.lo.sF = $FUNC($Arg0VarName.hi.lo.sF, ret.hi.lo.sF);
    ret.hi.hi.s1 = acc.hi.hi.s0 = $FUNC($Arg0VarName.hi.hi.s0, ret.hi.hi.s0);
    ret.hi.hi.s2 = acc.hi.hi.s1 = $FUNC($Arg0VarName.hi.hi.s1, ret.hi.hi.s1);
    ret.hi.hi.s3 = acc.hi.hi.s2 = $FUNC($Arg0VarName.hi.hi.s2, ret.hi.hi.s2);
    ret.hi.hi.s4 = acc.hi.hi.s3 = $FUNC($Arg0VarName.hi.hi.s3, ret.hi.hi.s3);
    ret.hi.hi.s5 = acc.hi.hi.s4 = $FUNC($Arg0VarName.hi.hi.s4, ret.hi.hi.s4);
    ret.hi.hi.s6 = acc.hi.hi.s5 = $FUNC($Arg0VarName.hi.hi.s5, ret.hi.hi.s5);
    ret.hi.hi.s7 = acc.hi.hi.s6 = $FUNC($Arg0VarName.hi.hi.s6, ret.hi.hi.s6);
    ret.hi.hi.s8 = acc.hi.hi.s7 = $FUNC($Arg0VarName.hi.hi.s7, ret.hi.hi.s7);
    ret.hi.hi.s9 = acc.hi.hi.s8 = $FUNC($Arg0VarName.hi.hi.s8, ret.hi.hi.s8);
    ret.hi.hi.sA = acc.hi.hi.s9 = $FUNC($Arg0VarName.hi.hi.s9, ret.hi.hi.s9);
    ret.hi.hi.sB = acc.hi.hi.sA = $FUNC($Arg0VarName.hi.hi.sA, ret.hi.hi.sA);
    ret.hi.hi.sC = acc.hi.hi.sB = $FUNC($Arg0VarName.hi.hi.sB, ret.hi.hi.sB);
    ret.hi.hi.sD = acc.hi.hi.sC = $FUNC($Arg0VarName.hi.hi.sC, ret.hi.hi.sC);
    ret.hi.hi.sE = acc.hi.hi.sD = $FUNC($Arg0VarName.hi.hi.sD, ret.hi.hi.sD);
    ret.hi.hi.sF = acc.hi.hi.sE = $FUNC($Arg0VarName.hi.hi.sE, ret.hi.hi.sE);
    acc.hi.hi.sF = $FUNC($Arg0VarName.hi.hi.sF, ret.hi.hi.sF);

    *$Arg1VarName = acc;
    return ret;
  }];

defvar work_group_scan_inclusive_function_op_vector4 = [{
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg1VarName;
    ret.s0 = $FUNC($Arg0VarName.s0, acc.s3);
    ret.s1 = $FUNC($Arg0VarName.s1, ret.s0);
    ret.s2 = $FUNC($Arg0VarName.s2, ret.s1);
    ret.s3 = $FUNC($Arg0VarName.s3, ret.s2);
    *$Arg1VarName = ret;
    return ret;
  }];

defvar work_group_scan_inclusive_function_op_vector8 = [{
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

defvar work_group_scan_inclusive_function_op_vector16 = [{
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

defvar work_group_scan_inclusive_function_op_vector32 = [{
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg1VarName;
    ret.lo.s0 = $FUNC($Arg0VarName.lo.s0, acc.hi.sF);
    ret.lo.s1 = $FUNC($Arg0VarName.lo.s1, ret.lo.s0);
    ret.lo.s2 = $FUNC($Arg0VarName.lo.s2, ret.lo.s1);
    ret.lo.s3 = $FUNC($Arg0VarName.lo.s3, ret.lo.s2);
    ret.lo.s4 = $FUNC($Arg0VarName.lo.s4, ret.lo.s3);
    ret.lo.s5 = $FUNC($Arg0VarName.lo.s5, ret.lo.s4);
    ret.lo.s6 = $FUNC($Arg0VarName.lo.s6, ret.lo.s5);
    ret.lo.s7 = $FUNC($Arg0VarName.lo.s7, ret.lo.s6);
    ret.lo.s8 = $FUNC($Arg0VarName.lo.s8, ret.lo.s7);
    ret.lo.s9 = $FUNC($Arg0VarName.lo.s9, ret.lo.s8);
    ret.lo.sA = $FUNC($Arg0VarName.lo.sA, ret.lo.s9);
    ret.lo.sB = $FUNC($Arg0VarName.lo.sB, ret.lo.sA);
    ret.lo.sC = $FUNC($Arg0VarName.lo.sC, ret.lo.sB);
    ret.lo.sD = $FUNC($Arg0VarName.lo.sD, ret.lo.sC);
    ret.lo.sE = $FUNC($Arg0VarName.lo.sE, ret.lo.sD);
    ret.lo.sF = $FUNC($Arg0VarName.lo.sF, ret.lo.sE);

    ret.hi.s0 = $FUNC($Arg0VarName.hi.s0, ret.lo.sF);
    ret.hi.s1 = $FUNC($Arg0VarName.hi.s1, ret.hi.s0);
    ret.hi.s2 = $FUNC($Arg0VarName.hi.s2, ret.hi.s1);
    ret.hi.s3 = $FUNC($Arg0VarName.hi.s3, ret.hi.s2);
    ret.hi.s4 = $FUNC($Arg0VarName.hi.s4, ret.hi.s3);
    ret.hi.s5 = $FUNC($Arg0VarName.hi.s5, ret.hi.s4);
    ret.hi.s6 = $FUNC($Arg0VarName.hi.s6, ret.hi.s5);
    ret.hi.s7 = $FUNC($Arg0VarName.hi.s7, ret.hi.s6);
    ret.hi.s8 = $FUNC($Arg0VarName.hi.s8, ret.hi.s7);
    ret.hi.s9 = $FUNC($Arg0VarName.hi.s9, ret.hi.s8);
    ret.hi.sA = $FUNC($Arg0VarName.hi.sA, ret.hi.s9);
    ret.hi.sB = $FUNC($Arg0VarName.hi.sB, ret.hi.sA);
    ret.hi.sC = $FUNC($Arg0VarName.hi.sC, ret.hi.sB);
    ret.hi.sD = $FUNC($Arg0VarName.hi.sD, ret.hi.sC);
    ret.hi.sE = $FUNC($Arg0VarName.hi.sE, ret.hi.sD);
    ret.hi.sF = $FUNC($Arg0VarName.hi.sF, ret.hi.sE);

    *$Arg1VarName = ret;
    return ret;
  }];

defvar work_group_scan_inclusive_function_op_vector64 = [{
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg1VarName;
    ret.lo.lo.s0 = $FUNC($Arg0VarName.lo.lo.s0, acc.hi.hi.sF);
    ret.lo.lo.s1 = $FUNC($Arg0VarName.lo.lo.s1, ret.lo.lo.s0);
    ret.lo.lo.s2 = $FUNC($Arg0VarName.lo.lo.s2, ret.lo.lo.s1);
    ret.lo.lo.s3 = $FUNC($Arg0VarName.lo.lo.s3, ret.lo.lo.s2);
    ret.lo.lo.s4 = $FUNC($Arg0VarName.lo.lo.s4, ret.lo.lo.s3);
    ret.lo.lo.s5 = $FUNC($Arg0VarName.lo.lo.s5, ret.lo.lo.s4);
    ret.lo.lo.s6 = $FUNC($Arg0VarName.lo.lo.s6, ret.lo.lo.s5);
    ret.lo.lo.s7 = $FUNC($Arg0VarName.lo.lo.s7, ret.lo.lo.s6);
    ret.lo.lo.s8 = $FUNC($Arg0VarName.lo.lo.s8, ret.lo.lo.s7);
    ret.lo.lo.s9 = $FUNC($Arg0VarName.lo.lo.s9, ret.lo.lo.s8);
    ret.lo.lo.sA = $FUNC($Arg0VarName.lo.lo.sA, ret.lo.lo.s9);
    ret.lo.lo.sB = $FUNC($Arg0VarName.lo.lo.sB, ret.lo.lo.sA);
    ret.lo.lo.sC = $FUNC($Arg0VarName.lo.lo.sC, ret.lo.lo.sB);
    ret.lo.lo.sD = $FUNC($Arg0VarName.lo.lo.sD, ret.lo.lo.sC);
    ret.lo.lo.sE = $FUNC($Arg0VarName.lo.lo.sE, ret.lo.lo.sD);
    ret.lo.lo.sF = $FUNC($Arg0VarName.lo.lo.sF, ret.lo.lo.sE);

    ret.lo.hi.s0 = $FUNC($Arg0VarName.lo.hi.s0, ret.lo.lo.sF);
    ret.lo.hi.s1 = $FUNC($Arg0VarName.lo.hi.s1, ret.lo.hi.s0);
    ret.lo.hi.s2 = $FUNC($Arg0VarName.lo.hi.s2, ret.lo.hi.s1);
    ret.lo.hi.s3 = $FUNC($Arg0VarName.lo.hi.s3, ret.lo.hi.s2);
    ret.lo.hi.s4 = $FUNC($Arg0VarName.lo.hi.s4, ret.lo.hi.s3);
    ret.lo.hi.s5 = $FUNC($Arg0VarName.lo.hi.s5, ret.lo.hi.s4);
    ret.lo.hi.s6 = $FUNC($Arg0VarName.lo.hi.s6, ret.lo.hi.s5);
    ret.lo.hi.s7 = $FUNC($Arg0VarName.lo.hi.s7, ret.lo.hi.s6);
    ret.lo.hi.s8 = $FUNC($Arg0VarName.lo.hi.s8, ret.lo.hi.s7);
    ret.lo.hi.s9 = $FUNC($Arg0VarName.lo.hi.s9, ret.lo.hi.s8);
    ret.lo.hi.sA = $FUNC($Arg0VarName.lo.hi.sA, ret.lo.hi.s9);
    ret.lo.hi.sB = $FUNC($Arg0VarName.lo.hi.sB, ret.lo.hi.sA);
    ret.lo.hi.sC = $FUNC($Arg0VarName.lo.hi.sC, ret.lo.hi.sB);
    ret.lo.hi.sD = $FUNC($Arg0VarName.lo.hi.sD, ret.lo.hi.sC);
    ret.lo.hi.sE = $FUNC($Arg0VarName.lo.hi.sE, ret.lo.hi.sD);
    ret.lo.hi.sF = $FUNC($Arg0VarName.lo.hi.sF, ret.lo.hi.sE);

    ret.hi.lo.s0 = $FUNC($Arg0VarName.hi.lo.s0, ret.lo.hi.sF);
    ret.hi.lo.s1 = $FUNC($Arg0VarName.hi.lo.s1, ret.hi.lo.s0);
    ret.hi.lo.s2 = $FUNC($Arg0VarName.hi.lo.s2, ret.hi.lo.s1);
    ret.hi.lo.s3 = $FUNC($Arg0VarName.hi.lo.s3, ret.hi.lo.s2);
    ret.hi.lo.s4 = $FUNC($Arg0VarName.hi.lo.s4, ret.hi.lo.s3);
    ret.hi.lo.s5 = $FUNC($Arg0VarName.hi.lo.s5, ret.hi.lo.s4);
    ret.hi.lo.s6 = $FUNC($Arg0VarName.hi.lo.s6, ret.hi.lo.s5);
    ret.hi.lo.s7 = $FUNC($Arg0VarName.hi.lo.s7, ret.hi.lo.s6);
    ret.hi.lo.s8 = $FUNC($Arg0VarName.hi.lo.s8, ret.hi.lo.s7);
    ret.hi.lo.s9 = $FUNC($Arg0VarName.hi.lo.s9, ret.hi.lo.s8);
    ret.hi.lo.sA = $FUNC($Arg0VarName.hi.lo.sA, ret.hi.lo.s9);
    ret.hi.lo.sB = $FUNC($Arg0VarName.hi.lo.sB, ret.hi.lo.sA);
    ret.hi.lo.sC = $FUNC($Arg0VarName.hi.lo.sC, ret.hi.lo.sB);
    ret.hi.lo.sD = $FUNC($Arg0VarName.hi.lo.sD, ret.hi.lo.sC);
    ret.hi.lo.sE = $FUNC($Arg0VarName.hi.lo.sE, ret.hi.lo.sD);
    ret.hi.lo.sF = $FUNC($Arg0VarName.hi.lo.sF, ret.hi.lo.sE);

    ret.hi.hi.s0 = $FUNC($Arg0VarName.hi.hi.s0, ret.hi.lo.sF);
    ret.hi.hi.s1 = $FUNC($Arg0VarName.hi.hi.s1, ret.hi.hi.s0);
    ret.hi.hi.s2 = $FUNC($Arg0VarName.hi.hi.s2, ret.hi.hi.s1);
    ret.hi.hi.s3 = $FUNC($Arg0VarName.hi.hi.s3, ret.hi.hi.s2);
    ret.hi.hi.s4 = $FUNC($Arg0VarName.hi.hi.s4, ret.hi.hi.s3);
    ret.hi.hi.s5 = $FUNC($Arg0VarName.hi.hi.s5, ret.hi.hi.s4);
    ret.hi.hi.s6 = $FUNC($Arg0VarName.hi.hi.s6, ret.hi.hi.s5);
    ret.hi.hi.s7 = $FUNC($Arg0VarName.hi.hi.s7, ret.hi.hi.s6);
    ret.hi.hi.s8 = $FUNC($Arg0VarName.hi.hi.s8, ret.hi.hi.s7);
    ret.hi.hi.s9 = $FUNC($Arg0VarName.hi.hi.s9, ret.hi.hi.s8);
    ret.hi.hi.sA = $FUNC($Arg0VarName.hi.hi.sA, ret.hi.hi.s9);
    ret.hi.hi.sB = $FUNC($Arg0VarName.hi.hi.sB, ret.hi.hi.sA);
    ret.hi.hi.sC = $FUNC($Arg0VarName.hi.hi.sC, ret.hi.hi.sB);
    ret.hi.hi.sD = $FUNC($Arg0VarName.hi.hi.sD, ret.hi.hi.sC);
    ret.hi.hi.sE = $FUNC($Arg0VarName.hi.hi.sE, ret.hi.hi.sD);
    ret.hi.hi.sF = $FUNC($Arg0VarName.hi.hi.sF, ret.hi.hi.sE);

    *$Arg1VarName = ret;
    return ret;
  }];


defvar work_group_scan_exclusive_function_op_mask4 = [{

    $MaskCastTy#4 arg0 = __builtin_astype($Arg0VarName, $MaskCastTy#4);
    $MaskCastTy#4 arg1 = __builtin_convertvector(__builtin_astype($Arg1VarName, int4), $MaskCastTy#4);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);

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

defvar work_group_scan_exclusive_function_op_mask8 = [{
    $MaskCastTy#8 arg0 = __builtin_astype($Arg0VarName, $MaskCastTy#8);
    $MaskCastTy#8 arg1 = __builtin_convertvector(__builtin_astype($Arg1VarName, int8), $MaskCastTy#8);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);
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

defvar work_group_scan_exclusive_function_op_mask16 = [{
    $MaskCastTy#16 arg0 = __builtin_astype($Arg0VarName, $MaskCastTy#16);
    $MaskCastTy#16 arg1 = __builtin_convertvector(__builtin_astype($Arg1VarName, int16), $MaskCastTy#16);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);
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

defvar work_group_scan_exclusive_function_op_mask32 = [{
    $MaskCastTy#32 arg0 = __builtin_astype($Arg0VarName, $MaskCastTy#32);
    $MaskCastTy#32 arg1 = __builtin_convertvector(__builtin_astype($Arg1VarName, int32), $MaskCastTy#32);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg2VarName;
    ret.lo.s0 = acc.hi.sF;
    ret.lo.s1 = acc.lo.s0 = $FUNC($Arg0VarName.lo.s0, ret.lo.s0);
    ret.lo.s2 = acc.lo.s1 = $FUNC($Arg0VarName.lo.s1, ret.lo.s1);
    ret.lo.s3 = acc.lo.s2 = $FUNC($Arg0VarName.lo.s2, ret.lo.s2);
    ret.lo.s4 = acc.lo.s3 = $FUNC($Arg0VarName.lo.s3, ret.lo.s3);
    ret.lo.s5 = acc.lo.s4 = $FUNC($Arg0VarName.lo.s4, ret.lo.s4);
    ret.lo.s6 = acc.lo.s5 = $FUNC($Arg0VarName.lo.s5, ret.lo.s5);
    ret.lo.s7 = acc.lo.s6 = $FUNC($Arg0VarName.lo.s6, ret.lo.s6);
    ret.lo.s8 = acc.lo.s7 = $FUNC($Arg0VarName.lo.s7, ret.lo.s7);
    ret.lo.s9 = acc.lo.s8 = $FUNC($Arg0VarName.lo.s8, ret.lo.s8);
    ret.lo.sA = acc.lo.s9 = $FUNC($Arg0VarName.lo.s9, ret.lo.s9);
    ret.lo.sB = acc.lo.sA = $FUNC($Arg0VarName.lo.sA, ret.lo.sA);
    ret.lo.sC = acc.lo.sB = $FUNC($Arg0VarName.lo.sB, ret.lo.sB);
    ret.lo.sD = acc.lo.sC = $FUNC($Arg0VarName.lo.sC, ret.lo.sC);
    ret.lo.sE = acc.lo.sD = $FUNC($Arg0VarName.lo.sD, ret.lo.sD);
    ret.lo.sF = acc.lo.sE = $FUNC($Arg0VarName.lo.sE, ret.lo.sE);

    ret.hi.s0 = acc.lo.sF = $FUNC($Arg0VarName.lo.sF, ret.lo.sF);
    ret.hi.s1 = acc.hi.s0 = $FUNC($Arg0VarName.hi.s0, ret.hi.s0);
    ret.hi.s2 = acc.hi.s1 = $FUNC($Arg0VarName.hi.s1, ret.hi.s1);
    ret.hi.s3 = acc.hi.s2 = $FUNC($Arg0VarName.hi.s2, ret.hi.s2);
    ret.hi.s4 = acc.hi.s3 = $FUNC($Arg0VarName.hi.s3, ret.hi.s3);
    ret.hi.s5 = acc.hi.s4 = $FUNC($Arg0VarName.hi.s4, ret.hi.s4);
    ret.hi.s6 = acc.hi.s5 = $FUNC($Arg0VarName.hi.s5, ret.hi.s5);
    ret.hi.s7 = acc.hi.s6 = $FUNC($Arg0VarName.hi.s6, ret.hi.s6);
    ret.hi.s8 = acc.hi.s7 = $FUNC($Arg0VarName.hi.s7, ret.hi.s7);
    ret.hi.s9 = acc.hi.s8 = $FUNC($Arg0VarName.hi.s8, ret.hi.s8);
    ret.hi.sA = acc.hi.s9 = $FUNC($Arg0VarName.hi.s9, ret.hi.s9);
    ret.hi.sB = acc.hi.sA = $FUNC($Arg0VarName.hi.sA, ret.hi.sA);
    ret.hi.sC = acc.hi.sB = $FUNC($Arg0VarName.hi.sB, ret.hi.sB);
    ret.hi.sD = acc.hi.sC = $FUNC($Arg0VarName.hi.sC, ret.hi.sC);
    ret.hi.sE = acc.hi.sD = $FUNC($Arg0VarName.hi.sD, ret.hi.sD);
    ret.hi.sF = acc.hi.sE = $FUNC($Arg0VarName.hi.sE, ret.hi.sE);

    acc.hi.sF = $FUNC($Arg0VarName.hi.sF, ret.hi.sF);
    *$Arg2VarName = acc;
    return ret;
  }];

defvar work_group_scan_exclusive_function_op_mask64 = [{
    $MaskCastTy#64 arg0 = __builtin_astype($Arg0VarName, $MaskCastTy#64);
    $MaskCastTy#64 arg1 = __builtin_convertvector(__builtin_astype($Arg1VarName, int64), $MaskCastTy#64);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg2VarName;
    ret.lo.lo.s0 = acc.hi.hi.sF;
    ret.lo.lo.s1 = acc.lo.lo.s0 = $FUNC($Arg0VarName.lo.lo.s0, ret.lo.lo.s0);
    ret.lo.lo.s2 = acc.lo.lo.s1 = $FUNC($Arg0VarName.lo.lo.s1, ret.lo.lo.s1);
    ret.lo.lo.s3 = acc.lo.lo.s2 = $FUNC($Arg0VarName.lo.lo.s2, ret.lo.lo.s2);
    ret.lo.lo.s4 = acc.lo.lo.s3 = $FUNC($Arg0VarName.lo.lo.s3, ret.lo.lo.s3);
    ret.lo.lo.s5 = acc.lo.lo.s4 = $FUNC($Arg0VarName.lo.lo.s4, ret.lo.lo.s4);
    ret.lo.lo.s6 = acc.lo.lo.s5 = $FUNC($Arg0VarName.lo.lo.s5, ret.lo.lo.s5);
    ret.lo.lo.s7 = acc.lo.lo.s6 = $FUNC($Arg0VarName.lo.lo.s6, ret.lo.lo.s6);
    ret.lo.lo.s8 = acc.lo.lo.s7 = $FUNC($Arg0VarName.lo.lo.s7, ret.lo.lo.s7);
    ret.lo.lo.s9 = acc.lo.lo.s8 = $FUNC($Arg0VarName.lo.lo.s8, ret.lo.lo.s8);
    ret.lo.lo.sA = acc.lo.lo.s9 = $FUNC($Arg0VarName.lo.lo.s9, ret.lo.lo.s9);
    ret.lo.lo.sB = acc.lo.lo.sA = $FUNC($Arg0VarName.lo.lo.sA, ret.lo.lo.sA);
    ret.lo.lo.sC = acc.lo.lo.sB = $FUNC($Arg0VarName.lo.lo.sB, ret.lo.lo.sB);
    ret.lo.lo.sD = acc.lo.lo.sC = $FUNC($Arg0VarName.lo.lo.sC, ret.lo.lo.sC);
    ret.lo.lo.sE = acc.lo.lo.sD = $FUNC($Arg0VarName.lo.lo.sD, ret.lo.lo.sD);
    ret.lo.lo.sF = acc.lo.lo.sE = $FUNC($Arg0VarName.lo.lo.sE, ret.lo.lo.sE);

    ret.lo.hi.s0 = acc.lo.lo.sF = $FUNC($Arg0VarName.lo.lo.sF, ret.lo.lo.sF);
    ret.lo.hi.s1 = acc.lo.hi.s0 = $FUNC($Arg0VarName.lo.hi.s0, ret.lo.hi.s0);
    ret.lo.hi.s2 = acc.lo.hi.s1 = $FUNC($Arg0VarName.lo.hi.s1, ret.lo.hi.s1);
    ret.lo.hi.s3 = acc.lo.hi.s2 = $FUNC($Arg0VarName.lo.hi.s2, ret.lo.hi.s2);
    ret.lo.hi.s4 = acc.lo.hi.s3 = $FUNC($Arg0VarName.lo.hi.s3, ret.lo.hi.s3);
    ret.lo.hi.s5 = acc.lo.hi.s4 = $FUNC($Arg0VarName.lo.hi.s4, ret.lo.hi.s4);
    ret.lo.hi.s6 = acc.lo.hi.s5 = $FUNC($Arg0VarName.lo.hi.s5, ret.lo.hi.s5);
    ret.lo.hi.s7 = acc.lo.hi.s6 = $FUNC($Arg0VarName.lo.hi.s6, ret.lo.hi.s6);
    ret.lo.hi.s8 = acc.lo.hi.s7 = $FUNC($Arg0VarName.lo.hi.s7, ret.lo.hi.s7);
    ret.lo.hi.s9 = acc.lo.hi.s8 = $FUNC($Arg0VarName.lo.hi.s8, ret.lo.hi.s8);
    ret.lo.hi.sA = acc.lo.hi.s9 = $FUNC($Arg0VarName.lo.hi.s9, ret.lo.hi.s9);
    ret.lo.hi.sB = acc.lo.hi.sA = $FUNC($Arg0VarName.lo.hi.sA, ret.lo.hi.sA);
    ret.lo.hi.sC = acc.lo.hi.sB = $FUNC($Arg0VarName.lo.hi.sB, ret.lo.hi.sB);
    ret.lo.hi.sD = acc.lo.hi.sC = $FUNC($Arg0VarName.lo.hi.sC, ret.lo.hi.sC);
    ret.lo.hi.sE = acc.lo.hi.sD = $FUNC($Arg0VarName.lo.hi.sD, ret.lo.hi.sD);
    ret.lo.hi.sF = acc.lo.hi.sE = $FUNC($Arg0VarName.lo.hi.sE, ret.lo.hi.sE);

    ret.hi.lo.s0 = acc.lo.hi.sF = $FUNC($Arg0VarName.lo.hi.sF, ret.lo.hi.sF);
    ret.hi.lo.s1 = acc.hi.lo.s0 = $FUNC($Arg0VarName.hi.lo.s0, ret.hi.lo.s0);
    ret.hi.lo.s2 = acc.hi.lo.s1 = $FUNC($Arg0VarName.hi.lo.s1, ret.hi.lo.s1);
    ret.hi.lo.s3 = acc.hi.lo.s2 = $FUNC($Arg0VarName.hi.lo.s2, ret.hi.lo.s2);
    ret.hi.lo.s4 = acc.hi.lo.s3 = $FUNC($Arg0VarName.hi.lo.s3, ret.hi.lo.s3);
    ret.hi.lo.s5 = acc.hi.lo.s4 = $FUNC($Arg0VarName.hi.lo.s4, ret.hi.lo.s4);
    ret.hi.lo.s6 = acc.hi.lo.s5 = $FUNC($Arg0VarName.hi.lo.s5, ret.hi.lo.s5);
    ret.hi.lo.s7 = acc.hi.lo.s6 = $FUNC($Arg0VarName.hi.lo.s6, ret.hi.lo.s6);
    ret.hi.lo.s8 = acc.hi.lo.s7 = $FUNC($Arg0VarName.hi.lo.s7, ret.hi.lo.s7);
    ret.hi.lo.s9 = acc.hi.lo.s8 = $FUNC($Arg0VarName.hi.lo.s8, ret.hi.lo.s8);
    ret.hi.lo.sA = acc.hi.lo.s9 = $FUNC($Arg0VarName.hi.lo.s9, ret.hi.lo.s9);
    ret.hi.lo.sB = acc.hi.lo.sA = $FUNC($Arg0VarName.hi.lo.sA, ret.hi.lo.sA);
    ret.hi.lo.sC = acc.hi.lo.sB = $FUNC($Arg0VarName.hi.lo.sB, ret.hi.lo.sB);
    ret.hi.lo.sD = acc.hi.lo.sC = $FUNC($Arg0VarName.hi.lo.sC, ret.hi.lo.sC);
    ret.hi.lo.sE = acc.hi.lo.sD = $FUNC($Arg0VarName.hi.lo.sD, ret.hi.lo.sD);
    ret.hi.lo.sF = acc.hi.lo.sE = $FUNC($Arg0VarName.hi.lo.sE, ret.hi.lo.sE);

    ret.hi.hi.s0 = acc.hi.lo.sF = $FUNC($Arg0VarName.hi.lo.sF, ret.hi.lo.sF);
    ret.hi.hi.s1 = acc.hi.hi.s0 = $FUNC($Arg0VarName.hi.hi.s0, ret.hi.hi.s0);
    ret.hi.hi.s2 = acc.hi.hi.s1 = $FUNC($Arg0VarName.hi.hi.s1, ret.hi.hi.s1);
    ret.hi.hi.s3 = acc.hi.hi.s2 = $FUNC($Arg0VarName.hi.hi.s2, ret.hi.hi.s2);
    ret.hi.hi.s4 = acc.hi.hi.s3 = $FUNC($Arg0VarName.hi.hi.s3, ret.hi.hi.s3);
    ret.hi.hi.s5 = acc.hi.hi.s4 = $FUNC($Arg0VarName.hi.hi.s4, ret.hi.hi.s4);
    ret.hi.hi.s6 = acc.hi.hi.s5 = $FUNC($Arg0VarName.hi.hi.s5, ret.hi.hi.s5);
    ret.hi.hi.s7 = acc.hi.hi.s6 = $FUNC($Arg0VarName.hi.hi.s6, ret.hi.hi.s6);
    ret.hi.hi.s8 = acc.hi.hi.s7 = $FUNC($Arg0VarName.hi.hi.s7, ret.hi.hi.s7);
    ret.hi.hi.s9 = acc.hi.hi.s8 = $FUNC($Arg0VarName.hi.hi.s8, ret.hi.hi.s8);
    ret.hi.hi.sA = acc.hi.hi.s9 = $FUNC($Arg0VarName.hi.hi.s9, ret.hi.hi.s9);
    ret.hi.hi.sB = acc.hi.hi.sA = $FUNC($Arg0VarName.hi.hi.sA, ret.hi.hi.sA);
    ret.hi.hi.sC = acc.hi.hi.sB = $FUNC($Arg0VarName.hi.hi.sB, ret.hi.hi.sB);
    ret.hi.hi.sD = acc.hi.hi.sC = $FUNC($Arg0VarName.hi.hi.sC, ret.hi.hi.sC);
    ret.hi.hi.sE = acc.hi.hi.sD = $FUNC($Arg0VarName.hi.hi.sD, ret.hi.hi.sD);
    ret.hi.hi.sF = acc.hi.hi.sE = $FUNC($Arg0VarName.hi.hi.sE, ret.hi.hi.sE);

    acc.hi.hi.sF = $FUNC($Arg0VarName.hi.hi.sF, ret.hi.hi.sF);
    *$Arg2VarName = acc;
    return ret;
  }];

defvar work_group_scan_inclusive_function_op_mask4 = [{
    $MaskCastTy#4 arg0 = __builtin_astype($Arg0VarName, $MaskCastTy#4);
    $MaskCastTy#4 arg1 = __builtin_convertvector(__builtin_astype($Arg1VarName, int4), $MaskCastTy#4);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg2VarName;
    ret.s0 = $FUNC($Arg0VarName.s0, acc.s3);
    ret.s1 = $FUNC($Arg0VarName.s1, ret.s0);
    ret.s2 = $FUNC($Arg0VarName.s2, ret.s1);
    ret.s3 = $FUNC($Arg0VarName.s3, ret.s2);
    *$Arg2VarName = ret;
    return ret;
  }];

defvar work_group_scan_inclusive_function_op_mask8 = [{
    $MaskCastTy#8 arg0 = __builtin_astype($Arg0VarName, $MaskCastTy#8);
    $MaskCastTy#8 arg1 = __builtin_convertvector(__builtin_astype($Arg1VarName, int8), $MaskCastTy#8);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);
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

defvar work_group_scan_inclusive_function_op_mask16 = [{
    $MaskCastTy#16 arg0 = __builtin_astype($Arg0VarName, $MaskCastTy#16);
    $MaskCastTy#16 arg1 = __builtin_convertvector(__builtin_astype($Arg1VarName, int16), $MaskCastTy#16);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);
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

defvar work_group_scan_inclusive_function_op_mask32 = [{
    $MaskCastTy#32 arg0 = __builtin_astype($Arg0VarName, $MaskCastTy#32);
    $MaskCastTy#32 arg1 = __builtin_convertvector(__builtin_astype($Arg1VarName, int32), $MaskCastTy#32);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg2VarName;
    ret.lo.s0 = $FUNC($Arg0VarName.lo.s0, acc.hi.sF);
    ret.lo.s1 = $FUNC($Arg0VarName.lo.s1, ret.lo.s0);
    ret.lo.s2 = $FUNC($Arg0VarName.lo.s2, ret.lo.s1);
    ret.lo.s3 = $FUNC($Arg0VarName.lo.s3, ret.lo.s2);
    ret.lo.s4 = $FUNC($Arg0VarName.lo.s4, ret.lo.s3);
    ret.lo.s5 = $FUNC($Arg0VarName.lo.s5, ret.lo.s4);
    ret.lo.s6 = $FUNC($Arg0VarName.lo.s6, ret.lo.s5);
    ret.lo.s7 = $FUNC($Arg0VarName.lo.s7, ret.lo.s6);
    ret.lo.s8 = $FUNC($Arg0VarName.lo.s8, ret.lo.s7);
    ret.lo.s9 = $FUNC($Arg0VarName.lo.s9, ret.lo.s8);
    ret.lo.sA = $FUNC($Arg0VarName.lo.sA, ret.lo.s9);
    ret.lo.sB = $FUNC($Arg0VarName.lo.sB, ret.lo.sA);
    ret.lo.sC = $FUNC($Arg0VarName.lo.sC, ret.lo.sB);
    ret.lo.sD = $FUNC($Arg0VarName.lo.sD, ret.lo.sC);
    ret.lo.sE = $FUNC($Arg0VarName.lo.sE, ret.lo.sD);
    ret.lo.sF = $FUNC($Arg0VarName.lo.sF, ret.lo.sE);

    ret.hi.s0 = $FUNC($Arg0VarName.hi.s0, ret.lo.sF);
    ret.hi.s1 = $FUNC($Arg0VarName.hi.s1, ret.hi.s0);
    ret.hi.s2 = $FUNC($Arg0VarName.hi.s2, ret.hi.s1);
    ret.hi.s3 = $FUNC($Arg0VarName.hi.s3, ret.hi.s2);
    ret.hi.s4 = $FUNC($Arg0VarName.hi.s4, ret.hi.s3);
    ret.hi.s5 = $FUNC($Arg0VarName.hi.s5, ret.hi.s4);
    ret.hi.s6 = $FUNC($Arg0VarName.hi.s6, ret.hi.s5);
    ret.hi.s7 = $FUNC($Arg0VarName.hi.s7, ret.hi.s6);
    ret.hi.s8 = $FUNC($Arg0VarName.hi.s8, ret.hi.s7);
    ret.hi.s9 = $FUNC($Arg0VarName.hi.s9, ret.hi.s8);
    ret.hi.sA = $FUNC($Arg0VarName.hi.sA, ret.hi.s9);
    ret.hi.sB = $FUNC($Arg0VarName.hi.sB, ret.hi.sA);
    ret.hi.sC = $FUNC($Arg0VarName.hi.sC, ret.hi.sB);
    ret.hi.sD = $FUNC($Arg0VarName.hi.sD, ret.hi.sC);
    ret.hi.sE = $FUNC($Arg0VarName.hi.sE, ret.hi.sD);
    ret.hi.sF = $FUNC($Arg0VarName.hi.sF, ret.hi.sE);

    *$Arg2VarName = ret;
    return ret;
  }];

defvar work_group_scan_inclusive_function_op_mask64 = [{
    $MaskCastTy#64 arg0 = __builtin_astype($Arg0VarName, $MaskCastTy#64);
    $MaskCastTy#64 arg1 = __builtin_convertvector(__builtin_astype($Arg1VarName, int64), $MaskCastTy#64);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg2VarName;
    ret.lo.lo.s0 = $FUNC($Arg0VarName.lo.lo.s0, acc.hi.hi.sF);
    ret.lo.lo.s1 = $FUNC($Arg0VarName.lo.lo.s1, ret.lo.lo.s0);
    ret.lo.lo.s2 = $FUNC($Arg0VarName.lo.lo.s2, ret.lo.lo.s1);
    ret.lo.lo.s3 = $FUNC($Arg0VarName.lo.lo.s3, ret.lo.lo.s2);
    ret.lo.lo.s4 = $FUNC($Arg0VarName.lo.lo.s4, ret.lo.lo.s3);
    ret.lo.lo.s5 = $FUNC($Arg0VarName.lo.lo.s5, ret.lo.lo.s4);
    ret.lo.lo.s6 = $FUNC($Arg0VarName.lo.lo.s6, ret.lo.lo.s5);
    ret.lo.lo.s7 = $FUNC($Arg0VarName.lo.lo.s7, ret.lo.lo.s6);
    ret.lo.lo.s8 = $FUNC($Arg0VarName.lo.lo.s8, ret.lo.lo.s7);
    ret.lo.lo.s9 = $FUNC($Arg0VarName.lo.lo.s9, ret.lo.lo.s8);
    ret.lo.lo.sA = $FUNC($Arg0VarName.lo.lo.sA, ret.lo.lo.s9);
    ret.lo.lo.sB = $FUNC($Arg0VarName.lo.lo.sB, ret.lo.lo.sA);
    ret.lo.lo.sC = $FUNC($Arg0VarName.lo.lo.sC, ret.lo.lo.sB);
    ret.lo.lo.sD = $FUNC($Arg0VarName.lo.lo.sD, ret.lo.lo.sC);
    ret.lo.lo.sE = $FUNC($Arg0VarName.lo.lo.sE, ret.lo.lo.sD);
    ret.lo.lo.sF = $FUNC($Arg0VarName.lo.lo.sF, ret.lo.lo.sE);

    ret.lo.hi.s0 = $FUNC($Arg0VarName.lo.hi.s0, ret.lo.lo.sF);
    ret.lo.hi.s1 = $FUNC($Arg0VarName.lo.hi.s1, ret.lo.hi.s0);
    ret.lo.hi.s2 = $FUNC($Arg0VarName.lo.hi.s2, ret.lo.hi.s1);
    ret.lo.hi.s3 = $FUNC($Arg0VarName.lo.hi.s3, ret.lo.hi.s2);
    ret.lo.hi.s4 = $FUNC($Arg0VarName.lo.hi.s4, ret.lo.hi.s3);
    ret.lo.hi.s5 = $FUNC($Arg0VarName.lo.hi.s5, ret.lo.hi.s4);
    ret.lo.hi.s6 = $FUNC($Arg0VarName.lo.hi.s6, ret.lo.hi.s5);
    ret.lo.hi.s7 = $FUNC($Arg0VarName.lo.hi.s7, ret.lo.hi.s6);
    ret.lo.hi.s8 = $FUNC($Arg0VarName.lo.hi.s8, ret.lo.hi.s7);
    ret.lo.hi.s9 = $FUNC($Arg0VarName.lo.hi.s9, ret.lo.hi.s8);
    ret.lo.hi.sA = $FUNC($Arg0VarName.lo.hi.sA, ret.lo.hi.s9);
    ret.lo.hi.sB = $FUNC($Arg0VarName.lo.hi.sB, ret.lo.hi.sA);
    ret.lo.hi.sC = $FUNC($Arg0VarName.lo.hi.sC, ret.lo.hi.sB);
    ret.lo.hi.sD = $FUNC($Arg0VarName.lo.hi.sD, ret.lo.hi.sC);
    ret.lo.hi.sE = $FUNC($Arg0VarName.lo.hi.sE, ret.lo.hi.sD);
    ret.lo.hi.sF = $FUNC($Arg0VarName.lo.hi.sF, ret.lo.hi.sE);

    ret.hi.lo.s0 = $FUNC($Arg0VarName.hi.lo.s0, ret.lo.hi.sF);
    ret.hi.lo.s1 = $FUNC($Arg0VarName.hi.lo.s1, ret.hi.lo.s0);
    ret.hi.lo.s2 = $FUNC($Arg0VarName.hi.lo.s2, ret.hi.lo.s1);
    ret.hi.lo.s3 = $FUNC($Arg0VarName.hi.lo.s3, ret.hi.lo.s2);
    ret.hi.lo.s4 = $FUNC($Arg0VarName.hi.lo.s4, ret.hi.lo.s3);
    ret.hi.lo.s5 = $FUNC($Arg0VarName.hi.lo.s5, ret.hi.lo.s4);
    ret.hi.lo.s6 = $FUNC($Arg0VarName.hi.lo.s6, ret.hi.lo.s5);
    ret.hi.lo.s7 = $FUNC($Arg0VarName.hi.lo.s7, ret.hi.lo.s6);
    ret.hi.lo.s8 = $FUNC($Arg0VarName.hi.lo.s8, ret.hi.lo.s7);
    ret.hi.lo.s9 = $FUNC($Arg0VarName.hi.lo.s9, ret.hi.lo.s8);
    ret.hi.lo.sA = $FUNC($Arg0VarName.hi.lo.sA, ret.hi.lo.s9);
    ret.hi.lo.sB = $FUNC($Arg0VarName.hi.lo.sB, ret.hi.lo.sA);
    ret.hi.lo.sC = $FUNC($Arg0VarName.hi.lo.sC, ret.hi.lo.sB);
    ret.hi.lo.sD = $FUNC($Arg0VarName.hi.lo.sD, ret.hi.lo.sC);
    ret.hi.lo.sE = $FUNC($Arg0VarName.hi.lo.sE, ret.hi.lo.sD);
    ret.hi.lo.sF = $FUNC($Arg0VarName.hi.lo.sF, ret.hi.lo.sE);

    ret.hi.hi.s0 = $FUNC($Arg0VarName.hi.hi.s0, ret.hi.lo.sF);
    ret.hi.hi.s1 = $FUNC($Arg0VarName.hi.hi.s1, ret.hi.hi.s0);
    ret.hi.hi.s2 = $FUNC($Arg0VarName.hi.hi.s2, ret.hi.hi.s1);
    ret.hi.hi.s3 = $FUNC($Arg0VarName.hi.hi.s3, ret.hi.hi.s2);
    ret.hi.hi.s4 = $FUNC($Arg0VarName.hi.hi.s4, ret.hi.hi.s3);
    ret.hi.hi.s5 = $FUNC($Arg0VarName.hi.hi.s5, ret.hi.hi.s4);
    ret.hi.hi.s6 = $FUNC($Arg0VarName.hi.hi.s6, ret.hi.hi.s5);
    ret.hi.hi.s7 = $FUNC($Arg0VarName.hi.hi.s7, ret.hi.hi.s6);
    ret.hi.hi.s8 = $FUNC($Arg0VarName.hi.hi.s8, ret.hi.hi.s7);
    ret.hi.hi.s9 = $FUNC($Arg0VarName.hi.hi.s9, ret.hi.hi.s8);
    ret.hi.hi.sA = $FUNC($Arg0VarName.hi.hi.sA, ret.hi.hi.s9);
    ret.hi.hi.sB = $FUNC($Arg0VarName.hi.hi.sB, ret.hi.hi.sA);
    ret.hi.hi.sC = $FUNC($Arg0VarName.hi.hi.sC, ret.hi.hi.sB);
    ret.hi.hi.sD = $FUNC($Arg0VarName.hi.hi.sD, ret.hi.hi.sC);
    ret.hi.hi.sE = $FUNC($Arg0VarName.hi.hi.sE, ret.hi.hi.sD);
    ret.hi.hi.sF = $FUNC($Arg0VarName.hi.hi.sF, ret.hi.hi.sE);

    *$Arg2VarName = ret;
    return ret;
  }];


// Vector versions of Scan_add/min/max/mul functions
OclBuiltinImpl work_group_scan_exclusive_add_vector_helper_gen4 =
      OclBuiltinImpl<work_group_scan_exclusive_add_helper, work_group_vector_types_all4,    0, work_group_scan_exclusive_function_op_vector4, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_scan_exclusive_add_vector_helper_gen8 =
      OclBuiltinImpl<work_group_scan_exclusive_add_helper, work_group_vector_types_all8,    0, work_group_scan_exclusive_function_op_vector8, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_scan_exclusive_add_vector_helper_gen16 =
      OclBuiltinImpl<work_group_scan_exclusive_add_helper, work_group_vector_types_all16,   0, work_group_scan_exclusive_function_op_vector16, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_scan_exclusive_add_vector_helper_gen32 =
      OclBuiltinImpl<work_group_scan_exclusive_add_helper, work_group_vector_types_all32,   0, work_group_scan_exclusive_function_op_vector32, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_scan_exclusive_add_vector_helper_gen64 =
      OclBuiltinImpl<work_group_scan_exclusive_add_helper, work_group_vector_types_all64,   0, work_group_scan_exclusive_function_op_vector64, [["$FUNC","work_group_add_util"]]>;

OclBuiltinImpl work_group_scan_exclusive_mul_vector_helper_gen4 =
      OclBuiltinImpl<work_group_scan_exclusive_mul_helper, work_group_vector_types_all4,    0, work_group_scan_exclusive_function_op_vector4, [["$FUNC","work_group_mul_util"]]>;
OclBuiltinImpl work_group_scan_exclusive_mul_vector_helper_gen8 =
      OclBuiltinImpl<work_group_scan_exclusive_mul_helper, work_group_vector_types_all8,    0, work_group_scan_exclusive_function_op_vector8, [["$FUNC","work_group_mul_util"]]>;
OclBuiltinImpl work_group_scan_exclusive_mul_vector_helper_gen16 =
      OclBuiltinImpl<work_group_scan_exclusive_mul_helper, work_group_vector_types_all16,   0, work_group_scan_exclusive_function_op_vector16, [["$FUNC","work_group_mul_util"]]>;
OclBuiltinImpl work_group_scan_exclusive_mul_vector_helper_gen32 =
      OclBuiltinImpl<work_group_scan_exclusive_mul_helper, work_group_vector_types_all32,   0, work_group_scan_exclusive_function_op_vector32, [["$FUNC","work_group_mul_util"]]>;
OclBuiltinImpl work_group_scan_exclusive_mul_vector_helper_gen64 =
      OclBuiltinImpl<work_group_scan_exclusive_mul_helper, work_group_vector_types_all64,   0, work_group_scan_exclusive_function_op_vector64, [["$FUNC","work_group_mul_util"]]>;

OclBuiltinImpl work_group_scan_exclusive_min_vector_helper_int4 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_vector_types_int4,    0, work_group_scan_exclusive_function_op_vector4, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_vector_helper_int8 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_vector_types_int8,    0, work_group_scan_exclusive_function_op_vector8, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_vector_helper_int16 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_vector_types_int16,   0, work_group_scan_exclusive_function_op_vector16, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_vector_helper_int32 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_vector_types_int32,   0, work_group_scan_exclusive_function_op_vector32, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_vector_helper_int64 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_vector_types_int64,   0, work_group_scan_exclusive_function_op_vector64, [["$FUNC","min"]]>;

OclBuiltinImpl work_group_scan_exclusive_min_vector_helper_float4 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_vector_types_float4,  0, work_group_scan_exclusive_function_op_vector4, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_vector_helper_float8 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_vector_types_float8,  0, work_group_scan_exclusive_function_op_vector8, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_vector_helper_float16 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_vector_types_float16, 0, work_group_scan_exclusive_function_op_vector16, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_vector_helper_float32 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_vector_types_float32, 0, work_group_scan_exclusive_function_op_vector32, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_vector_helper_float64 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_vector_types_float64, 0, work_group_scan_exclusive_function_op_vector64, [["$FUNC","fmin"]]>;

OclBuiltinImpl work_group_scan_exclusive_max_vector_helper_int4 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_int4,    0, work_group_scan_exclusive_function_op_vector4, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_vector_helper_int8 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_int8,    0, work_group_scan_exclusive_function_op_vector8, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_vector_helper_int16 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_int16,   0, work_group_scan_exclusive_function_op_vector16, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_vector_helper_int32 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_int32,   0, work_group_scan_exclusive_function_op_vector32, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_vector_helper_int64 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_int64,   0, work_group_scan_exclusive_function_op_vector64, [["$FUNC","max"]]>;

OclBuiltinImpl work_group_scan_exclusive_max_vector_helper_float4 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_float4,  0, work_group_scan_exclusive_function_op_vector4, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_vector_helper_float8 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_float8,  0, work_group_scan_exclusive_function_op_vector8, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_vector_helper_float16 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_float16, 0, work_group_scan_exclusive_function_op_vector16, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_vector_helper_float32 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_float32, 0, work_group_scan_exclusive_function_op_vector32, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_vector_helper_float64 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_float64, 0, work_group_scan_exclusive_function_op_vector64, [["$FUNC","fmax"]]>;

OclBuiltinImpl work_group_scan_inclusive_add_vector_helper_gen4 =
      OclBuiltinImpl<work_group_scan_inclusive_add_helper, work_group_vector_types_all4,    0, work_group_scan_inclusive_function_op_vector4, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_scan_inclusive_add_vector_helper_gen8 =
      OclBuiltinImpl<work_group_scan_inclusive_add_helper, work_group_vector_types_all8,    0, work_group_scan_inclusive_function_op_vector8, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_scan_inclusive_add_vector_helper_gen16 =
      OclBuiltinImpl<work_group_scan_inclusive_add_helper, work_group_vector_types_all16,   0, work_group_scan_inclusive_function_op_vector16, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_scan_inclusive_add_vector_helper_gen32 =
      OclBuiltinImpl<work_group_scan_inclusive_add_helper, work_group_vector_types_all32,   0, work_group_scan_inclusive_function_op_vector32, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_scan_inclusive_add_vector_helper_gen64 =
      OclBuiltinImpl<work_group_scan_inclusive_add_helper, work_group_vector_types_all64,   0, work_group_scan_inclusive_function_op_vector64, [["$FUNC","work_group_add_util"]]>;

OclBuiltinImpl work_group_scan_inclusive_mul_vector_helper_gen4 =
      OclBuiltinImpl<work_group_scan_inclusive_mul_helper, work_group_vector_types_all4,    0, work_group_scan_inclusive_function_op_vector4, [["$FUNC","work_group_mul_util"]]>;
OclBuiltinImpl work_group_scan_inclusive_mul_vector_helper_gen8 =
      OclBuiltinImpl<work_group_scan_inclusive_mul_helper, work_group_vector_types_all8,    0, work_group_scan_inclusive_function_op_vector8, [["$FUNC","work_group_mul_util"]]>;
OclBuiltinImpl work_group_scan_inclusive_mul_vector_helper_gen16 =
      OclBuiltinImpl<work_group_scan_inclusive_mul_helper, work_group_vector_types_all16,   0, work_group_scan_inclusive_function_op_vector16, [["$FUNC","work_group_mul_util"]]>;
OclBuiltinImpl work_group_scan_inclusive_mul_vector_helper_gen32 =
      OclBuiltinImpl<work_group_scan_inclusive_mul_helper, work_group_vector_types_all32,   0, work_group_scan_inclusive_function_op_vector32, [["$FUNC","work_group_mul_util"]]>;
OclBuiltinImpl work_group_scan_inclusive_mul_vector_helper_gen64 =
      OclBuiltinImpl<work_group_scan_inclusive_mul_helper, work_group_vector_types_all64,   0, work_group_scan_inclusive_function_op_vector64, [["$FUNC","work_group_mul_util"]]>;

OclBuiltinImpl work_group_scan_inclusive_min_vector_helper_int4 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_vector_types_int4,    0, work_group_scan_inclusive_function_op_vector4, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_vector_helper_int8 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_vector_types_int8,    0, work_group_scan_inclusive_function_op_vector8, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_vector_helper_int16 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_vector_types_int16,   0, work_group_scan_inclusive_function_op_vector16, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_vector_helper_int32 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_vector_types_int32,   0, work_group_scan_inclusive_function_op_vector32, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_vector_helper_int64 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_vector_types_int64,   0, work_group_scan_inclusive_function_op_vector64, [["$FUNC","min"]]>;

OclBuiltinImpl work_group_scan_inclusive_min_vector_helper_float4 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_vector_types_float4,  0, work_group_scan_inclusive_function_op_vector4, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_vector_helper_float8 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_vector_types_float8,  0, work_group_scan_inclusive_function_op_vector8, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_vector_helper_float16 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_vector_types_float16, 0, work_group_scan_inclusive_function_op_vector16, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_vector_helper_float32 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_vector_types_float32, 0, work_group_scan_inclusive_function_op_vector32, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_vector_helper_float64 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_vector_types_float64, 0, work_group_scan_inclusive_function_op_vector64, [["$FUNC","fmin"]]>;

OclBuiltinImpl work_group_scan_inclusive_max_vector_helper_int4 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_int4,    0, work_group_scan_inclusive_function_op_vector4, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_vector_helper_int8 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_int8,    0, work_group_scan_inclusive_function_op_vector8, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_vector_helper_int16 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_int16,   0, work_group_scan_inclusive_function_op_vector16, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_vector_helper_int32 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_int32,   0, work_group_scan_inclusive_function_op_vector32, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_vector_helper_int64 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_int64,   0, work_group_scan_inclusive_function_op_vector64, [["$FUNC","max"]]>;

OclBuiltinImpl work_group_scan_inclusive_max_vector_helper_float4 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_float4,  0, work_group_scan_inclusive_function_op_vector4, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_vector_helper_float8 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_float8,  0, work_group_scan_inclusive_function_op_vector8, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_vector_helper_float16 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_float16, 0, work_group_scan_inclusive_function_op_vector16, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_vector_helper_float32 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_float32, 0, work_group_scan_inclusive_function_op_vector32, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_vector_helper_float64 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_float64, 0, work_group_scan_inclusive_function_op_vector64, [["$FUNC","fmax"]]>;

// Vector versions of Scan_add/min/max functions
OclBuiltinImpl work_group_scan_exclusive_add_mask_helper_gen4 =
      OclBuiltinImpl<work_group_scan_exclusive_add_helper_mask, work_group_vector_types_all4,    0, work_group_scan_exclusive_function_op_mask4, [["$FUNC","work_group_add_util"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_exclusive_add_mask_helper_gen8 =
      OclBuiltinImpl<work_group_scan_exclusive_add_helper_mask, work_group_vector_types_all8,    0, work_group_scan_exclusive_function_op_mask8, [["$FUNC","work_group_add_util"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_exclusive_add_mask_helper_gen16 =
      OclBuiltinImpl<work_group_scan_exclusive_add_helper_mask, work_group_vector_types_all16,   0, work_group_scan_exclusive_function_op_mask16, [["$FUNC","work_group_add_util"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_exclusive_add_mask_helper_gen32 =
      OclBuiltinImpl<work_group_scan_exclusive_add_helper_mask, work_group_vector_types_all32,   0, work_group_scan_exclusive_function_op_mask32, [["$FUNC","work_group_add_util"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_exclusive_add_mask_helper_gen64 =
      OclBuiltinImpl<work_group_scan_exclusive_add_helper_mask, work_group_vector_types_all64,   0, work_group_scan_exclusive_function_op_mask64, [["$FUNC","work_group_add_util"], ["$VAL", "0"]]>;

OclBuiltinImpl work_group_scan_exclusive_mul_mask_helper_gen4 =
      OclBuiltinImpl<work_group_scan_exclusive_mul_helper_mask, work_group_vector_types_all4,    0, work_group_scan_exclusive_function_op_mask4, [["$FUNC","work_group_mul_util"], ["$VAL", "1"]]>;
OclBuiltinImpl work_group_scan_exclusive_mul_mask_helper_gen8 =
      OclBuiltinImpl<work_group_scan_exclusive_mul_helper_mask, work_group_vector_types_all8,    0, work_group_scan_exclusive_function_op_mask8, [["$FUNC","work_group_mul_util"], ["$VAL", "1"]]>;
OclBuiltinImpl work_group_scan_exclusive_mul_mask_helper_gen16 =
      OclBuiltinImpl<work_group_scan_exclusive_mul_helper_mask, work_group_vector_types_all16,   0, work_group_scan_exclusive_function_op_mask16, [["$FUNC","work_group_mul_util"], ["$VAL", "1"]]>;
OclBuiltinImpl work_group_scan_exclusive_mul_mask_helper_gen32 =
      OclBuiltinImpl<work_group_scan_exclusive_mul_helper_mask, work_group_vector_types_all32,   0, work_group_scan_exclusive_function_op_mask32, [["$FUNC","work_group_mul_util"], ["$VAL", "1"]]>;
OclBuiltinImpl work_group_scan_exclusive_mul_mask_helper_gen64 =
      OclBuiltinImpl<work_group_scan_exclusive_mul_helper_mask, work_group_vector_types_all64,   0, work_group_scan_exclusive_function_op_mask64, [["$FUNC","work_group_mul_util"], ["$VAL", "1"]]>;

OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_int4 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v4i32], 0, work_group_scan_exclusive_function_op_mask4, [["$FUNC","min"], ["$VAL", "INT_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_int8 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v8i32], 0, work_group_scan_exclusive_function_op_mask8, [["$FUNC","min"], ["$VAL", "INT_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_int16 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v16i32], 0, work_group_scan_exclusive_function_op_mask16, [["$FUNC","min"], ["$VAL", "INT_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_int32 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v32i32], 0, work_group_scan_exclusive_function_op_mask32, [["$FUNC","min"], ["$VAL", "INT_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_int64 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v64i32], 0, work_group_scan_exclusive_function_op_mask64, [["$FUNC","min"], ["$VAL", "INT_MAX"]]>;

OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_uint4 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v4u32], 0, work_group_scan_exclusive_function_op_mask4, [["$FUNC","min"], ["$VAL", "UINT_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_uint8 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v8u32], 0, work_group_scan_exclusive_function_op_mask8, [["$FUNC","min"], ["$VAL", "UINT_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_uint16 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v16u32], 0, work_group_scan_exclusive_function_op_mask16, [["$FUNC","min"], ["$VAL", "UINT_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_uint32 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v32u32], 0, work_group_scan_exclusive_function_op_mask32, [["$FUNC","min"], ["$VAL", "UINT_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_uint64 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v64u32], 0, work_group_scan_exclusive_function_op_mask64, [["$FUNC","min"], ["$VAL", "UINT_MAX"]]>;

OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_long4 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v4i64], 0, work_group_scan_exclusive_function_op_mask4, [["$FUNC","min"], ["$VAL", "LONG_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_long8 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v8i64], 0, work_group_scan_exclusive_function_op_mask8, [["$FUNC","min"], ["$VAL", "LONG_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_long16 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v16i64], 0, work_group_scan_exclusive_function_op_mask16, [["$FUNC","min"], ["$VAL", "LONG_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_long32 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v32i64], 0, work_group_scan_exclusive_function_op_mask32, [["$FUNC","min"], ["$VAL", "LONG_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_long64 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v64i64], 0, work_group_scan_exclusive_function_op_mask64, [["$FUNC","min"], ["$VAL", "LONG_MAX"]]>;

OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_ulong4 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v4u64], 0, work_group_scan_exclusive_function_op_mask4, [["$FUNC","min"], ["$VAL", "ULONG_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_ulong8 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v8u64], 0, work_group_scan_exclusive_function_op_mask8, [["$FUNC","min"], ["$VAL", "ULONG_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_ulong16 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v16u64], 0, work_group_scan_exclusive_function_op_mask16, [["$FUNC","min"], ["$VAL", "ULONG_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_ulong32 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v32u64], 0, work_group_scan_exclusive_function_op_mask32, [["$FUNC","min"], ["$VAL", "ULONG_MAX"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_ulong64 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, [v64u64], 0, work_group_scan_exclusive_function_op_mask64, [["$FUNC","min"], ["$VAL", "ULONG_MAX"]]>;

OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_float4 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, work_group_vector_types_float4,  0, work_group_scan_exclusive_function_op_mask4, [["$FUNC","fmin"], ["$VAL", "INFINITY"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_float8 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, work_group_vector_types_float8,  0, work_group_scan_exclusive_function_op_mask8, [["$FUNC","fmin"], ["$VAL", "INFINITY"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_float16 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, work_group_vector_types_float16, 0, work_group_scan_exclusive_function_op_mask16, [["$FUNC","fmin"], ["$VAL", "INFINITY"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_float32 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, work_group_vector_types_float32, 0, work_group_scan_exclusive_function_op_mask32, [["$FUNC","fmin"], ["$VAL", "INFINITY"]]>;
OclBuiltinImpl work_group_scan_exclusive_min_mask_helper_float64 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper_mask, work_group_vector_types_float64, 0, work_group_scan_exclusive_function_op_mask64, [["$FUNC","fmin"], ["$VAL", "INFINITY"]]>;

OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_int4 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v4i32], 0, work_group_scan_exclusive_function_op_mask4, [["$FUNC","max"], ["$VAL", "INT_MIN"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_int8 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v8i32], 0, work_group_scan_exclusive_function_op_mask8, [["$FUNC","max"], ["$VAL", "INT_MIN"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_int16 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v16i32], 0, work_group_scan_exclusive_function_op_mask16, [["$FUNC","max"], ["$VAL", "INT_MIN"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_int32 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v32i32], 0, work_group_scan_exclusive_function_op_mask32, [["$FUNC","max"], ["$VAL", "INT_MIN"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_int64 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v64i32], 0, work_group_scan_exclusive_function_op_mask64, [["$FUNC","max"], ["$VAL", "INT_MIN"]]>;

OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_uint4 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v4u32], 0, work_group_scan_exclusive_function_op_mask4, [["$FUNC","max"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_uint8 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v8u32], 0, work_group_scan_exclusive_function_op_mask8, [["$FUNC","max"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_uint16 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v16u32], 0, work_group_scan_exclusive_function_op_mask16, [["$FUNC","max"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_uint32 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v32u32], 0, work_group_scan_exclusive_function_op_mask32, [["$FUNC","max"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_uint64 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v64u32], 0, work_group_scan_exclusive_function_op_mask64, [["$FUNC","max"], ["$VAL", "0"]]>;

OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_long4 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v4i64], 0, work_group_scan_exclusive_function_op_mask4, [["$FUNC","max"], ["$VAL", "LONG_MIN"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_long8 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v8i64], 0, work_group_scan_exclusive_function_op_mask8, [["$FUNC","max"], ["$VAL", "LONG_MIN"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_long16 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v16i64], 0, work_group_scan_exclusive_function_op_mask16, [["$FUNC","max"], ["$VAL", "LONG_MIN"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_long32 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v32i64], 0, work_group_scan_exclusive_function_op_mask32, [["$FUNC","max"], ["$VAL", "LONG_MIN"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_long64 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v64i64], 0, work_group_scan_exclusive_function_op_mask64, [["$FUNC","max"], ["$VAL", "LONG_MIN"]]>;

OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_ulong4 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v4u64], 0, work_group_scan_exclusive_function_op_mask4, [["$FUNC","max"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_ulong8 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v8u64], 0, work_group_scan_exclusive_function_op_mask8, [["$FUNC","max"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_ulong16 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v16u64], 0, work_group_scan_exclusive_function_op_mask16, [["$FUNC","max"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_ulong32 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v32u64], 0, work_group_scan_exclusive_function_op_mask32, [["$FUNC","max"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_ulong64 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper_mask, [v64u64], 0, work_group_scan_exclusive_function_op_mask64, [["$FUNC","max"], ["$VAL", "0"]]>;

OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_float4 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_float4,  0, work_group_scan_exclusive_function_op_mask4, [["$FUNC","fmax"], ["$VAL", "-INFINITY"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_float8 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_float8,  0, work_group_scan_exclusive_function_op_mask8, [["$FUNC","fmax"], ["$VAL", "-INFINITY"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_float16 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_float16, 0, work_group_scan_exclusive_function_op_mask16, [["$FUNC","fmax"], ["$VAL", "-INFINITY"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_float32 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_float32, 0, work_group_scan_exclusive_function_op_mask32, [["$FUNC","fmax"], ["$VAL", "-INFINITY"]]>;
OclBuiltinImpl work_group_scan_exclusive_max_mask_helper_float64 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_float64, 0, work_group_scan_exclusive_function_op_mask64, [["$FUNC","fmax"], ["$VAL", "-INFINITY"]]>;

OclBuiltinImpl work_group_scan_inclusive_add_mask_helper_gen4 =
      OclBuiltinImpl<work_group_scan_inclusive_add_helper_mask, work_group_vector_types_all4,    0, work_group_scan_inclusive_function_op_mask4, [["$FUNC","work_group_add_util"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_inclusive_add_mask_helper_gen8 =
      OclBuiltinImpl<work_group_scan_inclusive_add_helper_mask, work_group_vector_types_all8,    0, work_group_scan_inclusive_function_op_mask8, [["$FUNC","work_group_add_util"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_inclusive_add_mask_helper_gen16 =
      OclBuiltinImpl<work_group_scan_inclusive_add_helper_mask, work_group_vector_types_all16,   0, work_group_scan_inclusive_function_op_mask16, [["$FUNC","work_group_add_util"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_inclusive_add_mask_helper_gen32 =
      OclBuiltinImpl<work_group_scan_inclusive_add_helper_mask, work_group_vector_types_all32,   0, work_group_scan_inclusive_function_op_mask32, [["$FUNC","work_group_add_util"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_inclusive_add_mask_helper_gen64 =
      OclBuiltinImpl<work_group_scan_inclusive_add_helper_mask, work_group_vector_types_all64,   0, work_group_scan_inclusive_function_op_mask64, [["$FUNC","work_group_add_util"], ["$VAL", "0"]]>;

OclBuiltinImpl work_group_scan_inclusive_mul_mask_helper_gen4 =
      OclBuiltinImpl<work_group_scan_inclusive_mul_helper_mask, work_group_vector_types_all4,    0, work_group_scan_inclusive_function_op_mask4, [["$FUNC","work_group_mul_util"], ["$VAL", "1"]]>;
OclBuiltinImpl work_group_scan_inclusive_mul_mask_helper_gen8 =
      OclBuiltinImpl<work_group_scan_inclusive_mul_helper_mask, work_group_vector_types_all8,    0, work_group_scan_inclusive_function_op_mask8, [["$FUNC","work_group_mul_util"], ["$VAL", "1"]]>;
OclBuiltinImpl work_group_scan_inclusive_mul_mask_helper_gen16 =
      OclBuiltinImpl<work_group_scan_inclusive_mul_helper_mask, work_group_vector_types_all16,   0, work_group_scan_inclusive_function_op_mask16, [["$FUNC","work_group_mul_util"], ["$VAL", "1"]]>;
OclBuiltinImpl work_group_scan_inclusive_mul_mask_helper_gen32 =
      OclBuiltinImpl<work_group_scan_inclusive_mul_helper_mask, work_group_vector_types_all32,   0, work_group_scan_inclusive_function_op_mask32, [["$FUNC","work_group_mul_util"], ["$VAL", "1"]]>;
OclBuiltinImpl work_group_scan_inclusive_mul_mask_helper_gen64 =
      OclBuiltinImpl<work_group_scan_inclusive_mul_helper_mask, work_group_vector_types_all64,   0, work_group_scan_inclusive_function_op_mask64, [["$FUNC","work_group_mul_util"], ["$VAL", "1"]]>;

OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_int4 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v4i32], 0, work_group_scan_inclusive_function_op_mask4, [["$FUNC","min"], ["$VAL", "INT_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_int8 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v8i32], 0, work_group_scan_inclusive_function_op_mask8, [["$FUNC","min"], ["$VAL", "INT_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_int16 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v16i32], 0, work_group_scan_inclusive_function_op_mask16, [["$FUNC","min"], ["$VAL", "INT_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_int32 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v32i32], 0, work_group_scan_inclusive_function_op_mask32, [["$FUNC","min"], ["$VAL", "INT_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_int64 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v64i32], 0, work_group_scan_inclusive_function_op_mask64, [["$FUNC","min"], ["$VAL", "INT_MAX"]]>;

OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_uint4 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v4u32], 0, work_group_scan_inclusive_function_op_mask4, [["$FUNC","min"], ["$VAL", "UINT_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_uint8 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v8u32], 0, work_group_scan_inclusive_function_op_mask8, [["$FUNC","min"], ["$VAL", "UINT_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_uint16 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v16u32], 0, work_group_scan_inclusive_function_op_mask16, [["$FUNC","min"], ["$VAL", "UINT_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_uint32 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v32u32], 0, work_group_scan_inclusive_function_op_mask32, [["$FUNC","min"], ["$VAL", "UINT_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_uint64 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v64u32], 0, work_group_scan_inclusive_function_op_mask64, [["$FUNC","min"], ["$VAL", "UINT_MAX"]]>;

OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_long4 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v4i64], 0, work_group_scan_inclusive_function_op_mask4, [["$FUNC","min"], ["$VAL", "LONG_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_long8 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v8i64], 0, work_group_scan_inclusive_function_op_mask8, [["$FUNC","min"], ["$VAL", "LONG_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_long16 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v16i64], 0, work_group_scan_inclusive_function_op_mask16, [["$FUNC","min"], ["$VAL", "LONG_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_long32 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v32i64], 0, work_group_scan_inclusive_function_op_mask32, [["$FUNC","min"], ["$VAL", "LONG_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_long64 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v64i64], 0, work_group_scan_inclusive_function_op_mask64, [["$FUNC","min"], ["$VAL", "LONG_MAX"]]>;

OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_ulong4 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v4u64], 0, work_group_scan_inclusive_function_op_mask4, [["$FUNC","min"], ["$VAL", "ULONG_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_ulong8 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v8u64], 0, work_group_scan_inclusive_function_op_mask8, [["$FUNC","min"], ["$VAL", "ULONG_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_ulong16 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v16u64], 0, work_group_scan_inclusive_function_op_mask16, [["$FUNC","min"], ["$VAL", "ULONG_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_ulong32 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v32u64], 0, work_group_scan_inclusive_function_op_mask32, [["$FUNC","min"], ["$VAL", "ULONG_MAX"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_ulong64 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, [v64u64], 0, work_group_scan_inclusive_function_op_mask64, [["$FUNC","min"], ["$VAL", "ULONG_MAX"]]>;

OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_float4 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, work_group_vector_types_float4,  0, work_group_scan_inclusive_function_op_mask4, [["$FUNC","fmin"], ["$VAL", "INFINITY"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_float8 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, work_group_vector_types_float8,  0, work_group_scan_inclusive_function_op_mask8, [["$FUNC","fmin"], ["$VAL", "INFINITY"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_float16 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, work_group_vector_types_float16, 0, work_group_scan_inclusive_function_op_mask16, [["$FUNC","fmin"], ["$VAL", "INFINITY"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_float32 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, work_group_vector_types_float32, 0, work_group_scan_inclusive_function_op_mask32, [["$FUNC","fmin"], ["$VAL", "INFINITY"]]>;
OclBuiltinImpl work_group_scan_inclusive_min_mask_helper_float64 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper_mask, work_group_vector_types_float64, 0, work_group_scan_inclusive_function_op_mask64, [["$FUNC","fmin"], ["$VAL", "INFINITY"]]>;

OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_int4 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v4i32], 0, work_group_scan_inclusive_function_op_mask4, [["$FUNC","max"], ["$VAL", "INT_MIN"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_int8 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v8i32], 0, work_group_scan_inclusive_function_op_mask8, [["$FUNC","max"], ["$VAL", "INT_MIN"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_int16 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v16i32], 0, work_group_scan_inclusive_function_op_mask16, [["$FUNC","max"], ["$VAL", "INT_MIN"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_int32 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v32i32], 0, work_group_scan_inclusive_function_op_mask32, [["$FUNC","max"], ["$VAL", "INT_MIN"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_int64 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v64i32], 0, work_group_scan_inclusive_function_op_mask64, [["$FUNC","max"], ["$VAL", "INT_MIN"]]>;

OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_uint4 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v4u32], 0, work_group_scan_inclusive_function_op_mask4, [["$FUNC","max"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_uint8 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v8u32], 0, work_group_scan_inclusive_function_op_mask8, [["$FUNC","max"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_uint16 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v16u32], 0, work_group_scan_inclusive_function_op_mask16, [["$FUNC","max"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_uint32 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v32u32], 0, work_group_scan_inclusive_function_op_mask32, [["$FUNC","max"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_uint64 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v64u32], 0, work_group_scan_inclusive_function_op_mask64, [["$FUNC","max"], ["$VAL", "0"]]>;

OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_long4 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v4i64], 0, work_group_scan_inclusive_function_op_mask4, [["$FUNC","max"], ["$VAL", "LONG_MIN"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_long8 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v8i64], 0, work_group_scan_inclusive_function_op_mask8, [["$FUNC","max"], ["$VAL", "LONG_MIN"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_long16 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v16i64], 0, work_group_scan_inclusive_function_op_mask16, [["$FUNC","max"], ["$VAL", "LONG_MIN"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_long32 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v32i64], 0, work_group_scan_inclusive_function_op_mask32, [["$FUNC","max"], ["$VAL", "LONG_MIN"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_long64 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v64i64], 0, work_group_scan_inclusive_function_op_mask64, [["$FUNC","max"], ["$VAL", "LONG_MIN"]]>;

OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_ulong4 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v4u64], 0, work_group_scan_inclusive_function_op_mask4, [["$FUNC","max"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_ulong8 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v8u64], 0, work_group_scan_inclusive_function_op_mask8, [["$FUNC","max"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_ulong16 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v16u64], 0, work_group_scan_inclusive_function_op_mask16, [["$FUNC","max"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_ulong32 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v32u64], 0, work_group_scan_inclusive_function_op_mask32, [["$FUNC","max"], ["$VAL", "0"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_ulong64 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper_mask, [v64u64], 0, work_group_scan_inclusive_function_op_mask64, [["$FUNC","max"], ["$VAL", "0"]]>;

OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_float4 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_float4,  0, work_group_scan_inclusive_function_op_mask4, [["$FUNC","fmax"], ["$VAL", "-INFINITY"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_float8 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_float8,  0, work_group_scan_inclusive_function_op_mask8, [["$FUNC","fmax"], ["$VAL", "-INFINITY"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_float16 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_float16, 0, work_group_scan_inclusive_function_op_mask16, [["$FUNC","fmax"], ["$VAL", "-INFINITY"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_float32 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_float32, 0, work_group_scan_inclusive_function_op_mask32, [["$FUNC","fmax"], ["$VAL", "-INFINITY"]]>;
OclBuiltinImpl work_group_scan_inclusive_max_mask_helper_float64 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_float64, 0, work_group_scan_inclusive_function_op_mask64, [["$FUNC","fmax"], ["$VAL", "-INFINITY"]]>;


// Scalar versions of Scan_add/min/max/mul functions
defvar work_group_scan_exclusive_function_op_scalar = [{
    $Arg0Type acc = *$Arg1VarName;
    *$Arg1VarName = $FUNC($Arg0VarName, *$Arg1VarName);
    return acc;
  }];

OclBuiltinImpl work_group_scan_exclusive_add_scalar_helper_gen =
      OclBuiltinImpl<work_group_scan_exclusive_add_helper, work_group_scalar_types_all,     0, work_group_scan_exclusive_function_op_scalar, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_scan_exclusive_mul_scalar_helper_gen =
      OclBuiltinImpl<work_group_scan_exclusive_mul_helper, work_group_scalar_types_all,     0, work_group_scan_exclusive_function_op_scalar, [["$FUNC","work_group_mul_util"]]>;
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
OclBuiltinImpl work_group_scan_inclusive_mul_scalar_helper_gen =
      OclBuiltinImpl<work_group_scan_inclusive_mul_helper, work_group_scalar_types_all,     0, work_group_inclusive_function_op_scalar, [["$FUNC","work_group_mul_util"]]>;
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

defvar work_group_finalize_function_op4 = [{
    $Arg0VecType ret;
    ret.s0 = $Arg0VarName.s0;
    ret.s0 = $FUNC($Arg0VarName.s1, ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s2, ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s3, ret.s0);
    return ($Arg0VecType)(ret.s0);
  }];

defvar work_group_finalize_function_op8 = [{
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

defvar work_group_finalize_function_op16 = [{
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

defvar work_group_finalize_function_op32 = [{
    $Arg0VecType ret;
    $Arg0BaseType#16 hi = $Arg0VarName.hi;
    $Arg0BaseType#16 lo = $Arg0VarName.lo;

    ret.s0 = lo.s0;
    ret.s0 = $FUNC(lo.s1,  ret.s0);
    ret.s0 = $FUNC(lo.s2,  ret.s0);
    ret.s0 = $FUNC(lo.s3,  ret.s0);
    ret.s0 = $FUNC(lo.s4,  ret.s0);
    ret.s0 = $FUNC(lo.s5,  ret.s0);
    ret.s0 = $FUNC(lo.s6,  ret.s0);
    ret.s0 = $FUNC(lo.s7,  ret.s0);
    ret.s0 = $FUNC(lo.s8,  ret.s0);
    ret.s0 = $FUNC(lo.s9,  ret.s0);
    ret.s0 = $FUNC(lo.sA,  ret.s0);
    ret.s0 = $FUNC(lo.sB,  ret.s0);
    ret.s0 = $FUNC(lo.sC,  ret.s0);
    ret.s0 = $FUNC(lo.sD,  ret.s0);
    ret.s0 = $FUNC(lo.sE,  ret.s0);
    ret.s0 = $FUNC(lo.sF,  ret.s0);

    ret.s0 = $FUNC(hi.s0,  ret.s0);
    ret.s0 = $FUNC(hi.s1,  ret.s0);
    ret.s0 = $FUNC(hi.s2,  ret.s0);
    ret.s0 = $FUNC(hi.s3,  ret.s0);
    ret.s0 = $FUNC(hi.s4,  ret.s0);
    ret.s0 = $FUNC(hi.s5,  ret.s0);
    ret.s0 = $FUNC(hi.s6,  ret.s0);
    ret.s0 = $FUNC(hi.s7,  ret.s0);
    ret.s0 = $FUNC(hi.s8,  ret.s0);
    ret.s0 = $FUNC(hi.s9,  ret.s0);
    ret.s0 = $FUNC(hi.sA,  ret.s0);
    ret.s0 = $FUNC(hi.sB,  ret.s0);
    ret.s0 = $FUNC(hi.sC,  ret.s0);
    ret.s0 = $FUNC(hi.sD,  ret.s0);
    ret.s0 = $FUNC(hi.sE,  ret.s0);
    ret.s0 = $FUNC(hi.sF,  ret.s0);

    return ($Arg0VecType)(ret.s0);
  }];

defvar work_group_finalize_function_op64 = [{
    $Arg0VecType ret;
    $Arg0BaseType#16 hihi = $Arg0VarName.hi.hi;
    $Arg0BaseType#16 hilo = $Arg0VarName.hi.lo;
    $Arg0BaseType#16 lohi = $Arg0VarName.lo.hi;
    $Arg0BaseType#16 lolo = $Arg0VarName.lo.lo;

    ret.s0 = lolo.s0;
    ret.s0 = $FUNC(lolo.s1,  ret.s0);
    ret.s0 = $FUNC(lolo.s2,  ret.s0);
    ret.s0 = $FUNC(lolo.s3,  ret.s0);
    ret.s0 = $FUNC(lolo.s4,  ret.s0);
    ret.s0 = $FUNC(lolo.s5,  ret.s0);
    ret.s0 = $FUNC(lolo.s6,  ret.s0);
    ret.s0 = $FUNC(lolo.s7,  ret.s0);
    ret.s0 = $FUNC(lolo.s8,  ret.s0);
    ret.s0 = $FUNC(lolo.s9,  ret.s0);
    ret.s0 = $FUNC(lolo.sA,  ret.s0);
    ret.s0 = $FUNC(lolo.sB,  ret.s0);
    ret.s0 = $FUNC(lolo.sC,  ret.s0);
    ret.s0 = $FUNC(lolo.sD,  ret.s0);
    ret.s0 = $FUNC(lolo.sE,  ret.s0);
    ret.s0 = $FUNC(lolo.sF,  ret.s0);

    ret.s0 = $FUNC(lohi.s0,  ret.s0);
    ret.s0 = $FUNC(lohi.s1,  ret.s0);
    ret.s0 = $FUNC(lohi.s2,  ret.s0);
    ret.s0 = $FUNC(lohi.s3,  ret.s0);
    ret.s0 = $FUNC(lohi.s4,  ret.s0);
    ret.s0 = $FUNC(lohi.s5,  ret.s0);
    ret.s0 = $FUNC(lohi.s6,  ret.s0);
    ret.s0 = $FUNC(lohi.s7,  ret.s0);
    ret.s0 = $FUNC(lohi.s8,  ret.s0);
    ret.s0 = $FUNC(lohi.s9,  ret.s0);
    ret.s0 = $FUNC(lohi.sA,  ret.s0);
    ret.s0 = $FUNC(lohi.sB,  ret.s0);
    ret.s0 = $FUNC(lohi.sC,  ret.s0);
    ret.s0 = $FUNC(lohi.sD,  ret.s0);
    ret.s0 = $FUNC(lohi.sE,  ret.s0);
    ret.s0 = $FUNC(lohi.sF,  ret.s0);

    ret.s0 = $FUNC(hilo.s0,  ret.s0);
    ret.s0 = $FUNC(hilo.s1,  ret.s0);
    ret.s0 = $FUNC(hilo.s2,  ret.s0);
    ret.s0 = $FUNC(hilo.s3,  ret.s0);
    ret.s0 = $FUNC(hilo.s4,  ret.s0);
    ret.s0 = $FUNC(hilo.s5,  ret.s0);
    ret.s0 = $FUNC(hilo.s6,  ret.s0);
    ret.s0 = $FUNC(hilo.s7,  ret.s0);
    ret.s0 = $FUNC(hilo.s8,  ret.s0);
    ret.s0 = $FUNC(hilo.s9,  ret.s0);
    ret.s0 = $FUNC(hilo.sA,  ret.s0);
    ret.s0 = $FUNC(hilo.sB,  ret.s0);
    ret.s0 = $FUNC(hilo.sC,  ret.s0);
    ret.s0 = $FUNC(hilo.sD,  ret.s0);
    ret.s0 = $FUNC(hilo.sE,  ret.s0);
    ret.s0 = $FUNC(hilo.sF,  ret.s0);

    ret.s0 = $FUNC(hihi.s0,  ret.s0);
    ret.s0 = $FUNC(hihi.s1,  ret.s0);
    ret.s0 = $FUNC(hihi.s2,  ret.s0);
    ret.s0 = $FUNC(hihi.s3,  ret.s0);
    ret.s0 = $FUNC(hihi.s4,  ret.s0);
    ret.s0 = $FUNC(hihi.s5,  ret.s0);
    ret.s0 = $FUNC(hihi.s6,  ret.s0);
    ret.s0 = $FUNC(hihi.s7,  ret.s0);
    ret.s0 = $FUNC(hihi.s8,  ret.s0);
    ret.s0 = $FUNC(hihi.s9,  ret.s0);
    ret.s0 = $FUNC(hihi.sA,  ret.s0);
    ret.s0 = $FUNC(hihi.sB,  ret.s0);
    ret.s0 = $FUNC(hihi.sC,  ret.s0);
    ret.s0 = $FUNC(hihi.sD,  ret.s0);
    ret.s0 = $FUNC(hihi.sE,  ret.s0);
    ret.s0 = $FUNC(hihi.sF,  ret.s0);

    return ($Arg0VecType)(ret.s0);
  }];
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

OclBuiltinImpl work_group_reduce_add_finalize_helper_gen4 =
      OclBuiltinImpl<work_group_reduce_add_finalize_helper, work_group_vector_types_all4,    0, work_group_finalize_function_op4, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_reduce_add_finalize_helper_gen8 =
      OclBuiltinImpl<work_group_reduce_add_finalize_helper, work_group_vector_types_all8,    0, work_group_finalize_function_op8, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_reduce_add_finalize_helper_gen16 =
      OclBuiltinImpl<work_group_reduce_add_finalize_helper, work_group_vector_types_all16,   0, work_group_finalize_function_op16, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_reduce_add_finalize_helper_gen32 =
      OclBuiltinImpl<work_group_reduce_add_finalize_helper, work_group_vector_types_all32,   0, work_group_finalize_function_op32, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_reduce_add_finalize_helper_gen64 =
      OclBuiltinImpl<work_group_reduce_add_finalize_helper, work_group_vector_types_all64,   0, work_group_finalize_function_op64, [["$FUNC","work_group_add_util"]]>;

OclBuiltinImpl work_group_reduce_mul_finalize_helper_gen4 =
      OclBuiltinImpl<work_group_reduce_mul_finalize_helper, work_group_vector_types_all4,    0, work_group_finalize_function_op4, [["$FUNC","work_group_mul_util"]]>;
OclBuiltinImpl work_group_reduce_mul_finalize_helper_gen8 =
      OclBuiltinImpl<work_group_reduce_mul_finalize_helper, work_group_vector_types_all8,    0, work_group_finalize_function_op8, [["$FUNC","work_group_mul_util"]]>;
OclBuiltinImpl work_group_reduce_mul_finalize_helper_gen16 =
      OclBuiltinImpl<work_group_reduce_mul_finalize_helper, work_group_vector_types_all16,   0, work_group_finalize_function_op16, [["$FUNC","work_group_mul_util"]]>;
OclBuiltinImpl work_group_reduce_mul_finalize_helper_gen32 =
      OclBuiltinImpl<work_group_reduce_mul_finalize_helper, work_group_vector_types_all32,   0, work_group_finalize_function_op32, [["$FUNC","work_group_mul_util"]]>;
OclBuiltinImpl work_group_reduce_mul_finalize_helper_gen64 =
      OclBuiltinImpl<work_group_reduce_mul_finalize_helper, work_group_vector_types_all64,   0, work_group_finalize_function_op64, [["$FUNC","work_group_mul_util"]]>;

OclBuiltinImpl work_group_reduce_bitwise_and_finalize_helper_gen4 =
      OclBuiltinImpl<work_group_reduce_bitwise_and_finalize_helper, work_group_vector_types_all_integer4,    0, work_group_finalize_function_op4, [["$FUNC","work_group_bitwise_and_util"]]>;
OclBuiltinImpl work_group_reduce_bitwise_and_finalize_helper_gen8 =
      OclBuiltinImpl<work_group_reduce_bitwise_and_finalize_helper, work_group_vector_types_all_integer8,    0, work_group_finalize_function_op8, [["$FUNC","work_group_bitwise_and_util"]]>;
OclBuiltinImpl work_group_reduce_bitwise_and_finalize_helper_gen16 =
      OclBuiltinImpl<work_group_reduce_bitwise_and_finalize_helper, work_group_vector_types_all_integer16,   0, work_group_finalize_function_op16, [["$FUNC","work_group_bitwise_and_util"]]>;
OclBuiltinImpl work_group_reduce_bitwise_and_finalize_helper_gen32 =
      OclBuiltinImpl<work_group_reduce_bitwise_and_finalize_helper, work_group_vector_types_all_integer32,   0, work_group_finalize_function_op32, [["$FUNC","work_group_bitwise_and_util"]]>;
OclBuiltinImpl work_group_reduce_bitwise_and_finalize_helper_gen64 =
      OclBuiltinImpl<work_group_reduce_bitwise_and_finalize_helper, work_group_vector_types_all_integer64,   0, work_group_finalize_function_op64, [["$FUNC","work_group_bitwise_and_util"]]>;

OclBuiltinImpl work_group_reduce_bitwise_or_finalize_helper_gen4 =
      OclBuiltinImpl<work_group_reduce_bitwise_or_finalize_helper, work_group_vector_types_all_integer4,    0, work_group_finalize_function_op4, [["$FUNC","work_group_bitwise_or_util"]]>;
OclBuiltinImpl work_group_reduce_bitwise_or_finalize_helper_gen8 =
      OclBuiltinImpl<work_group_reduce_bitwise_or_finalize_helper, work_group_vector_types_all_integer8,    0, work_group_finalize_function_op8, [["$FUNC","work_group_bitwise_or_util"]]>;
OclBuiltinImpl work_group_reduce_bitwise_or_finalize_helper_gen16 =
      OclBuiltinImpl<work_group_reduce_bitwise_or_finalize_helper, work_group_vector_types_all_integer16,   0, work_group_finalize_function_op16, [["$FUNC","work_group_bitwise_or_util"]]>;
OclBuiltinImpl work_group_reduce_bitwise_or_finalize_helper_gen32 =
      OclBuiltinImpl<work_group_reduce_bitwise_or_finalize_helper, work_group_vector_types_all_integer32,   0, work_group_finalize_function_op32, [["$FUNC","work_group_bitwise_or_util"]]>;
OclBuiltinImpl work_group_reduce_bitwise_or_finalize_helper_gen64 =
      OclBuiltinImpl<work_group_reduce_bitwise_or_finalize_helper, work_group_vector_types_all_integer64,   0, work_group_finalize_function_op64, [["$FUNC","work_group_bitwise_or_util"]]>;

OclBuiltinImpl work_group_reduce_bitwise_xor_finalize_helper_gen4 =
      OclBuiltinImpl<work_group_reduce_bitwise_xor_finalize_helper, work_group_vector_types_all_integer4,    0, work_group_finalize_function_op4, [["$FUNC","work_group_bitwise_xor_util"]]>;
OclBuiltinImpl work_group_reduce_bitwise_xor_finalize_helper_gen8 =
      OclBuiltinImpl<work_group_reduce_bitwise_xor_finalize_helper, work_group_vector_types_all_integer8,    0, work_group_finalize_function_op8, [["$FUNC","work_group_bitwise_xor_util"]]>;
OclBuiltinImpl work_group_reduce_bitwise_xor_finalize_helper_gen16 =
      OclBuiltinImpl<work_group_reduce_bitwise_xor_finalize_helper, work_group_vector_types_all_integer16,   0, work_group_finalize_function_op16, [["$FUNC","work_group_bitwise_xor_util"]]>;
OclBuiltinImpl work_group_reduce_bitwise_xor_finalize_helper_gen32 =
      OclBuiltinImpl<work_group_reduce_bitwise_xor_finalize_helper, work_group_vector_types_all_integer32,   0, work_group_finalize_function_op32, [["$FUNC","work_group_bitwise_xor_util"]]>;
OclBuiltinImpl work_group_reduce_bitwise_xor_finalize_helper_gen64 =
      OclBuiltinImpl<work_group_reduce_bitwise_xor_finalize_helper, work_group_vector_types_all_integer64,   0, work_group_finalize_function_op64, [["$FUNC","work_group_bitwise_xor_util"]]>;

OclBuiltinImpl work_group_reduce_logical_and_finalize_helper_gen4 =
      OclBuiltinImpl<work_group_reduce_logical_and_finalize_helper, [v4i32],    0, work_group_finalize_function_op4, [["$FUNC","work_group_logical_and_util"]]>;
OclBuiltinImpl work_group_reduce_logical_and_finalize_helper_gen8 =
      OclBuiltinImpl<work_group_reduce_logical_and_finalize_helper, [v8i32],    0, work_group_finalize_function_op8, [["$FUNC","work_group_logical_and_util"]]>;
OclBuiltinImpl work_group_reduce_logical_and_finalize_helper_gen16 =
      OclBuiltinImpl<work_group_reduce_logical_and_finalize_helper, [v16i32],   0, work_group_finalize_function_op16, [["$FUNC","work_group_logical_and_util"]]>;
OclBuiltinImpl work_group_reduce_logical_and_finalize_helper_gen32 =
      OclBuiltinImpl<work_group_reduce_logical_and_finalize_helper, [v32i32],   0, work_group_finalize_function_op32, [["$FUNC","work_group_logical_and_util"]]>;
OclBuiltinImpl work_group_reduce_logical_and_finalize_helper_gen64 =
      OclBuiltinImpl<work_group_reduce_logical_and_finalize_helper, [v64i32],   0, work_group_finalize_function_op64, [["$FUNC","work_group_logical_and_util"]]>;

OclBuiltinImpl work_group_reduce_logical_or_finalize_helper_gen4 =
      OclBuiltinImpl<work_group_reduce_logical_or_finalize_helper, [v4i32],    0, work_group_finalize_function_op4, [["$FUNC","work_group_logical_or_util"]]>;
OclBuiltinImpl work_group_reduce_logical_or_finalize_helper_gen8 =
      OclBuiltinImpl<work_group_reduce_logical_or_finalize_helper, [v8i32],    0, work_group_finalize_function_op8, [["$FUNC","work_group_logical_or_util"]]>;
OclBuiltinImpl work_group_reduce_logical_or_finalize_helper_gen16 =
      OclBuiltinImpl<work_group_reduce_logical_or_finalize_helper, [v16i32],   0, work_group_finalize_function_op16, [["$FUNC","work_group_logical_or_util"]]>;
OclBuiltinImpl work_group_reduce_logical_or_finalize_helper_gen32 =
      OclBuiltinImpl<work_group_reduce_logical_or_finalize_helper, [v32i32],   0, work_group_finalize_function_op32, [["$FUNC","work_group_logical_or_util"]]>;
OclBuiltinImpl work_group_reduce_logical_or_finalize_helper_gen64 =
      OclBuiltinImpl<work_group_reduce_logical_or_finalize_helper, [v64i32],   0, work_group_finalize_function_op64, [["$FUNC","work_group_logical_or_util"]]>;

OclBuiltinImpl work_group_reduce_logical_xor_finalize_helper_gen4 =
      OclBuiltinImpl<work_group_reduce_logical_xor_finalize_helper, [v4i32],    0, work_group_finalize_function_op4, [["$FUNC","work_group_logical_xor_util"]]>;
OclBuiltinImpl work_group_reduce_logical_xor_finalize_helper_gen8 =
      OclBuiltinImpl<work_group_reduce_logical_xor_finalize_helper, [v8i32],    0, work_group_finalize_function_op8, [["$FUNC","work_group_logical_xor_util"]]>;
OclBuiltinImpl work_group_reduce_logical_xor_finalize_helper_gen16 =
      OclBuiltinImpl<work_group_reduce_logical_xor_finalize_helper, [v16i32],   0, work_group_finalize_function_op16, [["$FUNC","work_group_logical_xor_util"]]>;
OclBuiltinImpl work_group_reduce_logical_xor_finalize_helper_gen32 =
      OclBuiltinImpl<work_group_reduce_logical_xor_finalize_helper, [v32i32],   0, work_group_finalize_function_op32, [["$FUNC","work_group_logical_xor_util"]]>;
OclBuiltinImpl work_group_reduce_logical_xor_finalize_helper_gen64 =
      OclBuiltinImpl<work_group_reduce_logical_xor_finalize_helper, [v64i32],   0, work_group_finalize_function_op64, [["$FUNC","work_group_logical_xor_util"]]>;

OclBuiltinImpl work_group_reduce_min_finalize_helper_int4 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper, work_group_vector_types_int4,    0, work_group_finalize_function_op4, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_int8 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper, work_group_vector_types_int8,    0, work_group_finalize_function_op8, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_int16 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper, work_group_vector_types_int16,   0, work_group_finalize_function_op16, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_int32 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper, work_group_vector_types_int32,   0, work_group_finalize_function_op32, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_int64 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper, work_group_vector_types_int64,   0, work_group_finalize_function_op64, [["$FUNC","min"]]>;

OclBuiltinImpl work_group_reduce_min_finalize_helper_float4 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper, work_group_vector_types_float4,  0, work_group_finalize_function_op4, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_float8 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper, work_group_vector_types_float8,  0, work_group_finalize_function_op8, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_float16 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper, work_group_vector_types_float16, 0, work_group_finalize_function_op16, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_float32 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper, work_group_vector_types_float32, 0, work_group_finalize_function_op32, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_float64 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper, work_group_vector_types_float64, 0, work_group_finalize_function_op64, [["$FUNC","fmin"]]>;

OclBuiltinImpl work_group_reduce_max_finalize_helper_int4 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper, work_group_vector_types_int4,    0, work_group_finalize_function_op4, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_int8 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper, work_group_vector_types_int8,    0, work_group_finalize_function_op8, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_int16 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper, work_group_vector_types_int16,   0, work_group_finalize_function_op16, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_int32 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper, work_group_vector_types_int32,   0, work_group_finalize_function_op32, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_int64 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper, work_group_vector_types_int64,   0, work_group_finalize_function_op64, [["$FUNC","max"]]>;

OclBuiltinImpl work_group_reduce_max_finalize_helper_float4 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper, work_group_vector_types_float4,  0, work_group_finalize_function_op4, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_float8 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper, work_group_vector_types_float8,  0, work_group_finalize_function_op8, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_float16 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper, work_group_vector_types_float16, 0, work_group_finalize_function_op16, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_float32 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper, work_group_vector_types_float64, 0, work_group_finalize_function_op64, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_float64 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper, work_group_vector_types_float64, 0, work_group_finalize_function_op64, [["$FUNC","fmax"]]>;

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

OclBuiltinImpl work_group_reduce_add_finalize_helper_mask_gen4 =
      OclBuiltinImpl<work_group_reduce_add_finalize_helper_mask, work_group_vector_types_all4,    0, work_group_finalize_function_op4, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_reduce_add_finalize_helper_mask_gen8 =
      OclBuiltinImpl<work_group_reduce_add_finalize_helper_mask, work_group_vector_types_all8,    0, work_group_finalize_function_op8, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_reduce_add_finalize_helper_mask_gen16 =
      OclBuiltinImpl<work_group_reduce_add_finalize_helper_mask, work_group_vector_types_all16,   0, work_group_finalize_function_op16, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_reduce_add_finalize_helper_mask_gen32 =
      OclBuiltinImpl<work_group_reduce_add_finalize_helper_mask, work_group_vector_types_all32,   0, work_group_finalize_function_op32, [["$FUNC","work_group_add_util"]]>;
OclBuiltinImpl work_group_reduce_add_finalize_helper_mask_gen64 =
      OclBuiltinImpl<work_group_reduce_add_finalize_helper_mask, work_group_vector_types_all64,   0, work_group_finalize_function_op64, [["$FUNC","work_group_add_util"]]>;

OclBuiltinImpl work_group_reduce_mul_finalize_helper_mask_gen4 =
      OclBuiltinImpl<work_group_reduce_mul_finalize_helper_mask, work_group_vector_types_all4,    0, work_group_finalize_function_op4, [["$FUNC","work_group_mul_util"]]>;
OclBuiltinImpl work_group_reduce_mul_finalize_helper_mask_gen8 =
      OclBuiltinImpl<work_group_reduce_mul_finalize_helper_mask, work_group_vector_types_all8,    0, work_group_finalize_function_op8, [["$FUNC","work_group_mul_util"]]>;
OclBuiltinImpl work_group_reduce_mul_finalize_helper_mask_gen16 =
      OclBuiltinImpl<work_group_reduce_mul_finalize_helper_mask, work_group_vector_types_all16,   0, work_group_finalize_function_op16, [["$FUNC","work_group_mul_util"]]>;
OclBuiltinImpl work_group_reduce_mul_finalize_helper_mask_gen32 =
      OclBuiltinImpl<work_group_reduce_mul_finalize_helper_mask, work_group_vector_types_all32,   0, work_group_finalize_function_op32, [["$FUNC","work_group_mul_util"]]>;
OclBuiltinImpl work_group_reduce_mul_finalize_helper_mask_gen64 =
      OclBuiltinImpl<work_group_reduce_mul_finalize_helper_mask, work_group_vector_types_all64,   0, work_group_finalize_function_op64, [["$FUNC","work_group_mul_util"]]>;

OclBuiltinImpl work_group_reduce_min_finalize_helper_mask_int4 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper_mask, work_group_vector_types_int4,    0, work_group_finalize_function_op4, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_mask_int8 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper_mask, work_group_vector_types_int8,    0, work_group_finalize_function_op8, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_mask_int16 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper_mask, work_group_vector_types_int16,   0, work_group_finalize_function_op16, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_mask_int32 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper_mask, work_group_vector_types_int32,   0, work_group_finalize_function_op32, [["$FUNC","min"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_mask_int64 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper_mask, work_group_vector_types_int64,   0, work_group_finalize_function_op64, [["$FUNC","min"]]>;

OclBuiltinImpl work_group_reduce_min_finalize_helper_mask_float4 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper_mask, work_group_vector_types_float4,  0, work_group_finalize_function_op4, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_mask_float8 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper_mask, work_group_vector_types_float8,  0, work_group_finalize_function_op8, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_mask_float16 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper_mask, work_group_vector_types_float16, 0, work_group_finalize_function_op16, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_mask_float32 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper_mask, work_group_vector_types_float32, 0, work_group_finalize_function_op32, [["$FUNC","fmin"]]>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_mask_float64 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper_mask, work_group_vector_types_float64, 0, work_group_finalize_function_op64, [["$FUNC","fmin"]]>;

OclBuiltinImpl work_group_reduce_max_finalize_helper_mask_int4 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper_mask, work_group_vector_types_int4,    0, work_group_finalize_function_op4, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_mask_int8 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper_mask, work_group_vector_types_int8,    0, work_group_finalize_function_op8, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_mask_int16 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper_mask, work_group_vector_types_int16,   0, work_group_finalize_function_op16, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_mask_int32 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper_mask, work_group_vector_types_int32,   0, work_group_finalize_function_op32, [["$FUNC","max"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_mask_int64 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper_mask, work_group_vector_types_int64,   0, work_group_finalize_function_op64, [["$FUNC","max"]]>;

OclBuiltinImpl work_group_reduce_max_finalize_helper_mask_float4 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper_mask, work_group_vector_types_float4,  0, work_group_finalize_function_op4, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_mask_float8 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper_mask, work_group_vector_types_float8,  0, work_group_finalize_function_op8, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_mask_float16 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper_mask, work_group_vector_types_float16, 0, work_group_finalize_function_op16, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_mask_float32 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper_mask, work_group_vector_types_float32, 0, work_group_finalize_function_op32, [["$FUNC","fmax"]]>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_mask_float64 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper_mask, work_group_vector_types_float64, 0, work_group_finalize_function_op64, [["$FUNC","fmax"]]>;
