//===-- CSAISelLowering.cpp - CSA DAG Lowering Implementation  ------------===//
//
// Copyright (C) 2017-2018 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file implements the CSATargetLowering class.
//
//===----------------------------------------------------------------------===//

#include "CSAISelLowering.h"
#include "CSA.h"
#include "CSAMachineFunctionInfo.h"
#include "CSASubtarget.h"
#include "CSATargetMachine.h"
#include "CSAUtils.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/CallingConv.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalAlias.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

#define DEBUG_TYPE "csa-lower"

STATISTIC(NumTailCalls, "Number of tail calls");

static cl::opt<bool> EnableCSATailCalls("enable-csa-tail-calls",
                                        cl::desc("CSA: Enable tail calls."));

CSATargetLowering::CSATargetLowering(const TargetMachine &TM,
                                     const CSASubtarget &ST)
    : TargetLowering(TM), Subtarget(ST) {

  // Set up the register classes.
  // The actual allocation should depend on the context (serial vs. parallel)
  addRegisterClass(MVT::i1, &CSA::I1RegClass);
  addRegisterClass(MVT::i8, &CSA::I8RegClass);
  addRegisterClass(MVT::i16, &CSA::I16RegClass);
  addRegisterClass(MVT::i32, &CSA::I32RegClass);
  addRegisterClass(MVT::i64, &CSA::I64RegClass);
  addRegisterClass(MVT::f32, &CSA::I32RegClass);
  addRegisterClass(MVT::f64, &CSA::I64RegClass);
  addRegisterClass(MVT::v2f32, &CSA::I64RegClass);

  // always lower memset, memcpy, and memmove intrinsics to load/store
  // instructions, rather
  // then generating calls to memset, mempcy or memmove.
  MaxStoresPerMemset  = (unsigned)0xFFFFFFFF;
  MaxStoresPerMemcpy  = (unsigned)0xFFFFFFFF;
  MaxStoresPerMemmove = (unsigned)0xFFFFFFFF;

  setBooleanContents(ZeroOrOneBooleanContent);
  setBooleanVectorContents(ZeroOrOneBooleanContent); // FIXME: Is this correct?

  // Compute derived properties from the register classes
  setStackPointerRegisterToSaveRestore(CSA::SP);
  computeRegisterProperties(ST.getRegisterInfo());

  // To disable the following OPT SelectinDAGBuilder.cpp cause it can
  // create multiple backedges for Fortran program using its interface feature:
  // If this is a series of conditions that are or'd or and'd together, emit
  // this as a sequence of branches instead of setcc's with and/or operations.
  // As long as jumps are not expensive, this should improve performance.
  // For example, instead of something like:
  //     cmp A, B
  //     C = seteq
  //     cmp D, E
  //     F = setle
  //     or C, F
  //     jnz foo
  // Emit:
  //     cmp A, B
  //     je foo
  //     cmp D, E
  //     jle foo
  //
  setJumpIsExpensive(true);

  // Scheduling shouldn't be relevant
  // setSchedulingPreference(Sched::ILP);

  // Provide all sorts of operation actions

  // Operations we want expanded for all types
  for (MVT VT : MVT::integer_valuetypes()) {
    // If this type is generally supported
    bool isTypeSupported =
      ((VT == MVT::i8 && ST.hasI8()) || (VT == MVT::i16 && ST.hasI16()) ||
       (VT == MVT::i32 && ST.hasI32()) || (VT == MVT::i64 && ST.hasI64()));

    setOperationAction(ISD::BR_CC, VT, Expand);
    setOperationAction(ISD::SELECT_CC, VT, Expand);

    setOperationAction(ISD::SIGN_EXTEND, VT, Expand);

    // Arithmetic
    setOperationAction(ISD::ADDC, VT, Expand);
    setOperationAction(ISD::ADDE, VT, Expand);
    setOperationAction(ISD::SUBC, VT, Expand);
    setOperationAction(ISD::SUBE, VT, Expand);
    setOperationAction(ISD::ADDCARRY, VT, Custom);
    setOperationAction(ISD::SUBCARRY, VT, Custom);
    setOperationAction(ISD::UADDO, VT, Custom);
    setOperationAction(ISD::USUBO, VT, Custom);
    // MUL_LOHI is represented as a custom expansion of XMUL.
    if (VT != MVT::i64) {
      setOperationAction(ISD::SMUL_LOHI, VT, Custom);
      setOperationAction(ISD::UMUL_LOHI, VT, Custom);
    } else {
      setOperationAction(ISD::SMUL_LOHI, VT, Expand);
      setOperationAction(ISD::UMUL_LOHI, VT, Expand);
    }
    setOperationAction(ISD::MULHS, VT, Expand);
    setOperationAction(ISD::MULHU, VT, Expand);
    setOperationAction(ISD::UREM, VT, Expand);
    setOperationAction(ISD::SREM, VT, Expand);
    setOperationAction(ISD::UDIVREM, VT, Expand);
    setOperationAction(ISD::SDIVREM, VT, Expand);

    setOperationAction(ISD::SHL_PARTS, VT, Expand);
    setOperationAction(ISD::SRL_PARTS, VT, Expand);
    setOperationAction(ISD::SRA_PARTS, VT, Expand);

    // Bit manipulation
    setOperationAction(ISD::ROTL, VT, Expand);
    setOperationAction(ISD::ROTR, VT, Expand);
    setOperationAction(ISD::BSWAP, VT, Expand);

    LegalizeAction action = (isTypeSupported && ST.hasBitOp()) ? Legal : Expand;
    setOperationAction(ISD::CTPOP, VT, action);
    setOperationAction(ISD::CTTZ, VT, action);
    setOperationAction(ISD::CTTZ_ZERO_UNDEF, VT, Expand);
    setOperationAction(ISD::CTLZ, VT, action);
    setOperationAction(ISD::CTLZ_ZERO_UNDEF, VT, Expand);

    // Atomic operations
    if (isTypeSupported && ST.hasRMWAtomic()) {
      setOperationAction(ISD::ATOMIC_LOAD_AND, VT, Legal);
      setOperationAction(ISD::ATOMIC_LOAD_ADD, VT, Legal);
      setOperationAction(ISD::ATOMIC_LOAD_MIN, VT, Legal);
      setOperationAction(ISD::ATOMIC_LOAD_MAX, VT, Legal);
      setOperationAction(ISD::ATOMIC_LOAD_OR,  VT, Legal);
      setOperationAction(ISD::ATOMIC_SWAP, VT, Legal);
      setOperationAction(ISD::ATOMIC_LOAD_XOR, VT, Legal);
    }
    setOperationAction(ISD::ATOMIC_CMP_SWAP, VT, Legal);

    setOperationAction(ISD::ATOMIC_LOAD, VT, Custom);
    setOperationAction(ISD::ATOMIC_STORE, VT, Custom);

    setOperationAction(ISD::DYNAMIC_STACKALLOC, VT, Expand);
  }

  setOperationAction(ISD::STACKSAVE, MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE, MVT::Other, Expand);

  for (MVT VT : MVT::fp_valuetypes()) {
    setOperationAction(ISD::BR_CC, VT, Expand);
    setOperationAction(ISD::SELECT_CC, VT, Expand);
  }

  setOperationAction(ISD::BR_JT, MVT::Other, Expand);

  // Loads & stores
  for (MVT VT : MVT::integer_valuetypes()) {
    //  i1 used to be promote, now expand...
    setLoadExtAction(ISD::EXTLOAD, VT, MVT::i1, Promote);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i1, Promote);
    setLoadExtAction(ISD::ZEXTLOAD, VT, MVT::i1, Promote);

    // for now (likely revisit)
    setLoadExtAction(ISD::EXTLOAD, VT, MVT::i8, Expand);
    setLoadExtAction(ISD::ZEXTLOAD, VT, MVT::i8, Expand);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i8, Expand);
    setLoadExtAction(ISD::EXTLOAD, VT, MVT::i16, Expand);
    setLoadExtAction(ISD::ZEXTLOAD, VT, MVT::i16, Expand);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i16, Expand);
    setLoadExtAction(ISD::EXTLOAD, VT, MVT::i32, Expand);
    setLoadExtAction(ISD::ZEXTLOAD, VT, MVT::i32, Expand);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i32, Expand);

    // We don't accept any truncstore of integer registers.
    setTruncStoreAction(VT, MVT::i32, Expand);
    setTruncStoreAction(VT, MVT::i16, Expand);
    setTruncStoreAction(VT, MVT::i8, Expand);
    setTruncStoreAction(VT, MVT::i1, Expand);
  }

  // Load and store floats as equivalently-sized integers.
  setOperationPromotedToType(ISD::LOAD, MVT::f32, MVT::i32);
  setOperationPromotedToType(ISD::LOAD, MVT::f64, MVT::i64);
  setOperationPromotedToType(ISD::STORE, MVT::f32, MVT::i32);
  setOperationPromotedToType(ISD::STORE, MVT::f64, MVT::i64);

  setLoadExtAction(ISD::EXTLOAD, MVT::f64, MVT::f32, Expand);

  setTruncStoreAction(MVT::f64, MVT::f32, Expand);

  // SETOEQ and SETUNE require checking two conditions.
  /*
  setCondCodeAction(ISD::SETOEQ, MVT::f32, Expand);
  setCondCodeAction(ISD::SETOEQ, MVT::f64, Expand);
  setCondCodeAction(ISD::SETOEQ, MVT::f80, Expand);
  setCondCodeAction(ISD::SETUNE, MVT::f32, Expand);
  setCondCodeAction(ISD::SETUNE, MVT::f64, Expand);
  setCondCodeAction(ISD::SETUNE, MVT::f80, Expand);
  */

  // No direct conversions to/from small integers and floating point
  setOperationAction(ISD::UINT_TO_FP, MVT::i1, Promote);
  setOperationAction(ISD::UINT_TO_FP, MVT::i8, Promote);
  setOperationAction(ISD::UINT_TO_FP, MVT::i16, Promote);

  setOperationAction(ISD::SINT_TO_FP, MVT::i1, Promote);
  setOperationAction(ISD::SINT_TO_FP, MVT::i8, Promote);
  setOperationAction(ISD::SINT_TO_FP, MVT::i16, Promote);

  setOperationAction(ISD::FP_TO_UINT, MVT::i1, Promote);
  setOperationAction(ISD::FP_TO_UINT, MVT::i8, Promote);
  setOperationAction(ISD::FP_TO_UINT, MVT::i16, Promote);

  setOperationAction(ISD::FP_TO_SINT, MVT::i1, Promote);
  setOperationAction(ISD::FP_TO_SINT, MVT::i8, Promote);
  setOperationAction(ISD::FP_TO_SINT, MVT::i16, Promote);

  // Allow full FP literals
  setOperationAction(ISD::ConstantFP, MVT::f32, Legal);
  setOperationAction(ISD::ConstantFP, MVT::f64, Legal);

  // SIMD ops--we have to mark what is not legal.
  setOperationAction(ISD::FNEG, MVT::v2f32, Expand);
  setOperationAction(ISD::FDIV, MVT::v2f32, Expand);
  setOperationAction(ISD::FREM, MVT::v2f32, Expand);
  setOperationAction(ISD::FP_ROUND, MVT::v2f32, Expand);
  setOperationAction(ISD::EXTRACT_VECTOR_ELT, MVT::v2f32, Custom);

  /*  These are to enable as CG work is done
    // Short float
    setOperationAction(ISD::FP16_TO_FP, MVT::f32, Expand);
    setOperationAction(ISD::FP_TO_FP16, MVT::f32, Expand);
  */

  setOperationAction(ISD::FNEG, MVT::f32, Legal);
  setOperationAction(ISD::FNEG, MVT::f64, Legal);
  setOperationAction(ISD::FABS, MVT::f32, Legal);
  setOperationAction(ISD::FABS, MVT::f64, Legal);

  // Allow various FP operations (temporarily.)
  // The intent is these will be provided via a math library
  if (ST.hasMath0()) {
    // Order from ISDOpcodes.h
    // setOperationAction(ISD::FREM,  MVT::f32, Legal);
    // setOperationAction(ISD::FREM,  MVT::f64, Legal);
    setOperationAction(ISD::FCOPYSIGN, MVT::f32, Expand);
    setOperationAction(ISD::FCOPYSIGN, MVT::f64, Expand);
    setOperationAction(ISD::FSQRT, MVT::f32, Legal);
    setOperationAction(ISD::FSQRT, MVT::f64, Legal);
    setOperationAction(ISD::FSIN, MVT::f32, Legal);
    setOperationAction(ISD::FSIN, MVT::f64, Legal);
    setOperationAction(ISD::FCOS, MVT::f32, Legal);
    setOperationAction(ISD::FCOS, MVT::f64, Legal);
    setOperationAction(ISD::FTAN, MVT::f32, Legal);
    setOperationAction(ISD::FTAN, MVT::f64, Legal);
    setOperationAction(ISD::FATAN, MVT::f32, Legal);
    setOperationAction(ISD::FATAN, MVT::f64, Legal);
    setOperationAction(ISD::FATAN2, MVT::f32, Legal);
    setOperationAction(ISD::FATAN2, MVT::f64, Legal);
    // setOperationAction(ISD::FPOWI, MVT::f32, Legal);
    // setOperationAction(ISD::FPOWI, MVT::f64, Legal);
    setOperationAction(ISD::FPOW, MVT::f32, Legal);
    setOperationAction(ISD::FPOW, MVT::f64, Legal);
    setOperationAction(ISD::FLOG, MVT::f32, Legal);
    setOperationAction(ISD::FLOG, MVT::f64, Legal);
    setOperationAction(ISD::FLOG2, MVT::f32, Legal);
    setOperationAction(ISD::FLOG2, MVT::f64, Legal);
    // setOperationAction(ISD::FLOG10,MVT::f32, Legal);
    // setOperationAction(ISD::FLOG10,MVT::f64, Legal);
    setOperationAction(ISD::FEXP, MVT::f32, Legal);
    setOperationAction(ISD::FEXP, MVT::f64, Legal);
    setOperationAction(ISD::FEXP2, MVT::f32, Legal);
    setOperationAction(ISD::FEXP2, MVT::f64, Legal);
    setOperationAction(ISD::FCEIL, MVT::f32, Legal);
    setOperationAction(ISD::FCEIL, MVT::f64, Legal);
    setOperationAction(ISD::FTRUNC, MVT::f32, Legal);
    setOperationAction(ISD::FTRUNC, MVT::f64, Legal);
    // setOperationAction(ISD::FRINT, MVT::f32, Legal);
    // setOperationAction(ISD::FRINT, MVT::f64, Legal);
    // setOperationAction(ISD::FNEARBYINT,MVT::f32, Legal);
    // setOperationAction(ISD::FNEARBYINT,MVT::f64, Legal);
    setOperationAction(ISD::FROUND, MVT::f32, Legal);
    setOperationAction(ISD::FROUND, MVT::f64, Legal);
    setOperationAction(ISD::FFLOOR, MVT::f32, Legal);
    setOperationAction(ISD::FFLOOR, MVT::f64, Legal);
    // setOperationAction(ISD::FSINCOS, MVT::f32, Legal);
    // setOperationAction(ISD::FSINCOS, MVT::f64, Legal);
  } else {
    // Order from ISDOpcodes.h
    // Same as hasMath0. May be changed when div/sqrt support is added
    setOperationAction(ISD::FCOPYSIGN, MVT::f32, Expand);
    setOperationAction(ISD::FCOPYSIGN, MVT::f64, Expand);
    setOperationAction(ISD::FSQRT, MVT::f32, Legal);
    setOperationAction(ISD::FSQRT, MVT::f64, Legal);
    
    setOperationAction(ISD::FSIN, MVT::f32, LibCall);
    setOperationAction(ISD::FSIN, MVT::f64, LibCall);
    setOperationAction(ISD::FCOS, MVT::f32, LibCall);
    setOperationAction(ISD::FCOS, MVT::f64, LibCall);
    setOperationAction(ISD::FTAN, MVT::f32, LibCall);
    setOperationAction(ISD::FTAN, MVT::f64, LibCall);
    setOperationAction(ISD::FATAN, MVT::f32, LibCall);
    setOperationAction(ISD::FATAN, MVT::f64, LibCall);
    setOperationAction(ISD::FATAN2, MVT::f32, LibCall);
    setOperationAction(ISD::FATAN2, MVT::f64, LibCall);
    setOperationAction(ISD::FPOW, MVT::f32, LibCall);
    setOperationAction(ISD::FPOW, MVT::f64, LibCall);
    setOperationAction(ISD::FLOG, MVT::f32, LibCall);
    setOperationAction(ISD::FLOG, MVT::f64, LibCall);
    setOperationAction(ISD::FLOG2, MVT::f32, LibCall);
    setOperationAction(ISD::FLOG2, MVT::f64, LibCall);
    setOperationAction(ISD::FLOG10,MVT::f32, LibCall);
    setOperationAction(ISD::FLOG10,MVT::f64, LibCall);
    setOperationAction(ISD::FEXP, MVT::f32, LibCall);
    setOperationAction(ISD::FEXP, MVT::f64, LibCall);
    setOperationAction(ISD::FEXP2, MVT::f32, LibCall);
    setOperationAction(ISD::FEXP2, MVT::f64, LibCall);
    setOperationAction(ISD::FCEIL, MVT::f32, LibCall);
    setOperationAction(ISD::FCEIL, MVT::f64, LibCall);
    setOperationAction(ISD::FTRUNC, MVT::f32, LibCall);
    setOperationAction(ISD::FTRUNC, MVT::f64, LibCall);
    setOperationAction(ISD::FROUND, MVT::f32, LibCall);
    setOperationAction(ISD::FROUND, MVT::f64, LibCall);
    setOperationAction(ISD::FFLOOR, MVT::f32, LibCall);
    setOperationAction(ISD::FFLOOR, MVT::f64, LibCall);
  }

  setOperationAction(ISD::GlobalAddress, MVT::i64, Custom);
  setOperationAction(ISD::ExternalSymbol, MVT::i64, Custom);
  setOperationAction(ISD::BlockAddress, MVT::i64, Custom);
  setOperationAction(ISD::JumpTable, MVT::i64, Custom);

  //  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1,   Expand);

  // varargs support
  setOperationAction(ISD::VASTART, MVT::Other, Expand);
  setOperationAction(ISD::VAARG, MVT::Other, Expand);
  setOperationAction(ISD::VAEND, MVT::Other, Expand);
  setOperationAction(ISD::VACOPY, MVT::Other, Expand);

  // setOperationAction(ISD::READCYCLECOUNTER,   MVT::i64,   Legal);

  setOperationAction(ISD::PREFETCH, MVT::Other, Legal);

  setTargetDAGCombine(ISD::SELECT);
  setTargetDAGCombine(ISD::SMIN);
  setTargetDAGCombine(ISD::SMAX);
  setTargetDAGCombine(ISD::UMIN);
  setTargetDAGCombine(ISD::UMAX);
  setTargetDAGCombine(ISD::FMINNUM);
  setTargetDAGCombine(ISD::FMAXNUM);
  setTargetDAGCombine(ISD::FMINNAN);
  setTargetDAGCombine(ISD::FMAXNAN);
}

EVT CSATargetLowering::getSetCCResultType(const DataLayout &DL,
                                          LLVMContext &Context, EVT VT) const {
  return MVT::i1;
}

SDValue CSATargetLowering::LowerOperation(SDValue Op, SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  case ISD::GlobalAddress:
    return LowerGlobalAddress(Op, DAG);
  case ISD::ExternalSymbol:
    return LowerExternalSymbol(Op, DAG);
  case ISD::BlockAddress:
    return LowerBlockAddress(Op, DAG);
  case ISD::JumpTable:
    return LowerJumpTable(Op, DAG);
    /*
  case ISD::RETURNADDR:       return LowerRETURNADDR(Op, DAG);
  case ISD::FRAMEADDR:        return LowerFRAMEADDR(Op, DAG);
  case ISD::VASTART:          return LowerVASTART(Op, DAG);
    */
  case ISD::ATOMIC_LOAD:
    return LowerAtomicLoad(Op, DAG);
  case ISD::ATOMIC_STORE:
    return LowerAtomicStore(Op, DAG);
  case ISD::SMUL_LOHI:
    return LowerMUL_LOHI(Op, DAG);
  case ISD::UMUL_LOHI:
    return LowerMUL_LOHI(Op, DAG);
  case ISD::ADDCARRY:
    return LowerADDSUB_Carry(Op, DAG, true);
  case ISD::SUBCARRY:
    return LowerADDSUB_Carry(Op, DAG, false);
  case ISD::UADDO:
    return LowerADDSUB_Carry(Op, DAG, true);
  case ISD::USUBO:
    return LowerADDSUB_Carry(Op, DAG, false);
  case ISD::EXTRACT_VECTOR_ELT:
    return LowerExtractElement(Op, DAG);
  default:
    llvm_unreachable("unimplemented operand");
  }
}

const char *CSATargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  default:
    return nullptr;
  case CSAISD::Call:
    return "CSAISD::Call";
  case CSAISD::TailCall:
    return "CSAISD::TailCall";
  case CSAISD::Ret:
    return "CSAISD::Ret";
  case CSAISD::Wrapper:
    return "CSAISD::Wrapper";
  case CSAISD::Min:
    return "CSAISD::Min";
  case CSAISD::UMin:
    return "CSAISD::UMin";
  case CSAISD::Max:
    return "CSAISD::Max";
  case CSAISD::UMax:
    return "CSAISD::UMax";
  }
}

SDValue CSATargetLowering::LowerGlobalAddress(SDValue Op,
                                              SelectionDAG &DAG) const {
  const GlobalValue *GV = cast<GlobalAddressSDNode>(Op)->getGlobal();
  int64_t Offset        = cast<GlobalAddressSDNode>(Op)->getOffset();

  // Create the TargetGlobalAddress node
  // DO NOT fold in the constant offset for now
  SDValue Result = DAG.getTargetGlobalAddress(
    GV, SDLoc(Op), getPointerTy(DAG.getDataLayout()), 0);
  Result = DAG.getNode(CSAISD::Wrapper, SDLoc(Op),
                       getPointerTy(DAG.getDataLayout()), Result);
  if (Offset) {
    SDValue Remaining = DAG.getConstant(Offset, SDLoc(Op), MVT::i64);
    Result = DAG.getNode(ISD::ADD, SDLoc(Op), MVT::i64, Result, Remaining);
  }

  return Result;
}

SDValue CSATargetLowering::LowerExternalSymbol(SDValue Op,
                                               SelectionDAG &DAG) const {
  const char *Sym = cast<ExternalSymbolSDNode>(Op)->getSymbol();
  SDValue Result =
    DAG.getTargetExternalSymbol(Sym, getPointerTy(DAG.getDataLayout()));
  return DAG.getNode(CSAISD::Wrapper, SDLoc(Op),
                     getPointerTy(DAG.getDataLayout()), Result);
}

SDValue CSATargetLowering::LowerBlockAddress(SDValue Op,
                                             SelectionDAG &DAG) const {
  const BlockAddress *BA = cast<BlockAddressSDNode>(Op)->getBlockAddress();
  SDValue Result =
    DAG.getTargetBlockAddress(BA, getPointerTy(DAG.getDataLayout()));
  return DAG.getNode(CSAISD::Wrapper, SDLoc(Op),
                     getPointerTy(DAG.getDataLayout()), Result);
}

SDValue CSATargetLowering::LowerJumpTable(SDValue Op, SelectionDAG &DAG) const {
  JumpTableSDNode *JT = cast<JumpTableSDNode>(Op);
  SDValue Result =
    DAG.getTargetJumpTable(JT->getIndex(), getPointerTy(DAG.getDataLayout()));
  return DAG.getNode(CSAISD::Wrapper, SDLoc(JT),
                     getPointerTy(DAG.getDataLayout()), Result);
}

SDValue CSATargetLowering::LowerAtomicLoad(SDValue Op,
                                           SelectionDAG &DAG) const {
  AtomicSDNode *AL = cast<AtomicSDNode>(Op);
  // Sufficiently small regular loads are already atomic.
  // If this is a large load, don't lower here. Probably the right thing to
  // do is to use AtomicExpandPass to take care of it.
  if (AL->getMemoryVT().getStoreSizeInBits() > 64) {
    return Op;
  }

  SDLoc DL(Op);
  return DAG.getLoad(AL->getMemoryVT(), DL, AL->getChain(), AL->getBasePtr(),
                     AL->getMemOperand());
}

SDValue CSATargetLowering::LowerAtomicStore(SDValue Op,
                                            SelectionDAG &DAG) const {
  AtomicSDNode *AS = cast<AtomicSDNode>(Op);
  // Sufficiently small regular stores are already atomic.
  if (AS->getMemoryVT().getStoreSizeInBits() > 64) {
    return Op;
  }

  SDLoc DL(Op);
  return DAG.getStore(AS->getChain(), DL, AS->getVal(), AS->getBasePtr(),
                      AS->getMemOperand());
}

SDValue CSATargetLowering::LowerMUL_LOHI(SDValue Op, SelectionDAG &DAG) const {
  SDLoc dl(Op);
  SDValue LHS     = Op.getOperand(0);
  SDValue RHS     = Op.getOperand(1);
  MVT partVT      = LHS.getSimpleValueType();
  MVT doubleVT    = MVT::getIntegerVT(partVT.getSizeInBits() * 2);
  SDValue InOps[] = {LHS, RHS};
  bool isSigned   = (Op.getNode()->getOpcode() == ISD::SMUL_LOHI);

  // We don't have an instruction that spits it out into two nodes, we have one
  // that breaks it into the one node.
  unsigned opcode;
  switch (partVT.SimpleTy) {
  case MVT::i8:
    opcode = isSigned ? CSA::XMULS8 : CSA::XMULU8;
    break;
  case MVT::i16:
    opcode = isSigned ? CSA::XMULS16 : CSA::XMULU16;
    break;
  case MVT::i32:
    opcode = isSigned ? CSA::XMULS32 : CSA::XMULU32;
    break;
  default:
    assert(false && "Illegal type for XMUL");
  }

  SDValue Xmul = SDValue{DAG.getMachineNode(opcode, dl, doubleVT, InOps), 0};
  SDValue Lo = DAG.getZExtOrTrunc(Xmul, dl, partVT);
  SDValue Hi = DAG.getZExtOrTrunc(
      DAG.getNode(ISD::SRL, dl, doubleVT, Xmul,
        DAG.getConstant(partVT.getSizeInBits(), dl, doubleVT)),
      dl, partVT);
  SDValue Ops[] = {Lo, Hi};
  return DAG.getMergeValues(Ops, dl);
}

SDValue CSATargetLowering::LowerADDSUB_Carry(SDValue Op, SelectionDAG &DAG,
    bool isAdd) const {
  SDLoc dl(Op);
  SDValue LHS     = Op.getOperand(0);
  SDValue RHS     = Op.getOperand(1);
  SDValue CarryIn = Op.getNumOperands() == 3 ? Op.getOperand(2) :
    DAG.getConstant(0, dl, MVT::i1);
  MVT VT          = LHS.getSimpleValueType();
  SDValue InOps[] = {LHS, RHS, CarryIn};

  // We don't have an instruction that spits it out into two nodes, we have one
  // that breaks it into the one node.
  unsigned opcode;
  switch (VT.SimpleTy) {
  case MVT::i8:
    opcode = isAdd ? CSA::ADC8 : CSA::SBB8;
    break;
  case MVT::i16:
    opcode = isAdd ? CSA::ADC16 : CSA::SBB16;
    break;
  case MVT::i32:
    opcode = isAdd ? CSA::ADC32 : CSA::SBB32;
    break;
  case MVT::i64:
    opcode = isAdd ? CSA::ADC64 : CSA::SBB64;
    break;
  default:
    assert(false && "Illegal type for ADC/SBB");
  }

  SDNode *carriedOp = DAG.getMachineNode(opcode, dl, VT, MVT::i1, InOps);
  SDValue res = SDValue{carriedOp, 0};
  SDValue carryOut = SDValue{carriedOp, 1}; 
  SDValue Ops[] = {res, carryOut};
  return DAG.getMergeValues(Ops, dl);
}

SDValue CSATargetLowering::LowerExtractElement(SDValue Op,
    SelectionDAG &DAG) const {
  SDLoc dl(Op);
  SDValue Src     = Op.getOperand(0);
  SDValue Idx     = Op.getOperand(1);
  MVT VT          = Op.getSimpleValueType();
  assert(VT.getSizeInBits() == 32 && "Only supporting 64->32 unpack");

  // Unpack the value into two LICs.
  SDNode *Unpack = DAG.getMachineNode(CSA::UNPACK64_32, dl,
      {MVT::f32, MVT::f32}, Src);

  // Create a select to choose the result of the UNPACK. If the condition is
  // constant, this should be constant-folded away.
  SDValue Res = DAG.getSelect(dl, MVT::f32, Idx, SDValue(Unpack, 1),
      SDValue(Unpack, 0));

  // If the type is not right, bitcast it.
  if (VT != MVT::f32)
    Res = DAG.getBitcast(VT, Res);

  return Res;
}

SDValue CSATargetLowering::PerformDAGCombine(SDNode *N,
                                             DAGCombinerInfo &DCI) const {
  SelectionDAG &DAG = DCI.DAG;
  switch (N->getOpcode()) {
  case ISD::SELECT:
    return CombineSelect(N, DAG);
  case ISD::SMIN:
  case ISD::SMAX:
  case ISD::UMIN:
  case ISD::UMAX:
  case ISD::FMINNUM:
  case ISD::FMAXNUM:
  case ISD::FMINNAN:
  case ISD::FMAXNAN:
    return CombineMinMax(N, DAG);

  // Return SDValue{} for opcodes that we don't handle to show that we don't
  // handle them.
  default:
    return SDValue{};
  }
}

SDValue CSATargetLowering::CombineSelect(SDNode *N, SelectionDAG &DAG) const {

  // Grab inputs to the select.
  const SDLoc DL{N};
  const EVT VT      = N->getValueType(0);
  SDValue Cond      = N->getOperand(0);
  const SDValue OpT = N->getOperand(1);
  const SDValue OpF = N->getOperand(2);

  // Is the condition a not? If so, record that and move along to its def.
  const bool CondBehindNot =
    Cond.getOpcode() == ISD::XOR and
    Cond.getOperand(1) == DAG.getBoolConstant(true, SDLoc{Cond},
                                              Cond.getValueType(),
                                              Cond.getValueType());
  const bool NotAlsoOneUse =
    not CondBehindNot or Cond.getOperand(0).hasOneUse();
  if (CondBehindNot)
    Cond = Cond->getOperand(0);

  // These are set depending on the type of operations to determine what type of
  // min/max to make.
  bool IsMax, IsUnsigned;

  // Since comparisons are replaced greedily, Cond might already be a min/max
  // op.
  const bool CondIsMinMax =
    Cond.getOpcode() == CSAISD::Min or Cond.getOpcode() == CSAISD::UMin or
    Cond.getOpcode() == CSAISD::Max or Cond.getOpcode() == CSAISD::UMax;

  // Otherwise, it should be a comparison.
  const bool CondIsCmp = Cond.getOpcode() == ISD::SETCC;
  if (not CondIsMinMax and not CondIsCmp)
    return SDValue{};

  // Check Cond to make sure that it has the same inputs. If this is the case,
  // Reversed will be set depending on whether the operand order differs between
  // Cond and the select.
  // It would also possible to replace selects of the form a < b ? 0 : 1 using
  // min/max operations where only the sel output is used, but in that case the
  // min/max would be equivalent to a comparison and a normal comparison op
  // should just be used for that instead. Therefore, this check ensures that
  // only selects with the same inputs as their conditions are considered for
  // conversion to min/max ops.
  const SDValue LHS = Cond.getOperand(0);
  const SDValue RHS = Cond.getOperand(1);
  bool Reversed;
  if (OpT == LHS and OpF == RHS)
    Reversed = false;
  else if (OpT == RHS and OpF == LHS)
    Reversed = true;
  else
    return SDValue{};

  if (CondIsMinMax) {
    // For min/max, the value can be reused if the operand order is _reversed_
    // XOR the existing min/max is behind a not.
    if (Reversed != CondBehindNot)
      return Cond.getValue(0);

    // Otherwise, a max/min will need to be constructed instead.
    IsMax = Cond.getOpcode() == CSAISD::Min or Cond.getOpcode() == CSAISD::UMin;
    IsUnsigned =
      Cond.getOpcode() == CSAISD::UMin or Cond.getOpcode() == CSAISD::UMax;
  }

  else {
    assert(CondIsCmp);

    // Comparisons should be </<=/>/>=. See the notes on ISD::CmpCode in
    // ISDOpcodes.h for more details about the bitfields used here.
    const ISD::CondCode CC = cast<CondCodeSDNode>(Cond.getOperand(2))->get();
    if (bool(CC & ISD::SETOLT) == bool(CC & ISD::SETOGT))
      return SDValue{};

    // This is a max if the condition is a >/>= XOR the operands are reversed
    // XOR the comparison is behind a not. Otherwise, it is a min.
    IsMax = (bool(CC & ISD::SETOGT) != Reversed) != CondBehindNot;

    // The signedness needs to be gotten from CC.
    IsUnsigned = VT.isInteger() and CC & ISD::SETUO;
  }

  // Replace the select and condition with a min/max of the appropriate type.
  const SDValue MinMax =
    DAG.getNode(IsUnsigned ? IsMax ? CSAISD::UMax : CSAISD::UMin
                           : IsMax ? CSAISD::Max : CSAISD::Min,
                SDLoc{N}, DAG.getVTList(VT, MVT::i1), OpF, OpT);
  LLVM_DEBUG(dbgs() << "Created min/max:\n");
  LLVM_DEBUG(MinMax->dumpr());

  // Try to fold in compares. If Cond has only one use (the select that was just
  // made dead), ignore it.
  FoldComparesIntoMinMax(MinMax.getValue(1), DAG,
                         Cond.hasOneUse() and NotAlsoOneUse ? Cond : SDValue{});

  return MinMax;
}

static unsigned CSAISDMinMaxOpForISDMinMaxOp(unsigned SDOp) {

  // NOTE: We are ignoring behavior with unordered compares/NaN for now.
  switch (SDOp) {
  case ISD::SMIN:
  case ISD::FMINNUM:
  case ISD::FMINNAN:
    return CSAISD::Min;
  case ISD::SMAX:
  case ISD::FMAXNUM:
  case ISD::FMAXNAN:
    return CSAISD::Max;
  case ISD::UMIN:
    return CSAISD::UMin;
  case ISD::UMAX:
    return CSAISD::UMax;
  default:
    llvm_unreachable("unrecognized min/max opcode");
  }
}

SDValue CSATargetLowering::CombineMinMax(SDNode *N, SelectionDAG &DAG) const {
  const SDValue MinMax =
    DAG.getNode(CSAISDMinMaxOpForISDMinMaxOp(N->getOpcode()), SDLoc{N},
                DAG.getVTList(N->getValueType(0), MVT::i1), N->getOperand(0),
                N->getOperand(1));
  LLVM_DEBUG(dbgs() << "Created min/max:\n");
  LLVM_DEBUG(MinMax->dumpr());
  FoldComparesIntoMinMax(MinMax.getValue(1), DAG);
  return MinMax;
}

void CSATargetLowering::FoldComparesIntoMinMax(SDValue Sel, SelectionDAG &DAG,
                                               SDValue IgnoreCmp) const {

  const unsigned OpC = Sel->getOpcode();
  const bool IsMax   = OpC == CSAISD::Max or OpC == CSAISD::UMax;
  const SDValue LHS  = Sel->getOperand(0);
  const SDValue RHS  = Sel->getOperand(1);

  // Min effectively implements a >= and max a <=, but we can get equivalent
  // values to other comparisons by a combination of swapping operands and
  // adding nots to the output. However, if we want to fold in multiple
  // comparisons all of them have to have the same operand order. The
  // comparisons are collected into SameCmps and RevCmps according to whether
  // they would require operand reversal or not. The number of nots needed for
  // each set is tracked in {Same,Rev}NotCount.
  SmallVector<SDValue, 2> SameCmps, RevCmps;
  int SameNotCount = 0, RevNotCount = 0;

  // Find comparisons that might use the same inputs as this min/max.
  for (SDNode *const Cmp : LHS->uses()) {
    if (Cmp->getOpcode() != ISD::SETCC)
      continue;

    // Don't bother with ones that have no uses.
    if (Cmp->use_empty())
      continue;

    // Ignore IgnoreCmp.
    if (Cmp == IgnoreCmp.getNode())
      continue;

    // Only handle </<=/>/>=. See the notes on ISD::CmpCode in ISDOpcodes.h for
    // more details about the bitfields used here.
    const ISD::CondCode CC = cast<CondCodeSDNode>(Cmp->getOperand(2))->get();
    if (bool(CC & ISD::SETOLT) == bool(CC & ISD::SETOGT))
      continue;

    // Ignore any unsigned comparisons for signed mins/maxs and vice-versa.
    if (Sel.getValueType().isInteger() and
        ((OpC == CSAISD::UMin or OpC == CSAISD::UMax) != bool(CC & ISD::SETUO)))
      continue;

    // Make sure that the operands match.
    const SDValue CLHS = Cmp->getOperand(0);
    const SDValue CRHS = Cmp->getOperand(1);
    bool Reversed;
    if (LHS == CLHS and RHS == CRHS)
      Reversed = false;
    else if (LHS == CRHS and RHS == CLHS)
      Reversed = true;
    else
      continue;

    // NOTE: NaNs/-0.0/other strange values aren't being considered here yet,
    // but when we have a clearer picture of what the architecture will do with
    // them there will probably need to be extra checks here.

    // This comparison looks like a candidate for folding; put it in the right
    // set. A >= with the same operand order can be folded into a min but
    // switching to a <=, >, max, or swapping inputs each reverse the set
    // assignment. This gives the string of XORs below. A not is also needed for
    // </>.
    const bool NeedsRev =
      (bool(CC & ISD::SETOLT) != not(CC & ISD::SETOEQ)) != (IsMax != Reversed);
    (NeedsRev ? RevCmps : SameCmps).emplace_back(Cmp, 0);
    (NeedsRev ? RevNotCount : SameNotCount) += not(CC & ISD::SETOEQ);

    LLVM_DEBUG(dbgs() << " Found " << (NeedsRev ? "reversed" : "non-reversed")
                      << " comparison:\n");
    LLVM_DEBUG(Cmp->dumpr());
  }

  // If there aren't any comparisons to fold, our work here is done.
  if (SameCmps.empty() and RevCmps.empty())
    return;

  // Decide whether or not to swap operands. The following rules are used for
  // this:
  //
  //  1. Choose the set with the most comparisons that can be folded.
  //  2. If there is a tie, choose the set with the least nots needed.
  //  3. If there is still a tie, don't reverse operands.
  const bool ReverseOperands =
    RevCmps.size() > SameCmps.size() or
    (RevCmps.size() == SameCmps.size() and RevNotCount < SameNotCount);
  if (ReverseOperands)
    DAG.UpdateNodeOperands(Sel.getNode(), RHS, LHS);
  LLVM_DEBUG(dbgs() << " Choosing "
                    << (ReverseOperands ? "reversed" : "non-reversed")
                    << " operands\n");

  for (SDValue Cmp : ReverseOperands ? RevCmps : SameCmps) {
    const ISD::CondCode CC = cast<CondCodeSDNode>(Cmp->getOperand(2))->get();
    DAG.ReplaceAllUsesOfValueWith(
      Cmp,
      (CC & ISD::SETOEQ) ? Sel : DAG.getLogicalNOT(SDLoc{Cmp}, Sel, MVT::i1));
  }
}

//===----------------------------------------------------------------------===//
//                       CSA Inline Assembly Support
//===----------------------------------------------------------------------===//

/// getConstraintType - Given a constraint letter, return the type of
/// constraint it is for this target.
CSATargetLowering::ConstraintType
CSATargetLowering::getConstraintType(StringRef Constraint) const {
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    default:
      break;
    case 'a':
    case 'b':
    case 'c':
    case 'd':
    case 'A':
    case 'B':
    case 'C':
    case 'D':
      return C_RegisterClass;
    }
  }
  return TargetLowering::getConstraintType(Constraint);
}

std::pair<unsigned, const TargetRegisterClass *>
CSATargetLowering::getRegForInlineAsmConstraint(const TargetRegisterInfo *TRI,
                                                StringRef Constraint,
                                                MVT VT) const {

  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    default:
      break;
    case 'a':
      return std::make_pair(0U, &CSA::I8RegClass);
    case 'b':
      return std::make_pair(0U, &CSA::I16RegClass);
    case 'c':
      return std::make_pair(0U, &CSA::I32RegClass);
    case 'd':
      return std::make_pair(0U, &CSA::I64RegClass);

    case 'A':
      return std::make_pair(0U, &CSA::RI8RegClass);
    case 'B':
      return std::make_pair(0U, &CSA::RI16RegClass);
    case 'C':
      return std::make_pair(0U, &CSA::RI32RegClass);
    case 'D':
      return std::make_pair(0U, &CSA::RI64RegClass);
    }
  }

  // This doesn't know about any one-letter constraints.
  return TargetLowering::getRegForInlineAsmConstraint(TRI, Constraint, VT);
}

bool CSATargetLowering::isTruncateFree(EVT VT1, EVT VT2) const {
  if (!VT1.isInteger() || !VT2.isInteger())
    return false;
  unsigned NumBits1 = VT1.getSizeInBits();
  unsigned NumBits2 = VT2.getSizeInBits();
  return NumBits1 > NumBits2;
}

bool CSATargetLowering::isTruncateFree(Type *Ty1, Type *Ty2) const {
  if (!Ty1->isIntegerTy() || !Ty2->isIntegerTy())
    return false;
  unsigned NumBits1 = Ty1->getPrimitiveSizeInBits();
  unsigned NumBits2 = Ty2->getPrimitiveSizeInBits();
  return NumBits1 > NumBits2;
}

bool CSATargetLowering::isZExtFree(Type *Ty1, Type *Ty2) const {
  // x86-64 implicitly zero-extends 32-bit results in 64-bit registers.
  return Ty1->isIntegerTy(32) && Ty2->isIntegerTy(64); //&& Subtarget->is64Bit()
}

bool CSATargetLowering::isZExtFree(EVT VT1, EVT VT2) const {
  // x86-64 implicitly zero-extends 32-bit results in 64-bit registers.
  return VT1 == MVT::i32 && VT2 == MVT::i64; // && Subtarget->is64Bit()
}

bool CSATargetLowering::isZExtFree(SDValue Val, EVT VT2) const {
  EVT VT1 = Val.getValueType();
  if (isZExtFree(VT1, VT2))
    return true;

  if (Val.getOpcode() != ISD::LOAD)
    return false;

  if (!VT1.isSimple() || !VT1.isInteger() || !VT2.isSimple() ||
      !VT2.isInteger())
    return false;

  switch (VT1.getSimpleVT().SimpleTy) {
  default:
    break;
  case MVT::i8:
  case MVT::i16:
  case MVT::i32:
    // X86 has 8, 16, and 32-bit zero-extending loads.
    return true;
  }

  return false;
}

bool CSATargetLowering::isNarrowingProfitable(EVT VT1, EVT VT2) const {
  return true;
}

bool CSATargetLowering::isFMAFasterThanFMulAndFAdd(EVT VT) const {
  if (!Subtarget.hasFMA())
    return false;

  VT = VT.getScalarType();

  if (!VT.isSimple())
    return false;

  switch (VT.getSimpleVT().SimpleTy) {
  case MVT::f32:
  case MVT::f64:
    return true;
  default:
    break;
  }

  return false;
}

// Overwrite mi_op with the value specified by sd_op. This is basically a really
// dumbed-down version of InstrEmitter::AddOperand, but since it doesn't have
// access to VRBaseMap it has to assume that the register value originally in
// the operand before the one that it needs to process (last_reg) is the one it
// wants to use if it doesn't have any other hints. This will be the case for
// the instructions that this function needs to operate on because the values
// based on the SDNode are all shifted left in the original instruction.
static void overwrite_operand(MachineOperand &mi_op, SDValue sd_op,
                              unsigned last_reg, bool is_def) {
  if (const ConstantSDNode *const C = dyn_cast<ConstantSDNode>(sd_op))
    return mi_op.ChangeToImmediate(C->getSExtValue());
  if (const ConstantFPSDNode *const F = dyn_cast<ConstantFPSDNode>(sd_op))
    return mi_op.ChangeToFPImmediate(F->getConstantFPValue());
  if (const RegisterSDNode *const R = dyn_cast<RegisterSDNode>(sd_op))
    return mi_op.ChangeToRegister(R->getReg(), is_def);
  if (const FrameIndexSDNode *const FI = dyn_cast<FrameIndexSDNode>(sd_op))
    return mi_op.ChangeToFrameIndex(FI->getIndex());
  if (const ExternalSymbolSDNode *const ES =
        dyn_cast<ExternalSymbolSDNode>(sd_op))
    return mi_op.ChangeToES(ES->getSymbol(), ES->getTargetFlags());
  if (const MCSymbolSDNode *const MCS = dyn_cast<MCSymbolSDNode>(sd_op))
    return mi_op.ChangeToMCSymbol(MCS->getMCSymbol());

  assert(last_reg && "can't figure out which register to use!");
  return mi_op.ChangeToRegister(last_reg, is_def);
}

void CSATargetLowering::AdjustInstrPostInstrSelection(MachineInstr &MI,
                                                      SDNode *Node) const {

  LLVM_DEBUG(errs() << "adjusting instruction: " << MI << "from node ");
  LLVM_DEBUG(Node->print(errs()));
  LLVM_DEBUG(errs() << "\n");

  // Instruction selection has a problem where it won't add any defs to an
  // instruction that doesn't have any non-chain results on its corresponding
  // SDNode, and as a result it produces broken instructions with too few
  // operands and with mismatched operand values. This is an issue for our store
  // instructions because their only output is the issued signal and that
  // doesn't appear as an SDNode result because it defaults to a physical
  // register (%ign). In order to make sure that those are generated correctly,
  // this code identifies memory operations that have too few operands and that
  // have defs which aren't represented as SDNode results and patches those
  // instructions to make sure that all of their operands are correct.
  const MCInstrDesc &II = MI.getDesc();
  if (Node->getNumValues() == 1 and II.getNumDefs() and
      not MI.memoperands_empty() and
      MI.getNumOperands() < II.getNumOperands()) {
    LLVM_DEBUG(errs() <<
               "found defective instruction with too few operands!\n");

    // Pad out the operands until there are enough of them.
    assert(II.getNumDefs() == 1 && "This hook assumes only one def is missing");
    MachineInstrBuilder MIB{*MI.getParent()->getParent(), &MI};
    MIB.addReg(CSA::IGN);

    // Then overwrite all of them with the correct values.
    unsigned last_reg = 0;
    for (unsigned op_ind = 0; op_ind < II.getNumOperands(); ++op_ind) {
      MachineOperand &mi_op        = MI.getOperand(op_ind);
      const unsigned next_last_reg = mi_op.isReg() ? mi_op.getReg() : 0;
      overwrite_operand(mi_op, Node->getOperand(op_ind), last_reg,
                        op_ind < II.getNumDefs());
      last_reg = next_last_reg;
    }

    LLVM_DEBUG(errs() << "instruction corrected to: " << MI << "\n");
  }
}

// isLegalAddressingMode - Return true if the addressing mode represented
// by AM is legal for this target, for a load/store of the specified type.
bool CSATargetLowering::isLegalAddressingMode(const DataLayout &DL,
                                              const AddrMode &AM, Type *Ty,
                                              unsigned AddrSpace, Instruction *i) const {
  /**/
  // X86 supports extremely general addressing modes.
  //  CodeModel::Model M = getTargetMachine().getCodeModel();
  //  Reloc::Model R = getTargetMachine().getRelocationModel();

  switch (AM.Scale) {
  case 0:
  case 1:
  case 2:
  case 4:
  case 8:
    // These scales always work.
    break;
  case 3:
  case 5:
  case 9:
    // These scales are formed with basereg+scalereg.  Only accept if there is
    // no basereg yet.
    if (AM.HasBaseReg)
      return false;
    break;
  default: // Other stuff never works.
    return false;
  }

  return true;
}

  //===----------------------------------------------------------------------===//
  //                      Calling Convention Implementation
  //===----------------------------------------------------------------------===//

#include "CSAGenCallingConv.inc"

/// IsEligibleForTailCallOptimization - Check whether the call is eligible
/// for tail call optimization.
bool CSATargetLowering::IsEligibleForTailCallOptimization(
  unsigned NextStackOffset, const CSAMachineFunctionInfo &FI) const {
  if (csa_utils::isAlwaysDataFlowLinkageSet()) {
    LLVM_DEBUG(errs() << "Tail call not supported for data flow linkage\n");
    return false;
  }
        
  //  if (!EnableCSATailCalls)
  //    return false;

  // Return false if either the callee or caller has a byval argument.
  //  if (FI.hasByValArg())
  //  return false;

  // Return true if the callee's argument area is no larger than the
  // caller's.
  // return NextStackOffset <= FI.getIncomingArgSize();
  return true;
}

/// LowerCall - functions arguments are copied from virtual regs to
/// (physical regs)/(stack frame), CALLSEQ_START and CALLSEQ_END are emitted.
/// TODO: isTailCall.
SDValue CSATargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                     SmallVectorImpl<SDValue> &InVals) const {

  SelectionDAG &DAG                     = CLI.DAG;
  SDLoc &dl                             = CLI.DL;
  SmallVector<ISD::OutputArg, 32> &Outs = CLI.Outs;
  SmallVector<SDValue, 32> &OutVals     = CLI.OutVals;
  SmallVector<ISD::InputArg, 32> &Ins   = CLI.Ins;
  SDValue Chain                         = CLI.Chain;
  SDValue Callee                        = CLI.Callee;
  bool &isTailCall                      = CLI.IsTailCall;
  CallingConv::ID CallConv              = CLI.CallConv;
  bool isVarArg                         = CLI.IsVarArg;

  MachineFunction &MF = DAG.getMachineFunction();

  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());

  if (isVarArg) {
    // Handle fixed and variable arguments differently.
    // Fixed arguments go into registers as long as registers are available.
    // Variable arguments always go into memory.
    unsigned NumArgs = Outs.size();
    for (unsigned i = 0; i != NumArgs; ++i) {
      MVT ArgVT                = Outs[i].VT;
      ISD::ArgFlagsTy ArgFlags = Outs[i].Flags;
      bool Result;

      if (Outs[i].IsFixed) {
        Result =
          CC_Reg_CSA(i, ArgVT, ArgVT, CCValAssign::Full, ArgFlags, CCInfo);
      } else {
        Result = CC_Reg_VarArg_CSA(i, ArgVT, ArgVT, CCValAssign::Full, ArgFlags,
                                   CCInfo);
      }

      if (Result) {
#ifndef NDEBUG
        dbgs() << "Call operand #" << i << " has unhandled type "
               << EVT(ArgVT).getEVTString();
#endif
        llvm_unreachable(0);
      }
    }
  } else {
    // All arguments are treated the same.
    if (csa_utils::isAlwaysDataFlowLinkageSet())
      CCInfo.AnalyzeCallOperands(Outs, CC_LIC_CSA);
    else
    CCInfo.AnalyzeCallOperands(Outs, CC_Reg_CSA);
  }

  // Get a count of how many bytes are to be pushed on the stack.
  unsigned NumBytes = CCInfo.getNextStackOffset();

  if (isTailCall)
    isTailCall =
      !isVarArg && IsEligibleForTailCallOptimization(
                     NumBytes, *MF.getInfo<CSAMachineFunctionInfo>());

  if (isTailCall)
    ++NumTailCalls;

  if (!csa_utils::isAlwaysDataFlowLinkageSet() && !isTailCall)
    Chain = DAG.getCALLSEQ_START(Chain, NumBytes, 0, dl);

  SDValue StackPtr =
    DAG.getCopyFromReg(Chain, dl, CSA::SP, getPointerTy(DAG.getDataLayout()));

  SmallVector<std::pair<unsigned, SDValue>, 8> RegsToPass;
  SmallVector<SDValue, 8> MemOpChains;

  // Walk the register/memloc assignments, inserting copies/loads.
  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA       = ArgLocs[i];
    SDValue Arg           = OutVals[i];
    ISD::ArgFlagsTy Flags = Outs[i].Flags;

    // Nothing should need promotion
    // Promote the value if needed.
    /*
    switch (VA.getLocInfo()) {
      default: llvm_unreachable("Unknown loc info!");
      case CCValAssign::Full: break;
      case CCValAssign::SExt:
        assert(Outs[i].VT == MVT::i1 && "Extending non i1 type");
        Arg = DAG.getNode(ISD::SIGN_EXTEND, dl, VA.getLocVT(), Arg);
        break;
      case CCValAssign::ZExt:
        assert(Outs[i].VT == MVT::i1 && "Extending non i1 type");
        Arg = DAG.getNode(ISD::ZERO_EXTEND, dl, VA.getLocVT(), Arg);
        break;
      case CCValAssign::AExt:
        assert(Outs[i].VT == MVT::i1 && "Extending non i1 type");
        Arg = DAG.getNode(ISD::ANY_EXTEND, dl, VA.getLocVT(), Arg);
        break;
    }
    */

    // Arguments that can be passed on register must be kept at
    // RegsToPass vector
    if (VA.isRegLoc()) {
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
      continue;
    }

    // Register can't get to this point...
    assert(VA.isMemLoc());

    // emit ISD::STORE whichs stores the
    // parameter value to a stack Location
    unsigned LocMemOffset = VA.getLocMemOffset();
    SDValue PtrOff        = DAG.getIntPtrConstant(LocMemOffset, dl);
    PtrOff = DAG.getNode(ISD::ADD, dl, getPointerTy(DAG.getDataLayout()),
                         StackPtr, PtrOff);
    if (Flags.isByVal()) {
      SDValue SizeNode = DAG.getConstant(Flags.getByValSize(), dl, MVT::i32);
      MemOpChains.push_back(DAG.getMemcpy(
        Chain, dl, PtrOff, Arg, SizeNode, Flags.getByValAlign(),
        /*isVolatile=*/false, /*AlwaysInline=*/false, /*isTailCall=*/false,
        MachinePointerInfo(), MachinePointerInfo()));
    } else {
      MemOpChains.push_back(
        DAG.getStore(Chain, dl, Arg, PtrOff,
                     MachinePointerInfo::getStack(MF, LocMemOffset)));
    }
  }

  // Transform all store nodes into one single node because all store
  // nodes are independent of each other.
  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, MemOpChains);

  // Build a sequence of copy-to-reg nodes chained together with token chain
  // and flag operands which copy the outgoing args into the appropriate regs.
  // The InFlag in necessary since all emited instructions must be stuck
  // together.
  SDValue InFlag;
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
    Chain  = DAG.getCopyToReg(Chain, dl, RegsToPass[i].first,
                             RegsToPass[i].second, InFlag);
    InFlag = Chain.getValue(1);
  }

  // If the callee is a GlobalAddress/ExternalSymbol node (quite common, every
  // direct call is) turn it into a TargetGlobalAddress/TargetExternalSymbol
  // node so that legalize doesn't hack it.
  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee)) {
    const GlobalValue *GV = G->getGlobal();
    Callee =
      DAG.getTargetGlobalAddress(GV, dl, getPointerTy(DAG.getDataLayout()));
  } else if (ExternalSymbolSDNode *S = dyn_cast<ExternalSymbolSDNode>(Callee)) {
    const char *Sym = S->getSymbol();
    Callee =
      DAG.getTargetExternalSymbol(Sym, getPointerTy(DAG.getDataLayout()));
  }

  // MipsJmpLink = #chain, #target_address, #opt_in_flags...
  //             = Chain, Callee, Reg#1, Reg#2, ...
  //
  // Returns a chain & a flag for retval copy to use.
  std::vector<SDValue> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);

  // Add argument registers to the end of the list so that they are known live
  // into the call.
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i)
    Ops.push_back(DAG.getRegister(RegsToPass[i].first,
                                  RegsToPass[i].second.getValueType()));

  if (InFlag.getNode())
    Ops.push_back(InFlag);

  if (isTailCall)
    return DAG.getNode(CSAISD::TailCall, dl, MVT::Other, Ops);

  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  Chain            = DAG.getNode(CSAISD::Call, dl, NodeTys, Ops);
  InFlag           = Chain.getValue(1);

  // Create the CALLSEQ_END node.
  if (!csa_utils::isAlwaysDataFlowLinkageSet())
  Chain = DAG.getCALLSEQ_END(Chain, DAG.getIntPtrConstant(NumBytes, dl, true),
                             DAG.getIntPtrConstant(0, dl, true), InFlag, dl);
  if (!Ins.empty())
    InFlag = Chain.getValue(1);

  // Handle result values, copying them out of physregs into vregs that we
  // return.
  return LowerCallResult(Chain, InFlag, CallConv, isVarArg, Ins, dl, DAG,
                         InVals);
}

/// LowerCallResult - Lower the result values of a call into the
/// appropriate copies out of appropriate physical registers.
SDValue CSATargetLowering::LowerCallResult(
  SDValue Chain, SDValue InFlag, CallingConv::ID CallConv, bool isVarArg,
  const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &dl, SelectionDAG &DAG,
  SmallVectorImpl<SDValue> &InVals) const {

  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  if (csa_utils::isAlwaysDataFlowLinkageSet())
	  CCInfo.AnalyzeCallResult(Ins, RetCC_LIC_CSA);
  else
  CCInfo.AnalyzeCallResult(Ins, RetCC_Reg_CSA);

  // Copy all of the result registers out of their specified physreg.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    CCValAssign VA = RVLocs[i];
    SDValue Val =
      DAG.getCopyFromReg(Chain, dl, VA.getLocReg(), VA.getValVT(), InFlag);
    Chain  = Val.getValue(1);
    InFlag = Val.getValue(2);

    InVals.push_back(Val);
  }

  return Chain;
}

//===----------------------------------------------------------------------===//
//             Formal Arguments Calling Convention Implementation
//===----------------------------------------------------------------------===//

/// LowerFormalArguments - transform physical registers into virtual registers
/// and generate load operations for arguments places on the stack.
SDValue CSATargetLowering::LowerFormalArguments(
  SDValue Chain, CallingConv::ID CallConv, bool isVarArg,
  const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &dl, SelectionDAG &DAG,
  SmallVectorImpl<SDValue> &InVals) const {

  MachineFunction &MF         = DAG.getMachineFunction();
  MachineFrameInfo &MFI       = MF.getFrameInfo();
  CSAMachineFunctionInfo *UFI = MF.getInfo<CSAMachineFunctionInfo>();

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());

  if (csa_utils::isAlwaysDataFlowLinkageSet())
	  CCInfo.AnalyzeFormalArguments(Ins, CC_LIC_CSA);
  else
  CCInfo.AnalyzeFormalArguments(Ins, CC_Reg_CSA);

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];

    // Arguments stored on registers
    if (VA.isRegLoc()) {
      // EVT RegVT = VA.getLocVT();

      bool isDF = (csa_utils::isAlwaysDataFlowLinkageSet());
      // Transform the arguments stored in physical registers into virtual ones
      const TargetRegisterClass *tClass = isDF ? &CSA::CI64RegClass : &CSA::RI64RegClass;
      MVT tVT                           = VA.getValVT();
      if (tVT == MVT::i64) {
        tClass = isDF ? &CSA::CI64RegClass : &CSA::I64RegClass;
      } else if (tVT == MVT::i32) {
        tClass = isDF ? &CSA::CI32RegClass : &CSA::I32RegClass;
      } else if (tVT == MVT::i16) {
        tClass = isDF ? &CSA::CI16RegClass : &CSA::I16RegClass;
      } else if (tVT == MVT::i8) {
        tClass = isDF ? &CSA::CI8RegClass : &CSA::I8RegClass;
      } else if (tVT == MVT::i1) {
        tClass = isDF ? &CSA::CI1RegClass : &CSA::I1RegClass;
      } else if (tVT == MVT::f64) {
        tClass = isDF ? &CSA::CI64RegClass : &CSA::I64RegClass;
      } else if (tVT == MVT::f32) {
        tClass = isDF ? &CSA::CI32RegClass : &CSA::I32RegClass;
      } else if (tVT == MVT::v2f32) {
        tClass = isDF ? &CSA::CI64RegClass : &CSA::I64RegClass;
      } else {
        llvm_unreachable("WTC!!");
      }
      unsigned Reg     = MF.addLiveIn(VA.getLocReg(), tClass);
      SDValue ArgValue = DAG.getCopyFromReg(Chain, dl, Reg, VA.getValVT());

      InVals.push_back(ArgValue);
    } else {
      // sanity check
      assert(VA.isMemLoc());
      ISD::ArgFlagsTy flags = Ins[VA.getValNo()].Flags;
      if (flags.isByVal()) {
        // Argument passed directly on the stack. We don't need to load it
        // NOTE: I *finally* figured out what the +8. It is just some non-zero
        // number to make sure that VA.getLocMemOffset() + 8 is always
        // *non-null* and therefore always negative as this is used to determine
        // the offset of the FrameIndex in 'eliminateFrameIndex'. The +8 and
        // negative are removed then...
        int FI = MFI.CreateFixedObject(
          flags.getByValSize(), -((int64_t)VA.getLocMemOffset() + 8LL), true);
        InVals.push_back(
          DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout())));
      } else {
        int FI =
          MFI.CreateFixedObject(VA.getLocVT().getSizeInBits() / 8,
                                -((int64_t)VA.getLocMemOffset() + 8LL), true);
        SDValue FIN = DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout()));
        // Create load to retrieve the argument from the stack
        InVals.push_back(
          DAG.getLoad(VA.getValVT(), dl, Chain, FIN,
                      MachinePointerInfo::getFixedStack(MF, FI, 0)));
      }
      /*
      unsigned ArgSize = VA.getLocVT().getSizeInBits()/8;
      int64_t Offset = VA.getLocMemOffset() + 8;
      int FI = MFI.CreateFixedObject(ArgSize, -Offset, true);

      // Create load nodes to retrieve arguments from the stack
      SDValue FIN = DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout()));
      InVals.push_back(
        DAG.getLoad(VA.getValVT(), dl, Chain, FIN,
                    MachinePointerInfo::getFixedStack(FI, 0));
      */
    }
  }

  // varargs
  if (isVarArg) {
    // This will point to the next argument passed via stack.
    unsigned ArgOffset = CCInfo.getNextStackOffset() + 8;
    UFI->setVarArgsFrameIndex(MFI.CreateFixedObject(8, -ArgOffset, true));
  }

  return Chain;
}

//===----------------------------------------------------------------------===//
//               Return Value Calling Convention Implementation
//===----------------------------------------------------------------------===//

SDValue
CSATargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CallConv,
                               bool isVarArg,
                               const SmallVectorImpl<ISD::OutputArg> &Outs,
                               const SmallVectorImpl<SDValue> &OutVals,
                               const SDLoc &dl, SelectionDAG &DAG) const {

  // CCValAssign - represent the assignment of the return value to a location
  SmallVector<CCValAssign, 16> RVLocs;

  // CCState - Info about the registers and stack slot.
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  // Analize return values.
  if (csa_utils::isAlwaysDataFlowLinkageSet())
	  CCInfo.AnalyzeReturn(Outs, RetCC_LIC_CSA);
  else
  CCInfo.AnalyzeReturn(Outs, RetCC_Reg_CSA);
  SDValue Flag;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  // Copy the result values into the output registers.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(), OutVals[i], Flag);

    // guarantee that all emitted copies are
    // stuck together, avoiding something bad
    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain; // Update chain.

  // Add the flag if we have it.
  if (Flag.getNode())
    RetOps.push_back(Flag);

  // Return is always a "ret %ra"
  return DAG.getNode(CSAISD::Ret, dl, MVT::Other, RetOps);
}

TargetLowering::AtomicExpansionKind
CSATargetLowering::shouldExpandAtomicRMWInIR(AtomicRMWInst *AI) const {
  unsigned NativeWidth = 64;
  Type *MemType = AI->getType();

  // If the operand is too big, resort to library calls.
  if (MemType->getPrimitiveSizeInBits() > NativeWidth) {
    return AtomicExpansionKind::None;
  }

  AtomicRMWInst::BinOp Op = AI->getOperation();
  switch (Op) {
  default:
    llvm_unreachable("Unknown atomic operation");
  case AtomicRMWInst::Xchg:
  case AtomicRMWInst::Add:
  case AtomicRMWInst::Sub:
  case AtomicRMWInst::Or:
  case AtomicRMWInst::And:
  case AtomicRMWInst::Xor:
  case AtomicRMWInst::Nand:
  case AtomicRMWInst::Max:
  case AtomicRMWInst::Min:
  case AtomicRMWInst::UMax:
  case AtomicRMWInst::UMin:
    // Use a cmpxchg loop if we don't have native atomic RMW ops
    return Subtarget.hasRMWAtomic() ? AtomicExpansionKind::None : AtomicExpansionKind::CmpXChg;
  }
}
/*
SDValue
CSATargetLowering::getReturnAddressFrameIndex(SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  CSAMachineFunctionInfo *FuncInfo = MF.getInfo<CSAMachineFunctionInfo>();
  int ReturnAddrIndex = FuncInfo->getRAIndex();

  if (ReturnAddrIndex == 0) {
    // Set up a frame object for the return address.
    uint64_t SlotSize = getDataLayout()->getPointerSize();
    ReturnAddrIndex = MF.getFrameInfo()->CreateFixedObject(SlotSize, -SlotSize,
                                                           true);
    FuncInfo->setRAIndex(ReturnAddrIndex);
  }

  return DAG.getFrameIndex(ReturnAddrIndex, getPointerTy(DAG.getDataLayout()));
}

SDValue CSATargetLowering::LowerRETURNADDR(SDValue Op,
                                              SelectionDAG &DAG) const {
  MachineFrameInfo &MFI = DAG.getMachineFunction().getFrameInfo();
  MFI.setReturnAddressIsTaken(true);

  if (verifyReturnAddressArgumentIsConstant(Op, DAG))
    return SDValue();

  unsigned Depth = cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue();
  SDLoc dl(Op);

  if (Depth > 0) {
    SDValue FrameAddr = LowerFRAMEADDR(Op, DAG);
    SDValue Offset =
        DAG.getConstant(getDataLayout()->getPointerSize(), MVT::i16);
    return DAG.getLoad(getPointerTy(DAG.getDataLayout()), dl,
DAG.getEntryNode(), DAG.getNode(ISD::ADD, dl, getPointerTy(DAG.getDataLayout()),
                                   FrameAddr, Offset),
                       MachinePointerInfo());
  }

  // Just load the return address.
  SDValue RetAddrFI = getReturnAddressFrameIndex(DAG);
  return DAG.getLoad(getPointerTy(DAG.getDataLayout()), dl, DAG.getEntryNode(),
                     RetAddrFI, MachinePointerInfo());
}

SDValue CSATargetLowering::LowerFRAMEADDR(SDValue Op,
                                             SelectionDAG &DAG) const {
  MachineFrameInfo &MFI = DAG.getMachineFunction().getFrameInfo();
  MFI.setFrameAddressIsTaken(true);

  EVT VT = Op.getValueType();
  SDLoc dl(Op);  // FIXME probably not meaningful
  unsigned Depth = cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue();
  SDValue FrameAddr = DAG.getCopyFromReg(DAG.getEntryNode(), dl,
                                         CSA::FP, VT);
  while (Depth--)
    FrameAddr = DAG.getLoad(VT, dl, DAG.getEntryNode(), FrameAddr,
                            MachinePointerInfo());
  return FrameAddr;
}

SDValue CSATargetLowering::LowerVASTART(SDValue Op,
                                           SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  CSAMachineFunctionInfo *FuncInfo = MF.getInfo<CSAMachineFunctionInfo>();

  // Frame index of first vararg argument
  SDValue FrameIndex = DAG.getFrameIndex(FuncInfo->getVarArgsFrameIndex(),
                                         getPointerTy(DAG.getDataLayout()));
  const Value *SV = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();

  // Create a store of the frame index to the location operand
  return DAG.getStore(Op.getOperand(0), SDLoc(Op), FrameIndex,
                      Op.getOperand(1), MachinePointerInfo(SV));
}

*/

//===----------------------------------------------------------------------===//
//  Other Lowering Code
//===----------------------------------------------------------------------===//
