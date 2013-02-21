// vim:ts=2:sw=2:et:

// Conversions with saturation optimized for MIC

// TODO: replace workarounds with PCG intrinsics once they are ready
// workaround begin
// bultins for the "native" vector types are defined in GENERIC/conversion_sat.td

list<OclType> sat_promote_types = [v1f32, v2f32, v3f32, v4f32, v8f32,
                                   v1f64, v2f64, v3f64, v4f64];
// promote to native vector lentgh
OclBuiltinImpl knc_convert_i8_sat_promote = OclBuiltinImpl<convert_i8_sat, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_i8_sat_rte_promote = OclBuiltinImpl<convert_i8_sat_rte, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_i8_sat_rtz_promote = OclBuiltinImpl<convert_i8_sat_rtz, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_i8_sat_rtp_promote = OclBuiltinImpl<convert_i8_sat_rtp, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_i8_sat_rtn_promote = OclBuiltinImpl<convert_i8_sat_rtn, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_u8_sat_promote = OclBuiltinImpl<convert_u8_sat, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_u8_sat_rte_promote = OclBuiltinImpl<convert_u8_sat_rte, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_u8_sat_rtz_promote = OclBuiltinImpl<convert_u8_sat_rtz, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_u8_sat_rtp_promote = OclBuiltinImpl<convert_u8_sat_rtp, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_u8_sat_rtn_promote = OclBuiltinImpl<convert_u8_sat_rtn, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_i16_sat_promote = OclBuiltinImpl<convert_i16_sat, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_i16_sat_rte_promote = OclBuiltinImpl<convert_i16_sat_rte, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_i16_sat_rtz_promote = OclBuiltinImpl<convert_i16_sat_rtz, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_i16_sat_rtp_promote = OclBuiltinImpl<convert_i16_sat_rtp, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_i16_sat_rtn_promote = OclBuiltinImpl<convert_i16_sat_rtn, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_u16_sat_promote = OclBuiltinImpl<convert_u16_sat, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_u16_sat_rte_promote = OclBuiltinImpl<convert_u16_sat_rte, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_u16_sat_rtz_promote = OclBuiltinImpl<convert_u16_sat_rtz, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_u16_sat_rtp_promote = OclBuiltinImpl<convert_u16_sat_rtp, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_u16_sat_rtn_promote = OclBuiltinImpl<convert_u16_sat_rtn, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_i32_sat_promote = OclBuiltinImpl<convert_i32_sat, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_i32_sat_rte_promote = OclBuiltinImpl<convert_i32_sat_rte, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_i32_sat_rtz_promote = OclBuiltinImpl<convert_i32_sat_rtz, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_i32_sat_rtp_promote = OclBuiltinImpl<convert_i32_sat_rtp, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_i32_sat_rtn_promote = OclBuiltinImpl<convert_i32_sat_rtn, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_u32_sat_promote = OclBuiltinImpl<convert_u32_sat, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_u32_sat_rte_promote = OclBuiltinImpl<convert_u32_sat_rte, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_u32_sat_rtz_promote = OclBuiltinImpl<convert_u32_sat_rtz, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_u32_sat_rtp_promote = OclBuiltinImpl<convert_u32_sat_rtp, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_u32_sat_rtn_promote = OclBuiltinImpl<convert_u32_sat_rtn, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_i64_sat_promote = OclBuiltinImpl<convert_i64_sat, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_i64_sat_rte_promote = OclBuiltinImpl<convert_i64_sat_rte, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_i64_sat_rtz_promote = OclBuiltinImpl<convert_i64_sat_rtz, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_i64_sat_rtp_promote = OclBuiltinImpl<convert_i64_sat_rtp, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_i64_sat_rtn_promote = OclBuiltinImpl<convert_i64_sat_rtn, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_u64_sat_promote = OclBuiltinImpl<convert_u64_sat, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_u64_sat_rte_promote = OclBuiltinImpl<convert_u64_sat_rte, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_u64_sat_rtz_promote = OclBuiltinImpl<convert_u64_sat_rtz, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_u64_sat_rtp_promote = OclBuiltinImpl<convert_u64_sat_rtp, sat_promote_types, 0, NonMaskPromote1>;
OclBuiltinImpl knc_convert_u64_sat_rtn_promote = OclBuiltinImpl<convert_u64_sat_rtn, sat_promote_types, 0, NonMaskPromote1>;

// split too wide vectors on two "native" vectors
OclBuiltinImpl knc_convert_i8_sat_expand = OclBuiltinImpl<convert_i8_sat, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_i8_sat_rte_expand = OclBuiltinImpl<convert_i8_sat_rte, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_i8_sat_rtz_expand = OclBuiltinImpl<convert_i8_sat_rtz, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_i8_sat_rtp_expand = OclBuiltinImpl<convert_i8_sat_rtp, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_i8_sat_rtn_expand = OclBuiltinImpl<convert_i8_sat_rtn, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_u8_sat_expand = OclBuiltinImpl<convert_u8_sat, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_u8_sat_rte_expand = OclBuiltinImpl<convert_u8_sat_rte, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_u8_sat_rtz_expand = OclBuiltinImpl<convert_u8_sat_rtz, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_u8_sat_rtp_expand = OclBuiltinImpl<convert_u8_sat_rtp, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_u8_sat_rtn_expand = OclBuiltinImpl<convert_u8_sat_rtn, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_i16_sat_expand = OclBuiltinImpl<convert_i16_sat, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_i16_sat_rte_expand = OclBuiltinImpl<convert_i16_sat_rte, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_i16_sat_rtz_expand = OclBuiltinImpl<convert_i16_sat_rtz, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_i16_sat_rtp_expand = OclBuiltinImpl<convert_i16_sat_rtp, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_i16_sat_rtn_expand = OclBuiltinImpl<convert_i16_sat_rtn, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_u16_sat_expand = OclBuiltinImpl<convert_u16_sat, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_u16_sat_rte_expand = OclBuiltinImpl<convert_u16_sat_rte, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_u16_sat_rtz_expand = OclBuiltinImpl<convert_u16_sat_rtz, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_u16_sat_rtp_expand = OclBuiltinImpl<convert_u16_sat_rtp, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_u16_sat_rtn_expand = OclBuiltinImpl<convert_u16_sat_rtn, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_i32_sat_expand = OclBuiltinImpl<convert_i32_sat, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_i32_sat_rte_expand = OclBuiltinImpl<convert_i32_sat_rte, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_i32_sat_rtz_expand = OclBuiltinImpl<convert_i32_sat_rtz, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_i32_sat_rtp_expand = OclBuiltinImpl<convert_i32_sat_rtp, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_i32_sat_rtn_expand = OclBuiltinImpl<convert_i32_sat_rtn, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_u32_sat_expand = OclBuiltinImpl<convert_u32_sat, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_u32_sat_rte_expand = OclBuiltinImpl<convert_u32_sat_rte, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_u32_sat_rtz_expand = OclBuiltinImpl<convert_u32_sat_rtz, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_u32_sat_rtp_expand = OclBuiltinImpl<convert_u32_sat_rtp, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_u32_sat_rtn_expand = OclBuiltinImpl<convert_u32_sat_rtn, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_i64_sat_expand = OclBuiltinImpl<convert_i64_sat, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_i64_sat_rte_expand = OclBuiltinImpl<convert_i64_sat_rte, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_i64_sat_rtz_expand = OclBuiltinImpl<convert_i64_sat_rtz, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_i64_sat_rtp_expand = OclBuiltinImpl<convert_i64_sat_rtp, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_i64_sat_rtn_expand = OclBuiltinImpl<convert_i64_sat_rtn, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_u64_sat_expand = OclBuiltinImpl<convert_u64_sat, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_u64_sat_rte_expand = OclBuiltinImpl<convert_u64_sat_rte, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_u64_sat_rtz_expand = OclBuiltinImpl<convert_u64_sat_rtz, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_u64_sat_rtp_expand = OclBuiltinImpl<convert_u64_sat_rtp, [v16u64, v16i64, v16f64], 0, Expand1>;
OclBuiltinImpl knc_convert_u64_sat_rtn_expand = OclBuiltinImpl<convert_u64_sat_rtn, [v16u64, v16i64, v16f64], 0, Expand1>;
// workaround end
