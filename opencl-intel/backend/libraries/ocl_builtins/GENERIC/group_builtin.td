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
    __builtin_memcpy((__private char*)$Arg0VarName, (const __private char*)$Arg1VarName, $Arg2VarName*sizeof(*$Arg0VarName));
    return $Arg3VarName;
  }];
code AsyncWorkGroupCopyCode_g2l = [{
    __builtin_memcpy((__private char*)$Arg0VarName, (const __private char*)$Arg1VarName, $Arg2VarName*sizeof(*$Arg0VarName));
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
OclBuiltinImpl wait_group_events_impl = OclBuiltinImpl<wait_group_events, wait_group_events.Types, 0, [{
  }]>;


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

// work_group_all
OclBuiltinImpl work_group_all_vector_helper_int = 
      OclBuiltinImpl<work_group_all_helper, [v4i32, v8i32, v16i32],               0, snippet_work_group_all_vector_helper_int.specialized>;
OclBuiltinImpl work_group_all_scalar_helper_int = 
      OclBuiltinImpl<work_group_all_helper, [v1i32],                              0, snippet_work_group_all_scalar_helper_int.specialized>;

// work_group_any
OclBuiltinImpl work_group_any_vector_helper_int = 
      OclBuiltinImpl<work_group_any_helper, [v4i32, v8i32, v16i32],               0, snippet_work_group_any_vector_helper_int.specialized>;
OclBuiltinImpl work_group_any_scalar_helper_int = 
      OclBuiltinImpl<work_group_any_helper, [v1i32],                              0, snippet_work_group_any_scalar_helper_int.specialized>;

// work_group_reduce_add
OclBuiltinImpl work_group_reduce_add_vector_helper_generic = 
      OclBuiltinImpl<work_group_reduce_add_helper, work_group_vector_types_all,   0, snippet_work_group_reduce_add_vector_helper_generic.specialized>;
OclBuiltinImpl work_group_reduce_add_scalar_helper_generic = 
      OclBuiltinImpl<work_group_reduce_add_helper, work_group_scalar_types_all,   0, snippet_work_group_reduce_add_scalar_helper_generic.specialized>;

// work_group_reduce_max/min
OclBuiltinImpl work_group_reduce_min_vector_helper_int = 
      OclBuiltinImpl<work_group_reduce_min_helper, work_group_vector_types_int,   0, snippet_work_group_reduce_min_vector_helper_int.specialized>;
OclBuiltinImpl work_group_reduce_min_scalar_helper_int =
      OclBuiltinImpl<work_group_reduce_min_helper, work_group_scalar_types_int,   0, snippet_work_group_reduce_min_scalar_helper_int.specialized>;
OclBuiltinImpl work_group_reduce_min_vector_helper_float =
      OclBuiltinImpl<work_group_reduce_min_helper, work_group_vector_types_float, 0, snippet_work_group_reduce_min_vector_helper_float.specialized>;
OclBuiltinImpl work_group_reduce_min_scalar_helper_float =
      OclBuiltinImpl<work_group_reduce_min_helper, work_group_scalar_types_float, 0, snippet_work_group_reduce_min_scalar_helper_float.specialized>;
OclBuiltinImpl work_group_reduce_max_vector_helper_int =
      OclBuiltinImpl<work_group_reduce_max_helper, work_group_vector_types_int,   0, snippet_work_group_reduce_max_vector_helper_int.specialized>;
OclBuiltinImpl work_group_reduce_max_scalar_helper_int =
      OclBuiltinImpl<work_group_reduce_max_helper, work_group_scalar_types_int,   0, snippet_work_group_reduce_max_scalar_helper_int.specialized>;
OclBuiltinImpl work_group_reduce_max_vector_helper_float =
      OclBuiltinImpl<work_group_reduce_max_helper, work_group_vector_types_float, 0, snippet_work_group_reduce_max_vector_helper_float.specialized>;
OclBuiltinImpl work_group_reduce_max_scalar_helper_float =
      OclBuiltinImpl<work_group_reduce_max_helper, work_group_scalar_types_float, 0, snippet_work_group_reduce_max_scalar_helper_float.specialized>;

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
      $Arg0ugentype mask = ($Arg0ugentype)(offset);
      $Arg0VecType acc = shuffle($Arg0VarName, mask);
      *$Arg3VarName = acc;
      return acc;
    } else {
      return *$Arg3VarName;
    }
  }]>;

//-----------------------------------------------
// 2. Non-uniform result
//-----------------------------------------------

// Vector versions of Scan_add/min/max functions
OclBuiltinImpl work_group_scan_exclusive_add_vector_helper_gen4 =
      OclBuiltinImpl<work_group_scan_exclusive_add_helper, work_group_vector_types_all4,    0, snippet_work_group_scan_exclusive_add_vector_helper_gen4.specialized>;
OclBuiltinImpl work_group_scan_exclusive_add_vector_helper_gen8 =
      OclBuiltinImpl<work_group_scan_exclusive_add_helper, work_group_vector_types_all8,    0, snippet_work_group_scan_exclusive_add_vector_helper_gen8.specialized>;
OclBuiltinImpl work_group_scan_exclusive_add_vector_helper_gen16 =
      OclBuiltinImpl<work_group_scan_exclusive_add_helper, work_group_vector_types_all16,   0, snippet_work_group_scan_exclusive_add_vector_helper_gen16.specialized>;
OclBuiltinImpl work_group_scan_exclusive_min_vector_helper_int4 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_vector_types_int4,    0, snippet_work_group_scan_exclusive_min_vector_helper_int4.specialized>;
OclBuiltinImpl work_group_scan_exclusive_min_vector_helper_int8 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_vector_types_int8,    0, snippet_work_group_scan_exclusive_min_vector_helper_int8.specialized>;
OclBuiltinImpl work_group_scan_exclusive_min_vector_helper_int16 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_vector_types_int16,   0, snippet_work_group_scan_exclusive_min_vector_helper_int16.specialized>;
OclBuiltinImpl work_group_scan_exclusive_min_vector_helper_float4 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_vector_types_float4,  0, snippet_work_group_scan_exclusive_min_vector_helper_float4.specialized>;
OclBuiltinImpl work_group_scan_exclusive_min_vector_helper_float8 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_vector_types_float8,  0, snippet_work_group_scan_exclusive_min_vector_helper_float8.specialized>;
OclBuiltinImpl work_group_scan_exclusive_min_vector_helper_float16 =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_vector_types_float16, 0, snippet_work_group_scan_exclusive_min_vector_helper_float16.specialized>;
OclBuiltinImpl work_group_scan_exclusive_max_vector_helper_int4 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_int4,    0, snippet_work_group_scan_exclusive_max_vector_helper_int4.specialized>;
OclBuiltinImpl work_group_scan_exclusive_max_vector_helper_int8 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_int8,    0, snippet_work_group_scan_exclusive_max_vector_helper_int8.specialized>;
OclBuiltinImpl work_group_scan_exclusive_max_vector_helper_int16 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_int16,   0, snippet_work_group_scan_exclusive_max_vector_helper_int16.specialized>;
OclBuiltinImpl work_group_scan_exclusive_max_vector_helper_float4 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_float4,  0, snippet_work_group_scan_exclusive_max_vector_helper_float4.specialized>;
OclBuiltinImpl work_group_scan_exclusive_max_vector_helper_float8 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_float8,  0, snippet_work_group_scan_exclusive_max_vector_helper_float8.specialized>;
OclBuiltinImpl work_group_scan_exclusive_max_vector_helper_float16 =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_vector_types_float16, 0, snippet_work_group_scan_exclusive_max_vector_helper_float16.specialized>;
OclBuiltinImpl work_group_scan_inclusive_add_vector_helper_gen4 =
      OclBuiltinImpl<work_group_scan_inclusive_add_helper, work_group_vector_types_all4,    0, snippet_work_group_scan_inclusive_add_vector_helper_gen4.specialized>;
OclBuiltinImpl work_group_scan_inclusive_add_vector_helper_gen8 =
      OclBuiltinImpl<work_group_scan_inclusive_add_helper, work_group_vector_types_all8,    0, snippet_work_group_scan_inclusive_add_vector_helper_gen8.specialized>;
OclBuiltinImpl work_group_scan_inclusive_add_vector_helper_gen16 =
      OclBuiltinImpl<work_group_scan_inclusive_add_helper, work_group_vector_types_all16,   0, snippet_work_group_scan_inclusive_add_vector_helper_gen16.specialized>;
OclBuiltinImpl work_group_scan_inclusive_min_vector_helper_int4 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_vector_types_int4,    0, snippet_work_group_scan_inclusive_min_vector_helper_int4.specialized>;
OclBuiltinImpl work_group_scan_inclusive_min_vector_helper_int8 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_vector_types_int8,    0, snippet_work_group_scan_inclusive_min_vector_helper_int8.specialized>;
OclBuiltinImpl work_group_scan_inclusive_min_vector_helper_int16 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_vector_types_int16,   0, snippet_work_group_scan_inclusive_min_vector_helper_int16.specialized>;
OclBuiltinImpl work_group_scan_inclusive_min_vector_helper_float4 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_vector_types_float4,  0, snippet_work_group_scan_inclusive_min_vector_helper_float4.specialized>;
OclBuiltinImpl work_group_scan_inclusive_min_vector_helper_float8 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_vector_types_float8,  0, snippet_work_group_scan_inclusive_min_vector_helper_float8.specialized>;
OclBuiltinImpl work_group_scan_inclusive_min_vector_helper_float16 =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_vector_types_float16, 0, snippet_work_group_scan_inclusive_min_vector_helper_float16.specialized>;
OclBuiltinImpl work_group_scan_inclusive_max_vector_helper_int4 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_int4,    0, snippet_work_group_scan_inclusive_max_vector_helper_int4.specialized>;
OclBuiltinImpl work_group_scan_inclusive_max_vector_helper_int8 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_int8,    0, snippet_work_group_scan_inclusive_max_vector_helper_int8.specialized>;
OclBuiltinImpl work_group_scan_inclusive_max_vector_helper_int16 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_int16,   0, snippet_work_group_scan_inclusive_max_vector_helper_int16.specialized>;
OclBuiltinImpl work_group_scan_inclusive_max_vector_helper_float4 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_float4,  0, snippet_work_group_scan_inclusive_max_vector_helper_float4.specialized>;
OclBuiltinImpl work_group_scan_inclusive_max_vector_helper_float8 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_float8,  0, snippet_work_group_scan_inclusive_max_vector_helper_float8.specialized>;
OclBuiltinImpl work_group_scan_inclusive_max_vector_helper_float16 =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_vector_types_float16, 0, snippet_work_group_scan_inclusive_max_vector_helper_float16.specialized>;

// Scalar versions of Scan_add/min/max functions
OclBuiltinImpl work_group_scan_exclusive_add_scalar_helper_gen =
      OclBuiltinImpl<work_group_scan_exclusive_add_helper, work_group_scalar_types_all,     0, snippet_work_group_scan_exclusive_add_scalar_helper_gen.specialized>;
OclBuiltinImpl work_group_scan_exclusive_min_scalar_helper_int =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_scalar_types_int,     0, snippet_work_group_scan_exclusive_min_scalar_helper_int.specialized>;
OclBuiltinImpl work_group_scan_exclusive_min_scalar_helper_float =
      OclBuiltinImpl<work_group_scan_exclusive_min_helper, work_group_scalar_types_float,   0, snippet_work_group_scan_exclusive_min_scalar_helper_float.specialized>;
OclBuiltinImpl work_group_scan_exclusive_max_scalar_helper_int =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_scalar_types_int,     0, snippet_work_group_scan_exclusive_max_scalar_helper_int.specialized>;
OclBuiltinImpl work_group_scan_exclusive_max_scalar_helper_float =
      OclBuiltinImpl<work_group_scan_exclusive_max_helper, work_group_scalar_types_float,   0, snippet_work_group_scan_exclusive_max_scalar_helper_float.specialized>;
OclBuiltinImpl work_group_scan_inclusive_add_scalar_helper_gen =
      OclBuiltinImpl<work_group_scan_inclusive_add_helper, work_group_scalar_types_all,     0, snippet_work_group_scan_inclusive_add_scalar_helper_gen.specialized>;
OclBuiltinImpl work_group_scan_inclusive_min_scalar_helper_int =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_scalar_types_int,     0, snippet_work_group_scan_inclusive_min_scalar_helper_int.specialized>;
OclBuiltinImpl work_group_scan_inclusive_min_scalar_helper_float =
      OclBuiltinImpl<work_group_scan_inclusive_min_helper, work_group_scalar_types_float,   0, snippet_work_group_scan_inclusive_min_scalar_helper_float.specialized>;
OclBuiltinImpl work_group_scan_inclusive_max_scalar_helper_int =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_scalar_types_int,     0, snippet_work_group_scan_inclusive_max_scalar_helper_int.specialized>;
OclBuiltinImpl work_group_scan_inclusive_max_scalar_helper_float =
      OclBuiltinImpl<work_group_scan_inclusive_max_helper, work_group_scalar_types_float,   0, snippet_work_group_scan_inclusive_max_scalar_helper_float.specialized>;


//-----------------------------------------------
// 3. Finalization of uniform result
//-----------------------------------------------


// Definitions of finalization functions
OclBuiltinImpl work_group_all_finalize_helper_int4 =
      OclBuiltinImpl<work_group_all_finalize_helper,        [v4i32],                         0, snippet_work_group_all_finalize_helper_int4.specialized>;
OclBuiltinImpl work_group_all_finalize_helper_int8 =
      OclBuiltinImpl<work_group_all_finalize_helper,        [v8i32],                         0, snippet_work_group_all_finalize_helper_int8.specialized>;
OclBuiltinImpl work_group_all_finalize_helper_int16 =
      OclBuiltinImpl<work_group_all_finalize_helper,        [v16i32],                        0, snippet_work_group_all_finalize_helper_int16.specialized>;
OclBuiltinImpl work_group_any_finalize_helper_int4 =
      OclBuiltinImpl<work_group_any_finalize_helper,        [v4i32],                         0, snippet_work_group_any_finalize_helper_int4.specialized>;
OclBuiltinImpl work_group_any_finalize_helper_int8 =
      OclBuiltinImpl<work_group_any_finalize_helper,        [v8i32],                         0, snippet_work_group_any_finalize_helper_int8.specialized>;
OclBuiltinImpl work_group_any_finalize_helper_int16 =
      OclBuiltinImpl<work_group_any_finalize_helper,        [v16i32],                        0, snippet_work_group_any_finalize_helper_int16.specialized>;
OclBuiltinImpl work_group_reduce_add_finalize_helper_gen4 =
      OclBuiltinImpl<work_group_reduce_add_finalize_helper, work_group_vector_types_all4,    0, snippet_work_group_reduce_add_finalize_helper_gen4.specialized>;
OclBuiltinImpl work_group_reduce_add_finalize_helper_gen8 =
      OclBuiltinImpl<work_group_reduce_add_finalize_helper, work_group_vector_types_all8,    0, snippet_work_group_reduce_add_finalize_helper_gen8.specialized>;
OclBuiltinImpl work_group_reduce_add_finalize_helper_gen16 =
      OclBuiltinImpl<work_group_reduce_add_finalize_helper, work_group_vector_types_all16,   0, snippet_work_group_reduce_add_finalize_helper_gen16.specialized>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_int4 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper, work_group_vector_types_int4,    0, snippet_work_group_reduce_min_finalize_helper_int4.specialized>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_int8 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper, work_group_vector_types_int8,    0, snippet_work_group_reduce_min_finalize_helper_int8.specialized>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_int16 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper, work_group_vector_types_int16,   0, snippet_work_group_reduce_min_finalize_helper_int16.specialized>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_float4 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper, work_group_vector_types_float4,  0, snippet_work_group_reduce_min_finalize_helper_float4.specialized>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_float8 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper, work_group_vector_types_float8,  0, snippet_work_group_reduce_min_finalize_helper_float8.specialized>;
OclBuiltinImpl work_group_reduce_min_finalize_helper_float16 =
      OclBuiltinImpl<work_group_reduce_min_finalize_helper, work_group_vector_types_float16, 0, snippet_work_group_reduce_min_finalize_helper_float16.specialized>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_int4 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper, work_group_vector_types_int4,    0, snippet_work_group_reduce_max_finalize_helper_int4.specialized>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_int8 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper, work_group_vector_types_int8,    0, snippet_work_group_reduce_max_finalize_helper_int8.specialized>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_int16 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper, work_group_vector_types_int16,   0, snippet_work_group_reduce_max_finalize_helper_int16.specialized>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_float4 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper, work_group_vector_types_float4,  0, snippet_work_group_reduce_max_finalize_helper_float4.specialized>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_float8 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper, work_group_vector_types_float8,  0, snippet_work_group_reduce_max_finalize_helper_float8.specialized>;
OclBuiltinImpl work_group_reduce_max_finalize_helper_float16 =
      OclBuiltinImpl<work_group_reduce_max_finalize_helper, work_group_vector_types_float16, 0, snippet_work_group_reduce_max_finalize_helper_float16.specialized>;
