code add_g = [{
    return __sync_fetch_and_add($Arg0VarName, $Arg1VarName);
  }];
code add_l = [{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
    $ReturnType oldValue = *$Arg0VarName;
    *$Arg0VarName = oldValue + $Arg1VarName;
    return oldValue;
  }];
code sub_g = [{
    return __sync_fetch_and_sub($Arg0VarName, $Arg1VarName);
  }];
code sub_l = [{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
    $ReturnType oldValue = *$Arg0VarName;
    *$Arg0VarName = oldValue - $Arg1VarName;
    return oldValue;
  }];
code xchg_g = [{
    _mm_mfence();
    return ($ReturnType)__sync_lock_test_and_set($Arg0VarName, $Arg1VarName);
  }];
code xchg_l = [{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
    $ReturnType oldValue = *$Arg0VarName;
    *$Arg0VarName = $Arg1VarName;
    return oldValue;
  }];
code xchg_g_fp = [{
    _mm_mfence();
    int ret = ( __sync_lock_test_and_set((intrin_type*)$Arg0VarName, *((int *)(&$Arg1VarName))) );
    return as_float(ret);
  }];



code max_g = [{
    while (true)
    {
      $ReturnType oldValue = *$Arg0VarName;
      if (oldValue >= $Arg1VarName) return oldValue;
      $ReturnType retVal = __sync_val_compare_and_swap((intrin_type *)$Arg0VarName, oldValue, $Arg1VarName);
      if (retVal == oldValue) return oldValue;
    }
  }];
code max_l = [{
    $ReturnType oldValue = *$Arg0VarName;
    *$Arg0VarName = (($Arg1VarName > oldValue) ? $Arg1VarName : oldValue);
    return oldValue;
  }];
code min_g = [{
    while (true)
    {
      $ReturnType oldValue = *$Arg0VarName;
      if (oldValue <= $Arg1VarName) return oldValue;
      $ReturnType retVal = __sync_val_compare_and_swap((intrin_type *)$Arg0VarName, oldValue, $Arg1VarName);
      if (retVal == oldValue) return oldValue;
    }
  }];
code min_l = [{
    $ReturnType oldValue = *$Arg0VarName;
    *$Arg0VarName = (($Arg1VarName < oldValue) ? $Arg1VarName : oldValue);
    return oldValue;
  }];

code inc_g = [{
    return __sync_fetch_and_add($Arg0VarName, 1);
  }];
code inc_l = [{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
    $ReturnType oldValue = *$Arg0VarName;
    *$Arg0VarName = oldValue + 1;
    return oldValue;
  }];
code dec_g = [{
    return __sync_fetch_and_sub($Arg0VarName, 1);
  }];
code dec_l = [{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
    $ReturnType oldValue = *$Arg0VarName;
    *$Arg0VarName = oldValue - 1;
    return oldValue;
  }];
code cmpxchg_g = [{
    return __sync_val_compare_and_swap($Arg0VarName, $Arg1VarName, $Arg2VarName);
  }];
code cmpxchg_l = [{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
    $ReturnType oldValue = *$Arg0VarName;
    if (oldValue == $Arg1VarName)
    {
      *$Arg0VarName = $Arg2VarName;
    }
    return oldValue;
  }];
code and_g = [{
    return __sync_fetch_and_and($Arg0VarName, $Arg1VarName);
  }];
code and_l = [{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
    $ReturnType oldValue = *$Arg0VarName;
    *$Arg0VarName = oldValue & $Arg1VarName;
    return oldValue;
  }];
code or_g = [{
    return __sync_fetch_and_or($Arg0VarName, $Arg1VarName);
  }];
code or_l = [{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
    $ReturnType oldValue = *$Arg0VarName;
    *$Arg0VarName = oldValue | $Arg1VarName;
    return oldValue;
  }];
code xor_g = [{
    return __sync_fetch_and_xor($Arg0VarName, $Arg1VarName);
  }];
code xor_l = [{
    // this implementation relies on that workgroup executed by single phy. thread see comment in the beginning of the file
    $ReturnType oldValue = *$Arg0VarName;
    *$Arg0VarName = oldValue ^ $Arg1VarName;
    return oldValue;
  }];


//=============================================

// inc
OclBuiltinImpl atomic_inc_g   = OclBuiltinImpl<atomic_inc_global, [v1i32, v1u32], 0, inc_g>;
OclBuiltinImpl atomic_inc_l   = OclBuiltinImpl<atomic_inc_local, [v1i32, v1u32], 0, inc_l>;
OclBuiltinImpl atom_inc_vg    = OclBuiltinImpl<atom_inc_vol_global, [v1i32, v1u32], 0, inc_g>;
OclBuiltinImpl atom_inc_g     = OclBuiltinImpl<atom_inc_global, [v1i32, v1u32], 0, inc_g>;
OclBuiltinImpl atom_inc_vl    = OclBuiltinImpl<atom_inc_vol_local, [v1i32, v1u32], 0, inc_l>;
OclBuiltinImpl atom_inc_l     = OclBuiltinImpl<atom_inc_local, [v1i32, v1u32], 0, inc_l>;
// dec
OclBuiltinImpl atomic_dec_g   = OclBuiltinImpl<atomic_dec_global, [v1i32, v1u32], 0, dec_g>;
OclBuiltinImpl atomic_dec_l   = OclBuiltinImpl<atomic_dec_local, [v1i32, v1u32], 0, dec_l>;
OclBuiltinImpl atom_dec_vg    = OclBuiltinImpl<atom_dec_vol_global, [v1i32, v1u32], 0, dec_g>;
OclBuiltinImpl atom_dec_g     = OclBuiltinImpl<atom_dec_global, [v1i32, v1u32], 0, dec_g>;
OclBuiltinImpl atom_dec_vl    = OclBuiltinImpl<atom_dec_vol_local, [v1i32, v1u32], 0, dec_l>;
OclBuiltinImpl atom_dec_l     = OclBuiltinImpl<atom_dec_local, [v1i32, v1u32], 0, dec_l>;
// add
OclBuiltinImpl atomic_add_g   = OclBuiltinImpl<atomic_add_global, [v1i32, v1u32], 0, add_g>;
OclBuiltinImpl atomic_add_l   = OclBuiltinImpl<atomic_add_local, [v1i32, v1u32], 0, add_l>;
OclBuiltinImpl atom_add_vg    = OclBuiltinImpl<atom_add_vol_global, [v1i32, v1u32], 0, add_g>;
OclBuiltinImpl atom_add_g     = OclBuiltinImpl<atom_add_global, [v1i32, v1u32], 0, add_g>;
OclBuiltinImpl atom_add_vl    = OclBuiltinImpl<atom_add_vol_local, [v1i32, v1u32], 0, add_l>;
OclBuiltinImpl atom_add_l     = OclBuiltinImpl<atom_add_local, [v1i32, v1u32], 0, add_l>;
// sub
OclBuiltinImpl atomic_sub_g   = OclBuiltinImpl<atomic_sub_global, [v1i32, v1u32], 0, sub_g>;
OclBuiltinImpl atomic_sub_l   = OclBuiltinImpl<atomic_sub_local, [v1i32, v1u32], 0, sub_l>;
OclBuiltinImpl atom_sub_vg    = OclBuiltinImpl<atom_sub_vol_global, [v1i32, v1u32], 0, sub_g>;
OclBuiltinImpl atom_sub_g     = OclBuiltinImpl<atom_sub_global, [v1i32, v1u32], 0, sub_g>;
OclBuiltinImpl atom_sub_vl    = OclBuiltinImpl<atom_sub_vol_local, [v1i32, v1u32], 0, sub_l>;
OclBuiltinImpl atom_sub_l     = OclBuiltinImpl<atom_sub_local, [v1i32, v1u32], 0, sub_l>;
// xchg
OclBuiltinImpl atomic_xchg_g   = OclBuiltinImpl<atomic_xchg_global, [v1i32, v1u32], 0, xchg_g>;
OclBuiltinImpl atomic_xchg_l   = OclBuiltinImpl<atomic_xchg_local, [v1i32, v1u32, v1f32], 0, xchg_l>;
OclBuiltinImpl atom_xchg_vg    = OclBuiltinImpl<atom_xchg_vol_global, [v1i32, v1u32], 0, xchg_g>;
OclBuiltinImpl atom_xchg_g     = OclBuiltinImpl<atom_xchg_global, [v1i32, v1u32], 0, xchg_g>;
OclBuiltinImpl atom_xchg_vl    = OclBuiltinImpl<atom_xchg_vol_local, [v1i32, v1u32, v1f32], 0, xchg_l>;
OclBuiltinImpl atom_xchg_l     = OclBuiltinImpl<atom_xchg_local, [v1i32, v1u32, v1f32], 0, xchg_l>;

OclBuiltinImpl atomic_xchg_gf   = OclBuiltinImpl<atomic_xchg_global, [v1f32], 0, xchg_g_fp>;
OclBuiltinImpl atom_xchg_vgf    = OclBuiltinImpl<atom_xchg_vol_global, [v1f32], 0, xchg_g_fp>;
OclBuiltinImpl atom_xchg_gf     = OclBuiltinImpl<atom_xchg_global, [v1f32], 0, xchg_g_fp>;
// cmpxchg
OclBuiltinImpl atomic_cmpxchg_g   = OclBuiltinImpl<atomic_cmpxchg_global, [v1i32, v1u32], 0, cmpxchg_g>;
OclBuiltinImpl atomic_cmpxchg_l   = OclBuiltinImpl<atomic_cmpxchg_local, [v1i32, v1u32], 0, cmpxchg_l>;
OclBuiltinImpl atom_cmpxchg_vg    = OclBuiltinImpl<atom_cmpxchg_vol_global, [v1i32, v1u32], 0, cmpxchg_g>;
OclBuiltinImpl atom_cmpxchg_g     = OclBuiltinImpl<atom_cmpxchg_global, [v1i32, v1u32], 0, cmpxchg_g>;
OclBuiltinImpl atom_cmpxchg_vl    = OclBuiltinImpl<atom_cmpxchg_vol_local, [v1i32, v1u32], 0, cmpxchg_l>;
OclBuiltinImpl atom_cmpxchg_l     = OclBuiltinImpl<atom_cmpxchg_local, [v1i32, v1u32], 0, cmpxchg_l>;
// max
OclBuiltinImpl atomic_max_g   = OclBuiltinImpl<atomic_max_global, [v1i32, v1u32], 0, max_g>;
OclBuiltinImpl atomic_max_l   = OclBuiltinImpl<atomic_max_local, [v1i32, v1u32], 0, max_l>;
OclBuiltinImpl atom_max_vg    = OclBuiltinImpl<atom_max_vol_global, [v1i32, v1u32], 0, max_g>;
OclBuiltinImpl atom_max_g     = OclBuiltinImpl<atom_max_global, [v1i32, v1u32], 0, max_g>;
OclBuiltinImpl atom_max_vl    = OclBuiltinImpl<atom_max_vol_local, [v1i32, v1u32], 0, max_l>;
OclBuiltinImpl atom_max_l     = OclBuiltinImpl<atom_max_local, [v1i32, v1u32], 0, max_l>;
// min
OclBuiltinImpl atomic_min_g   = OclBuiltinImpl<atomic_min_global, [v1i32, v1u32], 0, min_g>;
OclBuiltinImpl atomic_min_l   = OclBuiltinImpl<atomic_min_local, [v1i32, v1u32], 0, min_l>;
OclBuiltinImpl atom_min_vg    = OclBuiltinImpl<atom_min_vol_global, [v1i32, v1u32], 0, min_g>;
OclBuiltinImpl atom_min_g     = OclBuiltinImpl<atom_min_global, [v1i32, v1u32], 0, min_g>;
OclBuiltinImpl atom_min_vl    = OclBuiltinImpl<atom_min_vol_local, [v1i32, v1u32], 0, min_l>;
OclBuiltinImpl atom_min_l     = OclBuiltinImpl<atom_min_local, [v1i32, v1u32], 0, min_l>;
// and
OclBuiltinImpl atomic_and_g   = OclBuiltinImpl<atomic_and_global, [v1i32, v1u32], 0, and_g>;
OclBuiltinImpl atomic_and_l   = OclBuiltinImpl<atomic_and_local, [v1i32, v1u32], 0, and_l>;
OclBuiltinImpl atom_and_vg    = OclBuiltinImpl<atom_and_vol_global, [v1i32, v1u32], 0, and_g>;
OclBuiltinImpl atom_and_g     = OclBuiltinImpl<atom_and_global, [v1i32, v1u32], 0, and_g>;
OclBuiltinImpl atom_and_vl    = OclBuiltinImpl<atom_and_vol_local, [v1i32, v1u32], 0, and_l>;
OclBuiltinImpl atom_and_l     = OclBuiltinImpl<atom_and_local, [v1i32, v1u32], 0, and_l>;
// or
OclBuiltinImpl atomic_or_g   = OclBuiltinImpl<atomic_or_global, [v1i32, v1u32], 0, or_g>;
OclBuiltinImpl atomic_or_l   = OclBuiltinImpl<atomic_or_local, [v1i32, v1u32], 0, or_l>;
OclBuiltinImpl atom_or_vg    = OclBuiltinImpl<atom_or_vol_global, [v1i32, v1u32], 0, or_g>;
OclBuiltinImpl atom_or_g     = OclBuiltinImpl<atom_or_global, [v1i32, v1u32], 0, or_g>;
OclBuiltinImpl atom_or_vl    = OclBuiltinImpl<atom_or_vol_local, [v1i32, v1u32], 0, or_l>;
OclBuiltinImpl atom_or_l     = OclBuiltinImpl<atom_or_local, [v1i32, v1u32], 0, or_l>;
// xor
OclBuiltinImpl atomic_xor_g   = OclBuiltinImpl<atomic_xor_global, [v1i32, v1u32], 0, xor_g>;
OclBuiltinImpl atomic_xor_l   = OclBuiltinImpl<atomic_xor_local, [v1i32, v1u32], 0, xor_l>;
OclBuiltinImpl atom_xor_vg    = OclBuiltinImpl<atom_xor_vol_global, [v1i32, v1u32], 0, xor_g>;
OclBuiltinImpl atom_xor_g     = OclBuiltinImpl<atom_xor_global, [v1i32, v1u32], 0, xor_g>;
OclBuiltinImpl atom_xor_vl    = OclBuiltinImpl<atom_xor_vol_local, [v1i32, v1u32], 0, xor_l>;
OclBuiltinImpl atom_xor_l     = OclBuiltinImpl<atom_xor_local, [v1i32, v1u32], 0, xor_l>;



