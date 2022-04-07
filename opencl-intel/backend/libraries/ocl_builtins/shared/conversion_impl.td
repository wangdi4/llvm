// int2int conversion with rounding make no sense but we add here for completeness.
foreach type_name = ["i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64"] in {
  foreach sat = ["", "_sat"] in {
    foreach rounding = ["_rte", "_rtz", "_rtp", "_rtn"] in {
      defvar name = "_convert_" # type_name # sat # rounding;
      defvar builtin = !cast<OclBuiltin>(name);
      defvar types_v1_to_v16 = ExpandTypesByVFAndFlatten<
          [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64],
          [1, 2, 3, 4, 8, 16]>.Tout;
      defvar round_code = !cond(!empty(sat): _int2int_rounding, true: _int2int_rounding_sat);
      def name # _impl : OclBuiltinImpl<builtin, types_v1_to_v16, 0, round_code>;
    }
  }
}

// fp2 conversion with rounding make no sense but we add here for completeness.
foreach type_sat = ["f32_sat", "f64_sat"] in {
    foreach rounding = ["", "_rte", "_rtz", "_rtp", "_rtn"] in {
      defvar name = "_convert_" # type_sat # rounding;
      defvar builtin = !cast<OclBuiltin>(name);
      defvar types_v1_to_v16 = ExpandTypesByVFAndFlatten<
          [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64, v1f16, v1f32, v1f64],
          [1, 2, 3, 4, 8, 16]>.Tout;
      defvar round_code = !cond(!eq(rounding, "_rte"): _2fp_sat_rte,
                                !eq(rounding, "_rtz"): _2fp_sat_rtz,
                                !eq(rounding, "_rtp"): _2fp_sat_rtp,
                                !eq(rounding, "_rtn"): _2fp_sat_rtn,
                                true: _2fp_sat);
      def name # _impl : OclBuiltinImpl<builtin, types_v1_to_v16, 0, round_code>;
    }
}
