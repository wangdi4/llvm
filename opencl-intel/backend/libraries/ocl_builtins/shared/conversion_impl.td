// Copyright (C) 2022 Intel Corporation
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

// int2int conversion with rounding make no sense but we add here for completeness.
// Not sure if v32 and v64 versions are needed.
foreach type_name = ["i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64"] in {
  foreach sat = ["", "_sat"] in {
    foreach rounding = ["_rte", "_rtz", "_rtp", "_rtn"] in {
      defvar name = "_convert_" # type_name # sat # rounding;
      defvar builtin = !cast<OclBuiltin>(name);
      defvar types_v1_to_v16 = ExpandTypesByVFAndFlatten<
          [v1i8, v1u8, v1i16, v1u16, v1i32, v1u32, v1i64, v1u64],
          [1, 2, 3, 4, 8, 16]>.Tout;
      defvar round_code = !if(!empty(sat), _int2int_rounding, _int2int_rounding_sat);
      def name # _impl : OclBuiltinImpl<builtin, types_v1_to_v16, 0, round_code>;
    }
  }
}

// fp2 conversion with rounding make no sense but we add here for completeness.
// Not sure if v32 and v64 versions are needed.
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

foreach type_name = ["i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64"] in {
  defvar name = "convert_" # type_name;
  defvar builtin = !cast<OclBuiltin>(name);
  foreach len = [1, 2, 3, 4, 8, 16, 32, 64] in {
    defvar dst_type = !cast<OclType>("v"#len#type_name);
    foreach src_type_name = ["i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64"] in {
      defvar src_type = !cast<OclType>("v"#len#src_type_name);
      defvar cvt_code = !cond(!eq(type_name, src_type_name): NoCvt,
                              !eq(dst_type.BitWidth, src_type.BitWidth): BitCast,
                              !lt(dst_type.BitWidth, src_type.BitWidth): BitTrunc,
                              !and(!gt(dst_type.BitWidth, src_type.BitWidth), !ne(!find(src_type_name, "i"), -1)): BitSExt,
                              true: BitZExt);
      def : OclBuiltinImpl<builtin, [src_type], 0, cvt_code>;
    }

    foreach src_type_name = ["f16"] in {
      defvar src_type = !cast<OclType>("v"#len#src_type_name);
      def : OclBuiltinImpl<builtin, [src_type], 0, PromoteF16F32, [["$Round",""]]>;
      def : OclBuiltinImpl<!cast<OclBuiltin>(name#"_rte"), [src_type], 0, PromoteF16F32, [["$Round","_rte"]]>;
      def : OclBuiltinImpl<!cast<OclBuiltin>(name#"_rtz"), [src_type], 0, PromoteF16F32, [["$Round","_rtz"]]>;
      def : OclBuiltinImpl<!cast<OclBuiltin>(name#"_rtp"), [src_type], 0, PromoteF16F32, [["$Round","_rtp"]]>;
      def : OclBuiltinImpl<!cast<OclBuiltin>(name#"_rtn"), [src_type], 0, PromoteF16F32, [["$Round","_rtn"]]>;
    }
  } // foreach len
}

foreach type_name = ["i8", "u8", "i16", "u16"] in {
  defvar name = "convert_" # type_name;
  defvar cvt_code = !cond(!eq(type_name, "u16"): PromoteI32_ForU16,
                          !ne(!find(type_name, "u"), -1): PromoteU32,
                          true: PromoteI32);
  foreach len = [1, 2, 3, 4, 8, 16] in {
    foreach src_type_name = ["f32", "f64"] in {
      defvar src_type = !cast<OclType>("v"#len#src_type_name);
      def : OclBuiltinImpl<!cast<OclBuiltin>(name), [src_type], 0, cvt_code, [["$Round",""]]>;
      def : OclBuiltinImpl<!cast<OclBuiltin>(name#"_rte"), [src_type], 0, cvt_code, [["$Round","_rte"]]>;
      def : OclBuiltinImpl<!cast<OclBuiltin>(name#"_rtz"), [src_type], 0, cvt_code, [["$Round","_rtz"]]>;
      def : OclBuiltinImpl<!cast<OclBuiltin>(name#"_rtp"), [src_type], 0, cvt_code, [["$Round","_rtp"]]>;
      def : OclBuiltinImpl<!cast<OclBuiltin>(name#"_rtn"), [src_type], 0, cvt_code, [["$Round","_rtn"]]>;
    }
  }
}

foreach type_name = ["i64", "u64"] in {
  defvar name = "convert_" # type_name;
  defvar cvt_code = !if(!eq(type_name, "i64"), FPToI64, FPToU64);
  foreach src_type_name = ["f32", "f64"] in {
    defvar lens = !if(!eq(src_type_name, "f32"), [1, 4, 8], [1, 4]);
    foreach len = lens in {
      defvar src_type = !cast<OclType>("v"#len#src_type_name);
      def : OclBuiltinImpl<!cast<OclBuiltin>(name), [src_type], 0, cvt_code, [["$Round","$rtznosat"]]>;
      def : OclBuiltinImpl<!cast<OclBuiltin>(name#"_rte"), [src_type], 0, cvt_code, [["$Round","$rtnnosat"]]>;
      def : OclBuiltinImpl<!cast<OclBuiltin>(name#"_rtz"), [src_type], 0, cvt_code, [["$Round","$rtznosat"]]>;
      def : OclBuiltinImpl<!cast<OclBuiltin>(name#"_rtp"), [src_type], 0, cvt_code, [["$Round","$upnosat"]]>;
      def : OclBuiltinImpl<!cast<OclBuiltin>(name#"_rtn"), [src_type], 0, cvt_code, [["$Round","$downnosat"]]>;
    }
  }
}

foreach type_name = ["f16", "f32", "f64"] in {
  defvar name = "convert_" # type_name;
  defvar src_type_names = !if(!eq(type_name, "f16"),
      ["i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64", "f16", "f32", "f64"],
      ["f16"]);
  foreach src_type_name = src_type_names in {
    defvar cvt_code = !cond(!ne(type_name, "f16"): ExtFp,
                            !eq(src_type_name, "f16"): NoCvt,
                            !ne(!find(src_type_name, "f"), -1): TruncF16,
                            true: CvtF32TruncF16);
    foreach len = [1, 2, 3, 4, 8, 16, 32, 64] in {
      defvar src_type = !cast<OclType>("v"#len#src_type_name);
      def : OclBuiltinImpl<!cast<OclBuiltin>(name), [src_type], 0, cvt_code>;
      def : OclBuiltinImpl<!cast<OclBuiltin>(name#"_rte"), [src_type], 0, cvt_code>;
      def : OclBuiltinImpl<!cast<OclBuiltin>(name#"_rtz"), [src_type], 0, cvt_code>;
      def : OclBuiltinImpl<!cast<OclBuiltin>(name#"_rtp"), [src_type], 0, cvt_code>;
      def : OclBuiltinImpl<!cast<OclBuiltin>(name#"_rtn"), [src_type], 0, cvt_code>;
    }
  }
}

// Expand

foreach type_name = ["i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64", "f32", "f64"] in {
  defvar name = "convert_" # type_name;
  foreach src_type_name = ["i8", "u8", "i16", "u16", "i32", "u32", "i64", "u64", "f32", "f64"] in {
    foreach len = [32, 64] in {
      defvar src_type = !cast<OclType>("v"#len#src_type_name);
      def : OclBuiltinImpl<!cast<OclBuiltin>(name), [src_type], 0, Expand1>;
      def : OclBuiltinImpl<!cast<OclBuiltin>(name#"_rte"), [src_type], 0, Expand1>;
      def : OclBuiltinImpl<!cast<OclBuiltin>(name#"_rtz"), [src_type], 0, Expand1>;
      def : OclBuiltinImpl<!cast<OclBuiltin>(name#"_rtp"), [src_type], 0, Expand1>;
      def : OclBuiltinImpl<!cast<OclBuiltin>(name#"_rtn"), [src_type], 0, Expand1>;
    }
  }
}
