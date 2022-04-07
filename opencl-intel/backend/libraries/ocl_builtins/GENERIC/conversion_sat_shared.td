// vim:ts=2:sw=2:et:

// Conversion between integers will be implemented as cast, sext, zext, trunc
// based on input/output types.
// unsigned-to-unsigned/unsigned-to-signed up conversion with saturation

// convert_sat_shared and convert_sat_not_shared MUST not overlap. i.e. contain 
// the same functions, see NOTE in convert_shared.td

// fp types
list<OclType> f16_types = [v1f16, v2f16, v3f16, v4f16, v8f16, v16f16];
list<OclType> f32_types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32];
list<OclType> f64_types = [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64];
list<OclType> f64_types_no_v8 = [v1f64, v2f64, v3f64, v4f64, v16f64];
list<OclType> f32_types_no_v8 = [v1f32, v2f32, v3f32, v4f32, v16f32];

// convert_i8_sat
//list<OclType> i8_unsigned_up = [];
list<OclType> i8_same = [v1i8, v2i8, v3i8, v4i8, v8i8, v16i8];
list<OclType> i8_flat = [v1u8, v2u8, v3u8, v4u8, v8u8, v16u8];
list<OclType> i8_s2u_s2s_down = [v1i16, v1i32,
                                 v1i64, v2i64, v3i64, v4i64, v8i64, v16i64];
list<OclType> i8_u2u_u2s_down = [v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                                 v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
                                 v1u64, v2u64, v3u64, v4u64, v8u64, v16u64];
OclBuiltinImpl same_i8_sat = OclBuiltinImpl<convert_i8_sat, i8_same, 0, convertSameType>;
OclBuiltinImpl flat_i8_sat = OclBuiltinImpl<convert_i8_sat, i8_flat, 0, u2s_flat_sat>;
OclBuiltinImpl s2u_s2s_down_i8_sat = OclBuiltinImpl<convert_i8_sat, i8_s2u_s2s_down, 0, s2u_s2s_down_sat>;
OclBuiltinImpl u2u_u2s_down_i8_sat = OclBuiltinImpl<convert_i8_sat, i8_u2u_u2s_down, 0, u2s_u2u_down_sat>;
OclBuiltinImpl hp2signed_down_i8_sat = OclBuiltinImpl<convert_i8_sat, f16_types, 0, fp16_sat>;
OclBuiltinImpl fp2signed_down_i8_sat = OclBuiltinImpl<convert_i8_sat, f32_types, 0, fp2signed_down_sat>;
OclBuiltinImpl dp2signed_down_i8_sat = OclBuiltinImpl<convert_i8_sat, f64_types_no_v8, 0, dp2signed_down_sat>;
OclBuiltinImpl hp2signed_down_i8_sat_rte = OclBuiltinImpl<convert_i8_sat_rte, f16_types, 0, fp16_sat_rte>;
OclBuiltinImpl fp2signed_down_i8_sat_rte = OclBuiltinImpl<convert_i8_sat_rte, f32_types, 0, fp2signed_down_sat_rte>;
OclBuiltinImpl dp2signed_down_i8_sat_rte = OclBuiltinImpl<convert_i8_sat_rte, f64_types_no_v8, 0, dp2signed_down_sat_rte>;
OclBuiltinImpl hp2signed_down_i8_sat_rtz = OclBuiltinImpl<convert_i8_sat_rtz, f16_types, 0, fp16_sat_rtz>;
OclBuiltinImpl fp2signed_down_i8_sat_rtz = OclBuiltinImpl<convert_i8_sat_rtz, f32_types, 0, fp2signed_down_sat_rtz>;
OclBuiltinImpl dp2signed_down_i8_sat_rtz = OclBuiltinImpl<convert_i8_sat_rtz, f64_types_no_v8, 0, dp2signed_down_sat_rtz>;
OclBuiltinImpl hp2signed_down_i8_sat_rtp = OclBuiltinImpl<convert_i8_sat_rtp, f16_types, 0, fp16_sat_rtp>;
OclBuiltinImpl fp2signed_down_i8_sat_rtp = OclBuiltinImpl<convert_i8_sat_rtp, f32_types, 0, fp2signed_down_sat_rtp>;
OclBuiltinImpl dp2signed_down_i8_sat_rtp = OclBuiltinImpl<convert_i8_sat_rtp, f64_types_no_v8, 0, dp2signed_down_sat_rtp>;
OclBuiltinImpl hp2signed_down_i8_sat_rtn = OclBuiltinImpl<convert_i8_sat_rtn, f16_types, 0, fp16_sat_rtn>;
OclBuiltinImpl fp2signed_down_i8_sat_rtn = OclBuiltinImpl<convert_i8_sat_rtn, f32_types, 0, fp2signed_down_sat_rtn>;
OclBuiltinImpl dp2signed_down_i8_sat_rtn = OclBuiltinImpl<convert_i8_sat_rtn, f64_types_no_v8, 0, dp2signed_down_sat_rtn>;

// convert_u8
//list<OclType> u8_unsigned_up = [];
list<OclType> u8_same = [v1u8, v2u8, v3u8, v4u8, v8u8, v16u8];
list<OclType> u8_flat = [v1i8, v2i8, v3i8, v4i8, v8i8, v16i8];
list<OclType> u8_u2s_u2u_down = [v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                                 v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
                                 v1u64, v2u64, v3u64, v4u64, v8u64, v16u64];
list<OclType> u8_s2u_s2s_down = [v1i16, v1i32,
                                 v1i64, v2i64, v3i64, v4i64, v8i64, v16i64];
OclBuiltinImpl same_u8_sat = OclBuiltinImpl<convert_u8_sat, u8_same, 0, convertSameType>;
OclBuiltinImpl flat_u8_sat = OclBuiltinImpl<convert_u8_sat, u8_flat, 0, s2u_flat_sat>;
OclBuiltinImpl u2s_u2u_down_u8_sat = OclBuiltinImpl<convert_u8_sat, u8_u2s_u2u_down, 0, u2s_u2u_down_sat>;
OclBuiltinImpl s2u_s2s_down_u8_sat = OclBuiltinImpl<convert_u8_sat, u8_s2u_s2s_down, 0, s2u_s2s_down_sat>;
OclBuiltinImpl hp2signed_down_u8_sat = OclBuiltinImpl<convert_u8_sat, f16_types, 0, fp16_sat>;
OclBuiltinImpl fp2signed_down_u8_sat = OclBuiltinImpl<convert_u8_sat, f32_types, 0, fp2unsigned_down_sat>;
OclBuiltinImpl dp2signed_down_u8_sat = OclBuiltinImpl<convert_u8_sat, f64_types_no_v8, 0, dp2unsigned_down_sat>;
OclBuiltinImpl hp2signed_down_u8_sat_rte = OclBuiltinImpl<convert_u8_sat_rte, f16_types, 0, fp16_sat_rte>;
OclBuiltinImpl fp2signed_down_u8_sat_rte = OclBuiltinImpl<convert_u8_sat_rte, f32_types, 0, fp2unsigned_down_sat_rte>;
OclBuiltinImpl dp2signed_down_u8_sat_rte = OclBuiltinImpl<convert_u8_sat_rte, f64_types_no_v8, 0, dp2unsigned_down_sat_rte>;
OclBuiltinImpl hp2signed_down_u8_sat_rtz = OclBuiltinImpl<convert_u8_sat_rtz, f16_types, 0, fp16_sat_rtz>;
OclBuiltinImpl fp2signed_down_u8_sat_rtz = OclBuiltinImpl<convert_u8_sat_rtz, f32_types, 0, fp2unsigned_down_sat_rtz>;
OclBuiltinImpl dp2signed_down_u8_sat_rtz = OclBuiltinImpl<convert_u8_sat_rtz, f64_types_no_v8, 0, dp2unsigned_down_sat_rtz>;
OclBuiltinImpl hp2signed_down_u8_sat_rtp = OclBuiltinImpl<convert_u8_sat_rtp, f16_types, 0, fp16_sat_rtp>;
OclBuiltinImpl fp2signed_down_u8_sat_rtp = OclBuiltinImpl<convert_u8_sat_rtp, f32_types, 0, fp2unsigned_down_sat_rtp>;
OclBuiltinImpl dp2signed_down_u8_sat_rtp = OclBuiltinImpl<convert_u8_sat_rtp, f64_types_no_v8, 0, dp2unsigned_down_sat_rtp>;
OclBuiltinImpl hp2signed_down_u8_sat_rtn = OclBuiltinImpl<convert_u8_sat_rtn, f16_types, 0, fp16_sat_rtn>;
OclBuiltinImpl fp2signed_down_u8_sat_rtn = OclBuiltinImpl<convert_u8_sat_rtn, f32_types, 0, fp2unsigned_down_sat_rtn>;
OclBuiltinImpl dp2signed_down_u8_sat_rtn = OclBuiltinImpl<convert_u8_sat_rtn, f64_types_no_v8, 0, dp2unsigned_down_sat_rtn>;

// convert_i16
list<OclType> i16_unsigned_up = [v1u8, v2u8, v3u8, v4u8, v8u8, v16u8];
list<OclType> i16_same = [v1i16, v2i16, v3i16, v4i16, v8i16, v16i16];
list<OclType> i16_flat = [v1u16, v2u16, v3u16, v4u16, v8u16, v16u16];
list<OclType> i16_s2s_up = [v1i8, v2i8, v3i8, v4i8, v8i8, v16i8];
list<OclType> i16_u2u_u2s_down = [v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
                                  v1u64, v2u64, v3u64, v4u64, v8u64, v16u64];
list<OclType> i16_s2u_s2s_down = [v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
                                  v1i64, v2i64, v3i64, v4i64, v8i64, v16i64];
OclBuiltinImpl unsigned_up_i16_sat = OclBuiltinImpl<convert_i16_sat, i16_unsigned_up, 0, unsigned_up_sat>;
OclBuiltinImpl same_i16_sat = OclBuiltinImpl<convert_i16_sat, i16_same, 0, convertSameType>;
OclBuiltinImpl flat_i16_sat = OclBuiltinImpl<convert_i16_sat, i16_flat, 0, u2s_flat_sat>;
OclBuiltinImpl s2s_up_i16_sat = OclBuiltinImpl<convert_i16_sat, i16_s2s_up, 0, s2s_up_sat>;
OclBuiltinImpl u2u_u2s_down_i16_sat = OclBuiltinImpl<convert_i16_sat, i16_u2u_u2s_down, 0, u2s_u2u_down_sat>;
OclBuiltinImpl s2u_s2s_down_i16_sat = OclBuiltinImpl<convert_i16_sat, i16_s2u_s2s_down, 0, s2u_s2s_down_sat>;
OclBuiltinImpl hp2signed_down_i16_sat = OclBuiltinImpl<convert_i16_sat, f16_types, 0, fp16_sat>;
OclBuiltinImpl fp2signed_down_i16_sat = OclBuiltinImpl<convert_i16_sat, f32_types, 0, fp2signed_down_sat>;
OclBuiltinImpl dp2signed_down_i16_sat = OclBuiltinImpl<convert_i16_sat, f64_types, 0, dp2signed_down_sat>;
OclBuiltinImpl hp2signed_down_i16_sat_rte = OclBuiltinImpl<convert_i16_sat_rte, f16_types, 0, fp16_sat_rte>;
OclBuiltinImpl fp2signed_down_i16_sat_rte = OclBuiltinImpl<convert_i16_sat_rte, f32_types, 0, fp2signed_down_sat_rte>;
OclBuiltinImpl dp2signed_down_i16_sat_rte = OclBuiltinImpl<convert_i16_sat_rte, f64_types, 0, dp2signed_down_sat_rte>;
OclBuiltinImpl hp2signed_down_i16_sat_rtz = OclBuiltinImpl<convert_i16_sat_rtz, f16_types, 0, fp16_sat_rtz>;
OclBuiltinImpl fp2signed_down_i16_sat_rtz = OclBuiltinImpl<convert_i16_sat_rtz, f32_types, 0, fp2signed_down_sat_rtz>;
OclBuiltinImpl dp2signed_down_i16_sat_rtz = OclBuiltinImpl<convert_i16_sat_rtz, f64_types, 0, dp2signed_down_sat_rtz>;
OclBuiltinImpl hp2signed_down_i16_sat_rtp = OclBuiltinImpl<convert_i16_sat_rtp, f16_types, 0, fp16_sat_rtp>;
OclBuiltinImpl fp2signed_down_i16_sat_rtp = OclBuiltinImpl<convert_i16_sat_rtp, f32_types, 0, fp2signed_down_sat_rtp>;
OclBuiltinImpl dp2signed_down_i16_sat_rtp = OclBuiltinImpl<convert_i16_sat_rtp, f64_types, 0, dp2signed_down_sat_rtp>;
OclBuiltinImpl hp2signed_down_i16_sat_rtn = OclBuiltinImpl<convert_i16_sat_rtn, f16_types, 0, fp16_sat_rtn>;
OclBuiltinImpl fp2signed_down_i16_sat_rtn = OclBuiltinImpl<convert_i16_sat_rtn, f32_types, 0, fp2signed_down_sat_rtn>;
OclBuiltinImpl dp2signed_down_i16_sat_rtn = OclBuiltinImpl<convert_i16_sat_rtn, f64_types, 0, dp2signed_down_sat_rtn>;

// convert_u16
list<OclType> u16_unsigned_up = [v1u8, v2u8, v3u8, v4u8, v8u8, v16u8];
list<OclType> u16_same = [v1u16, v2u16, v3u16, v4u16, v8u16, v16u16];
list<OclType> u16_flat = [v1i16, v2i16, v3i16, v4i16, v8i16, v16i16];
list<OclType> u16_s2u_up = [v1i8, v2i8, v3i8, v4i8, v8i8, v16i8];
list<OclType> u16_u2s_u2u_down = [v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
                                  v1u64, v2u64, v3u64, v4u64, v8u64, v16u64];
list<OclType> u16_s2u_s2s_down = [v1i32, v2i32, v3i32, v4i32, v16i32,
                                  v1i64, v2i64, v3i64, v4i64, v8i64, v16i64];
OclBuiltinImpl unsigned_up_u16_sat = OclBuiltinImpl<convert_u16_sat, u16_unsigned_up, 0, unsigned_up_sat>;
OclBuiltinImpl same_u16_sat = OclBuiltinImpl<convert_u16_sat, u16_same, 0, convertSameType>;
OclBuiltinImpl flat_u16_sat = OclBuiltinImpl<convert_u16_sat, u16_flat, 0, s2u_flat_sat>;
OclBuiltinImpl s2u_up_u16_sat = OclBuiltinImpl<convert_u16_sat, u16_s2u_up, 0, s2u_up_sat>;
OclBuiltinImpl u2u_u2s_down_u16_sat = OclBuiltinImpl<convert_u16_sat, u16_u2s_u2u_down, 0, u2s_u2u_down_sat>;
OclBuiltinImpl s2u_s2s_down_u16_sat = OclBuiltinImpl<convert_u16_sat, u16_s2u_s2s_down, 0, s2u_s2s_down_sat>;
OclBuiltinImpl hp2unsigned_down_u16_sat = OclBuiltinImpl<convert_u16_sat, f16_types, 0, fp16_sat>;
OclBuiltinImpl fp2unsigned_down_u16_sat = OclBuiltinImpl<convert_u16_sat, f32_types, 0, fp2unsigned_down_sat>;
OclBuiltinImpl dp2unsigned_down_u16_sat = OclBuiltinImpl<convert_u16_sat, f64_types, 0, dp2unsigned_down_sat>;
OclBuiltinImpl hp2unsigned_down_u16_sat_rte = OclBuiltinImpl<convert_u16_sat_rte, f16_types, 0, fp16_sat_rte>;
OclBuiltinImpl fp2unsigned_down_u16_sat_rte = OclBuiltinImpl<convert_u16_sat_rte, f32_types, 0, fp2unsigned_down_sat_rte>;
OclBuiltinImpl dp2unsigned_down_u16_sat_rte = OclBuiltinImpl<convert_u16_sat_rte, f64_types, 0, dp2unsigned_down_sat_rte>;
OclBuiltinImpl hp2unsigned_down_u16_sat_rtz = OclBuiltinImpl<convert_u16_sat_rtz, f16_types, 0, fp16_sat_rtz>;
OclBuiltinImpl fp2unsigned_down_u16_sat_rtz = OclBuiltinImpl<convert_u16_sat_rtz, f32_types, 0, fp2unsigned_down_sat_rtz>;
OclBuiltinImpl dp2unsigned_down_u16_sat_rtz = OclBuiltinImpl<convert_u16_sat_rtz, f64_types, 0, dp2unsigned_down_sat_rtz>;
OclBuiltinImpl hp2unsigned_down_u16_sat_rtp = OclBuiltinImpl<convert_u16_sat_rtp, f16_types, 0, fp16_sat_rtp>;
OclBuiltinImpl fp2unsigned_down_u16_sat_rtp = OclBuiltinImpl<convert_u16_sat_rtp, f32_types, 0, fp2unsigned_down_sat_rtp>;
OclBuiltinImpl dp2unsigned_down_u16_sat_rtp = OclBuiltinImpl<convert_u16_sat_rtp, f64_types, 0, dp2unsigned_down_sat_rtp>;
OclBuiltinImpl hp2unsigned_down_u16_sat_rtn = OclBuiltinImpl<convert_u16_sat_rtn, f16_types, 0, fp16_sat_rtn>;
OclBuiltinImpl fp2unsigned_down_u16_sat_rtn = OclBuiltinImpl<convert_u16_sat_rtn, f32_types, 0, fp2unsigned_down_sat_rtn>;
OclBuiltinImpl dp2unsigned_down_u16_sat_rtn = OclBuiltinImpl<convert_u16_sat_rtn, f64_types, 0, dp2unsigned_down_sat_rtn>;

// convert_i32
list<OclType> i32_unsigned_up = [v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                 v1u16, v2u16, v3u16, v4u16, v8u16, v16u16];
list<OclType> i32_same = [v1i32, v2i32, v3i32, v4i32, v8i32, v16i32];
list<OclType> i32_flat = [v1u32, v2u32, v3u32, v4u32, v8u32, v16u32];
list<OclType> i32_s2s_up = [v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                            v1i16, v2i16, v3i16, v4i16, v8i16, v16i16];
list<OclType> i32_u2u_u2s_down = [v1u64, v2u64, v3u64, v4u64, v8u64, v16u64];
list<OclType> i32_s2u_s2s_down = [v1i64, v2i64, v3i64, v4i64, v8i64, v16i64];
OclBuiltinImpl unsigned_up_i32_sat = OclBuiltinImpl<convert_i32_sat, i32_unsigned_up, 0, unsigned_up_sat>;
OclBuiltinImpl same_i32_sat = OclBuiltinImpl<convert_i32_sat, i32_same, 0, convertSameType>;
OclBuiltinImpl flat_i32_sat = OclBuiltinImpl<convert_i32_sat, i32_flat, 0, u2s_flat_sat>;
OclBuiltinImpl s2s_up_i32_sat = OclBuiltinImpl<convert_i32_sat, i32_s2s_up, 0, s2s_up_sat>;
OclBuiltinImpl u2u_u2s_down_i32_sat = OclBuiltinImpl<convert_i32_sat, i32_u2u_u2s_down, 0, u2s_u2u_down_sat>;
OclBuiltinImpl s2u_s2s_down_i32_sat = OclBuiltinImpl<convert_i32_sat, i32_s2u_s2s_down, 0, s2u_s2s_down_sat>;
OclBuiltinImpl hp2signed_flat_i32_sat = OclBuiltinImpl<convert_i32_sat, f16_types, 0, fp16_sat>;
OclBuiltinImpl fp2signed_flat_i32_sat = OclBuiltinImpl<convert_i32_sat, f32_types, 0, fp2signed_flat_sat>;
OclBuiltinImpl dp2signed_down_i32_sat = OclBuiltinImpl<convert_i32_sat, f64_types_no_v8, 0, dp2signed_down_sat>;
OclBuiltinImpl hp2signed_flat_i32_sat_rte = OclBuiltinImpl<convert_i32_sat_rte, f16_types, 0, fp16_sat_rte>;
OclBuiltinImpl fp2signed_flat_i32_sat_rte = OclBuiltinImpl<convert_i32_sat_rte, f32_types, 0, fp2signed_flat_sat_rte>;
OclBuiltinImpl dp2signed_down_i32_sat_rte = OclBuiltinImpl<convert_i32_sat_rte, f64_types_no_v8, 0, dp2signed_down_sat_rte>;
OclBuiltinImpl hp2signed_flat_i32_sat_rtz = OclBuiltinImpl<convert_i32_sat_rtz, f16_types, 0, fp16_sat_rtz>;
OclBuiltinImpl fp2signed_flat_i32_sat_rtz = OclBuiltinImpl<convert_i32_sat_rtz, f32_types, 0, fp2signed_flat_sat_rtz>;
OclBuiltinImpl dp2signed_down_i32_sat_rtz = OclBuiltinImpl<convert_i32_sat_rtz, f64_types_no_v8, 0, dp2signed_down_sat_rtz>;
OclBuiltinImpl hp2signed_flat_i32_sat_rtp = OclBuiltinImpl<convert_i32_sat_rtp, f16_types, 0, fp16_sat_rtp>;
OclBuiltinImpl fp2signed_flat_i32_sat_rtp = OclBuiltinImpl<convert_i32_sat_rtp, f32_types, 0, fp2signed_flat_sat_rtp>;
OclBuiltinImpl dp2signed_down_i32_sat_rtp = OclBuiltinImpl<convert_i32_sat_rtp, f64_types_no_v8, 0, dp2signed_down_sat_rtp>;
OclBuiltinImpl hp2signed_flat_i32_sat_rtn = OclBuiltinImpl<convert_i32_sat_rtn, f16_types, 0, fp16_sat_rtn>;
OclBuiltinImpl fp2signed_flat_i32_sat_rtn = OclBuiltinImpl<convert_i32_sat_rtn, f32_types, 0, fp2signed_flat_sat_rtn>;
OclBuiltinImpl dp2signed_down_i32_sat_rtn = OclBuiltinImpl<convert_i32_sat_rtn, f64_types_no_v8, 0, dp2signed_down_sat_rtn>;

// convert_u32
list<OclType> u32_unsigned_up = [v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                 v1u16, v2u16, v3u16, v4u16, v8u16, v16u16];
list<OclType> u32_same = [v1u32, v2u32, v3u32, v4u32, v8u32, v16u32];
list<OclType> u32_flat = [v1i32, v2i32, v3i32, v4i32, v8i32, v16i32];
list<OclType> u32_s2u_up = [v1i8, v2i8, v3i8, v4i8, v8i8, v16i8,
                            v1i16, v2i16, v3i16, v4i16, v8i16, v16i16];
list<OclType> u32_u2s_u2u_down = [v1u64, v2u64, v3u64, v4u64, v8u64, v16u64];
list<OclType> u32_s2u_s2s_down = [v1i64, v2i64, v3i64, v4i64, v8i64, v16i64];
OclBuiltinImpl unsigned_up_u32_sat = OclBuiltinImpl<convert_u32_sat, u32_unsigned_up, 0, unsigned_up_sat>;
OclBuiltinImpl same_u32_sat = OclBuiltinImpl<convert_u32_sat, u32_same, 0, convertSameType>;
OclBuiltinImpl flat_u32_sat = OclBuiltinImpl<convert_u32_sat, u32_flat, 0, s2u_flat_sat>;
OclBuiltinImpl s2u_up_u32_sat = OclBuiltinImpl<convert_u32_sat, u32_s2u_up, 0, s2u_up_sat>;
OclBuiltinImpl u2s_u2u_down_u32_sat = OclBuiltinImpl<convert_u32_sat, u32_u2s_u2u_down, 0, u2s_u2u_down_sat>;
OclBuiltinImpl s2u_s2s_down_u32_sat = OclBuiltinImpl<convert_u32_sat, u32_s2u_s2s_down, 0, s2u_s2s_down_sat>;
OclBuiltinImpl hp2unsigned_flat_u32_sat = OclBuiltinImpl<convert_u32_sat, f16_types, 0, fp16_sat>;
OclBuiltinImpl fp2unsigned_flat_u32_sat = OclBuiltinImpl<convert_u32_sat, f32_types, 0, fp2unsigned_flat_sat>;
OclBuiltinImpl dp2unsigned_down_u32_sat = OclBuiltinImpl<convert_u32_sat, f64_types_no_v8, 0, dp2unsigned_down_sat>;
OclBuiltinImpl hp2unsigned_flat_u32_sat_rte = OclBuiltinImpl<convert_u32_sat_rte, f16_types, 0, fp16_sat_rte>;
OclBuiltinImpl fp2unsigned_flat_u32_sat_rte = OclBuiltinImpl<convert_u32_sat_rte, f32_types, 0, fp2unsigned_flat_sat_rte>;
OclBuiltinImpl dp2unsigned_down_u32_sat_rte = OclBuiltinImpl<convert_u32_sat_rte, f64_types_no_v8, 0, dp2unsigned_down_sat_rte>;
OclBuiltinImpl hp2unsigned_flat_u32_sat_rtz = OclBuiltinImpl<convert_u32_sat_rtz, f16_types, 0, fp16_sat_rtz>;
OclBuiltinImpl fp2unsigned_flat_u32_sat_rtz = OclBuiltinImpl<convert_u32_sat_rtz, f32_types, 0, fp2unsigned_flat_sat_rtz>;
OclBuiltinImpl dp2unsigned_down_u32_sat_rtz = OclBuiltinImpl<convert_u32_sat_rtz, f64_types_no_v8, 0, dp2unsigned_down_sat_rtz>;
OclBuiltinImpl hp2unsigned_flat_u32_sat_rtp = OclBuiltinImpl<convert_u32_sat_rtp, f16_types, 0, fp16_sat_rtp>;
OclBuiltinImpl fp2unsigned_flat_u32_sat_rtp = OclBuiltinImpl<convert_u32_sat_rtp, f32_types, 0, fp2unsigned_flat_sat_rtp>;
OclBuiltinImpl dp2unsigned_down_u32_sat_rtp = OclBuiltinImpl<convert_u32_sat_rtp, f64_types_no_v8, 0, dp2unsigned_down_sat_rtp>;
OclBuiltinImpl hp2unsigned_flat_u32_sat_rtn = OclBuiltinImpl<convert_u32_sat_rtn, f16_types, 0, fp16_sat_rtn>;
OclBuiltinImpl fp2unsigned_flat_u32_sat_rtn = OclBuiltinImpl<convert_u32_sat_rtn, f32_types, 0, fp2unsigned_flat_sat_rtn>;
OclBuiltinImpl dp2unsigned_down_u32_sat_rtn = OclBuiltinImpl<convert_u32_sat_rtn, f64_types_no_v8, 0, dp2unsigned_down_sat_rtn>;

// convert_i64
list<OclType> i64_unsigned_up = [v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                 v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                                 v1u32, v2u32, v3u32, v4u32, v8u32, v16u32];
list<OclType> i64_same = [v1i64, v2i64, v3i64, v4i64, v8i64, v16i64];
list<OclType> i64_flat = [v1u64, v2u64, v3u64, v4u64, v8u64, v16u64];
list<OclType> i64_s2s_up = [v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                            v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                            v1i32, v2i32, v3i32, v4i32, v8i32, v16i32];
OclBuiltinImpl unsigned_up_i64_sat = OclBuiltinImpl<convert_i64_sat, i64_unsigned_up, 0, unsigned_up_sat>;
OclBuiltinImpl same_i64_sat = OclBuiltinImpl<convert_i64_sat, i64_same, 0, convertSameType>;
OclBuiltinImpl flat_i64_sat = OclBuiltinImpl<convert_i64_sat, i64_flat, 0, u2s_flat_sat>;
OclBuiltinImpl s2s_up_i64_sat = OclBuiltinImpl<convert_i64_sat, i64_s2s_up, 0, s2s_up_sat>;
OclBuiltinImpl hp2signed_up_i64_sat = OclBuiltinImpl<convert_i64_sat, f16_types, 0, fp16_sat>;
OclBuiltinImpl fp2signed_up_i64_sat = OclBuiltinImpl<convert_i64_sat, f32_types_no_v8, 0, fp2signed_up_sat>;
OclBuiltinImpl dp2signed_flat_i64_sat = OclBuiltinImpl<convert_i64_sat, f64_types_no_v8, 0, dp2signed_flat_sat>;
OclBuiltinImpl hp2signed_up_i64_sat_rte = OclBuiltinImpl<convert_i64_sat_rte, f16_types, 0, fp16_sat_rte>;
OclBuiltinImpl fp2signed_up_i64_sat_rte = OclBuiltinImpl<convert_i64_sat_rte, f32_types_no_v8, 0, fp2signed_up_sat_rte>;
OclBuiltinImpl dp2signed_flat_i64_sat_rte = OclBuiltinImpl<convert_i64_sat_rte, f64_types_no_v8, 0, dp2signed_flat_sat_rte>;
OclBuiltinImpl hp2signed_up_i64_sat_rtz = OclBuiltinImpl<convert_i64_sat_rtz, f16_types, 0, fp16_sat_rtz>;
OclBuiltinImpl fp2signed_up_i64_sat_rtz = OclBuiltinImpl<convert_i64_sat_rtz, f32_types_no_v8, 0, fp2signed_up_sat_rtz>;
OclBuiltinImpl dp2signed_flat_i64_sat_rtz = OclBuiltinImpl<convert_i64_sat_rtz, f64_types_no_v8, 0, dp2signed_flat_sat_rtz>;
OclBuiltinImpl hp2signed_up_i64_sat_rtp = OclBuiltinImpl<convert_i64_sat_rtp, f16_types, 0, fp16_sat_rtp>;
OclBuiltinImpl fp2signed_up_i64_sat_rtp = OclBuiltinImpl<convert_i64_sat_rtp, f32_types_no_v8, 0, fp2signed_up_sat_rtp>;
OclBuiltinImpl dp2signed_flat_i64_sat_rtp = OclBuiltinImpl<convert_i64_sat_rtp, f64_types_no_v8, 0, dp2signed_flat_sat_rtp>;
OclBuiltinImpl hp2signed_up_i64_sat_rtn = OclBuiltinImpl<convert_i64_sat_rtn, f16_types, 0, fp16_sat_rtn>;
OclBuiltinImpl fp2signed_up_i64_sat_rtn = OclBuiltinImpl<convert_i64_sat_rtn, f32_types_no_v8, 0, fp2signed_up_sat_rtn>;
OclBuiltinImpl dp2signed_flat_i64_sat_rtn = OclBuiltinImpl<convert_i64_sat_rtn, f64_types_no_v8, 0, dp2signed_flat_sat_rtn>;

// convert_u64
list<OclType> u64_unsigned_up = [v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                 v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                                 v1u32, v2u32, v3u32, v4u32, v8u32, v16u32];
list<OclType> u64_same = [v1u64, v2u64, v3u64, v4u64, v8u64, v16u64];
list<OclType> u64_flat = [v1i64, v2i64, v3i64, v4i64, v8i64, v16i64];
list<OclType> u64_s2u_up = [v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                            v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                            v1i32, v2i32, v3i32, v4i32, v8i32, v16i32];
OclBuiltinImpl unsigned_up_u64_sat = OclBuiltinImpl<convert_u64_sat, u64_unsigned_up, 0, unsigned_up_sat>;
OclBuiltinImpl same_u64_sat = OclBuiltinImpl<convert_u64_sat, u64_same, 0, convertSameType>;
OclBuiltinImpl flat_u64_sat = OclBuiltinImpl<convert_u64_sat, u64_flat, 0, s2u_flat_sat>;
OclBuiltinImpl s2u_up_u64_sat = OclBuiltinImpl<convert_u64_sat, u64_s2u_up, 0, s2u_up_sat>;
OclBuiltinImpl hp2unsigned_up_u64_sat = OclBuiltinImpl<convert_u64_sat, f16_types, 0, fp16_sat>;
OclBuiltinImpl fp2unsigned_up_u64_sat = OclBuiltinImpl<convert_u64_sat, f32_types_no_v8, 0, fp2unsigned_up_sat>;
OclBuiltinImpl dp2unsigned_flat_u64_sat = OclBuiltinImpl<convert_u64_sat, f64_types_no_v8, 0, dp2unsigned_flat_sat>;
OclBuiltinImpl hp2unsigned_up_u64_sat_rte = OclBuiltinImpl<convert_u64_sat_rte, f16_types, 0, fp16_sat_rte>;
OclBuiltinImpl fp2unsigned_up_u64_sat_rte = OclBuiltinImpl<convert_u64_sat_rte, f32_types_no_v8, 0, fp2unsigned_up_sat_rte>;
OclBuiltinImpl dp2unsigned_flat_u64_sat_rte = OclBuiltinImpl<convert_u64_sat_rte, f64_types_no_v8, 0, dp2unsigned_flat_sat_rte>;
OclBuiltinImpl hp2unsigned_up_u64_sat_rtz = OclBuiltinImpl<convert_u64_sat_rtz, f16_types, 0, fp16_sat_rtz>;
OclBuiltinImpl fp2unsigned_up_u64_sat_rtz = OclBuiltinImpl<convert_u64_sat_rtz, f32_types_no_v8, 0, fp2unsigned_up_sat_rtz>;
OclBuiltinImpl dp2unsigned_flat_u64_sat_rtz = OclBuiltinImpl<convert_u64_sat_rtz, f64_types_no_v8, 0, dp2unsigned_flat_sat_rtz>;
OclBuiltinImpl hp2unsigned_up_u64_sat_rtp = OclBuiltinImpl<convert_u64_sat_rtp, f16_types, 0, fp16_sat_rtp>;
OclBuiltinImpl fp2unsigned_up_u64_sat_rtp = OclBuiltinImpl<convert_u64_sat_rtp, f32_types_no_v8, 0, fp2unsigned_up_sat_rtp>;
OclBuiltinImpl dp2unsigned_flat_u64_sat_rtp = OclBuiltinImpl<convert_u64_sat_rtp, f64_types_no_v8, 0, dp2unsigned_flat_sat_rtp>;
OclBuiltinImpl hp2unsigned_up_u64_sat_rtn = OclBuiltinImpl<convert_u64_sat_rtn, f16_types, 0, fp16_sat_rtn>;
OclBuiltinImpl fp2unsigned_up_u64_sat_rtn = OclBuiltinImpl<convert_u64_sat_rtn, f32_types_no_v8, 0, fp2unsigned_up_sat_rtn>;
OclBuiltinImpl dp2unsigned_flat_u64_sat_rtn = OclBuiltinImpl<convert_u64_sat_rtn, f64_types_no_v8, 0, dp2unsigned_flat_sat_rtn>;

