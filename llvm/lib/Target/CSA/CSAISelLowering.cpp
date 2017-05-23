//===-- CSAISelLowering.cpp - CSA DAG Lowering Implementation  ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
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

static cl::opt<bool>
EnableCSATailCalls("enable-csa-tail-calls",
                    cl::desc("CSA: Enable tail calls."));

CSATargetLowering::CSATargetLowering(const TargetMachine &TM, const CSASubtarget &ST)
    : TargetLowering(TM), Subtarget(ST) {

  // Set up the register classes.
  // The actual allocation should depend on the context (serial vs. parallel)
  addRegisterClass(MVT::i1,   &CSA::I1RegClass);
  addRegisterClass(MVT::i8,   &CSA::I8RegClass);
  addRegisterClass(MVT::i16,  &CSA::I16RegClass);
  addRegisterClass(MVT::i32,  &CSA::I32RegClass);
  addRegisterClass(MVT::i64,  &CSA::I64RegClass);
  addRegisterClass(MVT::f16,  &CSA::I16RegClass);
  addRegisterClass(MVT::f32,  &CSA::I32RegClass);
  addRegisterClass(MVT::f64,  &CSA::I64RegClass);

  // always lower memset, memcpy, and memmove intrinsics to load/store
  // instructions, rather
  // then generating calls to memset, mempcy or memmove.
  MaxStoresPerMemset = (unsigned) 0xFFFFFFFF;
  MaxStoresPerMemcpy = (unsigned) 0xFFFFFFFF;
  MaxStoresPerMemmove = (unsigned) 0xFFFFFFFF;
  
  setBooleanContents(ZeroOrOneBooleanContent);
  setBooleanVectorContents(ZeroOrOneBooleanContent); // FIXME: Is this correct?

  // Compute derived properties from the register classes
  setStackPointerRegisterToSaveRestore(CSA::SP);
  computeRegisterProperties(ST.getRegisterInfo());

  // Scheduling shouldn't be relevant
  // setSchedulingPreference(Sched::ILP);

  // Provide all sorts of operation actions

  // Operations we want expanded for all types
  for (MVT VT : MVT::integer_valuetypes()) {
    // If this type is generally supported
    bool isTypeSupported =
      ( (VT == MVT::i8 && ST.hasI8()) ||
        (VT == MVT::i16 && ST.hasI16()) ||
        (VT == MVT::i32 && ST.hasI32()) ||
        (VT == MVT::i64 && ST.hasI64()) );

    setOperationAction(ISD::BR_CC,            VT,    Expand);
    setOperationAction(ISD::SELECT_CC,        VT,    Expand);

    setOperationAction(ISD::SIGN_EXTEND,      VT,    Expand);

    // Arithmetic
    setOperationAction(ISD::ADDC,             VT,    Expand);
    setOperationAction(ISD::ADDE,             VT,    Expand);
    setOperationAction(ISD::SUBC,             VT,    Expand);
    setOperationAction(ISD::SUBE,             VT,    Expand);
    // Note: {U,S}MUL_LOHI must be Custom selected because TableGen cannot cope
    // with multi-output selection. This is a known weakness.
    setOperationAction(ISD::SMUL_LOHI,        VT,    Custom);
    setOperationAction(ISD::UMUL_LOHI,        VT,    Custom);
    setOperationAction(ISD::MULHS,            VT,    Expand);
    setOperationAction(ISD::MULHU,            VT,    Expand);
    setOperationAction(ISD::UREM,             VT,    Expand);
    setOperationAction(ISD::SREM,             VT,    Expand);
    setOperationAction(ISD::UDIVREM,          VT,    Expand);
    setOperationAction(ISD::SDIVREM,          VT,    Expand);

    setOperationAction(ISD::SHL_PARTS,        VT,    Expand);
    setOperationAction(ISD::SRL_PARTS,        VT,    Expand);
    setOperationAction(ISD::SRA_PARTS,        VT,    Expand);

    // Bit manipulation
    setOperationAction(ISD::ROTL,             VT,    Expand);
    setOperationAction(ISD::ROTR,             VT,    Expand);
    setOperationAction(ISD::BSWAP,            VT,    Expand);

    LegalizeAction action = (isTypeSupported && ST.hasBitOp()) ? Legal : Expand;
    setOperationAction(ISD::CTPOP,            VT,    action);
    setOperationAction(ISD::CTTZ,             VT,    action);
    setOperationAction(ISD::CTTZ_ZERO_UNDEF,  VT,    Expand);
    setOperationAction(ISD::CTLZ,             VT,    action);
    setOperationAction(ISD::CTLZ_ZERO_UNDEF,  VT,    Expand);

    // Atomic operations
    setOperationAction(ISD::ATOMIC_LOAD_AND,  VT,    Legal);
    setOperationAction(ISD::ATOMIC_LOAD_ADD,  VT,    Legal);
    setOperationAction(ISD::ATOMIC_LOAD_MIN,  VT,    Legal);
    setOperationAction(ISD::ATOMIC_LOAD_MAX,  VT,    Legal);
    setOperationAction(ISD::ATOMIC_LOAD_OR,   VT,    Legal);
    setOperationAction(ISD::ATOMIC_LOAD_XOR,  VT,    Legal);
    setOperationAction(ISD::ATOMIC_SWAP,      VT,    Legal);
    setOperationAction(ISD::ATOMIC_CMP_SWAP,  VT,    Legal);

    setOperationAction(ISD::ATOMIC_LOAD,      VT,    Custom);
    setOperationAction(ISD::ATOMIC_STORE,     VT,    Custom);

    setOperationAction(ISD::DYNAMIC_STACKALLOC,VT,   Expand);
  }

  setOperationAction(ISD::STACKSAVE,          MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE,       MVT::Other, Expand);

  for (MVT VT : MVT::fp_valuetypes()) {
    setOperationAction(ISD::BR_CC,            VT,    Expand);
    setOperationAction(ISD::SELECT_CC,        VT,    Expand);
  }

  setOperationAction(ISD::BR_JT,              MVT::Other, Expand);
  
  // Loads & stores
  for (MVT VT : MVT::integer_valuetypes()) {
    //  i1 used to be promote, now expand...
    setLoadExtAction(ISD::EXTLOAD,  VT, MVT::i1,  Promote);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i1,  Promote);
    setLoadExtAction(ISD::ZEXTLOAD, VT, MVT::i1,  Promote);

    // for now (likely revisit)
    setLoadExtAction(ISD::EXTLOAD , VT, MVT::i8,  Expand);
    setLoadExtAction(ISD::ZEXTLOAD, VT, MVT::i8,  Expand);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i8,  Expand);
    setLoadExtAction(ISD::EXTLOAD,  VT, MVT::i16, Expand);
    setLoadExtAction(ISD::ZEXTLOAD, VT, MVT::i16, Expand);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i16, Expand);
    setLoadExtAction(ISD::EXTLOAD,  VT, MVT::i32, Expand);
    setLoadExtAction(ISD::ZEXTLOAD, VT, MVT::i32, Expand);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i32, Expand);

    // We don't accept any truncstore of integer registers.
    setTruncStoreAction(VT, MVT::i32, Expand);
    setTruncStoreAction(VT, MVT::i16, Expand);
    setTruncStoreAction(VT, MVT::i8 , Expand);
    setTruncStoreAction(VT, MVT::i1,  Expand);
  }

  setLoadExtAction(ISD::EXTLOAD, MVT::f32, MVT::f16, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::f64, MVT::f16, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::f64, MVT::f32, Expand);

  setTruncStoreAction(MVT::f64, MVT::f32, Expand);
  setTruncStoreAction(MVT::f64, MVT::f16, Expand);
  setTruncStoreAction(MVT::f32, MVT::f16, Expand);
  
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
  setOperationAction(ISD::ConstantFP, MVT::f16, Legal);
  setOperationAction(ISD::ConstantFP, MVT::f32, Legal);
  setOperationAction(ISD::ConstantFP, MVT::f64, Legal);

/*  These are to enable as CG work is done
  // Short float
  setOperationAction(ISD::FP16_TO_FP, MVT::f32, Expand);
  setOperationAction(ISD::FP_TO_FP16, MVT::f32, Expand);
*/

  if (ST.hasF16()) {
    setOperationAction(ISD::FADD, MVT::f16, Legal);
    setOperationAction(ISD::FSUB, MVT::f16, Legal);
    setOperationAction(ISD::FMUL, MVT::f16, Legal);
    if (ST.hasFMA()) {
      setOperationAction(ISD::FMA,  MVT::f16, Legal);
    }
    setOperationAction(ISD::FDIV, MVT::f16, Legal);
    setOperationAction(ISD::FREM, MVT::f16, Expand);
    setOperationAction(ISD::FNEG, MVT::f16, Legal);
    setOperationAction(ISD::FABS, MVT::f16, Legal);
  }

  setOperationAction(ISD::FNEG,  MVT::f32, Legal);
  setOperationAction(ISD::FNEG,  MVT::f64, Legal);
  setOperationAction(ISD::FABS,  MVT::f32, Legal);
  setOperationAction(ISD::FABS,  MVT::f64, Legal);

  // Allow various FP operations (temporarily.)
  // The intent is these will be provided via a math library
  if (ST.hasMath0()) {
    // Order from ISDOpcodes.h
    //setOperationAction(ISD::FREM,  MVT::f32, Legal);
    //setOperationAction(ISD::FREM,  MVT::f64, Legal);
    setOperationAction(ISD::FCOPYSIGN,  MVT::f32, Expand);
    setOperationAction(ISD::FCOPYSIGN,  MVT::f64, Expand);
    if (ST.hasF16())
      setOperationAction(ISD::FSQRT, MVT::f16, Legal);
    setOperationAction(ISD::FSQRT, MVT::f32, Legal);
    setOperationAction(ISD::FSQRT, MVT::f64, Legal);
    setOperationAction(ISD::FSIN,  MVT::f32, Legal);
    setOperationAction(ISD::FSIN,  MVT::f64, Legal);
    setOperationAction(ISD::FCOS,  MVT::f32, Legal);
    setOperationAction(ISD::FCOS,  MVT::f64, Legal);
    //setOperationAction(ISD::FPOWI, MVT::f32, Legal);
    //setOperationAction(ISD::FPOWI, MVT::f64, Legal);
    setOperationAction(ISD::FPOW,  MVT::f32, Legal);
    setOperationAction(ISD::FPOW,  MVT::f64, Legal);
    setOperationAction(ISD::FLOG,  MVT::f32, Legal);
    setOperationAction(ISD::FLOG,  MVT::f64, Legal);
    setOperationAction(ISD::FLOG2, MVT::f32, Legal);
    setOperationAction(ISD::FLOG2, MVT::f64, Legal);
    //setOperationAction(ISD::FLOG10,MVT::f32, Legal);
    //setOperationAction(ISD::FLOG10,MVT::f64, Legal);
    setOperationAction(ISD::FEXP,  MVT::f32, Legal);
    setOperationAction(ISD::FEXP,  MVT::f64, Legal);
    setOperationAction(ISD::FEXP2, MVT::f32, Legal);
    setOperationAction(ISD::FEXP2, MVT::f64, Legal);
    //setOperationAction(ISD::FCEIL, MVT::f32, Legal);
    //setOperationAction(ISD::FCEIL, MVT::f64, Legal);
    //setOperationAction(ISD::FTRUNC,MVT::f32, Legal);
    //setOperationAction(ISD::FTRUNC,MVT::f64, Legal);
    //setOperationAction(ISD::FRINT, MVT::f32, Legal);
    //setOperationAction(ISD::FRINT, MVT::f64, Legal);
    //setOperationAction(ISD::FNEARBYINT,MVT::f32, Legal);
    //setOperationAction(ISD::FNEARBYINT,MVT::f64, Legal);
    //setOperationAction(ISD::FROUND,MVT::f32, Legal);
    //setOperationAction(ISD::FROUND,MVT::f64, Legal);
    //setOperationAction(ISD::FFLOOR,MVT::f32, Legal);
    //setOperationAction(ISD::FFLOOR,MVT::f64, Legal);
    //setOperationAction(ISD::FMINNUM, MVT::f32, Legal);
    //setOperationAction(ISD::FMINNUM, MVT::f64, Legal);
    //setOperationAction(ISD::FMAXNUM, MVT::f32, Legal);
    //setOperationAction(ISD::FMAXNUM, MVT::f64, Legal);
    //setOperationAction(ISD::FSINCOS, MVT::f32, Legal);
    //setOperationAction(ISD::FSINCOS, MVT::f64, Legal);
  }

  setOperationAction(ISD::GlobalAddress,    MVT::i64,   Custom);
  setOperationAction(ISD::ExternalSymbol,   MVT::i64,   Custom);
  setOperationAction(ISD::BlockAddress,     MVT::i64,   Custom);
  setOperationAction(ISD::JumpTable,        MVT::i64,   Custom);

  //  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1,   Expand);

  // varargs support
  setOperationAction(ISD::VASTART,          MVT::Other, Expand);
  setOperationAction(ISD::VAARG,            MVT::Other, Expand);
  setOperationAction(ISD::VAEND,            MVT::Other, Expand);
  setOperationAction(ISD::VACOPY,           MVT::Other, Expand);

  //setOperationAction(ISD::READCYCLECOUNTER,   MVT::i64,   Legal);
}

EVT CSATargetLowering::getSetCCResultType(const DataLayout &DL, LLVMContext &Context, EVT VT) const {
  return MVT::i1;
}

SDValue CSATargetLowering::LowerOperation(SDValue Op,
                                             SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  case ISD::GlobalAddress:    return LowerGlobalAddress(Op, DAG);
  case ISD::ExternalSymbol:   return LowerExternalSymbol(Op, DAG);
  case ISD::BlockAddress:     return LowerBlockAddress(Op, DAG);
  case ISD::JumpTable:        return LowerJumpTable(Op, DAG);
    /*
  case ISD::RETURNADDR:       return LowerRETURNADDR(Op, DAG);
  case ISD::FRAMEADDR:        return LowerFRAMEADDR(Op, DAG);
  case ISD::VASTART:          return LowerVASTART(Op, DAG);
    */
  case ISD::ATOMIC_LOAD:      return LowerAtomicLoad(Op, DAG);
  case ISD::ATOMIC_STORE:     return LowerAtomicStore(Op, DAG);
  case ISD::SMUL_LOHI:        return LowerMUL_LOHI(Op, DAG);
  case ISD::UMUL_LOHI:        return LowerMUL_LOHI(Op, DAG);
  default:
    llvm_unreachable("unimplemented operand");
  }
}

const char *CSATargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  default: return nullptr;
  case CSAISD::Call:               return "CSAISD::Call";
  case CSAISD::TailCall:           return "CSAISD::TailCall";
  case CSAISD::Ret:                return "CSAISD::Ret";
  case CSAISD::Wrapper:            return "CSAISD::Wrapper";
  }
}


SDValue CSATargetLowering::LowerGlobalAddress(SDValue Op,
                                                 SelectionDAG &DAG) const {
  const GlobalValue *GV = cast<GlobalAddressSDNode>(Op)->getGlobal();
  int64_t Offset = cast<GlobalAddressSDNode>(Op)->getOffset();

  // Create the TargetGlobalAddress node
  // DO NOT fold in the constant offset for now
  SDValue Result = DAG.getTargetGlobalAddress(GV, SDLoc(Op),
                                              getPointerTy(DAG.getDataLayout()),
                                              0);
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
  SDValue Result = DAG.getTargetExternalSymbol(Sym,
                                          getPointerTy(DAG.getDataLayout()));
  return DAG.getNode(CSAISD::Wrapper, SDLoc(Op),
                     getPointerTy(DAG.getDataLayout()), Result);
}

SDValue CSATargetLowering::LowerBlockAddress(SDValue Op,
                                                SelectionDAG &DAG) const {
  const BlockAddress *BA = cast<BlockAddressSDNode>(Op)->getBlockAddress();
  SDValue Result = DAG.getTargetBlockAddress(BA,
                                        getPointerTy(DAG.getDataLayout()));
  return DAG.getNode(CSAISD::Wrapper, SDLoc(Op),
                                getPointerTy(DAG.getDataLayout()), Result);
}

SDValue CSATargetLowering::LowerJumpTable(SDValue Op,
                                             SelectionDAG &DAG) const {
  JumpTableSDNode *JT = cast<JumpTableSDNode>(Op);
  SDValue Result = DAG.getTargetJumpTable(JT->getIndex(),
                                    getPointerTy(DAG.getDataLayout()));
  return DAG.getNode(CSAISD::Wrapper, SDLoc(JT),
                            getPointerTy(DAG.getDataLayout()), Result);
}

SDValue CSATargetLowering::LowerAtomicLoad(SDValue Op,
                                             SelectionDAG &DAG) const {
    AtomicSDNode *AL = cast<AtomicSDNode>(Op);
    // Sufficiently small regular loads are already atomic.
    // If this is a large load, don't lower here. Probably the right thing to
    // do is to use AtomicExpandPass to take care of it.
    if(AL->getMemoryVT().getStoreSizeInBits() > 64) {
        return Op;
    }

    SDLoc DL(Op);
    return DAG.getLoad(AL->getMemoryVT(),
            DL,
            AL->getChain(),
            AL->getBasePtr(),
            AL->getPointerInfo(),
            AL->getAlignment());
}

SDValue CSATargetLowering::LowerAtomicStore(SDValue Op,
                                             SelectionDAG &DAG) const {
    AtomicSDNode *AS = cast<AtomicSDNode>(Op);
    // Sufficiently small regular stores are already atomic.
    if(AS->getMemoryVT().getStoreSizeInBits() > 64) {
        return Op;
    }

    SDLoc DL(Op);
    return DAG.getStore(
            AS->getChain(),
            DL,
            AS->getVal(),
            AS->getBasePtr(),
            AS->getPointerInfo(),
            AS->getAlignment());
}

SDValue CSATargetLowering::
LowerMUL_LOHI(SDValue Op, SelectionDAG &DAG) const
{
  SDLoc dl(Op);
  SDValue LHS = Op.getOperand(0);
  SDValue RHS = Op.getOperand(1);
  MVT partVT = LHS.getSimpleValueType();
  SDValue InOps[] = { LHS, RHS };
  bool isSigned = (Op.getNode()->getOpcode() == ISD::SMUL_LOHI);

  unsigned opcode;
  switch(partVT.SimpleTy)
  {
    case MVT::i8:
      opcode = isSigned ? CSA::MULLOHIS8 : CSA::MULLOHIU8;
      break;
    case MVT::i16:
      opcode = isSigned ? CSA::MULLOHIS16 : CSA::MULLOHIU16;
      break;
    case MVT::i32:
      opcode = isSigned ? CSA::MULLOHIS32 : CSA::MULLOHIU32;
      break;
    case MVT::i64:
      opcode = isSigned ? CSA::MULLOHIS64 : CSA::MULLOHIU64;
      break;
    default:
      return Op;
  }

  SDNode *Mullohi = DAG.getMachineNode(opcode, dl,
                           DAG.getVTList(partVT, partVT), InOps);
  SDValue Lo(Mullohi, 0);
  SDValue Hi(Mullohi, 1);
  SDValue Ops[] = { Lo, Hi };
  return DAG.getMergeValues(Ops, dl);
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
  return Ty1->isIntegerTy(32) && Ty2->isIntegerTy(64);  //&& Subtarget->is64Bit()
}

bool CSATargetLowering::isZExtFree(EVT VT1, EVT VT2) const {
  // x86-64 implicitly zero-extends 32-bit results in 64-bit registers.
  return VT1 == MVT::i32 && VT2 == MVT::i64;  // && Subtarget->is64Bit()
}

bool CSATargetLowering::isZExtFree(SDValue Val, EVT VT2) const {
  EVT VT1 = Val.getValueType();
  if (isZExtFree(VT1, VT2))
    return true;

  if (Val.getOpcode() != ISD::LOAD)
    return false;

  if (!VT1.isSimple() || !VT1.isInteger() ||
    !VT2.isSimple() || !VT2.isInteger())
    return false;

  switch (VT1.getSimpleVT().SimpleTy) {
  default: break;
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

// isLegalAddressingMode - Return true if the addressing mode represented
// by AM is legal for this target, for a load/store of the specified type.
bool CSATargetLowering::isLegalAddressingMode(const DataLayout &DL,
                                              const AddrMode &AM,
                                              Type *Ty,
                                              unsigned AddrSpace) const {
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
  default:  // Other stuff never works.
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
bool CSATargetLowering::
IsEligibleForTailCallOptimization(unsigned NextStackOffset,
                                  const CSAMachineFunctionInfo& FI) const {
    //  if (!EnableCSATailCalls)
    //    return false;

  // Return false if either the callee or caller has a byval argument.
  //  if (FI.hasByValArg())
  //  return false;

  // Return true if the callee's argument area is no larger than the
  // caller's.
  //return NextStackOffset <= FI.getIncomingArgSize();
  return true;
}

/// LowerCall - functions arguments are copied from virtual regs to
/// (physical regs)/(stack frame), CALLSEQ_START and CALLSEQ_END are emitted.
/// TODO: isTailCall.
SDValue
CSATargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
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
      MVT ArgVT = Outs[i].VT;
      ISD::ArgFlagsTy ArgFlags = Outs[i].Flags;
      bool Result;

      if (Outs[i].IsFixed) {
        Result = CC_Reg_CSA(i, ArgVT, ArgVT, CCValAssign::Full, ArgFlags,
                            CCInfo);
      }
      else {
        Result = CC_Reg_VarArg_CSA(i, ArgVT, ArgVT, CCValAssign::Full,
                                   ArgFlags, CCInfo);
      }

      if (Result) {
        #ifndef NDEBUG
        dbgs() << "Call operand #" << i << " has unhandled type "
               << EVT(ArgVT).getEVTString();
        #endif
        llvm_unreachable(0);
      }
    }
  }
  else {
    // All arguments are treated the same.
    CCInfo.AnalyzeCallOperands(Outs, CC_Reg_CSA);
  }

  // Get a count of how many bytes are to be pushed on the stack.
  unsigned NumBytes = CCInfo.getNextStackOffset();

  if (isTailCall)
    isTailCall = !isVarArg &&
      IsEligibleForTailCallOptimization(NumBytes,
                                        *MF.getInfo<CSAMachineFunctionInfo>());

  if (isTailCall)
    ++NumTailCalls;

  if (!isTailCall)
    Chain = DAG.getCALLSEQ_START(Chain, DAG.getIntPtrConstant(NumBytes, dl, true), dl);

  SDValue StackPtr = DAG.getCopyFromReg(Chain, dl, CSA::SP,
                                        getPointerTy(DAG.getDataLayout()));

  SmallVector<std::pair<unsigned, SDValue>, 8> RegsToPass;
  SmallVector<SDValue, 8> MemOpChains;

  // Walk the register/memloc assignments, inserting copies/loads.
  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign    &VA    = ArgLocs[i];
    SDValue         Arg   = OutVals[i];
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
    SDValue PtrOff = DAG.getIntPtrConstant(LocMemOffset, dl);
    PtrOff = DAG.getNode(ISD::ADD, dl, getPointerTy(DAG.getDataLayout()),
        StackPtr, PtrOff);
    if (Flags.isByVal()) {
      SDValue SizeNode = DAG.getConstant(Flags.getByValSize(), dl, MVT::i32);
      MemOpChains.push_back(
        DAG.getMemcpy(Chain, dl, PtrOff, Arg, SizeNode, Flags.getByValAlign(),
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
    Chain = DAG.getCopyToReg(Chain, dl, RegsToPass[i].first,
                             RegsToPass[i].second, InFlag);
    InFlag = Chain.getValue(1);
  }

  // If the callee is a GlobalAddress/ExternalSymbol node (quite common, every
  // direct call is) turn it into a TargetGlobalAddress/TargetExternalSymbol
  // node so that legalize doesn't hack it.
  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee)) {
    const GlobalValue *GV = G->getGlobal();
    Callee = DAG.getTargetGlobalAddress(GV, dl,
        getPointerTy(DAG.getDataLayout()));
  } else if (ExternalSymbolSDNode *S = dyn_cast<ExternalSymbolSDNode>(Callee)) {
    const char *Sym = S->getSymbol();
    Callee = DAG.getTargetExternalSymbol(Sym,
        getPointerTy(DAG.getDataLayout()));
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
  Chain  = DAG.getNode(CSAISD::Call, dl, NodeTys, Ops);
  InFlag = Chain.getValue(1);

  // Create the CALLSEQ_END node.
  Chain = DAG.getCALLSEQ_END(Chain, DAG.getIntPtrConstant(NumBytes, dl, true),
                             DAG.getIntPtrConstant(0, dl, true), InFlag, dl);
  if (!Ins.empty())
    InFlag = Chain.getValue(1);

  // Handle result values, copying them out of physregs into vregs that we
  // return.
  return LowerCallResult(Chain, InFlag, CallConv, isVarArg, Ins,
                         dl, DAG, InVals);
}

/// LowerCallResult - Lower the result values of a call into the
/// appropriate copies out of appropriate physical registers.
SDValue
CSATargetLowering::LowerCallResult(SDValue Chain, SDValue InFlag,
                                    CallingConv::ID CallConv, bool isVarArg,
                                    const SmallVectorImpl<ISD::InputArg> &Ins,
                                    const SDLoc &dl, SelectionDAG &DAG,
                                    SmallVectorImpl<SDValue> &InVals) const {

  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 RVLocs, *DAG.getContext());

  CCInfo.AnalyzeCallResult(Ins, RetCC_Reg_CSA);

  // Copy all of the result registers out of their specified physreg.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    CCValAssign VA = RVLocs[i];
    SDValue Val     = DAG.getCopyFromReg(Chain, dl, VA.getLocReg(),
                                         VA.getValVT(), InFlag);
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
SDValue
CSATargetLowering::LowerFormalArguments(SDValue Chain,
                                        CallingConv::ID CallConv, bool isVarArg,
                                        const SmallVectorImpl<ISD::InputArg>
                                        &Ins,
                                        const SDLoc &dl, SelectionDAG &DAG,
                                        SmallVectorImpl<SDValue> &InVals)
                                          const {

  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  CSAMachineFunctionInfo *UFI = MF.getInfo<CSAMachineFunctionInfo>();

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 ArgLocs, *DAG.getContext());

  CCInfo.AnalyzeFormalArguments(Ins, CC_Reg_CSA);

  SDValue StackPtr;

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];

    // Arguments stored on registers
    if (VA.isRegLoc()) {
      //EVT RegVT = VA.getLocVT();

      // Transform the arguments stored in physical registers into virtual ones
      const TargetRegisterClass *tClass = &CSA::RI64RegClass;
      MVT tVT = VA.getValVT();
      if(tVT == MVT::i64) {
        tClass = &CSA::RI64RegClass;
      } else if(tVT == MVT::i32) {
        tClass = &CSA::RI32RegClass;
      } else if(tVT == MVT::i16) {
        tClass = &CSA::RI16RegClass;
      } else if(tVT == MVT::i8) {
        tClass = &CSA::RI8RegClass;
      } else if(tVT == MVT::i1) {
        tClass = &CSA::RI1RegClass;
      } else if(tVT == MVT::f64) {
        tClass = &CSA::RI64RegClass;
      } else if(tVT == MVT::f32) {
        tClass = &CSA::RI32RegClass;
      } else if(tVT == MVT::f16) {
        tClass = &CSA::RI16RegClass;
      } else {
        llvm_unreachable("WTC!!");
      }
      unsigned Reg = MF.addLiveIn(VA.getLocReg(), tClass);
      SDValue ArgValue = DAG.getCopyFromReg(Chain, dl, Reg, VA.getValVT());

      InVals.push_back(ArgValue);
    } else {
      // sanity check
      assert(VA.isMemLoc());
      ISD::ArgFlagsTy flags = Ins[VA.getValNo()].Flags;
      if(flags.isByVal()) {
        // Argument passed directly on the stack. We don't need to load it
        // NOTE: I *finally* figured out what the +8. It is just some non-zero number to make
        // sure that VA.getLocMemOffset() + 8 is always *non-null* and therefore always negative
        // as this is used to determine the offset of the FrameIndex in 'eliminateFrameIndex'. The +8
        // and negative are removed then...
        int FI = MFI.CreateFixedObject(flags.getByValSize(), -((int64_t)VA.getLocMemOffset() + 8LL), true);
        InVals.push_back(DAG.getFrameIndex(FI,
              getPointerTy(DAG.getDataLayout())));
      } else {
        int FI = MFI.CreateFixedObject(VA.getLocVT().getSizeInBits()/8, -((int64_t)VA.getLocMemOffset() + 8LL), true);
        SDValue FIN = DAG.getFrameIndex(FI, getPointerTy(DAG.getDataLayout()));
        // Create load to retrieve the argument from the stack
        InVals.push_back(DAG.getLoad(VA.getValVT(), dl, Chain, FIN,
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
CSATargetLowering::LowerReturn(SDValue Chain,
                                CallingConv::ID CallConv, bool isVarArg,
                                const SmallVectorImpl<ISD::OutputArg> &Outs,
                                const SmallVectorImpl<SDValue> &OutVals,
                                const SDLoc &dl, SelectionDAG &DAG) const {

  // CCValAssign - represent the assignment of the return value to a location
  SmallVector<CCValAssign, 16> RVLocs;

  // CCState - Info about the registers and stack slot.
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 RVLocs, *DAG.getContext());

  // Analize return values.
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

  RetOps[0] = Chain;  // Update chain.

  // Add the flag if we have it.
  if (Flag.getNode())
    RetOps.push_back(Flag);

  // Return is always a "ret %ra"
  return DAG.getNode(CSAISD::Ret, dl, MVT::Other, RetOps);
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
    return DAG.getLoad(getPointerTy(DAG.getDataLayout()), dl, DAG.getEntryNode(),
                       DAG.getNode(ISD::ADD, dl, getPointerTy(DAG.getDataLayout()),
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

