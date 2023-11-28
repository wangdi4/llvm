//===-- AsmOperands.h - Define mappings between assembler names and values ===//
//
// Copyright (C) 2017 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This files contains the mapping between assembly names for specific
// integer values of operands. It is meant to be included several times, with
// different values of CSA_ASM_OPERAND and (optionally) CSA_ASM_OPERAND_VALUE.
//
// This file contains several calls to a macro:
// CSA_ASM_OPERAND(Asm, Enum, Default, ...)
//
// The value Asm refers to the name of the operand type in the tablegen file.
// The value Enum refers to the name of the operand type as an enum
// (CSAInstrInfo.h contains a call to this file that defines enums containing
// all of these values). The value Default contains the default value, which is
// not necessarily equivalent to the 0 value.
// After those three parameters, the values of the operand enum are specified.
//
// Values are wrapped in a call to CSA_ASM_OPERAND_VALUE. By modifying the
// definition of this value, it is possible to get, for example, a list of
// stringified values for the enum. If the macro is not defined before calling
// this file, a default implementation that just passes the identifier through
// is provided.
//
//===----------------------------------------------------------------------===//

#ifndef CSA_ASM_OPERAND
#  error "Need to define CSA_ASM_OPERAND before including AsmOperands.h"
#endif

#ifndef CSA_ASM_OPERAND_VALUE
#  define CSA_ASM_OPERAND_VALUE(x) x
#endif

CSA_ASM_OPERAND(RMode, RoundingMode, ROUND_NEAREST,
  CSA_ASM_OPERAND_VALUE(ROUND_NEAREST),
  CSA_ASM_OPERAND_VALUE(ROUND_DOWNWARD),
  CSA_ASM_OPERAND_VALUE(ROUND_UPWARD),
  CSA_ASM_OPERAND_VALUE(ROUND_TOWARDZERO),
  CSA_ASM_OPERAND_VALUE(ROUND_NEAREST_NW),
  CSA_ASM_OPERAND_VALUE(ROUND_DOWNWARD_NW),
  CSA_ASM_OPERAND_VALUE(ROUND_UPWARD_NW),
  CSA_ASM_OPERAND_VALUE(ROUND_TOWARDZERO_NW))

// This is reversed from the simulator's convention to match the intrinsic.
// TODO: See if it is possible to match the simulator, to not confuse people who
// write the numeric value of the parameter in inline assembly.
CSA_ASM_OPERAND(MemLvl, MemLvl, MEMLEVEL_T0,
  CSA_ASM_OPERAND_VALUE(MEMLEVEL_NTA),
  CSA_ASM_OPERAND_VALUE(MEMLEVEL_T2),
  CSA_ASM_OPERAND_VALUE(MEMLEVEL_T1),
  CSA_ASM_OPERAND_VALUE(MEMLEVEL_T0))

// GETMANT operands.
CSA_ASM_OPERAND(Signctl, Signctl, SIGNCTL_PROP,
  CSA_ASM_OPERAND_VALUE(SIGNCTL_PROP),
  CSA_ASM_OPERAND_VALUE(SIGNCTL_FORCE),
  CSA_ASM_OPERAND_VALUE(SIGNCTL_FORCE_AND_CHECK))

CSA_ASM_OPERAND(Interval, Interval, INTERVAL0,
  CSA_ASM_OPERAND_VALUE(INTERVAL0),
  CSA_ASM_OPERAND_VALUE(INTERVAL1),
  CSA_ASM_OPERAND_VALUE(INTERVAL2),
  CSA_ASM_OPERAND_VALUE(INTERVAL3))

// FP comparison operands.
CSA_ASM_OPERAND(FPOrdered, FPOrdered, FLT_UNORDERED,
  CSA_ASM_OPERAND_VALUE(FLT_UNORDERED),
  CSA_ASM_OPERAND_VALUE(FLT_ORDERED))

CSA_ASM_OPERAND(FPSignaling, FPSignaling, FLT_NONSIGNALING,
  CSA_ASM_OPERAND_VALUE(FLT_NONSIGNALING),
  CSA_ASM_OPERAND_VALUE(FLT_SIGNALING))

#undef CSA_ASM_OPERAND
#undef CSA_ASM_OPERAND_VALUE
