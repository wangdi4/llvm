//
// non uniform logical
//
list<OclType> logical_vec_types = [v4i32, v8i32, v16i32];

// logical and
code reduce_logical_and = [{
  int$VecLength arg1 = as_int$VecLength($Arg1VarName);
  $Arg0VarName |= ~arg1;
  int res = 1;
#pragma clang unroll(full)
  for (int i=0; i < $VecLength; i++) {
    res = res && $Arg0VarName[i];
  }
  return res;
}];

OclBuiltinImpl sub_group_non_uniform_reduce_logical_and_all = OclBuiltinImpl<sub_group_non_uniform_reduce_logical_and_vec, logical_vec_types, 0, reduce_logical_and>;

OclBuiltinImpl sub_group_non_uniform_scan_inclusive_logical_and_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_logical_and_vec, [v4i32], 0,
[{
  int4 arg1 = as_int4($Arg1VarName);
  $Arg0VarName |= ~arg1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 3; i++) {
    temp[3] = 1;
    temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
    $Arg0VarName = $Arg0VarName && temp;
  }
  return $Arg0VarName;
}]>;

OclBuiltinImpl sub_group_non_uniform_scan_inclusive_logical_and_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_logical_and_vec, [v8i32], 0,
[{
  int8 arg1 = as_int8($Arg1VarName);
  $Arg0VarName |= ~arg1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 7; i++) {
    temp[7] = 1;
    temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName = $Arg0VarName && temp;
  }
  return $Arg0VarName;
}]>;

OclBuiltinImpl sub_group_non_uniform_scan_inclusive_logical_and_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_logical_and_vec, [v16i32], 0,
[{
  int16 arg1 = as_int16($Arg1VarName);
  $Arg0VarName |= ~arg1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 15; i++) {
    temp[15] = 1;
    temp = __builtin_shufflevector(temp, temp, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    $Arg0VarName = $Arg0VarName && temp;
  }
  return $Arg0VarName;
}]>;

OclBuiltinImpl sub_group_non_uniform_scan_exclusive_logical_and_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_logical_and_vec, [v4i32], 0,
[{
  int4 arg1 = as_int4($Arg1VarName);
  $Arg0VarName |= ~arg1;
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 3, 0, 1, 2);
  $Arg0VarName[0] = 1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 2; i++) {
    temp[3] = 1;
    temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
    $Arg0VarName = $Arg0VarName && temp;
  }
  return $Arg0VarName;
}]>;

OclBuiltinImpl sub_group_non_uniform_scan_exclusive_logical_and_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_logical_and_vec, [v8i32], 0,
[{
  int8 arg1 = as_int8($Arg1VarName);
  $Arg0VarName |= ~arg1;
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 7, 0, 1, 2, 3, 4, 5, 6);
  $Arg0VarName[0] = 1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 6; i++) {
    temp[7] = 1;
    temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName = $Arg0VarName && temp;
  }
  return $Arg0VarName;
}]>;

OclBuiltinImpl sub_group_non_uniform_scan_exclusive_logical_and_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_logical_and_vec, [v16i32], 0,
[{
  int16 arg1 = as_int16($Arg1VarName);
  $Arg0VarName |= ~arg1;
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
  $Arg0VarName[0] = 1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 14; i++) {
    temp[15] = 1;
    temp = __builtin_shufflevector(temp, temp, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    $Arg0VarName = $Arg0VarName && temp;
  }
  return $Arg0VarName;
}]>;

// logical or, xor
code reduce_logical_or_xor = [{
  int$VecLength arg1 = as_int$VecLength($Arg1VarName);
  $Arg0VarName &= arg1;
  int res = 0;
#pragma clang unroll(full)
  for (int i=0; i < $VecLength; i++) {
    res = res $OP $Arg0VarName[i];
  }
  return res;
}];

OclBuiltinImpl sub_group_non_uniform_reduce_logical_or_all = OclBuiltinImpl<sub_group_non_uniform_reduce_logical_or_vec, logical_vec_types, 0, reduce_logical_or_xor, [["$OP", "||"]]>;
//OclBuiltinImpl sub_group_non_uniform_reduce_logical_xor_all = OclBuiltinImpl<sub_group_non_uniform_reduce_logical_xor_vec, logical_vec_types, 0, reduce_logical_or_xor, [["$OP", "^^"]]>;

code scan_inclusive_logical_or_xor_v4 = [{
  int4 arg1 = as_int4($Arg1VarName);
  $Arg0VarName &= arg1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 3; i++) {
    temp[3] = 0;
    temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
    $Arg0VarName = $Arg0VarName $OP temp;
  }
  return $Arg0VarName;
}];

OclBuiltinImpl sub_group_non_uniform_scan_inclusive_logical_or_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_logical_or_vec, [v4i32], 0, scan_inclusive_logical_or_xor_v4, [["$OP", "||"]]>;
//OclBuiltinImpl sub_group_non_uniform_scan_inclusive_logical_xor_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_logical_xor_vec, [v4i32], 0, scan_inclusive_logical_or_xor_v4, [["$OP", "^^"]]>;

code scan_inclusive_logical_or_xor_v8 = [{
  int8 arg1 = as_int8($Arg1VarName);
  $Arg0VarName &= arg1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 7; i++) {
    temp[7] = 0;
    temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName = $Arg0VarName $OP temp;
  }
  return $Arg0VarName;
}];
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_logical_or_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_logical_or_vec, [v8i32], 0, scan_inclusive_logical_or_xor_v8, [["$OP", "||"]]>;
//OclBuiltinImpl sub_group_non_uniform_scan_inclusive_logical_xor_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_logical_xor_vec, [v8i32], 0, scan_inclusive_logical_or_xor_v8, [["$OP", "^^"]]>;

code scan_inclusive_logical_or_xor_v16 = [{
  int16 arg1 = as_int16($Arg1VarName);
  $Arg0VarName &= arg1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 15; i++) {
    temp[15] = 0;
    temp = __builtin_shufflevector(temp, temp, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    $Arg0VarName = $Arg0VarName $OP temp;
  }
  return $Arg0VarName;
}];
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_logical_or_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_logical_or_vec, [v16i32], 0, scan_inclusive_logical_or_xor_v16, [["$OP", "||"]]>;
//OclBuiltinImpl sub_group_non_uniform_scan_inclusive_logical_xor_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_logical_xor_vec, [v16i32], 0, scan_inclusive_logical_or_xor_v16, [["$OP", "^^"]]>;

code scan_exclusive_logical_or_xor_v4 = [{
  int4 arg1 = as_int4($Arg1VarName);
  $Arg0VarName &= arg1;
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 3, 0, 1, 2);
  $Arg0VarName[0] = 0;
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 2; i++) {
    temp[3] = 0;
    temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
    $Arg0VarName = $Arg0VarName $OP temp;
  }
  return $Arg0VarName;
}];
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_logical_or_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_logical_or_vec, [v4i32], 0, scan_exclusive_logical_or_xor_v4, [["$OP", "||"]]>;
//OclBuiltinImpl sub_group_non_uniform_scan_exclusive_logical_xor_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_logical_xor_vec, [v4i32], 0, scan_exclusive_logical_or_xor_v4, [["$OP", "^^"]]>;

code scan_exclusive_logical_or_xor_v8 = [{
  int8 arg1 = as_int8($Arg1VarName);
  $Arg0VarName &= arg1;
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 7, 0, 1, 2, 3, 4, 5, 6);
  $Arg0VarName[0] = 0;
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 6; i++) {
    temp[7] = 0;
    temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName = $Arg0VarName $OP temp;
  }
  return $Arg0VarName;
}];
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_logical_or_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_logical_or_vec, [v8i32], 0, scan_exclusive_logical_or_xor_v8, [["$OP", "||"]]>;
//OclBuiltinImpl sub_group_non_uniform_scan_exclusive_logical_xor_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_logical_xor_vec, [v8i32], 0, scan_exclusive_logical_or_xor_v8, [["$OP", "^^"]]>;

code scan_exclusive_logical_or_xor_v16 = [{
  int16 arg1 = as_int16($Arg1VarName);
  $Arg0VarName &= arg1;
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
  $Arg0VarName[0] = 0;
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 14; i++) {
    temp[15] = 0;
    temp = __builtin_shufflevector(temp, temp, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    $Arg0VarName = $Arg0VarName $OP temp;
  }
  return $Arg0VarName;
}];
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_logical_or_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_logical_or_vec, [v16i32], 0, scan_exclusive_logical_or_xor_v16, [["$OP", "||"]]>;
//OclBuiltinImpl sub_group_non_uniform_scan_exclusive_logical_xor_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_logical_xor_vec, [v16i32], 0, scan_exclusive_logical_or_xor_v16, [["$OP", "^^"]]>;


// non uniform bitwise
list<OclType> bitwise_vec_types = [v4i8, v4u8, v8i8, v8u8, v16i8, v16u8,
                                   v4i16, v4u16, v8i16, v8u16, v16i16, v16u16,
                                   v4i32, v4u32, v8i32, v8u32, v16i32, v16u32,
                                   v4i64, v4u64, v8i64, v8u64, v16i64, v16u64];
code reduce_and = [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 |= ~arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0BaseType res = $Arg0VarName[0];
#pragma clang unroll(full)
  for (int i=1; i < $VecLength; i++) {
    res &= $Arg0VarName[i];
  }
  return res;
}];
OclBuiltinImpl sub_group_non_uniform_reduce_and_all = OclBuiltinImpl<sub_group_non_uniform_reduce_and_vec, bitwise_vec_types, 0, reduce_and>;

OclBuiltinImpl sub_group_non_uniform_scan_inclusive_and_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_and_vec,
  [v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4i64, v4u64], 0, [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 |= ~arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 3; i++) {
    temp[3] = -1;
    temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
    $Arg0VarName &= temp;
  }
  return $Arg0VarName;
}]>;

OclBuiltinImpl sub_group_non_uniform_scan_inclusive_and_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_and_vec,
  [v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8i64, v8u64], 0, [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 |= ~arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 7; i++) {
    temp[7] = -1;
    temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName &= temp;
  }
  return $Arg0VarName;
}]>;

OclBuiltinImpl sub_group_non_uniform_scan_inclusive_and_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_and_vec,
  [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16i64, v16u64], 0, [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 |= ~arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 15; i++) {
    temp[15] = -1;
    temp = __builtin_shufflevector(temp, temp, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    $Arg0VarName &= temp;
  }
  return $Arg0VarName;
}]>;

OclBuiltinImpl sub_group_non_uniform_scan_exclusive_and_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_and_vec,
  [v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4i64, v4u64], 0, [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 |= ~arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 3, 0, 1, 2);
  $Arg0VarName[0] = -1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 2; i++) {
    temp[3] = -1;
    temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
    $Arg0VarName &= temp;
  }
  return $Arg0VarName;
}]>;

OclBuiltinImpl sub_group_non_uniform_scan_exclusive_and_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_and_vec,
  [v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8i64, v8u64], 0, [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 |= ~arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 7, 0, 1, 2, 3, 4, 5, 6);
  $Arg0VarName[0] = -1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 6; i++) {
    temp[7] = -1;
    temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName &= temp;
  }
  return $Arg0VarName;
}]>;

OclBuiltinImpl sub_group_non_uniform_scan_exclusive_and_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_and_vec,
  [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16i64, v16u64], 0, [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 |= ~arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
  $Arg0VarName[0] = -1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 14; i++) {
    temp[15] = -1;
    temp = __builtin_shufflevector(temp, temp, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    $Arg0VarName &= temp;
  }
  return $Arg0VarName;
}]>;

code reduce_or_xor = [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 &= arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0BaseType res = $Arg0VarName[0];
#pragma clang unroll(full)
  for (int i=1; i < $VecLength; i++) {
    res = $Arg0VarName[i] $OP res;
  }
  return res;
}];
OclBuiltinImpl sub_group_non_uniform_reduce_or_all = OclBuiltinImpl<sub_group_non_uniform_reduce_or_vec, bitwise_vec_types, 0, reduce_or_xor, [["$OP", "|"]]>;
OclBuiltinImpl sub_group_non_uniform_reduce_xor_all = OclBuiltinImpl<sub_group_non_uniform_reduce_xor_vec, bitwise_vec_types, 0, reduce_or_xor, [["$OP", "^"]]>;

code scan_inclusive_or_xor_v4 = [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 &= arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 3; i++) {
    temp[3] = 0;
    temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
    $Arg0VarName = temp $OP $Arg0VarName;
  }
  return $Arg0VarName;
}];
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_or_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_or_vec,
  [v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4i64, v4u64], 0, scan_inclusive_or_xor_v4, [["$OP", "|"]]>;
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_xor_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_xor_vec,
  [v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4i64, v4u64], 0, scan_inclusive_or_xor_v4, [["$OP", "^"]]>;

code scan_inclusive_or_xor_v8 = [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 &= arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 7; i++) {
    temp[7] = 0;
    temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName = temp $OP $Arg0VarName;
  }
  return $Arg0VarName;
}];
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_or_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_or_vec,
  [v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8i64, v8u64], 0, scan_inclusive_or_xor_v8, [["$OP", "|"]]>;
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_xor_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_xor_vec,
  [v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8i64, v8u64], 0, scan_inclusive_or_xor_v8, [["$OP", "^"]]>;

code scan_inclusive_or_xor_v16 = [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 &= arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 15; i++) {
    temp[15] = 0;
    temp = __builtin_shufflevector(temp, temp, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    $Arg0VarName = temp $OP $Arg0VarName;
  }
  return $Arg0VarName;
}];
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_or_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_or_vec,
  [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16i64, v16u64], 0, scan_inclusive_or_xor_v16, [["$OP", "|"]]>;
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_xor_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_xor_vec,
  [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16i64, v16u64], 0, scan_inclusive_or_xor_v16, [["$OP", "^"]]>;


code scan_exclusive_or_xor_v4 = [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 &= arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 3, 0, 1, 2);
  $Arg0VarName[0] = 0;
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 2; i++) {
    temp[3] = 0;
    temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
    $Arg0VarName = temp $OP $Arg0VarName;
  }
  return $Arg0VarName;
}];
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_or_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_or_vec,
  [v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4i64, v4u64], 0, scan_inclusive_or_xor_v4, [["$OP", "|"]]>;
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_xor_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_xor_vec,
  [v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4i64, v4u64], 0, scan_inclusive_or_xor_v4, [["$OP", "^"]]>;

code scan_exclusive_or_xor_v8 = [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 &= arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 7, 0, 1, 2, 3, 4, 5, 6);
  $Arg0VarName[0] = 0;
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 6; i++) {
    temp[7] = 0;
    temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName = temp $OP $Arg0VarName;
  }
  return $Arg0VarName;
}];
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_or_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_or_vec,
  [v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8i64, v8u64], 0, scan_inclusive_or_xor_v8, [["$OP", "|"]]>;
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_xor_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_xor_vec,
  [v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8i64, v8u64], 0, scan_inclusive_or_xor_v8, [["$OP", "^"]]>;

code scan_exclusive_or_xor_v16 = [{
  $MaskCastTy$VecLength arg1 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 &= arg1;
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
  $Arg0VarName[0] = 0;
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 14; i++) {
    temp[15] = 0;
    temp = __builtin_shufflevector(temp, temp, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    $Arg0VarName = temp $OP $Arg0VarName;
  }
  return $Arg0VarName;
}];
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_or_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_or_vec,
  [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16i64, v16u64], 0, scan_inclusive_or_xor_v16, [["$OP", "|"]]>;
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_xor_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_xor_vec,
  [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16i64, v16u64], 0, scan_inclusive_or_xor_v16, [["$OP", "^"]]>;


// non uniform arithmetic
list<OclType> arithmetic_vec_types = [v4i8, v4u8, v8i8, v8u8, v16i8, v16u8,
                                      v4i16, v4u16, v8i16, v8u16, v16i16, v16u16,
                                      v4i32, v4u32, v8i32, v8u32, v16i32, v16u32,
                                      v4i64, v4u64, v8i64, v8u64, v16i64, v16u64,
                                      v4f32, v8f32, v16f32, v4f64, v8f64, v16f64];

code reduce_mul = [{
  $MaskCastTy$VecLength mask0 = __builtin_convertvector(as_int$VecLength($Arg1VarName), $MaskCastTy$VecLength);
  $MaskCastTy$VecLength arg0 = as_$MaskCastTy$VecLength($Arg0VarName);
  arg0 &= mask0;
  $Arg0BaseType one = 1;
  arg0 |= (~mask0 & as_$MaskCastTy(one));
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0BaseType res = 1;
#pragma clang unroll(full)
  for (int i=0; i < $VecLength; i++) {
    res *= $Arg0VarName[i];
  }
  return res;
  }];
OclBuiltinImpl sub_group_non_uniform_reduce_mul_all = OclBuiltinImpl<sub_group_non_uniform_reduce_mul_vec, arithmetic_vec_types, 0, reduce_mul>;

code scan_inclusive_mul_v4 = [{
  $MaskCastTy#4 mask0 = __builtin_convertvector(as_int4($Arg1VarName), $MaskCastTy#4);
  $MaskCastTy#4 arg0 = as_$MaskCastTy#4($Arg0VarName);
  arg0 &= mask0;
  $Arg0BaseType one = 1;
  arg0 |= (~mask0 & as_$MaskCastTy(one));
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 3; i++) {
    temp[3] = 1;
    temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
    $Arg0VarName *= temp;
  }
  return $Arg0VarName;
}];
code scan_inclusive_mul_v8 = [{
  $MaskCastTy#8 mask0 = __builtin_convertvector(as_int8($Arg1VarName), $MaskCastTy#8);
  $MaskCastTy#8 arg0 = as_$MaskCastTy#8($Arg0VarName);
  arg0 &= mask0;
  $Arg0BaseType one = 1;
  arg0 |= (~mask0 & as_$MaskCastTy(one));
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 7; i++) {
    temp[7] = 1;
    temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName *= temp;
  }
  return $Arg0VarName;
}];
code scan_inclusive_mul_v16 = [{
  $MaskCastTy#16 mask0 = __builtin_convertvector(as_int16($Arg1VarName), $MaskCastTy#16);
  $MaskCastTy#16 arg0 = as_$MaskCastTy#16($Arg0VarName);
  arg0 &= mask0;
  $Arg0BaseType one = 1;
  arg0 |= (~mask0 & as_$MaskCastTy(one));
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 15; i++) {
    temp[15] = 1;
    temp = __builtin_shufflevector(temp, temp, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    $Arg0VarName *= temp;
  }
  return $Arg0VarName;
}];

OclBuiltinImpl sub_group_non_uniform_scan_inclusive_mul_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_mul_vec, [v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4f32, v4f64, v4i64, v4u64], 0, scan_inclusive_mul_v4>;
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_mul_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_mul_vec, [v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8f32, v8f64, v8i64, v8u64], 0, scan_inclusive_mul_v8>;
OclBuiltinImpl sub_group_non_uniform_scan_inclusive_mul_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_mul_vec, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32, v16f64, v16i64, v16u64], 0, scan_inclusive_mul_v16>;

code scan_exclusive_mul_v4 = [{
  $MaskCastTy#4 mask0 = __builtin_convertvector(as_int4($Arg1VarName), $MaskCastTy#4);
  $MaskCastTy#4 arg0 = as_$MaskCastTy#4($Arg0VarName);
  arg0 &= mask0;
  $Arg0BaseType one = 1;
  arg0 |= (~mask0 & as_$MaskCastTy(one));
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 3, 0, 1, 2);
  $Arg0VarName[0] = 1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 2; i++) {
    temp[3] = 1;
    temp = __builtin_shufflevector(temp, temp, 3, 0, 1, 2);
    $Arg0VarName *= temp;
  }
  return $Arg0VarName;
}];
code scan_exclusive_mul_v8 = [{
  $MaskCastTy#8 mask0 = __builtin_convertvector(as_int8($Arg1VarName), $MaskCastTy#8);
  $MaskCastTy#8 arg0 = as_$MaskCastTy#8($Arg0VarName);
  arg0 &= mask0;
  $Arg0BaseType one = 1;
  arg0 |= (~mask0 & as_$MaskCastTy(one));
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 7, 0, 1, 2, 3, 4, 5, 6);
  $Arg0VarName[0] = 1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 6; i++) {
    temp[7] = 1;
    temp = __builtin_shufflevector(temp, temp, 7, 0, 1, 2, 3, 4, 5, 6);
    $Arg0VarName *= temp;
  }
  return $Arg0VarName;
}];
code scan_exclusive_mul_v16 = [{
  $MaskCastTy#16 mask0 = __builtin_convertvector(as_int16($Arg1VarName), $MaskCastTy#16);
  $MaskCastTy#16 arg0 = as_$MaskCastTy#16($Arg0VarName);
  arg0 &= mask0;
  $Arg0BaseType one = 1;
  arg0 |= (~mask0 & as_$MaskCastTy(one));
  $Arg0VarName = as_$Arg0Type(arg0);
  $Arg0VarName = __builtin_shufflevector($Arg0VarName, $Arg0VarName, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
  $Arg0VarName[0] = 1;
  $Arg0Type temp = $Arg0VarName;
#pragma clang unroll(full)
  for (int i = 0; i < 14; i++) {
    temp[15] = 1;
    temp = __builtin_shufflevector(temp, temp, 15, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14);
    $Arg0VarName *= temp;
  }
  return $Arg0VarName;
}];

OclBuiltinImpl sub_group_non_uniform_scan_exclusive_mul_v4 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_mul_vec, [v4i8, v4u8, v4i16, v4u16, v4i32, v4u32, v4f32, v4f64, v4i64, v4u64], 0, scan_exclusive_mul_v4>;
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_mul_v8 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_mul_vec, [v8i8, v8u8, v8i16, v8u16, v8i32, v8u32, v8f32, v8f64, v8i64, v8u64], 0, scan_exclusive_mul_v8>;
OclBuiltinImpl sub_group_non_uniform_scan_exclusive_mul_v16 = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_mul_vec, [v16i8, v16u8, v16i16, v16u16, v16i32, v16u32, v16f32, v16f64, v16i64, v16u64], 0, scan_exclusive_mul_v16>;

//
// Broadcast
//

OclBuiltinImpl sub_group_broadcast_vector = OclBuiltinImpl<sub_group_broadcast_vec, [v4i32, v8i32, v16i32,
                                                                                     v4u32, v8u32, v16u32,
                                                                                     v4i64, v8i64, v16i64,
                                                                                     v4u64, v8u64, v16u64,
                                                                                     v4f32, v8f32, v16f32,
                                                                                     v4f64, v8f64, v16f64],  0,
  [{
    return $Arg0VarName[$Arg1VarName];
  }]>;

OclBuiltinImpl sub_group_broadcast_vector_cs = OclBuiltinImpl<sub_group_broadcast_vec_cs, [v4i8 , v8i8 , v16i8 ,
                                                                                           v4u8 , v8u8 , v16u8 ,
                                                                                           v4i16, v8i16, v16i16,
                                                                                           v4u16, v8u16, v16u16], 0,
  [{
    return $Arg0VarName[$Arg1VarName];
  }]>;

OclBuiltinImpl sub_group_shuffle_avx512v48gentype = OclBuiltinImpl<sub_group_shuffle_avx512, [v48i8, v48u8, v48i16, v48u16, v48f32,
                                                                                              v48i32, v48u32, v48i64, v48u64], 0,
  [{
     $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x16($Arg0VarName);
     $Arg0BaseType#16* arg0 = ($Arg0BaseType#16*)&$Arg0VarName;

     $Arg0BaseType#16 tmp1 = intel_sub_group_shuffle(arg0[0], $Arg1VarName, $Arg2VarName);
     $Arg0BaseType#16 tmp2 = intel_sub_group_shuffle(arg0[1], $Arg1VarName, $Arg2VarName);
     $Arg0BaseType#16 tmp3 = intel_sub_group_shuffle(arg0[2], $Arg1VarName, $Arg2VarName);

     $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_16x3(($Arg0Type)(tmp1, tmp2, tmp3));
     return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_shuffle_avx2v24gentype = OclBuiltinImpl<sub_group_shuffle_avx2, [v24i8, v24u8, v24i16, v24u16, v24f32,
                                                                                        v24i32, v24u32, v24i64, v24u64], 0,
  [{
     $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x8($Arg0VarName);
     $Arg0BaseType#8* arg0 = ($Arg0BaseType#8*)&$Arg0VarName;

     $Arg0BaseType#8 tmp1 = intel_sub_group_shuffle(arg0[0], $Arg1VarName, $Arg2VarName);
     $Arg0BaseType#8 tmp2 = intel_sub_group_shuffle(arg0[1], $Arg1VarName, $Arg2VarName);
     $Arg0BaseType#8 tmp3 = intel_sub_group_shuffle(arg0[2], $Arg1VarName, $Arg2VarName);

     $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_8x3(($Arg0Type)(tmp1, tmp2, tmp3));
     return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_shuffle_avxv12gentype = OclBuiltinImpl<sub_group_shuffle_avx, [v12i8, v12u8, v12i16, v12u16, v12f32,
                                                                                        v12i32, v12u32, v12i64, v12u64], 0,
  [{
     $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x4($Arg0VarName);
     $Arg0BaseType#4* arg0 = ($Arg0BaseType#4*)&$Arg0VarName;

     $Arg0BaseType#4 tmp1 = intel_sub_group_shuffle(arg0[0], $Arg1VarName, $Arg2VarName);
     $Arg0BaseType#4 tmp2 = intel_sub_group_shuffle(arg0[1], $Arg1VarName, $Arg2VarName);
     $Arg0BaseType#4 tmp3 = intel_sub_group_shuffle(arg0[2], $Arg1VarName, $Arg2VarName);

     $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_4x3(($Arg0Type)(tmp1, tmp2, tmp3));
     return $Arg0VarName;
  }]>;

// shuffle up
OclBuiltinImpl sub_group_shuffle_up_avx512v48gentype = OclBuiltinImpl<sub_group_shuffle_up_avx512, [v48i8, v48u8, v48i16, v48u16, v48f32,
                                                                                                    v48i32, v48u32, v48i64, v48u64], 0,
  [{
    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x16($Arg0VarName);
    $Arg1VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x16($Arg1VarName);

    $Arg0BaseType#16* arg0 = ($Arg0BaseType#16*)&$Arg0VarName;
    $Arg0BaseType#16* arg1 = ($Arg0BaseType#16*)&$Arg1VarName;

    $Arg0BaseType#16 tmp1 = intel_sub_group_shuffle_up(arg0[0], arg1[0], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#16 tmp2 = intel_sub_group_shuffle_up(arg0[1], arg1[1], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#16 tmp3 = intel_sub_group_shuffle_up(arg0[2], arg1[2], $Arg2VarName, $Arg3VarName);

    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_16x3(($Arg0Type)(tmp1, tmp2, tmp3));
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_shuffle_up_avx2v24gentype = OclBuiltinImpl<sub_group_shuffle_up_avx2, [v24i8, v24u8, v24i16, v24u16, v24f32,
                                                                                                v24i32, v24u32, v24i64, v24u64], 0,
  [{
    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x8($Arg0VarName);
    $Arg1VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x8($Arg1VarName);

    $Arg0BaseType#8* arg0 = ($Arg0BaseType#8*)&$Arg0VarName;
    $Arg0BaseType#8* arg1 = ($Arg0BaseType#8*)&$Arg1VarName;

    $Arg0BaseType#8 tmp1 = intel_sub_group_shuffle_up(arg0[0], arg1[0], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#8 tmp2 = intel_sub_group_shuffle_up(arg0[1], arg1[1], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#8 tmp3 = intel_sub_group_shuffle_up(arg0[2], arg1[2], $Arg2VarName, $Arg3VarName);

    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_8x3(($Arg0Type)(tmp1, tmp2, tmp3));
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_shuffle_up_avxv12gentype = OclBuiltinImpl<sub_group_shuffle_up_avx, [v12i8, v12u8, v12i16, v12u16, v12f32,
                                                                                              v12i32, v12u32, v12i64, v12u64], 0,
  [{
    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x4($Arg0VarName);
    $Arg1VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x4($Arg1VarName);

    $Arg0BaseType#4* arg0 = ($Arg0BaseType#4*)&$Arg0VarName;
    $Arg0BaseType#4* arg1 = ($Arg0BaseType#4*)&$Arg1VarName;

    $Arg0BaseType#4 tmp1 = intel_sub_group_shuffle_up(arg0[0], arg1[0], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#4 tmp2 = intel_sub_group_shuffle_up(arg0[1], arg1[1], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#4 tmp3 = intel_sub_group_shuffle_up(arg0[2], arg1[2], $Arg2VarName, $Arg3VarName);

    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_4x3(($Arg0Type)(tmp1, tmp2, tmp3));
    return $Arg0VarName;
  }]>;

// shuffle down
OclBuiltinImpl sub_group_shuffle_down_avx512v48gentype = OclBuiltinImpl<sub_group_shuffle_down_avx512, [v48i8, v48u8, v48i16, v48u16, v48f32,
                                                                                                        v48i32, v48u32, v48i64, v48u64], 0,
  [{
    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x16($Arg0VarName);
    $Arg1VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x16($Arg1VarName);

    $Arg0BaseType#16* arg0 = ($Arg0BaseType#16*)&$Arg0VarName;
    $Arg0BaseType#16* arg1 = ($Arg0BaseType#16*)&$Arg1VarName;

    $Arg0BaseType#16 tmp1 = intel_sub_group_shuffle_down(arg0[0], arg1[0], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#16 tmp2 = intel_sub_group_shuffle_down(arg0[1], arg1[1], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#16 tmp3 = intel_sub_group_shuffle_down(arg0[2], arg1[2], $Arg2VarName, $Arg3VarName);

    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_16x3(($Arg0Type)(tmp1, tmp2, tmp3));
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_shuffle_down_avx2v24gentype = OclBuiltinImpl<sub_group_shuffle_down_avx2, [v24i8, v24u8, v24i16, v24u16, v24f32,
                                                                                                    v24i32, v24u32, v24i64, v24u64], 0,
  [{
    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x8($Arg0VarName);
    $Arg1VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x8($Arg1VarName);

    $Arg0BaseType#8* arg0 = ($Arg0BaseType#8*)&$Arg0VarName;
    $Arg0BaseType#8* arg1 = ($Arg0BaseType#8*)&$Arg1VarName;

    $Arg0BaseType#8 tmp1 = intel_sub_group_shuffle_down(arg0[0], arg1[0], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#8 tmp2 = intel_sub_group_shuffle_down(arg0[1], arg1[1], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#8 tmp3 = intel_sub_group_shuffle_down(arg0[2], arg1[2], $Arg2VarName, $Arg3VarName);

    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_8x3(($Arg0Type)(tmp1, tmp2, tmp3));
    return $Arg0VarName;
  }]>;

OclBuiltinImpl sub_group_shuffle_down_avxv12gentype = OclBuiltinImpl<sub_group_shuffle_down_avx, [v12i8, v12u8, v12i16, v12u16, v12f32,
                                                                                                  v12i32, v12u32, v12i64, v12u64], 0,
  [{
    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x4($Arg0VarName);
    $Arg1VarName = __ocl_shuffle_transpose_$Arg0BaseType_3x4($Arg1VarName);

    $Arg0BaseType#4* arg0 = ($Arg0BaseType#4*)&$Arg0VarName;
    $Arg0BaseType#4* arg1 = ($Arg0BaseType#4*)&$Arg1VarName;

    $Arg0BaseType#4 tmp1 = intel_sub_group_shuffle_down(arg0[0], arg1[0], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#4 tmp2 = intel_sub_group_shuffle_down(arg0[1], arg1[1], $Arg2VarName, $Arg3VarName);
    $Arg0BaseType#4 tmp3 = intel_sub_group_shuffle_down(arg0[2], arg1[2], $Arg2VarName, $Arg3VarName);

    $Arg0VarName = __ocl_shuffle_transpose_$Arg0BaseType_4x3(($Arg0Type)(tmp1, tmp2, tmp3));
    return $Arg0VarName;
  }]>;

// shuffle xor

OclBuiltinImpl sub_group_shuffle_xor_avx512v48gentype = OclBuiltinImpl<sub_group_shuffle_xor_avx512, [v48i8, v48u8, v48i16, v48u16, v48f32,
                                                                                                      v48i32, v48u32, v48i64, v48u64], 0,
  [{
    uint16 indexes = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    indexes = indexes ^ $Arg1VarName;
    return intel_sub_group_shuffle($Arg0VarName, indexes, $Arg2VarName);
  }]>;

OclBuiltinImpl sub_group_shuffle_xor_avx2v24gentype = OclBuiltinImpl<sub_group_shuffle_xor_avx2, [v24i8, v24u8, v24i16, v24u16, v24f32,
                                                                                                  v24i32, v24u32, v24i64, v24u64], 0,
  [{
    uint8 indexes = {0, 1, 2, 3, 4, 5, 6, 7};
    indexes = indexes ^ $Arg1VarName;
    return intel_sub_group_shuffle($Arg0VarName, indexes, $Arg2VarName);
  }]>;

OclBuiltinImpl sub_group_shuffle_xor_avxv12gentype = OclBuiltinImpl<sub_group_shuffle_xor_avx, [v12i8, v12u8, v12i16, v12u16, v12f32,
                                                                                                v12i32, v12u32, v12i64, v12u64], 0,
  [{
    uint4 indexes = {0, 1, 2, 3};
    indexes = indexes ^ $Arg1VarName;
    return intel_sub_group_shuffle($Arg0VarName, indexes, $Arg2VarName);
  }]>;
