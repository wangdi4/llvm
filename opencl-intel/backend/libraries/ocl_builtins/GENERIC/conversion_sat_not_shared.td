// vim:ts=2:sw=2:et:

// Conversion between integers will be implemented as cast, sext, zext, trunc
// based on input/output types.
// unsigned-to-unsigned/unsigned-to-signed up conversion with saturation

// convert_sat_shared and convert_sat_not_shared MUST not overlap. i.e. contain 
// the same functions, see NOTE in convert_not_shared.td

// convert_i8_sat
//list<OclType> i8_unsigned_up = [];

list<OclType> i8_s2u_s2s_down_not_shared = [v2i16, v3i16, v4i16, v8i16, v16i16,
                                 v2i32, v3i32, v4i32, v8i32, v16i32];
OclBuiltinImpl s2u_s2s_down_i8_sat_not_shared = OclBuiltinImpl<convert_i8_sat, i8_s2u_s2s_down_not_shared, 0, s2u_s2s_down_sat>;
OclBuiltinImpl dp2signed_down_i8_sat_not_shared = OclBuiltinImpl<convert_i8_sat, [v8f64], 0, dp2signed_down_sat>;
OclBuiltinImpl dp2signed_down_i8_sat_rte_not_shared = OclBuiltinImpl<convert_i8_sat_rte, [v8f64], 0, dp2signed_down_sat_rte>;
OclBuiltinImpl dp2signed_down_i8_sat_rtz_not_shared = OclBuiltinImpl<convert_i8_sat_rtz, [v8f64], 0, dp2signed_down_sat_rtz>;
OclBuiltinImpl dp2signed_down_i8_sat_rtp_not_shared = OclBuiltinImpl<convert_i8_sat_rtp, [v8f64], 0, dp2signed_down_sat_rtp>;
OclBuiltinImpl dp2signed_down_i8_sat_rtn_not_shared = OclBuiltinImpl<convert_i8_sat_rtn, [v8f64], 0, dp2signed_down_sat_rtn>;

// convert_u8
//list<OclType> u8_unsigned_up = [];

list<OclType> u8_s2u_s2s_down_not_shared = [v2i16, v3i16, v4i16, v8i16, v16i16,
                                 v2i32, v3i32, v4i32, v8i32, v16i32];
OclBuiltinImpl s2u_s2s_down_u8_sat_not_shared = OclBuiltinImpl<convert_u8_sat, u8_s2u_s2s_down_not_shared, 0, s2u_s2s_down_sat>;
OclBuiltinImpl dp2signed_down_u8_sat_not_shared = OclBuiltinImpl<convert_u8_sat, [v8f64], 0, dp2unsigned_down_sat>;
OclBuiltinImpl dp2signed_down_u8_sat_rte_not_shared = OclBuiltinImpl<convert_u8_sat_rte, [v8f64], 0, dp2unsigned_down_sat_rte>;
OclBuiltinImpl dp2signed_down_u8_sat_rtz_not_shared = OclBuiltinImpl<convert_u8_sat_rtz, [v8f64], 0, dp2unsigned_down_sat_rtz>;
OclBuiltinImpl dp2signed_down_u8_sat_rtp_not_shared = OclBuiltinImpl<convert_u8_sat_rtp, [v8f64], 0, dp2unsigned_down_sat_rtp>;
OclBuiltinImpl dp2signed_down_u8_sat_rtn_not_shared = OclBuiltinImpl<convert_u8_sat_rtn, [v8f64], 0, dp2unsigned_down_sat_rtn>;

// convert_i16


// convert_u16

OclBuiltinImpl s2u_s2s_down_u16_sat_not_shared = OclBuiltinImpl<convert_u16_sat, [v8i32], 0, s2u_s2s_down_sat>;

// convert_i32
OclBuiltinImpl dp2signed_down_i32_sat_not_shared = OclBuiltinImpl<convert_i32_sat, [v8f64], 0, dp2signed_down_sat>;
OclBuiltinImpl dp2signed_down_i32_sat_rte_not_shared = OclBuiltinImpl<convert_i32_sat_rte, [v8f64], 0, dp2signed_down_sat_rte>;

OclBuiltinImpl dp2signed_down_i32_sat_rtz_not_shared = OclBuiltinImpl<convert_i32_sat_rtz, [v8f64], 0, dp2signed_down_sat_rtz>;
OclBuiltinImpl dp2signed_down_i32_sat_rtp_not_shared = OclBuiltinImpl<convert_i32_sat_rtp, [v8f64], 0, dp2signed_down_sat_rtp>;
OclBuiltinImpl dp2signed_down_i32_sat_rtn_not_shared = OclBuiltinImpl<convert_i32_sat_rtn, [v8f64], 0, dp2signed_down_sat_rtn>;

// convert_u32
OclBuiltinImpl dp2unsigned_down_u32_sat_not_shared = OclBuiltinImpl<convert_u32_sat, [v8f64], 0, dp2unsigned_down_sat>;
OclBuiltinImpl dp2unsigned_down_u32_sat_rte_not_shared = OclBuiltinImpl<convert_u32_sat_rte, [v8f64], 0, dp2unsigned_down_sat_rte>;
OclBuiltinImpl dp2unsigned_down_u32_sat_rtz_not_shared = OclBuiltinImpl<convert_u32_sat_rtz, [v8f64], 0, dp2unsigned_down_sat_rtz>;
OclBuiltinImpl dp2unsigned_down_u32_sat_rtp_not_shared = OclBuiltinImpl<convert_u32_sat_rtp, [v8f64], 0, dp2unsigned_down_sat_rtp>;
OclBuiltinImpl dp2unsigned_down_u32_sat_rtn_not_shared = OclBuiltinImpl<convert_u32_sat_rtn, [v8f64], 0, dp2unsigned_down_sat_rtn>;

// convert_i64
OclBuiltinImpl fp2signed_up_i64_sat_not_shared = OclBuiltinImpl<convert_i64_sat, [v8f32], 0, fp2signed_up_sat>;
OclBuiltinImpl dp2signed_flat_i64_sat_not_shared = OclBuiltinImpl<convert_i64_sat, [v8f64], 0, dp2signed_flat_sat>;
OclBuiltinImpl fp2signed_up_i64_sat_rte_not_shared = OclBuiltinImpl<convert_i64_sat_rte, [v8f32], 0, fp2signed_up_sat_rte>;
OclBuiltinImpl dp2signed_flat_i64_sat_rte_not_shared = OclBuiltinImpl<convert_i64_sat_rte, [v8f64], 0, dp2signed_flat_sat_rte>;
OclBuiltinImpl fp2signed_up_i64_sat_rtz_not_shared = OclBuiltinImpl<convert_i64_sat_rtz, [v8f32], 0, fp2signed_up_sat_rtz>;
OclBuiltinImpl dp2signed_flat_i64_sat_rtz_not_shared = OclBuiltinImpl<convert_i64_sat_rtz, [v8f64], 0, dp2signed_flat_sat_rtz>;
OclBuiltinImpl fp2signed_up_i64_sat_rtp_not_shared = OclBuiltinImpl<convert_i64_sat_rtp, [v8f32], 0, fp2signed_up_sat_rtp>;
OclBuiltinImpl dp2signed_flat_i64_sat_rtp_not_shared = OclBuiltinImpl<convert_i64_sat_rtp, [v8f64], 0, dp2signed_flat_sat_rtp>;
OclBuiltinImpl fp2signed_up_i64_sat_rtn_not_shared = OclBuiltinImpl<convert_i64_sat_rtn, [v8f32], 0, fp2signed_up_sat_rtn>;
OclBuiltinImpl dp2signed_flat_i64_sat_rtn_not_shared = OclBuiltinImpl<convert_i64_sat_rtn, [v8f64], 0, dp2signed_flat_sat_rtn>;

// convert_u64
OclBuiltinImpl fp2unsigned_up_u64_sat_not_shared = OclBuiltinImpl<convert_u64_sat, [v8f32], 0, fp2unsigned_up_sat>;
OclBuiltinImpl dp2unsigned_flat_u64_sat_not_shared = OclBuiltinImpl<convert_u64_sat, [v8f64], 0, dp2unsigned_flat_sat>;
OclBuiltinImpl fp2unsigned_up_u64_sat_rte_not_shared = OclBuiltinImpl<convert_u64_sat_rte, [v8f32], 0, fp2unsigned_up_sat_rte>;
OclBuiltinImpl dp2unsigned_flat_u64_sat_rte_not_shared = OclBuiltinImpl<convert_u64_sat_rte, [v8f64], 0, dp2unsigned_flat_sat_rte>;
OclBuiltinImpl fp2unsigned_up_u64_sat_rtz_not_shared = OclBuiltinImpl<convert_u64_sat_rtz, [v8f32], 0, fp2unsigned_up_sat_rtz>;
OclBuiltinImpl dp2unsigned_flat_u64_sat_rtz_not_shared = OclBuiltinImpl<convert_u64_sat_rtz, [v8f64], 0, dp2unsigned_flat_sat_rtz>;
OclBuiltinImpl fp2unsigned_up_u64_sat_rtp_not_shared = OclBuiltinImpl<convert_u64_sat_rtp, [v8f32], 0, fp2unsigned_up_sat_rtp>;
OclBuiltinImpl dp2unsigned_flat_u64_sat_rtp_not_shared = OclBuiltinImpl<convert_u64_sat_rtp, [v8f64], 0, dp2unsigned_flat_sat_rtp>;
OclBuiltinImpl fp2unsigned_up_u64_sat_rtn_not_shared = OclBuiltinImpl<convert_u64_sat_rtn, [v8f32], 0, fp2unsigned_up_sat_rtn>;
OclBuiltinImpl dp2unsigned_flat_u64_sat_rtn_not_shared = OclBuiltinImpl<convert_u64_sat_rtn, [v8f64], 0, dp2unsigned_flat_sat_rtn>;

