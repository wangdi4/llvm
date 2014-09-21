// Copyright (c) 2006-2012 Intel Corporation
// Atomic Functions
// OpenCL 1.2 rev 15, Table 6.12.11

list<OclType> atomic_types = [v1i32, v1u32];
// atomic_x functions
code inc_l = [{
    return atomicInc_local((intrin_type *)$Arg0VarName);
  }];
code inc_g = [{
    return atomicInc_global((intrin_type *)$Arg0VarName);
  }];
code dec_l = [{
    return atomicDec_local((intrin_type *)$Arg0VarName);
  }];
code dec_g = [{
    return atomicDec_global((intrin_type *)$Arg0VarName);
  }];
code add_l = [{
    return atomicAdd_local((intrin_type *)$Arg0VarName, $Arg1VarName);
  }];
code add_g = [{
    return atomicAdd_global((intrin_type *)$Arg0VarName, $Arg1VarName);
  }];
code sub_l = [{
    return atomicSub_local((intrin_type *)$Arg0VarName, $Arg1VarName);
  }];
code sub_g = [{
    return atomicSub_global((intrin_type *)$Arg0VarName, $Arg1VarName);
  }];
code xchg_l = [{
    return InterlockedExchange_local((intrin_type *)$Arg0VarName, $Arg1VarName);
  }];
code xchg_g = [{
    return InterlockedExchange_global((intrin_type *)$Arg0VarName, $Arg1VarName);
  }];
code xchg_l_fp = [{
    int ret = ( InterlockedExchange_local((intrin_type*)$Arg0VarName, as_int($Arg1VarName)) );
    return as_float(ret);
  }];
code xchg_g_fp = [{
    int ret = ( InterlockedExchange_global((intrin_type*)$Arg0VarName, as_int($Arg1VarName)) );
    return as_float(ret);
  }];
code cmpxchg_l = [{
    return (_InterlockedCompareExchange_local((intrin_type *)$Arg0VarName, $Arg1VarName, $Arg2VarName) );
  }];
code cmpxchg_g = [{
    return (_InterlockedCompareExchange_global((intrin_type *)$Arg0VarName, $Arg1VarName, $Arg2VarName) );
  }];
code max_l = [{
    while (true)
    {
      local $ReturnType* tmp = (local $ReturnType*)$Arg0VarName;
      int oldValue = *tmp;
      if (oldValue >= $Arg1VarName) return oldValue;
      int retVal = _InterlockedCompareExchange_local((intrin_type *)tmp, oldValue, $Arg1VarName);
      if (retVal == oldValue) return oldValue;
    }
  }];
code max_g = [{
    while (true)
    {
      int oldValue = *$Arg0VarName;
      if (oldValue >= $Arg1VarName) return oldValue;
      int retVal = _InterlockedCompareExchange_global((intrin_type *)$Arg0VarName, oldValue, $Arg1VarName);
      if (retVal == oldValue) return oldValue;
    }
  }];
code min_l = [{
    while (true)
    {
      local $ReturnType* tmp = (local $ReturnType*)$Arg0VarName;
      int oldValue = *tmp;
      if (oldValue <= $Arg1VarName) return oldValue;
      int retVal = _InterlockedCompareExchange_local((intrin_type *)tmp, oldValue, $Arg1VarName);
      if (retVal == oldValue) return oldValue;
    }
  }];
code min_g = [{
    while (true)
    {
      int oldValue = *$Arg0VarName;
      if (oldValue <= $Arg1VarName) return oldValue;
      int retVal = _InterlockedCompareExchange_global((intrin_type *)$Arg0VarName, oldValue, $Arg1VarName);
      if (retVal == oldValue) return oldValue;
    }
  }];
code and_l = [{
    return (_InterlockedAnd_local((intrin_type *)$Arg0VarName, $Arg1VarName) );
  }];
code and_g = [{
    return (_InterlockedAnd_global((intrin_type *)$Arg0VarName, $Arg1VarName) );
  }];
code or_l = [{
    return (_InterlockedOr_local((intrin_type *)$Arg0VarName, $Arg1VarName) );
  }];
code or_g = [{
    return (_InterlockedOr_global((intrin_type *)$Arg0VarName, $Arg1VarName) );
  }];
code xor_l = [{
    return (_InterlockedXor_local((intrin_type *)$Arg0VarName, $Arg1VarName) );
  }];
code xor_g = [{
    return (_InterlockedXor_global((intrin_type *)$Arg0VarName, $Arg1VarName) );
  }];
   
  
// inc
OclBuiltinImpl atomic_inc_g   = OclBuiltinImpl<atomic_inc_global, atomic_types, 0, inc_g>;
OclBuiltinImpl atomic_inc_l   = OclBuiltinImpl<atomic_inc_local, atomic_types, 0, inc_l>;
OclBuiltinImpl atom_inc_vg    = OclBuiltinImpl<atom_inc_vol_global, atomic_types, 0, inc_g>;
OclBuiltinImpl atom_inc_g     = OclBuiltinImpl<atom_inc_global, atomic_types, 0, inc_g>;
OclBuiltinImpl atom_inc_vl    = OclBuiltinImpl<atom_inc_vol_local, atomic_types, 0, inc_l>;
OclBuiltinImpl atom_inc_l     = OclBuiltinImpl<atom_inc_local, atomic_types, 0, inc_l>;
// dec
OclBuiltinImpl atomic_dec_g   = OclBuiltinImpl<atomic_dec_global, atomic_types, 0, dec_g>;
OclBuiltinImpl atomic_dec_l   = OclBuiltinImpl<atomic_dec_local, atomic_types, 0, dec_l>;
OclBuiltinImpl atom_dec_vg    = OclBuiltinImpl<atom_dec_vol_global, atomic_types, 0, dec_g>;
OclBuiltinImpl atom_dec_g     = OclBuiltinImpl<atom_dec_global, atomic_types, 0, dec_g>;
OclBuiltinImpl atom_dec_vl    = OclBuiltinImpl<atom_dec_vol_local, atomic_types, 0, dec_l>;
OclBuiltinImpl atom_dec_l     = OclBuiltinImpl<atom_dec_local, atomic_types, 0, dec_l>;
// add
OclBuiltinImpl atomic_add_g   = OclBuiltinImpl<atomic_add_global, atomic_types, 0, add_g>;
OclBuiltinImpl atomic_add_l   = OclBuiltinImpl<atomic_add_local, atomic_types, 0, add_l>;
OclBuiltinImpl atom_add_vg    = OclBuiltinImpl<atom_add_vol_global, atomic_types, 0, add_g>;
OclBuiltinImpl atom_add_g     = OclBuiltinImpl<atom_add_global, atomic_types, 0, add_g>;
OclBuiltinImpl atom_add_vl    = OclBuiltinImpl<atom_add_vol_local, atomic_types, 0, add_l>;
OclBuiltinImpl atom_add_l     = OclBuiltinImpl<atom_add_local, atomic_types, 0, add_l>;
// sub
OclBuiltinImpl atomic_sub_g   = OclBuiltinImpl<atomic_sub_global, atomic_types, 0, sub_g>;
OclBuiltinImpl atomic_sub_l   = OclBuiltinImpl<atomic_sub_local, atomic_types, 0, sub_l>;
OclBuiltinImpl atom_sub_vg    = OclBuiltinImpl<atom_sub_vol_global, atomic_types, 0, sub_g>;
OclBuiltinImpl atom_sub_g     = OclBuiltinImpl<atom_sub_global, atomic_types, 0, sub_g>;
OclBuiltinImpl atom_sub_vl    = OclBuiltinImpl<atom_sub_vol_local, atomic_types, 0, sub_l>;
OclBuiltinImpl atom_sub_l     = OclBuiltinImpl<atom_sub_local, atomic_types, 0, sub_l>;
// xchg
OclBuiltinImpl atomic_xchg_g   = OclBuiltinImpl<atomic_xchg_global, atomic_types, 0, xchg_g>;
OclBuiltinImpl atomic_xchg_l   = OclBuiltinImpl<atomic_xchg_local, atomic_types, 0, xchg_l>;
OclBuiltinImpl atom_xchg_vg    = OclBuiltinImpl<atom_xchg_vol_global, atomic_types, 0, xchg_g>;
OclBuiltinImpl atom_xchg_g     = OclBuiltinImpl<atom_xchg_global, atomic_types, 0, xchg_g>;
OclBuiltinImpl atom_xchg_vl    = OclBuiltinImpl<atom_xchg_vol_local, atomic_types, 0, xchg_l>;
OclBuiltinImpl atom_xchg_l     = OclBuiltinImpl<atom_xchg_local, atomic_types, 0, xchg_l>;

OclBuiltinImpl atomic_xchg_g_fp   = OclBuiltinImpl<atomic_xchg_global, [v1f32], 0, xchg_g_fp>;
OclBuiltinImpl atomic_xchg_l_fp   = OclBuiltinImpl<atomic_xchg_local, [v1f32], 0, xchg_l_fp>;
OclBuiltinImpl atom_xchg_vg_fp    = OclBuiltinImpl<atom_xchg_vol_global, [v1f32], 0, xchg_g_fp>;
OclBuiltinImpl atom_xchg_g_fp     = OclBuiltinImpl<atom_xchg_global, [v1f32], 0, xchg_g_fp>;
OclBuiltinImpl atom_xchg_vl_fp    = OclBuiltinImpl<atom_xchg_vol_local, [v1f32], 0, xchg_l_fp>;
OclBuiltinImpl atom_xchg_l_fp     = OclBuiltinImpl<atom_xchg_local, [v1f32], 0, xchg_l_fp>;
// cmpxchg
OclBuiltinImpl atomic_cmpxchg_g   = OclBuiltinImpl<atomic_cmpxchg_global, atomic_types, 0, cmpxchg_g>;
OclBuiltinImpl atomic_cmpxchg_l   = OclBuiltinImpl<atomic_cmpxchg_local, atomic_types, 0, cmpxchg_l>;
OclBuiltinImpl atom_cmpxchg_vg    = OclBuiltinImpl<atom_cmpxchg_vol_global, atomic_types, 0, cmpxchg_g>;
OclBuiltinImpl atom_cmpxchg_g     = OclBuiltinImpl<atom_cmpxchg_global, atomic_types, 0, cmpxchg_g>;
OclBuiltinImpl atom_cmpxchg_vl    = OclBuiltinImpl<atom_cmpxchg_vol_local, atomic_types, 0, cmpxchg_l>;
OclBuiltinImpl atom_cmpxchg_l     = OclBuiltinImpl<atom_cmpxchg_local, atomic_types, 0, cmpxchg_l>;
// max
OclBuiltinImpl atomic_max_g   = OclBuiltinImpl<atomic_max_global, atomic_types, 0, max_g>;
OclBuiltinImpl atomic_max_l   = OclBuiltinImpl<atomic_max_local, atomic_types, 0, max_l>;
OclBuiltinImpl atom_max_vg    = OclBuiltinImpl<atom_max_vol_global, atomic_types, 0, max_g>;
OclBuiltinImpl atom_max_g     = OclBuiltinImpl<atom_max_global, atomic_types, 0, max_g>;
OclBuiltinImpl atom_max_vl    = OclBuiltinImpl<atom_max_vol_local, atomic_types, 0, max_l>;
OclBuiltinImpl atom_max_l     = OclBuiltinImpl<atom_max_local, atomic_types, 0, max_l>;
// min
OclBuiltinImpl atomic_min_g   = OclBuiltinImpl<atomic_min_global, atomic_types, 0, min_g>;
OclBuiltinImpl atomic_min_l   = OclBuiltinImpl<atomic_min_local, atomic_types, 0, min_l>;
OclBuiltinImpl atom_min_vg    = OclBuiltinImpl<atom_min_vol_global, atomic_types, 0, min_g>;
OclBuiltinImpl atom_min_g     = OclBuiltinImpl<atom_min_global, atomic_types, 0, min_g>;
OclBuiltinImpl atom_min_vl    = OclBuiltinImpl<atom_min_vol_local, atomic_types, 0, min_l>;
OclBuiltinImpl atom_min_l     = OclBuiltinImpl<atom_min_local, atomic_types, 0, min_l>;
// and
OclBuiltinImpl atomic_and_g   = OclBuiltinImpl<atomic_and_global, atomic_types, 0, and_g>;
OclBuiltinImpl atomic_and_l   = OclBuiltinImpl<atomic_and_local, atomic_types, 0, and_l>;
OclBuiltinImpl atom_and_vg    = OclBuiltinImpl<atom_and_vol_global, atomic_types, 0, and_g>;
OclBuiltinImpl atom_and_g     = OclBuiltinImpl<atom_and_global, atomic_types, 0, and_g>;
OclBuiltinImpl atom_and_vl    = OclBuiltinImpl<atom_and_vol_local, atomic_types, 0, and_l>;
OclBuiltinImpl atom_and_l     = OclBuiltinImpl<atom_and_local, atomic_types, 0, and_l>;
// or
OclBuiltinImpl atomic_or_g   = OclBuiltinImpl<atomic_or_global, atomic_types, 0, or_g>;
OclBuiltinImpl atomic_or_l   = OclBuiltinImpl<atomic_or_local, atomic_types, 0, or_l>;
OclBuiltinImpl atom_or_vg    = OclBuiltinImpl<atom_or_vol_global, atomic_types, 0, or_g>;
OclBuiltinImpl atom_or_g     = OclBuiltinImpl<atom_or_global, atomic_types, 0, or_g>;
OclBuiltinImpl atom_or_vl    = OclBuiltinImpl<atom_or_vol_local, atomic_types, 0, or_l>;
OclBuiltinImpl atom_or_l     = OclBuiltinImpl<atom_or_local, atomic_types, 0, or_l>;
// xor
OclBuiltinImpl atomic_xor_g   = OclBuiltinImpl<atomic_xor_global, atomic_types, 0, xor_g>;
OclBuiltinImpl atomic_xor_l   = OclBuiltinImpl<atomic_xor_local, atomic_types, 0, xor_l>;
OclBuiltinImpl atom_xor_vg    = OclBuiltinImpl<atom_xor_vol_global, atomic_types, 0, xor_g>;
OclBuiltinImpl atom_xor_g     = OclBuiltinImpl<atom_xor_global, atomic_types, 0, xor_g>;
OclBuiltinImpl atom_xor_vl    = OclBuiltinImpl<atom_xor_vol_local, atomic_types, 0, xor_l>;
OclBuiltinImpl atom_xor_l     = OclBuiltinImpl<atom_xor_local, atomic_types, 0, xor_l>;

