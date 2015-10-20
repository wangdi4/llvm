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

EVT LPUTargetLowering::getSetCCResultType(LLVMContext &Context, EVT VT) const {
  return MVT::i1;
}

const char *LPUTargetLowering::getTargetNodeName(unsigned Opcode) const {
  switch (Opcode) {
  default: return nullptr;
  case LPUISD::Call:               return "LPUISD::Call";
  case LPUISD::TailCall:           return "LPUISD::TailCall";
  case LPUISD::Ret:                return "LPUISD::Ret";
  case LPUISD::ThreadPointer:      return "LPUISD::ThreadPointer";
  }
}

LPUTargetLowering::LPUTargetLowering(const TargetMachine &TM)
    : TargetLowering(TM) {

  const LPUSubtarget& ST = TM.getSubtarget<LPUSubtarget>();

  // Set up the register classes.
  // The actual allocation should depend on the context (serial vs. parallel)
  addRegisterClass(MVT::i1,   &LPU::I1RRegClass);
  addRegisterClass(MVT::i8,   &LPU::I8RRegClass);
  addRegisterClass(MVT::i16,  &LPU::I16RRegClass);
  addRegisterClass(MVT::i32,  &LPU::I32RRegClass);
  addRegisterClass(MVT::i64,  &LPU::I64RRegClass);
  addRegisterClass(MVT::f16,  &LPU::F16RRegClass);
  addRegisterClass(MVT::f32,  &LPU::F32RRegClass);
  addRegisterClass(MVT::f64,  &LPU::F64RRegClass);

  // Compute derived properties from the register classes
  computeRegisterProperties();

  // Provide all sorts of operation actions

  // Division is expensive
  setIntDivIsCheap(false);

//  setStackPointerRegisterToSaveRestore(LPU::SP);
  setBooleanContents(ZeroOrOneBooleanContent);
  setBooleanVectorContents(ZeroOrOneBooleanContent); // FIXME: Is this correct?

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
    setOperationAction(ISD::UDIVREM,          VT,    Expand);
    setOperationAction(ISD::SDIVREM,          VT,    Expand);

    setOperationAction(ISD::SHL_PARTS,        VT,    Expand);
    setOperationAction(ISD::SRL_PARTS,        VT,    Expand);
    setOperationAction(ISD::SRA_PARTS,        VT,    Expand);

    // Bit manipulation
    setOperationAction(ISD::ROTL,             VT,    Expand);
    setOperationAction(ISD::ROTR,             VT,    Expand);
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

  // We don't accept any truncstore of integer registers.
  setTruncStoreAction(MVT::i64, MVT::i32, Expand);
  setTruncStoreAction(MVT::i64, MVT::i16, Expand);
  setTruncStoreAction(MVT::i64, MVT::i8 , Expand);
  setTruncStoreAction(MVT::i32, MVT::i16, Expand);
  setTruncStoreAction(MVT::i32, MVT::i8 , Expand);
  setTruncStoreAction(MVT::i16, MVT::i8,  Expand);

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
    setOperationAction(ISD::FSQRT, MVT::f32, Legal);
    setOperationAction(ISD::FSQRT, MVT::f64, Legal);
    setOperationAction(ISD::FSIN,  MVT::f32, Legal);
    setOperationAction(ISD::FSIN,  MVT::f64, Legal);
    setOperationAction(ISD::FCOS,  MVT::f32, Legal);
    setOperationAction(ISD::FCOS,  MVT::f64, Legal);
    //setOperationAction(ISD::FPOWI, MVT::f32, Legal);
    //setOperationAction(ISD::FPOWI, MVT::f64, Legal);
    //setOperationAction(ISD::FPOW,  MVT::f32, Legal);
    //setOperationAction(ISD::FPOW,  MVT::f64, Legal);
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

  //  setOperationAction(ISD::GlobalAddress,    MVT::i16,   Custom);
  //  setOperationAction(ISD::ExternalSymbol,   MVT::i16,   Custom);
  //  setOperationAction(ISD::BlockAddress,     MVT::i16,   Custom);

  //  setOperationAction(ISD::SIGN_EXTEND_INREG, MVT::i1,   Expand);

  /*
  // varargs support
  setOperationAction(ISD::VASTART,          MVT::Other, Expand);
  setOperationAction(ISD::VAARG,            MVT::Other, Expand);
  setOperationAction(ISD::VAEND,            MVT::Other, Expand);
  setOperationAction(ISD::VACOPY,           MVT::Other, Expand);
  //  setOperationAction(ISD::JumpTable,        MVT::i16,   Expand);
  */
}

/*
SDValue LPUTargetLowering::LowerOperation(SDValue Op,
                                             SelectionDAG &DAG) const {
  switch (Op.getOpcode()) {
  case ISD::SHL: // FALLTHROUGH
  case ISD::SRL:
  case ISD::SRA:              return LowerShifts(Op, DAG);
  case ISD::GlobalAddress:    return LowerGlobalAddress(Op, DAG);
  case ISD::BlockAddress:     return LowerBlockAddress(Op, DAG);
  case ISD::ExternalSymbol:   return LowerExternalSymbol(Op, DAG);
  case ISD::SETCC:            return LowerSETCC(Op, DAG);
  case ISD::BR_CC:            return LowerBR_CC(Op, DAG);
  case ISD::SELECT_CC:        return LowerSELECT_CC(Op, DAG);
  case ISD::SIGN_EXTEND:      return LowerSIGN_EXTEND(Op, DAG);
  case ISD::RETURNADDR:       return LowerRETURNADDR(Op, DAG);
  case ISD::FRAMEADDR:        return LowerFRAMEADDR(Op, DAG);
  case ISD::VASTART:          return LowerVASTART(Op, DAG);
  case ISD::JumpTable:        return LowerJumpTable(Op, DAG);
  default:
    llvm_unreachable("unimplemented operand");
  }
}

//===----------------------------------------------------------------------===//
//                       LPU Inline Assembly Support
//===----------------------------------------------------------------------===//

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

bool LPUTargetLowering::isLegalICmpImmediate(int64_t Imm) const {
  return true;
}

bool LPUTargetLowering::isLegalAddImmediate(int64_t Imm) const {
  return true;
}

bool LPUTargetLowering::isNarrowingProfitable(EVT VT1, EVT VT2) const {
  return true;
}


//===----------------------------------------------------------------------===//
//                      Calling Convention Implementation
//===----------------------------------------------------------------------===//

#include "LPUGenCallingConv.inc"
/*
/// For each argument in a function store the number of pieces it is composed
/// of.
template<typename ArgT>
static void ParseFunctionArgs(const SmallVectorImpl<ArgT> &Args,
                              SmallVectorImpl<unsigned> &Out) {
  unsigned CurrentArgIndex = ~0U;
  for (unsigned i = 0, e = Args.size(); i != e; i++) {
    if (CurrentArgIndex == Args[i].OrigArgIndex) {
      Out.back()++;
    } else {
      Out.push_back(1);
      CurrentArgIndex++;
    }
  }
}

static void AnalyzeVarArgs(CCState &State,
                           const SmallVectorImpl<ISD::OutputArg> &Outs) {
  State.AnalyzeCallOperands(Outs, CC_LPU_AssignStack);
}

static void AnalyzeVarArgs(CCState &State,
                           const SmallVectorImpl<ISD::InputArg> &Ins) {
  State.AnalyzeFormalArguments(Ins, CC_LPU_AssignStack);
}

/// Analyze incoming and outgoing function arguments. We need custom C++ code
/// to handle special constraints in the ABI like reversing the order of the
/// pieces of splitted arguments. In addition, all pieces of a certain argument
/// have to be passed either using registers or the stack but never mixing both.
template<typename ArgT>
static void AnalyzeArguments(CCState &State,
                             SmallVectorImpl<CCValAssign> &ArgLocs,
                             const SmallVectorImpl<ArgT> &Args) {
  static const MCPhysReg RegList[] = {
    LPU::R15, LPU::R14, LPU::R13, LPU::R12
  };
  static const unsigned NbRegs = array_lengthof(RegList);

  if (State.isVarArg()) {
    AnalyzeVarArgs(State, Args);
    return;
  }

  SmallVector<unsigned, 4> ArgsParts;
  ParseFunctionArgs(Args, ArgsParts);

  unsigned RegsLeft = NbRegs;
  bool UseStack = false;
  unsigned ValNo = 0;

  for (unsigned i = 0, e = ArgsParts.size(); i != e; i++) {
    MVT ArgVT = Args[ValNo].VT;
    ISD::ArgFlagsTy ArgFlags = Args[ValNo].Flags;
    MVT LocVT = ArgVT;
    CCValAssign::LocInfo LocInfo = CCValAssign::Full;

    // Promote i8 to i16
    if (LocVT == MVT::i8) {
      LocVT = MVT::i16;
      if (ArgFlags.isSExt())
          LocInfo = CCValAssign::SExt;
      else if (ArgFlags.isZExt())
          LocInfo = CCValAssign::ZExt;
      else
          LocInfo = CCValAssign::AExt;
    }

    // Handle byval arguments
    if (ArgFlags.isByVal()) {
      State.HandleByVal(ValNo++, ArgVT, LocVT, LocInfo, 2, 2, ArgFlags);
      continue;
    }

    unsigned Parts = ArgsParts[i];

    if (!UseStack && Parts <= RegsLeft) {
      unsigned FirstVal = ValNo;
      for (unsigned j = 0; j < Parts; j++) {
        unsigned Reg = State.AllocateReg(RegList, NbRegs);
        State.addLoc(CCValAssign::getReg(ValNo++, ArgVT, Reg, LocVT, LocInfo));
        RegsLeft--;
      }

      // Reverse the order of the pieces to agree with the "big endian" format
      // required in the calling convention ABI.
      SmallVectorImpl<CCValAssign>::iterator B = ArgLocs.begin() + FirstVal;
      std::reverse(B, B + Parts);
    } else {
      UseStack = true;
      for (unsigned j = 0; j < Parts; j++)
        CC_LPU_AssignStack(ValNo++, ArgVT, LocVT, LocInfo, ArgFlags, State);
    }
  }
}
*/

SDValue
LPUTargetLowering::LowerFormalArguments(SDValue Chain,
                                           CallingConv::ID CallConv,
                                           bool isVarArg,
                                           const SmallVectorImpl<ISD::InputArg>
                                             &Ins,
                                           SDLoc dl,
                                           SelectionDAG &DAG,
                                           SmallVectorImpl<SDValue> &InVals)
                                             const {

  switch (CallConv) {
  default:
    llvm_unreachable("Unsupported calling convention");
  case CallingConv::C:
  case CallingConv::Fast:
    return LowerCCCArguments(Chain, CallConv, isVarArg, Ins, dl, DAG, InVals);
  }
}
/*
SDValue
LPUTargetLowering::LowerCall(TargetLowering::CallLoweringInfo &CLI,
                                SmallVectorImpl<SDValue> &InVals) const {
  SelectionDAG &DAG                     = CLI.DAG;
  SDLoc &dl                             = CLI.DL;
  SmallVectorImpl<ISD::OutputArg> &Outs = CLI.Outs;
  SmallVectorImpl<SDValue> &OutVals     = CLI.OutVals;
  SmallVectorImpl<ISD::InputArg> &Ins   = CLI.Ins;
  SDValue Chain                         = CLI.Chain;
  SDValue Callee                        = CLI.Callee;
  bool &isTailCall                      = CLI.IsTailCall;
  CallingConv::ID CallConv              = CLI.CallConv;
  bool isVarArg                         = CLI.IsVarArg;

  // LPU target does not yet support tail call optimization.
  isTailCall = false;

  switch (CallConv) {
  default:
    llvm_unreachable("Unsupported calling convention");
  case CallingConv::Fast:
  case CallingConv::C:
    return LowerCCCCallTo(Chain, Callee, CallConv, isVarArg, isTailCall,
                          Outs, OutVals, Ins, dl, DAG, InVals);
  case CallingConv::LPU_INTR:
    report_fatal_error("ISRs cannot be called directly");
  }
}
*/
/// LowerCCCArguments - transform physical registers into virtual registers and
/// generate load operations for arguments places on the stack.
// FIXME: struct return stuff
SDValue
LPUTargetLowering::LowerCCCArguments(SDValue Chain,
                                        CallingConv::ID CallConv,
                                        bool isVarArg,
                                        const SmallVectorImpl<ISD::InputArg>
                                          &Ins,
                                        SDLoc dl,
                                        SelectionDAG &DAG,
                                        SmallVectorImpl<SDValue> &InVals)
                                          const {
  MachineFunction &MF = DAG.getMachineFunction();
  MachineFrameInfo *MFI = MF.getFrameInfo();
  //  LPUMachineFunctionInfo *FuncInfo = MF.getInfo<LPUMachineFunctionInfo>();

  // Assign locations to all of the incoming arguments.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());

  CCInfo.AnalyzeFormalArguments(Ins, CC_Reg_LPU);

  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];
    if (VA.isRegLoc()) {
      // Arguments passed in registers
      unsigned Reg = MF.addLiveIn(VA.getLocReg(), &LPU::ANYRRegClass);
      SDValue ArgValue = DAG.getCopyFromReg(Chain, dl, Reg, VA.getLocVT());
      InVals.push_back(ArgValue);

    } else {
      // Sanity check
      assert(VA.isMemLoc());

      SDValue InVal;
      ISD::ArgFlagsTy Flags = Ins[i].Flags;

      if (Flags.isByVal()) {
        int FI = MFI->CreateFixedObject(Flags.getByValSize(),
                                        VA.getLocMemOffset(), true);
        InVal = DAG.getFrameIndex(FI, getPointerTy());
      } else {
        // Load the argument to a virtual register
        unsigned ObjSize = VA.getLocVT().getSizeInBits()/8;
        if (ObjSize > 2) {
            errs() << "LowerFormalArguments Unhandled argument type: "
                << EVT(VA.getLocVT()).getEVTString()
                << "\n";
        }
        // Create the frame index object for this incoming parameter...
        int FI = MFI->CreateFixedObject(ObjSize, VA.getLocMemOffset(), true);

        // Create the SelectionDAG nodes corresponding to a load
        //from this parameter
        SDValue FIN = DAG.getFrameIndex(FI, MVT::i16);
        InVal = DAG.getLoad(VA.getLocVT(), dl, Chain, FIN,
                            MachinePointerInfo::getFixedStack(FI),
                            false, false, false, 0);
      }

      InVals.push_back(InVal);
    }
  }

  return Chain;
}

SDValue
LPUTargetLowering::LowerReturn(SDValue Chain,
                                  CallingConv::ID CallConv, bool isVarArg,
                                  const SmallVectorImpl<ISD::OutputArg> &Outs,
                                  const SmallVectorImpl<SDValue> &OutVals,
                                  SDLoc dl, SelectionDAG &DAG) const {

  // CCValAssign - represent the assignment of the return value to a location
  SmallVector<CCValAssign, 16> RVLocs;

  // CCState - Info about the registers and stack slot.
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  // Analize return values.
  CCInfo.AnalyzeReturn(Outs,RetCC_Reg_LPU);

  SDValue Flag;
  SmallVector<SDValue, 4> RetOps(1, Chain);

  // Copy the result values into the output registers.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    CCValAssign &VA = RVLocs[i];
    assert(VA.isRegLoc() && "Can only return in registers!");

    Chain = DAG.getCopyToReg(Chain, dl, VA.getLocReg(),
                             OutVals[i], Flag);

    // Guarantee that all emitted copies are stuck together,
    // avoiding something bad.
    Flag = Chain.getValue(1);
    RetOps.push_back(DAG.getRegister(VA.getLocReg(), VA.getLocVT()));
  }

  RetOps[0] = Chain;  // Update chain.

  // Add the flag if we have it.
  if (Flag.getNode())
    RetOps.push_back(Flag);

  return DAG.getNode(LPUISD::Ret, dl, MVT::Other, RetOps);
}
/*
/// LowerCCCCallTo - functions arguments are copied from virtual regs to
/// (physical regs)/(stack frame), CALLSEQ_START and CALLSEQ_END are emitted.
// TODO: sret.
SDValue
LPUTargetLowering::LowerCCCCallTo(SDValue Chain, SDValue Callee,
                                     CallingConv::ID CallConv, bool isVarArg,
                                     bool isTailCall,
                                     const SmallVectorImpl<ISD::OutputArg>
                                       &Outs,
                                     const SmallVectorImpl<SDValue> &OutVals,
                                     const SmallVectorImpl<ISD::InputArg> &Ins,
                                     SDLoc dl, SelectionDAG &DAG,
                                     SmallVectorImpl<SDValue> &InVals) const {
  // Analyze operands of the call, assigning locations to each operand.
  SmallVector<CCValAssign, 16> ArgLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), ArgLocs,
                 *DAG.getContext());
  AnalyzeArguments(CCInfo, ArgLocs, Outs);

  // Get a count of how many bytes are to be pushed on the stack.
  unsigned NumBytes = CCInfo.getNextStackOffset();

  Chain = DAG.getCALLSEQ_START(Chain ,DAG.getConstant(NumBytes,
                                                      getPointerTy(), true),
                               dl);

  SmallVector<std::pair<unsigned, SDValue>, 4> RegsToPass;
  SmallVector<SDValue, 12> MemOpChains;
  SDValue StackPtr;

  // Walk the register/memloc assignments, inserting copies/loads.
  for (unsigned i = 0, e = ArgLocs.size(); i != e; ++i) {
    CCValAssign &VA = ArgLocs[i];

    SDValue Arg = OutVals[i];

    // Promote the value if needed.
    switch (VA.getLocInfo()) {
      default: llvm_unreachable("Unknown loc info!");
      case CCValAssign::Full: break;
      case CCValAssign::SExt:
        Arg = DAG.getNode(ISD::SIGN_EXTEND, dl, VA.getLocVT(), Arg);
        break;
      case CCValAssign::ZExt:
        Arg = DAG.getNode(ISD::ZERO_EXTEND, dl, VA.getLocVT(), Arg);
        break;
      case CCValAssign::AExt:
        Arg = DAG.getNode(ISD::ANY_EXTEND, dl, VA.getLocVT(), Arg);
        break;
    }

    // Arguments that can be passed on register must be kept at RegsToPass
    // vector
    if (VA.isRegLoc()) {
      RegsToPass.push_back(std::make_pair(VA.getLocReg(), Arg));
    } else {
      assert(VA.isMemLoc());

      if (!StackPtr.getNode())
        StackPtr = DAG.getCopyFromReg(Chain, dl, LPU::SP, getPointerTy());

      SDValue PtrOff = DAG.getNode(ISD::ADD, dl, getPointerTy(),
                                   StackPtr,
                                   DAG.getIntPtrConstant(VA.getLocMemOffset()));

      SDValue MemOp;
      ISD::ArgFlagsTy Flags = Outs[i].Flags;

      if (Flags.isByVal()) {
        SDValue SizeNode = DAG.getConstant(Flags.getByValSize(), MVT::i16);
        MemOp = DAG.getMemcpy(Chain, dl, PtrOff, Arg, SizeNode,
                              Flags.getByValAlign(),
                              false, //isVolatile
			      true, //AlwaysInline
                              MachinePointerInfo(),
                              MachinePointerInfo());
      } else {
        MemOp = DAG.getStore(Chain, dl, Arg, PtrOff, MachinePointerInfo(),
                             false, false, 0);
      }

      MemOpChains.push_back(MemOp);
    }
  }

  // Transform all store nodes into one single node because all store nodes are
  // independent of each other.
  if (!MemOpChains.empty())
    Chain = DAG.getNode(ISD::TokenFactor, dl, MVT::Other, MemOpChains);

  // Build a sequence of copy-to-reg nodes chained together with token chain and
  // flag operands which copy the outgoing args into registers.  The InFlag in
  // necessary since all emitted instructions must be stuck together.
  SDValue InFlag;
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i) {
    Chain = DAG.getCopyToReg(Chain, dl, RegsToPass[i].first,
                             RegsToPass[i].second, InFlag);
    InFlag = Chain.getValue(1);
  }

  // If the callee is a GlobalAddress node (quite common, every direct call is)
  // turn it into a TargetGlobalAddress node so that legalize doesn't hack it.
  // Likewise ExternalSymbol -> TargetExternalSymbol.
  if (GlobalAddressSDNode *G = dyn_cast<GlobalAddressSDNode>(Callee))
    Callee = DAG.getTargetGlobalAddress(G->getGlobal(), dl, MVT::i16);
  else if (ExternalSymbolSDNode *E = dyn_cast<ExternalSymbolSDNode>(Callee))
    Callee = DAG.getTargetExternalSymbol(E->getSymbol(), MVT::i16);

  // Returns a chain & a flag for retval copy to use.
  SDVTList NodeTys = DAG.getVTList(MVT::Other, MVT::Glue);
  SmallVector<SDValue, 8> Ops;
  Ops.push_back(Chain);
  Ops.push_back(Callee);

  // Add argument registers to the end of the list so that they are
  // known live into the call.
  for (unsigned i = 0, e = RegsToPass.size(); i != e; ++i)
    Ops.push_back(DAG.getRegister(RegsToPass[i].first,
                                  RegsToPass[i].second.getValueType()));

  if (InFlag.getNode())
    Ops.push_back(InFlag);

  Chain = DAG.getNode(LPUISD::CALL, dl, NodeTys, Ops);
  InFlag = Chain.getValue(1);

  // Create the CALLSEQ_END node.
  Chain = DAG.getCALLSEQ_END(Chain,
                             DAG.getConstant(NumBytes, getPointerTy(), true),
                             DAG.getConstant(0, getPointerTy(), true),
                             InFlag, dl);
  InFlag = Chain.getValue(1);

  // Handle result values, copying them out of physregs into vregs that we
  // return.
  return LowerCallResult(Chain, InFlag, CallConv, isVarArg, Ins, dl,
                         DAG, InVals);
}

/// LowerCallResult - Lower the result values of a call into the
/// appropriate copies out of appropriate physical registers.
///
SDValue
LPUTargetLowering::LowerCallResult(SDValue Chain, SDValue InFlag,
                                      CallingConv::ID CallConv, bool isVarArg,
                                      const SmallVectorImpl<ISD::InputArg> &Ins,
                                      SDLoc dl, SelectionDAG &DAG,
                                      SmallVectorImpl<SDValue> &InVals) const {

  // Assign locations to each value returned by this call.
  SmallVector<CCValAssign, 16> RVLocs;
  CCState CCInfo(CallConv, isVarArg, DAG.getMachineFunction(), RVLocs,
                 *DAG.getContext());

  CCInfo.AnalyzeCallResult(Ins,RetCC_Reg_LPU);

  // Copy all of the result registers out of their specified physreg.
  for (unsigned i = 0; i != RVLocs.size(); ++i) {
    Chain = DAG.getCopyFromReg(Chain, dl, RVLocs[i].getLocReg(),
                               RVLocs[i].getValVT(), InFlag).getValue(1);
    InFlag = Chain.getValue(2);
    InVals.push_back(Chain.getValue(0));
  }

  return Chain;
}

SDValue LPUTargetLowering::LowerShifts(SDValue Op,
                                          SelectionDAG &DAG) const {
  unsigned Opc = Op.getOpcode();
  SDNode* N = Op.getNode();
  EVT VT = Op.getValueType();
  SDLoc dl(N);

  // Expand non-constant shifts to loops:
  if (!isa<ConstantSDNode>(N->getOperand(1)))
    switch (Opc) {
    default: llvm_unreachable("Invalid shift opcode!");
    case ISD::SHL:
      return DAG.getNode(LPUISD::SHL, dl,
                         VT, N->getOperand(0), N->getOperand(1));
    case ISD::SRA:
      return DAG.getNode(LPUISD::SRA, dl,
                         VT, N->getOperand(0), N->getOperand(1));
    case ISD::SRL:
      return DAG.getNode(LPUISD::SRL, dl,
                         VT, N->getOperand(0), N->getOperand(1));
    }

  uint64_t ShiftAmount = cast<ConstantSDNode>(N->getOperand(1))->getZExtValue();

  // Expand the stuff into sequence of shifts.
  // FIXME: for some shift amounts this might be done better!
  // E.g.: foo >> (8 + N) => sxt(swpb(foo)) >> N
  SDValue Victim = N->getOperand(0);

  if (Opc == ISD::SRL && ShiftAmount) {
    // Emit a special goodness here:
    // srl A, 1 => clrc; rrc A
    Victim = DAG.getNode(LPUISD::RRC, dl, VT, Victim);
    ShiftAmount -= 1;
  }

  while (ShiftAmount--)
    Victim = DAG.getNode((Opc == ISD::SHL ? LPUISD::RLA : LPUISD::RRA),
                         dl, VT, Victim);

  return Victim;
}

SDValue LPUTargetLowering::LowerGlobalAddress(SDValue Op,
                                                 SelectionDAG &DAG) const {
  const GlobalValue *GV = cast<GlobalAddressSDNode>(Op)->getGlobal();
  int64_t Offset = cast<GlobalAddressSDNode>(Op)->getOffset();

  // Create the TargetGlobalAddress node, folding in the constant offset.
  SDValue Result = DAG.getTargetGlobalAddress(GV, SDLoc(Op),
                                              getPointerTy(), Offset);
  return DAG.getNode(LPUISD::Wrapper, SDLoc(Op),
                     getPointerTy(), Result);
}

SDValue LPUTargetLowering::LowerExternalSymbol(SDValue Op,
                                                  SelectionDAG &DAG) const {
  SDLoc dl(Op);
  const char *Sym = cast<ExternalSymbolSDNode>(Op)->getSymbol();
  SDValue Result = DAG.getTargetExternalSymbol(Sym, getPointerTy());

  return DAG.getNode(LPUISD::Wrapper, dl, getPointerTy(), Result);
}

SDValue LPUTargetLowering::LowerBlockAddress(SDValue Op,
                                                SelectionDAG &DAG) const {
  SDLoc dl(Op);
  const BlockAddress *BA = cast<BlockAddressSDNode>(Op)->getBlockAddress();
  SDValue Result = DAG.getTargetBlockAddress(BA, getPointerTy());

  return DAG.getNode(LPUISD::Wrapper, dl, getPointerTy(), Result);
}

static SDValue EmitCMP(SDValue &LHS, SDValue &RHS, SDValue &TargetCC,
                       ISD::CondCode CC,
                       SDLoc dl, SelectionDAG &DAG) {
  // FIXME: Handle bittests someday
  assert(!LHS.getValueType().isFloatingPoint() && "We don't handle FP yet");

  // FIXME: Handle jump negative someday
  LPUCC::CondCodes TCC = LPUCC::COND_INVALID;
  switch (CC) {
  default: llvm_unreachable("Invalid integer condition!");
  case ISD::SETEQ:
    TCC = LPUCC::COND_E;     // aka COND_Z
    // Minor optimization: if LHS is a constant, swap operands, then the
    // constant can be folded into comparison.
    if (LHS.getOpcode() == ISD::Constant)
      std::swap(LHS, RHS);
    break;
  case ISD::SETNE:
    TCC = LPUCC::COND_NE;    // aka COND_NZ
    // Minor optimization: if LHS is a constant, swap operands, then the
    // constant can be folded into comparison.
    if (LHS.getOpcode() == ISD::Constant)
      std::swap(LHS, RHS);
    break;
  case ISD::SETULE:
    std::swap(LHS, RHS);        // FALLTHROUGH
  case ISD::SETUGE:
    // Turn lhs u>= rhs with lhs constant into rhs u< lhs+1, this allows us to
    // fold constant into instruction.
    if (const ConstantSDNode * C = dyn_cast<ConstantSDNode>(LHS)) {
      LHS = RHS;
      RHS = DAG.getConstant(C->getSExtValue() + 1, C->getValueType(0));
      TCC = LPUCC::COND_LO;
      break;
    }
    TCC = LPUCC::COND_HS;    // aka COND_C
    break;
  case ISD::SETUGT:
    std::swap(LHS, RHS);        // FALLTHROUGH
  case ISD::SETULT:
    // Turn lhs u< rhs with lhs constant into rhs u>= lhs+1, this allows us to
    // fold constant into instruction.
    if (const ConstantSDNode * C = dyn_cast<ConstantSDNode>(LHS)) {
      LHS = RHS;
      RHS = DAG.getConstant(C->getSExtValue() + 1, C->getValueType(0));
      TCC = LPUCC::COND_HS;
      break;
    }
    TCC = LPUCC::COND_LO;    // aka COND_NC
    break;
  case ISD::SETLE:
    std::swap(LHS, RHS);        // FALLTHROUGH
  case ISD::SETGE:
    // Turn lhs >= rhs with lhs constant into rhs < lhs+1, this allows us to
    // fold constant into instruction.
    if (const ConstantSDNode * C = dyn_cast<ConstantSDNode>(LHS)) {
      LHS = RHS;
      RHS = DAG.getConstant(C->getSExtValue() + 1, C->getValueType(0));
      TCC = LPUCC::COND_L;
      break;
    }
    TCC = LPUCC::COND_GE;
    break;
  case ISD::SETGT:
    std::swap(LHS, RHS);        // FALLTHROUGH
  case ISD::SETLT:
    // Turn lhs < rhs with lhs constant into rhs >= lhs+1, this allows us to
    // fold constant into instruction.
    if (const ConstantSDNode * C = dyn_cast<ConstantSDNode>(LHS)) {
      LHS = RHS;
      RHS = DAG.getConstant(C->getSExtValue() + 1, C->getValueType(0));
      TCC = LPUCC::COND_GE;
      break;
    }
    TCC = LPUCC::COND_L;
    break;
  }

  TargetCC = DAG.getConstant(TCC, MVT::i8);
  return DAG.getNode(LPUISD::CMP, dl, MVT::Glue, LHS, RHS);
}


SDValue LPUTargetLowering::LowerBR_CC(SDValue Op, SelectionDAG &DAG) const {
  SDValue Chain = Op.getOperand(0);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(1))->get();
  SDValue LHS   = Op.getOperand(2);
  SDValue RHS   = Op.getOperand(3);
  SDValue Dest  = Op.getOperand(4);
  SDLoc dl  (Op);

  SDValue TargetCC;
  SDValue Flag = EmitCMP(LHS, RHS, TargetCC, CC, dl, DAG);

  return DAG.getNode(LPUISD::BR_CC, dl, Op.getValueType(),
                     Chain, Dest, TargetCC, Flag);
}

SDValue LPUTargetLowering::LowerSETCC(SDValue Op, SelectionDAG &DAG) const {
  SDValue LHS   = Op.getOperand(0);
  SDValue RHS   = Op.getOperand(1);
  SDLoc dl  (Op);

  // If we are doing an AND and testing against zero, then the CMP
  // will not be generated.  The AND (or BIT) will generate the condition codes,
  // but they are different from CMP.
  // FIXME: since we're doing a post-processing, use a pseudoinstr here, so
  // lowering & isel wouldn't diverge.
  bool andCC = false;
  if (ConstantSDNode *RHSC = dyn_cast<ConstantSDNode>(RHS)) {
    if (RHSC->isNullValue() && LHS.hasOneUse() &&
        (LHS.getOpcode() == ISD::AND ||
         (LHS.getOpcode() == ISD::TRUNCATE &&
          LHS.getOperand(0).getOpcode() == ISD::AND))) {
      andCC = true;
    }
  }
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(2))->get();
  SDValue TargetCC;
  SDValue Flag = EmitCMP(LHS, RHS, TargetCC, CC, dl, DAG);

  // Get the condition codes directly from the status register, if its easy.
  // Otherwise a branch will be generated.  Note that the AND and BIT
  // instructions generate different flags than CMP, the carry bit can be used
  // for NE/EQ.
  bool Invert = false;
  bool Shift = false;
  bool Convert = true;
  switch (cast<ConstantSDNode>(TargetCC)->getZExtValue()) {
   default:
    Convert = false;
    break;
   case LPUCC::COND_HS:
     // Res = SR & 1, no processing is required
     break;
   case LPUCC::COND_LO:
     // Res = ~(SR & 1)
     Invert = true;
     break;
   case LPUCC::COND_NE:
     if (andCC) {
       // C = ~Z, thus Res = SR & 1, no processing is required
     } else {
       // Res = ~((SR >> 1) & 1)
       Shift = true;
       Invert = true;
     }
     break;
   case LPUCC::COND_E:
     Shift = true;
     // C = ~Z for AND instruction, thus we can put Res = ~(SR & 1), however,
     // Res = (SR >> 1) & 1 is 1 word shorter.
     break;
  }
  EVT VT = Op.getValueType();
  SDValue One  = DAG.getConstant(1, VT);
  if (Convert) {
    SDValue SR = DAG.getCopyFromReg(DAG.getEntryNode(), dl, LPU::SR,
                                    MVT::i16, Flag);
    if (Shift)
      // FIXME: somewhere this is turned into a SRL, lower it LPU specific?
      SR = DAG.getNode(ISD::SRA, dl, MVT::i16, SR, One);
    SR = DAG.getNode(ISD::AND, dl, MVT::i16, SR, One);
    if (Invert)
      SR = DAG.getNode(ISD::XOR, dl, MVT::i16, SR, One);
    return SR;
  } else {
    SDValue Zero = DAG.getConstant(0, VT);
    SDVTList VTs = DAG.getVTList(Op.getValueType(), MVT::Glue);
    SmallVector<SDValue, 4> Ops;
    Ops.push_back(One);
    Ops.push_back(Zero);
    Ops.push_back(TargetCC);
    Ops.push_back(Flag);
    return DAG.getNode(LPUISD::SELECT_CC, dl, VTs, Ops);
  }
}

SDValue LPUTargetLowering::LowerSELECT_CC(SDValue Op,
                                             SelectionDAG &DAG) const {
  SDValue LHS    = Op.getOperand(0);
  SDValue RHS    = Op.getOperand(1);
  SDValue TrueV  = Op.getOperand(2);
  SDValue FalseV = Op.getOperand(3);
  ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(4))->get();
  SDLoc dl   (Op);

  SDValue TargetCC;
  SDValue Flag = EmitCMP(LHS, RHS, TargetCC, CC, dl, DAG);

  SDVTList VTs = DAG.getVTList(Op.getValueType(), MVT::Glue);
  SmallVector<SDValue, 4> Ops;
  Ops.push_back(TrueV);
  Ops.push_back(FalseV);
  Ops.push_back(TargetCC);
  Ops.push_back(Flag);

  return DAG.getNode(LPUISD::SELECT_CC, dl, VTs, Ops);
}

SDValue LPUTargetLowering::LowerSIGN_EXTEND(SDValue Op,
                                               SelectionDAG &DAG) const {
  SDValue Val = Op.getOperand(0);
  EVT VT      = Op.getValueType();
  SDLoc dl(Op);

  assert(VT == MVT::i16 && "Only support i16 for now!");

  return DAG.getNode(ISD::SIGN_EXTEND_INREG, dl, VT,
                     DAG.getNode(ISD::ANY_EXTEND, dl, VT, Val),
                     DAG.getValueType(Val.getValueType()));
}

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

SDValue LPUTargetLowering::LowerJumpTable(SDValue Op,
                                             SelectionDAG &DAG) const {
    JumpTableSDNode *JT = cast<JumpTableSDNode>(Op);
    SDValue Result = DAG.getTargetJumpTable(JT->getIndex(), getPointerTy());
    return DAG.getNode(LPUISD::Wrapper, SDLoc(JT),
                       getPointerTy(), Result);
}

/// getPostIndexedAddressParts - returns true by value, base pointer and
/// offset pointer and addressing mode by reference if this node can be
/// combined with a load / store to form a post-indexed load / store.
bool LPUTargetLowering::getPostIndexedAddressParts(SDNode *N, SDNode *Op,
                                                      SDValue &Base,
                                                      SDValue &Offset,
                                                      ISD::MemIndexedMode &AM,
                                                      SelectionDAG &DAG) const {

  LoadSDNode *LD = cast<LoadSDNode>(N);
  if (LD->getExtensionType() != ISD::NON_EXTLOAD)
    return false;

  EVT VT = LD->getMemoryVT();
  if (VT != MVT::i8 && VT != MVT::i16)
    return false;

  if (Op->getOpcode() != ISD::ADD)
    return false;

  if (ConstantSDNode *RHS = dyn_cast<ConstantSDNode>(Op->getOperand(1))) {
    uint64_t RHSC = RHS->getZExtValue();
    if ((VT == MVT::i16 && RHSC != 2) ||
        (VT == MVT::i8 && RHSC != 1))
      return false;

    Base = Op->getOperand(0);
    Offset = DAG.getConstant(RHSC, VT);
    AM = ISD::POST_INC;
    return true;
  }

  return false;
}
*/

/*
bool LPUTargetLowering::isTruncateFree(Type *Ty1,
                                          Type *Ty2) const {
  if (!Ty1->isIntegerTy() || !Ty2->isIntegerTy())
    return false;

  return (Ty1->getPrimitiveSizeInBits() > Ty2->getPrimitiveSizeInBits());
}
*/
bool LPUTargetLowering::isTruncateFree(EVT VT1, EVT VT2) const {
  if (!VT1.isInteger() || !VT2.isInteger())
    return false;
  unsigned NumBits1 = VT1.getSizeInBits();
  unsigned NumBits2 = VT2.getSizeInBits();
  return NumBits1 > NumBits2;
}
/*
bool LPUTargetLowering::isTruncateFree(EVT VT1, EVT VT2) const {
  if (!VT1.isInteger() || !VT2.isInteger())
    return false;

  return (VT1.getSizeInBits() > VT2.getSizeInBits());
}
*/

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

/*

//===----------------------------------------------------------------------===//
//  Other Lowering Code
//===----------------------------------------------------------------------===//

MachineBasicBlock*
LPUTargetLowering::EmitShiftInstr(MachineInstr *MI,
                                     MachineBasicBlock *BB) const {
  MachineFunction *F = BB->getParent();
  MachineRegisterInfo &RI = F->getRegInfo();
  DebugLoc dl = MI->getDebugLoc();
  const TargetInstrInfo &TII =
      *getTargetMachine().getSubtargetImpl()->getInstrInfo();

  unsigned Opc;
  const TargetRegisterClass * RC;
  switch (MI->getOpcode()) {
  default: llvm_unreachable("Invalid shift opcode!");
  case LPU::Shl8:
   Opc = LPU::SHL8r1;
   RC = &LPU::GR8RegClass;
   break;
  case LPU::Shl16:
   Opc = LPU::SHL16r1;
   RC = &LPU::GR16RegClass;
   break;
  case LPU::Sra8:
   Opc = LPU::SAR8r1;
   RC = &LPU::GR8RegClass;
   break;
  case LPU::Sra16:
   Opc = LPU::SAR16r1;
   RC = &LPU::GR16RegClass;
   break;
  case LPU::Srl8:
   Opc = LPU::SAR8r1c;
   RC = &LPU::GR8RegClass;
   break;
  case LPU::Srl16:
   Opc = LPU::SAR16r1c;
   RC = &LPU::GR16RegClass;
   break;
  }

  const BasicBlock *LLVM_BB = BB->getBasicBlock();
  MachineFunction::iterator I = BB;
  ++I;

  // Create loop block
  MachineBasicBlock *LoopBB = F->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *RemBB  = F->CreateMachineBasicBlock(LLVM_BB);

  F->insert(I, LoopBB);
  F->insert(I, RemBB);

  // Update machine-CFG edges by transferring all successors of the current
  // block to the block containing instructions after shift.
  RemBB->splice(RemBB->begin(), BB, std::next(MachineBasicBlock::iterator(MI)),
                BB->end());
  RemBB->transferSuccessorsAndUpdatePHIs(BB);

  // Add adges BB => LoopBB => RemBB, BB => RemBB, LoopBB => LoopBB
  BB->addSuccessor(LoopBB);
  BB->addSuccessor(RemBB);
  LoopBB->addSuccessor(RemBB);
  LoopBB->addSuccessor(LoopBB);

  unsigned ShiftAmtReg = RI.createVirtualRegister(&LPU::GR8RegClass);
  unsigned ShiftAmtReg2 = RI.createVirtualRegister(&LPU::GR8RegClass);
  unsigned ShiftReg = RI.createVirtualRegister(RC);
  unsigned ShiftReg2 = RI.createVirtualRegister(RC);
  unsigned ShiftAmtSrcReg = MI->getOperand(2).getReg();
  unsigned SrcReg = MI->getOperand(1).getReg();
  unsigned DstReg = MI->getOperand(0).getReg();

  // BB:
  // cmp 0, N
  // je RemBB
  BuildMI(BB, dl, TII.get(LPU::CMP8ri))
    .addReg(ShiftAmtSrcReg).addImm(0);
  BuildMI(BB, dl, TII.get(LPU::JCC))
    .addMBB(RemBB)
    .addImm(LPUCC::COND_E);

  // LoopBB:
  // ShiftReg = phi [%SrcReg, BB], [%ShiftReg2, LoopBB]
  // ShiftAmt = phi [%N, BB],      [%ShiftAmt2, LoopBB]
  // ShiftReg2 = shift ShiftReg
  // ShiftAmt2 = ShiftAmt - 1;
  BuildMI(LoopBB, dl, TII.get(LPU::PHI), ShiftReg)
    .addReg(SrcReg).addMBB(BB)
    .addReg(ShiftReg2).addMBB(LoopBB);
  BuildMI(LoopBB, dl, TII.get(LPU::PHI), ShiftAmtReg)
    .addReg(ShiftAmtSrcReg).addMBB(BB)
    .addReg(ShiftAmtReg2).addMBB(LoopBB);
  BuildMI(LoopBB, dl, TII.get(Opc), ShiftReg2)
    .addReg(ShiftReg);
  BuildMI(LoopBB, dl, TII.get(LPU::SUB8ri), ShiftAmtReg2)
    .addReg(ShiftAmtReg).addImm(1);
  BuildMI(LoopBB, dl, TII.get(LPU::JCC))
    .addMBB(LoopBB)
    .addImm(LPUCC::COND_NE);

  // RemBB:
  // DestReg = phi [%SrcReg, BB], [%ShiftReg, LoopBB]
  BuildMI(*RemBB, RemBB->begin(), dl, TII.get(LPU::PHI), DstReg)
    .addReg(SrcReg).addMBB(BB)
    .addReg(ShiftReg2).addMBB(LoopBB);

  MI->eraseFromParent();   // The pseudo instruction is gone now.
  return RemBB;
}

MachineBasicBlock*
LPUTargetLowering::EmitInstrWithCustomInserter(MachineInstr *MI,
                                                  MachineBasicBlock *BB) const {
  unsigned Opc = MI->getOpcode();

  if (Opc == LPU::Shl8 || Opc == LPU::Shl16 ||
      Opc == LPU::Sra8 || Opc == LPU::Sra16 ||
      Opc == LPU::Srl8 || Opc == LPU::Srl16)
    return EmitShiftInstr(MI, BB);

  const TargetInstrInfo &TII =
      *getTargetMachine().getSubtargetImpl()->getInstrInfo();
  DebugLoc dl = MI->getDebugLoc();

  assert((Opc == LPU::Select16 || Opc == LPU::Select8) &&
         "Unexpected instr type to insert");

  // To "insert" a SELECT instruction, we actually have to insert the diamond
  // control-flow pattern.  The incoming instruction knows the destination vreg
  // to set, the condition code register to branch on, the true/false values to
  // select between, and a branch opcode to use.
  const BasicBlock *LLVM_BB = BB->getBasicBlock();
  MachineFunction::iterator I = BB;
  ++I;

  //  thisMBB:
  //  ...
  //   TrueVal = ...
  //   cmpTY ccX, r1, r2
  //   jCC copy1MBB
  //   fallthrough --> copy0MBB
  MachineBasicBlock *thisMBB = BB;
  MachineFunction *F = BB->getParent();
  MachineBasicBlock *copy0MBB = F->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *copy1MBB = F->CreateMachineBasicBlock(LLVM_BB);
  F->insert(I, copy0MBB);
  F->insert(I, copy1MBB);
  // Update machine-CFG edges by transferring all successors of the current
  // block to the new block which will contain the Phi node for the select.
  copy1MBB->splice(copy1MBB->begin(), BB,
                   std::next(MachineBasicBlock::iterator(MI)), BB->end());
  copy1MBB->transferSuccessorsAndUpdatePHIs(BB);
  // Next, add the true and fallthrough blocks as its successors.
  BB->addSuccessor(copy0MBB);
  BB->addSuccessor(copy1MBB);

  BuildMI(BB, dl, TII.get(LPU::JCC))
    .addMBB(copy1MBB)
    .addImm(MI->getOperand(3).getImm());

  //  copy0MBB:
  //   %FalseValue = ...
  //   # fallthrough to copy1MBB
  BB = copy0MBB;

  // Update machine-CFG edges
  BB->addSuccessor(copy1MBB);

  //  copy1MBB:
  //   %Result = phi [ %FalseValue, copy0MBB ], [ %TrueValue, thisMBB ]
  //  ...
  BB = copy1MBB;
  BuildMI(*BB, BB->begin(), dl, TII.get(LPU::PHI),
          MI->getOperand(0).getReg())
    .addReg(MI->getOperand(2).getReg()).addMBB(copy0MBB)
    .addReg(MI->getOperand(1).getReg()).addMBB(thisMBB);

  MI->eraseFromParent();   // The pseudo instruction is gone now.
  return BB;
}
*/
