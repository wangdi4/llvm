//===-- LPUISelLowering.cpp - LPU DAG Lowering Implementation  ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the LPUTargetLowering class.
//
//===----------------------------------------------------------------------===//

#include "LPUISelLowering.h"
#include "LPU.h"
#include "LPUMachineFunctionInfo.h"
#include "LPUSubtarget.h"
#include "LPUTargetMachine.h"
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

#define DEBUG_TYPE "lpu-lower"

STATISTIC(NumTailCalls, "Number of tail calls");

static cl::opt<bool>
EnableLPUTailCalls("enable-lpu-tail-calls",
                    cl::desc("LPU: Enable tail calls."));

LPUTargetLowering::LPUTargetLowering(const TargetMachine &TM)
    : TargetLowering(TM) {

  const LPUSubtarget& ST = TM.getSubtarget<LPUSubtarget>();

  // Set up the register classes.
  // The actual allocation should depend on the context (serial vs. parallel)
  addRegisterClass(MVT::i1,   &LPU::I1RegClass);
  addRegisterClass(MVT::i8,   &LPU::I8RegClass);
  addRegisterClass(MVT::i16,  &LPU::I16RegClass);
  addRegisterClass(MVT::i32,  &LPU::I32RegClass);
  addRegisterClass(MVT::i64,  &LPU::I64RegClass);
  addRegisterClass(MVT::f16,  &LPU::I16RegClass);
  addRegisterClass(MVT::f32,  &LPU::I32RegClass);
  addRegisterClass(MVT::f64,  &LPU::I64RegClass);

  setBooleanContents(ZeroOrOneBooleanContent);
  setBooleanVectorContents(ZeroOrOneBooleanContent); // FIXME: Is this correct?

  // Compute derived properties from the register classes
  setStackPointerRegisterToSaveRestore(LPU::SP);
  computeRegisterProperties();

  // Scheduling shouldn't be relevant
  // setSchedulingPreference(Sched::ILP);

  // Provide all sorts of operation actions

  // Division is expensive
  setIntDivIsCheap(false);

  // Operations we want expanded for all types
  for (MVT VT : MVT::integer_valuetypes()) {
    // If this type is generally supported
    bool isTypeSupported =
      ( (VT == MVT::i1 && ST.hasI1()) ||
        (VT == MVT::i8 && ST.hasI8()) ||
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
    setOperationAction(ISD::SMUL_LOHI,        VT,    Expand);
    setOperationAction(ISD::UMUL_LOHI,        VT,    Expand);
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

    // Implement these?
    LegalizeAction action = (isTypeSupported && ST.hasBitOp()) ? Legal : Expand;
    setOperationAction(ISD::CTPOP,            VT,    action);
    setOperationAction(ISD::CTTZ,             VT,    action);
    setOperationAction(ISD::CTTZ_ZERO_UNDEF,  VT,    action);
    setOperationAction(ISD::CTLZ,             VT,    action);
    setOperationAction(ISD::CTLZ_ZERO_UNDEF,  VT,    action);

    setOperationAction(ISD::DYNAMIC_STACKALLOC,VT,   Expand);
  }

  for (MVT VT : MVT::fp_valuetypes()) {
    setOperationAction(ISD::BR_CC,            VT,    Expand);
    setOperationAction(ISD::SELECT_CC,        VT,    Expand);
  }

  // Loads
  for (MVT VT : MVT::integer_valuetypes()) {
    setLoadExtAction(ISD::EXTLOAD,  VT, MVT::i1,  Promote);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i1,  Promote);
    setLoadExtAction(ISD::ZEXTLOAD, VT, MVT::i1,  Promote);

    // for now (likely revisit)
    setLoadExtAction(ISD::EXTLOAD, VT, MVT::i8,  Expand);
    setLoadExtAction(ISD::ZEXTLOAD, VT, MVT::i8,  Expand);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i8,  Expand);
    setLoadExtAction(ISD::EXTLOAD, VT, MVT::i16, Expand);
    setLoadExtAction(ISD::ZEXTLOAD, VT, MVT::i16, Expand);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i16, Expand);
    setLoadExtAction(ISD::EXTLOAD, VT, MVT::i32, Expand);
    setLoadExtAction(ISD::ZEXTLOAD, VT, MVT::i32, Expand);
    setLoadExtAction(ISD::SEXTLOAD, VT, MVT::i32, Expand);
  }

  setLoadExtAction(ISD::EXTLOAD, MVT::f32, MVT::f16, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::f64, MVT::f16, Expand);
  setLoadExtAction(ISD::EXTLOAD, MVT::f64, MVT::f32, Expand);

  // We don't accept any truncstore of integer registers.
  setTruncStoreAction(MVT::i64, MVT::i32, Expand);
  setTruncStoreAction(MVT::i64, MVT::i16, Expand);
  setTruncStoreAction(MVT::i64, MVT::i8 , Expand);
  setTruncStoreAction(MVT::i32, MVT::i16, Expand);
  setTruncStoreAction(MVT::i32, MVT::i8 , Expand);
  setTruncStoreAction(MVT::i16, MVT::i8,  Expand);

  setTruncStoreAction(MVT::i64, MVT::i1,  Promote);
  setTruncStoreAction(MVT::i32, MVT::i1,  Promote);
  setTruncStoreAction(MVT::i16, MVT::i1,  Promote);
  setTruncStoreAction(MVT::i8,  MVT::i1,  Promote);

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
    //setOperationAction(ISD::FCOPYSIGN,  MVT::f32, Legal);
    //setOperationAction(ISD::FCOPYSIGN,  MVT::f64, Legal);
    //setOperationAction(ISD::FGETSIGN,  MVT::f32, Legal);
    //setOperationAction(ISD::FGETSIGN,  MVT::f64, Legal);
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

EVT LPUTargetLowering::getSetCCResultType(LLVMContext &Context, EVT VT) const {
  return MVT::i1;
}

SDValue LPUTargetLowering::LowerOperation(SDValue Op,
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
  default:
    llvm_unreachable("unimplemented operand");
  }
}

const char *LPUTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  default: return nullptr;
  case LPUISD::Call:               return "LPUISD::Call";
  case LPUISD::TailCall:           return "LPUISD::TailCall";
  case LPUISD::Ret:                return "LPUISD::Ret";
  case LPUISD::Wrapper:            return "LPUISD::Wrapper";
  }
}


SDValue LPUTargetLowering::LowerGlobalAddress(SDValue Op,
                                                 SelectionDAG &DAG) const {
  const GlobalValue *GV = cast<GlobalAddressSDNode>(Op)->getGlobal();
  int64_t Offset = cast<GlobalAddressSDNode>(Op)->getOffset();

  // Create the TargetGlobalAddress node, folding in the constant offset.
  SDValue Result = DAG.getTargetGlobalAddress(GV, SDLoc(Op), getPointerTy(),
                                              Offset);
  return DAG.getNode(LPUISD::Wrapper, SDLoc(Op), getPointerTy(), Result);
}

SDValue LPUTargetLowering::LowerExternalSymbol(SDValue Op,
                                                  SelectionDAG &DAG) const {
  const char *Sym = cast<ExternalSymbolSDNode>(Op)->getSymbol();
  SDValue Result = DAG.getTargetExternalSymbol(Sym, getPointerTy());
  return DAG.getNode(LPUISD::Wrapper, SDLoc(Op), getPointerTy(), Result);
}

SDValue LPUTargetLowering::LowerBlockAddress(SDValue Op,
                                                SelectionDAG &DAG) const {
  const BlockAddress *BA = cast<BlockAddressSDNode>(Op)->getBlockAddress();
  SDValue Result =  DAG.getTargetBlockAddress(BA, getPointerTy());
  return DAG.getNode(LPUISD::Wrapper, SDLoc(Op), getPointerTy(), Result);
}

SDValue LPUTargetLowering::LowerJumpTable(SDValue Op,
                                             SelectionDAG &DAG) const {
  JumpTableSDNode *JT = cast<JumpTableSDNode>(Op);
  SDValue Result = DAG.getTargetJumpTable(JT->getIndex(), getPointerTy());
  return DAG.getNode(LPUISD::Wrapper, SDLoc(JT), getPointerTy(), Result);
}

//===----------------------------------------------------------------------===//
//                       LPU Inline Assembly Support
//===----------------------------------------------------------------------===//

/*
/// getConstraintType - Given a constraint letter, return the type of
/// constraint it is for this target.
TargetLowering::ConstraintType
LPUTargetLowering::getConstraintType(const std::string &Constraint) const {
  if (Constraint.size() == 1) {
    switch (Constraint[0]) {
    case 'r':
      return C_RegisterClass;
    default:
      break;
    }
  }
  return TargetLowering::getConstraintType(Constraint);
}

std::pair<unsigned, const TargetRegisterClass*>
LPUTargetLowering::
getRegForInlineAsmConstraint(const std::string &Constraint,
                             MVT VT) const {
  if (Constraint.size() == 1) {
    // GCC Constraint Letters
    switch (Constraint[0]) {
    default: break;
    case 'r':   // GENERAL_REGS
      if (VT == MVT::i8)
        return std::make_pair(0U, &LPU::GR8RegClass);

      return std::make_pair(0U, &LPU::GR16RegClass);
    }
  }

  return TargetLowering::getRegForInlineAsmConstraint(Constraint, VT);
}
*/

bool LPUTargetLowering::isTruncateFree(EVT VT1, EVT VT2) const {
  if (!VT1.isInteger() || !VT2.isInteger())
    return false;
  unsigned NumBits1 = VT1.getSizeInBits();
  unsigned NumBits2 = VT2.getSizeInBits();
  return NumBits1 > NumBits2;
}

bool LPUTargetLowering::isZExtFree(Type *Ty1, Type *Ty2) const {
  // Using a small op only references the relevant bits
  return true;
}

bool LPUTargetLowering::isZExtFree(EVT VT1, EVT VT2) const {
  // Using a small op only references the relevant bits
  return true;
}

bool LPUTargetLowering::isZExtFree(SDValue Val, EVT VT2) const {
  EVT VT1 = Val.getValueType();
  return (isZExtFree(VT1, VT2));
}

bool LPUTargetLowering::isNarrowingProfitable(EVT VT1, EVT VT2) const {
  return true;
}


//===----------------------------------------------------------------------===//
//                      Calling Convention Implementation
//===----------------------------------------------------------------------===//

#include "LPUGenCallingConv.inc"

/// IsEligibleForTailCallOptimization - Check whether the call is eligible
/// for tail call optimization.
bool LPUTargetLowering::
IsEligibleForTailCallOptimization(unsigned NextStackOffset,
                                  const LPUMachineFunctionInfo& FI) const {
    //  if (!EnableLPUTailCalls)
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
LPUTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
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
        Result = CC_Reg_LPU(i, ArgVT, ArgVT, CCValAssign::Full, ArgFlags,
                            CCInfo);
      }
      else {
        Result = CC_Reg_VarArg_LPU(i, ArgVT, ArgVT, CCValAssign::Full,
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
    CCInfo.AnalyzeCallOperands(Outs, CC_Reg_LPU);
  }

  // Get a count of how many bytes are to be pushed on the stack.
  unsigned NumBytes = CCInfo.getNextStackOffset();

  if (isTailCall)
    isTailCall = !isVarArg &&
      IsEligibleForTailCallOptimization(NumBytes,
                                        *MF.getInfo<LPUMachineFunctionInfo>());

  if (isTailCall)
    ++NumTailCalls;

  if (!isTailCall)
    Chain = DAG.getCALLSEQ_START(Chain, DAG.getIntPtrConstant(NumBytes, true), dl);

  SDValue StackPtr = DAG.getCopyFromReg(Chain, dl, LPU::SP, getPointerTy());

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
    SDValue PtrOff = DAG.getIntPtrConstant(LocMemOffset);
    PtrOff = DAG.getNode(ISD::ADD, dl, getPointerTy(), StackPtr, PtrOff);
    if (Flags.isByVal()) {
      SDValue SizeNode = DAG.getConstant(Flags.getByValSize(), MVT::i32);
      MemOpChains.push_back(
        DAG.getMemcpy(Chain, dl, PtrOff, Arg, SizeNode, Flags.getByValAlign(),
                      /*isVolatile=*/false, /*AlwaysInline=*/false,
                      MachinePointerInfo(), MachinePointerInfo()));
    } else {
      MemOpChains.push_back(
        DAG.getStore(Chain, dl, Arg, PtrOff,
                     MachinePointerInfo::getStack(LocMemOffset),
                     false, false, 0));
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
    Callee = DAG.getTargetGlobalAddress(GV, dl, getPointerTy());
  } else if (ExternalSymbolSDNode *S = dyn_cast<ExternalSymbolSDNode>(Callee)) {
    const char *Sym = S->getSymbol();
    Callee = DAG.getTargetExternalSymbol(Sym, getPointerTy());
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
      return DAG.getNode(LPUISD::TailCall, dl, MVT::Other, Ops);

  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  Chain  = DAG.getNode(LPUISD::Call, dl, NodeTys, Ops);
  InFlag = Chain.getValue(1);

  // Create the CALLSEQ_END node.
  Chain = DAG.getCALLSEQ_END(Chain, DAG.getIntPtrConstant(NumBytes, true),
                             DAG.getIntPtrConstant(0, true), InFlag, dl);
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
LPUTargetLowering::LowerCallResult(SDValue Chain, SDValue InFlag,
                                    CallingConv::ID CallConv, bool isVarArg,
                                    const SmallVectorImpl<ISD::InputArg> &Ins,
                                    SDLoc dl, SelectionDAG &DAG,
                                    SmallVectorImpl<SDValue> &InVals) const {

  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 RVLocs, *DAG.getContext());

  CCInfo.AnalyzeCallResult(Ins, RetCC_Reg_LPU);

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
LPUTargetLowering::LowerFormalArguments(SDValue Chain,
                                        CallingConv::ID CallConv, bool isVarArg,
                                        const SmallVectorImpl<ISD::InputArg>
                                        &Ins,
                                        SDLoc dl, SelectionDAG &DAG,
                                        SmallVectorImpl<SDValue> &InVals)
                                          const {

  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo *MFI = MF.getFrameInfo();
  LPUMachineFunctionInfo *UFI = MF.getInfo<LPUMachineFunctionInfo>();

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 ArgLocs, *DAG.getContext());

  CCInfo.AnalyzeFormalArguments(Ins, CC_Reg_LPU);

  SDValue StackPtr;

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];

    // Arguments stored on registers
    if (VA.isRegLoc()) {
      //EVT RegVT = VA.getLocVT();

      // Transform the arguments stored in physical registers into virtual ones
      const TargetRegisterClass *tClass = &LPU::RI64RegClass;
      MVT tVT = VA.getValVT();
      if(tVT == MVT::i64) {
        tClass = &LPU::RI64RegClass;
      } else if(tVT == MVT::i32) {
        tClass = &LPU::RI32RegClass;
      } else if(tVT == MVT::i16) {
        tClass = &LPU::RI16RegClass;
      } else if(tVT == MVT::i8) {
        tClass = &LPU::RI8RegClass;
      } else if(tVT == MVT::i1) {
        tClass = &LPU::RI1RegClass;
      } else if(tVT == MVT::f64) {
        tClass = &LPU::RI64RegClass;
      } else if(tVT == MVT::f32) {
        tClass = &LPU::RI32RegClass;
      } else if(tVT == MVT::f16) {
        tClass = &LPU::RI16RegClass;
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
        int FI = MFI->CreateFixedObject(flags.getByValSize(), -((int64_t)VA.getLocMemOffset() + 8LL), true);
        InVals.push_back(DAG.getFrameIndex(FI, getPointerTy()));
      } else {
        int FI = MFI->CreateFixedObject(VA.getLocVT().getSizeInBits()/8, -((int64_t)VA.getLocMemOffset() + 8LL), true);
        SDValue FIN = DAG.getFrameIndex(FI, getPointerTy());
        // Create load to retrieve the argument from the stack
        InVals.push_back(DAG.getLoad(VA.getValVT(), dl, Chain, FIN,
	    MachinePointerInfo::getFixedStack(FI, 0), false, false, false, 0));
      }
      /*
      unsigned ArgSize = VA.getLocVT().getSizeInBits()/8;
      int64_t Offset = VA.getLocMemOffset() + 8;
      int FI = MFI->CreateFixedObject(ArgSize, -Offset, true);

      // Create load nodes to retrieve arguments from the stack
      SDValue FIN = DAG.getFrameIndex(FI, getPointerTy());
      InVals.push_back(
        DAG.getLoad(VA.getValVT(), dl, Chain, FIN,
                    MachinePointerInfo::getFixedStack(FI, 0),
                    false, false, false, 0));
      */
    }
  }

  // varargs
  if (isVarArg) {
    // This will point to the next argument passed via stack.
    unsigned ArgOffset = CCInfo.getNextStackOffset() + 8;
    UFI->setVarArgsFrameIndex(MFI->CreateFixedObject(8, -ArgOffset, true));
  }

  return Chain;
}

//===----------------------------------------------------------------------===//
//               Return Value Calling Convention Implementation
//===----------------------------------------------------------------------===//

SDValue
LPUTargetLowering::LowerReturn(SDValue Chain,
                                CallingConv::ID CallConv, bool isVarArg,
                                const SmallVectorImpl<ISD::OutputArg> &Outs,
                                const SmallVectorImpl<SDValue> &OutVals,
                                SDLoc dl, SelectionDAG &DAG) const {

  // CCValAssign - represent the assignment of the return value to a location
  SmallVector<CCValAssign, 16> RVLocs;

  // CCState - Info about the registers and stack slot.
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(),
                 RVLocs, *DAG.getContext());

  // Analize return values.
  CCInfo.AnalyzeReturn(Outs, RetCC_Reg_LPU);

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
  return DAG.getNode(LPUISD::Ret, dl, MVT::Other, RetOps);
}

/*
SDValue
LPUTargetLowering::getReturnAddressFrameIndex(SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  LPUMachineFunctionInfo *FuncInfo = MF.getInfo<LPUMachineFunctionInfo>();
  int ReturnAddrIndex = FuncInfo->getRAIndex();

  if (ReturnAddrIndex == 0) {
    // Set up a frame object for the return address.
    uint64_t SlotSize = getDataLayout()->getPointerSize();
    ReturnAddrIndex = MF.getFrameInfo()->CreateFixedObject(SlotSize, -SlotSize,
                                                           true);
    FuncInfo->setRAIndex(ReturnAddrIndex);
  }

  return DAG.getFrameIndex(ReturnAddrIndex, getPointerTy());
}

SDValue LPUTargetLowering::LowerRETURNADDR(SDValue Op,
                                              SelectionDAG &DAG) const {
  MachineFrameInfo *MFI = DAG.getMachineFunction().getFrameInfo();
  MFI->setReturnAddressIsTaken(true);

  if (verifyReturnAddressArgumentIsConstant(Op, DAG))
    return SDValue();

  unsigned Depth = cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue();
  SDLoc dl(Op);

  if (Depth > 0) {
    SDValue FrameAddr = LowerFRAMEADDR(Op, DAG);
    SDValue Offset =
        DAG.getConstant(getDataLayout()->getPointerSize(), MVT::i16);
    return DAG.getLoad(getPointerTy(), dl, DAG.getEntryNode(),
                       DAG.getNode(ISD::ADD, dl, getPointerTy(),
                                   FrameAddr, Offset),
                       MachinePointerInfo(), false, false, false, 0);
  }

  // Just load the return address.
  SDValue RetAddrFI = getReturnAddressFrameIndex(DAG);
  return DAG.getLoad(getPointerTy(), dl, DAG.getEntryNode(),
                     RetAddrFI, MachinePointerInfo(), false, false, false, 0);
}

SDValue LPUTargetLowering::LowerFRAMEADDR(SDValue Op,
                                             SelectionDAG &DAG) const {
  MachineFrameInfo *MFI = DAG.getMachineFunction().getFrameInfo();
  MFI->setFrameAddressIsTaken(true);

  EVT VT = Op.getValueType();
  SDLoc dl(Op);  // FIXME probably not meaningful
  unsigned Depth = cast<ConstantSDNode>(Op.getOperand(0))->getZExtValue();
  SDValue FrameAddr = DAG.getCopyFromReg(DAG.getEntryNode(), dl,
                                         LPU::FP, VT);
  while (Depth--)
    FrameAddr = DAG.getLoad(VT, dl, DAG.getEntryNode(), FrameAddr,
                            MachinePointerInfo(),
                            false, false, false, 0);
  return FrameAddr;
}

SDValue LPUTargetLowering::LowerVASTART(SDValue Op,
                                           SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  LPUMachineFunctionInfo *FuncInfo = MF.getInfo<LPUMachineFunctionInfo>();

  // Frame index of first vararg argument
  SDValue FrameIndex = DAG.getFrameIndex(FuncInfo->getVarArgsFrameIndex(),
                                         getPointerTy());
  const Value *SV = cast<SrcValueSDNode>(Op.getOperand(2))->getValue();

  // Create a store of the frame index to the location operand
  return DAG.getStore(Op.getOperand(0), SDLoc(Op), FrameIndex,
                      Op.getOperand(1), MachinePointerInfo(SV),
                      false, false, 0);
}

*/

//===----------------------------------------------------------------------===//
//  Other Lowering Code
//===----------------------------------------------------------------------===//

