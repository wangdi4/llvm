//
// Copyright (C) 2023 Intel Corporation
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

//-----------------------------------------------
// 1. Uniform result
//-----------------------------------------------

defvar work_group_inclusive_function_op_scalar = [{
    $Arg0Type acc = $FUNC($Arg0VarName, *$Arg1VarName);
    *$Arg1VarName = acc;
    return acc;
  }];

defvar work_group_inclusive_function_op_vector = [{
    $Arg0Type acc = $FUNC($Arg0VarName, *$Arg1VarName);
    *$Arg1VarName = acc;
    return acc;
  }];

defvar work_group_inclusive_function_op_vector32 = [{
    $Arg0Type acc;
    acc.lo = $FUNC($Arg0VarName.lo, (*$Arg1VarName).lo);
    acc.hi = $FUNC($Arg0VarName.hi, (*$Arg1VarName).hi);
    *$Arg1VarName = acc;
    return acc;
  }];

defvar work_group_inclusive_function_op_vector64 = [{
    $Arg0Type acc;
    acc.lo.lo = $FUNC($Arg0VarName.lo.lo, (*$Arg1VarName).lo.lo);
    acc.lo.hi = $FUNC($Arg0VarName.lo.hi, (*$Arg1VarName).lo.hi);
    acc.hi.lo = $FUNC($Arg0VarName.hi.lo, (*$Arg1VarName).hi.lo);
    acc.hi.hi = $FUNC($Arg0VarName.hi.hi, (*$Arg1VarName).hi.hi);
    *$Arg1VarName = acc;
    return acc;
  }];

defvar work_group_inclusive_function_op_mask = [{
    $MaskCastTy$VecLength arg0 = __builtin_astype($Arg0VarName, $MaskCastTy$VecLength);
    $MaskCastTy$VecLength arg1 = __builtin_astype($Arg1VarName, $MaskCastTy$VecLength);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);
    $Arg0Type acc = $FUNC($Arg0VarName, *$Arg2VarName);
    *$Arg2VarName = acc;
    return acc;
  }];

defvar work_group_inclusive_function_op_mask32 = [{
    $MaskCastTy$VecLength arg0 = __builtin_astype($Arg0VarName, $MaskCastTy$VecLength);
    $MaskCastTy$VecLength arg1 = __builtin_astype($Arg1VarName, $MaskCastTy$VecLength);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);
    $Arg0Type acc;
    acc.lo = $FUNC($Arg0VarName.lo, (*$Arg2VarName).lo);
    acc.hi = $FUNC($Arg0VarName.hi, (*$Arg2VarName).hi);
    *$Arg2VarName = acc;
    return acc;
  }];

defvar work_group_inclusive_function_op_mask64 = [{
    $MaskCastTy$VecLength arg0 = __builtin_astype($Arg0VarName, $MaskCastTy$VecLength);
    $MaskCastTy$VecLength arg1 = __builtin_astype($Arg1VarName, $MaskCastTy$VecLength);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);
    $Arg0Type acc;
    acc.lo.lo = $FUNC($Arg0VarName.lo.lo, (*$Arg2VarName).lo.lo);
    acc.lo.hi = $FUNC($Arg0VarName.lo.hi, (*$Arg2VarName).lo.hi);
    acc.hi.lo = $FUNC($Arg0VarName.hi.lo, (*$Arg2VarName).hi.lo);
    acc.hi.hi = $FUNC($Arg0VarName.hi.hi, (*$Arg2VarName).hi.hi);
    *$Arg2VarName = acc;
    return acc;
  }];

defvar work_group_inclusive_bitwise_op_mask = [{
    $Arg0VarName &= $Arg1VarName;
    $Arg1VarName = ~$Arg1VarName & __builtin_astype(($Arg0ugentype)($VAL), $Arg0Type);
    $Arg0VarName |= $Arg1VarName;
    $Arg0Type acc = $FUNC($Arg0VarName, *$Arg2VarName);
    *$Arg2VarName = acc;
    return acc;
  }];

defvar work_group_inclusive_bitwise_op_mask32 = [{
    $Arg0VarName &= $Arg1VarName;
    $Arg1VarName = ~$Arg1VarName & __builtin_astype(($Arg0ugentype)($VAL), $Arg0Type);
    $Arg0VarName |= $Arg1VarName;
    $Arg0Type acc;
    acc.lo = $FUNC($Arg0VarName.lo, (*$Arg2VarName).lo);
    acc.hi = $FUNC($Arg0VarName.hi, (*$Arg2VarName).hi);
    *$Arg2VarName = acc;
    return acc;
  }];

defvar work_group_inclusive_bitwise_op_mask64 = [{
    $Arg0VarName &= $Arg1VarName;
    $Arg1VarName = ~$Arg1VarName & __builtin_astype(($Arg0ugentype)($VAL), $Arg0Type);
    $Arg0VarName |= $Arg1VarName;
    $Arg0Type acc;
    acc.lo.lo = $FUNC($Arg0VarName.lo.lo, (*$Arg2VarName).lo.lo);
    acc.lo.hi = $FUNC($Arg0VarName.lo.hi, (*$Arg2VarName).lo.hi);
    acc.hi.lo = $FUNC($Arg0VarName.hi.lo, (*$Arg2VarName).hi.lo);
    acc.hi.hi = $FUNC($Arg0VarName.hi.hi, (*$Arg2VarName).hi.hi);
    *$Arg2VarName = acc;
    return acc;
  }];

//-----------------------------------------------
// 2. Non-uniform result
//-----------------------------------------------

defvar work_group_scan_exclusive_function_op_vector4 = [{
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg1VarName;
    ret.s0 = acc.s3;
    ret.s1 = acc.s0 = $FUNC($Arg0VarName.s0, ret.s0);
    ret.s2 = acc.s1 = $FUNC($Arg0VarName.s1, ret.s1);
    ret.s3 = acc.s2 = $FUNC($Arg0VarName.s2, ret.s2);
    acc.s3 = $FUNC($Arg0VarName.s3, ret.s3);
    *$Arg1VarName = acc;
    return ret;
  }];

defvar work_group_scan_exclusive_function_op_vector8 = [{
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg1VarName;
    ret.s0 = acc.s7;
    ret.s1 = acc.s0 = $FUNC($Arg0VarName.s0, ret.s0);
    ret.s2 = acc.s1 = $FUNC($Arg0VarName.s1, ret.s1);
    ret.s3 = acc.s2 = $FUNC($Arg0VarName.s2, ret.s2);
    ret.s4 = acc.s3 = $FUNC($Arg0VarName.s3, ret.s3);
    ret.s5 = acc.s4 = $FUNC($Arg0VarName.s4, ret.s4);
    ret.s6 = acc.s5 = $FUNC($Arg0VarName.s5, ret.s5);
    ret.s7 = acc.s6 = $FUNC($Arg0VarName.s6, ret.s6);
    acc.s7 = $FUNC($Arg0VarName.s7, ret.s7);
    *$Arg1VarName = acc;
    return ret;
  }];

defvar work_group_scan_exclusive_function_op_vector16 = [{
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg1VarName;
    ret.s0 = acc.sF;
    ret.s1 = acc.s0 = $FUNC($Arg0VarName.s0, ret.s0);
    ret.s2 = acc.s1 = $FUNC($Arg0VarName.s1, ret.s1);
    ret.s3 = acc.s2 = $FUNC($Arg0VarName.s2, ret.s2);
    ret.s4 = acc.s3 = $FUNC($Arg0VarName.s3, ret.s3);
    ret.s5 = acc.s4 = $FUNC($Arg0VarName.s4, ret.s4);
    ret.s6 = acc.s5 = $FUNC($Arg0VarName.s5, ret.s5);
    ret.s7 = acc.s6 = $FUNC($Arg0VarName.s6, ret.s6);
    ret.s8 = acc.s7 = $FUNC($Arg0VarName.s7, ret.s7);
    ret.s9 = acc.s8 = $FUNC($Arg0VarName.s8, ret.s8);
    ret.sA = acc.s9 = $FUNC($Arg0VarName.s9, ret.s9);
    ret.sB = acc.sA = $FUNC($Arg0VarName.sA, ret.sA);
    ret.sC = acc.sB = $FUNC($Arg0VarName.sB, ret.sB);
    ret.sD = acc.sC = $FUNC($Arg0VarName.sC, ret.sC);
    ret.sE = acc.sD = $FUNC($Arg0VarName.sD, ret.sD);
    ret.sF = acc.sE = $FUNC($Arg0VarName.sE, ret.sE);
    acc.sF = $FUNC($Arg0VarName.sF, ret.sF);
    *$Arg1VarName = acc;
    return ret;
  }];

defvar work_group_scan_exclusive_function_op_vector32 = [{
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg1VarName;
    ret.lo.s0 = acc.hi.sF;
    ret.lo.s1 = acc.lo.s0 = $FUNC($Arg0VarName.lo.s0, ret.lo.s0);
    ret.lo.s2 = acc.lo.s1 = $FUNC($Arg0VarName.lo.s1, ret.lo.s1);
    ret.lo.s3 = acc.lo.s2 = $FUNC($Arg0VarName.lo.s2, ret.lo.s2);
    ret.lo.s4 = acc.lo.s3 = $FUNC($Arg0VarName.lo.s3, ret.lo.s3);
    ret.lo.s5 = acc.lo.s4 = $FUNC($Arg0VarName.lo.s4, ret.lo.s4);
    ret.lo.s6 = acc.lo.s5 = $FUNC($Arg0VarName.lo.s5, ret.lo.s5);
    ret.lo.s7 = acc.lo.s6 = $FUNC($Arg0VarName.lo.s6, ret.lo.s6);
    ret.lo.s8 = acc.lo.s7 = $FUNC($Arg0VarName.lo.s7, ret.lo.s7);
    ret.lo.s9 = acc.lo.s8 = $FUNC($Arg0VarName.lo.s8, ret.lo.s8);
    ret.lo.sA = acc.lo.s9 = $FUNC($Arg0VarName.lo.s9, ret.lo.s9);
    ret.lo.sB = acc.lo.sA = $FUNC($Arg0VarName.lo.sA, ret.lo.sA);
    ret.lo.sC = acc.lo.sB = $FUNC($Arg0VarName.lo.sB, ret.lo.sB);
    ret.lo.sD = acc.lo.sC = $FUNC($Arg0VarName.lo.sC, ret.lo.sC);
    ret.lo.sE = acc.lo.sD = $FUNC($Arg0VarName.lo.sD, ret.lo.sD);
    ret.lo.sF = acc.lo.sE = $FUNC($Arg0VarName.lo.sE, ret.lo.sE);

    ret.hi.s0 = acc.lo.sF = $FUNC($Arg0VarName.lo.sF, ret.lo.sF);
    ret.hi.s1 = acc.hi.s0 = $FUNC($Arg0VarName.hi.s0, ret.hi.s0);
    ret.hi.s2 = acc.hi.s1 = $FUNC($Arg0VarName.hi.s1, ret.hi.s1);
    ret.hi.s3 = acc.hi.s2 = $FUNC($Arg0VarName.hi.s2, ret.hi.s2);
    ret.hi.s4 = acc.hi.s3 = $FUNC($Arg0VarName.hi.s3, ret.hi.s3);
    ret.hi.s5 = acc.hi.s4 = $FUNC($Arg0VarName.hi.s4, ret.hi.s4);
    ret.hi.s6 = acc.hi.s5 = $FUNC($Arg0VarName.hi.s5, ret.hi.s5);
    ret.hi.s7 = acc.hi.s6 = $FUNC($Arg0VarName.hi.s6, ret.hi.s6);
    ret.hi.s8 = acc.hi.s7 = $FUNC($Arg0VarName.hi.s7, ret.hi.s7);
    ret.hi.s9 = acc.hi.s8 = $FUNC($Arg0VarName.hi.s8, ret.hi.s8);
    ret.hi.sA = acc.hi.s9 = $FUNC($Arg0VarName.hi.s9, ret.hi.s9);
    ret.hi.sB = acc.hi.sA = $FUNC($Arg0VarName.hi.sA, ret.hi.sA);
    ret.hi.sC = acc.hi.sB = $FUNC($Arg0VarName.hi.sB, ret.hi.sB);
    ret.hi.sD = acc.hi.sC = $FUNC($Arg0VarName.hi.sC, ret.hi.sC);
    ret.hi.sE = acc.hi.sD = $FUNC($Arg0VarName.hi.sD, ret.hi.sD);
    ret.hi.sF = acc.hi.sE = $FUNC($Arg0VarName.hi.sE, ret.hi.sE);
    acc.hi.sF = $FUNC($Arg0VarName.hi.sF, ret.hi.sF);

    *$Arg1VarName = acc;
    return ret;
  }];

defvar work_group_scan_exclusive_function_op_vector64 = [{
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg1VarName;
    ret.lo.lo.s0 = acc.hi.hi.sF;
    ret.lo.lo.s1 = acc.lo.lo.s0 = $FUNC($Arg0VarName.lo.lo.s0, ret.lo.lo.s0);
    ret.lo.lo.s2 = acc.lo.lo.s1 = $FUNC($Arg0VarName.lo.lo.s1, ret.lo.lo.s1);
    ret.lo.lo.s3 = acc.lo.lo.s2 = $FUNC($Arg0VarName.lo.lo.s2, ret.lo.lo.s2);
    ret.lo.lo.s4 = acc.lo.lo.s3 = $FUNC($Arg0VarName.lo.lo.s3, ret.lo.lo.s3);
    ret.lo.lo.s5 = acc.lo.lo.s4 = $FUNC($Arg0VarName.lo.lo.s4, ret.lo.lo.s4);
    ret.lo.lo.s6 = acc.lo.lo.s5 = $FUNC($Arg0VarName.lo.lo.s5, ret.lo.lo.s5);
    ret.lo.lo.s7 = acc.lo.lo.s6 = $FUNC($Arg0VarName.lo.lo.s6, ret.lo.lo.s6);
    ret.lo.lo.s8 = acc.lo.lo.s7 = $FUNC($Arg0VarName.lo.lo.s7, ret.lo.lo.s7);
    ret.lo.lo.s9 = acc.lo.lo.s8 = $FUNC($Arg0VarName.lo.lo.s8, ret.lo.lo.s8);
    ret.lo.lo.sA = acc.lo.lo.s9 = $FUNC($Arg0VarName.lo.lo.s9, ret.lo.lo.s9);
    ret.lo.lo.sB = acc.lo.lo.sA = $FUNC($Arg0VarName.lo.lo.sA, ret.lo.lo.sA);
    ret.lo.lo.sC = acc.lo.lo.sB = $FUNC($Arg0VarName.lo.lo.sB, ret.lo.lo.sB);
    ret.lo.lo.sD = acc.lo.lo.sC = $FUNC($Arg0VarName.lo.lo.sC, ret.lo.lo.sC);
    ret.lo.lo.sE = acc.lo.lo.sD = $FUNC($Arg0VarName.lo.lo.sD, ret.lo.lo.sD);
    ret.lo.lo.sF = acc.lo.lo.sE = $FUNC($Arg0VarName.lo.lo.sE, ret.lo.lo.sE);

    ret.lo.hi.s0 = acc.lo.lo.sF = $FUNC($Arg0VarName.lo.lo.sF, ret.lo.lo.sF);
    ret.lo.hi.s1 = acc.lo.hi.s0 = $FUNC($Arg0VarName.lo.hi.s0, ret.lo.hi.s0);
    ret.lo.hi.s2 = acc.lo.hi.s1 = $FUNC($Arg0VarName.lo.hi.s1, ret.lo.hi.s1);
    ret.lo.hi.s3 = acc.lo.hi.s2 = $FUNC($Arg0VarName.lo.hi.s2, ret.lo.hi.s2);
    ret.lo.hi.s4 = acc.lo.hi.s3 = $FUNC($Arg0VarName.lo.hi.s3, ret.lo.hi.s3);
    ret.lo.hi.s5 = acc.lo.hi.s4 = $FUNC($Arg0VarName.lo.hi.s4, ret.lo.hi.s4);
    ret.lo.hi.s6 = acc.lo.hi.s5 = $FUNC($Arg0VarName.lo.hi.s5, ret.lo.hi.s5);
    ret.lo.hi.s7 = acc.lo.hi.s6 = $FUNC($Arg0VarName.lo.hi.s6, ret.lo.hi.s6);
    ret.lo.hi.s8 = acc.lo.hi.s7 = $FUNC($Arg0VarName.lo.hi.s7, ret.lo.hi.s7);
    ret.lo.hi.s9 = acc.lo.hi.s8 = $FUNC($Arg0VarName.lo.hi.s8, ret.lo.hi.s8);
    ret.lo.hi.sA = acc.lo.hi.s9 = $FUNC($Arg0VarName.lo.hi.s9, ret.lo.hi.s9);
    ret.lo.hi.sB = acc.lo.hi.sA = $FUNC($Arg0VarName.lo.hi.sA, ret.lo.hi.sA);
    ret.lo.hi.sC = acc.lo.hi.sB = $FUNC($Arg0VarName.lo.hi.sB, ret.lo.hi.sB);
    ret.lo.hi.sD = acc.lo.hi.sC = $FUNC($Arg0VarName.lo.hi.sC, ret.lo.hi.sC);
    ret.lo.hi.sE = acc.lo.hi.sD = $FUNC($Arg0VarName.lo.hi.sD, ret.lo.hi.sD);
    ret.lo.hi.sF = acc.lo.hi.sE = $FUNC($Arg0VarName.lo.hi.sE, ret.lo.hi.sE);

    ret.hi.lo.s0 = acc.lo.hi.sF = $FUNC($Arg0VarName.lo.hi.sF, ret.lo.hi.sF);
    ret.hi.lo.s1 = acc.hi.lo.s0 = $FUNC($Arg0VarName.hi.lo.s0, ret.hi.lo.s0);
    ret.hi.lo.s2 = acc.hi.lo.s1 = $FUNC($Arg0VarName.hi.lo.s1, ret.hi.lo.s1);
    ret.hi.lo.s3 = acc.hi.lo.s2 = $FUNC($Arg0VarName.hi.lo.s2, ret.hi.lo.s2);
    ret.hi.lo.s4 = acc.hi.lo.s3 = $FUNC($Arg0VarName.hi.lo.s3, ret.hi.lo.s3);
    ret.hi.lo.s5 = acc.hi.lo.s4 = $FUNC($Arg0VarName.hi.lo.s4, ret.hi.lo.s4);
    ret.hi.lo.s6 = acc.hi.lo.s5 = $FUNC($Arg0VarName.hi.lo.s5, ret.hi.lo.s5);
    ret.hi.lo.s7 = acc.hi.lo.s6 = $FUNC($Arg0VarName.hi.lo.s6, ret.hi.lo.s6);
    ret.hi.lo.s8 = acc.hi.lo.s7 = $FUNC($Arg0VarName.hi.lo.s7, ret.hi.lo.s7);
    ret.hi.lo.s9 = acc.hi.lo.s8 = $FUNC($Arg0VarName.hi.lo.s8, ret.hi.lo.s8);
    ret.hi.lo.sA = acc.hi.lo.s9 = $FUNC($Arg0VarName.hi.lo.s9, ret.hi.lo.s9);
    ret.hi.lo.sB = acc.hi.lo.sA = $FUNC($Arg0VarName.hi.lo.sA, ret.hi.lo.sA);
    ret.hi.lo.sC = acc.hi.lo.sB = $FUNC($Arg0VarName.hi.lo.sB, ret.hi.lo.sB);
    ret.hi.lo.sD = acc.hi.lo.sC = $FUNC($Arg0VarName.hi.lo.sC, ret.hi.lo.sC);
    ret.hi.lo.sE = acc.hi.lo.sD = $FUNC($Arg0VarName.hi.lo.sD, ret.hi.lo.sD);
    ret.hi.lo.sF = acc.hi.lo.sE = $FUNC($Arg0VarName.hi.lo.sE, ret.hi.lo.sE);

    ret.hi.hi.s0 = acc.hi.lo.sF = $FUNC($Arg0VarName.hi.lo.sF, ret.hi.lo.sF);
    ret.hi.hi.s1 = acc.hi.hi.s0 = $FUNC($Arg0VarName.hi.hi.s0, ret.hi.hi.s0);
    ret.hi.hi.s2 = acc.hi.hi.s1 = $FUNC($Arg0VarName.hi.hi.s1, ret.hi.hi.s1);
    ret.hi.hi.s3 = acc.hi.hi.s2 = $FUNC($Arg0VarName.hi.hi.s2, ret.hi.hi.s2);
    ret.hi.hi.s4 = acc.hi.hi.s3 = $FUNC($Arg0VarName.hi.hi.s3, ret.hi.hi.s3);
    ret.hi.hi.s5 = acc.hi.hi.s4 = $FUNC($Arg0VarName.hi.hi.s4, ret.hi.hi.s4);
    ret.hi.hi.s6 = acc.hi.hi.s5 = $FUNC($Arg0VarName.hi.hi.s5, ret.hi.hi.s5);
    ret.hi.hi.s7 = acc.hi.hi.s6 = $FUNC($Arg0VarName.hi.hi.s6, ret.hi.hi.s6);
    ret.hi.hi.s8 = acc.hi.hi.s7 = $FUNC($Arg0VarName.hi.hi.s7, ret.hi.hi.s7);
    ret.hi.hi.s9 = acc.hi.hi.s8 = $FUNC($Arg0VarName.hi.hi.s8, ret.hi.hi.s8);
    ret.hi.hi.sA = acc.hi.hi.s9 = $FUNC($Arg0VarName.hi.hi.s9, ret.hi.hi.s9);
    ret.hi.hi.sB = acc.hi.hi.sA = $FUNC($Arg0VarName.hi.hi.sA, ret.hi.hi.sA);
    ret.hi.hi.sC = acc.hi.hi.sB = $FUNC($Arg0VarName.hi.hi.sB, ret.hi.hi.sB);
    ret.hi.hi.sD = acc.hi.hi.sC = $FUNC($Arg0VarName.hi.hi.sC, ret.hi.hi.sC);
    ret.hi.hi.sE = acc.hi.hi.sD = $FUNC($Arg0VarName.hi.hi.sD, ret.hi.hi.sD);
    ret.hi.hi.sF = acc.hi.hi.sE = $FUNC($Arg0VarName.hi.hi.sE, ret.hi.hi.sE);
    acc.hi.hi.sF = $FUNC($Arg0VarName.hi.hi.sF, ret.hi.hi.sF);

    *$Arg1VarName = acc;
    return ret;
  }];

defvar work_group_scan_inclusive_function_op_vector4 = [{
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg1VarName;
    ret.s0 = $FUNC($Arg0VarName.s0, acc.s3);
    ret.s1 = $FUNC($Arg0VarName.s1, ret.s0);
    ret.s2 = $FUNC($Arg0VarName.s2, ret.s1);
    ret.s3 = $FUNC($Arg0VarName.s3, ret.s2);
    *$Arg1VarName = ret;
    return ret;
  }];

defvar work_group_scan_inclusive_function_op_vector8 = [{
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg1VarName;
    ret.s0 = $FUNC($Arg0VarName.s0, acc.s7);
    ret.s1 = $FUNC($Arg0VarName.s1, ret.s0);
    ret.s2 = $FUNC($Arg0VarName.s2, ret.s1);
    ret.s3 = $FUNC($Arg0VarName.s3, ret.s2);
    ret.s4 = $FUNC($Arg0VarName.s4, ret.s3);
    ret.s5 = $FUNC($Arg0VarName.s5, ret.s4);
    ret.s6 = $FUNC($Arg0VarName.s6, ret.s5);
    ret.s7 = $FUNC($Arg0VarName.s7, ret.s6);
    *$Arg1VarName = ret;
    return ret;
  }];

defvar work_group_scan_inclusive_function_op_vector16 = [{
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg1VarName;
    ret.s0 = $FUNC($Arg0VarName.s0, acc.sF);
    ret.s1 = $FUNC($Arg0VarName.s1, ret.s0);
    ret.s2 = $FUNC($Arg0VarName.s2, ret.s1);
    ret.s3 = $FUNC($Arg0VarName.s3, ret.s2);
    ret.s4 = $FUNC($Arg0VarName.s4, ret.s3);
    ret.s5 = $FUNC($Arg0VarName.s5, ret.s4);
    ret.s6 = $FUNC($Arg0VarName.s6, ret.s5);
    ret.s7 = $FUNC($Arg0VarName.s7, ret.s6);
    ret.s8 = $FUNC($Arg0VarName.s8, ret.s7);
    ret.s9 = $FUNC($Arg0VarName.s9, ret.s8);
    ret.sA = $FUNC($Arg0VarName.sA, ret.s9);
    ret.sB = $FUNC($Arg0VarName.sB, ret.sA);
    ret.sC = $FUNC($Arg0VarName.sC, ret.sB);
    ret.sD = $FUNC($Arg0VarName.sD, ret.sC);
    ret.sE = $FUNC($Arg0VarName.sE, ret.sD);
    ret.sF = $FUNC($Arg0VarName.sF, ret.sE);
    *$Arg1VarName = ret;
    return ret;
  }];

defvar work_group_scan_inclusive_function_op_vector32 = [{
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg1VarName;
    ret.lo.s0 = $FUNC($Arg0VarName.lo.s0, acc.hi.sF);
    ret.lo.s1 = $FUNC($Arg0VarName.lo.s1, ret.lo.s0);
    ret.lo.s2 = $FUNC($Arg0VarName.lo.s2, ret.lo.s1);
    ret.lo.s3 = $FUNC($Arg0VarName.lo.s3, ret.lo.s2);
    ret.lo.s4 = $FUNC($Arg0VarName.lo.s4, ret.lo.s3);
    ret.lo.s5 = $FUNC($Arg0VarName.lo.s5, ret.lo.s4);
    ret.lo.s6 = $FUNC($Arg0VarName.lo.s6, ret.lo.s5);
    ret.lo.s7 = $FUNC($Arg0VarName.lo.s7, ret.lo.s6);
    ret.lo.s8 = $FUNC($Arg0VarName.lo.s8, ret.lo.s7);
    ret.lo.s9 = $FUNC($Arg0VarName.lo.s9, ret.lo.s8);
    ret.lo.sA = $FUNC($Arg0VarName.lo.sA, ret.lo.s9);
    ret.lo.sB = $FUNC($Arg0VarName.lo.sB, ret.lo.sA);
    ret.lo.sC = $FUNC($Arg0VarName.lo.sC, ret.lo.sB);
    ret.lo.sD = $FUNC($Arg0VarName.lo.sD, ret.lo.sC);
    ret.lo.sE = $FUNC($Arg0VarName.lo.sE, ret.lo.sD);
    ret.lo.sF = $FUNC($Arg0VarName.lo.sF, ret.lo.sE);

    ret.hi.s0 = $FUNC($Arg0VarName.hi.s0, ret.lo.sF);
    ret.hi.s1 = $FUNC($Arg0VarName.hi.s1, ret.hi.s0);
    ret.hi.s2 = $FUNC($Arg0VarName.hi.s2, ret.hi.s1);
    ret.hi.s3 = $FUNC($Arg0VarName.hi.s3, ret.hi.s2);
    ret.hi.s4 = $FUNC($Arg0VarName.hi.s4, ret.hi.s3);
    ret.hi.s5 = $FUNC($Arg0VarName.hi.s5, ret.hi.s4);
    ret.hi.s6 = $FUNC($Arg0VarName.hi.s6, ret.hi.s5);
    ret.hi.s7 = $FUNC($Arg0VarName.hi.s7, ret.hi.s6);
    ret.hi.s8 = $FUNC($Arg0VarName.hi.s8, ret.hi.s7);
    ret.hi.s9 = $FUNC($Arg0VarName.hi.s9, ret.hi.s8);
    ret.hi.sA = $FUNC($Arg0VarName.hi.sA, ret.hi.s9);
    ret.hi.sB = $FUNC($Arg0VarName.hi.sB, ret.hi.sA);
    ret.hi.sC = $FUNC($Arg0VarName.hi.sC, ret.hi.sB);
    ret.hi.sD = $FUNC($Arg0VarName.hi.sD, ret.hi.sC);
    ret.hi.sE = $FUNC($Arg0VarName.hi.sE, ret.hi.sD);
    ret.hi.sF = $FUNC($Arg0VarName.hi.sF, ret.hi.sE);

    *$Arg1VarName = ret;
    return ret;
  }];

defvar work_group_scan_inclusive_function_op_vector64 = [{
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg1VarName;
    ret.lo.lo.s0 = $FUNC($Arg0VarName.lo.lo.s0, acc.hi.hi.sF);
    ret.lo.lo.s1 = $FUNC($Arg0VarName.lo.lo.s1, ret.lo.lo.s0);
    ret.lo.lo.s2 = $FUNC($Arg0VarName.lo.lo.s2, ret.lo.lo.s1);
    ret.lo.lo.s3 = $FUNC($Arg0VarName.lo.lo.s3, ret.lo.lo.s2);
    ret.lo.lo.s4 = $FUNC($Arg0VarName.lo.lo.s4, ret.lo.lo.s3);
    ret.lo.lo.s5 = $FUNC($Arg0VarName.lo.lo.s5, ret.lo.lo.s4);
    ret.lo.lo.s6 = $FUNC($Arg0VarName.lo.lo.s6, ret.lo.lo.s5);
    ret.lo.lo.s7 = $FUNC($Arg0VarName.lo.lo.s7, ret.lo.lo.s6);
    ret.lo.lo.s8 = $FUNC($Arg0VarName.lo.lo.s8, ret.lo.lo.s7);
    ret.lo.lo.s9 = $FUNC($Arg0VarName.lo.lo.s9, ret.lo.lo.s8);
    ret.lo.lo.sA = $FUNC($Arg0VarName.lo.lo.sA, ret.lo.lo.s9);
    ret.lo.lo.sB = $FUNC($Arg0VarName.lo.lo.sB, ret.lo.lo.sA);
    ret.lo.lo.sC = $FUNC($Arg0VarName.lo.lo.sC, ret.lo.lo.sB);
    ret.lo.lo.sD = $FUNC($Arg0VarName.lo.lo.sD, ret.lo.lo.sC);
    ret.lo.lo.sE = $FUNC($Arg0VarName.lo.lo.sE, ret.lo.lo.sD);
    ret.lo.lo.sF = $FUNC($Arg0VarName.lo.lo.sF, ret.lo.lo.sE);

    ret.lo.hi.s0 = $FUNC($Arg0VarName.lo.hi.s0, ret.lo.lo.sF);
    ret.lo.hi.s1 = $FUNC($Arg0VarName.lo.hi.s1, ret.lo.hi.s0);
    ret.lo.hi.s2 = $FUNC($Arg0VarName.lo.hi.s2, ret.lo.hi.s1);
    ret.lo.hi.s3 = $FUNC($Arg0VarName.lo.hi.s3, ret.lo.hi.s2);
    ret.lo.hi.s4 = $FUNC($Arg0VarName.lo.hi.s4, ret.lo.hi.s3);
    ret.lo.hi.s5 = $FUNC($Arg0VarName.lo.hi.s5, ret.lo.hi.s4);
    ret.lo.hi.s6 = $FUNC($Arg0VarName.lo.hi.s6, ret.lo.hi.s5);
    ret.lo.hi.s7 = $FUNC($Arg0VarName.lo.hi.s7, ret.lo.hi.s6);
    ret.lo.hi.s8 = $FUNC($Arg0VarName.lo.hi.s8, ret.lo.hi.s7);
    ret.lo.hi.s9 = $FUNC($Arg0VarName.lo.hi.s9, ret.lo.hi.s8);
    ret.lo.hi.sA = $FUNC($Arg0VarName.lo.hi.sA, ret.lo.hi.s9);
    ret.lo.hi.sB = $FUNC($Arg0VarName.lo.hi.sB, ret.lo.hi.sA);
    ret.lo.hi.sC = $FUNC($Arg0VarName.lo.hi.sC, ret.lo.hi.sB);
    ret.lo.hi.sD = $FUNC($Arg0VarName.lo.hi.sD, ret.lo.hi.sC);
    ret.lo.hi.sE = $FUNC($Arg0VarName.lo.hi.sE, ret.lo.hi.sD);
    ret.lo.hi.sF = $FUNC($Arg0VarName.lo.hi.sF, ret.lo.hi.sE);

    ret.hi.lo.s0 = $FUNC($Arg0VarName.hi.lo.s0, ret.lo.hi.sF);
    ret.hi.lo.s1 = $FUNC($Arg0VarName.hi.lo.s1, ret.hi.lo.s0);
    ret.hi.lo.s2 = $FUNC($Arg0VarName.hi.lo.s2, ret.hi.lo.s1);
    ret.hi.lo.s3 = $FUNC($Arg0VarName.hi.lo.s3, ret.hi.lo.s2);
    ret.hi.lo.s4 = $FUNC($Arg0VarName.hi.lo.s4, ret.hi.lo.s3);
    ret.hi.lo.s5 = $FUNC($Arg0VarName.hi.lo.s5, ret.hi.lo.s4);
    ret.hi.lo.s6 = $FUNC($Arg0VarName.hi.lo.s6, ret.hi.lo.s5);
    ret.hi.lo.s7 = $FUNC($Arg0VarName.hi.lo.s7, ret.hi.lo.s6);
    ret.hi.lo.s8 = $FUNC($Arg0VarName.hi.lo.s8, ret.hi.lo.s7);
    ret.hi.lo.s9 = $FUNC($Arg0VarName.hi.lo.s9, ret.hi.lo.s8);
    ret.hi.lo.sA = $FUNC($Arg0VarName.hi.lo.sA, ret.hi.lo.s9);
    ret.hi.lo.sB = $FUNC($Arg0VarName.hi.lo.sB, ret.hi.lo.sA);
    ret.hi.lo.sC = $FUNC($Arg0VarName.hi.lo.sC, ret.hi.lo.sB);
    ret.hi.lo.sD = $FUNC($Arg0VarName.hi.lo.sD, ret.hi.lo.sC);
    ret.hi.lo.sE = $FUNC($Arg0VarName.hi.lo.sE, ret.hi.lo.sD);
    ret.hi.lo.sF = $FUNC($Arg0VarName.hi.lo.sF, ret.hi.lo.sE);

    ret.hi.hi.s0 = $FUNC($Arg0VarName.hi.hi.s0, ret.hi.lo.sF);
    ret.hi.hi.s1 = $FUNC($Arg0VarName.hi.hi.s1, ret.hi.hi.s0);
    ret.hi.hi.s2 = $FUNC($Arg0VarName.hi.hi.s2, ret.hi.hi.s1);
    ret.hi.hi.s3 = $FUNC($Arg0VarName.hi.hi.s3, ret.hi.hi.s2);
    ret.hi.hi.s4 = $FUNC($Arg0VarName.hi.hi.s4, ret.hi.hi.s3);
    ret.hi.hi.s5 = $FUNC($Arg0VarName.hi.hi.s5, ret.hi.hi.s4);
    ret.hi.hi.s6 = $FUNC($Arg0VarName.hi.hi.s6, ret.hi.hi.s5);
    ret.hi.hi.s7 = $FUNC($Arg0VarName.hi.hi.s7, ret.hi.hi.s6);
    ret.hi.hi.s8 = $FUNC($Arg0VarName.hi.hi.s8, ret.hi.hi.s7);
    ret.hi.hi.s9 = $FUNC($Arg0VarName.hi.hi.s9, ret.hi.hi.s8);
    ret.hi.hi.sA = $FUNC($Arg0VarName.hi.hi.sA, ret.hi.hi.s9);
    ret.hi.hi.sB = $FUNC($Arg0VarName.hi.hi.sB, ret.hi.hi.sA);
    ret.hi.hi.sC = $FUNC($Arg0VarName.hi.hi.sC, ret.hi.hi.sB);
    ret.hi.hi.sD = $FUNC($Arg0VarName.hi.hi.sD, ret.hi.hi.sC);
    ret.hi.hi.sE = $FUNC($Arg0VarName.hi.hi.sE, ret.hi.hi.sD);
    ret.hi.hi.sF = $FUNC($Arg0VarName.hi.hi.sF, ret.hi.hi.sE);

    *$Arg1VarName = ret;
    return ret;
  }];


defvar work_group_scan_exclusive_function_op_mask4 = [{

    $MaskCastTy#4 arg0 = __builtin_astype($Arg0VarName, $MaskCastTy#4);
    $MaskCastTy#4 arg1 = __builtin_astype($Arg1VarName, $MaskCastTy#4);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);

    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg2VarName;
    ret.s0 = acc.s3;
    ret.s1 = acc.s0 = $FUNC($Arg0VarName.s0, ret.s0);
    ret.s2 = acc.s1 = $FUNC($Arg0VarName.s1, ret.s1);
    ret.s3 = acc.s2 = $FUNC($Arg0VarName.s2, ret.s2);
    acc.s3 = $FUNC($Arg0VarName.s3, ret.s3);
    *$Arg2VarName = acc;
    return ret;
  }];

defvar work_group_scan_exclusive_function_op_mask8 = [{
    $MaskCastTy#8 arg0 = __builtin_astype($Arg0VarName, $MaskCastTy#8);
    $MaskCastTy#8 arg1 = __builtin_astype($Arg1VarName, $MaskCastTy#8);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg2VarName;
    ret.s0 = acc.s7;
    ret.s1 = acc.s0 = $FUNC($Arg0VarName.s0, ret.s0);
    ret.s2 = acc.s1 = $FUNC($Arg0VarName.s1, ret.s1);
    ret.s3 = acc.s2 = $FUNC($Arg0VarName.s2, ret.s2);
    ret.s4 = acc.s3 = $FUNC($Arg0VarName.s3, ret.s3);
    ret.s5 = acc.s4 = $FUNC($Arg0VarName.s4, ret.s4);
    ret.s6 = acc.s5 = $FUNC($Arg0VarName.s5, ret.s5);
    ret.s7 = acc.s6 = $FUNC($Arg0VarName.s6, ret.s6);
    acc.s7 = $FUNC($Arg0VarName.s7, ret.s7);
    *$Arg2VarName = acc;
    return ret;
  }];

defvar work_group_scan_exclusive_function_op_mask16 = [{
    $MaskCastTy#16 arg0 = __builtin_astype($Arg0VarName, $MaskCastTy#16);
    $MaskCastTy#16 arg1 = __builtin_astype($Arg1VarName, $MaskCastTy#16);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg2VarName;
    ret.s0 = acc.sF;
    ret.s1 = acc.s0 = $FUNC($Arg0VarName.s0, ret.s0);
    ret.s2 = acc.s1 = $FUNC($Arg0VarName.s1, ret.s1);
    ret.s3 = acc.s2 = $FUNC($Arg0VarName.s2, ret.s2);
    ret.s4 = acc.s3 = $FUNC($Arg0VarName.s3, ret.s3);
    ret.s5 = acc.s4 = $FUNC($Arg0VarName.s4, ret.s4);
    ret.s6 = acc.s5 = $FUNC($Arg0VarName.s5, ret.s5);
    ret.s7 = acc.s6 = $FUNC($Arg0VarName.s6, ret.s6);
    ret.s8 = acc.s7 = $FUNC($Arg0VarName.s7, ret.s7);
    ret.s9 = acc.s8 = $FUNC($Arg0VarName.s8, ret.s8);
    ret.sA = acc.s9 = $FUNC($Arg0VarName.s9, ret.s9);
    ret.sB = acc.sA = $FUNC($Arg0VarName.sA, ret.sA);
    ret.sC = acc.sB = $FUNC($Arg0VarName.sB, ret.sB);
    ret.sD = acc.sC = $FUNC($Arg0VarName.sC, ret.sC);
    ret.sE = acc.sD = $FUNC($Arg0VarName.sD, ret.sD);
    ret.sF = acc.sE = $FUNC($Arg0VarName.sE, ret.sE);
    acc.sF = $FUNC($Arg0VarName.sF, ret.sF);
    *$Arg2VarName = acc;
    return ret;
  }];

defvar work_group_scan_exclusive_function_op_mask32 = [{
    $MaskCastTy#32 arg0 = __builtin_astype($Arg0VarName, $MaskCastTy#32);
    $MaskCastTy#32 arg1 = __builtin_astype($Arg1VarName, $MaskCastTy#32);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg2VarName;
    ret.lo.s0 = acc.hi.sF;
    ret.lo.s1 = acc.lo.s0 = $FUNC($Arg0VarName.lo.s0, ret.lo.s0);
    ret.lo.s2 = acc.lo.s1 = $FUNC($Arg0VarName.lo.s1, ret.lo.s1);
    ret.lo.s3 = acc.lo.s2 = $FUNC($Arg0VarName.lo.s2, ret.lo.s2);
    ret.lo.s4 = acc.lo.s3 = $FUNC($Arg0VarName.lo.s3, ret.lo.s3);
    ret.lo.s5 = acc.lo.s4 = $FUNC($Arg0VarName.lo.s4, ret.lo.s4);
    ret.lo.s6 = acc.lo.s5 = $FUNC($Arg0VarName.lo.s5, ret.lo.s5);
    ret.lo.s7 = acc.lo.s6 = $FUNC($Arg0VarName.lo.s6, ret.lo.s6);
    ret.lo.s8 = acc.lo.s7 = $FUNC($Arg0VarName.lo.s7, ret.lo.s7);
    ret.lo.s9 = acc.lo.s8 = $FUNC($Arg0VarName.lo.s8, ret.lo.s8);
    ret.lo.sA = acc.lo.s9 = $FUNC($Arg0VarName.lo.s9, ret.lo.s9);
    ret.lo.sB = acc.lo.sA = $FUNC($Arg0VarName.lo.sA, ret.lo.sA);
    ret.lo.sC = acc.lo.sB = $FUNC($Arg0VarName.lo.sB, ret.lo.sB);
    ret.lo.sD = acc.lo.sC = $FUNC($Arg0VarName.lo.sC, ret.lo.sC);
    ret.lo.sE = acc.lo.sD = $FUNC($Arg0VarName.lo.sD, ret.lo.sD);
    ret.lo.sF = acc.lo.sE = $FUNC($Arg0VarName.lo.sE, ret.lo.sE);

    ret.hi.s0 = acc.lo.sF = $FUNC($Arg0VarName.lo.sF, ret.lo.sF);
    ret.hi.s1 = acc.hi.s0 = $FUNC($Arg0VarName.hi.s0, ret.hi.s0);
    ret.hi.s2 = acc.hi.s1 = $FUNC($Arg0VarName.hi.s1, ret.hi.s1);
    ret.hi.s3 = acc.hi.s2 = $FUNC($Arg0VarName.hi.s2, ret.hi.s2);
    ret.hi.s4 = acc.hi.s3 = $FUNC($Arg0VarName.hi.s3, ret.hi.s3);
    ret.hi.s5 = acc.hi.s4 = $FUNC($Arg0VarName.hi.s4, ret.hi.s4);
    ret.hi.s6 = acc.hi.s5 = $FUNC($Arg0VarName.hi.s5, ret.hi.s5);
    ret.hi.s7 = acc.hi.s6 = $FUNC($Arg0VarName.hi.s6, ret.hi.s6);
    ret.hi.s8 = acc.hi.s7 = $FUNC($Arg0VarName.hi.s7, ret.hi.s7);
    ret.hi.s9 = acc.hi.s8 = $FUNC($Arg0VarName.hi.s8, ret.hi.s8);
    ret.hi.sA = acc.hi.s9 = $FUNC($Arg0VarName.hi.s9, ret.hi.s9);
    ret.hi.sB = acc.hi.sA = $FUNC($Arg0VarName.hi.sA, ret.hi.sA);
    ret.hi.sC = acc.hi.sB = $FUNC($Arg0VarName.hi.sB, ret.hi.sB);
    ret.hi.sD = acc.hi.sC = $FUNC($Arg0VarName.hi.sC, ret.hi.sC);
    ret.hi.sE = acc.hi.sD = $FUNC($Arg0VarName.hi.sD, ret.hi.sD);
    ret.hi.sF = acc.hi.sE = $FUNC($Arg0VarName.hi.sE, ret.hi.sE);

    acc.hi.sF = $FUNC($Arg0VarName.hi.sF, ret.hi.sF);
    *$Arg2VarName = acc;
    return ret;
  }];

defvar work_group_scan_exclusive_function_op_mask64 = [{
    $MaskCastTy#64 arg0 = __builtin_astype($Arg0VarName, $MaskCastTy#64);
    $MaskCastTy#64 arg1 = __builtin_astype($Arg1VarName, $MaskCastTy#64);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg2VarName;
    ret.lo.lo.s0 = acc.hi.hi.sF;
    ret.lo.lo.s1 = acc.lo.lo.s0 = $FUNC($Arg0VarName.lo.lo.s0, ret.lo.lo.s0);
    ret.lo.lo.s2 = acc.lo.lo.s1 = $FUNC($Arg0VarName.lo.lo.s1, ret.lo.lo.s1);
    ret.lo.lo.s3 = acc.lo.lo.s2 = $FUNC($Arg0VarName.lo.lo.s2, ret.lo.lo.s2);
    ret.lo.lo.s4 = acc.lo.lo.s3 = $FUNC($Arg0VarName.lo.lo.s3, ret.lo.lo.s3);
    ret.lo.lo.s5 = acc.lo.lo.s4 = $FUNC($Arg0VarName.lo.lo.s4, ret.lo.lo.s4);
    ret.lo.lo.s6 = acc.lo.lo.s5 = $FUNC($Arg0VarName.lo.lo.s5, ret.lo.lo.s5);
    ret.lo.lo.s7 = acc.lo.lo.s6 = $FUNC($Arg0VarName.lo.lo.s6, ret.lo.lo.s6);
    ret.lo.lo.s8 = acc.lo.lo.s7 = $FUNC($Arg0VarName.lo.lo.s7, ret.lo.lo.s7);
    ret.lo.lo.s9 = acc.lo.lo.s8 = $FUNC($Arg0VarName.lo.lo.s8, ret.lo.lo.s8);
    ret.lo.lo.sA = acc.lo.lo.s9 = $FUNC($Arg0VarName.lo.lo.s9, ret.lo.lo.s9);
    ret.lo.lo.sB = acc.lo.lo.sA = $FUNC($Arg0VarName.lo.lo.sA, ret.lo.lo.sA);
    ret.lo.lo.sC = acc.lo.lo.sB = $FUNC($Arg0VarName.lo.lo.sB, ret.lo.lo.sB);
    ret.lo.lo.sD = acc.lo.lo.sC = $FUNC($Arg0VarName.lo.lo.sC, ret.lo.lo.sC);
    ret.lo.lo.sE = acc.lo.lo.sD = $FUNC($Arg0VarName.lo.lo.sD, ret.lo.lo.sD);
    ret.lo.lo.sF = acc.lo.lo.sE = $FUNC($Arg0VarName.lo.lo.sE, ret.lo.lo.sE);

    ret.lo.hi.s0 = acc.lo.lo.sF = $FUNC($Arg0VarName.lo.lo.sF, ret.lo.lo.sF);
    ret.lo.hi.s1 = acc.lo.hi.s0 = $FUNC($Arg0VarName.lo.hi.s0, ret.lo.hi.s0);
    ret.lo.hi.s2 = acc.lo.hi.s1 = $FUNC($Arg0VarName.lo.hi.s1, ret.lo.hi.s1);
    ret.lo.hi.s3 = acc.lo.hi.s2 = $FUNC($Arg0VarName.lo.hi.s2, ret.lo.hi.s2);
    ret.lo.hi.s4 = acc.lo.hi.s3 = $FUNC($Arg0VarName.lo.hi.s3, ret.lo.hi.s3);
    ret.lo.hi.s5 = acc.lo.hi.s4 = $FUNC($Arg0VarName.lo.hi.s4, ret.lo.hi.s4);
    ret.lo.hi.s6 = acc.lo.hi.s5 = $FUNC($Arg0VarName.lo.hi.s5, ret.lo.hi.s5);
    ret.lo.hi.s7 = acc.lo.hi.s6 = $FUNC($Arg0VarName.lo.hi.s6, ret.lo.hi.s6);
    ret.lo.hi.s8 = acc.lo.hi.s7 = $FUNC($Arg0VarName.lo.hi.s7, ret.lo.hi.s7);
    ret.lo.hi.s9 = acc.lo.hi.s8 = $FUNC($Arg0VarName.lo.hi.s8, ret.lo.hi.s8);
    ret.lo.hi.sA = acc.lo.hi.s9 = $FUNC($Arg0VarName.lo.hi.s9, ret.lo.hi.s9);
    ret.lo.hi.sB = acc.lo.hi.sA = $FUNC($Arg0VarName.lo.hi.sA, ret.lo.hi.sA);
    ret.lo.hi.sC = acc.lo.hi.sB = $FUNC($Arg0VarName.lo.hi.sB, ret.lo.hi.sB);
    ret.lo.hi.sD = acc.lo.hi.sC = $FUNC($Arg0VarName.lo.hi.sC, ret.lo.hi.sC);
    ret.lo.hi.sE = acc.lo.hi.sD = $FUNC($Arg0VarName.lo.hi.sD, ret.lo.hi.sD);
    ret.lo.hi.sF = acc.lo.hi.sE = $FUNC($Arg0VarName.lo.hi.sE, ret.lo.hi.sE);

    ret.hi.lo.s0 = acc.lo.hi.sF = $FUNC($Arg0VarName.lo.hi.sF, ret.lo.hi.sF);
    ret.hi.lo.s1 = acc.hi.lo.s0 = $FUNC($Arg0VarName.hi.lo.s0, ret.hi.lo.s0);
    ret.hi.lo.s2 = acc.hi.lo.s1 = $FUNC($Arg0VarName.hi.lo.s1, ret.hi.lo.s1);
    ret.hi.lo.s3 = acc.hi.lo.s2 = $FUNC($Arg0VarName.hi.lo.s2, ret.hi.lo.s2);
    ret.hi.lo.s4 = acc.hi.lo.s3 = $FUNC($Arg0VarName.hi.lo.s3, ret.hi.lo.s3);
    ret.hi.lo.s5 = acc.hi.lo.s4 = $FUNC($Arg0VarName.hi.lo.s4, ret.hi.lo.s4);
    ret.hi.lo.s6 = acc.hi.lo.s5 = $FUNC($Arg0VarName.hi.lo.s5, ret.hi.lo.s5);
    ret.hi.lo.s7 = acc.hi.lo.s6 = $FUNC($Arg0VarName.hi.lo.s6, ret.hi.lo.s6);
    ret.hi.lo.s8 = acc.hi.lo.s7 = $FUNC($Arg0VarName.hi.lo.s7, ret.hi.lo.s7);
    ret.hi.lo.s9 = acc.hi.lo.s8 = $FUNC($Arg0VarName.hi.lo.s8, ret.hi.lo.s8);
    ret.hi.lo.sA = acc.hi.lo.s9 = $FUNC($Arg0VarName.hi.lo.s9, ret.hi.lo.s9);
    ret.hi.lo.sB = acc.hi.lo.sA = $FUNC($Arg0VarName.hi.lo.sA, ret.hi.lo.sA);
    ret.hi.lo.sC = acc.hi.lo.sB = $FUNC($Arg0VarName.hi.lo.sB, ret.hi.lo.sB);
    ret.hi.lo.sD = acc.hi.lo.sC = $FUNC($Arg0VarName.hi.lo.sC, ret.hi.lo.sC);
    ret.hi.lo.sE = acc.hi.lo.sD = $FUNC($Arg0VarName.hi.lo.sD, ret.hi.lo.sD);
    ret.hi.lo.sF = acc.hi.lo.sE = $FUNC($Arg0VarName.hi.lo.sE, ret.hi.lo.sE);

    ret.hi.hi.s0 = acc.hi.lo.sF = $FUNC($Arg0VarName.hi.lo.sF, ret.hi.lo.sF);
    ret.hi.hi.s1 = acc.hi.hi.s0 = $FUNC($Arg0VarName.hi.hi.s0, ret.hi.hi.s0);
    ret.hi.hi.s2 = acc.hi.hi.s1 = $FUNC($Arg0VarName.hi.hi.s1, ret.hi.hi.s1);
    ret.hi.hi.s3 = acc.hi.hi.s2 = $FUNC($Arg0VarName.hi.hi.s2, ret.hi.hi.s2);
    ret.hi.hi.s4 = acc.hi.hi.s3 = $FUNC($Arg0VarName.hi.hi.s3, ret.hi.hi.s3);
    ret.hi.hi.s5 = acc.hi.hi.s4 = $FUNC($Arg0VarName.hi.hi.s4, ret.hi.hi.s4);
    ret.hi.hi.s6 = acc.hi.hi.s5 = $FUNC($Arg0VarName.hi.hi.s5, ret.hi.hi.s5);
    ret.hi.hi.s7 = acc.hi.hi.s6 = $FUNC($Arg0VarName.hi.hi.s6, ret.hi.hi.s6);
    ret.hi.hi.s8 = acc.hi.hi.s7 = $FUNC($Arg0VarName.hi.hi.s7, ret.hi.hi.s7);
    ret.hi.hi.s9 = acc.hi.hi.s8 = $FUNC($Arg0VarName.hi.hi.s8, ret.hi.hi.s8);
    ret.hi.hi.sA = acc.hi.hi.s9 = $FUNC($Arg0VarName.hi.hi.s9, ret.hi.hi.s9);
    ret.hi.hi.sB = acc.hi.hi.sA = $FUNC($Arg0VarName.hi.hi.sA, ret.hi.hi.sA);
    ret.hi.hi.sC = acc.hi.hi.sB = $FUNC($Arg0VarName.hi.hi.sB, ret.hi.hi.sB);
    ret.hi.hi.sD = acc.hi.hi.sC = $FUNC($Arg0VarName.hi.hi.sC, ret.hi.hi.sC);
    ret.hi.hi.sE = acc.hi.hi.sD = $FUNC($Arg0VarName.hi.hi.sD, ret.hi.hi.sD);
    ret.hi.hi.sF = acc.hi.hi.sE = $FUNC($Arg0VarName.hi.hi.sE, ret.hi.hi.sE);

    acc.hi.hi.sF = $FUNC($Arg0VarName.hi.hi.sF, ret.hi.hi.sF);
    *$Arg2VarName = acc;
    return ret;
  }];

defvar work_group_scan_inclusive_function_op_mask4 = [{
    $MaskCastTy#4 arg0 = __builtin_astype($Arg0VarName, $MaskCastTy#4);
    $MaskCastTy#4 arg1 = __builtin_astype($Arg1VarName, $MaskCastTy#4);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg2VarName;
    ret.s0 = $FUNC($Arg0VarName.s0, acc.s3);
    ret.s1 = $FUNC($Arg0VarName.s1, ret.s0);
    ret.s2 = $FUNC($Arg0VarName.s2, ret.s1);
    ret.s3 = $FUNC($Arg0VarName.s3, ret.s2);
    *$Arg2VarName = ret;
    return ret;
  }];

defvar work_group_scan_inclusive_function_op_mask8 = [{
    $MaskCastTy#8 arg0 = __builtin_astype($Arg0VarName, $MaskCastTy#8);
    $MaskCastTy#8 arg1 = __builtin_astype($Arg1VarName, $MaskCastTy#8);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg2VarName;
    ret.s0 = $FUNC($Arg0VarName.s0, acc.s7);
    ret.s1 = $FUNC($Arg0VarName.s1, ret.s0);
    ret.s2 = $FUNC($Arg0VarName.s2, ret.s1);
    ret.s3 = $FUNC($Arg0VarName.s3, ret.s2);
    ret.s4 = $FUNC($Arg0VarName.s4, ret.s3);
    ret.s5 = $FUNC($Arg0VarName.s5, ret.s4);
    ret.s6 = $FUNC($Arg0VarName.s6, ret.s5);
    ret.s7 = $FUNC($Arg0VarName.s7, ret.s6);
    *$Arg2VarName = ret;
    return ret;
  }];

defvar work_group_scan_inclusive_function_op_mask16 = [{
    $MaskCastTy#16 arg0 = __builtin_astype($Arg0VarName, $MaskCastTy#16);
    $MaskCastTy#16 arg1 = __builtin_astype($Arg1VarName, $MaskCastTy#16);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg2VarName;
    ret.s0 = $FUNC($Arg0VarName.s0, acc.sF);
    ret.s1 = $FUNC($Arg0VarName.s1, ret.s0);
    ret.s2 = $FUNC($Arg0VarName.s2, ret.s1);
    ret.s3 = $FUNC($Arg0VarName.s3, ret.s2);
    ret.s4 = $FUNC($Arg0VarName.s4, ret.s3);
    ret.s5 = $FUNC($Arg0VarName.s5, ret.s4);
    ret.s6 = $FUNC($Arg0VarName.s6, ret.s5);
    ret.s7 = $FUNC($Arg0VarName.s7, ret.s6);
    ret.s8 = $FUNC($Arg0VarName.s8, ret.s7);
    ret.s9 = $FUNC($Arg0VarName.s9, ret.s8);
    ret.sA = $FUNC($Arg0VarName.sA, ret.s9);
    ret.sB = $FUNC($Arg0VarName.sB, ret.sA);
    ret.sC = $FUNC($Arg0VarName.sC, ret.sB);
    ret.sD = $FUNC($Arg0VarName.sD, ret.sC);
    ret.sE = $FUNC($Arg0VarName.sE, ret.sD);
    ret.sF = $FUNC($Arg0VarName.sF, ret.sE);
    *$Arg2VarName = ret;
    return ret;
  }];

defvar work_group_scan_inclusive_function_op_mask32 = [{
    $MaskCastTy#32 arg0 = __builtin_astype($Arg0VarName, $MaskCastTy#32);
    $MaskCastTy#32 arg1 = __builtin_astype($Arg1VarName, $MaskCastTy#32);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg2VarName;
    ret.lo.s0 = $FUNC($Arg0VarName.lo.s0, acc.hi.sF);
    ret.lo.s1 = $FUNC($Arg0VarName.lo.s1, ret.lo.s0);
    ret.lo.s2 = $FUNC($Arg0VarName.lo.s2, ret.lo.s1);
    ret.lo.s3 = $FUNC($Arg0VarName.lo.s3, ret.lo.s2);
    ret.lo.s4 = $FUNC($Arg0VarName.lo.s4, ret.lo.s3);
    ret.lo.s5 = $FUNC($Arg0VarName.lo.s5, ret.lo.s4);
    ret.lo.s6 = $FUNC($Arg0VarName.lo.s6, ret.lo.s5);
    ret.lo.s7 = $FUNC($Arg0VarName.lo.s7, ret.lo.s6);
    ret.lo.s8 = $FUNC($Arg0VarName.lo.s8, ret.lo.s7);
    ret.lo.s9 = $FUNC($Arg0VarName.lo.s9, ret.lo.s8);
    ret.lo.sA = $FUNC($Arg0VarName.lo.sA, ret.lo.s9);
    ret.lo.sB = $FUNC($Arg0VarName.lo.sB, ret.lo.sA);
    ret.lo.sC = $FUNC($Arg0VarName.lo.sC, ret.lo.sB);
    ret.lo.sD = $FUNC($Arg0VarName.lo.sD, ret.lo.sC);
    ret.lo.sE = $FUNC($Arg0VarName.lo.sE, ret.lo.sD);
    ret.lo.sF = $FUNC($Arg0VarName.lo.sF, ret.lo.sE);

    ret.hi.s0 = $FUNC($Arg0VarName.hi.s0, ret.lo.sF);
    ret.hi.s1 = $FUNC($Arg0VarName.hi.s1, ret.hi.s0);
    ret.hi.s2 = $FUNC($Arg0VarName.hi.s2, ret.hi.s1);
    ret.hi.s3 = $FUNC($Arg0VarName.hi.s3, ret.hi.s2);
    ret.hi.s4 = $FUNC($Arg0VarName.hi.s4, ret.hi.s3);
    ret.hi.s5 = $FUNC($Arg0VarName.hi.s5, ret.hi.s4);
    ret.hi.s6 = $FUNC($Arg0VarName.hi.s6, ret.hi.s5);
    ret.hi.s7 = $FUNC($Arg0VarName.hi.s7, ret.hi.s6);
    ret.hi.s8 = $FUNC($Arg0VarName.hi.s8, ret.hi.s7);
    ret.hi.s9 = $FUNC($Arg0VarName.hi.s9, ret.hi.s8);
    ret.hi.sA = $FUNC($Arg0VarName.hi.sA, ret.hi.s9);
    ret.hi.sB = $FUNC($Arg0VarName.hi.sB, ret.hi.sA);
    ret.hi.sC = $FUNC($Arg0VarName.hi.sC, ret.hi.sB);
    ret.hi.sD = $FUNC($Arg0VarName.hi.sD, ret.hi.sC);
    ret.hi.sE = $FUNC($Arg0VarName.hi.sE, ret.hi.sD);
    ret.hi.sF = $FUNC($Arg0VarName.hi.sF, ret.hi.sE);

    *$Arg2VarName = ret;
    return ret;
  }];

defvar work_group_scan_inclusive_function_op_mask64 = [{
    $MaskCastTy#64 arg0 = __builtin_astype($Arg0VarName, $MaskCastTy#64);
    $MaskCastTy#64 arg1 = __builtin_astype($Arg1VarName, $MaskCastTy#64);
    arg0 &= arg1;
    arg1 = ~arg1 & __builtin_astype(($Arg0BaseType)($VAL), $MaskCastTy);
    arg0 |= arg1;
    $Arg0VarName = __builtin_astype(arg0, $Arg0Type);
    $Arg0VecType ret;
    $Arg0VecType acc = *$Arg2VarName;
    ret.lo.lo.s0 = $FUNC($Arg0VarName.lo.lo.s0, acc.hi.hi.sF);
    ret.lo.lo.s1 = $FUNC($Arg0VarName.lo.lo.s1, ret.lo.lo.s0);
    ret.lo.lo.s2 = $FUNC($Arg0VarName.lo.lo.s2, ret.lo.lo.s1);
    ret.lo.lo.s3 = $FUNC($Arg0VarName.lo.lo.s3, ret.lo.lo.s2);
    ret.lo.lo.s4 = $FUNC($Arg0VarName.lo.lo.s4, ret.lo.lo.s3);
    ret.lo.lo.s5 = $FUNC($Arg0VarName.lo.lo.s5, ret.lo.lo.s4);
    ret.lo.lo.s6 = $FUNC($Arg0VarName.lo.lo.s6, ret.lo.lo.s5);
    ret.lo.lo.s7 = $FUNC($Arg0VarName.lo.lo.s7, ret.lo.lo.s6);
    ret.lo.lo.s8 = $FUNC($Arg0VarName.lo.lo.s8, ret.lo.lo.s7);
    ret.lo.lo.s9 = $FUNC($Arg0VarName.lo.lo.s9, ret.lo.lo.s8);
    ret.lo.lo.sA = $FUNC($Arg0VarName.lo.lo.sA, ret.lo.lo.s9);
    ret.lo.lo.sB = $FUNC($Arg0VarName.lo.lo.sB, ret.lo.lo.sA);
    ret.lo.lo.sC = $FUNC($Arg0VarName.lo.lo.sC, ret.lo.lo.sB);
    ret.lo.lo.sD = $FUNC($Arg0VarName.lo.lo.sD, ret.lo.lo.sC);
    ret.lo.lo.sE = $FUNC($Arg0VarName.lo.lo.sE, ret.lo.lo.sD);
    ret.lo.lo.sF = $FUNC($Arg0VarName.lo.lo.sF, ret.lo.lo.sE);

    ret.lo.hi.s0 = $FUNC($Arg0VarName.lo.hi.s0, ret.lo.lo.sF);
    ret.lo.hi.s1 = $FUNC($Arg0VarName.lo.hi.s1, ret.lo.hi.s0);
    ret.lo.hi.s2 = $FUNC($Arg0VarName.lo.hi.s2, ret.lo.hi.s1);
    ret.lo.hi.s3 = $FUNC($Arg0VarName.lo.hi.s3, ret.lo.hi.s2);
    ret.lo.hi.s4 = $FUNC($Arg0VarName.lo.hi.s4, ret.lo.hi.s3);
    ret.lo.hi.s5 = $FUNC($Arg0VarName.lo.hi.s5, ret.lo.hi.s4);
    ret.lo.hi.s6 = $FUNC($Arg0VarName.lo.hi.s6, ret.lo.hi.s5);
    ret.lo.hi.s7 = $FUNC($Arg0VarName.lo.hi.s7, ret.lo.hi.s6);
    ret.lo.hi.s8 = $FUNC($Arg0VarName.lo.hi.s8, ret.lo.hi.s7);
    ret.lo.hi.s9 = $FUNC($Arg0VarName.lo.hi.s9, ret.lo.hi.s8);
    ret.lo.hi.sA = $FUNC($Arg0VarName.lo.hi.sA, ret.lo.hi.s9);
    ret.lo.hi.sB = $FUNC($Arg0VarName.lo.hi.sB, ret.lo.hi.sA);
    ret.lo.hi.sC = $FUNC($Arg0VarName.lo.hi.sC, ret.lo.hi.sB);
    ret.lo.hi.sD = $FUNC($Arg0VarName.lo.hi.sD, ret.lo.hi.sC);
    ret.lo.hi.sE = $FUNC($Arg0VarName.lo.hi.sE, ret.lo.hi.sD);
    ret.lo.hi.sF = $FUNC($Arg0VarName.lo.hi.sF, ret.lo.hi.sE);

    ret.hi.lo.s0 = $FUNC($Arg0VarName.hi.lo.s0, ret.lo.hi.sF);
    ret.hi.lo.s1 = $FUNC($Arg0VarName.hi.lo.s1, ret.hi.lo.s0);
    ret.hi.lo.s2 = $FUNC($Arg0VarName.hi.lo.s2, ret.hi.lo.s1);
    ret.hi.lo.s3 = $FUNC($Arg0VarName.hi.lo.s3, ret.hi.lo.s2);
    ret.hi.lo.s4 = $FUNC($Arg0VarName.hi.lo.s4, ret.hi.lo.s3);
    ret.hi.lo.s5 = $FUNC($Arg0VarName.hi.lo.s5, ret.hi.lo.s4);
    ret.hi.lo.s6 = $FUNC($Arg0VarName.hi.lo.s6, ret.hi.lo.s5);
    ret.hi.lo.s7 = $FUNC($Arg0VarName.hi.lo.s7, ret.hi.lo.s6);
    ret.hi.lo.s8 = $FUNC($Arg0VarName.hi.lo.s8, ret.hi.lo.s7);
    ret.hi.lo.s9 = $FUNC($Arg0VarName.hi.lo.s9, ret.hi.lo.s8);
    ret.hi.lo.sA = $FUNC($Arg0VarName.hi.lo.sA, ret.hi.lo.s9);
    ret.hi.lo.sB = $FUNC($Arg0VarName.hi.lo.sB, ret.hi.lo.sA);
    ret.hi.lo.sC = $FUNC($Arg0VarName.hi.lo.sC, ret.hi.lo.sB);
    ret.hi.lo.sD = $FUNC($Arg0VarName.hi.lo.sD, ret.hi.lo.sC);
    ret.hi.lo.sE = $FUNC($Arg0VarName.hi.lo.sE, ret.hi.lo.sD);
    ret.hi.lo.sF = $FUNC($Arg0VarName.hi.lo.sF, ret.hi.lo.sE);

    ret.hi.hi.s0 = $FUNC($Arg0VarName.hi.hi.s0, ret.hi.lo.sF);
    ret.hi.hi.s1 = $FUNC($Arg0VarName.hi.hi.s1, ret.hi.hi.s0);
    ret.hi.hi.s2 = $FUNC($Arg0VarName.hi.hi.s2, ret.hi.hi.s1);
    ret.hi.hi.s3 = $FUNC($Arg0VarName.hi.hi.s3, ret.hi.hi.s2);
    ret.hi.hi.s4 = $FUNC($Arg0VarName.hi.hi.s4, ret.hi.hi.s3);
    ret.hi.hi.s5 = $FUNC($Arg0VarName.hi.hi.s5, ret.hi.hi.s4);
    ret.hi.hi.s6 = $FUNC($Arg0VarName.hi.hi.s6, ret.hi.hi.s5);
    ret.hi.hi.s7 = $FUNC($Arg0VarName.hi.hi.s7, ret.hi.hi.s6);
    ret.hi.hi.s8 = $FUNC($Arg0VarName.hi.hi.s8, ret.hi.hi.s7);
    ret.hi.hi.s9 = $FUNC($Arg0VarName.hi.hi.s9, ret.hi.hi.s8);
    ret.hi.hi.sA = $FUNC($Arg0VarName.hi.hi.sA, ret.hi.hi.s9);
    ret.hi.hi.sB = $FUNC($Arg0VarName.hi.hi.sB, ret.hi.hi.sA);
    ret.hi.hi.sC = $FUNC($Arg0VarName.hi.hi.sC, ret.hi.hi.sB);
    ret.hi.hi.sD = $FUNC($Arg0VarName.hi.hi.sD, ret.hi.hi.sC);
    ret.hi.hi.sE = $FUNC($Arg0VarName.hi.hi.sE, ret.hi.hi.sD);
    ret.hi.hi.sF = $FUNC($Arg0VarName.hi.hi.sF, ret.hi.hi.sE);

    *$Arg2VarName = ret;
    return ret;
  }];

//-----------------------------------------------
// 3. Finalization of uniform result
//-----------------------------------------------

defvar work_group_finalize_function_op4 = [{
    $Arg0VecType ret;
    ret.s0 = $Arg0VarName.s0;
    ret.s0 = $FUNC($Arg0VarName.s1, ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s2, ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s3, ret.s0);
    return ($Arg0VecType)(ret.s0);
  }];

defvar work_group_finalize_function_op8 = [{
    $Arg0VecType ret;
    ret.s0 = $Arg0VarName.s0;
    ret.s0 = $FUNC($Arg0VarName.s1, ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s2, ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s3, ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s4, ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s5, ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s6, ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s7, ret.s0);
    return ($Arg0VecType)(ret.s0);
  }];

defvar work_group_finalize_function_op16 = [{
    $Arg0VecType ret;
    ret.s0 = $Arg0VarName.s0;
    ret.s0 = $FUNC($Arg0VarName.s1,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s2,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s3,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s4,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s5,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s6,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s7,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s8,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.s9,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.sA,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.sB,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.sC,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.sD,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.sE,  ret.s0);
    ret.s0 = $FUNC($Arg0VarName.sF,  ret.s0);
    return ($Arg0VecType)(ret.s0);
  }];

defvar work_group_finalize_function_op32 = [{
    $Arg0VecType ret;
    $Arg0BaseType#16 hi = $Arg0VarName.hi;
    $Arg0BaseType#16 lo = $Arg0VarName.lo;

    ret.s0 = lo.s0;
    ret.s0 = $FUNC(lo.s1,  ret.s0);
    ret.s0 = $FUNC(lo.s2,  ret.s0);
    ret.s0 = $FUNC(lo.s3,  ret.s0);
    ret.s0 = $FUNC(lo.s4,  ret.s0);
    ret.s0 = $FUNC(lo.s5,  ret.s0);
    ret.s0 = $FUNC(lo.s6,  ret.s0);
    ret.s0 = $FUNC(lo.s7,  ret.s0);
    ret.s0 = $FUNC(lo.s8,  ret.s0);
    ret.s0 = $FUNC(lo.s9,  ret.s0);
    ret.s0 = $FUNC(lo.sA,  ret.s0);
    ret.s0 = $FUNC(lo.sB,  ret.s0);
    ret.s0 = $FUNC(lo.sC,  ret.s0);
    ret.s0 = $FUNC(lo.sD,  ret.s0);
    ret.s0 = $FUNC(lo.sE,  ret.s0);
    ret.s0 = $FUNC(lo.sF,  ret.s0);

    ret.s0 = $FUNC(hi.s0,  ret.s0);
    ret.s0 = $FUNC(hi.s1,  ret.s0);
    ret.s0 = $FUNC(hi.s2,  ret.s0);
    ret.s0 = $FUNC(hi.s3,  ret.s0);
    ret.s0 = $FUNC(hi.s4,  ret.s0);
    ret.s0 = $FUNC(hi.s5,  ret.s0);
    ret.s0 = $FUNC(hi.s6,  ret.s0);
    ret.s0 = $FUNC(hi.s7,  ret.s0);
    ret.s0 = $FUNC(hi.s8,  ret.s0);
    ret.s0 = $FUNC(hi.s9,  ret.s0);
    ret.s0 = $FUNC(hi.sA,  ret.s0);
    ret.s0 = $FUNC(hi.sB,  ret.s0);
    ret.s0 = $FUNC(hi.sC,  ret.s0);
    ret.s0 = $FUNC(hi.sD,  ret.s0);
    ret.s0 = $FUNC(hi.sE,  ret.s0);
    ret.s0 = $FUNC(hi.sF,  ret.s0);

    return ($Arg0VecType)(ret.s0);
  }];

defvar work_group_finalize_function_op64 = [{
    $Arg0VecType ret;
    $Arg0BaseType#16 hihi = $Arg0VarName.hi.hi;
    $Arg0BaseType#16 hilo = $Arg0VarName.hi.lo;
    $Arg0BaseType#16 lohi = $Arg0VarName.lo.hi;
    $Arg0BaseType#16 lolo = $Arg0VarName.lo.lo;

    ret.s0 = lolo.s0;
    ret.s0 = $FUNC(lolo.s1,  ret.s0);
    ret.s0 = $FUNC(lolo.s2,  ret.s0);
    ret.s0 = $FUNC(lolo.s3,  ret.s0);
    ret.s0 = $FUNC(lolo.s4,  ret.s0);
    ret.s0 = $FUNC(lolo.s5,  ret.s0);
    ret.s0 = $FUNC(lolo.s6,  ret.s0);
    ret.s0 = $FUNC(lolo.s7,  ret.s0);
    ret.s0 = $FUNC(lolo.s8,  ret.s0);
    ret.s0 = $FUNC(lolo.s9,  ret.s0);
    ret.s0 = $FUNC(lolo.sA,  ret.s0);
    ret.s0 = $FUNC(lolo.sB,  ret.s0);
    ret.s0 = $FUNC(lolo.sC,  ret.s0);
    ret.s0 = $FUNC(lolo.sD,  ret.s0);
    ret.s0 = $FUNC(lolo.sE,  ret.s0);
    ret.s0 = $FUNC(lolo.sF,  ret.s0);

    ret.s0 = $FUNC(lohi.s0,  ret.s0);
    ret.s0 = $FUNC(lohi.s1,  ret.s0);
    ret.s0 = $FUNC(lohi.s2,  ret.s0);
    ret.s0 = $FUNC(lohi.s3,  ret.s0);
    ret.s0 = $FUNC(lohi.s4,  ret.s0);
    ret.s0 = $FUNC(lohi.s5,  ret.s0);
    ret.s0 = $FUNC(lohi.s6,  ret.s0);
    ret.s0 = $FUNC(lohi.s7,  ret.s0);
    ret.s0 = $FUNC(lohi.s8,  ret.s0);
    ret.s0 = $FUNC(lohi.s9,  ret.s0);
    ret.s0 = $FUNC(lohi.sA,  ret.s0);
    ret.s0 = $FUNC(lohi.sB,  ret.s0);
    ret.s0 = $FUNC(lohi.sC,  ret.s0);
    ret.s0 = $FUNC(lohi.sD,  ret.s0);
    ret.s0 = $FUNC(lohi.sE,  ret.s0);
    ret.s0 = $FUNC(lohi.sF,  ret.s0);

    ret.s0 = $FUNC(hilo.s0,  ret.s0);
    ret.s0 = $FUNC(hilo.s1,  ret.s0);
    ret.s0 = $FUNC(hilo.s2,  ret.s0);
    ret.s0 = $FUNC(hilo.s3,  ret.s0);
    ret.s0 = $FUNC(hilo.s4,  ret.s0);
    ret.s0 = $FUNC(hilo.s5,  ret.s0);
    ret.s0 = $FUNC(hilo.s6,  ret.s0);
    ret.s0 = $FUNC(hilo.s7,  ret.s0);
    ret.s0 = $FUNC(hilo.s8,  ret.s0);
    ret.s0 = $FUNC(hilo.s9,  ret.s0);
    ret.s0 = $FUNC(hilo.sA,  ret.s0);
    ret.s0 = $FUNC(hilo.sB,  ret.s0);
    ret.s0 = $FUNC(hilo.sC,  ret.s0);
    ret.s0 = $FUNC(hilo.sD,  ret.s0);
    ret.s0 = $FUNC(hilo.sE,  ret.s0);
    ret.s0 = $FUNC(hilo.sF,  ret.s0);

    ret.s0 = $FUNC(hihi.s0,  ret.s0);
    ret.s0 = $FUNC(hihi.s1,  ret.s0);
    ret.s0 = $FUNC(hihi.s2,  ret.s0);
    ret.s0 = $FUNC(hihi.s3,  ret.s0);
    ret.s0 = $FUNC(hihi.s4,  ret.s0);
    ret.s0 = $FUNC(hihi.s5,  ret.s0);
    ret.s0 = $FUNC(hihi.s6,  ret.s0);
    ret.s0 = $FUNC(hihi.s7,  ret.s0);
    ret.s0 = $FUNC(hihi.s8,  ret.s0);
    ret.s0 = $FUNC(hihi.s9,  ret.s0);
    ret.s0 = $FUNC(hihi.sA,  ret.s0);
    ret.s0 = $FUNC(hihi.sB,  ret.s0);
    ret.s0 = $FUNC(hihi.sC,  ret.s0);
    ret.s0 = $FUNC(hihi.sD,  ret.s0);
    ret.s0 = $FUNC(hihi.sE,  ret.s0);
    ret.s0 = $FUNC(hihi.sF,  ret.s0);

    return ($Arg0VecType)(ret.s0);
  }];

defvar work_group_scan_exclusive_function_op_scalar = [{
    $Arg0Type acc = *$Arg1VarName;
    *$Arg1VarName = $FUNC($Arg0VarName, *$Arg1VarName);
    return acc;
  }];
