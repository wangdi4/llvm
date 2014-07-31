// vim:ts=2:sw=2:et:

// NOTE:
// this is generic implementations for conversion functions which can not
// be common for all platforms, so functions collected here has some 
// differences between particular realisations, for example:

// built-in convert_i32_rte has no specific code for data types
// v1f32, v2f32, v3f32, v4f32, v8f32, v1f64, v2f64, v3f64, v4f64, v16f64
// and realisation for these data types is placed in shared library and in
// another file: conversion_shared.td

// but the code for convert_i32_rte8 and convert_i32_rtef16 has some 
// differences between ssse3 and sse42 realisations, so built-ins 
// for data types v8f64 and v16f32 exist here

// convert_shared and convert_not_shared MUST not overlap. i.e. contain 
// the same functions

OclBuiltinImpl fp2int_i32_f32_rte_not_shared = OclBuiltinImpl<convert_i32_rte,
                                                   [v16f32,
                                                    v8f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi32$rtnnosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_i32_f32_rtz_not_shared = OclBuiltinImpl<convert_i32_rtz,
                                                   [v8f32, v8f64, v16f32,
                                                    v3f64, v4f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi32$rtznosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_i32_f32_rtp_not_shared = OclBuiltinImpl<convert_i32_rtp,
                                                   [v16f32, v8f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi32$upnosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_i32_f32_rtn_not_shared = OclBuiltinImpl<convert_i32_rtn,
                                                   [v16f32, v8f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi32$downnosat$SVMLSuffix($Arg0VarName);
  }]>;



OclBuiltinImpl fp2int_u32_f32_rte_not_shared = OclBuiltinImpl<convert_u32_rte,
                                                   [v16f32, v8f64], 0, [{
    return __ocl_svml_$Target_cvtfptou32$rtnnosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_u32_f32_rtz_not_shared = OclBuiltinImpl<convert_u32_rtz,
                                                   [v8f32, v8f64, v16f32], 0, [{
    return __ocl_svml_$Target_cvtfptou32$rtznosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_u32_f32_rtp_not_shared = OclBuiltinImpl<convert_u32_rtp,
                                                   [v16f32,  v8f64], 0, [{
    return __ocl_svml_$Target_cvtfptou32$upnosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_u32_f32_rtn_not_shared = OclBuiltinImpl<convert_u32_rtn,
                                                   [v16f32, v8f64], 0, [{
    return __ocl_svml_$Target_cvtfptou32$downnosat$SVMLSuffix($Arg0VarName);
  }]>;

// 64-bit integer output


OclBuiltinImpl fp2int_i64_f32_not_shared = OclBuiltinImpl<convert_i64,
                                                   [v16f32, v8f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi64$rtznosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_i64_f32_rte_not_shared = OclBuiltinImpl<convert_i64_rte,
                                                   [ v16f32, v8f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi64$rtnnosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_i64_f32_rtz_not_shared = OclBuiltinImpl<convert_i64_rtz,
                                                   [v16f32, v8f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi64$rtznosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_i64_f32_rtp_not_shared = OclBuiltinImpl<convert_i64_rtp,
                                                   [v16f32, v8f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi64$upnosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_i64_f32_rtn_not_shared = OclBuiltinImpl<convert_i64_rtn,
                                                   [v16f32, v8f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi64$downnosat$SVMLSuffix($Arg0VarName);
  }]>;


OclBuiltinImpl fp2int_u64_f32_not_shared = OclBuiltinImpl<convert_u64,
                                               [v16f32, v8f64], 0, [{
    return  __ocl_svml_$Target_cvtfptou64$rtznosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_u64_f32_rte_not_shared = OclBuiltinImpl<convert_u64_rte,
                                                   [v16f32, v8f64], 0, [{
    return  __ocl_svml_$Target_cvtfptou64$rtnnosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_u64_f32_rtz_not_shared = OclBuiltinImpl<convert_u64_rtz,
                                                   [v16f32, v8f64], 0, [{
    return  __ocl_svml_$Target_cvtfptou64$rtznosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_u64_f32_rtp_not_shared = OclBuiltinImpl<convert_u64_rtp,
                                                   [v16f32, v8f64], 0, [{
    return  __ocl_svml_$Target_cvtfptou64$upnosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_u64_f32_rtn_not_shared = OclBuiltinImpl<convert_u64_rtn,
                                                   [v16f32, v8f64], 0, [{
    return  __ocl_svml_$Target_cvtfptou64$downnosat$SVMLSuffix($Arg0VarName);
  }]>;

// 32-bit floating output



OclBuiltinImpl cvt_f32_f64_not_shared = OclBuiltinImpl<convert_f32,
                                          [v4f64], 0, [{
    return __ocl_svml_$Target_cvtfp64tofp32$rtn$SVMLSuffix($Arg0VarName);
  }]>;


OclBuiltinImpl cvt_f32_i32_not_shared = OclBuiltinImpl<convert_f32,
                                            [v8i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;



OclBuiltinImpl cvt_f32_i64_not_shared = OclBuiltinImpl<convert_f32,
                                            [v8i64, v16i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtn$SVMLFSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl cvt_f32_u64_not_shared = OclBuiltinImpl<convert_f32,
                                            [v8u64, v16u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtn$SVMLFSuffix($Arg0VarName);
  }]>;



OclBuiltinImpl rtz_cvt_f32_f64_not_shared = OclBuiltinImpl<convert_f32_rtz,
                                                [v8f64], 0, [{
    return __ocl_svml_$Target_cvtfp64tofp32$rtz$SVMLSuffix($Arg0VarName);
  }]>;



OclBuiltinImpl rtz_cvt_f32_i32_not_shared = OclBuiltinImpl<convert_f32_rtz,
                                                [v16i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$rtz$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtz_cvt_f32_u32_not_shared = OclBuiltinImpl<convert_f32_rtz,
                                                [v16u32], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$rtz$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtz_cvt_f32_i64_not_shared = OclBuiltinImpl<convert_f32_rtz,
                                                [v8i64, v16i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtz$SVMLFSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtz_cvt_f32_u64_not_shared = OclBuiltinImpl<convert_f32_rtz,
                                                [v8u64, v16u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtz$SVMLFSuffix($Arg0VarName);
  }]>;



OclBuiltinImpl rtp_cvt_f32_f64_not_shared = OclBuiltinImpl<convert_f32_rtp,
                                                [v8f64], 0, [{
    return __ocl_svml_$Target_cvtfp64tofp32$up$SVMLSuffix($Arg0VarName);
  }]>;



OclBuiltinImpl rtp_cvt_f32_i32_not_shared = OclBuiltinImpl<convert_f32_rtp,
                                                [v16i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$up$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtp_cvt_f32_u32_not_shared = OclBuiltinImpl<convert_f32_rtp,
                                                [v16u32], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$up$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtp_cvt_f32_i64_not_shared = OclBuiltinImpl<convert_f32_rtp,
                                                [v8i64, v16i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$up$SVMLFSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtp_cvt_f32_u64_not_shared = OclBuiltinImpl<convert_f32_rtp,
                                                [v8u64, v16u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$up$SVMLFSuffix($Arg0VarName);
  }]>;


OclBuiltinImpl rtn_cvt_f32_f64_not_shared = OclBuiltinImpl<convert_f32_rtn,
                                                [v8f64], 0, [{
    return __ocl_svml_$Target_cvtfp64tofp32$down$SVMLSuffix($Arg0VarName);
  }]>;





OclBuiltinImpl rtn_cvt_f32_i32_not_shared = OclBuiltinImpl<convert_f32_rtn,
                                                [v16i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$down$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtn_cvt_f32_u32_not_shared = OclBuiltinImpl<convert_f32_rtn,
                                                [v16u32], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$down$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtn_cvt_f32_i64_not_shared = OclBuiltinImpl<convert_f32_rtn,
                                                [v8i64, v16i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$down$SVMLFSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtn_cvt_f32_u64_not_shared = OclBuiltinImpl<convert_f32_rtn,
                                                [v8u64, v16u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$down$SVMLFSuffix($Arg0VarName);
  }]>;

// 64-bit floating output

// cvt_f64_f32
// conversion from f32 to f64 is easily handled by target insn alone and there
// is no difference among rounding modes.
// |fS1|fE8|fM23| -> |dS1|dE11|dM52|
// where
// - dS   := fS1
// - dE11 := fE8 + 127 - 1023
// - dM52 := fM23[22:0], pad[28:0], where pad = 0.
//



OclBuiltinImpl cvt_f64_i8i16i32_not_shared = OclBuiltinImpl<convert_f64,
                                                 [ v3i32, v4i32, v8i32], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtn$VecLength(
            as_$Arg0i64gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl cvt_f32_u8u16u32_not_shared = OclBuiltinImpl<convert_f64,
                                                 [v8u32], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtn$VecLength(
            as_$Arg0u64gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl cvt_f64_i64_not_shared = OclBuiltinImpl<convert_f64,
                                            [v8i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl cvt_f64_u64 = OclBuiltinImpl<convert_f64,
                                            [v8u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;



OclBuiltinImpl rte_cvt_f64_i8i16i32_not_shared = OclBuiltinImpl<convert_f64_rte,
                                                     [v8i32], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtn$VecLength(
            as_$Arg0i64gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rte_cvt_f32_u8u16u32_not_shared = OclBuiltinImpl<convert_f64_rte,
                                                     [v8u32], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtn$VecLength(
            as_$Arg0u64gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rte_cvt_f64_i64_not_shared = OclBuiltinImpl<convert_f64_rte,
                                                [v8i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rte_cvt_f64_u64_not_shared = OclBuiltinImpl<convert_f64_rte,
                                                [v8u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;


OclBuiltinImpl rtz_cvt_f64_i8i16i32_not_shared = OclBuiltinImpl<convert_f64_rtz,
                                                     [v8i32], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtz$VecLength(
            as_$Arg0i64gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtz_cvt_f32_u8u16u32_not_shared = OclBuiltinImpl<convert_f64_rtz,
                                                     [v8u32], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtz$VecLength(
            as_$Arg0u64gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtz_cvt_f64_i64_not_shared = OclBuiltinImpl<convert_f64_rtz,
                                                [v8i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtz$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtz_cvt_f64_u64_not_shared = OclBuiltinImpl<convert_f64_rtz,
                                                [v8u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtz$SVMLSuffix($Arg0VarName);
  }]>;



OclBuiltinImpl rtp_cvt_f64_i8i16i32_not_shared = OclBuiltinImpl<convert_f64_rtp,
                                                     [v8i32], 0, [{
    return __ocl_svml_$Target_cvti64tofp$up$VecLength(
            as_$Arg0i64gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtp_cvt_f32_u8u16u32_not_shared = OclBuiltinImpl<convert_f64_rtp,
                                                     [v8u32], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$up$VecLength(
            as_$Arg0u64gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtp_cvt_f64_i64_not_shared = OclBuiltinImpl<convert_f64_rtp,
                                                [v8i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$up$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtp_cvt_f64_u64_not_shared = OclBuiltinImpl<convert_f64_rtp,
                                                [v8u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$up$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtn_cvt_f64_i8i16i32_not_shared = OclBuiltinImpl<convert_f64_rtn,
                                                     [v8i32], 0, [{
    return __ocl_svml_$Target_cvti64tofp$down$VecLength(
            as_$Arg0i64gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtn_cvt_f32_u8u16u32_not_shared = OclBuiltinImpl<convert_f64_rtn,
                                                     [v8u32], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$down$VecLength(
            as_$Arg0u64gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtn_cvt_f64_i64_not_shared = OclBuiltinImpl<convert_f64_rtn,
                                                [v8i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$down$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtn_cvt_f64_u64_not_shared = OclBuiltinImpl<convert_f64_rtn,
                                                [v8u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$down$SVMLSuffix($Arg0VarName);
  }]>;

// int2int conversion with rounding make no sense but we add here for completeness.
//
