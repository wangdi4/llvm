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

//
// Generic Intel Subgroups Implementation
//

// Assume uniform codes
// Number of params = 3, assuming the second parameter as uniform
code AssumeUniform3_2 = [{
    return $Func($Arg0VarName, $Arg1VarName[0], $Arg2VarName);
  }];

// All
code generic_all = [{
for (int i = 0; i < $VecLength; ++i) {
 if ($Arg1VarName[i] == 0)
   continue;
 if ($Arg0VarName[i] == 0)
   return 0;
}
return 1;
}];

OclBuiltinImpl sub_group_all_generic = OclBuiltinImpl<sub_group_all_vec, sub_group_all_vec.Types, 0, generic_all>;

// Any
code generic_any = [{
for (int i = 0; i < $VecLength; ++i) {
 if ($Arg1VarName[i] == 0)
   continue;
 if ($Arg0VarName[i] != 0)
   return 1;
}
return 0;
}];

OclBuiltinImpl sub_group_any_generic = OclBuiltinImpl<sub_group_any_vec, sub_group_any_vec.Types, 0, generic_any>;

// Broadcast
OclBuiltinImpl sub_group_broadcast_generic = OclBuiltinImpl<sub_group_broadcast_vec, sub_group_broadcast_vec.Types,  0,
  [{
    return $Arg0VarName[$Arg1VarName];
  }]>;

OclBuiltinImpl sub_group_broadcast_cs_generic = OclBuiltinImpl<sub_group_broadcast_vec_cs, sub_group_broadcast_vec_cs.Types,  0,
  [{
    return $Arg0VarName[$Arg1VarName];
  }]>;

// Broadcast assume uniform wrapper
OclBuiltinImpl sub_group_broadcast_assume_uniform_vec_generic = OclBuiltinImpl<sub_group_broadcast_assume_uniform_vec, sub_group_broadcast_assume_uniform_vec.Types, 0, AssumeUniform3_2>;
OclBuiltinImpl sub_group_broadcast_assume_uniform_vec_cs_generic = OclBuiltinImpl<sub_group_broadcast_assume_uniform_vec_cs, sub_group_broadcast_assume_uniform_vec_cs.Types, 0, AssumeUniform3_2>;
OclBuiltinImpl sub_group_broadcast_assume_uniform_extended_avx_generic = OclBuiltinImpl<sub_group_broadcast_assume_uniform_extended_avx, sub_group_broadcast_assume_uniform_extended_avx.Types, 0, AssumeUniform3_2>;
OclBuiltinImpl sub_group_broadcast_assume_uniform_extended_avx2_generic = OclBuiltinImpl<sub_group_broadcast_assume_uniform_extended_avx2, sub_group_broadcast_assume_uniform_extended_avx2.Types, 0, AssumeUniform3_2>;
OclBuiltinImpl sub_group_broadcast_assume_uniform_extended_avx512_generic = OclBuiltinImpl<sub_group_broadcast_assume_uniform_extended_avx512, sub_group_broadcast_assume_uniform_extended_avx512.Types, 0, AssumeUniform3_2>;
OclBuiltinImpl sub_group_broadcast_assume_uniform_extended_x32_generic = OclBuiltinImpl<sub_group_broadcast_assume_uniform_extended_x32, sub_group_broadcast_assume_uniform_extended_x32.Types, 0, AssumeUniform3_2>;
OclBuiltinImpl sub_group_broadcast_assume_uniform_extended_x64_generic = OclBuiltinImpl<sub_group_broadcast_assume_uniform_extended_x64, sub_group_broadcast_assume_uniform_extended_x64.Types, 0, AssumeUniform3_2>;

// Reduce add
code generic_reduce_add = [{
$Arg0BaseType init = 0;
for (int i = 0; i < $VecLength; ++i) {
 if ($Arg1VarName[i] == 0)
   continue;
 init += $Arg0VarName[i];
}
return init;
}];

OclBuiltinImpl sub_group_reduce_add_generic = OclBuiltinImpl<sub_group_reduce_add_vec, sub_group_reduce_add_vec.Types, 0, generic_reduce_add>;
OclBuiltinImpl sub_group_reduce_add_cs_generic = OclBuiltinImpl<sub_group_reduce_add_vec_cs, sub_group_reduce_add_vec_cs.Types, 0, generic_reduce_add>;

// Reduce min/max
code generic_reduce_comp = [{
$Arg0BaseType init = $INIT;
for (int i = 0; i < $VecLength; ++i) {
 if ($Arg1VarName[i] == 0)
   continue;
 if (init $OP  $Arg0VarName[i])
   init = $Arg0VarName[i];
}
return init;
}];

OclBuiltinImpl sub_group_reduce_min_generic = OclBuiltinImpl<sub_group_reduce_min_vec, sub_group_reduce_min_vec.Types, 0, generic_reduce_comp, [["$OP", ">"], ["$INIT", "$MaxVal"]]>;
OclBuiltinImpl sub_group_reduce_max_generic = OclBuiltinImpl<sub_group_reduce_max_vec, sub_group_reduce_max_vec.Types, 0, generic_reduce_comp, [["$OP", "<"], ["$INIT", "$MinVal"]]>;
OclBuiltinImpl sub_group_reduce_min_cs_generic = OclBuiltinImpl<sub_group_reduce_min_vec_cs, sub_group_reduce_min_vec_cs.Types, 0, generic_reduce_comp, [["$OP", ">"], ["$INIT", "$MaxVal"]]>;
OclBuiltinImpl sub_group_reduce_max_cs_generic = OclBuiltinImpl<sub_group_reduce_max_vec_cs, sub_group_reduce_max_vec_cs.Types, 0, generic_reduce_comp, [["$OP", "<"], ["$INIT", "$MinVal"]]>;

// Reduce mul
code generic_reduce_mul = [{
$Arg0BaseType init = 1;
for (int i = 0; i < $VecLength; ++i) {
 if ($Arg1VarName[i] == 0)
   continue;
 init *= $Arg0VarName[i];
}
return init;
}];

OclBuiltinImpl sub_group_reduce_mul_generic = OclBuiltinImpl<sub_group_non_uniform_reduce_mul_vec, sub_group_non_uniform_reduce_mul_vec.Types, 0, generic_reduce_mul>;

// Scan inclusive add
code generic_scan_inclusive_add = [{
if ($Arg1VarName[0] == 0)
  $Arg0VarName[0] = 0;
for (int i = 1; i < $VecLength; ++i) {
  if ($Arg1VarName[i] != 0)
    $Arg0VarName[i] += $Arg0VarName[i-1];
  else
    $Arg0VarName[i] = $Arg0VarName[i-1];
}
return $Arg0VarName;
}];

OclBuiltinImpl sub_group_scan_inclusive_add_generic = OclBuiltinImpl<sub_group_scan_inclusive_add_vec, sub_group_scan_inclusive_add_vec.Types, 0, generic_scan_inclusive_add>;
OclBuiltinImpl sub_group_scan_inclusive_add_cs_generic = OclBuiltinImpl<sub_group_scan_inclusive_add_vec_cs, sub_group_scan_inclusive_add_vec_cs.Types, 0, generic_scan_inclusive_add>;

// Scan inclusive min/max
code generic_scan_inclusive_comp = [{
if ($Arg1VarName[0] == 0)
  $Arg0VarName[0] = $INIT;
for (int i = 1; i < $VecLength; ++i) {
  if ($Arg1VarName[i] != 0)
    $Arg0VarName[i] = $Arg0VarName[i] $OP $Arg0VarName[i-1] ? $Arg0VarName[i] : $Arg0VarName[i-1];
  else
    $Arg0VarName[i] = $Arg0VarName[i-1];
}
return $Arg0VarName;
}];

OclBuiltinImpl sub_group_scan_inclusive_min_generic = OclBuiltinImpl<sub_group_scan_inclusive_min_vec, sub_group_scan_inclusive_min_vec.Types, 0, generic_scan_inclusive_comp,  [["$OP", "<"], ["$INIT", "$MaxVal"]]>;
OclBuiltinImpl sub_group_scan_inclusive_max_generic = OclBuiltinImpl<sub_group_scan_inclusive_max_vec, sub_group_scan_inclusive_max_vec.Types, 0, generic_scan_inclusive_comp,  [["$OP", ">"], ["$INIT", "$MinVal"]]>;
OclBuiltinImpl sub_group_scan_inclusive_min_cs_generic = OclBuiltinImpl<sub_group_scan_inclusive_min_vec_cs, sub_group_scan_inclusive_min_vec_cs.Types, 0, generic_scan_inclusive_comp,  [["$OP", "<"], ["$INIT", "$MaxVal"]]>;
OclBuiltinImpl sub_group_scan_inclusive_max_cs_generic = OclBuiltinImpl<sub_group_scan_inclusive_max_vec_cs, sub_group_scan_inclusive_max_vec_cs.Types, 0, generic_scan_inclusive_comp,  [["$OP", ">"], ["$INIT", "$MinVal"]]>;

// Scan inclusive mul
code generic_scan_inclusive_mul = [{
if ($Arg1VarName[0] == 0)
  $Arg0VarName[0] = 1;
for (int i = 1; i < $VecLength; ++i) {
  if ($Arg1VarName[i] != 0)
    $Arg0VarName[i] *= $Arg0VarName[i-1];
  else
    $Arg0VarName[i] = $Arg0VarName[i-1];
}
return $Arg0VarName;
}];

OclBuiltinImpl sub_group_scan_inclusive_mul_generic = OclBuiltinImpl<sub_group_non_uniform_scan_inclusive_mul_vec, sub_group_non_uniform_scan_inclusive_mul_vec.Types, 0, generic_scan_inclusive_mul>;

// Scan exclusive add
code generic_scan_exclusive_add = [{
$Arg0Type helper;
helper[0] = 0;
if ($Arg1VarName[0] == 0)
  $Arg0VarName[0] = 0;
for (int i = 1; i < $VecLength; ++i) {
  if ($Arg1VarName[i] != 0)
    $Arg0VarName[i] += $Arg0VarName[i-1];
  else
    $Arg0VarName[i] = $Arg0VarName[i-1];
  helper[i] = $Arg0VarName[i-1];
}
return helper;
}];

OclBuiltinImpl sub_group_scan_exclusive_add_generic = OclBuiltinImpl<sub_group_scan_exclusive_add_vec, sub_group_scan_exclusive_add_vec.Types, 0, generic_scan_exclusive_add>;
OclBuiltinImpl sub_group_scan_exclusive_add_cs_generic = OclBuiltinImpl<sub_group_scan_exclusive_add_vec_cs, sub_group_scan_exclusive_add_vec_cs.Types, 0, generic_scan_exclusive_add>;

// Scan exclusive min/max
code generic_scan_exclusive_comp = [{
$Arg0Type helper;
helper[0] = $INIT;
if ($Arg1VarName[0] == 0)
  $Arg0VarName[0] = $INIT;
for (int i = 1; i < $VecLength; ++i) {
  if ($Arg1VarName[i] != 0)
    $Arg0VarName[i] = $Arg0VarName[i] $OP $Arg0VarName[i-1] ? $Arg0VarName[i] : $Arg0VarName[i-1];
  else
    $Arg0VarName[i] = $Arg0VarName[i-1];
  helper[i] = $Arg0VarName[i-1];
}
return helper;
}];

OclBuiltinImpl sub_group_scan_exclusive_min_generic = OclBuiltinImpl<sub_group_scan_exclusive_min_vec, sub_group_scan_exclusive_min_vec.Types, 0, generic_scan_exclusive_comp,  [["$OP", "<"], ["$INIT", "$MaxVal"]]>;
OclBuiltinImpl sub_group_scan_exclusive_max_generic = OclBuiltinImpl<sub_group_scan_exclusive_max_vec, sub_group_scan_exclusive_max_vec.Types, 0, generic_scan_exclusive_comp,  [["$OP", ">"], ["$INIT", "$MinVal"]]>;
OclBuiltinImpl sub_group_scan_exclusive_min_cs_generic = OclBuiltinImpl<sub_group_scan_exclusive_min_vec_cs, sub_group_scan_exclusive_min_vec_cs.Types, 0, generic_scan_exclusive_comp,  [["$OP", "<"], ["$INIT", "$MaxVal"]]>;
OclBuiltinImpl sub_group_scan_exclusive_max_cs_generic = OclBuiltinImpl<sub_group_scan_exclusive_max_vec_cs, sub_group_scan_exclusive_max_vec_cs.Types, 0, generic_scan_exclusive_comp,  [["$OP", ">"], ["$INIT", "$MinVal"]]>;

// Scan exclusive mul
code generic_scan_exclusive_mul = [{
$Arg0Type helper;
helper[0] = 1;
if ($Arg1VarName[0] == 0)
  $Arg0VarName[0] = 1;
for (int i = 1; i < $VecLength; ++i) {
  if ($Arg1VarName[i] != 0)
    $Arg0VarName[i] *= $Arg0VarName[i-1];
  else
    $Arg0VarName[i] = $Arg0VarName[i-1];
  helper[i] = $Arg0VarName[i-1];
}
return helper;
}];

OclBuiltinImpl sub_group_scan_exclusive_mul_generic = OclBuiltinImpl<sub_group_non_uniform_scan_exclusive_mul_vec, sub_group_non_uniform_scan_exclusive_mul_vec.Types, 0, generic_scan_exclusive_mul>;

// Shuffle
code generic_shuffle = [{
$Arg0Type res;
int eleWidth = $VecLength / $VF;
for (int i = 0; i < $VF; ++i) {
  if ($Arg2VarName[i] == 0)
    continue;
  int offsetSrc = $Arg1VarName[i] * eleWidth;
  int offsetDst = i * eleWidth;
  for (int j = 0; j < eleWidth; ++j)
    res[offsetDst + j] = $Arg0VarName[offsetSrc + j];
}
return res;
}];

OclBuiltinImpl sub_group_shuffle_avx_generic = OclBuiltinImpl<sub_group_shuffle_avx, sub_group_shuffle_avx.Types, 0, generic_shuffle, [["$VF", "4"]]>;
OclBuiltinImpl sub_group_shuffle_avx2_generic = OclBuiltinImpl<sub_group_shuffle_avx2, sub_group_shuffle_avx2.Types, 0, generic_shuffle, [["$VF", "8"]]>;
OclBuiltinImpl sub_group_shuffle_avx512_generic = OclBuiltinImpl<sub_group_shuffle_avx512, sub_group_shuffle_avx512.Types, 0, generic_shuffle, [["$VF", "16"]]>;
OclBuiltinImpl sub_group_shuffle_x32_generic = OclBuiltinImpl<sub_group_shuffle_x32, sub_group_shuffle_x32.Types, 0, generic_shuffle, [["$VF", "32"]]>;
OclBuiltinImpl sub_group_shuffle_x64_generic = OclBuiltinImpl<sub_group_shuffle_x64, sub_group_shuffle_x64.Types, 0, generic_shuffle, [["$VF", "64"]]>;

// Shuffle xor
code generic_shuffle_xor = [{
uint$VF indexes = $INDEXES;
indexes ^= $Arg1VarName;
return intel_sub_group_shuffle($Arg0VarName, indexes, $Arg2VarName);
}];

OclBuiltinImpl sub_group_shuffle_xor_avx_generic = OclBuiltinImpl<sub_group_shuffle_xor_avx, sub_group_shuffle_xor_avx.Types, 0, generic_shuffle_xor, [["$INDEXES", "{0, 1, 2, 3}"], ["$VF", "4"]]>;
OclBuiltinImpl sub_group_shuffle_xor_avx2_generic = OclBuiltinImpl<sub_group_shuffle_xor_avx2, sub_group_shuffle_xor_avx2.Types, 0, generic_shuffle_xor, [["$INDEXES", "{0, 1, 2, 3, 4, 5, 6, 7}"], ["$VF", "8"]]>;
OclBuiltinImpl sub_group_shuffle_xor_avx512_generic = OclBuiltinImpl<sub_group_shuffle_xor_avx512, sub_group_shuffle_xor_avx512.Types, 0, generic_shuffle_xor, [["$INDEXES", "{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}"], ["$VF", "16"]]>;
OclBuiltinImpl sub_group_shuffle_xor_x32_generic = OclBuiltinImpl<sub_group_shuffle_xor_x32, sub_group_shuffle_xor_x32.Types, 0, generic_shuffle_xor, [["$INDEXES", "{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31}"], ["$VF", "32"]]>;
OclBuiltinImpl sub_group_shuffle_xor_x64_generic = OclBuiltinImpl<sub_group_shuffle_xor_x64, sub_group_shuffle_xor_x64.Types, 0, generic_shuffle_xor, [["$INDEXES", "{0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63}"], ["$VF", "64"]]>;

// Shuffle down
code generic_shuffle_down = [{
$Arg0Type res;
int eleWidth = $VecLength / $VF;
for (int i = 0; i < $VF; ++i) {
  if ($Arg3VarName[i] == 0)
    continue;

  int index = i +  $Arg2VarName[i];
  int ind = index >= $VF ? index - $VF : index;
  $Arg0Type dataSrc = index >= $VF ? $Arg1VarName : $Arg0VarName;
  int offsetSrc = ind * eleWidth;
  int offsetDst = i * eleWidth;
  for (int j = 0; j < eleWidth; ++j)
    res[offsetDst + j] = dataSrc[offsetSrc + j];
}
return res;
}];

OclBuiltinImpl sub_group_shuffle_down_avx_generic = OclBuiltinImpl<sub_group_shuffle_down_avx, sub_group_shuffle_down_avx.Types, 0, generic_shuffle_down, [["$VF", "4"]]>;
OclBuiltinImpl sub_group_shuffle_down_avx2_generic = OclBuiltinImpl<sub_group_shuffle_down_avx2, sub_group_shuffle_down_avx2.Types, 0, generic_shuffle_down, [["$VF", "8"]]>;
OclBuiltinImpl sub_group_shuffle_down_avx512_generic = OclBuiltinImpl<sub_group_shuffle_down_avx512, sub_group_shuffle_down_avx512.Types, 0, generic_shuffle_down, [["$VF", "16"]]>;
OclBuiltinImpl sub_group_shuffle_down_x32_generic = OclBuiltinImpl<sub_group_shuffle_down_x32, sub_group_shuffle_down_x32.Types, 0, generic_shuffle_down, [["$VF", "32"]]>;
OclBuiltinImpl sub_group_shuffle_down_x64_generic = OclBuiltinImpl<sub_group_shuffle_down_x64, sub_group_shuffle_down_x64.Types, 0, generic_shuffle_down, [["$VF", "64"]]>;

// Shuffle up
code generic_shuffle_up = [{
$Arg0Type res;
int eleWidth = $VecLength / $VF;
for (int i = 0; i < $VF; ++i) {
  if ($Arg3VarName[i] == 0)
    continue;

  int index = i -  $Arg2VarName[i];
  int ind = index < 0 ? index + $VF : index;
  $Arg0Type dataSrc = index < 0 ? $Arg0VarName : $Arg1VarName;
  int offsetSrc = ind * eleWidth;
  int offsetDst = i * eleWidth;
  for (int j = 0; j < eleWidth; ++j)
    res[offsetDst + j] = dataSrc[offsetSrc + j];
}
return res;
}];

OclBuiltinImpl sub_group_shuffle_up_avx_generic = OclBuiltinImpl<sub_group_shuffle_up_avx, sub_group_shuffle_up_avx.Types, 0, generic_shuffle_up, [["$VF", "4"]]>;
OclBuiltinImpl sub_group_shuffle_up_avx2_generic = OclBuiltinImpl<sub_group_shuffle_up_avx2, sub_group_shuffle_up_avx2.Types, 0, generic_shuffle_up, [["$VF", "8"]]>;
OclBuiltinImpl sub_group_shuffle_up_avx512_generic = OclBuiltinImpl<sub_group_shuffle_up_avx512, sub_group_shuffle_up_avx512.Types, 0, generic_shuffle_up, [["$VF", "16"]]>;
OclBuiltinImpl sub_group_shuffle_up_x32_generic = OclBuiltinImpl<sub_group_shuffle_up_x32, sub_group_shuffle_up_x32.Types, 0, generic_shuffle_up, [["$VF", "32"]]>;
OclBuiltinImpl sub_group_shuffle_up_x64_generic = OclBuiltinImpl<sub_group_shuffle_up_x64, sub_group_shuffle_up_x64.Types, 0, generic_shuffle_up, [["$VF", "64"]]>;

// Block read
code generic_read = [{
$ReturnType helper;
for (int i = 0; i <  $WIDTH * $VF; ++i)
  helper[i] = *($Arg0VarName + i);
return __ocl_shuffle_transpose_$ReturnBaseType#_$VF#x$WIDTH(helper);
}];

// Block write
code generic_write = [{
$Arg1Type helper = __ocl_shuffle_transpose_$Arg1BaseType#_$WIDTH#x$VF($Arg1VarName);
for (int i = 0; i <  $WIDTH * $VF; ++i)
  *($Arg0VarName + i) = helper[i];
}];

// Generated by:
// for op in ['read', 'write']:
//   for vf in [4, 8, 16, 32, 64]:
//     print('// X{} {}'.format(vf, op))
//     for width in [1, 2, 4, 8, 16]:
//       if width != 16:
//         print('OclBuiltinImpl intel_sub_group_block_{2}{0}_{1}_generic = OclBuiltinImpl<intel_sub_group_block_{2}{0}_{1}, intel_sub_group_block_{2}{0}_{1}.Types, 0, generic_{2}, [["$WIDTH", "{0}"], ["$VF", "{1}"]]>;'.format(width, vf, op))
//       print('OclBuiltinImpl intel_sub_group_block_{2}_us{0}_{1}_generic = OclBuiltinImpl<intel_sub_group_block_{2}_us{0}_{1}, intel_sub_group_block_{2}_us{0}_{1}.Types, 0, generic_{2}, [["$WIDTH", "{0}"], ["$VF", "{1}"]]>;'.format(width, vf, op))

// X4 read
OclBuiltinImpl intel_sub_group_block_read1_4_generic = OclBuiltinImpl<intel_sub_group_block_read1_4, intel_sub_group_block_read1_4.Types, 0, generic_read, [["$WIDTH", "1"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read_us1_4_generic = OclBuiltinImpl<intel_sub_group_block_read_us1_4, intel_sub_group_block_read_us1_4.Types, 0, generic_read, [["$WIDTH", "1"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read2_4_generic = OclBuiltinImpl<intel_sub_group_block_read2_4, intel_sub_group_block_read2_4.Types, 0, generic_read, [["$WIDTH", "2"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read_us2_4_generic = OclBuiltinImpl<intel_sub_group_block_read_us2_4, intel_sub_group_block_read_us2_4.Types, 0, generic_read, [["$WIDTH", "2"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read4_4_generic = OclBuiltinImpl<intel_sub_group_block_read4_4, intel_sub_group_block_read4_4.Types, 0, generic_read, [["$WIDTH", "4"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read_us4_4_generic = OclBuiltinImpl<intel_sub_group_block_read_us4_4, intel_sub_group_block_read_us4_4.Types, 0, generic_read, [["$WIDTH", "4"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read8_4_generic = OclBuiltinImpl<intel_sub_group_block_read8_4, intel_sub_group_block_read8_4.Types, 0, generic_read, [["$WIDTH", "8"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read_us8_4_generic = OclBuiltinImpl<intel_sub_group_block_read_us8_4, intel_sub_group_block_read_us8_4.Types, 0, generic_read, [["$WIDTH", "8"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read_us16_4_generic = OclBuiltinImpl<intel_sub_group_block_read_us16_4, intel_sub_group_block_read_us16_4.Types, 0, generic_read, [["$WIDTH", "16"], ["$VF", "4"]]>;
// X8 read
OclBuiltinImpl intel_sub_group_block_read1_8_generic = OclBuiltinImpl<intel_sub_group_block_read1_8, intel_sub_group_block_read1_8.Types, 0, generic_read, [["$WIDTH", "1"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read_us1_8_generic = OclBuiltinImpl<intel_sub_group_block_read_us1_8, intel_sub_group_block_read_us1_8.Types, 0, generic_read, [["$WIDTH", "1"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read2_8_generic = OclBuiltinImpl<intel_sub_group_block_read2_8, intel_sub_group_block_read2_8.Types, 0, generic_read, [["$WIDTH", "2"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read_us2_8_generic = OclBuiltinImpl<intel_sub_group_block_read_us2_8, intel_sub_group_block_read_us2_8.Types, 0, generic_read, [["$WIDTH", "2"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read4_8_generic = OclBuiltinImpl<intel_sub_group_block_read4_8, intel_sub_group_block_read4_8.Types, 0, generic_read, [["$WIDTH", "4"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read_us4_8_generic = OclBuiltinImpl<intel_sub_group_block_read_us4_8, intel_sub_group_block_read_us4_8.Types, 0, generic_read, [["$WIDTH", "4"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read8_8_generic = OclBuiltinImpl<intel_sub_group_block_read8_8, intel_sub_group_block_read8_8.Types, 0, generic_read, [["$WIDTH", "8"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read_us8_8_generic = OclBuiltinImpl<intel_sub_group_block_read_us8_8, intel_sub_group_block_read_us8_8.Types, 0, generic_read, [["$WIDTH", "8"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read_us16_8_generic = OclBuiltinImpl<intel_sub_group_block_read_us16_8, intel_sub_group_block_read_us16_8.Types, 0, generic_read, [["$WIDTH", "16"], ["$VF", "8"]]>;
// X16 read
OclBuiltinImpl intel_sub_group_block_read1_16_generic = OclBuiltinImpl<intel_sub_group_block_read1_16, intel_sub_group_block_read1_16.Types, 0, generic_read, [["$WIDTH", "1"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read_us1_16_generic = OclBuiltinImpl<intel_sub_group_block_read_us1_16, intel_sub_group_block_read_us1_16.Types, 0, generic_read, [["$WIDTH", "1"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read2_16_generic = OclBuiltinImpl<intel_sub_group_block_read2_16, intel_sub_group_block_read2_16.Types, 0, generic_read, [["$WIDTH", "2"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read_us2_16_generic = OclBuiltinImpl<intel_sub_group_block_read_us2_16, intel_sub_group_block_read_us2_16.Types, 0, generic_read, [["$WIDTH", "2"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read4_16_generic = OclBuiltinImpl<intel_sub_group_block_read4_16, intel_sub_group_block_read4_16.Types, 0, generic_read, [["$WIDTH", "4"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read_us4_16_generic = OclBuiltinImpl<intel_sub_group_block_read_us4_16, intel_sub_group_block_read_us4_16.Types, 0, generic_read, [["$WIDTH", "4"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read8_16_generic = OclBuiltinImpl<intel_sub_group_block_read8_16, intel_sub_group_block_read8_16.Types, 0, generic_read, [["$WIDTH", "8"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read_us8_16_generic = OclBuiltinImpl<intel_sub_group_block_read_us8_16, intel_sub_group_block_read_us8_16.Types, 0, generic_read, [["$WIDTH", "8"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read_us16_16_generic = OclBuiltinImpl<intel_sub_group_block_read_us16_16, intel_sub_group_block_read_us16_16.Types, 0, generic_read, [["$WIDTH", "16"], ["$VF", "16"]]>;
// X32 read
OclBuiltinImpl intel_sub_group_block_read1_32_generic = OclBuiltinImpl<intel_sub_group_block_read1_32, intel_sub_group_block_read1_32.Types, 0, generic_read, [["$WIDTH", "1"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read_us1_32_generic = OclBuiltinImpl<intel_sub_group_block_read_us1_32, intel_sub_group_block_read_us1_32.Types, 0, generic_read, [["$WIDTH", "1"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read2_32_generic = OclBuiltinImpl<intel_sub_group_block_read2_32, intel_sub_group_block_read2_32.Types, 0, generic_read, [["$WIDTH", "2"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read_us2_32_generic = OclBuiltinImpl<intel_sub_group_block_read_us2_32, intel_sub_group_block_read_us2_32.Types, 0, generic_read, [["$WIDTH", "2"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read4_32_generic = OclBuiltinImpl<intel_sub_group_block_read4_32, intel_sub_group_block_read4_32.Types, 0, generic_read, [["$WIDTH", "4"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read_us4_32_generic = OclBuiltinImpl<intel_sub_group_block_read_us4_32, intel_sub_group_block_read_us4_32.Types, 0, generic_read, [["$WIDTH", "4"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read8_32_generic = OclBuiltinImpl<intel_sub_group_block_read8_32, intel_sub_group_block_read8_32.Types, 0, generic_read, [["$WIDTH", "8"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read_us8_32_generic = OclBuiltinImpl<intel_sub_group_block_read_us8_32, intel_sub_group_block_read_us8_32.Types, 0, generic_read, [["$WIDTH", "8"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read_us16_32_generic = OclBuiltinImpl<intel_sub_group_block_read_us16_32, intel_sub_group_block_read_us16_32.Types, 0, generic_read, [["$WIDTH", "16"], ["$VF", "32"]]>;
// X64 read
OclBuiltinImpl intel_sub_group_block_read1_64_generic = OclBuiltinImpl<intel_sub_group_block_read1_64, intel_sub_group_block_read1_64.Types, 0, generic_read, [["$WIDTH", "1"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read_us1_64_generic = OclBuiltinImpl<intel_sub_group_block_read_us1_64, intel_sub_group_block_read_us1_64.Types, 0, generic_read, [["$WIDTH", "1"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read2_64_generic = OclBuiltinImpl<intel_sub_group_block_read2_64, intel_sub_group_block_read2_64.Types, 0, generic_read, [["$WIDTH", "2"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read_us2_64_generic = OclBuiltinImpl<intel_sub_group_block_read_us2_64, intel_sub_group_block_read_us2_64.Types, 0, generic_read, [["$WIDTH", "2"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read4_64_generic = OclBuiltinImpl<intel_sub_group_block_read4_64, intel_sub_group_block_read4_64.Types, 0, generic_read, [["$WIDTH", "4"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read_us4_64_generic = OclBuiltinImpl<intel_sub_group_block_read_us4_64, intel_sub_group_block_read_us4_64.Types, 0, generic_read, [["$WIDTH", "4"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read8_64_generic = OclBuiltinImpl<intel_sub_group_block_read8_64, intel_sub_group_block_read8_64.Types, 0, generic_read, [["$WIDTH", "8"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read_us8_64_generic = OclBuiltinImpl<intel_sub_group_block_read_us8_64, intel_sub_group_block_read_us8_64.Types, 0, generic_read, [["$WIDTH", "8"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read_us16_64_generic = OclBuiltinImpl<intel_sub_group_block_read_us16_64, intel_sub_group_block_read_us16_64.Types, 0, generic_read, [["$WIDTH", "16"], ["$VF", "64"]]>;
// X4 write
OclBuiltinImpl intel_sub_group_block_write1_4_generic = OclBuiltinImpl<intel_sub_group_block_write1_4, intel_sub_group_block_write1_4.Types, 0, generic_write, [["$WIDTH", "1"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write_us1_4_generic = OclBuiltinImpl<intel_sub_group_block_write_us1_4, intel_sub_group_block_write_us1_4.Types, 0, generic_write, [["$WIDTH", "1"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write2_4_generic = OclBuiltinImpl<intel_sub_group_block_write2_4, intel_sub_group_block_write2_4.Types, 0, generic_write, [["$WIDTH", "2"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write_us2_4_generic = OclBuiltinImpl<intel_sub_group_block_write_us2_4, intel_sub_group_block_write_us2_4.Types, 0, generic_write, [["$WIDTH", "2"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write4_4_generic = OclBuiltinImpl<intel_sub_group_block_write4_4, intel_sub_group_block_write4_4.Types, 0, generic_write, [["$WIDTH", "4"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write_us4_4_generic = OclBuiltinImpl<intel_sub_group_block_write_us4_4, intel_sub_group_block_write_us4_4.Types, 0, generic_write, [["$WIDTH", "4"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write8_4_generic = OclBuiltinImpl<intel_sub_group_block_write8_4, intel_sub_group_block_write8_4.Types, 0, generic_write, [["$WIDTH", "8"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write_us8_4_generic = OclBuiltinImpl<intel_sub_group_block_write_us8_4, intel_sub_group_block_write_us8_4.Types, 0, generic_write, [["$WIDTH", "8"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write_us16_4_generic = OclBuiltinImpl<intel_sub_group_block_write_us16_4, intel_sub_group_block_write_us16_4.Types, 0, generic_write, [["$WIDTH", "16"], ["$VF", "4"]]>;
// X8 write
OclBuiltinImpl intel_sub_group_block_write1_8_generic = OclBuiltinImpl<intel_sub_group_block_write1_8, intel_sub_group_block_write1_8.Types, 0, generic_write, [["$WIDTH", "1"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write_us1_8_generic = OclBuiltinImpl<intel_sub_group_block_write_us1_8, intel_sub_group_block_write_us1_8.Types, 0, generic_write, [["$WIDTH", "1"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write2_8_generic = OclBuiltinImpl<intel_sub_group_block_write2_8, intel_sub_group_block_write2_8.Types, 0, generic_write, [["$WIDTH", "2"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write_us2_8_generic = OclBuiltinImpl<intel_sub_group_block_write_us2_8, intel_sub_group_block_write_us2_8.Types, 0, generic_write, [["$WIDTH", "2"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write4_8_generic = OclBuiltinImpl<intel_sub_group_block_write4_8, intel_sub_group_block_write4_8.Types, 0, generic_write, [["$WIDTH", "4"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write_us4_8_generic = OclBuiltinImpl<intel_sub_group_block_write_us4_8, intel_sub_group_block_write_us4_8.Types, 0, generic_write, [["$WIDTH", "4"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write8_8_generic = OclBuiltinImpl<intel_sub_group_block_write8_8, intel_sub_group_block_write8_8.Types, 0, generic_write, [["$WIDTH", "8"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write_us8_8_generic = OclBuiltinImpl<intel_sub_group_block_write_us8_8, intel_sub_group_block_write_us8_8.Types, 0, generic_write, [["$WIDTH", "8"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write_us16_8_generic = OclBuiltinImpl<intel_sub_group_block_write_us16_8, intel_sub_group_block_write_us16_8.Types, 0, generic_write, [["$WIDTH", "16"], ["$VF", "8"]]>;
// X16 write
OclBuiltinImpl intel_sub_group_block_write1_16_generic = OclBuiltinImpl<intel_sub_group_block_write1_16, intel_sub_group_block_write1_16.Types, 0, generic_write, [["$WIDTH", "1"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write_us1_16_generic = OclBuiltinImpl<intel_sub_group_block_write_us1_16, intel_sub_group_block_write_us1_16.Types, 0, generic_write, [["$WIDTH", "1"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write2_16_generic = OclBuiltinImpl<intel_sub_group_block_write2_16, intel_sub_group_block_write2_16.Types, 0, generic_write, [["$WIDTH", "2"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write_us2_16_generic = OclBuiltinImpl<intel_sub_group_block_write_us2_16, intel_sub_group_block_write_us2_16.Types, 0, generic_write, [["$WIDTH", "2"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write4_16_generic = OclBuiltinImpl<intel_sub_group_block_write4_16, intel_sub_group_block_write4_16.Types, 0, generic_write, [["$WIDTH", "4"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write_us4_16_generic = OclBuiltinImpl<intel_sub_group_block_write_us4_16, intel_sub_group_block_write_us4_16.Types, 0, generic_write, [["$WIDTH", "4"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write8_16_generic = OclBuiltinImpl<intel_sub_group_block_write8_16, intel_sub_group_block_write8_16.Types, 0, generic_write, [["$WIDTH", "8"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write_us8_16_generic = OclBuiltinImpl<intel_sub_group_block_write_us8_16, intel_sub_group_block_write_us8_16.Types, 0, generic_write, [["$WIDTH", "8"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write_us16_16_generic = OclBuiltinImpl<intel_sub_group_block_write_us16_16, intel_sub_group_block_write_us16_16.Types, 0, generic_write, [["$WIDTH", "16"], ["$VF", "16"]]>;
// X32 write
OclBuiltinImpl intel_sub_group_block_write1_32_generic = OclBuiltinImpl<intel_sub_group_block_write1_32, intel_sub_group_block_write1_32.Types, 0, generic_write, [["$WIDTH", "1"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write_us1_32_generic = OclBuiltinImpl<intel_sub_group_block_write_us1_32, intel_sub_group_block_write_us1_32.Types, 0, generic_write, [["$WIDTH", "1"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write2_32_generic = OclBuiltinImpl<intel_sub_group_block_write2_32, intel_sub_group_block_write2_32.Types, 0, generic_write, [["$WIDTH", "2"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write_us2_32_generic = OclBuiltinImpl<intel_sub_group_block_write_us2_32, intel_sub_group_block_write_us2_32.Types, 0, generic_write, [["$WIDTH", "2"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write4_32_generic = OclBuiltinImpl<intel_sub_group_block_write4_32, intel_sub_group_block_write4_32.Types, 0, generic_write, [["$WIDTH", "4"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write_us4_32_generic = OclBuiltinImpl<intel_sub_group_block_write_us4_32, intel_sub_group_block_write_us4_32.Types, 0, generic_write, [["$WIDTH", "4"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write8_32_generic = OclBuiltinImpl<intel_sub_group_block_write8_32, intel_sub_group_block_write8_32.Types, 0, generic_write, [["$WIDTH", "8"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write_us8_32_generic = OclBuiltinImpl<intel_sub_group_block_write_us8_32, intel_sub_group_block_write_us8_32.Types, 0, generic_write, [["$WIDTH", "8"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write_us16_32_generic = OclBuiltinImpl<intel_sub_group_block_write_us16_32, intel_sub_group_block_write_us16_32.Types, 0, generic_write, [["$WIDTH", "16"], ["$VF", "32"]]>;
// X64 write
OclBuiltinImpl intel_sub_group_block_write1_64_generic = OclBuiltinImpl<intel_sub_group_block_write1_64, intel_sub_group_block_write1_64.Types, 0, generic_write, [["$WIDTH", "1"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write_us1_64_generic = OclBuiltinImpl<intel_sub_group_block_write_us1_64, intel_sub_group_block_write_us1_64.Types, 0, generic_write, [["$WIDTH", "1"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write2_64_generic = OclBuiltinImpl<intel_sub_group_block_write2_64, intel_sub_group_block_write2_64.Types, 0, generic_write, [["$WIDTH", "2"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write_us2_64_generic = OclBuiltinImpl<intel_sub_group_block_write_us2_64, intel_sub_group_block_write_us2_64.Types, 0, generic_write, [["$WIDTH", "2"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write4_64_generic = OclBuiltinImpl<intel_sub_group_block_write4_64, intel_sub_group_block_write4_64.Types, 0, generic_write, [["$WIDTH", "4"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write_us4_64_generic = OclBuiltinImpl<intel_sub_group_block_write_us4_64, intel_sub_group_block_write_us4_64.Types, 0, generic_write, [["$WIDTH", "4"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write8_64_generic = OclBuiltinImpl<intel_sub_group_block_write8_64, intel_sub_group_block_write8_64.Types, 0, generic_write, [["$WIDTH", "8"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write_us8_64_generic = OclBuiltinImpl<intel_sub_group_block_write_us8_64, intel_sub_group_block_write_us8_64.Types, 0, generic_write, [["$WIDTH", "8"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write_us16_64_generic = OclBuiltinImpl<intel_sub_group_block_write_us16_64, intel_sub_group_block_write_us16_64.Types, 0, generic_write, [["$WIDTH", "16"], ["$VF", "64"]]>;


code generic_read_img =
[{
$ReturnType res;
$ReturnBaseType *r = ($ReturnBaseType*)&res;
$ReturnBaseType#$VF* p;
for (int i=0; i < $WIDTH; ++i) {
  p = ($ReturnBaseType#$VF *)extract_pixel_in_byte($Arg0VarName, $Arg1VarName);
  *($ReturnBaseType#$VF*)&r[i * $VF] = *p;
  $Arg1VarName.hi += 1;
}
return __ocl_shuffle_transpose_$ReturnBaseType#_$VF#x$WIDTH(res);
}];

code  generic_write_img =
[{
$Arg2Type d = __ocl_shuffle_transpose_$Arg2BaseType#_$WIDTH#x$VF($Arg2VarName);
$Arg2BaseType *r = ($Arg2BaseType*)&d;
$Arg2BaseType#$VF* p;
for (int i=0; i < $WIDTH; ++i) {
  p = ($Arg2BaseType#$VF *)extract_pixel_in_byte($Arg0VarName, $Arg1VarName);
  *p = *($Arg2BaseType#$VF *)&r[i * $VF];
  $Arg1VarName.hi += 1;
}
}];

// Generated by:
// for op in ['read', 'write']:
//   for vf in [4, 8, 16, 32, 64]:
//     print('// X{} {}'.format(vf, op))
//     for width in [1, 2, 4, 8, 16]:
//       if width != 16:
//         print('OclBuiltinImpl intel_sub_group_block_{2}{0}_{1}_img_generic = OclBuiltinImpl<intel_sub_group_block_{2}{0}_{1}_img, intel_sub_group_block_{2}{0}_{1}_img.Types, 0, generic_{2}_img, [["$WIDTH", "{0}"], ["$VF", "{1}"]]>;'.format(width, vf, op))
//         print('OclBuiltinImpl intel_sub_group_block_{2}{0}_{1}_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_{2}{0}_{1}_img_rw, intel_sub_group_block_{2}{0}_{1}_img_rw.Types, 0, generic_{2}_img, [["$WIDTH", "{0}"], ["$VF", "{1}"]]>;'.format(width, vf, op))
//       print('OclBuiltinImpl intel_sub_group_block_{2}_us{0}_{1}_img_generic = OclBuiltinImpl<intel_sub_group_block_{2}_us{0}_{1}_img, intel_sub_group_block_{2}_us{0}_{1}_img.Types, 0, generic_{2}_img, [["$WIDTH", "{0}"], ["$VF", "{1}"]]>;'.format(width, vf, op))
//       print('OclBuiltinImpl intel_sub_group_block_{2}_us{0}_{1}_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_{2}_us{0}_{1}_img_rw, intel_sub_group_block_{2}_us{0}_{1}_img_rw.Types, 0, generic_{2}_img, [["$WIDTH", "{0}"], ["$VF", "{1}"]]>;'.format(width, vf, op))

// X4 read
OclBuiltinImpl intel_sub_group_block_read1_4_img_generic = OclBuiltinImpl<intel_sub_group_block_read1_4_img, intel_sub_group_block_read1_4_img.Types, 0, generic_read_img, [["$WIDTH", "1"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read1_4_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read1_4_img_rw, intel_sub_group_block_read1_4_img_rw.Types, 0, generic_read_img, [["$WIDTH", "1"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read_us1_4_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us1_4_img, intel_sub_group_block_read_us1_4_img.Types, 0, generic_read_img, [["$WIDTH", "1"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read_us1_4_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us1_4_img_rw, intel_sub_group_block_read_us1_4_img_rw.Types, 0, generic_read_img, [["$WIDTH", "1"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read2_4_img_generic = OclBuiltinImpl<intel_sub_group_block_read2_4_img, intel_sub_group_block_read2_4_img.Types, 0, generic_read_img, [["$WIDTH", "2"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read2_4_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read2_4_img_rw, intel_sub_group_block_read2_4_img_rw.Types, 0, generic_read_img, [["$WIDTH", "2"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read_us2_4_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us2_4_img, intel_sub_group_block_read_us2_4_img.Types, 0, generic_read_img, [["$WIDTH", "2"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read_us2_4_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us2_4_img_rw, intel_sub_group_block_read_us2_4_img_rw.Types, 0, generic_read_img, [["$WIDTH", "2"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read4_4_img_generic = OclBuiltinImpl<intel_sub_group_block_read4_4_img, intel_sub_group_block_read4_4_img.Types, 0, generic_read_img, [["$WIDTH", "4"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read4_4_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read4_4_img_rw, intel_sub_group_block_read4_4_img_rw.Types, 0, generic_read_img, [["$WIDTH", "4"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read_us4_4_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us4_4_img, intel_sub_group_block_read_us4_4_img.Types, 0, generic_read_img, [["$WIDTH", "4"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read_us4_4_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us4_4_img_rw, intel_sub_group_block_read_us4_4_img_rw.Types, 0, generic_read_img, [["$WIDTH", "4"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read8_4_img_generic = OclBuiltinImpl<intel_sub_group_block_read8_4_img, intel_sub_group_block_read8_4_img.Types, 0, generic_read_img, [["$WIDTH", "8"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read8_4_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read8_4_img_rw, intel_sub_group_block_read8_4_img_rw.Types, 0, generic_read_img, [["$WIDTH", "8"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read_us8_4_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us8_4_img, intel_sub_group_block_read_us8_4_img.Types, 0, generic_read_img, [["$WIDTH", "8"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read_us8_4_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us8_4_img_rw, intel_sub_group_block_read_us8_4_img_rw.Types, 0, generic_read_img, [["$WIDTH", "8"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read_us16_4_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us16_4_img, intel_sub_group_block_read_us16_4_img.Types, 0, generic_read_img, [["$WIDTH", "16"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_read_us16_4_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us16_4_img_rw, intel_sub_group_block_read_us16_4_img_rw.Types, 0, generic_read_img, [["$WIDTH", "16"], ["$VF", "4"]]>;
// X8 read
OclBuiltinImpl intel_sub_group_block_read1_8_img_generic = OclBuiltinImpl<intel_sub_group_block_read1_8_img, intel_sub_group_block_read1_8_img.Types, 0, generic_read_img, [["$WIDTH", "1"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read1_8_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read1_8_img_rw, intel_sub_group_block_read1_8_img_rw.Types, 0, generic_read_img, [["$WIDTH", "1"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read_us1_8_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us1_8_img, intel_sub_group_block_read_us1_8_img.Types, 0, generic_read_img, [["$WIDTH", "1"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read_us1_8_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us1_8_img_rw, intel_sub_group_block_read_us1_8_img_rw.Types, 0, generic_read_img, [["$WIDTH", "1"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read2_8_img_generic = OclBuiltinImpl<intel_sub_group_block_read2_8_img, intel_sub_group_block_read2_8_img.Types, 0, generic_read_img, [["$WIDTH", "2"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read2_8_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read2_8_img_rw, intel_sub_group_block_read2_8_img_rw.Types, 0, generic_read_img, [["$WIDTH", "2"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read_us2_8_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us2_8_img, intel_sub_group_block_read_us2_8_img.Types, 0, generic_read_img, [["$WIDTH", "2"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read_us2_8_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us2_8_img_rw, intel_sub_group_block_read_us2_8_img_rw.Types, 0, generic_read_img, [["$WIDTH", "2"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read4_8_img_generic = OclBuiltinImpl<intel_sub_group_block_read4_8_img, intel_sub_group_block_read4_8_img.Types, 0, generic_read_img, [["$WIDTH", "4"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read4_8_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read4_8_img_rw, intel_sub_group_block_read4_8_img_rw.Types, 0, generic_read_img, [["$WIDTH", "4"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read_us4_8_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us4_8_img, intel_sub_group_block_read_us4_8_img.Types, 0, generic_read_img, [["$WIDTH", "4"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read_us4_8_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us4_8_img_rw, intel_sub_group_block_read_us4_8_img_rw.Types, 0, generic_read_img, [["$WIDTH", "4"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read8_8_img_generic = OclBuiltinImpl<intel_sub_group_block_read8_8_img, intel_sub_group_block_read8_8_img.Types, 0, generic_read_img, [["$WIDTH", "8"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read8_8_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read8_8_img_rw, intel_sub_group_block_read8_8_img_rw.Types, 0, generic_read_img, [["$WIDTH", "8"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read_us8_8_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us8_8_img, intel_sub_group_block_read_us8_8_img.Types, 0, generic_read_img, [["$WIDTH", "8"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read_us8_8_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us8_8_img_rw, intel_sub_group_block_read_us8_8_img_rw.Types, 0, generic_read_img, [["$WIDTH", "8"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read_us16_8_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us16_8_img, intel_sub_group_block_read_us16_8_img.Types, 0, generic_read_img, [["$WIDTH", "16"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_read_us16_8_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us16_8_img_rw, intel_sub_group_block_read_us16_8_img_rw.Types, 0, generic_read_img, [["$WIDTH", "16"], ["$VF", "8"]]>;
// X16 read
OclBuiltinImpl intel_sub_group_block_read1_16_img_generic = OclBuiltinImpl<intel_sub_group_block_read1_16_img, intel_sub_group_block_read1_16_img.Types, 0, generic_read_img, [["$WIDTH", "1"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read1_16_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read1_16_img_rw, intel_sub_group_block_read1_16_img_rw.Types, 0, generic_read_img, [["$WIDTH", "1"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read_us1_16_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us1_16_img, intel_sub_group_block_read_us1_16_img.Types, 0, generic_read_img, [["$WIDTH", "1"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read_us1_16_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us1_16_img_rw, intel_sub_group_block_read_us1_16_img_rw.Types, 0, generic_read_img, [["$WIDTH", "1"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read2_16_img_generic = OclBuiltinImpl<intel_sub_group_block_read2_16_img, intel_sub_group_block_read2_16_img.Types, 0, generic_read_img, [["$WIDTH", "2"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read2_16_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read2_16_img_rw, intel_sub_group_block_read2_16_img_rw.Types, 0, generic_read_img, [["$WIDTH", "2"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read_us2_16_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us2_16_img, intel_sub_group_block_read_us2_16_img.Types, 0, generic_read_img, [["$WIDTH", "2"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read_us2_16_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us2_16_img_rw, intel_sub_group_block_read_us2_16_img_rw.Types, 0, generic_read_img, [["$WIDTH", "2"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read4_16_img_generic = OclBuiltinImpl<intel_sub_group_block_read4_16_img, intel_sub_group_block_read4_16_img.Types, 0, generic_read_img, [["$WIDTH", "4"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read4_16_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read4_16_img_rw, intel_sub_group_block_read4_16_img_rw.Types, 0, generic_read_img, [["$WIDTH", "4"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read_us4_16_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us4_16_img, intel_sub_group_block_read_us4_16_img.Types, 0, generic_read_img, [["$WIDTH", "4"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read_us4_16_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us4_16_img_rw, intel_sub_group_block_read_us4_16_img_rw.Types, 0, generic_read_img, [["$WIDTH", "4"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read8_16_img_generic = OclBuiltinImpl<intel_sub_group_block_read8_16_img, intel_sub_group_block_read8_16_img.Types, 0, generic_read_img, [["$WIDTH", "8"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read8_16_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read8_16_img_rw, intel_sub_group_block_read8_16_img_rw.Types, 0, generic_read_img, [["$WIDTH", "8"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read_us8_16_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us8_16_img, intel_sub_group_block_read_us8_16_img.Types, 0, generic_read_img, [["$WIDTH", "8"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read_us8_16_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us8_16_img_rw, intel_sub_group_block_read_us8_16_img_rw.Types, 0, generic_read_img, [["$WIDTH", "8"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read_us16_16_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us16_16_img, intel_sub_group_block_read_us16_16_img.Types, 0, generic_read_img, [["$WIDTH", "16"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_read_us16_16_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us16_16_img_rw, intel_sub_group_block_read_us16_16_img_rw.Types, 0, generic_read_img, [["$WIDTH", "16"], ["$VF", "16"]]>;
// X32 read
OclBuiltinImpl intel_sub_group_block_read1_32_img_generic = OclBuiltinImpl<intel_sub_group_block_read1_32_img, intel_sub_group_block_read1_32_img.Types, 0, generic_read_img, [["$WIDTH", "1"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read1_32_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read1_32_img_rw, intel_sub_group_block_read1_32_img_rw.Types, 0, generic_read_img, [["$WIDTH", "1"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read_us1_32_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us1_32_img, intel_sub_group_block_read_us1_32_img.Types, 0, generic_read_img, [["$WIDTH", "1"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read_us1_32_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us1_32_img_rw, intel_sub_group_block_read_us1_32_img_rw.Types, 0, generic_read_img, [["$WIDTH", "1"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read2_32_img_generic = OclBuiltinImpl<intel_sub_group_block_read2_32_img, intel_sub_group_block_read2_32_img.Types, 0, generic_read_img, [["$WIDTH", "2"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read2_32_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read2_32_img_rw, intel_sub_group_block_read2_32_img_rw.Types, 0, generic_read_img, [["$WIDTH", "2"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read_us2_32_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us2_32_img, intel_sub_group_block_read_us2_32_img.Types, 0, generic_read_img, [["$WIDTH", "2"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read_us2_32_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us2_32_img_rw, intel_sub_group_block_read_us2_32_img_rw.Types, 0, generic_read_img, [["$WIDTH", "2"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read4_32_img_generic = OclBuiltinImpl<intel_sub_group_block_read4_32_img, intel_sub_group_block_read4_32_img.Types, 0, generic_read_img, [["$WIDTH", "4"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read4_32_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read4_32_img_rw, intel_sub_group_block_read4_32_img_rw.Types, 0, generic_read_img, [["$WIDTH", "4"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read_us4_32_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us4_32_img, intel_sub_group_block_read_us4_32_img.Types, 0, generic_read_img, [["$WIDTH", "4"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read_us4_32_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us4_32_img_rw, intel_sub_group_block_read_us4_32_img_rw.Types, 0, generic_read_img, [["$WIDTH", "4"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read8_32_img_generic = OclBuiltinImpl<intel_sub_group_block_read8_32_img, intel_sub_group_block_read8_32_img.Types, 0, generic_read_img, [["$WIDTH", "8"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read8_32_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read8_32_img_rw, intel_sub_group_block_read8_32_img_rw.Types, 0, generic_read_img, [["$WIDTH", "8"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read_us8_32_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us8_32_img, intel_sub_group_block_read_us8_32_img.Types, 0, generic_read_img, [["$WIDTH", "8"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read_us8_32_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us8_32_img_rw, intel_sub_group_block_read_us8_32_img_rw.Types, 0, generic_read_img, [["$WIDTH", "8"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read_us16_32_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us16_32_img, intel_sub_group_block_read_us16_32_img.Types, 0, generic_read_img, [["$WIDTH", "16"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_read_us16_32_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us16_32_img_rw, intel_sub_group_block_read_us16_32_img_rw.Types, 0, generic_read_img, [["$WIDTH", "16"], ["$VF", "32"]]>;
// X64 read
OclBuiltinImpl intel_sub_group_block_read1_64_img_generic = OclBuiltinImpl<intel_sub_group_block_read1_64_img, intel_sub_group_block_read1_64_img.Types, 0, generic_read_img, [["$WIDTH", "1"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read1_64_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read1_64_img_rw, intel_sub_group_block_read1_64_img_rw.Types, 0, generic_read_img, [["$WIDTH", "1"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read_us1_64_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us1_64_img, intel_sub_group_block_read_us1_64_img.Types, 0, generic_read_img, [["$WIDTH", "1"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read_us1_64_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us1_64_img_rw, intel_sub_group_block_read_us1_64_img_rw.Types, 0, generic_read_img, [["$WIDTH", "1"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read2_64_img_generic = OclBuiltinImpl<intel_sub_group_block_read2_64_img, intel_sub_group_block_read2_64_img.Types, 0, generic_read_img, [["$WIDTH", "2"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read2_64_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read2_64_img_rw, intel_sub_group_block_read2_64_img_rw.Types, 0, generic_read_img, [["$WIDTH", "2"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read_us2_64_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us2_64_img, intel_sub_group_block_read_us2_64_img.Types, 0, generic_read_img, [["$WIDTH", "2"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read_us2_64_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us2_64_img_rw, intel_sub_group_block_read_us2_64_img_rw.Types, 0, generic_read_img, [["$WIDTH", "2"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read4_64_img_generic = OclBuiltinImpl<intel_sub_group_block_read4_64_img, intel_sub_group_block_read4_64_img.Types, 0, generic_read_img, [["$WIDTH", "4"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read4_64_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read4_64_img_rw, intel_sub_group_block_read4_64_img_rw.Types, 0, generic_read_img, [["$WIDTH", "4"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read_us4_64_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us4_64_img, intel_sub_group_block_read_us4_64_img.Types, 0, generic_read_img, [["$WIDTH", "4"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read_us4_64_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us4_64_img_rw, intel_sub_group_block_read_us4_64_img_rw.Types, 0, generic_read_img, [["$WIDTH", "4"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read8_64_img_generic = OclBuiltinImpl<intel_sub_group_block_read8_64_img, intel_sub_group_block_read8_64_img.Types, 0, generic_read_img, [["$WIDTH", "8"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read8_64_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read8_64_img_rw, intel_sub_group_block_read8_64_img_rw.Types, 0, generic_read_img, [["$WIDTH", "8"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read_us8_64_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us8_64_img, intel_sub_group_block_read_us8_64_img.Types, 0, generic_read_img, [["$WIDTH", "8"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read_us8_64_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us8_64_img_rw, intel_sub_group_block_read_us8_64_img_rw.Types, 0, generic_read_img, [["$WIDTH", "8"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read_us16_64_img_generic = OclBuiltinImpl<intel_sub_group_block_read_us16_64_img, intel_sub_group_block_read_us16_64_img.Types, 0, generic_read_img, [["$WIDTH", "16"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_read_us16_64_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_read_us16_64_img_rw, intel_sub_group_block_read_us16_64_img_rw.Types, 0, generic_read_img, [["$WIDTH", "16"], ["$VF", "64"]]>;
// X4 write
OclBuiltinImpl intel_sub_group_block_write1_4_img_generic = OclBuiltinImpl<intel_sub_group_block_write1_4_img, intel_sub_group_block_write1_4_img.Types, 0, generic_write_img, [["$WIDTH", "1"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write1_4_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write1_4_img_rw, intel_sub_group_block_write1_4_img_rw.Types, 0, generic_write_img, [["$WIDTH", "1"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write_us1_4_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us1_4_img, intel_sub_group_block_write_us1_4_img.Types, 0, generic_write_img, [["$WIDTH", "1"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write_us1_4_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us1_4_img_rw, intel_sub_group_block_write_us1_4_img_rw.Types, 0, generic_write_img, [["$WIDTH", "1"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write2_4_img_generic = OclBuiltinImpl<intel_sub_group_block_write2_4_img, intel_sub_group_block_write2_4_img.Types, 0, generic_write_img, [["$WIDTH", "2"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write2_4_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write2_4_img_rw, intel_sub_group_block_write2_4_img_rw.Types, 0, generic_write_img, [["$WIDTH", "2"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write_us2_4_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us2_4_img, intel_sub_group_block_write_us2_4_img.Types, 0, generic_write_img, [["$WIDTH", "2"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write_us2_4_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us2_4_img_rw, intel_sub_group_block_write_us2_4_img_rw.Types, 0, generic_write_img, [["$WIDTH", "2"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write4_4_img_generic = OclBuiltinImpl<intel_sub_group_block_write4_4_img, intel_sub_group_block_write4_4_img.Types, 0, generic_write_img, [["$WIDTH", "4"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write4_4_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write4_4_img_rw, intel_sub_group_block_write4_4_img_rw.Types, 0, generic_write_img, [["$WIDTH", "4"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write_us4_4_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us4_4_img, intel_sub_group_block_write_us4_4_img.Types, 0, generic_write_img, [["$WIDTH", "4"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write_us4_4_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us4_4_img_rw, intel_sub_group_block_write_us4_4_img_rw.Types, 0, generic_write_img, [["$WIDTH", "4"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write8_4_img_generic = OclBuiltinImpl<intel_sub_group_block_write8_4_img, intel_sub_group_block_write8_4_img.Types, 0, generic_write_img, [["$WIDTH", "8"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write8_4_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write8_4_img_rw, intel_sub_group_block_write8_4_img_rw.Types, 0, generic_write_img, [["$WIDTH", "8"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write_us8_4_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us8_4_img, intel_sub_group_block_write_us8_4_img.Types, 0, generic_write_img, [["$WIDTH", "8"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write_us8_4_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us8_4_img_rw, intel_sub_group_block_write_us8_4_img_rw.Types, 0, generic_write_img, [["$WIDTH", "8"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write_us16_4_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us16_4_img, intel_sub_group_block_write_us16_4_img.Types, 0, generic_write_img, [["$WIDTH", "16"], ["$VF", "4"]]>;
OclBuiltinImpl intel_sub_group_block_write_us16_4_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us16_4_img_rw, intel_sub_group_block_write_us16_4_img_rw.Types, 0, generic_write_img, [["$WIDTH", "16"], ["$VF", "4"]]>;
// X8 write
OclBuiltinImpl intel_sub_group_block_write1_8_img_generic = OclBuiltinImpl<intel_sub_group_block_write1_8_img, intel_sub_group_block_write1_8_img.Types, 0, generic_write_img, [["$WIDTH", "1"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write1_8_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write1_8_img_rw, intel_sub_group_block_write1_8_img_rw.Types, 0, generic_write_img, [["$WIDTH", "1"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write_us1_8_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us1_8_img, intel_sub_group_block_write_us1_8_img.Types, 0, generic_write_img, [["$WIDTH", "1"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write_us1_8_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us1_8_img_rw, intel_sub_group_block_write_us1_8_img_rw.Types, 0, generic_write_img, [["$WIDTH", "1"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write2_8_img_generic = OclBuiltinImpl<intel_sub_group_block_write2_8_img, intel_sub_group_block_write2_8_img.Types, 0, generic_write_img, [["$WIDTH", "2"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write2_8_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write2_8_img_rw, intel_sub_group_block_write2_8_img_rw.Types, 0, generic_write_img, [["$WIDTH", "2"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write_us2_8_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us2_8_img, intel_sub_group_block_write_us2_8_img.Types, 0, generic_write_img, [["$WIDTH", "2"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write_us2_8_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us2_8_img_rw, intel_sub_group_block_write_us2_8_img_rw.Types, 0, generic_write_img, [["$WIDTH", "2"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write4_8_img_generic = OclBuiltinImpl<intel_sub_group_block_write4_8_img, intel_sub_group_block_write4_8_img.Types, 0, generic_write_img, [["$WIDTH", "4"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write4_8_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write4_8_img_rw, intel_sub_group_block_write4_8_img_rw.Types, 0, generic_write_img, [["$WIDTH", "4"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write_us4_8_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us4_8_img, intel_sub_group_block_write_us4_8_img.Types, 0, generic_write_img, [["$WIDTH", "4"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write_us4_8_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us4_8_img_rw, intel_sub_group_block_write_us4_8_img_rw.Types, 0, generic_write_img, [["$WIDTH", "4"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write8_8_img_generic = OclBuiltinImpl<intel_sub_group_block_write8_8_img, intel_sub_group_block_write8_8_img.Types, 0, generic_write_img, [["$WIDTH", "8"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write8_8_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write8_8_img_rw, intel_sub_group_block_write8_8_img_rw.Types, 0, generic_write_img, [["$WIDTH", "8"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write_us8_8_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us8_8_img, intel_sub_group_block_write_us8_8_img.Types, 0, generic_write_img, [["$WIDTH", "8"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write_us8_8_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us8_8_img_rw, intel_sub_group_block_write_us8_8_img_rw.Types, 0, generic_write_img, [["$WIDTH", "8"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write_us16_8_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us16_8_img, intel_sub_group_block_write_us16_8_img.Types, 0, generic_write_img, [["$WIDTH", "16"], ["$VF", "8"]]>;
OclBuiltinImpl intel_sub_group_block_write_us16_8_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us16_8_img_rw, intel_sub_group_block_write_us16_8_img_rw.Types, 0, generic_write_img, [["$WIDTH", "16"], ["$VF", "8"]]>;
// X16 write
OclBuiltinImpl intel_sub_group_block_write1_16_img_generic = OclBuiltinImpl<intel_sub_group_block_write1_16_img, intel_sub_group_block_write1_16_img.Types, 0, generic_write_img, [["$WIDTH", "1"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write1_16_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write1_16_img_rw, intel_sub_group_block_write1_16_img_rw.Types, 0, generic_write_img, [["$WIDTH", "1"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write_us1_16_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us1_16_img, intel_sub_group_block_write_us1_16_img.Types, 0, generic_write_img, [["$WIDTH", "1"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write_us1_16_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us1_16_img_rw, intel_sub_group_block_write_us1_16_img_rw.Types, 0, generic_write_img, [["$WIDTH", "1"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write2_16_img_generic = OclBuiltinImpl<intel_sub_group_block_write2_16_img, intel_sub_group_block_write2_16_img.Types, 0, generic_write_img, [["$WIDTH", "2"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write2_16_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write2_16_img_rw, intel_sub_group_block_write2_16_img_rw.Types, 0, generic_write_img, [["$WIDTH", "2"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write_us2_16_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us2_16_img, intel_sub_group_block_write_us2_16_img.Types, 0, generic_write_img, [["$WIDTH", "2"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write_us2_16_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us2_16_img_rw, intel_sub_group_block_write_us2_16_img_rw.Types, 0, generic_write_img, [["$WIDTH", "2"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write4_16_img_generic = OclBuiltinImpl<intel_sub_group_block_write4_16_img, intel_sub_group_block_write4_16_img.Types, 0, generic_write_img, [["$WIDTH", "4"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write4_16_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write4_16_img_rw, intel_sub_group_block_write4_16_img_rw.Types, 0, generic_write_img, [["$WIDTH", "4"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write_us4_16_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us4_16_img, intel_sub_group_block_write_us4_16_img.Types, 0, generic_write_img, [["$WIDTH", "4"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write_us4_16_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us4_16_img_rw, intel_sub_group_block_write_us4_16_img_rw.Types, 0, generic_write_img, [["$WIDTH", "4"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write8_16_img_generic = OclBuiltinImpl<intel_sub_group_block_write8_16_img, intel_sub_group_block_write8_16_img.Types, 0, generic_write_img, [["$WIDTH", "8"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write8_16_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write8_16_img_rw, intel_sub_group_block_write8_16_img_rw.Types, 0, generic_write_img, [["$WIDTH", "8"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write_us8_16_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us8_16_img, intel_sub_group_block_write_us8_16_img.Types, 0, generic_write_img, [["$WIDTH", "8"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write_us8_16_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us8_16_img_rw, intel_sub_group_block_write_us8_16_img_rw.Types, 0, generic_write_img, [["$WIDTH", "8"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write_us16_16_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us16_16_img, intel_sub_group_block_write_us16_16_img.Types, 0, generic_write_img, [["$WIDTH", "16"], ["$VF", "16"]]>;
OclBuiltinImpl intel_sub_group_block_write_us16_16_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us16_16_img_rw, intel_sub_group_block_write_us16_16_img_rw.Types, 0, generic_write_img, [["$WIDTH", "16"], ["$VF", "16"]]>;
// X32 write
OclBuiltinImpl intel_sub_group_block_write1_32_img_generic = OclBuiltinImpl<intel_sub_group_block_write1_32_img, intel_sub_group_block_write1_32_img.Types, 0, generic_write_img, [["$WIDTH", "1"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write1_32_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write1_32_img_rw, intel_sub_group_block_write1_32_img_rw.Types, 0, generic_write_img, [["$WIDTH", "1"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write_us1_32_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us1_32_img, intel_sub_group_block_write_us1_32_img.Types, 0, generic_write_img, [["$WIDTH", "1"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write_us1_32_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us1_32_img_rw, intel_sub_group_block_write_us1_32_img_rw.Types, 0, generic_write_img, [["$WIDTH", "1"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write2_32_img_generic = OclBuiltinImpl<intel_sub_group_block_write2_32_img, intel_sub_group_block_write2_32_img.Types, 0, generic_write_img, [["$WIDTH", "2"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write2_32_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write2_32_img_rw, intel_sub_group_block_write2_32_img_rw.Types, 0, generic_write_img, [["$WIDTH", "2"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write_us2_32_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us2_32_img, intel_sub_group_block_write_us2_32_img.Types, 0, generic_write_img, [["$WIDTH", "2"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write_us2_32_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us2_32_img_rw, intel_sub_group_block_write_us2_32_img_rw.Types, 0, generic_write_img, [["$WIDTH", "2"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write4_32_img_generic = OclBuiltinImpl<intel_sub_group_block_write4_32_img, intel_sub_group_block_write4_32_img.Types, 0, generic_write_img, [["$WIDTH", "4"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write4_32_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write4_32_img_rw, intel_sub_group_block_write4_32_img_rw.Types, 0, generic_write_img, [["$WIDTH", "4"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write_us4_32_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us4_32_img, intel_sub_group_block_write_us4_32_img.Types, 0, generic_write_img, [["$WIDTH", "4"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write_us4_32_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us4_32_img_rw, intel_sub_group_block_write_us4_32_img_rw.Types, 0, generic_write_img, [["$WIDTH", "4"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write8_32_img_generic = OclBuiltinImpl<intel_sub_group_block_write8_32_img, intel_sub_group_block_write8_32_img.Types, 0, generic_write_img, [["$WIDTH", "8"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write8_32_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write8_32_img_rw, intel_sub_group_block_write8_32_img_rw.Types, 0, generic_write_img, [["$WIDTH", "8"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write_us8_32_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us8_32_img, intel_sub_group_block_write_us8_32_img.Types, 0, generic_write_img, [["$WIDTH", "8"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write_us8_32_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us8_32_img_rw, intel_sub_group_block_write_us8_32_img_rw.Types, 0, generic_write_img, [["$WIDTH", "8"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write_us16_32_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us16_32_img, intel_sub_group_block_write_us16_32_img.Types, 0, generic_write_img, [["$WIDTH", "16"], ["$VF", "32"]]>;
OclBuiltinImpl intel_sub_group_block_write_us16_32_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us16_32_img_rw, intel_sub_group_block_write_us16_32_img_rw.Types, 0, generic_write_img, [["$WIDTH", "16"], ["$VF", "32"]]>;
// X64 write
OclBuiltinImpl intel_sub_group_block_write1_64_img_generic = OclBuiltinImpl<intel_sub_group_block_write1_64_img, intel_sub_group_block_write1_64_img.Types, 0, generic_write_img, [["$WIDTH", "1"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write1_64_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write1_64_img_rw, intel_sub_group_block_write1_64_img_rw.Types, 0, generic_write_img, [["$WIDTH", "1"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write_us1_64_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us1_64_img, intel_sub_group_block_write_us1_64_img.Types, 0, generic_write_img, [["$WIDTH", "1"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write_us1_64_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us1_64_img_rw, intel_sub_group_block_write_us1_64_img_rw.Types, 0, generic_write_img, [["$WIDTH", "1"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write2_64_img_generic = OclBuiltinImpl<intel_sub_group_block_write2_64_img, intel_sub_group_block_write2_64_img.Types, 0, generic_write_img, [["$WIDTH", "2"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write2_64_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write2_64_img_rw, intel_sub_group_block_write2_64_img_rw.Types, 0, generic_write_img, [["$WIDTH", "2"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write_us2_64_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us2_64_img, intel_sub_group_block_write_us2_64_img.Types, 0, generic_write_img, [["$WIDTH", "2"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write_us2_64_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us2_64_img_rw, intel_sub_group_block_write_us2_64_img_rw.Types, 0, generic_write_img, [["$WIDTH", "2"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write4_64_img_generic = OclBuiltinImpl<intel_sub_group_block_write4_64_img, intel_sub_group_block_write4_64_img.Types, 0, generic_write_img, [["$WIDTH", "4"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write4_64_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write4_64_img_rw, intel_sub_group_block_write4_64_img_rw.Types, 0, generic_write_img, [["$WIDTH", "4"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write_us4_64_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us4_64_img, intel_sub_group_block_write_us4_64_img.Types, 0, generic_write_img, [["$WIDTH", "4"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write_us4_64_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us4_64_img_rw, intel_sub_group_block_write_us4_64_img_rw.Types, 0, generic_write_img, [["$WIDTH", "4"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write8_64_img_generic = OclBuiltinImpl<intel_sub_group_block_write8_64_img, intel_sub_group_block_write8_64_img.Types, 0, generic_write_img, [["$WIDTH", "8"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write8_64_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write8_64_img_rw, intel_sub_group_block_write8_64_img_rw.Types, 0, generic_write_img, [["$WIDTH", "8"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write_us8_64_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us8_64_img, intel_sub_group_block_write_us8_64_img.Types, 0, generic_write_img, [["$WIDTH", "8"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write_us8_64_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us8_64_img_rw, intel_sub_group_block_write_us8_64_img_rw.Types, 0, generic_write_img, [["$WIDTH", "8"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write_us16_64_img_generic = OclBuiltinImpl<intel_sub_group_block_write_us16_64_img, intel_sub_group_block_write_us16_64_img.Types, 0, generic_write_img, [["$WIDTH", "16"], ["$VF", "64"]]>;
OclBuiltinImpl intel_sub_group_block_write_us16_64_img_rw_generic = OclBuiltinImpl<intel_sub_group_block_write_us16_64_img_rw, intel_sub_group_block_write_us16_64_img_rw.Types, 0, generic_write_img, [["$WIDTH", "16"], ["$VF", "64"]]>;
