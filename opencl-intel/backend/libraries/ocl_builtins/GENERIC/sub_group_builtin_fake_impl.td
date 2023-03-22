// Copyright (C) 2023 Intel Corporation
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

list<OclType> sub_group_types_all_any_scalar = [v1i32];

list<OclType> sub_group_types_broadcast_scalar = [v1i32, v1u32, v1i64, v1u64, v1f32, v1f64];

list<OclType> sub_group_types_broadcast_extended_scalar = [v2i8, v2u8, v2i16, v2u16, v2i32, v2u32, v2i64, v2u64, v2f32, v2f64,
                                                           v3i8, v3u8, v3i16, v3u16, v3i32, v3u32, v3i64, v3u64, v3f32, v3f64,
                                                           v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4i64, v4u64, v4f32, v4f64,
                                                           v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8i64, v8u64, v8f32, v8f64,
                                                           v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16i64, v16u64, v16f32, v16f64];


list<OclType> sub_group_types_reduce_scan_scalar = [v1i32, v1u32, v1i64, v1u64, v1f32, v1f64];

list<OclType> sub_group_types_suffle_scalar = [v1i32, v1u32, v1i64, v1u64, v1f32, v1f64, v1i8, v1u8, v1i16, v1u16,
                                         v2i32, v3i32, v4i32, v8i32, v16i32,
                                         v2u32, v3u32, v4u32, v8u32, v16u32,
                                         v2f32, v3f32, v4f32, v8f32, v16f32,
                                         v2i8 , v3i8 , v4i8 , v8i8 , v16i8 ,
                                         v2u8 , v3u8 , v4u8 , v8u8 , v16u8 ,
                                         v2i16, v3i16, v4i16, v8i16, v16i16,
                                         v2u16, v3u16, v4u16, v8u16, v16u16,
                                         v2i64, v3i64, v4i64, v8i64, v16i64,
                                         v2u64, v3u64, v4u64, v8u64, v16u64];

list<OclType> sub_group_types_block_read_write_scalar = [v1u32];

list<OclType> sub_group_types_block_read_write_scalar_us = [v1u64, v1u32, v1u16, v1u8];


defvar DummyReturn =
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

// non uniform arithmetic
list<OclType> sub_group_non_uniform_arithmetic_scalar_types = [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f32, v1f64];
list<OclType> sub_group_non_uniform_bitwise_scalar_types = [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64];

OclBuiltinImpl sub_group_non_uniform_reduce_mul_fake = OclBuiltinImpl<sub_group_non_uniform_reduce_mul, sub_group_non_uniform_arithmetic_scalar_types, 0, DummyReturn>;
OclBuiltinImpl sub_group_non_uniform_reduce_logical_and_fake = OclBuiltinImpl<sub_group_non_uniform_reduce_logical_and, [v1i32], 0, DummyReturn>;
OclBuiltinImpl sub_group_non_uniform_reduce_logical_or_fake = OclBuiltinImpl<sub_group_non_uniform_reduce_logical_or, [v1i32], 0, DummyReturn>;
OclBuiltinImpl sub_group_non_uniform_reduce_logical_xor_fake = OclBuiltinImpl<sub_group_non_uniform_reduce_logical_xor, [v1i32], 0, DummyReturn>;
OclBuiltinImpl sub_group_non_uniform_reduce_and_fake = OclBuiltinImpl<sub_group_non_uniform_reduce_and, sub_group_non_uniform_bitwise_scalar_types, 0, DummyReturn>;
OclBuiltinImpl sub_group_non_uniform_reduce_or_fake = OclBuiltinImpl<sub_group_non_uniform_reduce_or, sub_group_non_uniform_bitwise_scalar_types, 0, DummyReturn>;
OclBuiltinImpl sub_group_non_uniform_reduce_xor_fake = OclBuiltinImpl<sub_group_non_uniform_reduce_xor, sub_group_non_uniform_bitwise_scalar_types, 0, DummyReturn>;

OclBuiltinImpl sub_group_non_uniform_scan_inclusive_mul_fake = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_mul, sub_group_non_uniform_arithmetic_scalar_types, 0, DummyReturn>;
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_logical_and_fake = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_logical_and, [v1i32], 0, DummyReturn>;
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_logical_or_fake = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_logical_or, [v1i32], 0, DummyReturn>;
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_logical_xor_fake = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_logical_xor, [v1i32], 0, DummyReturn>;
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_and_fake = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_and, sub_group_non_uniform_bitwise_scalar_types, 0, DummyReturn>;
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_or_fake = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_or, sub_group_non_uniform_bitwise_scalar_types, 0, DummyReturn>;
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_xor_fake = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_xor, sub_group_non_uniform_bitwise_scalar_types, 0, DummyReturn>;

OclBuiltinImpl sub_group_non_uniform_scan_exclusive_mul_fake = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_mul, sub_group_non_uniform_arithmetic_scalar_types, 0, DummyReturn>;
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_logical_and_fake = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_logical_and, [v1i32], 0, DummyReturn>;
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_logical_or_fake = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_logical_or, [v1i32], 0, DummyReturn>;
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_logical_xor_fake = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_logical_xor, [v1i32], 0, DummyReturn>;
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_and_fake = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_and, sub_group_non_uniform_bitwise_scalar_types, 0, DummyReturn>;
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_or_fake = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_or, sub_group_non_uniform_bitwise_scalar_types, 0, DummyReturn>;
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_xor_fake = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_xor, sub_group_non_uniform_bitwise_scalar_types, 0, DummyReturn>;


// reduce [add|min|max]

OclBuiltinImpl sub_group_reduce_add_fake = OclBuiltinImpl<sub_group_reduce_add, sub_group_types_reduce_scan_scalar, 0, DummyReturn>;
OclBuiltinImpl sub_group_reduce_min_fake = OclBuiltinImpl<sub_group_reduce_min, sub_group_types_reduce_scan_scalar, 0, DummyReturn>;
OclBuiltinImpl sub_group_reduce_max_fake = OclBuiltinImpl<sub_group_reduce_max, sub_group_types_reduce_scan_scalar, 0, DummyReturn>;
OclBuiltinImpl sub_group_reduce_add_cs_fake = OclBuiltinImpl<sub_group_reduce_add_cs, [v1i8, v1u8, v1i16, v1u16], 0, DummyReturn>;
OclBuiltinImpl sub_group_reduce_min_cs_fake = OclBuiltinImpl<sub_group_reduce_min_cs, [v1i8, v1u8, v1i16, v1u16], 0, DummyReturn>;
OclBuiltinImpl sub_group_reduce_max_cs_fake = OclBuiltinImpl<sub_group_reduce_max_cs, [v1i8, v1u8, v1i16, v1u16], 0, DummyReturn>;

// exclusive [add|min|max]
OclBuiltinImpl sub_group_scan_exclusive_add_fake = OclBuiltinImpl<sub_group_scan_exclusive_add, sub_group_types_reduce_scan_scalar, 0, DummyReturn>;
OclBuiltinImpl sub_group_scan_exclusive_min_fake = OclBuiltinImpl<sub_group_scan_exclusive_min, sub_group_types_reduce_scan_scalar, 0, DummyReturn>;
OclBuiltinImpl sub_group_scan_exclusive_max_fake = OclBuiltinImpl<sub_group_scan_exclusive_max, sub_group_types_reduce_scan_scalar, 0, DummyReturn>;
OclBuiltinImpl sub_group_scan_exclusive_add_cs_fake = OclBuiltinImpl<sub_group_scan_exclusive_add_cs, [v1i8, v1u8, v1i16, v1u16], 0, DummyReturn>;
OclBuiltinImpl sub_group_scan_exclusive_min_cs_fake = OclBuiltinImpl<sub_group_scan_exclusive_min_cs, [v1i8, v1u8, v1i16, v1u16], 0, DummyReturn>;
OclBuiltinImpl sub_group_scan_exclusive_max_cs_fake = OclBuiltinImpl<sub_group_scan_exclusive_max_cs, [v1i8, v1u8, v1i16, v1u16], 0, DummyReturn>;

// inclusive [add|min|max]

OclBuiltinImpl sub_group_scan_inclusive_add_fake = OclBuiltinImpl<sub_group_scan_inclusive_add, sub_group_types_reduce_scan_scalar, 0, DummyReturn>;
OclBuiltinImpl sub_group_scan_inclusive_min_fake = OclBuiltinImpl<sub_group_scan_inclusive_min, sub_group_types_reduce_scan_scalar, 0, DummyReturn>;
OclBuiltinImpl sub_group_scan_inclusive_max_fake = OclBuiltinImpl<sub_group_scan_inclusive_max, sub_group_types_reduce_scan_scalar, 0, DummyReturn>;
OclBuiltinImpl sub_group_scan_inclusive_add_cs_fake = OclBuiltinImpl<sub_group_scan_inclusive_add_cs, [v1i8, v1u8, v1i16, v1u16], 0, DummyReturn>;
OclBuiltinImpl sub_group_scan_inclusive_min_cs_fake = OclBuiltinImpl<sub_group_scan_inclusive_min_cs, [v1i8, v1u8, v1i16, v1u16], 0, DummyReturn>;
OclBuiltinImpl sub_group_scan_inclusive_max_cs_fake = OclBuiltinImpl<sub_group_scan_inclusive_max_cs, [v1i8, v1u8, v1i16, v1u16], 0, DummyReturn>;

//
// Shuffles
//

OclBuiltinImpl sub_group_shuffle_vec_type_fake = OclBuiltinImpl<sub_group_shuffle, sub_group_types_suffle_scalar, 0, DummyReturn>;
OclBuiltinImpl sub_group_shuffle_xor_vec_type_fake = OclBuiltinImpl<sub_group_shuffle_xor, sub_group_types_suffle_scalar, 0, DummyReturn>;
OclBuiltinImpl sub_group_shuffle_up_vec_type_fake = OclBuiltinImpl<sub_group_shuffle_up, sub_group_types_suffle_scalar, 0, DummyReturn>;
OclBuiltinImpl sub_group_shuffle_down_vec_type_fake = OclBuiltinImpl<sub_group_shuffle_down, sub_group_types_suffle_scalar, 0, DummyReturn>;


OclBuiltinImpl sub_group_shuffle_avx_scalar_type_fake = OclBuiltinImpl<sub_group_shuffle_avx, [v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4f32, v4f64, v4i64, v4u64], 0, DummyReturn>;
OclBuiltinImpl sub_group_shuffle_avx2_scalar_type_fake = OclBuiltinImpl<sub_group_shuffle_avx2, [v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8f32, v8f64, v8i64, v8u64], 0, DummyReturn>;
OclBuiltinImpl sub_group_shuffle_avx512_scalar_type_fake = OclBuiltinImpl<sub_group_shuffle_avx512, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32, v16f64, v16i64, v16u64], 0, DummyReturn>;

OclBuiltinImpl sub_group_shuffle_up_avx_scalar_type_fake = OclBuiltinImpl<sub_group_shuffle_up_avx, [v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4f32, v4f64, v4i64, v4u64], 0, DummyReturn>;
OclBuiltinImpl sub_group_shuffle_up_avx2_scalar_type_fake = OclBuiltinImpl<sub_group_shuffle_up_avx2, [v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8f32, v8f64, v8i64, v8u64], 0, DummyReturn>;
OclBuiltinImpl sub_group_shuffle_up_avx512_scalar_type_fake = OclBuiltinImpl<sub_group_shuffle_up_avx512, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32, v16f64, v16i64, v16u64], 0, DummyReturn>;

OclBuiltinImpl sub_group_shuffle_down_avx_scalar_type_fake = OclBuiltinImpl<sub_group_shuffle_down_avx, [v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4f32, v4f64, v4i64, v4u64], 0, DummyReturn>;
OclBuiltinImpl sub_group_shuffle_down_avx2_scalar_type_fake = OclBuiltinImpl<sub_group_shuffle_down_avx2, [v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8f32, v8f64, v8i64, v8u64], 0, DummyReturn>;
OclBuiltinImpl sub_group_shuffle_down_avx512_scalar_type_fake = OclBuiltinImpl<sub_group_shuffle_down_avx512, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32, v16f64, v16i64, v16u64], 0, DummyReturn>;

OclBuiltinImpl sub_group_shuffle_xor_avx_scalar_type_fake = OclBuiltinImpl<sub_group_shuffle_xor_avx, [v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4f32, v4f64, v4i64, v4u64], 0, DummyReturn>;
OclBuiltinImpl sub_group_shuffle_xor_avx2_scalar_type_fake = OclBuiltinImpl<sub_group_shuffle_xor_avx2, [v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8f32, v8f64, v8i64, v8u64], 0, DummyReturn>;
OclBuiltinImpl sub_group_shuffle_xor_avx512_scalar_type_fake = OclBuiltinImpl<sub_group_shuffle_xor_avx512, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32, v16f64, v16i64, v16u64], 0, DummyReturn>;

///
/// Block read
///

defvar BlockRead =
  [{
    return *$Arg0VarName;
  }];

defvar BlockWrite =
  [{
    *$Arg0VarName = $Arg1VarName;
  }];

defvar BlockWriteS0 =
  [{
    *$Arg0VarName = $Arg1VarName.s0;
  }];

defvar BlockReadImg =
  [{
    $ReturnType* p = ($ReturnType*)extract_pixel_in_byte($Arg0VarName, $Arg1VarName);
    return *p;
  }];

defvar BlockWriteImg =
  [{
    $Arg2Type* p = ($Arg2Type*)extract_pixel_in_byte($Arg0VarName, $Arg1VarName);
    *p = $Arg2VarName;
  }];

// scalar types

OclBuiltinImpl intel_sub_group_block_read_fake  = OclBuiltinImpl<intel_sub_group_block_read,  sub_group_types_block_read_write_scalar, 0, BlockRead>;
OclBuiltinImpl intel_sub_group_block_read2_fake = OclBuiltinImpl<intel_sub_group_block_read2, sub_group_types_block_read_write_scalar, 0, BlockRead>;
OclBuiltinImpl intel_sub_group_block_read4_fake = OclBuiltinImpl<intel_sub_group_block_read4, sub_group_types_block_read_write_scalar, 0, BlockRead>;
OclBuiltinImpl intel_sub_group_block_read8_fake = OclBuiltinImpl<intel_sub_group_block_read8, sub_group_types_block_read_write_scalar, 0, BlockRead>;


OclBuiltinImpl intel_sub_group_block_read_img_fake  = OclBuiltinImpl<intel_sub_group_block_read_img,  sub_group_types_block_read_write_scalar, 0, BlockReadImg>;
OclBuiltinImpl intel_sub_group_block_read2_img_fake = OclBuiltinImpl<intel_sub_group_block_read2_img, sub_group_types_block_read_write_scalar, 0, BlockReadImg>;
OclBuiltinImpl intel_sub_group_block_read4_img_fake = OclBuiltinImpl<intel_sub_group_block_read4_img, sub_group_types_block_read_write_scalar, 0, BlockReadImg>;
OclBuiltinImpl intel_sub_group_block_read8_img_fake = OclBuiltinImpl<intel_sub_group_block_read8_img, sub_group_types_block_read_write_scalar, 0, BlockReadImg>;

OclBuiltinImpl intel_sub_group_block_read_img_rw_fake  = OclBuiltinImpl<intel_sub_group_block_read_img_rw,  sub_group_types_block_read_write_scalar, 0, BlockReadImg>;
OclBuiltinImpl intel_sub_group_block_read2_img_rw_fake = OclBuiltinImpl<intel_sub_group_block_read2_img_rw, sub_group_types_block_read_write_scalar, 0, BlockReadImg>;
OclBuiltinImpl intel_sub_group_block_read4_img_rw_fake = OclBuiltinImpl<intel_sub_group_block_read4_img_rw, sub_group_types_block_read_write_scalar, 0, BlockReadImg>;
OclBuiltinImpl intel_sub_group_block_read8_img_rw_fake = OclBuiltinImpl<intel_sub_group_block_read8_img_rw, sub_group_types_block_read_write_scalar, 0, BlockReadImg>;
///
/// Block write
///

// scalar types

OclBuiltinImpl intel_sub_group_block_write_fake = OclBuiltinImpl<intel_sub_group_block_write, sub_group_types_block_read_write_scalar, 0, BlockWrite>;
OclBuiltinImpl intel_sub_group_block_write2_fake = OclBuiltinImpl<intel_sub_group_block_write2, sub_group_types_block_read_write_scalar, 0, BlockWriteS0>;
OclBuiltinImpl intel_sub_group_block_write4_fake = OclBuiltinImpl<intel_sub_group_block_write4, sub_group_types_block_read_write_scalar, 0, BlockWriteS0>;
OclBuiltinImpl intel_sub_group_block_write8_fake = OclBuiltinImpl<intel_sub_group_block_write8, sub_group_types_block_read_write_scalar, 0, BlockWriteS0>;

OclBuiltinImpl intel_sub_group_block_write_img_fake = OclBuiltinImpl<intel_sub_group_block_write_img, sub_group_types_block_read_write_scalar, 0, BlockWriteImg>;
OclBuiltinImpl intel_sub_group_block_write2_img_fake = OclBuiltinImpl<intel_sub_group_block_write2_img, sub_group_types_block_read_write_scalar, 0, BlockWriteImg>;
OclBuiltinImpl intel_sub_group_block_write4_img_fake = OclBuiltinImpl<intel_sub_group_block_write4_img, sub_group_types_block_read_write_scalar, 0, BlockWriteImg>;
OclBuiltinImpl intel_sub_group_block_write8_img_fake = OclBuiltinImpl<intel_sub_group_block_write8_img, sub_group_types_block_read_write_scalar, 0, BlockWriteImg>;

OclBuiltinImpl intel_sub_group_block_write_img_rw_fake = OclBuiltinImpl<intel_sub_group_block_write_img_rw, sub_group_types_block_read_write_scalar, 0, BlockWriteImg>;
OclBuiltinImpl intel_sub_group_block_write2_img_rw_fake = OclBuiltinImpl<intel_sub_group_block_write2_img_rw, sub_group_types_block_read_write_scalar, 0, BlockWriteImg>;
OclBuiltinImpl intel_sub_group_block_write4_img_rw_fake = OclBuiltinImpl<intel_sub_group_block_write4_img_rw, sub_group_types_block_read_write_scalar, 0, BlockWriteImg>;
OclBuiltinImpl intel_sub_group_block_write8_img_rw_fake = OclBuiltinImpl<intel_sub_group_block_write8_img_rw, sub_group_types_block_read_write_scalar, 0, BlockWriteImg>;
//
// Block read for uchar, ushort, uint, ulong
//
OclBuiltinImpl intel_sub_group_block_read_us_fake  = OclBuiltinImpl<intel_sub_group_block_read_us,  sub_group_types_block_read_write_scalar_us, 0, BlockRead>;
OclBuiltinImpl intel_sub_group_block_read_us2_fake = OclBuiltinImpl<intel_sub_group_block_read_us2, sub_group_types_block_read_write_scalar_us, 0, BlockRead>;
OclBuiltinImpl intel_sub_group_block_read_us4_fake = OclBuiltinImpl<intel_sub_group_block_read_us4, sub_group_types_block_read_write_scalar_us, 0, BlockRead>;
OclBuiltinImpl intel_sub_group_block_read_us8_fake = OclBuiltinImpl<intel_sub_group_block_read_us8, sub_group_types_block_read_write_scalar_us, 0, BlockRead>;
OclBuiltinImpl intel_sub_group_block_read_us16_fake = OclBuiltinImpl<intel_sub_group_block_read_us16, [v1u8], 0, BlockRead>;

OclBuiltinImpl intel_sub_group_block_write_us_fake  = OclBuiltinImpl<intel_sub_group_block_write_us, sub_group_types_block_read_write_scalar_us, 0, BlockWrite>;
OclBuiltinImpl intel_sub_group_block_write_us2_fake  = OclBuiltinImpl<intel_sub_group_block_write_us2, sub_group_types_block_read_write_scalar_us, 0, BlockWriteS0>;
OclBuiltinImpl intel_sub_group_block_write_us4_fake  = OclBuiltinImpl<intel_sub_group_block_write_us4, sub_group_types_block_read_write_scalar_us, 0, BlockWriteS0>;
OclBuiltinImpl intel_sub_group_block_write_us8_fake  = OclBuiltinImpl<intel_sub_group_block_write_us8, sub_group_types_block_read_write_scalar_us, 0, BlockWriteS0>;
OclBuiltinImpl intel_sub_group_block_write_us16_fake  = OclBuiltinImpl<intel_sub_group_block_write_us16, [v1u8], 0, BlockWriteS0>;

OclBuiltinImpl intel_sub_group_block_read_us_img_fake  = OclBuiltinImpl<intel_sub_group_block_read_us_img, sub_group_types_block_read_write_scalar_us, 0, BlockReadImg>;
OclBuiltinImpl intel_sub_group_block_read_us2_img_fake = OclBuiltinImpl<intel_sub_group_block_read_us2_img, sub_group_types_block_read_write_scalar_us, 0, BlockReadImg>;
OclBuiltinImpl intel_sub_group_block_read_us4_img_fake = OclBuiltinImpl<intel_sub_group_block_read_us4_img, sub_group_types_block_read_write_scalar_us, 0, BlockReadImg>;
OclBuiltinImpl intel_sub_group_block_read_us8_img_fake = OclBuiltinImpl<intel_sub_group_block_read_us8_img, sub_group_types_block_read_write_scalar_us, 0, BlockReadImg>;
OclBuiltinImpl intel_sub_group_block_read_us16_img_fake = OclBuiltinImpl<intel_sub_group_block_read_us16_img, [v1u8], 0, BlockReadImg>;
OclBuiltinImpl intel_sub_group_block_write_us_img_fake  = OclBuiltinImpl<intel_sub_group_block_write_us_img, sub_group_types_block_read_write_scalar_us, 0, BlockWriteImg>;
OclBuiltinImpl intel_sub_group_block_write_us2_img_fake = OclBuiltinImpl<intel_sub_group_block_write_us2_img, sub_group_types_block_read_write_scalar_us, 0, BlockWriteImg>;
OclBuiltinImpl intel_sub_group_block_write_us4_img_fake = OclBuiltinImpl<intel_sub_group_block_write_us4_img, sub_group_types_block_read_write_scalar_us, 0, BlockWriteImg>;
OclBuiltinImpl intel_sub_group_block_write_us8_img_fake = OclBuiltinImpl<intel_sub_group_block_write_us8_img, sub_group_types_block_read_write_scalar_us, 0, BlockWriteImg>;
OclBuiltinImpl intel_sub_group_block_write_us16_img_fake = OclBuiltinImpl<intel_sub_group_block_write_us16_img, [v1u8], 0, BlockWriteImg>;

OclBuiltinImpl intel_sub_group_block_read_us_img_rw_fake  = OclBuiltinImpl<intel_sub_group_block_read_us_img_rw, sub_group_types_block_read_write_scalar_us, 0, BlockReadImg>;
OclBuiltinImpl intel_sub_group_block_read_us2_img_rw_fake = OclBuiltinImpl<intel_sub_group_block_read_us2_img_rw, sub_group_types_block_read_write_scalar_us, 0, BlockReadImg>;
OclBuiltinImpl intel_sub_group_block_read_us4_img_rw_fake = OclBuiltinImpl<intel_sub_group_block_read_us4_img_rw, sub_group_types_block_read_write_scalar_us, 0, BlockReadImg>;
OclBuiltinImpl intel_sub_group_block_read_us8_img_rw_fake = OclBuiltinImpl<intel_sub_group_block_read_us8_img_rw, sub_group_types_block_read_write_scalar_us, 0, BlockReadImg>;
OclBuiltinImpl intel_sub_group_block_read_us16_img_rw_fake = OclBuiltinImpl<intel_sub_group_block_read_us16_img_rw, [v1u8], 0, BlockReadImg>;
OclBuiltinImpl intel_sub_group_block_write_us_img_rw_fake  = OclBuiltinImpl<intel_sub_group_block_write_us_img_rw, sub_group_types_block_read_write_scalar_us, 0, BlockWriteImg>;
OclBuiltinImpl intel_sub_group_block_write_us2_img_rw_fake = OclBuiltinImpl<intel_sub_group_block_write_us2_img_rw, sub_group_types_block_read_write_scalar_us, 0, BlockWriteImg>;
OclBuiltinImpl intel_sub_group_block_write_us4_img_rw_fake = OclBuiltinImpl<intel_sub_group_block_write_us4_img_rw, sub_group_types_block_read_write_scalar_us, 0, BlockWriteImg>;
OclBuiltinImpl intel_sub_group_block_write_us8_img_rw_fake = OclBuiltinImpl<intel_sub_group_block_write_us8_img_rw, sub_group_types_block_read_write_scalar_us, 0, BlockWriteImg>;
OclBuiltinImpl intel_sub_group_block_write_us16_img_rw_fake = OclBuiltinImpl<intel_sub_group_block_write_us16_img_rw, [v1u8], 0, BlockWriteImg>;
