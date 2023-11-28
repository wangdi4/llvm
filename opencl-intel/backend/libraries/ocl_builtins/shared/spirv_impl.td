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

def spirv_convert_f_to_bf16_impl : OclBuiltinImpl<spirv_convert_f_to_bf16, spirv_convert_f_to_bf16.Types, /*IsDeclOnly*/false, [{
    $Arg0u16gentype isNaN = convert_$Arg0u16gentype(isnan($Arg0VarName));
    $ReturnType nanResult = 0xffc1;
    $Arg0u32gentype arg0 = __builtin_astype($Arg0VarName, $Arg0u32gentype);
    $Arg0u32gentype roundingBias = ((arg0 >> 16) & 0x1) + 0x00007FFF;
    $Arg0u32gentype tmp = arg0 + roundingBias;
    return select(convert_$ReturnType(tmp >> 16), nanResult, isNaN);
}]>;

def spirv_convert_bf16_to_f_impl : OclBuiltinImpl<spirv_convert_bf16_to_f, spirv_convert_bf16_to_f.Types, /*IsDeclOnly*/false, [{
    $Arg0u16gentype _u16 = __builtin_astype($Arg0VarName, $Arg0u16gentype);
    return __builtin_astype(convert_$Arg0u32gentype(_u16) << 16, $Arg0f32gentype);
}]>;

// Binary format of TF32: 1 sign bit + 8 exponent bits + 10 mantissa bits + 13 padding bits
def spirv_round_f_to_tf32_impl : OclBuiltinImpl<spirv_round_f_to_tf32, spirv_round_f_to_tf32.Types, /*IsDeclOnly*/false, [{
    $Arg0u32gentype _arg0 = __builtin_astype($Arg0VarName, $Arg0u32gentype);
    $Arg0u32gentype _exp = _arg0 & 0x7F800000;
    $Arg0u32gentype _mantissa = _arg0 & 0x7FFFFF; // least 23 bits
    $Arg0u32gentype _trunc = _arg0 & 0x1FFF; // least 13 bits

    $ReturnType _nan = __builtin_astype(($Arg0u32gentype)(0x7F800000 | 0x00400000), $ReturnType);

    // Round to nearest, ties to even
    $Arg0i32gentype _ties = _trunc == (1 << 12); // exactly halfway in the middle
    $Arg0i32gentype _tf32_odd = (_mantissa & (1 << 13)) != 0; // least significant bit is set (odd)
    // Round up if truncated bits are exact halfway and trailing bit of tf32 mantissa is set,
    // or the truncated bits exceed halfway of ULP.
    $Arg0i32gentype _round_up = (_ties & _tf32_odd) | (_trunc > (1 << 12));
    return select(
        __builtin_astype(_arg0 + __builtin_astype(0x1000 & _round_up, $Arg0u32gentype), $ReturnType),
        select(
            _nan, // NaN
            $Arg0VarName, // Inf: same representation for tf32 and f32
            _mantissa == 0), // Inf?
        _exp == 0x7F800000); // NaN or Inf?
}]>;
