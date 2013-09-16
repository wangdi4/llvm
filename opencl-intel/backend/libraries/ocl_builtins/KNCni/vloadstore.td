// vim:ts=2:sw=2:et:


list<OclType> all_v16x64_types  = [v16i64, v16u64, v16f64];
list<OclType> non_v16x64_types  = [ v2i8,  v3i8,  v4i8,  v8i8,  v16i8,
                                    v2u8,  v3u8,  v4u8,  v8u8,  v16u8,
                                   v2i16, v3i16, v4i16, v8i16, v16i16,
                                   v2u16, v3u16, v4u16, v8u16, v16u16,
                                   v2i32, v3i32, v4i32, v8i32, v16i32,
                                   v2u32, v3u32, v4u32, v8u32, v16u32,
                                   v2i64, v3i64, v4i64, v8i64,
                                   v2u64, v3u64, v4u64, v8u64,
                                   v2f32, v3f32, v4f32, v8f32, v16f32,
                                   v2f64, v3f64, v4f64, v8f64];

// vload
code MaskVLoadV16U8 = [{
    __m512i $ReturnVarName = _mm512_undefined_epi32();
    switch ($Arg0VarName) {
      case 0xFFFF:
        $ReturnVarName = _mm512_check_load_epi32($ReturnVarName, (void const *)($Arg2VarName + ($Arg1VarName * 16)), _MM_UPCONV_EPI32_UINT8, _MM_HINT_NONE);
        break;
      case 0x00FF:
        $ReturnVarName = _mm512_mask_check_load_epi32($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 8)), _MM_UPCONV_EPI32_UINT8, _MM_HINT_NONE, 8*1);
        break;
      case 0x000F:
        $ReturnVarName = _mm512_mask_check_load_epi32($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 4)), _MM_UPCONV_EPI32_UINT8, _MM_HINT_NONE, 4*1);
        break;
      case 0x0007:
        $ReturnVarName = _mm512_mask_check_load_epi32($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 3)), _MM_UPCONV_EPI32_UINT8, _MM_HINT_NONE, 3*1);
        break;
      case 0x0003:
        $ReturnVarName = _mm512_mask_check_load_epi32($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 2)), _MM_UPCONV_EPI32_UINT8, _MM_HINT_NONE, 2*1);
        break;
      case 0x0001:
        $ReturnVarName = _mm512_extload_epi32((void const *)($Arg2VarName + $Arg1VarName), _MM_UPCONV_EPI32_UINT8, _MM_BROADCAST_1X16, _MM_HINT_NONE);
        break;
    }
    return convert_$ReturnType(as_uint16($ReturnVarName));
  }];
OclBuiltinImpl mask_vload_gas_v16_u8 = OclBuiltinImpl<mask_vload_gas, [v16u8], 0, MaskVLoadV16U8>;
OclBuiltinImpl mask_vload_cas_v16_u8 = OclBuiltinImpl<mask_vload_cas, [v16u8], 0, MaskVLoadV16U8>;
OclBuiltinImpl mask_vload_las_v16_u8 = OclBuiltinImpl<mask_vload_las, [v16u8], 0, MaskVLoadV16U8>;
OclBuiltinImpl mask_vload_pas_v16_u8 = OclBuiltinImpl<mask_vload_pas, [v16u8], 0, MaskVLoadV16U8>;

code MaskVLoadV16I8 = [{
    __m512i $ReturnVarName = _mm512_undefined_epi32();
    switch ($Arg0VarName) {
      case 0xFFFF:
        $ReturnVarName = _mm512_check_load_epi32($ReturnVarName, (void const *)($Arg2VarName + ($Arg1VarName * 16)), _MM_UPCONV_EPI32_SINT8, _MM_HINT_NONE);
        break;
      case 0x00FF:
        $ReturnVarName = _mm512_mask_check_load_epi32($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 8)), _MM_UPCONV_EPI32_SINT8, _MM_HINT_NONE, 8*1);
        break;
      case 0x000F:
        $ReturnVarName = _mm512_mask_check_load_epi32($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 4)), _MM_UPCONV_EPI32_SINT8, _MM_HINT_NONE, 4*1);
        break;
      case 0x0007:
        $ReturnVarName = _mm512_mask_check_load_epi32($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 3)), _MM_UPCONV_EPI32_SINT8, _MM_HINT_NONE, 3*1);
        break;
      case 0x0003:
        $ReturnVarName = _mm512_mask_check_load_epi32($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 2)), _MM_UPCONV_EPI32_SINT8, _MM_HINT_NONE, 2*1);
        break;
      case 0x0001:
        $ReturnVarName = _mm512_extload_epi32((void const *)($Arg2VarName + $Arg1VarName), _MM_UPCONV_EPI32_SINT8, _MM_BROADCAST_1X16, _MM_HINT_NONE);
        break;
    }
    return convert_$ReturnType(as_int16($ReturnVarName));
  }];
OclBuiltinImpl mask_vload_gas_v16_i8 = OclBuiltinImpl<mask_vload_gas, [v16i8], 0, MaskVLoadV16I8>;
OclBuiltinImpl mask_vload_cas_v16_i8 = OclBuiltinImpl<mask_vload_cas, [v16i8], 0, MaskVLoadV16I8>;
OclBuiltinImpl mask_vload_las_v16_i8 = OclBuiltinImpl<mask_vload_las, [v16i8], 0, MaskVLoadV16I8>;
OclBuiltinImpl mask_vload_pas_v16_i8 = OclBuiltinImpl<mask_vload_pas, [v16i8], 0, MaskVLoadV16I8>;

code MaskVLoadV16U16 = [{
    __m512i $ReturnVarName = _mm512_undefined_epi32();
    switch ($Arg0VarName) {
      case 0xFFFF:
        $ReturnVarName = _mm512_check_load_epi32($ReturnVarName, (void const *)($Arg2VarName + ($Arg1VarName * 16)), _MM_UPCONV_EPI32_UINT16, _MM_HINT_NONE);
        break;
      case 0x00FF:
        $ReturnVarName = _mm512_mask_check_load_epi32($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 8)), _MM_UPCONV_EPI32_UINT16, _MM_HINT_NONE, 8*2);
        break;
      case 0x000F:
        $ReturnVarName = _mm512_mask_check_load_epi32($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 4)), _MM_UPCONV_EPI32_UINT16, _MM_HINT_NONE, 4*2);
        break;
      case 0x0007:
        $ReturnVarName = _mm512_mask_check_load_epi32($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 3)), _MM_UPCONV_EPI32_UINT16, _MM_HINT_NONE, 3*2);
        break;
      case 0x0003:
        $ReturnVarName = _mm512_mask_check_load_epi32($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 2)), _MM_UPCONV_EPI32_UINT16, _MM_HINT_NONE, 2*2);
        break;
      case 0x0001:
        $ReturnVarName = _mm512_extload_epi32((void const *)($Arg2VarName + $Arg1VarName), _MM_UPCONV_EPI32_UINT16, _MM_BROADCAST_1X16, _MM_HINT_NONE);
        break;
    }
    return convert_$ReturnType(as_uint16($ReturnVarName));
  }];
OclBuiltinImpl mask_vload_gas_v16_u16 = OclBuiltinImpl<mask_vload_gas, [v16u16], 0, MaskVLoadV16U16>;
OclBuiltinImpl mask_vload_cas_v16_u16 = OclBuiltinImpl<mask_vload_cas, [v16u16], 0, MaskVLoadV16U16>;
OclBuiltinImpl mask_vload_las_v16_u16 = OclBuiltinImpl<mask_vload_las, [v16u16], 0, MaskVLoadV16U16>;
OclBuiltinImpl mask_vload_pas_v16_u16 = OclBuiltinImpl<mask_vload_pas, [v16u16], 0, MaskVLoadV16U16>;

code MaskVLoadV16I16 = [{
    __m512i $ReturnVarName = _mm512_undefined_epi32();
    switch ($Arg0VarName) {
      case 0xFFFF:
        $ReturnVarName = _mm512_check_load_epi32($ReturnVarName, (void const *)($Arg2VarName + ($Arg1VarName * 16)), _MM_UPCONV_EPI32_SINT16, _MM_HINT_NONE);
        break;
      case 0x00FF:
        $ReturnVarName = _mm512_mask_check_load_epi32($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 8)), _MM_UPCONV_EPI32_SINT16, _MM_HINT_NONE, 8*2);
        break;
      case 0x000F:
        $ReturnVarName = _mm512_mask_check_load_epi32($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 4)), _MM_UPCONV_EPI32_SINT16, _MM_HINT_NONE, 4*2);
        break;
      case 0x0007:
        $ReturnVarName = _mm512_mask_check_load_epi32($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 3)), _MM_UPCONV_EPI32_SINT16, _MM_HINT_NONE, 3*2);
        break;
      case 0x0003:
        $ReturnVarName = _mm512_mask_check_load_epi32($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 2)), _MM_UPCONV_EPI32_SINT16, _MM_HINT_NONE, 2*2);
        break;
      case 0x0001:
        $ReturnVarName = _mm512_extload_epi32((void const *)($Arg2VarName + $Arg1VarName), _MM_UPCONV_EPI32_SINT16, _MM_BROADCAST_1X16, _MM_HINT_NONE);
        break;
    }
    return convert_$ReturnType(as_int16($ReturnVarName));
  }];
OclBuiltinImpl mask_vload_gas_v16_i16 = OclBuiltinImpl<mask_vload_gas, [v16i16], 0, MaskVLoadV16I16>;
OclBuiltinImpl mask_vload_cas_v16_i16 = OclBuiltinImpl<mask_vload_cas, [v16i16], 0, MaskVLoadV16I16>;
OclBuiltinImpl mask_vload_las_v16_i16 = OclBuiltinImpl<mask_vload_las, [v16i16], 0, MaskVLoadV16I16>;
OclBuiltinImpl mask_vload_pas_v16_i16 = OclBuiltinImpl<mask_vload_pas, [v16i16], 0, MaskVLoadV16I16>;

code MaskVLoadV16UI32 = [{
    __m512i $ReturnVarName = _mm512_undefined_epi32();
    switch ($Arg0VarName) {
      case 0xFFFF:
        $ReturnVarName = _mm512_check_load_epi32($ReturnVarName, (void const *)($Arg2VarName + ($Arg1VarName * 16)), _MM_UPCONV_EPI32_NONE, _MM_HINT_NONE);
        break;
      case 0x00FF:
        $ReturnVarName = _mm512_mask_check_load_epi32($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 8)), _MM_UPCONV_EPI32_NONE, _MM_HINT_NONE, 8*4);
        break;
      case 0x000F:
        $ReturnVarName = _mm512_mask_check_load_epi32($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 4)), _MM_UPCONV_EPI32_NONE, _MM_HINT_NONE, 4*4);
        break;
      case 0x0007:
        $ReturnVarName = _mm512_mask_check_load_epi32($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 3)), _MM_UPCONV_EPI32_NONE, _MM_HINT_NONE, 3*4);
        break;
      case 0x0003:
        $ReturnVarName = _mm512_mask_check_load_epi32($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 2)), _MM_UPCONV_EPI32_NONE, _MM_HINT_NONE, 2*4);
        break;
      case 0x0001:
        $ReturnVarName = _mm512_extload_epi32((void const *)($Arg2VarName + $Arg1VarName), _MM_UPCONV_EPI32_NONE, _MM_BROADCAST_1X16, _MM_HINT_NONE);
        break;
    }
    return as_$ReturnType($ReturnVarName);
  }];
OclBuiltinImpl mask_vload_gas_v16_i32 = OclBuiltinImpl<mask_vload_gas, [v16i32, v16u32], 0, MaskVLoadV16UI32>;
OclBuiltinImpl mask_vload_cas_v16_i32 = OclBuiltinImpl<mask_vload_cas, [v16i32, v16u32], 0, MaskVLoadV16UI32>;
OclBuiltinImpl mask_vload_las_v16_i32 = OclBuiltinImpl<mask_vload_las, [v16i32, v16u32], 0, MaskVLoadV16UI32>;
OclBuiltinImpl mask_vload_pas_v16_i32 = OclBuiltinImpl<mask_vload_pas, [v16i32, v16u32], 0, MaskVLoadV16UI32>;

code MaskVLoadV16F32 = [{
    $ReturnType $ReturnVarName = _mm512_undefined_ps();
    switch ($Arg0VarName) {
      case 0xFFFF:
        $ReturnVarName = _mm512_check_load_ps($ReturnVarName, (void const *)($Arg2VarName + ($Arg1VarName * 16)), _MM_UPCONV_PS_NONE, _MM_HINT_NONE);
        break;
      case 0x00FF:
        $ReturnVarName = _mm512_mask_check_load_ps($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 8)), _MM_UPCONV_PS_NONE, _MM_HINT_NONE, 8*4);
        break;
      case 0x000F:
        $ReturnVarName = _mm512_mask_check_load_ps($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 4)), _MM_UPCONV_PS_NONE, _MM_HINT_NONE, 4*4);
        break;
      case 0x0007:
        $ReturnVarName = _mm512_mask_check_load_ps($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 3)), _MM_UPCONV_PS_NONE, _MM_HINT_NONE, 3*4);
        break;
      case 0x0003:
        $ReturnVarName = _mm512_mask_check_load_ps($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 2)), _MM_UPCONV_PS_NONE, _MM_HINT_NONE, 2*4);
        break;
      case 0x0001:
        $ReturnVarName = _mm512_extload_ps((void const *)($Arg2VarName + $Arg1VarName), _MM_UPCONV_PS_NONE, _MM_BROADCAST_1X16, _MM_HINT_NONE);
        break;
    }
    return $ReturnVarName;
  }];
OclBuiltinImpl mask_vload_gas_v16_f32 = OclBuiltinImpl<mask_vload_gas, [v16f32], 0, MaskVLoadV16F32>;
OclBuiltinImpl mask_vload_cas_v16_f32 = OclBuiltinImpl<mask_vload_cas, [v16f32], 0, MaskVLoadV16F32>;
OclBuiltinImpl mask_vload_las_v16_f32 = OclBuiltinImpl<mask_vload_las, [v16f32], 0, MaskVLoadV16F32>;
OclBuiltinImpl mask_vload_pas_v16_f32 = OclBuiltinImpl<mask_vload_pas, [v16f32], 0, MaskVLoadV16F32>;

code MaskVLoadV8UI64 = [{
    __m512i $ReturnVarName = _mm512_undefined_epi64();
    switch ($Arg0VarName) {
      case 0xFF:
        $ReturnVarName = _mm512_check_load_epi64($ReturnVarName, (void const *)($Arg2VarName + ($Arg1VarName * 8)), _MM_UPCONV_EPI64_NONE, _MM_HINT_NONE);
        break;
      case 0x0F:
        $ReturnVarName = _mm512_mask_check_load_epi64($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 4)), _MM_UPCONV_EPI64_NONE, _MM_HINT_NONE, 4*8);
        break;
      case 0x07:
        $ReturnVarName = _mm512_mask_check_load_epi64($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 3)), _MM_UPCONV_EPI64_NONE, _MM_HINT_NONE, 3*8);
        break;
      case 0x03:
        $ReturnVarName = _mm512_mask_check_load_epi64($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 2)), _MM_UPCONV_EPI64_NONE, _MM_HINT_NONE, 2*8);
        break;
      case 0x01:
        $ReturnVarName = _mm512_extload_epi64((void const *)($Arg2VarName + $Arg1VarName), _MM_UPCONV_EPI64_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);
        break;
    }
    return as_$ReturnType($ReturnVarName);
  }];
OclBuiltinImpl mask_vload_gas_v8_ui64 = OclBuiltinImpl<mask_vload_gas, [v8i64, v8u64], 0, MaskVLoadV8UI64>;
OclBuiltinImpl mask_vload_cas_v8_ui64 = OclBuiltinImpl<mask_vload_cas, [v8i64, v8u64], 0, MaskVLoadV8UI64>;
OclBuiltinImpl mask_vload_las_v8_ui64 = OclBuiltinImpl<mask_vload_las, [v8i64, v8u64], 0, MaskVLoadV8UI64>;
OclBuiltinImpl mask_vload_pas_v8_ui64 = OclBuiltinImpl<mask_vload_pas, [v8i64, v8u64], 0, MaskVLoadV8UI64>;

code MaskVLoadV8F64 = [{
    $ReturnType $ReturnVarName = _mm512_undefined_pd();
    switch ($Arg0VarName) {
      case 0xFF:
        $ReturnVarName = _mm512_check_load_pd($ReturnVarName, (void const *)($Arg2VarName + ($Arg1VarName * 8)), _MM_UPCONV_PD_NONE, _MM_HINT_NONE);
        break;
      case 0x0F:
        $ReturnVarName = _mm512_mask_check_load_pd($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 4)), _MM_UPCONV_PD_NONE, _MM_HINT_NONE, 4*8);
        break;
      case 0x07:
        $ReturnVarName = _mm512_mask_check_load_pd($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 3)), _MM_UPCONV_PD_NONE, _MM_HINT_NONE, 3*8);
        break;
      case 0x03:
        $ReturnVarName = _mm512_mask_check_load_pd($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 2)), _MM_UPCONV_PD_NONE, _MM_HINT_NONE, 2*8);
        break;
      case 0x01:
        $ReturnVarName = _mm512_extload_pd((void const *)($Arg2VarName + $Arg1VarName), _MM_UPCONV_PD_NONE, _MM_BROADCAST_1X8, _MM_HINT_NONE);
        break;
    }
    return $ReturnVarName;
  }];
OclBuiltinImpl mask_vload_gas_v8_f64 = OclBuiltinImpl<mask_vload_gas, [v8f64], 0, MaskVLoadV8F64>;
OclBuiltinImpl mask_vload_cas_v8_f64 = OclBuiltinImpl<mask_vload_cas, [v8f64], 0, MaskVLoadV8F64>;
OclBuiltinImpl mask_vload_las_v8_f64 = OclBuiltinImpl<mask_vload_las, [v8f64], 0, MaskVLoadV8F64>;
OclBuiltinImpl mask_vload_pas_v8_f64 = OclBuiltinImpl<mask_vload_pas, [v8f64], 0, MaskVLoadV8F64>;

code VLoadPromote = [{
    $NativeReturnType $ReturnVarName = mask_vload($Mask, $Arg0VarName, $Arg1VarName);
    return $ReturnVarName$Pattern;
  }];
code VLoadExpand = [{
    $ReturnType $ReturnVarName;
    $ReturnVarName$ExpandLoPattern = $ExpandLoFunc(0, $Arg1VarName + ($Arg0VarName * $VecLength));
    $ReturnVarName$ExpandHiPattern = $ExpandHiFunc(0, $Arg1VarName + ($Arg0VarName * $VecLength) + $VecLength/2);
    return $ReturnVarName;
  }];
OclBuiltinImpl vload_gas_promote  = OclBuiltinImpl<vload_gas, non_v16x64_types, 0, VLoadPromote>;
OclBuiltinImpl vload_cas_promote  = OclBuiltinImpl<vload_cas, non_v16x64_types, 0, VLoadPromote>;
OclBuiltinImpl vload_las_promote  = OclBuiltinImpl<vload_las, non_v16x64_types, 0, VLoadPromote>;
OclBuiltinImpl vload_pas_promote  = OclBuiltinImpl<vload_pas, non_v16x64_types, 0, VLoadPromote>;
OclBuiltinImpl vload_gas_expand   = OclBuiltinImpl<vload_gas, all_v16x64_types, 0, VLoadExpand>;
OclBuiltinImpl vload_cas_expand   = OclBuiltinImpl<vload_cas, all_v16x64_types, 0, VLoadExpand>;
OclBuiltinImpl vload_las_expand   = OclBuiltinImpl<vload_las, all_v16x64_types, 0, VLoadExpand>;
OclBuiltinImpl vload_pas_expand   = OclBuiltinImpl<vload_pas, all_v16x64_types, 0, VLoadExpand>;

// vstore
code MaskVStoreV16U8 = [{
    __m512i tmp = (__m512i)convert_uint16($Arg1VarName);
    switch ($Arg0VarName) {
        case 0xFFFF:
             _mm512_check_store_epi32((void *)($Arg3VarName + ($Arg2VarName * 16)), tmp, _MM_DOWNCONV_EPI32_UINT8, _MM_HINT_NONE);
            break;
        case 0x00FF:
             _mm512_mask_check_store_epi32((void *)($Arg3VarName + ($Arg2VarName * 8)), $Arg0VarName, tmp, _MM_DOWNCONV_EPI32_UINT8, _MM_HINT_NONE, 1*8);
            break;
        case 0x000F:
            _mm512_mask_check_store_epi32((void *)($Arg3VarName + ($Arg2VarName * 4)), $Arg0VarName, tmp, _MM_DOWNCONV_EPI32_UINT8, _MM_HINT_NONE, 1*4);
            break;
        case 0x0007:
            _mm512_mask_check_store_epi32((void *)($Arg3VarName + ($Arg2VarName * 3)), $Arg0VarName, tmp, _MM_DOWNCONV_EPI32_UINT8, _MM_HINT_NONE, 1*3);
            break;
        case 0x0003:
            _mm512_mask_check_store_epi32((void *)($Arg3VarName + ($Arg2VarName * 2)), $Arg0VarName, tmp, _MM_DOWNCONV_EPI32_UINT8, _MM_HINT_NONE, 1*2);
            break;
        case 0x0001:
            _mm512_extstore_epi32((void *)($Arg3VarName + ($Arg2VarName * 1)), tmp, _MM_DOWNCONV_EPI32_UINT8, _MM_HINT_NONE);
            break;
    }
  }];
OclBuiltinImpl mask_vstore_gas_v16_u8 = OclBuiltinImpl<mask_vstore_gas, [v16u8], 0, MaskVStoreV16U8>;
OclBuiltinImpl mask_vstore_las_v16_u8 = OclBuiltinImpl<mask_vstore_las, [v16u8], 0, MaskVStoreV16U8>;
OclBuiltinImpl mask_vstore_pas_v16_u8 = OclBuiltinImpl<mask_vstore_pas, [v16u8], 0, MaskVStoreV16U8>;

code MaskVStoreV16I8 = [{
    __m512i tmp = (__m512i)convert_int16($Arg1VarName);
    switch ($Arg0VarName) {
        case 0xFFFF:
            _mm512_check_store_epi32((void *)($Arg3VarName + ($Arg2VarName * 16)), tmp, _MM_DOWNCONV_EPI32_SINT8, _MM_HINT_NONE);
            break;
        case 0x00FF:
            _mm512_mask_check_store_epi32((void *)($Arg3VarName + ($Arg2VarName * 8)), $Arg0VarName, tmp, _MM_DOWNCONV_EPI32_SINT8, _MM_HINT_NONE, 1*8);
            break;
        case 0x000F:
            _mm512_mask_check_store_epi32((void *)($Arg3VarName + ($Arg2VarName * 4)), $Arg0VarName, tmp, _MM_DOWNCONV_EPI32_SINT8, _MM_HINT_NONE, 1*4);
            break;
        case 0x0007:
            _mm512_mask_check_store_epi32((void *)($Arg3VarName + ($Arg2VarName * 3)), $Arg0VarName, tmp, _MM_DOWNCONV_EPI32_SINT8, _MM_HINT_NONE, 1*3);
            break;
        case 0x0003:
            _mm512_mask_check_store_epi32((void *)($Arg3VarName + ($Arg2VarName * 2)), $Arg0VarName, tmp, _MM_DOWNCONV_EPI32_SINT8, _MM_HINT_NONE, 1*2);
            break;
        case 0x0001:
            _mm512_extstore_epi32((void *)($Arg3VarName + ($Arg2VarName * 1)), tmp, _MM_DOWNCONV_EPI32_SINT8, _MM_HINT_NONE);
            break;
    }
  }];
OclBuiltinImpl mask_vstore_gas_v16_i8 = OclBuiltinImpl<mask_vstore_gas, [v16i8], 0, MaskVStoreV16I8>;
OclBuiltinImpl mask_vstore_las_v16_i8 = OclBuiltinImpl<mask_vstore_las, [v16i8], 0, MaskVStoreV16I8>;
OclBuiltinImpl mask_vstore_pas_v16_i8 = OclBuiltinImpl<mask_vstore_pas, [v16i8], 0, MaskVStoreV16I8>;

code MaskVStoreV16U16 = [{
    __m512i tmp = (__m512i)convert_uint16($Arg1VarName);
    switch ($Arg0VarName) {
        case 0xFFFF:
            _mm512_check_store_epi32((void *)($Arg3VarName + ($Arg2VarName * 16)), tmp, _MM_DOWNCONV_EPI32_UINT16, _MM_HINT_NONE);
            break;
        case 0x00FF:
            _mm512_mask_check_store_epi32((void *)($Arg3VarName + ($Arg2VarName * 8)), $Arg0VarName, tmp, _MM_DOWNCONV_EPI32_UINT16, _MM_HINT_NONE, 2*8);
            break;
        case 0x000F:
            _mm512_mask_check_store_epi32((void *)($Arg3VarName + ($Arg2VarName * 4)), $Arg0VarName, tmp, _MM_DOWNCONV_EPI32_UINT16, _MM_HINT_NONE, 2*4);
            break;
        case 0x0007:
            _mm512_mask_check_store_epi32((void *)($Arg3VarName + ($Arg2VarName * 3)), $Arg0VarName, tmp, _MM_DOWNCONV_EPI32_UINT16, _MM_HINT_NONE, 2*3);
            break;
        case 0x0003:
            _mm512_mask_check_store_epi32((void *)($Arg3VarName + ($Arg2VarName * 2)), $Arg0VarName, tmp, _MM_DOWNCONV_EPI32_UINT16, _MM_HINT_NONE,2*2);
            break;
        case 0x0001:
            _mm512_extstore_epi32((void *)($Arg3VarName + ($Arg2VarName * 1)), tmp, _MM_DOWNCONV_EPI32_UINT16, _MM_HINT_NONE);
            break;
    }
  }];
OclBuiltinImpl mask_vstore_gas_v16_u16 = OclBuiltinImpl<mask_vstore_gas, [v16u16], 0, MaskVStoreV16U16>;
OclBuiltinImpl mask_vstore_las_v16_u16 = OclBuiltinImpl<mask_vstore_las, [v16u16], 0, MaskVStoreV16U16>;
OclBuiltinImpl mask_vstore_pas_v16_u16 = OclBuiltinImpl<mask_vstore_pas, [v16u16], 0, MaskVStoreV16U16>;

code MaskVStoreV16I16 = [{
    __m512i tmp = (__m512i)convert_int16($Arg1VarName);
    switch ($Arg0VarName) {
        case 0xFFFF:
            _mm512_check_store_epi32((void *)($Arg3VarName + ($Arg2VarName * 16)), tmp, _MM_DOWNCONV_EPI32_SINT16, _MM_HINT_NONE);
            break;
        case 0x00FF:
            _mm512_mask_check_store_epi32((void *)($Arg3VarName + ($Arg2VarName * 8)), $Arg0VarName, tmp, _MM_DOWNCONV_EPI32_SINT16, _MM_HINT_NONE, 2*8);
            break;
        case 0x000F:
            _mm512_mask_check_store_epi32((void *)($Arg3VarName + ($Arg2VarName * 4)), $Arg0VarName, tmp, _MM_DOWNCONV_EPI32_SINT16, _MM_HINT_NONE, 2*4);
            break;
        case 0x0007:
            _mm512_mask_check_store_epi32 ((void *)($Arg3VarName + ($Arg2VarName * 3)), $Arg0VarName, tmp, _MM_DOWNCONV_EPI32_SINT16, _MM_HINT_NONE, 2*3);
            break;
        case 0x0003:
            _mm512_mask_check_store_epi32((void *)($Arg3VarName + ($Arg2VarName * 2)), $Arg0VarName, tmp, _MM_DOWNCONV_EPI32_SINT16, _MM_HINT_NONE, 2*2);
            break;
        case 0x0001:
            _mm512_extstore_epi32((void *)($Arg3VarName + ($Arg2VarName * 1)), tmp, _MM_DOWNCONV_EPI32_SINT16, _MM_HINT_NONE);
            break;
    }
  }];
OclBuiltinImpl mask_vstore_gas_v16_i16 = OclBuiltinImpl<mask_vstore_gas, [v16i16], 0, MaskVStoreV16I16>;
OclBuiltinImpl mask_vstore_las_v16_i16 = OclBuiltinImpl<mask_vstore_las, [v16i16], 0, MaskVStoreV16I16>;
OclBuiltinImpl mask_vstore_pas_v16_i16 = OclBuiltinImpl<mask_vstore_pas, [v16i16], 0, MaskVStoreV16I16>;

code MaskVStoreV16UIF32 = [{
    switch ($Arg0VarName) {
        case 0xFFFF:
            _mm512_check_store_ps((void *)($Arg3VarName + ($Arg2VarName * 16)), (__m512)$Arg1VarName, _MM_DOWNCONV_PS_NONE, _MM_HINT_NONE);
            break;
        case 0x00FF:
            _mm512_mask_check_store_ps((void *)($Arg3VarName + ($Arg2VarName * 8)), $Arg0VarName, (__m512)$Arg1VarName, _MM_DOWNCONV_PS_NONE, _MM_HINT_NONE, 4*8);
            break;
        case 0x000F:
            _mm512_mask_check_store_ps((void *)($Arg3VarName + ($Arg2VarName * 4)), $Arg0VarName, (__m512)$Arg1VarName, _MM_DOWNCONV_PS_NONE, _MM_HINT_NONE, 4*4);
            break;
        case 0x0007:
            _mm512_mask_check_store_ps((void *)($Arg3VarName + ($Arg2VarName * 3)), $Arg0VarName, (__m512)$Arg1VarName, _MM_DOWNCONV_PS_NONE, _MM_HINT_NONE, 4*3);
            break;
        case 0x0003:
            _mm512_mask_check_store_ps((void *)($Arg3VarName + ($Arg2VarName * 2)), $Arg0VarName, (__m512)$Arg1VarName, _MM_DOWNCONV_PS_NONE, _MM_HINT_NONE, 4*2);
            break;
        case 0x0001:
            _mm512_extstore_ps((void *)($Arg3VarName + ($Arg2VarName * 1)), (__m512)$Arg1VarName, _MM_DOWNCONV_PS_NONE, _MM_HINT_NONE);
            break;
    }
  }];
OclBuiltinImpl mask_vstore_gas_v16_uif32 = OclBuiltinImpl<mask_vstore_gas, [v16i32, v16u32, v16f32], 0, MaskVStoreV16UIF32>;
OclBuiltinImpl mask_vstore_las_v16_uif32 = OclBuiltinImpl<mask_vstore_las, [v16i32, v16u32, v16f32], 0, MaskVStoreV16UIF32>;
OclBuiltinImpl mask_vstore_pas_v16_uif32 = OclBuiltinImpl<mask_vstore_pas, [v16i32, v16u32, v16f32], 0, MaskVStoreV16UIF32>;

code MaskVStoreV8UIF64 = [{
    switch ($Arg0VarName) {
        case 0xFF:
            _mm512_check_store_pd((void *)($Arg3VarName + ($Arg2VarName * 8)), (__m512d)$Arg1VarName, _MM_DOWNCONV_PD_NONE, _MM_HINT_NONE);
            break;
        case 0x0F:
             _mm512_mask_check_store_pd((void *)($Arg3VarName + ($Arg2VarName * 4)), $Arg0VarName, (__m512d)$Arg1VarName, _MM_DOWNCONV_PD_NONE, _MM_HINT_NONE, 8*4);
            break;
        case 0x07:
            _mm512_mask_check_store_pd((void *)($Arg3VarName + ($Arg2VarName * 3)), $Arg0VarName, (__m512d)$Arg1VarName, _MM_DOWNCONV_PD_NONE, _MM_HINT_NONE, 8*3);
            break;
        case 0x03:
            _mm512_mask_check_store_pd((void *)($Arg3VarName + ($Arg2VarName * 2)), $Arg0VarName, (__m512d)$Arg1VarName, _MM_DOWNCONV_PD_NONE, _MM_HINT_NONE, 8*2);
            break;
        case 0x01:
            _mm512_extstore_pd((void *)($Arg3VarName + ($Arg2VarName * 1)), (__m512d)$Arg1VarName, _MM_DOWNCONV_PD_NONE, _MM_HINT_NONE);
            break;
    }
  }];
OclBuiltinImpl mask_vstore_gas_v8_uif64 = OclBuiltinImpl<mask_vstore_gas, [v8i64, v8u64, v8f64], 0, MaskVStoreV8UIF64>;
OclBuiltinImpl mask_vstore_las_v8_uif64 = OclBuiltinImpl<mask_vstore_las, [v8i64, v8u64, v8f64], 0, MaskVStoreV8UIF64>;
OclBuiltinImpl mask_vstore_pas_v8_uif64 = OclBuiltinImpl<mask_vstore_pas, [v8i64, v8u64, v8f64], 0, MaskVStoreV8UIF64>;

code VStorePromote = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    mask_vstore($Mask, n$Arg0VarName, $Arg1VarName, $Arg2VarName);
  }];
code VStoreExpand = [{
    $ExpandLoFunc($Arg0VarName$ExpandLoPattern, 0, $Arg2VarName + ($Arg1VarName * $VecLength));
    $ExpandHiFunc($Arg0VarName$ExpandHiPattern, 0, $Arg2VarName + ($Arg1VarName * $VecLength) + $VecLength/2);
  }];
OclBuiltinImpl vstore_gas_promote = OclBuiltinImpl<vstore_gas, non_v16x64_types, 0, VStorePromote>;
OclBuiltinImpl vstore_las_promote = OclBuiltinImpl<vstore_las, non_v16x64_types, 0, VStorePromote>;
OclBuiltinImpl vstore_pas_promote = OclBuiltinImpl<vstore_pas, non_v16x64_types, 0, VStorePromote>;
OclBuiltinImpl vstore_gas_expand  = OclBuiltinImpl<vstore_gas, all_v16x64_types, 0, VStoreExpand>;
OclBuiltinImpl vstore_las_expand  = OclBuiltinImpl<vstore_las, all_v16x64_types, 0, VStoreExpand>;
OclBuiltinImpl vstore_pas_expand  = OclBuiltinImpl<vstore_pas, all_v16x64_types, 0, VStoreExpand>;

// vload_half
code MaskVLoadHalf = [{
    $ReturnType $ReturnVarName = _mm512_undefined_ps();
    switch ($Arg0VarName) {
        case 0xFFFF:
            $ReturnVarName = _mm512_check_load_ps($ReturnVarName, (void const *)($Arg2VarName + ($Arg1VarName * 16)), _MM_UPCONV_PS_FLOAT16, _MM_HINT_NONE);
            break;
        case 0x00FF:
            $ReturnVarName = _mm512_mask_check_load_ps($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 8)), _MM_UPCONV_PS_FLOAT16, _MM_HINT_NONE, 8*2);
            break;
        case 0x000F:
            $ReturnVarName = _mm512_mask_check_load_ps($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 4)), _MM_UPCONV_PS_FLOAT16, _MM_HINT_NONE, 4*2);
            break;
        case 0x0007:
            $ReturnVarName = _mm512_mask_check_load_ps($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 3)), _MM_UPCONV_PS_FLOAT16, _MM_HINT_NONE, 3*2);
            break;
        case 0x0003:
            $ReturnVarName = _mm512_mask_check_load_ps($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 2)), _MM_UPCONV_PS_FLOAT16, _MM_HINT_NONE, 2*2);
            break;
        case 0x0001:
            $ReturnVarName = _mm512_extload_ps((void const *)($Arg2VarName + $Arg1VarName), _MM_UPCONV_PS_FLOAT16, _MM_BROADCAST_1X16, _MM_HINT_NONE);
            break;
    }
    return $ReturnVarName;
  }];
code MaskVLoadAHalf = [{
    $ReturnType $ReturnVarName = _mm512_undefined_ps();
    switch ($Arg0VarName) {
        case 0xFFFF:
            $ReturnVarName = _mm512_check_load_ps($ReturnVarName, (void const *)($Arg2VarName + ($Arg1VarName * 16)), _MM_UPCONV_PS_FLOAT16, _MM_HINT_NONE);
            break;
        case 0x00FF:
            $ReturnVarName = _mm512_mask_check_load_ps($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 8)), _MM_UPCONV_PS_FLOAT16, _MM_HINT_NONE, 8*2);
            break;
        case 0x000F:
            $ReturnVarName = _mm512_mask_check_load_ps($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 4)), _MM_UPCONV_PS_FLOAT16, _MM_HINT_NONE, 4*2);
            break;
        case 0x0007:
            $ReturnVarName = _mm512_mask_check_load_ps($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 4)), _MM_UPCONV_PS_FLOAT16, _MM_HINT_NONE, 3*2);
            break;
        case 0x0003:
            $ReturnVarName = _mm512_mask_check_load_ps($ReturnVarName, $Arg0VarName, (void const *)($Arg2VarName + ($Arg1VarName * 2)), _MM_UPCONV_PS_FLOAT16, _MM_HINT_NONE, 2*2);
            break;
        case 0x0001:
            $ReturnVarName = _mm512_extload_ps((void const *)($Arg2VarName + $Arg1VarName), _MM_UPCONV_PS_FLOAT16, _MM_BROADCAST_1X16, _MM_HINT_NONE);
            break;
    }
    return $ReturnVarName;
  }];
OclBuiltinImpl mask_vload_half_gas_v16 = OclBuiltinImpl<mask_vload_half_gas, [v16f16], 0, MaskVLoadHalf>;
OclBuiltinImpl mask_vload_half_cas_v16 = OclBuiltinImpl<mask_vload_half_cas, [v16f16], 0, MaskVLoadHalf>;
OclBuiltinImpl mask_vload_half_las_v16 = OclBuiltinImpl<mask_vload_half_las, [v16f16], 0, MaskVLoadHalf>;
OclBuiltinImpl mask_vload_half_pas_v16 = OclBuiltinImpl<mask_vload_half_pas, [v16f16], 0, MaskVLoadHalf>;
OclBuiltinImpl mask_vloada_half_gas_v16 = OclBuiltinImpl<mask_vloada_half_gas, [v16f16], 0, MaskVLoadAHalf>;
OclBuiltinImpl mask_vloada_half_cas_v16 = OclBuiltinImpl<mask_vloada_half_cas, [v16f16], 0, MaskVLoadAHalf>;
OclBuiltinImpl mask_vloada_half_las_v16 = OclBuiltinImpl<mask_vloada_half_las, [v16f16], 0, MaskVLoadAHalf>;
OclBuiltinImpl mask_vloada_half_pas_v16 = OclBuiltinImpl<mask_vloada_half_pas, [v16f16], 0, MaskVLoadAHalf>;

code VLoadHalfPromote = [{
    $NativeReturnType $ReturnVarName = mask_vload_half($Mask, $Arg0VarName, $Arg1VarName);
    return $ReturnVarName$Pattern;
  }];
code VLoadAHalfPromote = [{
    $NativeReturnType $ReturnVarName = mask_vloada_half($Mask, $Arg0VarName, $Arg1VarName);
    return $ReturnVarName$Pattern;
  }];
OclBuiltinImpl vload_half_gas_promote = OclBuiltinImpl<vload_half_gas, vload_half_gas.Types, 0, VLoadHalfPromote>;
OclBuiltinImpl vload_half_cas_promote = OclBuiltinImpl<vload_half_cas, vload_half_cas.Types, 0, VLoadHalfPromote>;
OclBuiltinImpl vload_half_las_promote = OclBuiltinImpl<vload_half_las, vload_half_las.Types, 0, VLoadHalfPromote>;
OclBuiltinImpl vload_half_pas_promote = OclBuiltinImpl<vload_half_pas, vload_half_pas.Types, 0, VLoadHalfPromote>;
OclBuiltinImpl vloada_half_gas_promote = OclBuiltinImpl<vloada_half_gas, vloada_half_gas.Types, 0, VLoadAHalfPromote>;
OclBuiltinImpl vloada_half_cas_promote = OclBuiltinImpl<vloada_half_cas, vloada_half_cas.Types, 0, VLoadAHalfPromote>;
OclBuiltinImpl vloada_half_las_promote = OclBuiltinImpl<vloada_half_las, vloada_half_las.Types, 0, VLoadAHalfPromote>;
OclBuiltinImpl vloada_half_pas_promote = OclBuiltinImpl<vloada_half_pas, vloada_half_pas.Types, 0, VLoadAHalfPromote>;

// vstoref_half/vstored_half
code MaskVStoreFHalf = [{
    mask_vstore_half_rte($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName);
  }];
code MaskVStoreFHalfRTE = [{
    unsigned int old = set_rounding_mode(_MM_ROUND_NEAREST);
    mask_vstore_half_final($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName);
    set_rounding_mode(old);
  }];
code MaskVStoreFHalfRTZ = [{
    unsigned int old = set_rounding_mode(_MM_ROUND_TOWARD_ZERO);
    mask_vstore_half_final($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName);
    set_rounding_mode(old);
  }];
code MaskVStoreFHalfRTP = [{
    unsigned int old = set_rounding_mode(_MM_ROUND_UP);
    mask_vstore_half_final($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName);
    set_rounding_mode(old);
  }];

code MaskVStoreFHalfRTN = [{
    unsigned int old = set_rounding_mode(_MM_ROUND_DOWN);
    mask_vstore_half_final($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName);
    set_rounding_mode(old);
  }];

code MaskVStoreFHalfFinal = [{
    switch ($Arg0VarName) {
        case 0xFFFF:
            _mm512_check_store_ps((void *)($Arg3VarName + ($Arg2VarName * 16)), $Arg1VarName, _MM_DOWNCONV_PS_FLOAT16, _MM_HINT_NONE);
            break;
        case 0x00FF:
            _mm512_mask_check_store_ps((void *)($Arg3VarName + ($Arg2VarName * 8)), $Arg0VarName, $Arg1VarName, _MM_DOWNCONV_PS_FLOAT16, _MM_HINT_NONE, 2*8);
            break;
        case 0x000F:
            _mm512_mask_check_store_ps((void *)($Arg3VarName + ($Arg2VarName * 4)), $Arg0VarName, $Arg1VarName, _MM_DOWNCONV_PS_FLOAT16, _MM_HINT_NONE, 2*4);
            break;
        case 0x0007:
            _mm512_mask_check_store_ps((void *)($Arg3VarName + ($Arg2VarName * 3)), $Arg0VarName, $Arg1VarName, _MM_DOWNCONV_PS_FLOAT16, _MM_HINT_NONE, 2*3);
            break;
        case 0x0003:
            _mm512_mask_check_store_ps((void *)($Arg3VarName + ($Arg2VarName * 2)), $Arg0VarName, $Arg1VarName, _MM_DOWNCONV_PS_FLOAT16, _MM_HINT_NONE, 2*2);
            break;
        case 0x0001:
            _mm512_mask_check_store_ps((void *)($Arg3VarName + ($Arg2VarName * 1)), $Arg0VarName, $Arg1VarName, _MM_DOWNCONV_PS_FLOAT16, _MM_HINT_NONE, 2*1 );
            break; 
    }
  }];

OclBuiltinImpl mask_vstoref_half_final_gas_v16 = OclBuiltinImpl<mask_vstoref_half_final_gas,     [v16f16], 0, MaskVStoreFHalfFinal>;
OclBuiltinImpl mask_vstoref_half_final_las_v16 = OclBuiltinImpl<mask_vstoref_half_final_las,     [v16f16], 0, MaskVStoreFHalfFinal>;
OclBuiltinImpl mask_vstoref_half_final_pas_v16 = OclBuiltinImpl<mask_vstoref_half_final_pas,     [v16f16], 0, MaskVStoreFHalfFinal>;

OclBuiltinImpl mask_vstoref_half_gas_v16      = OclBuiltinImpl<mask_vstoref_half_gas,     [v16f16], 0, MaskVStoreFHalf>;
OclBuiltinImpl mask_vstoref_half_las_v16      = OclBuiltinImpl<mask_vstoref_half_las,     [v16f16], 0, MaskVStoreFHalf>;
OclBuiltinImpl mask_vstoref_half_pas_v16      = OclBuiltinImpl<mask_vstoref_half_pas,     [v16f16], 0, MaskVStoreFHalf>;
OclBuiltinImpl mask_vstoref_half_rte_gas_v16  = OclBuiltinImpl<mask_vstoref_half_rte_gas, [v16f16], 0, MaskVStoreFHalfRTE>;
OclBuiltinImpl mask_vstoref_half_rte_las_v16  = OclBuiltinImpl<mask_vstoref_half_rte_las, [v16f16], 0, MaskVStoreFHalfRTE>;
OclBuiltinImpl mask_vstoref_half_rte_pas_v16  = OclBuiltinImpl<mask_vstoref_half_rte_pas, [v16f16], 0, MaskVStoreFHalfRTE>;
OclBuiltinImpl mask_vstoref_half_rtz_gas_v16  = OclBuiltinImpl<mask_vstoref_half_rtz_gas, [v16f16], 0, MaskVStoreFHalfRTZ>;
OclBuiltinImpl mask_vstoref_half_rtz_las_v16  = OclBuiltinImpl<mask_vstoref_half_rtz_las, [v16f16], 0, MaskVStoreFHalfRTZ>;
OclBuiltinImpl mask_vstoref_half_rtz_pas_v16  = OclBuiltinImpl<mask_vstoref_half_rtz_pas, [v16f16], 0, MaskVStoreFHalfRTZ>;
OclBuiltinImpl mask_vstoref_half_rtp_gas_v16  = OclBuiltinImpl<mask_vstoref_half_rtp_gas, [v16f16], 0, MaskVStoreFHalfRTP>;
OclBuiltinImpl mask_vstoref_half_rtp_las_v16  = OclBuiltinImpl<mask_vstoref_half_rtp_las, [v16f16], 0, MaskVStoreFHalfRTP>;
OclBuiltinImpl mask_vstoref_half_rtp_pas_v16  = OclBuiltinImpl<mask_vstoref_half_rtp_pas, [v16f16], 0, MaskVStoreFHalfRTP>;
OclBuiltinImpl mask_vstoref_half_rtn_gas_v16  = OclBuiltinImpl<mask_vstoref_half_rtn_gas, [v16f16], 0, MaskVStoreFHalfRTN>;
OclBuiltinImpl mask_vstoref_half_rtn_las_v16  = OclBuiltinImpl<mask_vstoref_half_rtn_las, [v16f16], 0, MaskVStoreFHalfRTN>;
OclBuiltinImpl mask_vstoref_half_rtn_pas_v16  = OclBuiltinImpl<mask_vstoref_half_rtn_pas, [v16f16], 0, MaskVStoreFHalfRTN>;

code MaskVStoreDHalf = [{
    mask_vstore_half_rte($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName);
  }];
code MaskVStoreDHalfRTE = [{
    typedef union { double8 d; long8 ix; } double8_t;
    double8_t x,j;
    float16 tmp;
    x.d = $Arg1VarName;
    j.ix = x.ix + 0x1fffffff;
    j.ix &= 0x20000000;
    x.ix |= j.ix;
    x.ix &= ~0x1fffffff;  
    tmp.lo = convert_float8_rte(x.d);
    ushort mask16 = $Arg0VarName;
    mask_vstore_half_rte(mask16, tmp, $Arg2VarName, $Arg3VarName);
  }];
code MaskVStoreDHalfRTZ = [{
    float16 tmp;
    tmp.lo = convert_float8_rtz($Arg1VarName);
    ushort mask16 = $Arg0VarName;
    mask_vstore_half_rtz(mask16, tmp, $Arg2VarName, $Arg3VarName);
  }];
code MaskVStoreDHalfRTP = [{
    float16 tmp;
    tmp.lo = convert_float8_rtp($Arg1VarName);
    ushort mask16 = $Arg0VarName;
    mask_vstore_half_rtp(mask16, tmp, $Arg2VarName, $Arg3VarName);
  }];
code MaskVStoreDHalfRTN = [{
    float16 tmp; 
    tmp.lo = convert_float8_rtn($Arg1VarName);
    ushort mask16 = $Arg0VarName;
    mask_vstore_half_rtn(mask16, tmp, $Arg2VarName, $Arg3VarName);
  }];
OclBuiltinImpl mask_vstored_half_gas_v16      = OclBuiltinImpl<mask_vstored_half_gas,     [ v8f16], 0, MaskVStoreDHalf>;
OclBuiltinImpl mask_vstored_half_las_v16      = OclBuiltinImpl<mask_vstored_half_las,     [ v8f16], 0, MaskVStoreDHalf>;
OclBuiltinImpl mask_vstored_half_pas_v16      = OclBuiltinImpl<mask_vstored_half_pas,     [ v8f16], 0, MaskVStoreDHalf>;
OclBuiltinImpl mask_vstored_half_rte_gas_v16  = OclBuiltinImpl<mask_vstored_half_rte_gas, [ v8f16], 0, MaskVStoreDHalfRTE>;
OclBuiltinImpl mask_vstored_half_rte_las_v16  = OclBuiltinImpl<mask_vstored_half_rte_las, [ v8f16], 0, MaskVStoreDHalfRTE>;
OclBuiltinImpl mask_vstored_half_rte_pas_v16  = OclBuiltinImpl<mask_vstored_half_rte_pas, [ v8f16], 0, MaskVStoreDHalfRTE>;
OclBuiltinImpl mask_vstored_half_rtz_gas_v16  = OclBuiltinImpl<mask_vstored_half_rtz_gas, [ v8f16], 0, MaskVStoreDHalfRTZ>;
OclBuiltinImpl mask_vstored_half_rtz_las_v16  = OclBuiltinImpl<mask_vstored_half_rtz_las, [ v8f16], 0, MaskVStoreDHalfRTZ>;
OclBuiltinImpl mask_vstored_half_rtz_pas_v16  = OclBuiltinImpl<mask_vstored_half_rtz_pas, [ v8f16], 0, MaskVStoreDHalfRTZ>;
OclBuiltinImpl mask_vstored_half_rtp_gas_v16  = OclBuiltinImpl<mask_vstored_half_rtp_gas, [ v8f16], 0, MaskVStoreDHalfRTP>;
OclBuiltinImpl mask_vstored_half_rtp_las_v16  = OclBuiltinImpl<mask_vstored_half_rtp_las, [ v8f16], 0, MaskVStoreDHalfRTP>;
OclBuiltinImpl mask_vstored_half_rtp_pas_v16  = OclBuiltinImpl<mask_vstored_half_rtp_pas, [ v8f16], 0, MaskVStoreDHalfRTP>;
OclBuiltinImpl mask_vstored_half_rtn_gas_v16  = OclBuiltinImpl<mask_vstored_half_rtn_gas, [ v8f16], 0, MaskVStoreDHalfRTN>;
OclBuiltinImpl mask_vstored_half_rtn_las_v16  = OclBuiltinImpl<mask_vstored_half_rtn_las, [ v8f16], 0, MaskVStoreDHalfRTN>;
OclBuiltinImpl mask_vstored_half_rtn_pas_v16  = OclBuiltinImpl<mask_vstored_half_rtn_pas, [ v8f16], 0, MaskVStoreDHalfRTN>;

// vstoreaf_half/vstoread_half
code MaskVStoreAFHalf = [{
    mask_vstorea_half_rte($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName);
  }];
code MaskVStoreAFHalfRTE = [{
    unsigned int old = set_rounding_mode(_MM_ROUND_NEAREST);
    mask_vstorea_half_final($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName);
    set_rounding_mode(old);
  }];
code MaskVStoreAFHalfRTZ = [{
    unsigned int old = set_rounding_mode(_MM_ROUND_TOWARD_ZERO);
    mask_vstorea_half_final($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName);
    set_rounding_mode(old);
  }];
code MaskVStoreAFHalfRTP = [{
    unsigned int old = set_rounding_mode(_MM_ROUND_UP);
    mask_vstorea_half_final($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName);
    set_rounding_mode(old);
  }];

code MaskVStoreAFHalfRTN = [{
    unsigned int old = set_rounding_mode(_MM_ROUND_DOWN);
    mask_vstorea_half_final($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName);
    set_rounding_mode(old);
  }];

code MaskVStoreAFHalfFinal = [{
    switch ($Arg0VarName) {
        case 0xFFFF:
            _mm512_check_store_ps((void *)($Arg3VarName + ($Arg2VarName * 16)), $Arg1VarName, _MM_DOWNCONV_PS_FLOAT16, _MM_HINT_NONE);
            break;
        case 0x00FF:
            _mm512_mask_check_store_ps((void *)($Arg3VarName + ($Arg2VarName * 8)), $Arg0VarName, $Arg1VarName, _MM_DOWNCONV_PS_FLOAT16, _MM_HINT_NONE, 2*8);
            break;
        case 0x000F:
            _mm512_mask_check_store_ps((void *)($Arg3VarName + ($Arg2VarName * 4)), $Arg0VarName, $Arg1VarName, _MM_DOWNCONV_PS_FLOAT16, _MM_HINT_NONE, 2*4);
            break;
        case 0x0007:
            _mm512_mask_check_store_ps((void *)($Arg3VarName + ($Arg2VarName * 4)), $Arg0VarName, $Arg1VarName, _MM_DOWNCONV_PS_FLOAT16, _MM_HINT_NONE, 2*3);
            break;
        case 0x0003:
            _mm512_mask_check_store_ps((void *)($Arg3VarName + ($Arg2VarName * 2)), $Arg0VarName, $Arg1VarName, _MM_DOWNCONV_PS_FLOAT16, _MM_HINT_NONE, 2*2);
            break;
        case 0x0001:
            _mm512_extstore_ps((void *)($Arg3VarName + ($Arg2VarName * 1)), $Arg1VarName, _MM_DOWNCONV_PS_FLOAT16, _MM_HINT_NONE);
            break;
    }
  }];

OclBuiltinImpl mask_vstoreaf_half_final_gas_v16 = OclBuiltinImpl<mask_vstoreaf_half_final_gas,     [v16f16], 0, MaskVStoreAFHalfFinal>;
OclBuiltinImpl mask_vstoreaf_half_final_las_v16 = OclBuiltinImpl<mask_vstoreaf_half_final_las,     [v16f16], 0, MaskVStoreAFHalfFinal>;
OclBuiltinImpl mask_vstoreaf_half_final_pas_v16 = OclBuiltinImpl<mask_vstoreaf_half_final_pas,     [v16f16], 0, MaskVStoreAFHalfFinal>;

OclBuiltinImpl mask_vstoreaf_half_gas_v16     = OclBuiltinImpl<mask_vstoreaf_half_gas,      [v16f16], 0, MaskVStoreAFHalf>;
OclBuiltinImpl mask_vstoreaf_half_las_v16     = OclBuiltinImpl<mask_vstoreaf_half_las,      [v16f16], 0, MaskVStoreAFHalf>;
OclBuiltinImpl mask_vstoreaf_half_pas_v16     = OclBuiltinImpl<mask_vstoreaf_half_pas,      [v16f16], 0, MaskVStoreAFHalf>;
OclBuiltinImpl mask_vstoreaf_half_rte_gas_v16 = OclBuiltinImpl<mask_vstoreaf_half_rte_gas,  [v16f16], 0, MaskVStoreAFHalfRTE>;
OclBuiltinImpl mask_vstoreaf_half_rte_las_v16 = OclBuiltinImpl<mask_vstoreaf_half_rte_las,  [v16f16], 0, MaskVStoreAFHalfRTE>;
OclBuiltinImpl mask_vstoreaf_half_rte_pas_v16 = OclBuiltinImpl<mask_vstoreaf_half_rte_pas,  [v16f16], 0, MaskVStoreAFHalfRTE>;
OclBuiltinImpl mask_vstoreaf_half_rtz_gas_v16 = OclBuiltinImpl<mask_vstoreaf_half_rtz_gas,  [v16f16], 0, MaskVStoreAFHalfRTZ>;
OclBuiltinImpl mask_vstoreaf_half_rtz_las_v16 = OclBuiltinImpl<mask_vstoreaf_half_rtz_las,  [v16f16], 0, MaskVStoreAFHalfRTZ>;
OclBuiltinImpl mask_vstoreaf_half_rtz_pas_v16 = OclBuiltinImpl<mask_vstoreaf_half_rtz_pas,  [v16f16], 0, MaskVStoreAFHalfRTZ>;
OclBuiltinImpl mask_vstoreaf_half_rtp_gas_v16 = OclBuiltinImpl<mask_vstoreaf_half_rtp_gas,  [v16f16], 0, MaskVStoreAFHalfRTP>;
OclBuiltinImpl mask_vstoreaf_half_rtp_las_v16 = OclBuiltinImpl<mask_vstoreaf_half_rtp_las,  [v16f16], 0, MaskVStoreAFHalfRTP>;
OclBuiltinImpl mask_vstoreaf_half_rtp_pas_v16 = OclBuiltinImpl<mask_vstoreaf_half_rtp_pas,  [v16f16], 0, MaskVStoreAFHalfRTP>;
OclBuiltinImpl mask_vstoreaf_half_rtn_gas_v16 = OclBuiltinImpl<mask_vstoreaf_half_rtn_gas,  [v16f16], 0, MaskVStoreAFHalfRTN>;
OclBuiltinImpl mask_vstoreaf_half_rtn_las_v16 = OclBuiltinImpl<mask_vstoreaf_half_rtn_las,  [v16f16], 0, MaskVStoreAFHalfRTN>;
OclBuiltinImpl mask_vstoreaf_half_rtn_pas_v16 = OclBuiltinImpl<mask_vstoreaf_half_rtn_pas,  [v16f16], 0, MaskVStoreAFHalfRTN>;

code MaskVStoreADHalf = [{
    mask_vstorea_half_rte($Arg0VarName, $Arg1VarName, $Arg2VarName, $Arg3VarName);
  }];
code MaskVStoreADHalfRTE = [{
    typedef union { double8 d; long8 ix; } double8_t;
    double8_t x,j;
    float16 tmp;
    x.d = $Arg1VarName;
    j.ix = x.ix + 0x1fffffff;
    j.ix &= 0x20000000;
    x.ix |= j.ix;
    x.ix &= ~0x1fffffff;  
    tmp.lo = convert_float8_rte(x.d);
    ushort mask16 = $Arg0VarName;
    mask_vstorea_half_rte(mask16, tmp, $Arg2VarName, $Arg3VarName);
  }];
code MaskVStoreADHalfRTZ = [{
    float16 tmp;
    tmp.lo = convert_float8_rtz($Arg1VarName);
    ushort mask16 = $Arg0VarName;
    mask_vstorea_half_rtz(mask16, tmp, $Arg2VarName, $Arg3VarName);
  }];
code MaskVStoreADHalfRTP = [{
    float16 tmp;
    tmp.lo = convert_float8_rtp($Arg1VarName);
    ushort mask16 = $Arg0VarName;
    mask_vstorea_half_rtp(mask16, tmp, $Arg2VarName, $Arg3VarName);
  }];
code MaskVStoreADHalfRTN = [{
    float16 tmp;
    tmp.lo = convert_float8_rtn($Arg1VarName);
    ushort mask16 = $Arg0VarName;
    mask_vstorea_half_rtn(mask16, tmp, $Arg2VarName, $Arg3VarName);
  }];
OclBuiltinImpl mask_vstoread_half_gas_v16     = OclBuiltinImpl<mask_vstoread_half_gas,      [ v8f16], 0, MaskVStoreADHalf>;
OclBuiltinImpl mask_vstoread_half_las_v16     = OclBuiltinImpl<mask_vstoread_half_las,      [ v8f16], 0, MaskVStoreADHalf>;
OclBuiltinImpl mask_vstoread_half_pas_v16     = OclBuiltinImpl<mask_vstoread_half_pas,      [ v8f16], 0, MaskVStoreADHalf>;
OclBuiltinImpl mask_vstoread_half_rte_gas_v16 = OclBuiltinImpl<mask_vstoread_half_rte_gas,  [ v8f16], 0, MaskVStoreADHalfRTE>;
OclBuiltinImpl mask_vstoread_half_rte_las_v16 = OclBuiltinImpl<mask_vstoread_half_rte_las,  [ v8f16], 0, MaskVStoreADHalfRTE>;
OclBuiltinImpl mask_vstoread_half_rte_pas_v16 = OclBuiltinImpl<mask_vstoread_half_rte_pas,  [ v8f16], 0, MaskVStoreADHalfRTE>;
OclBuiltinImpl mask_vstoread_half_rtz_gas_v16 = OclBuiltinImpl<mask_vstoread_half_rtz_gas,  [ v8f16], 0, MaskVStoreADHalfRTZ>;
OclBuiltinImpl mask_vstoread_half_rtz_las_v16 = OclBuiltinImpl<mask_vstoread_half_rtz_las,  [ v8f16], 0, MaskVStoreADHalfRTZ>;
OclBuiltinImpl mask_vstoread_half_rtz_pas_v16 = OclBuiltinImpl<mask_vstoread_half_rtz_pas,  [ v8f16], 0, MaskVStoreADHalfRTZ>;
OclBuiltinImpl mask_vstoread_half_rtp_gas_v16 = OclBuiltinImpl<mask_vstoread_half_rtp_gas,  [ v8f16], 0, MaskVStoreADHalfRTP>;
OclBuiltinImpl mask_vstoread_half_rtp_las_v16 = OclBuiltinImpl<mask_vstoread_half_rtp_las,  [ v8f16], 0, MaskVStoreADHalfRTP>;
OclBuiltinImpl mask_vstoread_half_rtp_pas_v16 = OclBuiltinImpl<mask_vstoread_half_rtp_pas,  [ v8f16], 0, MaskVStoreADHalfRTP>;
OclBuiltinImpl mask_vstoread_half_rtn_gas_v16 = OclBuiltinImpl<mask_vstoread_half_rtn_gas,  [ v8f16], 0, MaskVStoreADHalfRTN>;
OclBuiltinImpl mask_vstoread_half_rtn_las_v16 = OclBuiltinImpl<mask_vstoread_half_rtn_las,  [ v8f16], 0, MaskVStoreADHalfRTN>;
OclBuiltinImpl mask_vstoread_half_rtn_pas_v16 = OclBuiltinImpl<mask_vstoread_half_rtn_pas,  [ v8f16], 0, MaskVStoreADHalfRTN>;

code VStoreHalfPromote = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    mask_vstore_half($Mask, n$Arg0VarName, $Arg1VarName, $Arg2VarName);
  }];
code VStoreHalfPromoteRTE = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    mask_vstore_half_rte($Mask, n$Arg0VarName, $Arg1VarName, $Arg2VarName);
  }];
code VStoreHalfPromoteRTZ = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    mask_vstore_half_rtz($Mask, n$Arg0VarName, $Arg1VarName, $Arg2VarName);
  }];
code VStoreHalfPromoteRTN = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    mask_vstore_half_rtn($Mask, n$Arg0VarName, $Arg1VarName, $Arg2VarName);
  }];
code VStoreHalfPromoteRTP = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    mask_vstore_half_rtp($Mask, n$Arg0VarName, $Arg1VarName, $Arg2VarName);
  }];
code VStoreAHalfPromote = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    mask_vstorea_half($Mask, n$Arg0VarName, $Arg1VarName, $Arg2VarName);
  }];
code VStoreAHalfPromoteRTE = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    mask_vstorea_half_rte($Mask, n$Arg0VarName, $Arg1VarName, $Arg2VarName);
  }];
code VStoreAHalfPromoteRTZ = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    mask_vstorea_half_rtz($Mask, n$Arg0VarName, $Arg1VarName, $Arg2VarName);
  }];
code VStoreAHalfPromoteRTN = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    mask_vstorea_half_rtn($Mask, n$Arg0VarName, $Arg1VarName, $Arg2VarName);
  }];
code VStoreAHalfPromoteRTP = [{
    $NativeArg0Type n$Arg0VarName; n$Arg0VarName$Pattern = $Arg0VarName;
    mask_vstorea_half_rtp($Mask, n$Arg0VarName, $Arg1VarName, $Arg2VarName);
  }];
code VStoreHalfExpand = [{
    $ExpandLoFunc($Arg0VarName$ExpandLoPattern, 0, $Arg2VarName + ($Arg1VarName * $VecLength));
    $ExpandHiFunc($Arg0VarName$ExpandHiPattern, 0, $Arg2VarName + ($Arg1VarName * $VecLength) + $VecLength/2);
  }];
OclBuiltinImpl vstoref_half_gas_promote       = OclBuiltinImpl<vstoref_half_gas,      vstoref_half_gas.Types, 0, VStoreHalfPromote>;
OclBuiltinImpl vstoref_half_las_promote       = OclBuiltinImpl<vstoref_half_las,      vstoref_half_las.Types, 0, VStoreHalfPromote>;
OclBuiltinImpl vstoref_half_pas_promote       = OclBuiltinImpl<vstoref_half_pas,      vstoref_half_pas.Types, 0, VStoreHalfPromote>;
OclBuiltinImpl vstoref_half_rte_gas_promote   = OclBuiltinImpl<vstoref_half_rte_gas,  vstoref_half_rte_gas.Types, 0, VStoreHalfPromoteRTE>;
OclBuiltinImpl vstoref_half_rte_las_promote   = OclBuiltinImpl<vstoref_half_rte_las,  vstoref_half_rte_las.Types, 0, VStoreHalfPromoteRTE>;
OclBuiltinImpl vstoref_half_rte_pas_promote   = OclBuiltinImpl<vstoref_half_rte_pas,  vstoref_half_rte_pas.Types, 0, VStoreHalfPromoteRTE>;
OclBuiltinImpl vstoref_half_rtz_gas_promote   = OclBuiltinImpl<vstoref_half_rtz_gas,  vstoref_half_rtz_gas.Types, 0, VStoreHalfPromoteRTZ>;
OclBuiltinImpl vstoref_half_rtz_las_promote   = OclBuiltinImpl<vstoref_half_rtz_las,  vstoref_half_rtz_las.Types, 0, VStoreHalfPromoteRTZ>;
OclBuiltinImpl vstoref_half_rtz_pas_promote   = OclBuiltinImpl<vstoref_half_rtz_pas,  vstoref_half_rtz_pas.Types, 0, VStoreHalfPromoteRTZ>;
OclBuiltinImpl vstoref_half_rtp_gas_promote   = OclBuiltinImpl<vstoref_half_rtp_gas,  vstoref_half_rtp_gas.Types, 0, VStoreHalfPromoteRTP>;
OclBuiltinImpl vstoref_half_rtp_las_promote   = OclBuiltinImpl<vstoref_half_rtp_las,  vstoref_half_rtp_las.Types, 0, VStoreHalfPromoteRTP>;
OclBuiltinImpl vstoref_half_rtp_pas_promote   = OclBuiltinImpl<vstoref_half_rtp_pas,  vstoref_half_rtp_pas.Types, 0, VStoreHalfPromoteRTP>;
OclBuiltinImpl vstoref_half_rtn_gas_promote   = OclBuiltinImpl<vstoref_half_rtn_gas,  vstoref_half_rtn_gas.Types, 0, VStoreHalfPromoteRTN>;
OclBuiltinImpl vstoref_half_rtn_las_promote   = OclBuiltinImpl<vstoref_half_rtn_las,  vstoref_half_rtn_las.Types, 0, VStoreHalfPromoteRTN>;
OclBuiltinImpl vstoref_half_rtn_pas_promote   = OclBuiltinImpl<vstoref_half_rtn_pas,  vstoref_half_rtn_pas.Types, 0, VStoreHalfPromoteRTN>;

OclBuiltinImpl vstored_half_gas_promote       = OclBuiltinImpl<vstored_half_gas,      [v1f16, v2f16, v3f16, v4f16, v8f16], 0, VStoreHalfPromote>;
OclBuiltinImpl vstored_half_las_promote       = OclBuiltinImpl<vstored_half_las,      [v1f16, v2f16, v3f16, v4f16, v8f16], 0, VStoreHalfPromote>;
OclBuiltinImpl vstored_half_pas_promote       = OclBuiltinImpl<vstored_half_pas,      [v1f16, v2f16, v3f16, v4f16, v8f16], 0, VStoreHalfPromote>;
OclBuiltinImpl vstored_half_rte_gas_promote   = OclBuiltinImpl<vstored_half_rte_gas,  [v1f16, v2f16, v3f16, v4f16, v8f16], 0, VStoreHalfPromoteRTE>;
OclBuiltinImpl vstored_half_rte_las_promote   = OclBuiltinImpl<vstored_half_rte_las,  [v1f16, v2f16, v3f16, v4f16, v8f16], 0, VStoreHalfPromoteRTE>;
OclBuiltinImpl vstored_half_rte_pas_promote   = OclBuiltinImpl<vstored_half_rte_pas,  [v1f16, v2f16, v3f16, v4f16, v8f16], 0, VStoreHalfPromoteRTE>;
OclBuiltinImpl vstored_half_rtz_gas_promote   = OclBuiltinImpl<vstored_half_rtz_gas,  [v1f16, v2f16, v3f16, v4f16, v8f16], 0, VStoreHalfPromoteRTZ>;
OclBuiltinImpl vstored_half_rtz_las_promote   = OclBuiltinImpl<vstored_half_rtz_las,  [v1f16, v2f16, v3f16, v4f16, v8f16], 0, VStoreHalfPromoteRTZ>;
OclBuiltinImpl vstored_half_rtz_pas_promote   = OclBuiltinImpl<vstored_half_rtz_pas,  [v1f16, v2f16, v3f16, v4f16, v8f16], 0, VStoreHalfPromoteRTZ>;
OclBuiltinImpl vstored_half_rtp_gas_promote   = OclBuiltinImpl<vstored_half_rtp_gas,  [v1f16, v2f16, v3f16, v4f16, v8f16], 0, VStoreHalfPromoteRTP>;
OclBuiltinImpl vstored_half_rtp_las_promote   = OclBuiltinImpl<vstored_half_rtp_las,  [v1f16, v2f16, v3f16, v4f16, v8f16], 0, VStoreHalfPromoteRTP>;
OclBuiltinImpl vstored_half_rtp_pas_promote   = OclBuiltinImpl<vstored_half_rtp_pas,  [v1f16, v2f16, v3f16, v4f16, v8f16], 0, VStoreHalfPromoteRTP>;
OclBuiltinImpl vstored_half_rtn_gas_promote   = OclBuiltinImpl<vstored_half_rtn_gas,  [v1f16, v2f16, v3f16, v4f16, v8f16], 0, VStoreHalfPromoteRTN>;
OclBuiltinImpl vstored_half_rtn_las_promote   = OclBuiltinImpl<vstored_half_rtn_las,  [v1f16, v2f16, v3f16, v4f16, v8f16], 0, VStoreHalfPromoteRTN>;
OclBuiltinImpl vstored_half_rtn_pas_promote   = OclBuiltinImpl<vstored_half_rtn_pas,  [v1f16, v2f16, v3f16, v4f16, v8f16], 0, VStoreHalfPromoteRTN>;

OclBuiltinImpl vstored_half_gas_expand        = OclBuiltinImpl<vstored_half_gas,      [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstored_half_las_expand        = OclBuiltinImpl<vstored_half_las,      [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstored_half_pas_expand        = OclBuiltinImpl<vstored_half_pas,      [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstored_half_rte_gas_expand    = OclBuiltinImpl<vstored_half_rte_gas,  [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstored_half_rte_las_expand    = OclBuiltinImpl<vstored_half_rte_las,  [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstored_half_rte_pas_expand    = OclBuiltinImpl<vstored_half_rte_pas,  [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstored_half_rtz_gas_expand    = OclBuiltinImpl<vstored_half_rtz_gas,  [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstored_half_rtz_las_expand    = OclBuiltinImpl<vstored_half_rtz_las,  [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstored_half_rtz_pas_expand    = OclBuiltinImpl<vstored_half_rtz_pas,  [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstored_half_rtp_gas_expand    = OclBuiltinImpl<vstored_half_rtp_gas,  [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstored_half_rtp_las_expand    = OclBuiltinImpl<vstored_half_rtp_las,  [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstored_half_rtp_pas_expand    = OclBuiltinImpl<vstored_half_rtp_pas,  [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstored_half_rtn_gas_expand    = OclBuiltinImpl<vstored_half_rtn_gas,  [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstored_half_rtn_las_expand    = OclBuiltinImpl<vstored_half_rtn_las,  [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstored_half_rtn_pas_expand    = OclBuiltinImpl<vstored_half_rtn_pas,  [v16f16], 0, VStoreHalfExpand>;

OclBuiltinImpl vstoreaf_half_gas_promote      = OclBuiltinImpl<vstoreaf_half_gas,     vstoreaf_half_gas.Types, 0, VStoreAHalfPromote>;
OclBuiltinImpl vstoreaf_half_las_promote      = OclBuiltinImpl<vstoreaf_half_las,     vstoreaf_half_las.Types, 0, VStoreAHalfPromote>;
OclBuiltinImpl vstoreaf_half_pas_promote      = OclBuiltinImpl<vstoreaf_half_pas,     vstoreaf_half_pas.Types, 0, VStoreAHalfPromote>;
OclBuiltinImpl vstoreaf_half_rte_gas_promote  = OclBuiltinImpl<vstoreaf_half_rte_gas, vstoreaf_half_gas.Types, 0, VStoreAHalfPromoteRTE>;
OclBuiltinImpl vstoreaf_half_rte_las_promote  = OclBuiltinImpl<vstoreaf_half_rte_las, vstoreaf_half_las.Types, 0, VStoreAHalfPromoteRTE>;
OclBuiltinImpl vstoreaf_half_rte_pas_promote  = OclBuiltinImpl<vstoreaf_half_rte_pas, vstoreaf_half_pas.Types, 0, VStoreAHalfPromoteRTE>;
OclBuiltinImpl vstoreaf_half_rtz_gas_promote  = OclBuiltinImpl<vstoreaf_half_rtz_gas, vstoreaf_half_gas.Types, 0, VStoreAHalfPromoteRTZ>;
OclBuiltinImpl vstoreaf_half_rtz_las_promote  = OclBuiltinImpl<vstoreaf_half_rtz_las, vstoreaf_half_las.Types, 0, VStoreAHalfPromoteRTZ>;
OclBuiltinImpl vstoreaf_half_rtz_pas_promote  = OclBuiltinImpl<vstoreaf_half_rtz_pas, vstoreaf_half_pas.Types, 0, VStoreAHalfPromoteRTZ>;
OclBuiltinImpl vstoreaf_half_rtp_gas_promote  = OclBuiltinImpl<vstoreaf_half_rtp_gas, vstoreaf_half_gas.Types, 0, VStoreAHalfPromoteRTP>;
OclBuiltinImpl vstoreaf_half_rtp_las_promote  = OclBuiltinImpl<vstoreaf_half_rtp_las, vstoreaf_half_las.Types, 0, VStoreAHalfPromoteRTP>;
OclBuiltinImpl vstoreaf_half_rtp_pas_promote  = OclBuiltinImpl<vstoreaf_half_rtp_pas, vstoreaf_half_pas.Types, 0, VStoreAHalfPromoteRTP>;
OclBuiltinImpl vstoreaf_half_rtn_gas_promote  = OclBuiltinImpl<vstoreaf_half_rtn_gas, vstoreaf_half_gas.Types, 0, VStoreAHalfPromoteRTN>;
OclBuiltinImpl vstoreaf_half_rtn_las_promote  = OclBuiltinImpl<vstoreaf_half_rtn_las, vstoreaf_half_las.Types, 0, VStoreAHalfPromoteRTN>;
OclBuiltinImpl vstoreaf_half_rtn_pas_promote  = OclBuiltinImpl<vstoreaf_half_rtn_pas, vstoreaf_half_pas.Types, 0, VStoreAHalfPromoteRTN>;

OclBuiltinImpl vstoread_half_gas_promote      = OclBuiltinImpl<vstoread_half_gas,     [v1f16, v2f16, v3f16, v4f16, v8f16], 0, VStoreAHalfPromote>;
OclBuiltinImpl vstoread_half_las_promote      = OclBuiltinImpl<vstoread_half_las,     [v1f16, v2f16, v3f16, v4f16, v8f16], 0, VStoreAHalfPromote>;
OclBuiltinImpl vstoread_half_pas_promote      = OclBuiltinImpl<vstoread_half_pas,     [v1f16, v2f16, v3f16, v4f16, v8f16], 0, VStoreAHalfPromote>;
OclBuiltinImpl vstoread_half_rte_gas_promote  = OclBuiltinImpl<vstoread_half_rte_gas, [v2f16, v3f16, v4f16, v8f16], 0, VStoreAHalfPromoteRTE>;
OclBuiltinImpl vstoread_half_rte_las_promote  = OclBuiltinImpl<vstoread_half_rte_las, [v2f16, v3f16, v4f16, v8f16], 0, VStoreAHalfPromoteRTE>;
OclBuiltinImpl vstoread_half_rte_pas_promote  = OclBuiltinImpl<vstoread_half_rte_pas, [v2f16, v3f16, v4f16, v8f16], 0, VStoreAHalfPromoteRTE>;
OclBuiltinImpl vstoread_half_rtz_gas_promote  = OclBuiltinImpl<vstoread_half_rtz_gas, [v2f16, v3f16, v4f16, v8f16], 0, VStoreAHalfPromoteRTZ>;
OclBuiltinImpl vstoread_half_rtz_las_promote  = OclBuiltinImpl<vstoread_half_rtz_las, [v2f16, v3f16, v4f16, v8f16], 0, VStoreAHalfPromoteRTZ>;
OclBuiltinImpl vstoread_half_rtz_pas_promote  = OclBuiltinImpl<vstoread_half_rtz_pas, [v2f16, v3f16, v4f16, v8f16], 0, VStoreAHalfPromoteRTZ>;
OclBuiltinImpl vstoread_half_rtp_gas_promote  = OclBuiltinImpl<vstoread_half_rtp_gas, [v2f16, v3f16, v4f16, v8f16], 0, VStoreAHalfPromoteRTP>;
OclBuiltinImpl vstoread_half_rtp_las_promote  = OclBuiltinImpl<vstoread_half_rtp_las, [v2f16, v3f16, v4f16, v8f16], 0, VStoreAHalfPromoteRTP>;
OclBuiltinImpl vstoread_half_rtp_pas_promote  = OclBuiltinImpl<vstoread_half_rtp_pas, [v2f16, v3f16, v4f16, v8f16], 0, VStoreAHalfPromoteRTP>;
OclBuiltinImpl vstoread_half_rtn_gas_promote  = OclBuiltinImpl<vstoread_half_rtn_gas, [v2f16, v3f16, v4f16, v8f16], 0, VStoreAHalfPromoteRTN>;
OclBuiltinImpl vstoread_half_rtn_las_promote  = OclBuiltinImpl<vstoread_half_rtn_las, [v2f16, v3f16, v4f16, v8f16], 0, VStoreAHalfPromoteRTN>;
OclBuiltinImpl vstoread_half_rtn_pas_promote  = OclBuiltinImpl<vstoread_half_rtn_pas, [v2f16, v3f16, v4f16, v8f16], 0, VStoreAHalfPromoteRTN>;

OclBuiltinImpl vstoread_half_gas_expand       = OclBuiltinImpl<vstoread_half_gas,     [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstoread_half_las_expand       = OclBuiltinImpl<vstoread_half_las,     [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstoread_half_pas_expand       = OclBuiltinImpl<vstoread_half_pas,     [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstoread_half_rte_gas_expand   = OclBuiltinImpl<vstoread_half_rte_gas, [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstoread_half_rte_las_expand   = OclBuiltinImpl<vstoread_half_rte_las, [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstoread_half_rte_pas_expand   = OclBuiltinImpl<vstoread_half_rte_pas, [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstoread_half_rtz_gas_expand   = OclBuiltinImpl<vstoread_half_rtz_gas, [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstoread_half_rtz_las_expand   = OclBuiltinImpl<vstoread_half_rtz_las, [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstoread_half_rtz_pas_expand   = OclBuiltinImpl<vstoread_half_rtz_pas, [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstoread_half_rtp_gas_expand   = OclBuiltinImpl<vstoread_half_rtp_gas, [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstoread_half_rtp_las_expand   = OclBuiltinImpl<vstoread_half_rtp_las, [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstoread_half_rtp_pas_expand   = OclBuiltinImpl<vstoread_half_rtp_pas, [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstoread_half_rtn_gas_expand   = OclBuiltinImpl<vstoread_half_rtn_gas, [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstoread_half_rtn_las_expand   = OclBuiltinImpl<vstoread_half_rtn_las, [v16f16], 0, VStoreHalfExpand>;
OclBuiltinImpl vstoread_half_rtn_pas_expand   = OclBuiltinImpl<vstoread_half_rtn_pas, [v16f16], 0, VStoreHalfExpand>;
