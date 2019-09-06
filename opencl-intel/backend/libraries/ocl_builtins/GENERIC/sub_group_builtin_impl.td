list<OclType> sub_group_types_all_any_scalar = [v1i32];

list<OclType> sub_group_types_broadcast_scalar = [v1i32, v1u32, v1i64, v1u64, v1f32, v1f64];

list<OclType> sub_group_types_reduce_scan_scalar = [v1i32, v1u32, v1i64, v1u64, v1f32, v1f64];

list<OclType> sub_group_types_suffle_scalar = [v1i32, v1u32, v1i64, v1u64, v1f32, v1f64,
                                         v2i32, v4i32, v8i32, v16i32,
                                         v2u32, v4u32, v8u32, v16u32,
                                         v2f32, v4f32, v8f32, v16f32];

list<OclType> sub_group_types_block_read_write_scalar = [v1u32];
//
// Reductions
//

// any/all

OclBuiltinImpl sub_group_all_fake = OclBuiltinImpl<sub_group_all, sub_group_types_all_any_scalar, 0,
  [{
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_any_fake = OclBuiltinImpl<sub_group_any, sub_group_types_all_any_scalar, 0,
  [{
    return $Arg0VarName;
  }]>;

// broadcast

OclBuiltinImpl sub_group_broadcast_fake = OclBuiltinImpl<sub_group_broadcast, sub_group_types_broadcast_scalar, 0,
  [{
    return $Arg0VarName;
  }]>;

// reduce [add|min|max]

OclBuiltinImpl sub_group_reduce_add_fake = OclBuiltinImpl<sub_group_reduce_add, sub_group_types_reduce_scan_scalar, 0,
  [{
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_reduce_min_fake = OclBuiltinImpl<sub_group_reduce_min, sub_group_types_reduce_scan_scalar, 0,
  [{
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_reduce_max_fake = OclBuiltinImpl<sub_group_reduce_max, sub_group_types_reduce_scan_scalar, 0,
  [{
    return $Arg0VarName;
  }]>;

// exclusive [add|min|max]
OclBuiltinImpl sub_group_scan_exclusive_add_fake = OclBuiltinImpl<sub_group_scan_exclusive_add, sub_group_types_reduce_scan_scalar, 0,
  [{
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_min_fake = OclBuiltinImpl<sub_group_scan_exclusive_min, sub_group_types_reduce_scan_scalar, 0,
  [{
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_scan_exclusive_max_fake = OclBuiltinImpl<sub_group_scan_exclusive_max, sub_group_types_reduce_scan_scalar, 0,
  [{
    return $Arg0VarName;
  }]>;

// inclusive [add|min|max]

OclBuiltinImpl sub_group_scan_inclusive_add_fake = OclBuiltinImpl<sub_group_scan_inclusive_add, sub_group_types_reduce_scan_scalar, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl sub_group_scan_inclusive_min_fake = OclBuiltinImpl<sub_group_scan_inclusive_min, sub_group_types_reduce_scan_scalar, 0,
  [{
    return $Arg0VarName;
  }]>;
OclBuiltinImpl sub_group_scan_inclusive_max_fake = OclBuiltinImpl<sub_group_scan_inclusive_max, sub_group_types_reduce_scan_scalar, 0,
  [{
    return $Arg0VarName;
  }]>;

//
// Shuffles
//

OclBuiltinImpl sub_group_shuffle_vec_type_fake = OclBuiltinImpl<sub_group_shuffle, sub_group_types_suffle_scalar, 0,
  [{
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_shuffle_xor_vec_type_fake = OclBuiltinImpl<sub_group_shuffle_xor, sub_group_types_suffle_scalar, 0,
  [{
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_shuffle_up_vec_type_fake = OclBuiltinImpl<sub_group_shuffle_up, sub_group_types_suffle_scalar, 0,
  [{
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_shuffle_down_vec_type_fake = OclBuiltinImpl<sub_group_shuffle_down, sub_group_types_suffle_scalar, 0,
  [{
    return $Arg0VarName;
  }]>;


///
/// Block read
///

// scalar types

OclBuiltinImpl intel_sub_group_block_read_fake = OclBuiltinImpl<intel_sub_group_block_read, sub_group_types_block_read_write_scalar, 0,
  [{
    return 0;
  }]>;

OclBuiltinImpl intel_sub_group_block_read2_fake = OclBuiltinImpl<intel_sub_group_block_read2, sub_group_types_block_read_write_scalar, 0,
  [{
    return 0;
  }]>;

OclBuiltinImpl intel_sub_group_block_read4_fake = OclBuiltinImpl<intel_sub_group_block_read4, sub_group_types_block_read_write_scalar, 0,
  [{
    return 0;
  }]>;

OclBuiltinImpl intel_sub_group_block_read8_fake = OclBuiltinImpl<intel_sub_group_block_read8, sub_group_types_block_read_write_scalar, 0,
  [{
    return 0;
  }]>;

// vector types
OclBuiltinImpl intel_sub_group_block_read1_4_fake = OclBuiltinImpl<intel_sub_group_block_read1_4, sub_group_types_block_read_write_scalar, 0,
  [{
    return 0;
  }]>;

OclBuiltinImpl intel_sub_group_block_read2_4_fake = OclBuiltinImpl<intel_sub_group_block_read2_4, sub_group_types_block_read_write_scalar, 0,
  [{
    return 0;
  }]>;

OclBuiltinImpl intel_sub_group_block_read4_4_fake = OclBuiltinImpl<intel_sub_group_block_read4_4, sub_group_types_block_read_write_scalar, 0,
  [{
    return 0;
  }]>;

OclBuiltinImpl intel_sub_group_block_read8_4_fake = OclBuiltinImpl<intel_sub_group_block_read8_4, sub_group_types_block_read_write_scalar, 0,
  [{
    return 0;
  }]>;

OclBuiltinImpl intel_sub_group_block_read1_8_fake = OclBuiltinImpl<intel_sub_group_block_read1_8, sub_group_types_block_read_write_scalar, 0,
  [{
    return 0;
  }]>;

OclBuiltinImpl intel_sub_group_block_read2_8_fake = OclBuiltinImpl<intel_sub_group_block_read2_8, sub_group_types_block_read_write_scalar, 0,
  [{
    return 0;
  }]>;

OclBuiltinImpl intel_sub_group_block_read4_8_fake = OclBuiltinImpl<intel_sub_group_block_read4_8, sub_group_types_block_read_write_scalar, 0,
  [{
    return 0;
  }]>;

OclBuiltinImpl intel_sub_group_block_read8_8_fake = OclBuiltinImpl<intel_sub_group_block_read8_8, sub_group_types_block_read_write_scalar, 0,
  [{
    return 0;
  }]>;

OclBuiltinImpl intel_sub_group_block_read1_16_fake = OclBuiltinImpl<intel_sub_group_block_read1_16, sub_group_types_block_read_write_scalar, 0,
  [{
    return 0;
  }]>;

OclBuiltinImpl intel_sub_group_block_read2_16_fake = OclBuiltinImpl<intel_sub_group_block_read2_16, sub_group_types_block_read_write_scalar, 0,
  [{
    return 0;
  }]>;

OclBuiltinImpl intel_sub_group_block_read4_16_fake = OclBuiltinImpl<intel_sub_group_block_read4_16, sub_group_types_block_read_write_scalar, 0,
  [{
    return 0;
  }]>;

OclBuiltinImpl intel_sub_group_block_read8_16_fake = OclBuiltinImpl<intel_sub_group_block_read8_16, sub_group_types_block_read_write_scalar, 0,
  [{
    return 0;
  }]>;

///
/// Block write
///

// scalar types

OclBuiltinImpl intel_sub_group_block_write_fake = OclBuiltinImpl<intel_sub_group_block_write, sub_group_types_block_read_write_scalar, 0,
  [{
    return;
  }]>;

OclBuiltinImpl intel_sub_group_block_write2_fake = OclBuiltinImpl<intel_sub_group_block_write2, sub_group_types_block_read_write_scalar, 0,
  [{
    return;
  }]>;

OclBuiltinImpl intel_sub_group_block_write4_fake = OclBuiltinImpl<intel_sub_group_block_write4, sub_group_types_block_read_write_scalar, 0,
  [{
    return;
  }]>;

OclBuiltinImpl intel_sub_group_block_write8_fake = OclBuiltinImpl<intel_sub_group_block_write8, sub_group_types_block_read_write_scalar, 0,
  [{
    return;
  }]>;

// vector

OclBuiltinImpl intel_sub_group_block_write1_4_fake = OclBuiltinImpl<intel_sub_group_block_write1_4, sub_group_types_block_read_write_scalar, 0,
  [{
  }]>;

OclBuiltinImpl intel_sub_group_block_write2_4_fake = OclBuiltinImpl<intel_sub_group_block_write2_4, sub_group_types_block_read_write_scalar, 0,
  [{
  }]>;

OclBuiltinImpl intel_sub_group_block_write4_4_fake = OclBuiltinImpl<intel_sub_group_block_write4_4, sub_group_types_block_read_write_scalar, 0,
  [{
  }]>;

OclBuiltinImpl intel_sub_group_block_write8_4_fake = OclBuiltinImpl<intel_sub_group_block_write8_4, sub_group_types_block_read_write_scalar, 0,
  [{
  }]>;

OclBuiltinImpl intel_sub_group_block_write1_8_fake = OclBuiltinImpl<intel_sub_group_block_write1_8, sub_group_types_block_read_write_scalar, 0,
  [{
  }]>;

OclBuiltinImpl intel_sub_group_block_write2_8_fake = OclBuiltinImpl<intel_sub_group_block_write2_8, sub_group_types_block_read_write_scalar, 0,
  [{
  }]>;

OclBuiltinImpl intel_sub_group_block_write4_8_fake = OclBuiltinImpl<intel_sub_group_block_write4_8, sub_group_types_block_read_write_scalar, 0,
  [{
  }]>;

OclBuiltinImpl intel_sub_group_block_write8_8_fake = OclBuiltinImpl<intel_sub_group_block_write8_8, sub_group_types_block_read_write_scalar, 0,
  [{
  }]>;

OclBuiltinImpl intel_sub_group_block_write1_16_fake = OclBuiltinImpl<intel_sub_group_block_write1_16, sub_group_types_block_read_write_scalar, 0,
  [{
  }]>;

OclBuiltinImpl intel_sub_group_block_write2_16_fake = OclBuiltinImpl<intel_sub_group_block_write2_16, sub_group_types_block_read_write_scalar, 0,
  [{
  }]>;

OclBuiltinImpl intel_sub_group_block_write4_16_fake = OclBuiltinImpl<intel_sub_group_block_write4_16, sub_group_types_block_read_write_scalar, 0,
  [{
  }]>;

OclBuiltinImpl intel_sub_group_block_write8_16_fake = OclBuiltinImpl<intel_sub_group_block_write8_16, sub_group_types_block_read_write_scalar, 0,
  [{
  }]>;
