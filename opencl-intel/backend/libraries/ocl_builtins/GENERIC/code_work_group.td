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


defvar work_group_inclusive_function_op_scalar = [{
    $Arg0Type acc = $FUNC($Arg0VarName, *$Arg1VarName);
    *$Arg1VarName = acc;
    return acc;
  }];

defvar work_group_inclusive_function_op_mask = [{
    $MaskCastTy$VecLength arg0 = __builtin_astype($Arg0VarName, $MaskCastTy$VecLength);
    $MaskCastTy$VecLength arg1 = __builtin_convertvector(__builtin_astype($Arg1VarName, int$VecLength), $MaskCastTy$VecLength);
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
    $MaskCastTy$VecLength arg1 = __builtin_convertvector(__builtin_astype($Arg1VarName, int$VecLength), $MaskCastTy$VecLength);
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
    $MaskCastTy$VecLength arg1 = __builtin_convertvector(__builtin_astype($Arg1VarName, int$VecLength), $MaskCastTy$VecLength);
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
