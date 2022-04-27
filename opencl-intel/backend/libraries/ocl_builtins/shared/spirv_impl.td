def spirv_convert_f_to_bf16_impl : OclBuiltinImpl<spirv_convert_f_to_bf16, spirv_convert_f_to_bf16.Types, /*IsDeclOnly*/false, [{
    return convert_$ReturnType(__builtin_astype($Arg0VarName, $Arg0u32gentype) >> 16);
}]>;

def spirv_convert_bf16_to_f_impl : OclBuiltinImpl<spirv_convert_bf16_to_f, spirv_convert_bf16_to_f.Types, /*IsDeclOnly*/false, [{
    $Arg0u16gentype _u16 = __builtin_astype($Arg0VarName, $Arg0u16gentype);
    return __builtin_astype(convert_$Arg0u32gentype(_u16) << 16, $Arg0f32gentype);
}]>;
