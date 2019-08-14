// In our implementation a work-group is a sequence of work-items executed by a single phy. thread
// in sequence. So no synchronization is needed among work-items of the same work-group operating on
// the __local memory.
code add_g = [{
    return __sync_fetch_and_add($Arg0VarName, $Arg1VarName);
  }];
code add_l = [{
    // this implementation relies on that a workgroup is executed by single phy. thread
    local $ReturnType* tmp = (local $ReturnType*)$Arg0VarName;
    $ReturnType oldValue = *tmp;
    *tmp = oldValue + $Arg1VarName;
    return oldValue;
  }];
code add_g_fp32 = [{
    while (true)
    {
      $ReturnType oldValue = *$Arg0VarName;
      int oldValueInt = *((int*)&oldValue);
      $ReturnType addValue = oldValue + $Arg1VarName;
      int addInt = *((int*)&addValue);
      if (oldValueInt == addInt) return oldValue;
      int retValInt = __sync_val_compare_and_swap((int*)$Arg0VarName, oldValueInt, addInt);
      if (oldValueInt == retValInt) return oldValue;
    }
  }];
code sub_g = [{
    return __sync_fetch_and_sub($Arg0VarName, $Arg1VarName);
  }];
code sub_l = [{
    // this implementation relies on that a workgroup is executed by single phy. thread
    local $ReturnType* tmp = (local $ReturnType*)$Arg0VarName;
    $ReturnType oldValue = *tmp;
    *tmp = oldValue - $Arg1VarName;
    return oldValue;
  }];
code xchg_g = [{
    _mm_mfence();
    return ($ReturnType)__sync_lock_test_and_set($Arg0VarName, $Arg1VarName);
  }];
code xchg_l = [{
    local $ReturnType* tmp = (local $ReturnType*)$Arg0VarName;
    // this implementation relies on that a workgroup is executed by single phy. thread
    $ReturnType oldValue = *tmp;
    *tmp = $Arg1VarName;
    return oldValue;
  }];
code xchg_g_fp = [{
    _mm_mfence();
    int ret = ( __sync_lock_test_and_set((int*)$Arg0VarName, *((int *)(&$Arg1VarName))) );
    return as_float(ret);
  }];

code max_g = [{
    while (true)
    {
      $ReturnType oldValue = *$Arg0VarName;
      if (oldValue >= $Arg1VarName) return oldValue;
      $ReturnType retVal = __sync_val_compare_and_swap(($ReturnType *)$Arg0VarName, oldValue, $Arg1VarName);
      if (retVal == oldValue) return oldValue;
    }
  }];
code max_l = [{
    local $ReturnType* tmp = (local $ReturnType*)$Arg0VarName;
    $ReturnType oldValue = *tmp;
    *tmp = (($Arg1VarName > oldValue) ? $Arg1VarName : oldValue);
    return oldValue;
  }];
// floating point min/max.
// OpenCL C spec says:
//
// "fmin and fmax behave as defined by C99 and may not match the IEEE
// 754-2008 definition for minNum and maxNum with regard to signaling NaNs.
// Specifically, signaling NaNs may behave as quiet NaNs,"
// referrint to C for signed zero behavior.
//
// According to C standard:
//
// fmin(NaN,-1) = -1.000000
// fmin(-Inf,0) = -inf
// fmin(+/-Inf, NaN) = +/-Inf
//
// However,
// fmin(-0, +0) = ??
//
// C standard says:
//
// "Ideally, fmax would be sensitive to the sign of zero, for example
// fmax(-0.0, +0.0) would return +0; however, implementation in software
// might be impractical", so the implementation can choose either sign."
//
// SVML fmin/fmax functions comply with this behavior, so I reused them.
// In turn that means that desired behaviour of comparing -0 and +0
// is not supported.
code max_g_fp32 = [{
    while (true)
    {
      $ReturnType oldValue = *$Arg0VarName;
      int oldValueInt = *((int*)&oldValue);
      $ReturnType max = fmax(oldValue, $Arg1VarName);
      int maxInt = *((int*)&max);
      if (oldValueInt == maxInt) return oldValue;
      int retValInt = __sync_val_compare_and_swap((int*)$Arg0VarName, oldValueInt, maxInt);
      if (retValInt == oldValueInt) return oldValue;
    }
  }];
code max_l_fp = [{
    local $ReturnType* tmp = (local $ReturnType*)$Arg0VarName;
    $ReturnType oldValue = *tmp;
    *tmp = fmax($Arg1VarName, oldValue);
    return oldValue;
  }];
code min_g = [{
    while (true)
    {
      $ReturnType oldValue = *$Arg0VarName;
      if (oldValue <= $Arg1VarName) return oldValue;
      $ReturnType retVal = __sync_val_compare_and_swap(($ReturnType *)$Arg0VarName, oldValue, $Arg1VarName);
      if (retVal == oldValue) return oldValue;
    }
  }];
code min_l = [{
    local $ReturnType* tmp = (local $ReturnType*)$Arg0VarName;
    $ReturnType oldValue = *tmp;
    *tmp = (($Arg1VarName < oldValue) ? $Arg1VarName : oldValue);
    return oldValue;
  }];
code min_g_fp32 = [{
    while (true)
    {
      $ReturnType oldValue = *$Arg0VarName;
      int oldValueInt = *((int*) &oldValue);
      $ReturnType min = fmin(oldValue, $Arg1VarName);
      int minInt = *((int*)&min);
      if (oldValueInt == minInt)  return oldValue;
      int retValInt = __sync_val_compare_and_swap((int *)$Arg0VarName, oldValueInt, minInt);
      if (retValInt == oldValueInt) return oldValue;
    }
  }];
code min_l_fp = [{
    local $ReturnType* tmp = (local $ReturnType*)$Arg0VarName;
    $ReturnType oldValue = *tmp;
    *tmp = fmin($Arg1VarName, oldValue);
    return oldValue;
  }];
code inc_g = [{
    return __sync_fetch_and_add($Arg0VarName, 1);
  }];
code inc_l = [{
    // this implementation relies on that a workgroup is executed by single phy. thread
    local $ReturnType* tmp = (local $ReturnType*)$Arg0VarName;
    $ReturnType oldValue = *tmp;
    *tmp = oldValue + 1;
    return oldValue;
  }];
code dec_g = [{
    return __sync_fetch_and_sub($Arg0VarName, 1);
  }];
code dec_l = [{
    // this implementation relies on that a workgroup is executed by single phy. thread
    local $ReturnType* tmp = (local $ReturnType*)$Arg0VarName;
    $ReturnType oldValue = *tmp;
    *tmp = oldValue - 1;
    return oldValue;
  }];
code cmpxchg_g = [{
    return __sync_val_compare_and_swap($Arg0VarName, $Arg1VarName, $Arg2VarName);
  }];
code cmpxchg_g_fp32 = [{
    return as_float(__sync_val_compare_and_swap((int*)$Arg0VarName, as_int($Arg1VarName), as_int($Arg2VarName)));
  }];
code cmpxchg_l = [{
    // this implementation relies on that a workgroup is executed by single phy. thread
    local $ReturnType* tmp = (local $ReturnType*)$Arg0VarName;
    $ReturnType oldValue = *tmp;
    if (oldValue == $Arg1VarName)
    {
      *tmp = $Arg2VarName;
    }
    return oldValue;
  }];
code and_g = [{
    return __sync_fetch_and_and($Arg0VarName, $Arg1VarName);
  }];
code and_l = [{
    // this implementation relies on that a workgroup is executed by single phy. thread
    local $ReturnType* tmp = (local $ReturnType*)$Arg0VarName;
    $ReturnType oldValue = *tmp;
    *tmp = oldValue & $Arg1VarName;
    return oldValue;
  }];
code or_g = [{
    return __sync_fetch_and_or($Arg0VarName, $Arg1VarName);
  }];
code or_l = [{
    // this implementation relies on that a workgroup is executed by single phy. thread
    local $ReturnType* tmp = (local $ReturnType*)$Arg0VarName;
    $ReturnType oldValue = *tmp;
    *tmp = oldValue | $Arg1VarName;
    return oldValue;
  }];
code xor_g = [{
    return __sync_fetch_and_xor($Arg0VarName, $Arg1VarName);
  }];
code xor_l = [{
    // this implementation relies on that a workgroup is executed by single phy. thread
    local $ReturnType* tmp = (local $ReturnType*)$Arg0VarName;
    $ReturnType oldValue = *tmp;
    *tmp = oldValue ^ $Arg1VarName;
    return oldValue;
  }];


//=============================================

// inc
OclBuiltinImpl atomic_inc_g   = OclBuiltinImpl<atomic_inc_global, [v1i32, v1u32], 0, inc_g>;
OclBuiltinImpl atomic_inc_l   = OclBuiltinImpl<atomic_inc_local, [v1i32, v1u32], 0, inc_l>;
OclBuiltinImpl atom_inc_vg    = OclBuiltinImpl<atom_inc_vol_global, [v1i32, v1u32, v1i64, v1u64], 0, inc_g>;
OclBuiltinImpl atom_inc_g     = OclBuiltinImpl<atom_inc_global, [v1i32, v1u32, v1i64, v1u64], 0, inc_g>;
OclBuiltinImpl atom_inc_vl    = OclBuiltinImpl<atom_inc_vol_local, [v1i32, v1u32, v1i64, v1u64], 0, inc_l>;
OclBuiltinImpl atom_inc_l     = OclBuiltinImpl<atom_inc_local, [v1i32, v1u32, v1i64, v1u64], 0, inc_l>;
// dec
OclBuiltinImpl atomic_dec_g   = OclBuiltinImpl<atomic_dec_global, [v1i32, v1u32], 0, dec_g>;
OclBuiltinImpl atomic_dec_l   = OclBuiltinImpl<atomic_dec_local, [v1i32, v1u32], 0, dec_l>;
OclBuiltinImpl atom_dec_vg    = OclBuiltinImpl<atom_dec_vol_global, [v1i32, v1u32, v1i64, v1u64], 0, dec_g>;
OclBuiltinImpl atom_dec_g     = OclBuiltinImpl<atom_dec_global, [v1i32, v1u32, v1i64, v1u64], 0, dec_g>;
OclBuiltinImpl atom_dec_vl    = OclBuiltinImpl<atom_dec_vol_local, [v1i32, v1u32, v1i64, v1u64], 0, dec_l>;
OclBuiltinImpl atom_dec_l     = OclBuiltinImpl<atom_dec_local, [v1i32, v1u32, v1i64, v1u64], 0, dec_l>;
// add
OclBuiltinImpl atomic_add_g   = OclBuiltinImpl<atomic_add_global, [v1i32, v1u32], 0, add_g>;
OclBuiltinImpl atomic_add_l   = OclBuiltinImpl<atomic_add_local, [v1i32, v1u32, v1f32], 0, add_l>;
OclBuiltinImpl atom_add_vg    = OclBuiltinImpl<atom_add_vol_global, [v1i32, v1u32, v1i64, v1u64], 0, add_g>;
OclBuiltinImpl atom_add_g     = OclBuiltinImpl<atom_add_global, [v1i32, v1u32, v1i64, v1u64], 0, add_g>;
OclBuiltinImpl atom_add_vl    = OclBuiltinImpl<atom_add_vol_local, [v1i32, v1u32, v1f32, v1i64, v1u64], 0, add_l>;
OclBuiltinImpl atom_add_l     = OclBuiltinImpl<atom_add_local, [v1i32, v1u32, v1f32, v1i64, v1u64], 0, add_l>;

OclBuiltinImpl atomic_add_gf   = OclBuiltinImpl<atomic_add_global, [v1f32], 0, add_g_fp32>;
OclBuiltinImpl atom_add_vgf    = OclBuiltinImpl<atom_add_vol_global, [v1f32], 0, add_g_fp32>;
OclBuiltinImpl atom_add_gf     = OclBuiltinImpl<atom_add_global, [v1f32], 0, add_g_fp32>;
// sub
OclBuiltinImpl atomic_sub_g   = OclBuiltinImpl<atomic_sub_global, [v1i32, v1u32], 0, sub_g>;
OclBuiltinImpl atomic_sub_l   = OclBuiltinImpl<atomic_sub_local, [v1i32, v1u32], 0, sub_l>;
OclBuiltinImpl atom_sub_vg    = OclBuiltinImpl<atom_sub_vol_global, [v1i32, v1u32, v1i64, v1u64], 0, sub_g>;
OclBuiltinImpl atom_sub_g     = OclBuiltinImpl<atom_sub_global, [v1i32, v1u32, v1i64, v1u64], 0, sub_g>;
OclBuiltinImpl atom_sub_vl    = OclBuiltinImpl<atom_sub_vol_local, [v1i32, v1u32, v1i64, v1u64], 0, sub_l>;
OclBuiltinImpl atom_sub_l     = OclBuiltinImpl<atom_sub_local, [v1i32, v1u32, v1i64, v1u64], 0, sub_l>;
// xchg
OclBuiltinImpl atomic_xchg_g   = OclBuiltinImpl<atomic_xchg_global, [v1i32, v1u32], 0, xchg_g>;
OclBuiltinImpl atomic_xchg_l   = OclBuiltinImpl<atomic_xchg_local, [v1i32, v1u32, v1f32], 0, xchg_l>;
OclBuiltinImpl atom_xchg_vg    = OclBuiltinImpl<atom_xchg_vol_global, [v1i32, v1u32, v1i64, v1u64], 0, xchg_g>;
OclBuiltinImpl atom_xchg_g     = OclBuiltinImpl<atom_xchg_global, [v1i32, v1u32, v1i64, v1u64], 0, xchg_g>;
OclBuiltinImpl atom_xchg_vl    = OclBuiltinImpl<atom_xchg_vol_local, [v1i32, v1u32, v1f32, v1i64, v1u64], 0, xchg_l>;
OclBuiltinImpl atom_xchg_l     = OclBuiltinImpl<atom_xchg_local, [v1i32, v1u32, v1f32, v1i64, v1u64], 0, xchg_l>;

OclBuiltinImpl atomic_xchg_gf   = OclBuiltinImpl<atomic_xchg_global, [v1f32], 0, xchg_g_fp>;
OclBuiltinImpl atom_xchg_vgf    = OclBuiltinImpl<atom_xchg_vol_global, [v1f32], 0, xchg_g_fp>;
OclBuiltinImpl atom_xchg_gf     = OclBuiltinImpl<atom_xchg_global, [v1f32], 0, xchg_g_fp>;
// cmpxchg
OclBuiltinImpl atomic_cmpxchg_g   = OclBuiltinImpl<atomic_cmpxchg_global, [v1i32, v1u32], 0, cmpxchg_g>;
OclBuiltinImpl atomic_cmpxchg_l   = OclBuiltinImpl<atomic_cmpxchg_local, [v1i32, v1u32, v1f32], 0, cmpxchg_l>;
OclBuiltinImpl atom_cmpxchg_vg    = OclBuiltinImpl<atom_cmpxchg_vol_global, [v1i32, v1u32, v1i64, v1u64], 0, cmpxchg_g>;
OclBuiltinImpl atom_cmpxchg_g     = OclBuiltinImpl<atom_cmpxchg_global, [v1i32, v1u32, v1i64, v1u64], 0, cmpxchg_g>;
OclBuiltinImpl atom_cmpxchg_vl    = OclBuiltinImpl<atom_cmpxchg_vol_local, [v1i32, v1u32, v1i64, v1u64], 0, cmpxchg_l>;
OclBuiltinImpl atom_cmpxchg_l     = OclBuiltinImpl<atom_cmpxchg_local, [v1i32, v1u32, v1i64, v1u64], 0, cmpxchg_l>;

OclBuiltinImpl atomic_cmpxchg_gf  = OclBuiltinImpl<atomic_cmpxchg_global, [v1f32], 0, cmpxchg_g_fp32>;
// max
OclBuiltinImpl atomic_max_g   = OclBuiltinImpl<atomic_max_global, [v1i32, v1u32], 0, max_g>;
OclBuiltinImpl atomic_max_l   = OclBuiltinImpl<atomic_max_local, [v1i32, v1u32], 0, max_l>;
OclBuiltinImpl atom_max_vg    = OclBuiltinImpl<atom_max_vol_global, [v1i32, v1u32, v1i64, v1u64], 0, max_g>;
OclBuiltinImpl atom_max_g     = OclBuiltinImpl<atom_max_global, [v1i32, v1u32, v1i64, v1u64], 0, max_g>;
OclBuiltinImpl atom_max_vl    = OclBuiltinImpl<atom_max_vol_local, [v1i32, v1u32, v1i64, v1u64], 0, max_l>;
OclBuiltinImpl atom_max_l     = OclBuiltinImpl<atom_max_local, [v1i32, v1u32, v1i64, v1u64], 0, max_l>;

OclBuiltinImpl atomic_max_gf  = OclBuiltinImpl<atomic_max_global, [v1f32], 0, max_g_fp32>;
OclBuiltinImpl atomic_max_lf  = OclBuiltinImpl<atomic_max_local, [v1f32], 0, max_l_fp>;
// min
OclBuiltinImpl atomic_min_g   = OclBuiltinImpl<atomic_min_global, [v1i32, v1u32], 0, min_g>;
OclBuiltinImpl atomic_min_l   = OclBuiltinImpl<atomic_min_local, [v1i32, v1u32], 0, min_l>;
OclBuiltinImpl atom_min_vg    = OclBuiltinImpl<atom_min_vol_global, [v1i32, v1u32, v1i64, v1u64], 0, min_g>;
OclBuiltinImpl atom_min_g     = OclBuiltinImpl<atom_min_global, [v1i32, v1u32, v1i64, v1u64], 0, min_g>;
OclBuiltinImpl atom_min_vl    = OclBuiltinImpl<atom_min_vol_local, [v1i32, v1u32, v1i64, v1u64], 0, min_l>;
OclBuiltinImpl atom_min_l     = OclBuiltinImpl<atom_min_local, [v1i32, v1u32, v1i64, v1u64], 0, min_l>;

OclBuiltinImpl atomic_min_gf  = OclBuiltinImpl<atomic_min_global, [v1f32], 0, min_g_fp32>;
OclBuiltinImpl atomic_min_lf  = OclBuiltinImpl<atomic_min_local, [v1f32], 0, min_l_fp>;
// and
OclBuiltinImpl atomic_and_g   = OclBuiltinImpl<atomic_and_global, [v1i32, v1u32], 0, and_g>;
OclBuiltinImpl atomic_and_l   = OclBuiltinImpl<atomic_and_local, [v1i32, v1u32], 0, and_l>;
OclBuiltinImpl atom_and_vg    = OclBuiltinImpl<atom_and_vol_global, [v1i32, v1u32, v1i64, v1u64], 0, and_g>;
OclBuiltinImpl atom_and_g     = OclBuiltinImpl<atom_and_global, [v1i32, v1u32, v1i64, v1u64], 0, and_g>;
OclBuiltinImpl atom_and_vl    = OclBuiltinImpl<atom_and_vol_local, [v1i32, v1u32, v1i64, v1u64], 0, and_l>;
OclBuiltinImpl atom_and_l     = OclBuiltinImpl<atom_and_local, [v1i32, v1u32, v1i64, v1u64], 0, and_l>;
// or
OclBuiltinImpl atomic_or_g   = OclBuiltinImpl<atomic_or_global, [v1i32, v1u32], 0, or_g>;
OclBuiltinImpl atomic_or_l   = OclBuiltinImpl<atomic_or_local, [v1i32, v1u32], 0, or_l>;
OclBuiltinImpl atom_or_vg    = OclBuiltinImpl<atom_or_vol_global, [v1i32, v1u32, v1i64, v1u64], 0, or_g>;
OclBuiltinImpl atom_or_g     = OclBuiltinImpl<atom_or_global, [v1i32, v1u32, v1i64, v1u64], 0, or_g>;
OclBuiltinImpl atom_or_vl    = OclBuiltinImpl<atom_or_vol_local, [v1i32, v1u32, v1i64, v1u64], 0, or_l>;
OclBuiltinImpl atom_or_l     = OclBuiltinImpl<atom_or_local, [v1i32, v1u32, v1i64, v1u64], 0, or_l>;
// xor
OclBuiltinImpl atomic_xor_g   = OclBuiltinImpl<atomic_xor_global, [v1i32, v1u32], 0, xor_g>;
OclBuiltinImpl atomic_xor_l   = OclBuiltinImpl<atomic_xor_local, [v1i32, v1u32], 0, xor_l>;
OclBuiltinImpl atom_xor_vg    = OclBuiltinImpl<atom_xor_vol_global, [v1i32, v1u32, v1i64, v1u64], 0, xor_g>;
OclBuiltinImpl atom_xor_g     = OclBuiltinImpl<atom_xor_global, [v1i32, v1u32, v1i64, v1u64], 0, xor_g>;
OclBuiltinImpl atom_xor_vl    = OclBuiltinImpl<atom_xor_vol_local, [v1i32, v1u32, v1i64, v1u64], 0, xor_l>;
OclBuiltinImpl atom_xor_l     = OclBuiltinImpl<atom_xor_local, [v1i32, v1u32, v1i64, v1u64], 0, xor_l>;

// OpenCL 2.0 atomic builtins
// The implementation relies on "generic address space" optimization passes
// There are two of them: Static and Dynamic
// 1. The static pass is to replace calls to __generic builtin with the source pointer's address space
// 2. The dynamic pass is to replace all calls which static wasn't able to handle.
//    For CPU it will replace them to __global address space builtins (the most strict for atomics)
// None of __generic address space builtins must survive the passes.

// The functions not ending in _explicit have the same semantics as the corresponding _explicit function
// with memory_order_seq_cst for the memory_order argument and memory_scope_device for the
// memory_scope argument

//atomic_init
code atomic_init_code = [{
    *($Arg0atom2gentype)$Arg0VarName = $Arg1VarName;
  }];
OclBuiltinImpl atomic_init_l_impl = OclBuiltinImpl<atomic_init_l, atomic_init_l.Types, 0, atomic_init_code>;
OclBuiltinImpl atomic_init_g_impl = OclBuiltinImpl<atomic_init_g, atomic_init_g.Types, 0, atomic_init_code>;

// atomic_store
OclBuiltinImpl atomic_store_g_impl = OclBuiltinImpl<atomic_store_g, atomic_store_g.Types, 0, [{
    atomic_store_explicit($Arg0VarName, $Arg1VarName, memory_order_seq_cst, memory_scope_device);
  }]>;

OclBuiltinImpl atomic_store_expl_g_impl = OclBuiltinImpl<atomic_store_explicit_g, atomic_store_explicit_g.Types, 0, [{
    atomic_store_explicit($Arg0VarName, $Arg1VarName, $Arg2VarName, memory_scope_device);
  }]>;
OclBuiltinImpl atomic_store_expl_scope_g_impl = OclBuiltinImpl<atomic_store_explicit_scope_g, atomic_store_explicit_scope_g.Types, 0, [{
    if($Arg2VarName == memory_order_seq_cst) {
      (void) atomic_exchange_explicit($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName);

    } else if($Arg2VarName == memory_order_acq_rel ||
              $Arg2VarName == memory_order_release) {
      *($Arg0atom2gentype)$Arg0VarName = $Arg1VarName;
      atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, $Arg2VarName, $Arg3VarName);

    } else if($Arg2VarName == memory_order_acquire ||
              $Arg2VarName == memory_order_relaxed) {     // relaxed is here just for code minimization
      atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, $Arg2VarName, $Arg3VarName); // relaxed fence do nothing
      *($Arg0atom2gentype)$Arg0VarName = $Arg1VarName;
    }
  }]>;

OclBuiltinImpl atomic_store_l_impl = OclBuiltinImpl<atomic_store_l, atomic_store_l.Types, 0, [{
    atomic_store_explicit($Arg0VarName, $Arg1VarName, memory_order_seq_cst, memory_scope_device);
  }]>;
OclBuiltinImpl atomic_store_expl_l_impl = OclBuiltinImpl<atomic_store_explicit_l, atomic_store_explicit_l.Types, 0, [{
    atomic_store_explicit($Arg0VarName, $Arg1VarName, $Arg2VarName, memory_scope_device);
  }]>;
OclBuiltinImpl atomic_store_expl_scope_l_impl = OclBuiltinImpl<atomic_store_explicit_scope_l, atomic_store_explicit_scope_l.Types, 0, [{
    // this implementation relies on that a workgroup is executed by single phy. thread
    *(volatile local $Arg1Type *)$Arg0VarName = $Arg1VarName;
    // avoid warnings about unused variables
    (void) $Arg2VarName;
    (void) $Arg3VarName;
  }]>;

// atomic_load
OclBuiltinImpl atomic_load_g_impl = OclBuiltinImpl<atomic_load_g, atomic_load_g.Types, 0, [{
    return atomic_load_explicit($Arg0VarName, memory_order_seq_cst, memory_scope_device);
  }]>;
OclBuiltinImpl atomic_load_expl_g_impl = OclBuiltinImpl<atomic_load_explicit_g, atomic_load_explicit_g.Types, 0, [{
    return atomic_load_explicit($Arg0VarName, $Arg1VarName, memory_scope_device);
  }]>;

OclBuiltinImpl atomic_load_expl_scope_g_impl = OclBuiltinImpl<atomic_load_explicit_scope_g, atomic_load_explicit_scope_g.Types, 0, [{
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, $Arg1VarName, $Arg2VarName);
    return *($Arg0atom2gentype)$Arg0VarName;
  }]>;

OclBuiltinImpl atomic_load_l_impl = OclBuiltinImpl<atomic_load_l, atomic_load_l.Types, 0, [{
    return atomic_load_explicit($Arg0VarName, memory_order_seq_cst, memory_scope_device);
  }]>;
OclBuiltinImpl atomic_load_expl_l_impl = OclBuiltinImpl<atomic_load_explicit_l, atomic_load_explicit_l.Types, 0, [{
    return atomic_load_explicit($Arg0VarName, $Arg1VarName, memory_scope_device);
  }]>;
OclBuiltinImpl atomic_load_expl_scope_l_impl = OclBuiltinImpl<atomic_load_explicit_scope_l, atomic_load_explicit_scope_l.Types, 0, [{
    // this implementation relies on that a workgroup is executed by single phy. thread
    return *(volatile local $ReturnType *)$Arg0VarName;
    // avoid warnings about unused variables
    (void) $Arg1VarName;
    (void) $Arg2VarName;
  }]>;

// atomic_exchange
OclBuiltinImpl atomic_exchange_g_impl = OclBuiltinImpl<atomic_exchange_g, atomic_exchange_g.Types, 0, [{
    return atomic_exchange_explicit($Arg0VarName, $Arg1VarName, memory_order_seq_cst, memory_scope_device);
  }]>;
OclBuiltinImpl atomic_exchange_expl_g_impl = OclBuiltinImpl<atomic_exchange_explicit_g, atomic_exchange_explicit_g.Types, 0, [{
    return atomic_exchange_explicit($Arg0VarName, $Arg1VarName, $Arg2VarName, memory_scope_device);
  }]>;
OclBuiltinImpl atomic_exchange_expl_scope_g_impl = OclBuiltinImpl<atomic_exchange_explicit_scope_g, [v1i32, v1u32, v1f32], 0, [{
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, $Arg2VarName, $Arg3VarName);
    int old = __sync_lock_test_and_set((int*)$Arg0VarName, as_int($Arg1VarName));
    return as_$ReturnType(old);
  }]>;
OclBuiltinImpl atomic_exchange_expl_scope_g_impl_64 = OclBuiltinImpl<atomic_exchange_explicit_scope_g, [v1i64, v1u64, v1f64], 0, [{
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, $Arg2VarName, $Arg3VarName);
    long old = __sync_lock_test_and_set((long*)$Arg0VarName, as_long($Arg1VarName));
    return as_$ReturnType(old);
  }]>;

OclBuiltinImpl atomic_exchange_l_impl = OclBuiltinImpl<atomic_exchange_l, atomic_exchange_l.Types, 0, [{
    return atomic_exchange_explicit($Arg0VarName, $Arg1VarName, memory_order_seq_cst, memory_scope_device);
  }]>;
OclBuiltinImpl atomic_exchange_expl_l_impl = OclBuiltinImpl<atomic_exchange_explicit_l, atomic_exchange_explicit_l.Types, 0, [{
    return atomic_exchange_explicit($Arg0VarName, $Arg1VarName, $Arg2VarName, memory_scope_device);
  }]>;
OclBuiltinImpl atomic_exchange_expl_scope_l_impl = OclBuiltinImpl<atomic_exchange_explicit_scope_l, atomic_exchange_explicit_scope_l.Types, 0, [{
    // this implementation relies on that a workgroup is executed by single phy. thread
    $Arg1Type old = *($Arg0atom2gentype)$Arg0VarName;
    *($Arg0atom2gentype)$Arg0VarName = $Arg1VarName;
    (void) $Arg2VarName;
    (void) $Arg3VarName;
    return old;
  }]>;

// atomic_compare_exchange_<strong|weak> code
code atomic_compare_exchange_g_code = [{
    int cmp = as_int(*$Arg1VarName);
    // this turns into locked instruction which affects the memory in total ordering on Intel CPUs
    int old = __sync_val_compare_and_swap((int*)$Arg0VarName, cmp, as_int($Arg2VarName));

    if(old != cmp) {
      *$Arg1VarName = as_$Arg2Type(old);
      atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, $Arg3VarName, $Arg5VarName);
    } else {
      atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, $Arg4VarName, $Arg5VarName);
    }
    return old == cmp;
  }];
code atomic_compare_exchange_g_code_64 = [{
    long cmp = as_long(*$Arg1VarName);
    // this turns into locked instruction which affects the memory in total ordering on Intel CPUs
    long old = __sync_val_compare_and_swap((long*)$Arg0VarName, cmp, as_long($Arg2VarName));

    if(old != cmp) {
      *$Arg1VarName = as_$Arg2Type(old);
      atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, $Arg3VarName, $Arg5VarName);
    } else {
      atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, $Arg4VarName, $Arg5VarName);
    }
    return old == cmp;
  }];
code atomic_compare_exchange_l_code = [{
    (void) $Arg3VarName;
    (void) $Arg4VarName;
    (void) $Arg5VarName;

    // this implementation relies on that a workgroup is executed by single phy. thread
    $Arg2Type oldValue = *($Arg1Type)(void*)$Arg0VarName;
    if(oldValue == *$Arg1VarName) {
      *($Arg1Type)(void*)$Arg0VarName = $Arg2VarName;
      return true;
    }
    *$Arg1VarName = oldValue;
    return false;
  }];
code atomic_compare_exchange_disp1st_code = [{
    return atomic_compare_exchange_strong_explicit($Arg0VarName, $Arg1VarName, $Arg2VarName,
                                          memory_order_seq_cst, memory_order_relaxed, memory_scope_device);
  }];
code atomic_compare_exchange_disp2nd_code = [{
    return atomic_compare_exchange_strong_explicit($Arg0VarName, $Arg1VarName, $Arg2VarName,
                                          $Arg3VarName, $Arg4VarName, memory_scope_device);
  }];

// atomic_compare_exchange_strong
// global global
OclBuiltinImpl atomic_compare_exchange_strong_g_gp_impl =
  OclBuiltinImpl<atomic_compare_exchange_strong_g_gp, atomic_compare_exchange_strong_g_gp.Types, 0, atomic_compare_exchange_disp1st_code>;
OclBuiltinImpl atomic_compare_exchange_strong_expl_g_gp_impl =
  OclBuiltinImpl<atomic_compare_exchange_strong_explicit_g_gp, atomic_compare_exchange_strong_explicit_g_gp.Types, 0, atomic_compare_exchange_disp2nd_code>;
OclBuiltinImpl atomic_compare_exchange_strong_expl_scope_g_gp_impl =
  OclBuiltinImpl<atomic_compare_exchange_strong_explicit_scope_g_gp, [v1i32, v1u32, v1f32], 0, atomic_compare_exchange_g_code>;
OclBuiltinImpl atomic_compare_exchange_strong_expl_scope_g_gp_impl_64 =
  OclBuiltinImpl<atomic_compare_exchange_strong_explicit_scope_g_gp, [v1i64, v1u64, v1f64], 0, atomic_compare_exchange_g_code_64>;
// local global
OclBuiltinImpl atomic_compare_exchange_strong_l_gp_impl =
  OclBuiltinImpl<atomic_compare_exchange_strong_l_gp, atomic_compare_exchange_strong_l_gp.Types, 0, atomic_compare_exchange_disp1st_code>;
OclBuiltinImpl atomic_compare_exchange_strong_expl_l_gp_impl =
  OclBuiltinImpl<atomic_compare_exchange_strong_explicit_l_gp, atomic_compare_exchange_strong_explicit_l_gp.Types, 0, atomic_compare_exchange_disp2nd_code>;
OclBuiltinImpl atomic_compare_exchange_strong_expl_scope_l_gp_impl =
  OclBuiltinImpl<atomic_compare_exchange_strong_explicit_scope_l_gp, atomic_compare_exchange_strong_explicit_scope_l_gp.Types, 0, atomic_compare_exchange_l_code>;

// global local
OclBuiltinImpl atomic_compare_exchange_strong_g_lp_impl =
  OclBuiltinImpl<atomic_compare_exchange_strong_g_lp, atomic_compare_exchange_strong_g_lp.Types, 0, atomic_compare_exchange_disp1st_code>;
OclBuiltinImpl atomic_compare_exchange_strong_expl_g_lp_impl =
  OclBuiltinImpl<atomic_compare_exchange_strong_explicit_g_lp, atomic_compare_exchange_strong_explicit_g_lp.Types, 0, atomic_compare_exchange_disp2nd_code>;
OclBuiltinImpl atomic_compare_exchange_strong_expl_scope_g_lp_impl =
  OclBuiltinImpl<atomic_compare_exchange_strong_explicit_scope_g_lp, [v1i32, v1u32, v1f32], 0, atomic_compare_exchange_g_code>;
OclBuiltinImpl atomic_compare_exchange_strong_expl_scope_g_lp_impl_64 =
  OclBuiltinImpl<atomic_compare_exchange_strong_explicit_scope_g_lp, [v1i64, v1u64, v1f64], 0, atomic_compare_exchange_g_code_64>;
// local local
OclBuiltinImpl atomic_compare_exchange_strong_l_lp_impl =
  OclBuiltinImpl<atomic_compare_exchange_strong_l_lp, atomic_compare_exchange_strong_l_lp.Types, 0, atomic_compare_exchange_disp1st_code>;
OclBuiltinImpl atomic_compare_exchange_strong_expl_l_lp_impl =
  OclBuiltinImpl<atomic_compare_exchange_strong_explicit_l_lp, atomic_compare_exchange_strong_explicit_l_lp.Types, 0, atomic_compare_exchange_disp2nd_code>;
OclBuiltinImpl atomic_compare_exchange_strong_expl_scope_l_lp_impl =
  OclBuiltinImpl<atomic_compare_exchange_strong_explicit_scope_l_lp, atomic_compare_exchange_strong_explicit_scope_l_lp.Types, 0, atomic_compare_exchange_l_code>;

// global private
OclBuiltinImpl atomic_compare_exchange_strong_g_pp_impl =
  OclBuiltinImpl<atomic_compare_exchange_strong_g_pp, atomic_compare_exchange_strong_g_pp.Types, 0, atomic_compare_exchange_disp1st_code>;
OclBuiltinImpl atomic_compare_exchange_strong_expl_g_pp_impl =
  OclBuiltinImpl<atomic_compare_exchange_strong_explicit_g_pp, atomic_compare_exchange_strong_explicit_g_pp.Types, 0, atomic_compare_exchange_disp2nd_code>;
OclBuiltinImpl atomic_compare_exchange_strong_expl_scope_g_pp_impl =
  OclBuiltinImpl<atomic_compare_exchange_strong_explicit_scope_g_pp, [v1i32, v1u32, v1f32], 0, atomic_compare_exchange_g_code>;
OclBuiltinImpl atomic_compare_exchange_strong_expl_scope_g_pp_impl_64 =
  OclBuiltinImpl<atomic_compare_exchange_strong_explicit_scope_g_pp, [v1i64, v1u64, v1f64], 0, atomic_compare_exchange_g_code_64>;
// local private
OclBuiltinImpl atomic_compare_exchange_strong_l_pp_impl =
  OclBuiltinImpl<atomic_compare_exchange_strong_l_pp, atomic_compare_exchange_strong_l_pp.Types, 0, atomic_compare_exchange_disp1st_code>;
OclBuiltinImpl atomic_compare_exchange_strong_expl_l_pp_impl =
  OclBuiltinImpl<atomic_compare_exchange_strong_explicit_l_pp, atomic_compare_exchange_strong_explicit_l_pp.Types, 0, atomic_compare_exchange_disp2nd_code>;
OclBuiltinImpl atomic_compare_exchange_strong_expl_scope_l_pp_impl =
  OclBuiltinImpl<atomic_compare_exchange_strong_explicit_scope_l_pp, atomic_compare_exchange_strong_explicit_scope_l_pp.Types, 0, atomic_compare_exchange_l_code>;

// atomic_compare_exchange_weak
// global global
OclBuiltinImpl atomic_compare_exchange_weak_g_gp_impl =
  OclBuiltinImpl<atomic_compare_exchange_weak_g_gp, atomic_compare_exchange_weak_g_gp.Types, 0, atomic_compare_exchange_disp1st_code>;
OclBuiltinImpl atomic_compare_exchange_weak_expl_g_gp_impl =
  OclBuiltinImpl<atomic_compare_exchange_weak_explicit_g_gp, atomic_compare_exchange_weak_explicit_g_gp.Types, 0, atomic_compare_exchange_disp2nd_code>;
OclBuiltinImpl atomic_compare_exchange_weak_expl_scope_g_gp_impl =
  OclBuiltinImpl<atomic_compare_exchange_weak_explicit_scope_g_gp, [v1i32, v1u32, v1f32], 0, atomic_compare_exchange_g_code>;
OclBuiltinImpl atomic_compare_exchange_weak_expl_scope_g_gp_impl_64 =
  OclBuiltinImpl<atomic_compare_exchange_weak_explicit_scope_g_gp, [v1i64, v1u64, v1f64], 0, atomic_compare_exchange_g_code_64>;
// local global
OclBuiltinImpl atomic_compare_exchange_weak_l_gp_impl =
  OclBuiltinImpl<atomic_compare_exchange_weak_l_gp, atomic_compare_exchange_weak_l_gp.Types, 0, atomic_compare_exchange_disp1st_code>;
OclBuiltinImpl atomic_compare_exchange_weak_expl_l_gp_impl =
  OclBuiltinImpl<atomic_compare_exchange_weak_explicit_l_gp, atomic_compare_exchange_weak_explicit_l_gp.Types, 0, atomic_compare_exchange_disp2nd_code>;
OclBuiltinImpl atomic_compare_exchange_weak_expl_scope_l_gp_impl =
  OclBuiltinImpl<atomic_compare_exchange_weak_explicit_scope_l_gp, atomic_compare_exchange_weak_explicit_scope_l_gp.Types, 0, atomic_compare_exchange_l_code>;

// global local
OclBuiltinImpl atomic_compare_exchange_weak_g_lp_impl =
  OclBuiltinImpl<atomic_compare_exchange_weak_g_lp, atomic_compare_exchange_weak_g_lp.Types, 0, atomic_compare_exchange_disp1st_code>;
OclBuiltinImpl atomic_compare_exchange_weak_expl_g_lp_impl =
  OclBuiltinImpl<atomic_compare_exchange_weak_explicit_g_lp, atomic_compare_exchange_weak_explicit_g_lp.Types, 0, atomic_compare_exchange_disp2nd_code>;
OclBuiltinImpl atomic_compare_exchange_weak_expl_scope_g_lp_impl =
  OclBuiltinImpl<atomic_compare_exchange_weak_explicit_scope_g_lp, [v1i32, v1u32, v1f32], 0, atomic_compare_exchange_g_code>;
OclBuiltinImpl atomic_compare_exchange_weak_expl_scope_g_lp_impl_64 =
  OclBuiltinImpl<atomic_compare_exchange_weak_explicit_scope_g_lp, [v1i64, v1u64, v1f64], 0, atomic_compare_exchange_g_code_64>;
// local local
OclBuiltinImpl atomic_compare_exchange_weak_l_lp_impl =
  OclBuiltinImpl<atomic_compare_exchange_weak_l_lp, atomic_compare_exchange_weak_l_lp.Types, 0, atomic_compare_exchange_disp1st_code>;
OclBuiltinImpl atomic_compare_exchange_weak_expl_l_lp_impl =
  OclBuiltinImpl<atomic_compare_exchange_weak_explicit_l_lp, atomic_compare_exchange_weak_explicit_l_lp.Types, 0, atomic_compare_exchange_disp2nd_code>;
OclBuiltinImpl atomic_compare_exchange_weak_expl_scope_l_lp_impl =
  OclBuiltinImpl<atomic_compare_exchange_weak_explicit_scope_l_lp, atomic_compare_exchange_weak_explicit_scope_l_lp.Types, 0, atomic_compare_exchange_l_code>;

// global private
OclBuiltinImpl atomic_compare_exchange_weak_g_pp_impl =
  OclBuiltinImpl<atomic_compare_exchange_weak_g_pp, atomic_compare_exchange_weak_g_pp.Types, 0, atomic_compare_exchange_disp1st_code>;
OclBuiltinImpl atomic_compare_exchange_weak_expl_g_pp_impl =
  OclBuiltinImpl<atomic_compare_exchange_weak_explicit_g_pp, atomic_compare_exchange_weak_explicit_g_pp.Types, 0, atomic_compare_exchange_disp2nd_code>;
OclBuiltinImpl atomic_compare_exchange_weak_expl_scope_g_pp_impl =
  OclBuiltinImpl<atomic_compare_exchange_weak_explicit_scope_g_pp, [v1i32, v1u32, v1f32], 0, atomic_compare_exchange_g_code>;
OclBuiltinImpl atomic_compare_exchange_weak_expl_scope_g_pp_impl_64 =
  OclBuiltinImpl<atomic_compare_exchange_weak_explicit_scope_g_pp, [v1i64, v1u64, v1f64], 0, atomic_compare_exchange_g_code_64>;
// local private
OclBuiltinImpl atomic_compare_exchange_weak_l_pp_impl =
  OclBuiltinImpl<atomic_compare_exchange_weak_l_pp, atomic_compare_exchange_weak_l_pp.Types, 0, atomic_compare_exchange_disp1st_code>;
OclBuiltinImpl atomic_compare_exchange_weak_expl_l_pp_impl =
  OclBuiltinImpl<atomic_compare_exchange_weak_explicit_l_pp, atomic_compare_exchange_weak_explicit_l_pp.Types, 0, atomic_compare_exchange_disp2nd_code>;
OclBuiltinImpl atomic_compare_exchange_weak_expl_scope_l_pp_impl =
  OclBuiltinImpl<atomic_compare_exchange_weak_explicit_scope_l_pp, atomic_compare_exchange_weak_explicit_scope_l_pp.Types, 0, atomic_compare_exchange_l_code>;

// auxiliary code
code ocl20_binop_code = [{
    return $Arg0VarName $BINOP $Arg1VarName;
  }];
code ocl20_relop_code = [{
    return $Arg0VarName $RELOP $Arg1VarName ? $Arg0VarName : $Arg1VarName;
  }];

// auxiliary functions
OclBuiltinImpl ocl20_add_impl = OclBuiltinImpl<ocl20_add, ocl20_add.Types, 0, ocl20_binop_code, [["$BINOP","+"]]>;
OclBuiltinImpl ocl20_sub_impl = OclBuiltinImpl<ocl20_sub, ocl20_sub.Types, 0, ocl20_binop_code, [["$BINOP","-"]]>;
OclBuiltinImpl ocl20_or_impl  = OclBuiltinImpl<ocl20_or,  ocl20_or.Types,  0, ocl20_binop_code, [["$BINOP","|"]]>;
OclBuiltinImpl ocl20_xor_impl = OclBuiltinImpl<ocl20_xor, ocl20_xor.Types, 0, ocl20_binop_code, [["$BINOP","^"]]>;
OclBuiltinImpl ocl20_and_impl = OclBuiltinImpl<ocl20_and, ocl20_and.Types, 0, ocl20_binop_code, [["$BINOP","&"]]>;
OclBuiltinImpl ocl20_min_impl = OclBuiltinImpl<ocl20_min, ocl20_min.Types, 0, ocl20_relop_code, [["$RELOP","<="]]>;
OclBuiltinImpl ocl20_max_impl = OclBuiltinImpl<ocl20_max, ocl20_max.Types, 0, ocl20_relop_code, [["$RELOP",">="]]>;
OclBuiltinImpl ocl20_add_uintptr_impl = OclBuiltinImpl<ocl20_add_uintptr, ocl20_add_uintptr.Types, 0, ocl20_binop_code, [["$BINOP","+"]]>;
OclBuiltinImpl ocl20_sub_uintptr_impl = OclBuiltinImpl<ocl20_sub_uintptr, ocl20_sub_uintptr.Types, 0, ocl20_binop_code, [["$BINOP","-"]]>;
OclBuiltinImpl ocl20_or_uintptr_impl  = OclBuiltinImpl<ocl20_or_uintptr,  ocl20_or_uintptr.Types,  0, ocl20_binop_code, [["$BINOP","|"]]>;
OclBuiltinImpl ocl20_xor_uintptr_impl = OclBuiltinImpl<ocl20_xor_uintptr, ocl20_xor_uintptr.Types, 0, ocl20_binop_code, [["$BINOP","^"]]>;
OclBuiltinImpl ocl20_and_uintptr_impl = OclBuiltinImpl<ocl20_and_uintptr, ocl20_and_uintptr.Types, 0, ocl20_binop_code, [["$BINOP","&"]]>;
OclBuiltinImpl ocl20_min_uintptr_impl = OclBuiltinImpl<ocl20_min_uintptr, ocl20_min_uintptr.Types, 0, ocl20_relop_code, [["$RELOP","<="]]>;
OclBuiltinImpl ocl20_max_uintptr_impl = OclBuiltinImpl<ocl20_max_uintptr, ocl20_max_uintptr.Types, 0, ocl20_relop_code, [["$RELOP",">="]]>;
OclBuiltinImpl ocl20_or_intptr_impl  = OclBuiltinImpl<ocl20_or_intptr,  ocl20_or_intptr.Types,  0, ocl20_binop_code, [["$BINOP","|"]]>;
OclBuiltinImpl ocl20_xor_intptr_impl = OclBuiltinImpl<ocl20_xor_intptr, ocl20_xor_intptr.Types, 0, ocl20_binop_code, [["$BINOP","^"]]>;
OclBuiltinImpl ocl20_and_intptr_impl = OclBuiltinImpl<ocl20_and_intptr, ocl20_and_intptr.Types, 0, ocl20_binop_code, [["$BINOP","&"]]>;
OclBuiltinImpl ocl20_min_intptr_impl = OclBuiltinImpl<ocl20_min_intptr, ocl20_min_intptr.Types, 0, ocl20_relop_code, [["$RELOP","<="]]>;
OclBuiltinImpl ocl20_max_intptr_impl = OclBuiltinImpl<ocl20_max_intptr, ocl20_max_intptr.Types, 0, ocl20_relop_code, [["$RELOP",">="]]>;

OclBuiltinImpl as_uintptr_t_impl = OclBuiltinImpl<as_uintptr_t, as_uintptr_t.Types, 0,
  [{
    return (uintptr_t)as_uint($Arg0VarName);
  }]>;
OclBuiltinImpl as_uintptr_t_64_impl = OclBuiltinImpl<as_uintptr_t_64, as_uintptr_t_64.Types, 0,
  [{
    return (uintptr_t)as_ulong($Arg0VarName);
  }]>;
OclBuiltinImpl as_intptr_t_impl = OclBuiltinImpl<as_intptr_t, as_intptr_t.Types, 0,
  [{
    return (intptr_t)as_int($Arg0VarName);
  }]>;
OclBuiltinImpl as_intptr_t_64_impl = OclBuiltinImpl<as_intptr_t_64, as_intptr_t_64.Types, 0,
  [{
    return (intptr_t)as_long($Arg0VarName);
  }]>;

// atomic_fetch_key code
code atomic_fetch_key_code = [{
    return atomic_fetch_$KEY_explicit($Arg0VarName, $Arg1VarName, memory_order_seq_cst, memory_scope_device);
  }];
code atomic_fetch_key_explicit_code = [{
    return atomic_fetch_$KEY_explicit($Arg0VarName, $Arg1VarName, $Arg2VarName, memory_scope_device);
  }];
code atomic_fetch_binop_explicit_scope_g_code = [{
    atomic_work_item_fence(CLK_GLOBAL_MEM_FENCE, $Arg2VarName, $Arg3VarName);
    // this turns into locked instruction which affects the memory in total ordering on Intel CPUs
    return as_$ReturnType(__sync_fetch_and_$KEY(($Arg1Type*)$Arg0VarName, $Arg1VarName));
  }];
code atomic_fetch_binop_explicit_scope_l_code = [{
    (void) $Arg2VarName;
    (void) $Arg3VarName;
    // this implementation relies on that a workgroup is executed by single phy. thread
    $Arg1Type old = *($Arg0atom2gentype)$Arg0VarName;
    *($Arg0atom2gentype)$Arg0VarName = __ocl20_$KEY(old, $Arg1VarName);
    return as_$ReturnType(old);
  }];
code atomic_fetch_relop_explicit_scope_g_code = [{
    // any atomic builtin imply memory fence with the specified memory order and scope
    // so pass $Arg2VarName and $Arg3VarName to atomic_load_explicit
    private $ReturnType old = atomic_load_explicit($Arg0VarName, $Arg2VarName, $Arg3VarName);
    do {
      if (old == __ocl20_$KEY(old, $Arg1VarName)) break;
    } while(!atomic_compare_exchange_strong_explicit($Arg0VarName, &old, $Arg1VarName,
                                                     memory_order_relaxed, memory_order_relaxed, $Arg3VarName));
    // 1. The old value is result of "$KEY" so nothing has to be written, break and return the old value.
    // 2. Otherwise if atomic_compare_exchange_strong fails then the old value has been changed, go to the 1st step.
    // 3. Otherwise the old value wasn't result of "$KEY" and atomic_compare_exchange_strong succeeded.
    //    So the new value has been written, return the old value
    return old;
  }];
code atomic_fetch_relop_explicit_scope_l_code = [{
    // this implementation relies on that a workgroup is executed by single phy. thread
    $Arg1Type old = *($Arg0atom2gentype)$Arg0VarName;
    *($Arg0atom2gentype)$Arg0VarName = __ocl20_$KEY(old, $Arg1VarName);
    return as_$ReturnType(old);
  }];

// atomic_fetch_key builtins
OclBuiltinImpl atomic_fetch_add_g_impl = OclBuiltinImpl<atomic_fetch_add_g, atomic_fetch_add_g.Types, 0, atomic_fetch_key_code, [["$KEY","add"]]>;
OclBuiltinImpl atomic_fetch_add_explicit_g_impl = OclBuiltinImpl<atomic_fetch_add_explicit_g, atomic_fetch_add_explicit_g.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","add"]]>;
OclBuiltinImpl atomic_fetch_add_explicit_scope_g_impl = OclBuiltinImpl<atomic_fetch_add_explicit_scope_g, atomic_fetch_add_explicit_scope_g.Types, 0, atomic_fetch_binop_explicit_scope_g_code, [["$KEY","add"]]>;

OclBuiltinImpl atomic_fetch_add_l_impl = OclBuiltinImpl<atomic_fetch_add_l, atomic_fetch_add_l.Types, 0, atomic_fetch_key_code, [["$KEY","add"]]>;
OclBuiltinImpl atomic_fetch_add_explicit_l_impl = OclBuiltinImpl<atomic_fetch_add_explicit_l, atomic_fetch_add_explicit_l.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","add"]]>;
OclBuiltinImpl atomic_fetch_add_explicit_scope_l_impl = OclBuiltinImpl<atomic_fetch_add_explicit_scope_l, atomic_fetch_add_explicit_scope_l.Types, 0, atomic_fetch_binop_explicit_scope_l_code, [["$KEY","add"]]>;

OclBuiltinImpl atomic_fetch_sub_g_impl = OclBuiltinImpl<atomic_fetch_sub_g, atomic_fetch_sub_g.Types, 0, atomic_fetch_key_code, [["$KEY","sub"]]>;
OclBuiltinImpl atomic_fetch_sub_explicit_g_impl = OclBuiltinImpl<atomic_fetch_sub_explicit_g, atomic_fetch_sub_explicit_g.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","sub"]]>;
OclBuiltinImpl atomic_fetch_sub_explicit_scope_g_impl = OclBuiltinImpl<atomic_fetch_sub_explicit_scope_g, atomic_fetch_sub_explicit_scope_g.Types, 0, atomic_fetch_binop_explicit_scope_g_code, [["$KEY","sub"]]>;

OclBuiltinImpl atomic_fetch_sub_l_impl = OclBuiltinImpl<atomic_fetch_sub_l, atomic_fetch_sub_l.Types, 0, atomic_fetch_key_code, [["$KEY","sub"]]>;
OclBuiltinImpl atomic_fetch_sub_explicit_l_impl = OclBuiltinImpl<atomic_fetch_sub_explicit_l, atomic_fetch_sub_explicit_l.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","sub"]]>;
OclBuiltinImpl atomic_fetch_sub_explicit_scope_l_impl = OclBuiltinImpl<atomic_fetch_sub_explicit_scope_l, atomic_fetch_sub_explicit_scope_l.Types, 0, atomic_fetch_binop_explicit_scope_l_code, [["$KEY","sub"]]>;

OclBuiltinImpl atomic_fetch_add_uintptr_g_impl = OclBuiltinImpl<atomic_fetch_add_uintptr_g, atomic_fetch_add_uintptr_g.Types, 0, atomic_fetch_key_code, [["$KEY","add"]]>;
OclBuiltinImpl atomic_fetch_add_explicit_uintptr_g_impl = OclBuiltinImpl<atomic_fetch_add_explicit_uintptr_g, atomic_fetch_add_explicit_uintptr_g.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","add"]]>;
OclBuiltinImpl atomic_fetch_add_explicit_scope_uintptr_g_impl = OclBuiltinImpl<atomic_fetch_add_explicit_scope_uintptr_g, atomic_fetch_add_explicit_scope_uintptr_g.Types, 0, atomic_fetch_binop_explicit_scope_g_code, [["$KEY","add"]]>;

OclBuiltinImpl atomic_fetch_add_uintptr_l_impl = OclBuiltinImpl<atomic_fetch_add_uintptr_l, atomic_fetch_add_uintptr_l.Types, 0, atomic_fetch_key_code, [["$KEY","add"]]>;
OclBuiltinImpl atomic_fetch_add_explicit_uintptr_l_impl = OclBuiltinImpl<atomic_fetch_add_explicit_uintptr_l, atomic_fetch_add_explicit_uintptr_l.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","add"]]>;
OclBuiltinImpl atomic_fetch_add_explicit_scope_uintptr_l_impl = OclBuiltinImpl<atomic_fetch_add_explicit_scope_uintptr_l, atomic_fetch_add_explicit_scope_uintptr_l.Types, 0, atomic_fetch_binop_explicit_scope_l_code, [["$KEY","add"]]>;

OclBuiltinImpl atomic_fetch_sub_uintptr_g_impl = OclBuiltinImpl<atomic_fetch_sub_uintptr_g, atomic_fetch_sub_uintptr_g.Types, 0, atomic_fetch_key_code, [["$KEY","sub"]]>;
OclBuiltinImpl atomic_fetch_sub_explicit_uintptr_g_impl = OclBuiltinImpl<atomic_fetch_sub_explicit_uintptr_g, atomic_fetch_sub_explicit_uintptr_g.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","sub"]]>;
OclBuiltinImpl atomic_fetch_sub_explicit_scope_uintptr_g_impl = OclBuiltinImpl<atomic_fetch_sub_explicit_scope_uintptr_g, atomic_fetch_sub_explicit_scope_uintptr_g.Types, 0, atomic_fetch_binop_explicit_scope_g_code, [["$KEY","sub"]]>;

OclBuiltinImpl atomic_fetch_sub_uintptr_l_impl = OclBuiltinImpl<atomic_fetch_sub_uintptr_l, atomic_fetch_sub_uintptr_l.Types, 0, atomic_fetch_key_code, [["$KEY","sub"]]>;
OclBuiltinImpl atomic_fetch_sub_explicit_uintptr_l_impl = OclBuiltinImpl<atomic_fetch_sub_explicit_uintptr_l, atomic_fetch_sub_explicit_uintptr_l.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","sub"]]>;
OclBuiltinImpl atomic_fetch_sub_explicit_scope_uintptr_l_impl = OclBuiltinImpl<atomic_fetch_sub_explicit_scope_uintptr_l, atomic_fetch_sub_explicit_scope_uintptr_l.Types, 0, atomic_fetch_binop_explicit_scope_l_code, [["$KEY","sub"]]>;

OclBuiltinImpl atomic_fetch_or_g_impl = OclBuiltinImpl<atomic_fetch_or_g, atomic_fetch_or_g.Types, 0, atomic_fetch_key_code, [["$KEY","or"]]>;
OclBuiltinImpl atomic_fetch_or_explicit_g_impl = OclBuiltinImpl<atomic_fetch_or_explicit_g, atomic_fetch_or_explicit_g.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","or"]]>;
OclBuiltinImpl atomic_fetch_or_explicit_scope_g_impl = OclBuiltinImpl<atomic_fetch_or_explicit_scope_g, atomic_fetch_or_explicit_scope_g.Types, 0, atomic_fetch_binop_explicit_scope_g_code, [["$KEY","or"]]>;

OclBuiltinImpl atomic_fetch_or_l_impl = OclBuiltinImpl<atomic_fetch_or_l, atomic_fetch_or_l.Types, 0, atomic_fetch_key_code, [["$KEY","or"]]>;
OclBuiltinImpl atomic_fetch_or_explicit_l_impl = OclBuiltinImpl<atomic_fetch_or_explicit_l, atomic_fetch_or_explicit_l.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","or"]]>;
OclBuiltinImpl atomic_fetch_or_explicit_scope_l_impl = OclBuiltinImpl<atomic_fetch_or_explicit_scope_l, atomic_fetch_or_explicit_scope_l.Types, 0, atomic_fetch_binop_explicit_scope_l_code, [["$KEY","or"]]>;

OclBuiltinImpl atomic_fetch_or_uintptr_g_impl = OclBuiltinImpl<atomic_fetch_or_uintptr_g, atomic_fetch_or_uintptr_g.Types, 0, atomic_fetch_key_code, [["$KEY","or"]]>;
OclBuiltinImpl atomic_fetch_or_explicit_uintptr_g_impl = OclBuiltinImpl<atomic_fetch_or_explicit_uintptr_g, atomic_fetch_or_explicit_uintptr_g.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","or"]]>;
OclBuiltinImpl atomic_fetch_or_explicit_scope_uintptr_g_impl = OclBuiltinImpl<atomic_fetch_or_explicit_scope_uintptr_g, atomic_fetch_or_explicit_scope_uintptr_g.Types, 0, atomic_fetch_binop_explicit_scope_g_code, [["$KEY","or"]]>;

OclBuiltinImpl atomic_fetch_or_uintptr_l_impl = OclBuiltinImpl<atomic_fetch_or_uintptr_l, atomic_fetch_or_uintptr_l.Types, 0, atomic_fetch_key_code, [["$KEY","or"]]>;
OclBuiltinImpl atomic_fetch_or_explicit_uintptr_l_impl = OclBuiltinImpl<atomic_fetch_or_explicit_uintptr_l, atomic_fetch_or_explicit_uintptr_l.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","or"]]>;
OclBuiltinImpl atomic_fetch_or_explicit_scope_uintptr_l_impl = OclBuiltinImpl<atomic_fetch_or_explicit_scope_uintptr_l, atomic_fetch_or_explicit_scope_uintptr_l.Types, 0, atomic_fetch_binop_explicit_scope_l_code, [["$KEY","or"]]>;

OclBuiltinImpl atomic_fetch_or_intptr_g_impl = OclBuiltinImpl<atomic_fetch_or_intptr_g, atomic_fetch_or_intptr_g.Types, 0, atomic_fetch_key_code, [["$KEY","or"]]>;
OclBuiltinImpl atomic_fetch_or_explicit_intptr_g_impl = OclBuiltinImpl<atomic_fetch_or_explicit_intptr_g, atomic_fetch_or_explicit_intptr_g.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","or"]]>;
OclBuiltinImpl atomic_fetch_or_explicit_scope_intptr_g_impl = OclBuiltinImpl<atomic_fetch_or_explicit_scope_intptr_g, atomic_fetch_or_explicit_scope_intptr_g.Types, 0, atomic_fetch_binop_explicit_scope_g_code, [["$KEY","or"]]>;

OclBuiltinImpl atomic_fetch_or_intptr_l_impl = OclBuiltinImpl<atomic_fetch_or_intptr_l, atomic_fetch_or_intptr_l.Types, 0, atomic_fetch_key_code, [["$KEY","or"]]>;
OclBuiltinImpl atomic_fetch_or_explicit_intptr_l_impl = OclBuiltinImpl<atomic_fetch_or_explicit_intptr_l, atomic_fetch_or_explicit_intptr_l.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","or"]]>;
OclBuiltinImpl atomic_fetch_or_explicit_scope_intptr_l_impl = OclBuiltinImpl<atomic_fetch_or_explicit_scope_intptr_l, atomic_fetch_or_explicit_scope_intptr_l.Types, 0, atomic_fetch_binop_explicit_scope_l_code, [["$KEY","or"]]>;

OclBuiltinImpl atomic_fetch_xor_g_impl = OclBuiltinImpl<atomic_fetch_xor_g, atomic_fetch_xor_g.Types, 0, atomic_fetch_key_code, [["$KEY","xor"]]>;
OclBuiltinImpl atomic_fetch_xor_explicit_g_impl = OclBuiltinImpl<atomic_fetch_xor_explicit_g, atomic_fetch_xor_explicit_g.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","xor"]]>;
OclBuiltinImpl atomic_fetch_xor_explicit_scope_g_impl = OclBuiltinImpl<atomic_fetch_xor_explicit_scope_g, atomic_fetch_xor_explicit_scope_g.Types, 0, atomic_fetch_binop_explicit_scope_g_code, [["$KEY","xor"]]>;

OclBuiltinImpl atomic_fetch_xor_l_impl = OclBuiltinImpl<atomic_fetch_xor_l, atomic_fetch_xor_l.Types, 0, atomic_fetch_key_code, [["$KEY","xor"]]>;
OclBuiltinImpl atomic_fetch_xor_explicit_l_impl = OclBuiltinImpl<atomic_fetch_xor_explicit_l, atomic_fetch_xor_explicit_l.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","xor"]]>;
OclBuiltinImpl atomic_fetch_xor_explicit_scope_l_impl = OclBuiltinImpl<atomic_fetch_xor_explicit_scope_l, atomic_fetch_xor_explicit_scope_l.Types, 0, atomic_fetch_binop_explicit_scope_l_code, [["$KEY","xor"]]>;

OclBuiltinImpl atomic_fetch_xor_uintptr_g_impl = OclBuiltinImpl<atomic_fetch_xor_uintptr_g, atomic_fetch_xor_uintptr_g.Types, 0, atomic_fetch_key_code, [["$KEY","xor"]]>;
OclBuiltinImpl atomic_fetch_xor_explicit_uintptr_g_impl = OclBuiltinImpl<atomic_fetch_xor_explicit_uintptr_g, atomic_fetch_xor_explicit_uintptr_g.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","xor"]]>;
OclBuiltinImpl atomic_fetch_xor_explicit_scope_uintptr_g_impl = OclBuiltinImpl<atomic_fetch_xor_explicit_scope_uintptr_g, atomic_fetch_xor_explicit_scope_uintptr_g.Types, 0, atomic_fetch_binop_explicit_scope_g_code, [["$KEY","xor"]]>;

OclBuiltinImpl atomic_fetch_xor_uintptr_l_impl = OclBuiltinImpl<atomic_fetch_xor_uintptr_l, atomic_fetch_xor_uintptr_l.Types, 0, atomic_fetch_key_code, [["$KEY","xor"]]>;
OclBuiltinImpl atomic_fetch_xor_explicit_uintptr_l_impl = OclBuiltinImpl<atomic_fetch_xor_explicit_uintptr_l, atomic_fetch_xor_explicit_uintptr_l.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","xor"]]>;
OclBuiltinImpl atomic_fetch_xor_explicit_scope_uintptr_l_impl = OclBuiltinImpl<atomic_fetch_xor_explicit_scope_uintptr_l, atomic_fetch_xor_explicit_scope_uintptr_l.Types, 0, atomic_fetch_binop_explicit_scope_l_code, [["$KEY","xor"]]>;

OclBuiltinImpl atomic_fetch_xor_intptr_g_impl = OclBuiltinImpl<atomic_fetch_xor_intptr_g, atomic_fetch_xor_intptr_g.Types, 0, atomic_fetch_key_code, [["$KEY","xor"]]>;
OclBuiltinImpl atomic_fetch_xor_explicit_intptr_g_impl = OclBuiltinImpl<atomic_fetch_xor_explicit_intptr_g, atomic_fetch_xor_explicit_intptr_g.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","xor"]]>;
OclBuiltinImpl atomic_fetch_xor_explicit_scope_intptr_g_impl = OclBuiltinImpl<atomic_fetch_xor_explicit_scope_intptr_g, atomic_fetch_xor_explicit_scope_intptr_g.Types, 0, atomic_fetch_binop_explicit_scope_g_code, [["$KEY","xor"]]>;

OclBuiltinImpl atomic_fetch_xor_intptr_l_impl = OclBuiltinImpl<atomic_fetch_xor_intptr_l, atomic_fetch_xor_intptr_l.Types, 0, atomic_fetch_key_code, [["$KEY","xor"]]>;
OclBuiltinImpl atomic_fetch_xor_explicit_intptr_l_impl = OclBuiltinImpl<atomic_fetch_xor_explicit_intptr_l, atomic_fetch_xor_explicit_intptr_l.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","xor"]]>;
OclBuiltinImpl atomic_fetch_xor_explicit_scope_intptr_l_impl = OclBuiltinImpl<atomic_fetch_xor_explicit_scope_intptr_l, atomic_fetch_xor_explicit_scope_intptr_l.Types, 0, atomic_fetch_binop_explicit_scope_l_code, [["$KEY","xor"]]>;

OclBuiltinImpl atomic_fetch_and_g_impl = OclBuiltinImpl<atomic_fetch_and_g, atomic_fetch_and_g.Types, 0, atomic_fetch_key_code, [["$KEY","and"]]>;
OclBuiltinImpl atomic_fetch_and_explicit_g_impl = OclBuiltinImpl<atomic_fetch_and_explicit_g, atomic_fetch_and_explicit_g.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","and"]]>;
OclBuiltinImpl atomic_fetch_and_explicit_scope_g_impl = OclBuiltinImpl<atomic_fetch_and_explicit_scope_g, atomic_fetch_and_explicit_scope_g.Types, 0, atomic_fetch_binop_explicit_scope_g_code, [["$KEY","and"]]>;

OclBuiltinImpl atomic_fetch_and_l_impl = OclBuiltinImpl<atomic_fetch_and_l, atomic_fetch_and_l.Types, 0, atomic_fetch_key_code, [["$KEY","and"]]>;
OclBuiltinImpl atomic_fetch_and_explicit_l_impl = OclBuiltinImpl<atomic_fetch_and_explicit_l, atomic_fetch_and_explicit_l.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","and"]]>;
OclBuiltinImpl atomic_fetch_and_explicit_scope_l_impl = OclBuiltinImpl<atomic_fetch_and_explicit_scope_l, atomic_fetch_and_explicit_scope_l.Types, 0, atomic_fetch_binop_explicit_scope_l_code, [["$KEY","and"]]>;

OclBuiltinImpl atomic_fetch_and_uintptr_g_impl = OclBuiltinImpl<atomic_fetch_and_uintptr_g, atomic_fetch_and_uintptr_g.Types, 0, atomic_fetch_key_code, [["$KEY","and"]]>;
OclBuiltinImpl atomic_fetch_and_explicit_uintptr_g_impl = OclBuiltinImpl<atomic_fetch_and_explicit_uintptr_g, atomic_fetch_and_explicit_uintptr_g.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","and"]]>;
OclBuiltinImpl atomic_fetch_and_explicit_scope_uintptr_g_impl = OclBuiltinImpl<atomic_fetch_and_explicit_scope_uintptr_g, atomic_fetch_and_explicit_scope_uintptr_g.Types, 0, atomic_fetch_binop_explicit_scope_g_code, [["$KEY","and"]]>;

OclBuiltinImpl atomic_fetch_and_uintptr_l_impl = OclBuiltinImpl<atomic_fetch_and_uintptr_l, atomic_fetch_and_uintptr_l.Types, 0, atomic_fetch_key_code, [["$KEY","and"]]>;
OclBuiltinImpl atomic_fetch_and_explicit_uintptr_l_impl = OclBuiltinImpl<atomic_fetch_and_explicit_uintptr_l, atomic_fetch_and_explicit_uintptr_l.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","and"]]>;
OclBuiltinImpl atomic_fetch_and_explicit_scope_uintptr_l_impl = OclBuiltinImpl<atomic_fetch_and_explicit_scope_uintptr_l, atomic_fetch_and_explicit_scope_uintptr_l.Types, 0, atomic_fetch_binop_explicit_scope_l_code, [["$KEY","and"]]>;

OclBuiltinImpl atomic_fetch_and_intptr_g_impl = OclBuiltinImpl<atomic_fetch_and_intptr_g, atomic_fetch_and_intptr_g.Types, 0, atomic_fetch_key_code, [["$KEY","and"]]>;
OclBuiltinImpl atomic_fetch_and_explicit_intptr_g_impl = OclBuiltinImpl<atomic_fetch_and_explicit_intptr_g, atomic_fetch_and_explicit_intptr_g.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","and"]]>;
OclBuiltinImpl atomic_fetch_and_explicit_scope_intptr_g_impl = OclBuiltinImpl<atomic_fetch_and_explicit_scope_intptr_g, atomic_fetch_and_explicit_scope_intptr_g.Types, 0, atomic_fetch_binop_explicit_scope_g_code, [["$KEY","and"]]>;

OclBuiltinImpl atomic_fetch_and_intptr_l_impl = OclBuiltinImpl<atomic_fetch_and_intptr_l, atomic_fetch_and_intptr_l.Types, 0, atomic_fetch_key_code, [["$KEY","and"]]>;
OclBuiltinImpl atomic_fetch_and_explicit_intptr_l_impl = OclBuiltinImpl<atomic_fetch_and_explicit_intptr_l, atomic_fetch_and_explicit_intptr_l.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","and"]]>;
OclBuiltinImpl atomic_fetch_and_explicit_scopeintptr_l_impl = OclBuiltinImpl<atomic_fetch_and_explicit_scope_intptr_l, atomic_fetch_and_explicit_scope_intptr_l.Types, 0, atomic_fetch_binop_explicit_scope_l_code, [["$KEY","and"]]>;

OclBuiltinImpl atomic_fetch_min_g_impl = OclBuiltinImpl<atomic_fetch_min_g, atomic_fetch_min_g.Types, 0, atomic_fetch_key_code, [["$KEY","min"]]>;
OclBuiltinImpl atomic_fetch_min_explicit_g_impl = OclBuiltinImpl<atomic_fetch_min_explicit_g, atomic_fetch_min_explicit_g.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","min"]]>;
OclBuiltinImpl atomic_fetch_min_explicit_scope_g_impl = OclBuiltinImpl<atomic_fetch_min_explicit_scope_g, atomic_fetch_min_explicit_scope_g.Types, 0, atomic_fetch_relop_explicit_scope_g_code, [["$KEY","min"]]>;

OclBuiltinImpl atomic_fetch_min_l_impl = OclBuiltinImpl<atomic_fetch_min_l, atomic_fetch_min_l.Types, 0, atomic_fetch_key_code, [["$KEY","min"]]>;
OclBuiltinImpl atomic_fetch_min_explicit_l_impl = OclBuiltinImpl<atomic_fetch_min_explicit_l, atomic_fetch_min_explicit_l.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","min"]]>;
OclBuiltinImpl atomic_fetch_min_explicit_scope_l_impl = OclBuiltinImpl<atomic_fetch_min_explicit_scope_l, atomic_fetch_min_explicit_scope_l.Types, 0, atomic_fetch_relop_explicit_scope_l_code, [["$KEY","min"]]>;

OclBuiltinImpl atomic_fetch_min_uintptr_g_impl = OclBuiltinImpl<atomic_fetch_min_uintptr_g, atomic_fetch_min_uintptr_g.Types, 0, atomic_fetch_key_code, [["$KEY","min"]]>;
OclBuiltinImpl atomic_fetch_min_explicit_uintptr_g_impl = OclBuiltinImpl<atomic_fetch_min_explicit_uintptr_g, atomic_fetch_min_explicit_uintptr_g.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","min"]]>;
OclBuiltinImpl atomic_fetch_min_explicit_scope_uintptr_g_impl = OclBuiltinImpl<atomic_fetch_min_explicit_scope_uintptr_g, atomic_fetch_min_explicit_scope_uintptr_g.Types, 0, atomic_fetch_relop_explicit_scope_g_code, [["$KEY","min"]]>;

OclBuiltinImpl atomic_fetch_min_uintptr_l_impl = OclBuiltinImpl<atomic_fetch_min_uintptr_l, atomic_fetch_min_uintptr_l.Types, 0, atomic_fetch_key_code, [["$KEY","min"]]>;
OclBuiltinImpl atomic_fetch_min_explicit_uintptr_l_impl = OclBuiltinImpl<atomic_fetch_min_explicit_uintptr_l, atomic_fetch_min_explicit_uintptr_l.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","min"]]>;
OclBuiltinImpl atomic_fetch_min_explicit_scope_uintptr_l_impl = OclBuiltinImpl<atomic_fetch_min_explicit_scope_uintptr_l, atomic_fetch_min_explicit_scope_uintptr_l.Types, 0, atomic_fetch_relop_explicit_scope_l_code, [["$KEY","min"]]>;

OclBuiltinImpl atomic_fetch_min_intptr_g_impl = OclBuiltinImpl<atomic_fetch_min_intptr_g, atomic_fetch_min_intptr_g.Types, 0, atomic_fetch_key_code, [["$KEY","min"]]>;
OclBuiltinImpl atomic_fetch_min_explicit_intptr_g_impl = OclBuiltinImpl<atomic_fetch_min_explicit_intptr_g, atomic_fetch_min_explicit_intptr_g.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","min"]]>;
OclBuiltinImpl atomic_fetch_min_explicit_scope_intptr_g_impl = OclBuiltinImpl<atomic_fetch_min_explicit_scope_intptr_g, atomic_fetch_min_explicit_scope_intptr_g.Types, 0, atomic_fetch_relop_explicit_scope_g_code, [["$KEY","min"]]>;

OclBuiltinImpl atomic_fetch_min_intptr_l_impl = OclBuiltinImpl<atomic_fetch_min_intptr_l, atomic_fetch_min_intptr_l.Types, 0, atomic_fetch_key_code, [["$KEY","min"]]>;
OclBuiltinImpl atomic_fetch_min_explicit_intptr_l_impl = OclBuiltinImpl<atomic_fetch_min_explicit_intptr_l, atomic_fetch_min_explicit_intptr_l.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","min"]]>;
OclBuiltinImpl atomic_fetch_min_explicit_scope_intptr_l_impl = OclBuiltinImpl<atomic_fetch_min_explicit_scope_intptr_l, atomic_fetch_min_explicit_scope_intptr_l.Types, 0, atomic_fetch_relop_explicit_scope_l_code, [["$KEY","min"]]>;

OclBuiltinImpl atomic_fetch_max_g_impl = OclBuiltinImpl<atomic_fetch_max_g, atomic_fetch_max_g.Types, 0, atomic_fetch_key_code, [["$KEY","max"]]>;
OclBuiltinImpl atomic_fetch_max_explicit_g_impl = OclBuiltinImpl<atomic_fetch_max_explicit_g, atomic_fetch_max_explicit_g.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","max"]]>;
OclBuiltinImpl atomic_fetch_max_explicit_scope_g_impl = OclBuiltinImpl<atomic_fetch_max_explicit_scope_g, atomic_fetch_max_explicit_scope_g.Types, 0, atomic_fetch_relop_explicit_scope_g_code, [["$KEY","max"]]>;

OclBuiltinImpl atomic_fetch_max_l_impl = OclBuiltinImpl<atomic_fetch_max_l, atomic_fetch_max_l.Types, 0, atomic_fetch_key_code, [["$KEY","max"]]>;
OclBuiltinImpl atomic_fetch_max_explicit_l_impl = OclBuiltinImpl<atomic_fetch_max_explicit_l, atomic_fetch_max_explicit_l.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","max"]]>;
OclBuiltinImpl atomic_fetch_max_explicit_scope_l_impl = OclBuiltinImpl<atomic_fetch_max_explicit_scope_l, atomic_fetch_max_explicit_scope_l.Types, 0, atomic_fetch_relop_explicit_scope_l_code, [["$KEY","max"]]>;

OclBuiltinImpl atomic_fetch_max_uintptr_g_impl = OclBuiltinImpl<atomic_fetch_max_uintptr_g, atomic_fetch_max_uintptr_g.Types, 0, atomic_fetch_key_code, [["$KEY","max"]]>;
OclBuiltinImpl atomic_fetch_max_explicit_uintptr_g_impl = OclBuiltinImpl<atomic_fetch_max_explicit_uintptr_g, atomic_fetch_max_explicit_uintptr_g.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","max"]]>;
OclBuiltinImpl atomic_fetch_max_explicit_scope_uintptr_g_impl = OclBuiltinImpl<atomic_fetch_max_explicit_scope_uintptr_g, atomic_fetch_max_explicit_scope_uintptr_g.Types, 0, atomic_fetch_relop_explicit_scope_g_code, [["$KEY","max"]]>;

OclBuiltinImpl atomic_fetch_max_uintptr_l_impl = OclBuiltinImpl<atomic_fetch_max_uintptr_l, atomic_fetch_max_uintptr_l.Types, 0, atomic_fetch_key_code, [["$KEY","max"]]>;
OclBuiltinImpl atomic_fetch_max_explicit_uintptr_l_impl = OclBuiltinImpl<atomic_fetch_max_explicit_uintptr_l, atomic_fetch_max_explicit_uintptr_l.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","max"]]>;
OclBuiltinImpl atomic_fetch_max_explicit_scope_uintptr_l_impl = OclBuiltinImpl<atomic_fetch_max_explicit_scope_uintptr_l, atomic_fetch_max_explicit_scope_uintptr_l.Types, 0, atomic_fetch_relop_explicit_scope_l_code, [["$KEY","max"]]>;

OclBuiltinImpl atomic_fetch_max_intptr_g_impl = OclBuiltinImpl<atomic_fetch_max_intptr_g, atomic_fetch_max_intptr_g.Types, 0, atomic_fetch_key_code, [["$KEY","max"]]>;
OclBuiltinImpl atomic_fetch_max_explicit_intptr_g_impl = OclBuiltinImpl<atomic_fetch_max_explicit_intptr_g, atomic_fetch_max_explicit_intptr_g.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","max"]]>;
OclBuiltinImpl atomic_fetch_max_explicit_scope_intptr_g_impl = OclBuiltinImpl<atomic_fetch_max_explicit_scope_intptr_g, atomic_fetch_max_explicit_scope_intptr_g.Types, 0, atomic_fetch_relop_explicit_scope_g_code, [["$KEY","max"]]>;

OclBuiltinImpl atomic_fetch_max_intptr_l_impl = OclBuiltinImpl<atomic_fetch_max_intptr_l, atomic_fetch_max_intptr_l.Types, 0, atomic_fetch_key_code, [["$KEY","max"]]>;
OclBuiltinImpl atomic_fetch_max_explicit_intptr_l_impl = OclBuiltinImpl<atomic_fetch_max_explicit_intptr_l, atomic_fetch_max_explicit_intptr_l.Types, 0, atomic_fetch_key_explicit_code, [["$KEY","max"]]>;
OclBuiltinImpl atomic_fetch_max_explicit_scope_intptr_l_impl = OclBuiltinImpl<atomic_fetch_max_explicit_scope_intptr_l, atomic_fetch_max_explicit_scope_intptr_l.Types, 0, atomic_fetch_relop_explicit_scope_l_code, [["$KEY","max"]]>;

// atomic_flag_test_and_set
OclBuiltinImpl atomic_flag_test_and_set_g_impl = OclBuiltinImpl<atomic_flag_test_and_set_g, atomic_flag_test_and_set_g.Types, 0, [{
    return atomic_flag_test_and_set_explicit($Arg0VarName, memory_order_seq_cst, memory_scope_device);
  }]>;
OclBuiltinImpl atomic_flag_test_and_set_explicit_g_impl = OclBuiltinImpl<atomic_flag_test_and_set_explicit_g, atomic_flag_test_and_set_explicit_g.Types, 0, [{
    return atomic_flag_test_and_set_explicit($Arg0VarName, $Arg1VarName, memory_scope_device);
  }]>;
OclBuiltinImpl atomic_flag_test_and_set_explicit_scope_g_impl = OclBuiltinImpl<atomic_flag_test_and_set_explicit_scope_g, atomic_flag_test_and_set_explicit_scope_g.Types, 0, [{
    return atomic_exchange_explicit((volatile global atomic_int*)$Arg0VarName, 1, $Arg1VarName, $Arg2VarName);
  }]>;

OclBuiltinImpl atomic_flag_test_and_set_l_impl = OclBuiltinImpl<atomic_flag_test_and_set_l, atomic_flag_test_and_set_l.Types, 0, [{
    return atomic_flag_test_and_set_explicit($Arg0VarName, memory_order_relaxed, memory_scope_work_group);
  }]>;
OclBuiltinImpl atomic_flag_test_and_set_explicit_l_impl = OclBuiltinImpl<atomic_flag_test_and_set_explicit_l, atomic_flag_test_and_set_explicit_l.Types, 0, [{
    return atomic_flag_test_and_set_explicit($Arg0VarName, $Arg1VarName, memory_scope_work_group);
  }]>;
OclBuiltinImpl atomic_flag_test_and_set_explicit_scope_l_impl = OclBuiltinImpl<atomic_flag_test_and_set_explicit_scope_l, atomic_flag_test_and_set_explicit_scope_l.Types, 0, [{
    return atomic_exchange_explicit((volatile local atomic_int*)$Arg0VarName, 1, $Arg1VarName, $Arg2VarName);
  }]>;

//atomic_flag_clear
OclBuiltinImpl atomic_flag_clear_g_impl = OclBuiltinImpl<atomic_flag_clear_g, atomic_flag_clear_g.Types, 0, [{
    atomic_flag_clear_explicit($Arg0VarName, memory_order_seq_cst, memory_scope_device);
  }]>;
OclBuiltinImpl atomic_flag_clear_explicit_g_impl = OclBuiltinImpl<atomic_flag_clear_explicit_g, atomic_flag_clear_explicit_g.Types, 0, [{
    atomic_flag_clear_explicit($Arg0VarName, $Arg1VarName, memory_scope_device);
  }]>;
OclBuiltinImpl atomic_flag_clear_explicit_scope_g_impl = OclBuiltinImpl<atomic_flag_clear_explicit_scope_g, atomic_flag_clear_explicit_scope_g.Types, 0, [{
    (void) atomic_exchange_explicit((volatile global atomic_int*)$Arg0VarName, 0, $Arg1VarName, $Arg2VarName);
  }]>;

OclBuiltinImpl atomic_flag_clear_l_impl = OclBuiltinImpl<atomic_flag_clear_l, atomic_flag_clear_l.Types, 0, [{
    atomic_flag_clear_explicit($Arg0VarName, memory_order_relaxed, memory_scope_work_group);
  }]>;
OclBuiltinImpl atomic_flag_clear_explicit_l_impl = OclBuiltinImpl<atomic_flag_clear_explicit_l, atomic_flag_clear_explicit_l.Types, 0, [{
    atomic_flag_clear_explicit($Arg0VarName, $Arg1VarName, memory_scope_work_group);
  }]>;
OclBuiltinImpl atomic_flag_clear_explicit_scope_l_impl = OclBuiltinImpl<atomic_flag_clear_explicit_scope_l, atomic_flag_clear_explicit_scope_l.Types, 0, [{
    (void) atomic_exchange_explicit((volatile local atomic_int*)$Arg0VarName, 0, $Arg1VarName, $Arg2VarName);
  }]>;

// atomic_work_item_fence
OclBuiltinImpl atomic_work_item_fence_impl = OclBuiltinImpl<atomic_work_item_fence, atomic_work_item_fence.Types, 0, [{
    if($Arg0VarName & (CLK_GLOBAL_MEM_FENCE | CLK_CHANNEL_MEM_FENCE) &&
       $Arg2VarName != memory_scope_work_item  &&
       $Arg2VarName != memory_scope_work_group &&
       $Arg2VarName != memory_scope_sub_group) {  // this implementation relies on that a workgroup is executed
                                                  // by single phy. thread
      if($Arg1VarName == memory_order_acquire) {
        _mm_lfence();
      } else if($Arg1VarName == memory_order_release) {
        _mm_sfence();
      } else if($Arg1VarName == memory_order_acq_rel ||
                $Arg1VarName == memory_order_seq_cst) {
        _mm_mfence();
      }
      // No affect if memory order is relaxed
    }
  }]>;
