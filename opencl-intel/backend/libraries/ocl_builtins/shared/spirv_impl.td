// Copyright (C) 2022-2023 Intel Corporation
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
