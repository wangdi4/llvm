list<OclType> sub_group_types_all_any_scalar = [v1i32];

list<OclType> sub_group_types_broadcast_scalar = [v1i32, v1u32, v1i64, v1u64, v1f32, v1f64];

list<OclType> sub_group_types_reduce_scan_scalar = [v1i32, v1u32, v1i64, v1u64, v1f32, v1f64];

list<OclType> sub_group_types_suffle_scalar = [v1i32, v1u32, v1i64, v1u64, v1f32, v1f64, v1i8, v1u8, v1i16, v1u16,
                                         v2i32, v4i32, v8i32, v16i32,
                                         v2u32, v4u32, v8u32, v16u32,
                                         v2f32, v4f32, v8f32, v16f32,
                                         v2i8 , v4i8 , v8i8 , v16i8 ,
                                         v2u8 , v4u8 , v8u8 , v16u8 ,
                                         v2i16, v4i16, v8i16, v16i16,
                                         v2u16, v4u16, v8u16, v16u16,
                                         v2i64, v4i64, v8i64, v16i64,
                                         v2u64, v4u64, v8u64, v16u64];

list<OclType> sub_group_types_block_read_write_scalar = [v1u32];

list<OclType> sub_group_types_block_read_write_scalar_us = [v1u64, v1u32, v1u16, v1u8];


code DummyReturn =
  [{
    return $Arg0VarName;
  }];

//
// Ballot (NEO-like)
//

OclBuiltinImpl intel_sub_group_ballot_igc_fake = OclBuiltinImpl<intel_sub_group_ballot_igc, [v1i32], 0, DummyReturn>;

//
// Ballot
//

OclBuiltinImpl intel_sub_group_ballot_fake = OclBuiltinImpl<intel_sub_group_ballot, sub_group_types_all_any_scalar, 0,
  [{
    return (uint4)$Arg0VarName;
  }]>;

//
// Reductions
//

// any/all

OclBuiltinImpl sub_group_all_fake = OclBuiltinImpl<sub_group_all, sub_group_types_all_any_scalar, 0, DummyReturn>;
OclBuiltinImpl sub_group_any_fake = OclBuiltinImpl<sub_group_any, sub_group_types_all_any_scalar, 0, DummyReturn>;

// broadcast

OclBuiltinImpl sub_group_broadcast_fake = OclBuiltinImpl<sub_group_broadcast, sub_group_types_broadcast_scalar, 0, DummyReturn>;

// reduce [add|min|max]

OclBuiltinImpl sub_group_reduce_add_fake = OclBuiltinImpl<sub_group_reduce_add, sub_group_types_reduce_scan_scalar, 0, DummyReturn>;
OclBuiltinImpl sub_group_reduce_min_fake = OclBuiltinImpl<sub_group_reduce_min, sub_group_types_reduce_scan_scalar, 0, DummyReturn>;
OclBuiltinImpl sub_group_reduce_max_fake = OclBuiltinImpl<sub_group_reduce_max, sub_group_types_reduce_scan_scalar, 0, DummyReturn>;

// exclusive [add|min|max]
OclBuiltinImpl sub_group_scan_exclusive_add_fake = OclBuiltinImpl<sub_group_scan_exclusive_add, sub_group_types_reduce_scan_scalar, 0, DummyReturn>;
OclBuiltinImpl sub_group_scan_exclusive_min_fake = OclBuiltinImpl<sub_group_scan_exclusive_min, sub_group_types_reduce_scan_scalar, 0, DummyReturn>;
OclBuiltinImpl sub_group_scan_exclusive_max_fake = OclBuiltinImpl<sub_group_scan_exclusive_max, sub_group_types_reduce_scan_scalar, 0, DummyReturn>;

// inclusive [add|min|max]

OclBuiltinImpl sub_group_scan_inclusive_add_fake = OclBuiltinImpl<sub_group_scan_inclusive_add, sub_group_types_reduce_scan_scalar, 0, DummyReturn>;
OclBuiltinImpl sub_group_scan_inclusive_min_fake = OclBuiltinImpl<sub_group_scan_inclusive_min, sub_group_types_reduce_scan_scalar, 0, DummyReturn>;
OclBuiltinImpl sub_group_scan_inclusive_max_fake = OclBuiltinImpl<sub_group_scan_inclusive_max, sub_group_types_reduce_scan_scalar, 0, DummyReturn>;

//
// Shuffles
//

OclBuiltinImpl sub_group_shuffle_vec_type_fake = OclBuiltinImpl<sub_group_shuffle, sub_group_types_suffle_scalar, 0, DummyReturn>;
OclBuiltinImpl sub_group_shuffle_xor_vec_type_fake = OclBuiltinImpl<sub_group_shuffle_xor, sub_group_types_suffle_scalar, 0, DummyReturn>;
OclBuiltinImpl sub_group_shuffle_up_vec_type_fake = OclBuiltinImpl<sub_group_shuffle_up, sub_group_types_suffle_scalar, 0, DummyReturn>;
OclBuiltinImpl sub_group_shuffle_down_vec_type_fake = OclBuiltinImpl<sub_group_shuffle_down, sub_group_types_suffle_scalar, 0, DummyReturn>;

///
/// Block read
///

code BlockRead =
  [{
    return *$Arg0VarName;
  }];

code BlockWrite =
  [{
    *$Arg0VarName = $Arg1VarName;
  }];

code BlockWriteS0 =
  [{
    *$Arg0VarName = $Arg1VarName.s0;
  }];


// scalar types

OclBuiltinImpl intel_sub_group_block_read_fake  = OclBuiltinImpl<intel_sub_group_block_read,  sub_group_types_block_read_write_scalar, 0, BlockRead>;
OclBuiltinImpl intel_sub_group_block_read2_fake = OclBuiltinImpl<intel_sub_group_block_read2, sub_group_types_block_read_write_scalar, 0, BlockRead>;
OclBuiltinImpl intel_sub_group_block_read4_fake = OclBuiltinImpl<intel_sub_group_block_read4, sub_group_types_block_read_write_scalar, 0, BlockRead>;
OclBuiltinImpl intel_sub_group_block_read8_fake = OclBuiltinImpl<intel_sub_group_block_read8, sub_group_types_block_read_write_scalar, 0, BlockRead>;

///
/// Block write
///

// scalar types

OclBuiltinImpl intel_sub_group_block_write_fake = OclBuiltinImpl<intel_sub_group_block_write, sub_group_types_block_read_write_scalar, 0, BlockWrite>;

OclBuiltinImpl intel_sub_group_block_write2_fake = OclBuiltinImpl<intel_sub_group_block_write2, sub_group_types_block_read_write_scalar, 0, BlockWriteS0>;
OclBuiltinImpl intel_sub_group_block_write4_fake = OclBuiltinImpl<intel_sub_group_block_write4, sub_group_types_block_read_write_scalar, 0, BlockWriteS0>;
OclBuiltinImpl intel_sub_group_block_write8_fake = OclBuiltinImpl<intel_sub_group_block_write8, sub_group_types_block_read_write_scalar, 0, BlockWriteS0>;

//
// Block read for uchar, ushort, uint, ulong
//
OclBuiltinImpl intel_sub_group_block_read_us_fake  = OclBuiltinImpl<intel_sub_group_block_read_us,  sub_group_types_block_read_write_scalar_us, 0, BlockRead>;
OclBuiltinImpl intel_sub_group_block_read_us2_fake = OclBuiltinImpl<intel_sub_group_block_read_us2, sub_group_types_block_read_write_scalar_us, 0, BlockRead>;
OclBuiltinImpl intel_sub_group_block_read_us4_fake = OclBuiltinImpl<intel_sub_group_block_read_us4, sub_group_types_block_read_write_scalar_us, 0, BlockRead>;
OclBuiltinImpl intel_sub_group_block_read_us8_fake = OclBuiltinImpl<intel_sub_group_block_read_us8, sub_group_types_block_read_write_scalar_us, 0, BlockRead>;

OclBuiltinImpl intel_sub_group_block_write_us_fake  = OclBuiltinImpl<intel_sub_group_block_write_us, sub_group_types_block_read_write_scalar_us, 0, BlockWrite>;

OclBuiltinImpl intel_sub_group_block_write_us2_fake  = OclBuiltinImpl<intel_sub_group_block_write_us2, sub_group_types_block_read_write_scalar_us, 0, BlockWriteS0>;
OclBuiltinImpl intel_sub_group_block_write_us4_fake  = OclBuiltinImpl<intel_sub_group_block_write_us4, sub_group_types_block_read_write_scalar_us, 0, BlockWriteS0>;
OclBuiltinImpl intel_sub_group_block_write_us8_fake  = OclBuiltinImpl<intel_sub_group_block_write_us8, sub_group_types_block_read_write_scalar_us, 0, BlockWriteS0>;

