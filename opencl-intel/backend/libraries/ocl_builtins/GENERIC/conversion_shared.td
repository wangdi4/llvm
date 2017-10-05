// vim:ts=2:sw=2:et:

// Conversion between integers will be implemented as cast, sext, zext, trunc
// based on input/output types.


// NOTE:
// this is generic implementations for conversion functions common for all 
// platforms, so functions collected here has no optimisation for any 
// particular architecture with data types used in this file, for example:

// built-in convert_i32_rte has no specific code for data types
// v1f32, v2f32, v3f32, v4f32, v8f32, v1f64, v2f64, v3f64, v4f64, v16f64
// and realisation for these data types is placed in shared library

// but the code for convert_i32_rte8 and convert_i32_rtef16 has some 
// differences between ssse3 and sse42 realisations, so built-ins 
// for data types v8f64 and v16f32 exist in another file: conversion_not_shared.td

// convert_shared and convert_not_shared MUST not overlap. i.e. contain 
// the same functions

code BitCast = [{
    return as_$ReturnType($Arg0VarName);
  }];

code BitSExt = [{
    return as_$ReturnType(__ocl_sext_$Arg0Symigentype_$ReturnSymigentype(as_$Arg0igentype($Arg0VarName)));
  }];

code BitZExt = [{
    return as_$ReturnType(__ocl_zext_$Arg0Symigentype_$ReturnSymigentype(as_$Arg0igentype($Arg0VarName)));
  }];

code BitTrunc = [{
    return as_$ReturnType(__ocl_trunc_$Arg0Symigentype_$ReturnSymigentype(as_$Arg0igentype($Arg0VarName)));
  }];

code PromoteI32 = [{
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symi32gentype_$ReturnSymigentype(
              convert_$Arg0i32gentype$Round($Arg0VarName)));
  }];
  
code PromoteU32 = [{
    return as_$ReturnType(
            __ocl_trunc_$Arg0Symi32gentype_$ReturnSymigentype(
              as_$Arg0i32gentype(
                convert_$Arg0u32gentype$Round($Arg0VarName))));
  }];

list<OclType> float_types = [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
                             v1f64, v2f64, v3f64, v4f64, v8f64, v16f64];

// 8-bit integer output
OclBuiltinImpl bitcast_i8 = OclBuiltinImpl<convert_i8,
                                           [v1i8, v2i8, v3i8, v4i8, v8i8, v16i8,
                                            v1u8, v2u8, v3u8, v4u8, v8u8, v16u8], 0, BitCast>;

OclBuiltinImpl trunc_i8   = OclBuiltinImpl<convert_i8,
                                           [v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                            v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                                            v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
                                            v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
                                            v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
                                            v1u64, v2u64, v3u64, v4u64, v8u64, v16u64], 0, BitTrunc>;

OclBuiltinImpl fp2int_i8_f  = OclBuiltinImpl<convert_i8, float_types, 0,
                                                PromoteI32, [["$Round",""]]>;

OclBuiltinImpl fp2int_i8_f_rte  = OclBuiltinImpl<convert_i8_rte, float_types, 0,
                                                    PromoteI32, [["$Round","_rte"]]>;

OclBuiltinImpl fp2int_i8_f_rtz  = OclBuiltinImpl<convert_i8_rtz, float_types, 0,
                                                    PromoteI32, [["$Round","_rtz"]]>;

OclBuiltinImpl fp2int_i8_f_rtp  = OclBuiltinImpl<convert_i8_rtp, float_types, 0,
                                                    PromoteI32, [["$Round","_rtp"]]>;

OclBuiltinImpl fp2int_i8_f_rtn  = OclBuiltinImpl<convert_i8_rtn, float_types, 0,
                                                    PromoteI32, [["$Round","_rtn"]]>;

OclBuiltinImpl bitcast_u8 = OclBuiltinImpl<convert_u8,
                                           [v1i8, v2i8, v3i8, v4i8, v8i8, v16i8,
                                            v1u8, v2u8, v3u8, v4u8, v8u8, v16u8], 0, BitCast>;

OclBuiltinImpl trunc_u8   = OclBuiltinImpl<convert_u8,
                                           [v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                            v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                                            v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
                                            v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
                                            v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
                                            v1u64, v2u64, v3u64, v4u64, v8u64, v16u64], 0, BitTrunc>;


OclBuiltinImpl fp2int_u8_f  = OclBuiltinImpl<convert_u8, float_types, 0,
                                                PromoteU32, [["$Round",""]]>;

OclBuiltinImpl fp2int_u8_f_rte  = OclBuiltinImpl<convert_u8_rte, float_types, 0,
                                                    PromoteU32, [["$Round","_rte"]]>;

OclBuiltinImpl fp2int_u8_f_rtz  = OclBuiltinImpl<convert_u8_rtz, float_types, 0,
                                                    PromoteU32, [["$Round","_rtz"]]>;

OclBuiltinImpl fp2int_u8_f_rtp  = OclBuiltinImpl<convert_u8_rtp, float_types, 0,
                                                    PromoteU32, [["$Round","_rtp"]]>;

OclBuiltinImpl fp2int_u8_f_rtn  = OclBuiltinImpl<convert_u8_rtn, float_types, 0,
                                                    PromoteU32, [["$Round","_rtn"]]>;

// 16-bit integer output
OclBuiltinImpl sext_i16    = OclBuiltinImpl<convert_i16,
                                            [v1i8, v2i8, v3i8, v4i8, v8i8, v16i8], 0, BitSExt>;

OclBuiltinImpl zext_i16    = OclBuiltinImpl<convert_i16,
                                            [v1u8, v2u8, v3u8, v4u8, v8u8, v16u8], 0, BitZExt>;

OclBuiltinImpl bitcast_i16 = OclBuiltinImpl<convert_i16,
                                            [v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                             v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, BitCast>;

OclBuiltinImpl trunc_i16   = OclBuiltinImpl<convert_i16,
                                            [v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
                                             v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
                                             v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
                                             v1u64, v2u64, v3u64, v4u64, v8u64, v16u64], 0, BitTrunc>;

OclBuiltinImpl fp2int_i16_f  = OclBuiltinImpl<convert_i16, float_types, 0,
                                                PromoteI32, [["$Round",""]]>;


OclBuiltinImpl fp2int_i16_f_rte  = OclBuiltinImpl<convert_i16_rte, float_types, 0,
                                                    PromoteI32, [["$Round","_rte"]]>;

OclBuiltinImpl fp2int_i16_f_rtz  = OclBuiltinImpl<convert_i16_rtz, float_types, 0,
                                                    PromoteI32, [["$Round","_rtz"]]>;

OclBuiltinImpl fp2int_i16_f_rtp  = OclBuiltinImpl<convert_i16_rtp, float_types, 0,
                                                    PromoteI32, [["$Round","_rtp"]]>;

OclBuiltinImpl fp2int_i16_f_rtn  = OclBuiltinImpl<convert_i16_rtn, float_types, 0,
                                                    PromoteI32, [["$Round","_rtn"]]>;


OclBuiltinImpl sext_u16    = OclBuiltinImpl<convert_u16,
                                            [v1i8, v2i8, v3i8, v4i8, v8i8, v16i8], 0, BitSExt>;

OclBuiltinImpl zext_u16    = OclBuiltinImpl<convert_u16,
                                            [v1u8, v2u8, v3u8, v4u8, v8u8, v16u8], 0, BitZExt>;

OclBuiltinImpl bitcast_u16 = OclBuiltinImpl<convert_u16,
                                            [v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                             v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, BitCast>;

OclBuiltinImpl trunc_u16   = OclBuiltinImpl<convert_u16,
                                            [v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
                                             v1u32, v2u32, v3u32, v4u32, v8u32, v16u32,
                                             v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
                                             v1u64, v2u64, v3u64, v4u64, v8u64, v16u64], 0, BitTrunc>;




OclBuiltinImpl fp2int_u16_f  = OclBuiltinImpl<convert_u16, float_types, 0,
                                              PromoteU32, [["$Round",""]]>;

OclBuiltinImpl fp2int_u16_f_rte  = OclBuiltinImpl<convert_u16_rte, float_types, 0,
                                                  PromoteU32, [["$Round","_rte"]]>;

OclBuiltinImpl fp2int_u16_f_rtz  = OclBuiltinImpl<convert_u16_rtz, float_types, 0,
                                                  PromoteU32, [["$Round","_rtz"]]>;

OclBuiltinImpl fp2int_u16_f_rtp  = OclBuiltinImpl<convert_u16_rtp, float_types, 0,
                                                  PromoteU32, [["$Round","_rtp"]]>;

OclBuiltinImpl fp2int_u16_f_rtn  = OclBuiltinImpl<convert_u16_rtn, float_types, 0,
                                                  PromoteU32, [["$Round","_rtn"]]>;

// 32-bit integer output
OclBuiltinImpl sext_i32    = OclBuiltinImpl<convert_i32,
                                            [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16], 0, BitSExt>;

OclBuiltinImpl zext_i32    = OclBuiltinImpl<convert_i32,
                                            [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                             v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, BitZExt>;

OclBuiltinImpl bitcast_i32 = OclBuiltinImpl<convert_i32,
                                            [v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
                                             v1u32, v2u32, v3u32, v4u32, v8u32, v16u32], 0, BitCast>;

OclBuiltinImpl trunc_i32   = OclBuiltinImpl<convert_i32,
                                            [v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
                                             v1u64, v2u64, v3u64, v4u64, v8u64, v16u64], 0, BitTrunc>;

OclBuiltinImpl fp2int_i32_f32 = OclBuiltinImpl<convert_i32,
                                               [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
                                                v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi32$rtznosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_i32_f32_rte = OclBuiltinImpl<convert_i32_rte,
                                                   [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                    v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi32$rtnnosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_i32_f32_rtz = OclBuiltinImpl<convert_i32_rtz,
                                                   [v1f32, v2f32, v3f32, v4f32,
                                                    v1f64, v2f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi32$rtznosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_i32_f32_rtp = OclBuiltinImpl<convert_i32_rtp,
                                                   [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                    v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi32$upnosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_i32_f32_rtn = OclBuiltinImpl<convert_i32_rtn,
                                                   [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                    v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi32$downnosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl sext_u32    = OclBuiltinImpl<convert_u32,
                                            [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16], 0, BitSExt>;

OclBuiltinImpl zext_u32    = OclBuiltinImpl<convert_u32,
                                            [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                             v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, BitZExt>;

OclBuiltinImpl bitcast_u32 = OclBuiltinImpl<convert_u32,
                                            [v1i32, v2i32, v3i32, v4i32, v8i32, v16i32,
                                             v1u32, v2u32, v3u32, v4u32, v8u32, v16u32], 0, BitCast>;

OclBuiltinImpl trunc_u32   = OclBuiltinImpl<convert_u32,
                                            [v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
                                             v1u64, v2u64, v3u64, v4u64, v8u64, v16u64], 0, BitTrunc>;

OclBuiltinImpl fp2int_u32_f32 = OclBuiltinImpl<convert_u32,
                                                   [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32,
                                                    v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptou32$rtznosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_u32_f32_rte = OclBuiltinImpl<convert_u32_rte,
                                                   [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                    v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptou32$rtnnosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_u32_f32_rtz = OclBuiltinImpl<convert_u32_rtz,
                                                   [v1f32, v2f32, v3f32, v4f32,
                                                    v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptou32$rtznosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_u32_f32_rtp = OclBuiltinImpl<convert_u32_rtp,
                                                   [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                    v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptou32$upnosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_u32_f32_rtn = OclBuiltinImpl<convert_u32_rtn,
                                                   [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                    v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptou32$downnosat$SVMLSuffix($Arg0VarName);
  }]>;

// 64-bit integer output
OclBuiltinImpl sext_i64    = OclBuiltinImpl<convert_i64,
                                            [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32], 0, BitSExt>;

OclBuiltinImpl zext_i64    = OclBuiltinImpl<convert_i64,
                                            [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                             v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                                             v1u32, v2u32, v3u32, v4u32, v8u32, v16u32], 0, BitZExt>;

OclBuiltinImpl bitcast_i64 = OclBuiltinImpl<convert_i64,
                                            [v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
                                             v1u64, v2u64, v3u64, v4u64, v8u64, v16u64], 0, BitCast>;

OclBuiltinImpl fp2int_i64_f32 = OclBuiltinImpl<convert_i64,
                                                   [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                    v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi64$rtznosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_i64_f32_rte = OclBuiltinImpl<convert_i64_rte,
                                                   [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                    v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi64$rtnnosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_i64_f32_rtz = OclBuiltinImpl<convert_i64_rtz,
                                                   [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                    v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi64$rtznosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_i64_f32_rtp = OclBuiltinImpl<convert_i64_rtp,
                                                   [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                    v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi64$upnosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_i64_f32_rtn = OclBuiltinImpl<convert_i64_rtn,
                                                   [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                    v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfptoi64$downnosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl sext_u64    = OclBuiltinImpl<convert_u64,
                                            [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                             v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                             v1i32, v2i32, v3i32, v4i32, v8i32, v16i32], 0, BitSExt>;

OclBuiltinImpl zext_u64    = OclBuiltinImpl<convert_u64,
                                            [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                             v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                                             v1u32, v2u32, v3u32, v4u32, v8u32, v16u32], 0, BitZExt>;

OclBuiltinImpl bitcast_u64 = OclBuiltinImpl<convert_u64,
                                            [v1i64, v2i64, v3i64, v4i64, v8i64, v16i64,
                                             v1u64, v2u64, v3u64, v4u64, v8u64, v16u64], 0, BitCast>;

OclBuiltinImpl fp2int_u64_f32 = OclBuiltinImpl<convert_u64,
                                               [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return  __ocl_svml_$Target_cvtfptou64$rtznosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_u64_f32_rte = OclBuiltinImpl<convert_u64_rte,
                                                   [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                    v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return  __ocl_svml_$Target_cvtfptou64$rtnnosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_u64_f32_rtz = OclBuiltinImpl<convert_u64_rtz,
                                                   [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                    v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return  __ocl_svml_$Target_cvtfptou64$rtznosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_u64_f32_rtp = OclBuiltinImpl<convert_u64_rtp,
                                                   [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                    v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return  __ocl_svml_$Target_cvtfptou64$upnosat$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl fp2int_u64_f32_rtn = OclBuiltinImpl<convert_u64_rtn,
                                                   [v1f32, v2f32, v3f32, v4f32, v8f32,
                                                    v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return  __ocl_svml_$Target_cvtfptou64$downnosat$SVMLSuffix($Arg0VarName);
  }]>;

// 32-bit floating output

// default rounding mode
OclBuiltinImpl nocvt_f32 = OclBuiltinImpl<convert_f32,
                                          [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32], 0, [{
    return $Arg0VarName;
  }]>;

OclBuiltinImpl cvt_f32_f64 = OclBuiltinImpl<convert_f32,
                                          [v1f64, v2f64, v3f64], 0, [{
    return __ocl_svml_$Target_cvtfp64tofp32$rtn$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl cvt_f32_i8i16 = OclBuiltinImpl<convert_f32,
                                              [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                               v1i16, v2i16, v3i16, v4i16, v8i16, v16i16], 0, [{
    return __ocl_svml_$Target_cvti32tofp$rtn$SVMLSuffix(
            as_$Arg0i32gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi32gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl cvt_f32_u8u16 = OclBuiltinImpl<convert_f32,
                                              [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                               v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$rtn$SVMLSuffix(
            as_$Arg0u32gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi32gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl cvt_f32_i32 = OclBuiltinImpl<convert_f32,
                                            [v1i32, v2i32, v3i32, v4i32, v16i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl cvt_f32_u32 = OclBuiltinImpl<convert_f32,
                                            [v1u32, v2u32, v3u32, v4u32, v8u32, v16u32], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl cvt_f32_i64 = OclBuiltinImpl<convert_f32,
                                            [v1i64, v2i64, v3i64, v4i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtn$SVMLFSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl cvt_f32_u64 = OclBuiltinImpl<convert_f32,
                                            [v1u64, v2u64, v3u64, v4u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtn$SVMLFSuffix($Arg0VarName);
  }]>;

// rte rounding mode
OclBuiltinImpl rte_nocvt_f32 = OclBuiltinImpl<convert_f32_rte,
                                              [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32], 0, [{
    return $Arg0VarName;
  }]>;

OclBuiltinImpl rte_cvt_f32_f64 = OclBuiltinImpl<convert_f32_rte,
                                                [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfp64tofp32$rtn$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rte_cvt_f32_i8i16 = OclBuiltinImpl<convert_f32_rte,
                                                  [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                   v1i16, v2i16, v3i16, v4i16, v8i16, v16i16], 0, [{
    return __ocl_svml_$Target_cvti32tofp$rtn$SVMLSuffix(
            as_$Arg0i32gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi32gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rte_cvt_f32_u8u16 = OclBuiltinImpl<convert_f32_rte,
                                                  [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                   v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$rtn$SVMLSuffix(
            as_$Arg0u32gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi32gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rte_cvt_f32_i32 = OclBuiltinImpl<convert_f32_rte,
                                                [v1i32, v2i32, v3i32, v4i32, v8i32, v16i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rte_cvt_f32_u32 = OclBuiltinImpl<convert_f32_rte,
                                                [v1u32, v2u32, v3u32, v4u32, v8u32, v16u32], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rte_cvt_f32_i64 = OclBuiltinImpl<convert_f32_rte,
                                                [v1i64, v2i64, v3i64, v4i64, v8i64, v16i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtn$SVMLFSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rte_cvt_f32_u64 = OclBuiltinImpl<convert_f32_rte,
                                                [v1u64, v2u64, v3u64, v4u64, v8u64, v16u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtn$SVMLFSuffix($Arg0VarName);
  }]>;

// rtz rounding mode
OclBuiltinImpl rtz_nocvt_f32 = OclBuiltinImpl<convert_f32_rtz,
                                              [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32], 0, [{
    return $Arg0VarName;
  }]>;

OclBuiltinImpl rtz_cvt_f32_f64 = OclBuiltinImpl<convert_f32_rtz,
                                                [v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfp64tofp32$rtz$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtz_cvt_f32_i8i16 = OclBuiltinImpl<convert_f32_rtz,
                                                  [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                   v1i16, v2i16, v3i16, v4i16, v8i16, v16i16], 0, [{
    return __ocl_svml_$Target_cvti32tofp$rtz$SVMLSuffix(
            as_$Arg0i32gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi32gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtz_cvt_f32_u8u16 = OclBuiltinImpl<convert_f32_rtz,
                                                  [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                   v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$rtz$SVMLSuffix(
            as_$Arg0u32gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi32gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtz_cvt_f32_i32 = OclBuiltinImpl<convert_f32_rtz,
                                                [v1i32, v2i32, v3i32, v4i32, v8i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$rtz$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtz_cvt_f32_u32 = OclBuiltinImpl<convert_f32_rtz,
                                                [v1u32, v2u32, v3u32, v4u32, v8u32], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$rtz$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtz_cvt_f32_i64 = OclBuiltinImpl<convert_f32_rtz,
                                                [v1i64, v2i64, v3i64, v4i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtz$SVMLFSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtz_cvt_f32_u64 = OclBuiltinImpl<convert_f32_rtz,
                                                [v1u64, v2u64, v3u64, v4u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtz$SVMLFSuffix($Arg0VarName);
  }]>;

// rtp rounding mode
OclBuiltinImpl rtp_nocvt_f32 = OclBuiltinImpl<convert_f32_rtp,
                                              [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32], 0, [{
    return $Arg0VarName;
  }]>;

OclBuiltinImpl rtp_cvt_f32_f64 = OclBuiltinImpl<convert_f32_rtp,
                                                [v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfp64tofp32$up$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtp_cvt_f32_i8i16 = OclBuiltinImpl<convert_f32_rtp,
                                                  [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                   v1i16, v2i16, v3i16, v4i16, v8i16, v16i16], 0, [{
    return __ocl_svml_$Target_cvti32tofp$up$SVMLSuffix(
            as_$Arg0i32gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi32gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtp_cvt_f32_u8u16 = OclBuiltinImpl<convert_f32_rtp,
                                                  [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                   v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$up$SVMLSuffix(
            as_$Arg0u32gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi32gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtp_cvt_f32_i32 = OclBuiltinImpl<convert_f32_rtp,
                                                [v1i32, v2i32, v3i32, v4i32, v8i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$up$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtp_cvt_f32_u32 = OclBuiltinImpl<convert_f32_rtp,
                                                [v1u32, v2u32, v3u32, v4u32, v8u32], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$up$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtp_cvt_f32_i64 = OclBuiltinImpl<convert_f32_rtp,
                                                [v1i64, v2i64, v3i64, v4i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$up$SVMLFSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtp_cvt_f32_u64 = OclBuiltinImpl<convert_f32_rtp,
                                                [v1u64, v2u64, v3u64, v4u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$up$SVMLFSuffix($Arg0VarName);
  }]>;

// rtn rounding mode
OclBuiltinImpl rtn_nocvt_f32 = OclBuiltinImpl<convert_f32_rtn,
                                              [v1f32, v2f32, v3f32, v4f32, v8f32, v16f32], 0, [{
    return $Arg0VarName;
  }]>;

OclBuiltinImpl rtn_cvt_f32_f64 = OclBuiltinImpl<convert_f32_rtn,
                                                [v1f64, v2f64, v3f64, v4f64, v16f64], 0, [{
    return __ocl_svml_$Target_cvtfp64tofp32$down$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtn_cvt_f32_i8i16 = OclBuiltinImpl<convert_f32_rtn,
                                                  [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                   v1i16, v2i16, v3i16, v4i16, v8i16, v16i16], 0, [{
    return __ocl_svml_$Target_cvti32tofp$down$SVMLSuffix(
            as_$Arg0i32gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi32gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtn_cvt_f32_u8u16 = OclBuiltinImpl<convert_f32_rtn,
                                                  [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                   v1u16, v2u16, v3u16, v4u16, v8u16, v16u16], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$down$SVMLSuffix(
            as_$Arg0u32gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi32gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtn_cvt_f32_i32 = OclBuiltinImpl<convert_f32_rtn,
                                                [v1i32, v2i32, v3i32, v4i32, v8i32], 0, [{
    return __ocl_svml_$Target_cvti32tofp$down$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtn_cvt_f32_u32 = OclBuiltinImpl<convert_f32_rtn,
                                                [v1u32, v2u32, v3u32, v4u32, v8u32], 0, [{
    return __ocl_svml_$Target_cvtu32tofp$down$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtn_cvt_f32_i64 = OclBuiltinImpl<convert_f32_rtn,
                                                [v1i64, v2i64, v3i64, v4i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$down$SVMLFSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtn_cvt_f32_u64 = OclBuiltinImpl<convert_f32_rtn,
                                                [v1u64, v2u64, v3u64, v4u64], 0, [{
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

// default rounding mode
OclBuiltinImpl nocvt_f64 = OclBuiltinImpl<convert_f64,
                                          [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, [{
    return $Arg0VarName;
  }]>;

OclBuiltinImpl cvt_f64_i8i16i32 = OclBuiltinImpl<convert_f64,
                                                 [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                  v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                  v1i32, v2i32, v16i32], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtn$VecLength(
            as_$Arg0i64gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl cvt_f32_u8u16u32 = OclBuiltinImpl<convert_f64,
                                                 [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                  v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                                                  v1u32, v2u32, v3u32, v4u32, v16u32], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtn$VecLength(
            as_$Arg0u64gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl cvt_f64_i64 = OclBuiltinImpl<convert_f64,
                                            [v1i64, v2i64, v3i64, v4i64, v16i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl cvt_f64_u64 = OclBuiltinImpl<convert_f64,
                                            [v1u64, v2u64, v3u64, v4u64, v16u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;

// rte rounding mode
OclBuiltinImpl rte_nocvt_f64 = OclBuiltinImpl<convert_f64_rte,
                                              [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, [{
    return $Arg0VarName;
  }]>;

OclBuiltinImpl rte_cvt_f64_i8i16i32 = OclBuiltinImpl<convert_f64_rte,
                                                     [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                      v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                      v1i32, v2i32, v3i32, v4i32, v16i32], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtn$VecLength(
            as_$Arg0i64gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rte_cvt_f32_u8u16u32 = OclBuiltinImpl<convert_f64_rte,
                                                     [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                      v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                                                      v1u32, v2u32, v3u32, v4u32, v16u32], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtn$VecLength(
            as_$Arg0u64gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rte_cvt_f64_i64 = OclBuiltinImpl<convert_f64_rte,
                                                [v1i64, v2i64, v3i64, v4i64, v16i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rte_cvt_f64_u64 = OclBuiltinImpl<convert_f64_rte,
                                                [v1u64, v2u64, v3u64, v4u64, v16u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtn$SVMLSuffix($Arg0VarName);
  }]>;

// rtz rounding mode
OclBuiltinImpl rtz_nocvt_f64 = OclBuiltinImpl<convert_f64_rtz,
                                              [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, [{
    return $Arg0VarName;
  }]>;

OclBuiltinImpl rtz_cvt_f64_i8i16i32 = OclBuiltinImpl<convert_f64_rtz,
                                                     [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                      v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                      v1i32, v2i32, v3i32, v4i32, v16i32], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtz$VecLength(
            as_$Arg0i64gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtz_cvt_f32_u8u16u32 = OclBuiltinImpl<convert_f64_rtz,
                                                     [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                      v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                                                      v1u32, v2u32, v3u32, v4u32, v16u32], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtz$VecLength(
            as_$Arg0u64gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtz_cvt_f64_i64 = OclBuiltinImpl<convert_f64_rtz,
                                                [v1i64, v2i64, v3i64, v4i64, v16i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$rtz$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtz_cvt_f64_u64 = OclBuiltinImpl<convert_f64_rtz,
                                                [v1u64, v2u64, v3u64, v4u64, v16u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$rtz$SVMLSuffix($Arg0VarName);
  }]>;

// rtp rounding mode
OclBuiltinImpl rtp_nocvt_f64 = OclBuiltinImpl<convert_f64_rtp,
                                              [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, [{
    return $Arg0VarName;
  }]>;

OclBuiltinImpl rtp_cvt_f64_i8i16i32 = OclBuiltinImpl<convert_f64_rtp,
                                                     [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                      v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                      v1i32, v2i32, v3i32, v4i32, v16i32], 0, [{
    return __ocl_svml_$Target_cvti64tofp$up$VecLength(
            as_$Arg0i64gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtp_cvt_f32_u8u16u32 = OclBuiltinImpl<convert_f64_rtp,
                                                     [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                      v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                                                      v1u32, v2u32, v3u32, v4u32, v16u32], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$up$VecLength(
            as_$Arg0u64gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtp_cvt_f64_i64 = OclBuiltinImpl<convert_f64_rtp,
                                                [v1i64, v2i64, v3i64, v4i64, v16i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$up$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtp_cvt_f64_u64 = OclBuiltinImpl<convert_f64_rtp,
                                                [v1u64, v2u64, v3u64, v4u64, v16u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$up$SVMLSuffix($Arg0VarName);
  }]>;

// rtn rounding mode
OclBuiltinImpl rtn_nocvt_f64 = OclBuiltinImpl<convert_f64_rtn,
                                              [v1f64, v2f64, v3f64, v4f64, v8f64, v16f64], 0, [{
    return $Arg0VarName;
  }]>;

OclBuiltinImpl rtn_cvt_f64_i8i16i32 = OclBuiltinImpl<convert_f64_rtn,
                                                     [ v1i8,  v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                                      v1i16, v2i16, v3i16, v4i16, v8i16, v16i16,
                                                      v1i32, v2i32, v3i32, v4i32, v16i32], 0, [{
    return __ocl_svml_$Target_cvti64tofp$down$VecLength(
            as_$Arg0i64gentype(
              __ocl_sext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtn_cvt_f32_u8u16u32 = OclBuiltinImpl<convert_f64_rtn,
                                                     [ v1u8,  v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                                      v1u16, v2u16, v3u16, v4u16, v8u16, v16u16,
                                                      v1u32, v2u32, v3u32, v4u32, v16u32], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$down$VecLength(
            as_$Arg0u64gentype(
              __ocl_zext_$Arg0Symigentype_$Arg0Symi64gentype(
                as_$Arg0igentype($Arg0VarName))));
  }]>;

OclBuiltinImpl rtn_cvt_f64_i64 = OclBuiltinImpl<convert_f64_rtn,
                                                [v1i64, v2i64, v3i64, v4i64, v16i64], 0, [{
    return __ocl_svml_$Target_cvti64tofp$down$SVMLSuffix($Arg0VarName);
  }]>;

OclBuiltinImpl rtn_cvt_f64_u64 = OclBuiltinImpl<convert_f64_rtn,
                                                [v1u64, v2u64, v3u64, v4u64, v16u64], 0, [{
    return __ocl_svml_$Target_cvtu64tofp$down$SVMLSuffix($Arg0VarName);
  }]>;

// int2int conversion with rounding make no sense but we add here for completeness.
//

code _int2int_rounding = [{
    return convert_$ReturnType($Arg0VarName);
  }];

code _int2int_rounding_sat = [{
    return convert_$ReturnType_sat($Arg0VarName);
  }];

OclBuiltinImpl impl_convert_i8_rte      = OclBuiltinImpl<_convert_i8_rte,      _convert_i8_rte.Types,     0, _int2int_rounding>;
OclBuiltinImpl impl_convert_i8_rtz      = OclBuiltinImpl<_convert_i8_rtz,      _convert_i8_rtz.Types,     0, _int2int_rounding>;
OclBuiltinImpl impl_convert_i8_rtp      = OclBuiltinImpl<_convert_i8_rtp,      _convert_i8_rtp.Types,     0, _int2int_rounding>;
OclBuiltinImpl impl_convert_i8_rtn      = OclBuiltinImpl<_convert_i8_rtn,      _convert_i8_rtn.Types,     0, _int2int_rounding>;
OclBuiltinImpl impl_convert_i8_sat_rte  = OclBuiltinImpl<_convert_i8_sat_rte,  _convert_i8_sat_rte.Types, 0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_i8_sat_rtz  = OclBuiltinImpl<_convert_i8_sat_rtz,  _convert_i8_sat_rtz.Types, 0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_i8_sat_rtp  = OclBuiltinImpl<_convert_i8_sat_rtp,  _convert_i8_sat_rtp.Types, 0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_i8_sat_rtn  = OclBuiltinImpl<_convert_i8_sat_rtn,  _convert_i8_sat_rtn.Types, 0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_u8_rte      = OclBuiltinImpl<_convert_u8_rte,      _convert_u8_rte.Types,     0, _int2int_rounding>;
OclBuiltinImpl impl_convert_u8_rtz      = OclBuiltinImpl<_convert_u8_rtz,      _convert_u8_rtz.Types,     0, _int2int_rounding>;
OclBuiltinImpl impl_convert_u8_rtp      = OclBuiltinImpl<_convert_u8_rtp,      _convert_u8_rtp.Types,     0, _int2int_rounding>;
OclBuiltinImpl impl_convert_u8_rtn      = OclBuiltinImpl<_convert_u8_rtn,      _convert_u8_rtn.Types,     0, _int2int_rounding>;
OclBuiltinImpl impl_convert_u8_sat_rte  = OclBuiltinImpl<_convert_u8_sat_rte,  _convert_u8_sat_rte.Types, 0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_u8_sat_rtz  = OclBuiltinImpl<_convert_u8_sat_rtz,  _convert_u8_sat_rtz.Types, 0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_u8_sat_rtp  = OclBuiltinImpl<_convert_u8_sat_rtp,  _convert_u8_sat_rtp.Types, 0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_u8_sat_rtn  = OclBuiltinImpl<_convert_u8_sat_rtn,  _convert_u8_sat_rtn.Types, 0, _int2int_rounding_sat>;

OclBuiltinImpl impl_convert_i16_rte     = OclBuiltinImpl<_convert_i16_rte,     _convert_i16_rte.Types,    0, _int2int_rounding>;
OclBuiltinImpl impl_convert_i16_rtz     = OclBuiltinImpl<_convert_i16_rtz,     _convert_i16_rtz.Types,    0, _int2int_rounding>;
OclBuiltinImpl impl_convert_i16_rtp     = OclBuiltinImpl<_convert_i16_rtp,     _convert_i16_rtp.Types,    0, _int2int_rounding>;
OclBuiltinImpl impl_convert_i16_rtn     = OclBuiltinImpl<_convert_i16_rtn,     _convert_i16_rtn.Types,    0, _int2int_rounding>;
OclBuiltinImpl impl_convert_i16_sat_rte = OclBuiltinImpl<_convert_i16_sat_rte, _convert_i16_sat_rte.Types,0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_i16_sat_rtz = OclBuiltinImpl<_convert_i16_sat_rtz, _convert_i16_sat_rtz.Types,0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_i16_sat_rtp = OclBuiltinImpl<_convert_i16_sat_rtp, _convert_i16_sat_rtp.Types,0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_i16_sat_rtn = OclBuiltinImpl<_convert_i16_sat_rtn, _convert_i16_sat_rtn.Types,0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_u16_rte     = OclBuiltinImpl<_convert_u16_rte,     _convert_u16_rte.Types,    0, _int2int_rounding>;
OclBuiltinImpl impl_convert_u16_rtz     = OclBuiltinImpl<_convert_u16_rtz,     _convert_u16_rtz.Types,    0, _int2int_rounding>;
OclBuiltinImpl impl_convert_u16_rtp     = OclBuiltinImpl<_convert_u16_rtp,     _convert_u16_rtp.Types,    0, _int2int_rounding>;
OclBuiltinImpl impl_convert_u16_rtn     = OclBuiltinImpl<_convert_u16_rtn,     _convert_u16_rtn.Types,    0, _int2int_rounding>;
OclBuiltinImpl impl_convert_u16_sat_rte = OclBuiltinImpl<_convert_u16_sat_rte, _convert_u16_sat_rte.Types,0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_u16_sat_rtz = OclBuiltinImpl<_convert_u16_sat_rtz, _convert_u16_sat_rtz.Types,0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_u16_sat_rtp = OclBuiltinImpl<_convert_u16_sat_rtp, _convert_u16_sat_rtp.Types,0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_u16_sat_rtn = OclBuiltinImpl<_convert_u16_sat_rtn, _convert_u16_sat_rtn.Types,0, _int2int_rounding_sat>;

OclBuiltinImpl impl_convert_i32_rte     = OclBuiltinImpl<_convert_i32_rte,     _convert_i32_rte.Types,    0, _int2int_rounding>;
OclBuiltinImpl impl_convert_i32_rtz     = OclBuiltinImpl<_convert_i32_rtz,     _convert_i32_rtz.Types,    0, _int2int_rounding>;
OclBuiltinImpl impl_convert_i32_rtp     = OclBuiltinImpl<_convert_i32_rtp,     _convert_i32_rtp.Types,    0, _int2int_rounding>;
OclBuiltinImpl impl_convert_i32_rtn     = OclBuiltinImpl<_convert_i32_rtn,     _convert_i32_rtn.Types,    0, _int2int_rounding>;
OclBuiltinImpl impl_convert_i32_sat_rte = OclBuiltinImpl<_convert_i32_sat_rte, _convert_i32_sat_rte.Types,0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_i32_sat_rtz = OclBuiltinImpl<_convert_i32_sat_rtz, _convert_i32_sat_rtz.Types,0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_i32_sat_rtp = OclBuiltinImpl<_convert_i32_sat_rtp, _convert_i32_sat_rtp.Types,0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_i32_sat_rtn = OclBuiltinImpl<_convert_i32_sat_rtn, _convert_i32_sat_rtn.Types,0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_u32_rte     = OclBuiltinImpl<_convert_u32_rte,     _convert_u32_rte.Types,    0, _int2int_rounding>;
OclBuiltinImpl impl_convert_u32_rtz     = OclBuiltinImpl<_convert_u32_rtz,     _convert_u32_rtz.Types,    0, _int2int_rounding>;
OclBuiltinImpl impl_convert_u32_rtp     = OclBuiltinImpl<_convert_u32_rtp,     _convert_u32_rtp.Types,    0, _int2int_rounding>;
OclBuiltinImpl impl_convert_u32_rtn     = OclBuiltinImpl<_convert_u32_rtn,     _convert_u32_rtn.Types,    0, _int2int_rounding>;
OclBuiltinImpl impl_convert_u32_sat_rte = OclBuiltinImpl<_convert_u32_sat_rte, _convert_u32_sat_rte.Types,0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_u32_sat_rtz = OclBuiltinImpl<_convert_u32_sat_rtz, _convert_u32_sat_rtz.Types,0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_u32_sat_rtp = OclBuiltinImpl<_convert_u32_sat_rtp, _convert_u32_sat_rtp.Types,0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_u32_sat_rtn = OclBuiltinImpl<_convert_u32_sat_rtn, _convert_u32_sat_rtn.Types,0, _int2int_rounding_sat>;

OclBuiltinImpl impl_convert_i64_rte     = OclBuiltinImpl<_convert_i64_rte,     _convert_i64_rte.Types,    0, _int2int_rounding>;
OclBuiltinImpl impl_convert_i64_rtz     = OclBuiltinImpl<_convert_i64_rtz,     _convert_i64_rtz.Types,    0, _int2int_rounding>;
OclBuiltinImpl impl_convert_i64_rtp     = OclBuiltinImpl<_convert_i64_rtp,     _convert_i64_rtp.Types,    0, _int2int_rounding>;
OclBuiltinImpl impl_convert_i64_rtn     = OclBuiltinImpl<_convert_i64_rtn,     _convert_i64_rtn.Types,    0, _int2int_rounding>;
OclBuiltinImpl impl_convert_i64_sat_rte = OclBuiltinImpl<_convert_i64_sat_rte, _convert_i64_sat_rte.Types,0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_i64_sat_rtz = OclBuiltinImpl<_convert_i64_sat_rtz, _convert_i64_sat_rtz.Types,0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_i64_sat_rtp = OclBuiltinImpl<_convert_i64_sat_rtp, _convert_i64_sat_rtp.Types,0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_i64_sat_rtn = OclBuiltinImpl<_convert_i64_sat_rtn, _convert_i64_sat_rtn.Types,0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_u64_rte     = OclBuiltinImpl<_convert_u64_rte,     _convert_u64_rte.Types,    0, _int2int_rounding>;
OclBuiltinImpl impl_convert_u64_rtz     = OclBuiltinImpl<_convert_u64_rtz,     _convert_u64_rtz.Types,    0, _int2int_rounding>;
OclBuiltinImpl impl_convert_u64_rtp     = OclBuiltinImpl<_convert_u64_rtp,     _convert_u64_rtp.Types,    0, _int2int_rounding>;
OclBuiltinImpl impl_convert_u64_rtn     = OclBuiltinImpl<_convert_u64_rtn,     _convert_u64_rtn.Types,    0, _int2int_rounding>;
OclBuiltinImpl impl_convert_u64_sat_rte = OclBuiltinImpl<_convert_u64_sat_rte, _convert_u64_sat_rte.Types,0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_u64_sat_rtz = OclBuiltinImpl<_convert_u64_sat_rtz, _convert_u64_sat_rtz.Types,0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_u64_sat_rtp = OclBuiltinImpl<_convert_u64_sat_rtp, _convert_u64_sat_rtp.Types,0, _int2int_rounding_sat>;
OclBuiltinImpl impl_convert_u64_sat_rtn = OclBuiltinImpl<_convert_u64_sat_rtn, _convert_u64_sat_rtn.Types,0, _int2int_rounding_sat>;

// fp2 conversion with rounding make no sense but we add here for completeness.
//

code _2fp_sat = [{
    return convert_$ReturnType($Arg0VarName);
  }];

code _2fp_sat_rte = [{
    return convert_$ReturnType_rte($Arg0VarName);
  }];

code _2fp_sat_rtz = [{
    return convert_$ReturnType_rtz($Arg0VarName);
  }];

code _2fp_sat_rtp = [{
    return convert_$ReturnType_rtp($Arg0VarName);
  }];

code _2fp_sat_rtn = [{
    return convert_$ReturnType_rtn($Arg0VarName);
  }];

OclBuiltinImpl impl_convert_f32_sat     = OclBuiltinImpl<_convert_f32_sat,     _convert_f32_sat.Types,     0, _2fp_sat>;
OclBuiltinImpl impl_convert_f32_sat_rte = OclBuiltinImpl<_convert_f32_sat_rte, _convert_f32_sat_rte.Types, 0, _2fp_sat_rte>;
OclBuiltinImpl impl_convert_f32_sat_rtz = OclBuiltinImpl<_convert_f32_sat_rtz, _convert_f32_sat_rtz.Types, 0, _2fp_sat_rtz>;
OclBuiltinImpl impl_convert_f32_sat_rtp = OclBuiltinImpl<_convert_f32_sat_rtp, _convert_f32_sat_rtp.Types, 0, _2fp_sat_rtp>;
OclBuiltinImpl impl_convert_f32_sat_rtn = OclBuiltinImpl<_convert_f32_sat_rtn, _convert_f32_sat_rtn.Types, 0, _2fp_sat_rtn>;

OclBuiltinImpl impl_convert_f64_sat     = OclBuiltinImpl<_convert_f64_sat,     _convert_f64_sat.Types,     0, _2fp_sat>;
OclBuiltinImpl impl_convert_f64_sat_rte = OclBuiltinImpl<_convert_f64_sat_rte, _convert_f64_sat_rte.Types, 0, _2fp_sat_rte>;
OclBuiltinImpl impl_convert_f64_sat_rtz = OclBuiltinImpl<_convert_f64_sat_rtz, _convert_f64_sat_rtz.Types, 0, _2fp_sat_rtz>;
OclBuiltinImpl impl_convert_f64_sat_rtp = OclBuiltinImpl<_convert_f64_sat_rtp, _convert_f64_sat_rtp.Types, 0, _2fp_sat_rtp>;
OclBuiltinImpl impl_convert_f64_sat_rtn = OclBuiltinImpl<_convert_f64_sat_rtn, _convert_f64_sat_rtn.Types, 0, _2fp_sat_rtn>;
