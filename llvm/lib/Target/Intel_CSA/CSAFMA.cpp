//====-- CSAFMA.cpp - Fused Multiply Add optimization for CSA --------------====
//
//      Copyright (c) 2019 Intel Corporation.
//      All rights reserved.
//
//        INTEL CORPORATION PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license
// agreement or nondisclosure agreement with Intel Corp.
// and may not be copied or disclosed except in accordance
// with the terms of that agreement.
//
// This file defines the pass which finds the best representations of
// the original expression trees consisting of MUL/ADD/SUB/FMA/NEG
// operations and performs transformations.
//
// Implementation was originally derived from the X86 Global FMA pass (see
// lib/Target/X86/Intel_X86FMA.cpp for details), but customized for CSA target.

#include "CSA.h"
#include "CSAInstBuilder.h"
#include "CSAInstrInfo.h"
#include "CSASubtarget.h"
#include "llvm/CodeGen/Intel_FMACGCommon.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"

using namespace llvm;

#define DEBUG_TYPE "global-fma"

/// This internal switch can be used to turn off the Global FMA optimization.
static cl::opt<bool> EnableFMAOpt("csa-global-fma",
                                  cl::desc("Enable the global FMA opt."),
                                  cl::init(false), cl::Hidden);

/// The bits that are used to define various FMA heuristics in the internal
/// switch FMAControl/"-csa-global-fma-control".
static const unsigned FMAControlForceFMAs = 0x1u;
static const unsigned FMAControlTuneForLatency = 0x2u;
static const unsigned FMAControlTuneForThroughput = 0x4u;

/// The internal switch that is used to re-define FMA heuristics.
static cl::opt<unsigned> FMAControl("csa-global-fma-control",
                                    cl::desc("FMA heuristics control."),
                                    cl::init(0), cl::Hidden);

/// Returns true iff all of the passed features \p F are enabled
/// by the internal switch FMAControl/"-csa-global-fma-control".
static bool checkFMAControl(unsigned F) { return (FMAControl & F) == F; }

/// Returns immediate value for 1.0 constant of the given type.
static int64_t getOne(MVT VT) {
  switch (VT.SimpleTy) {
  case MVT::f16:
    return 0x3C00;
  case MVT::f32:
    return 0x3f800000L;
  case MVT::f64:
    return 0x3ff0000000000000LL;
  case MVT::v4f16:
    return 0x3C003C003C003C00LL;
  case MVT::v2f32:
    return 0x3f8000003f800000LL;
  default:
    llvm_unreachable("Unsupported type");
  }
}

/// Returns immediate value representing sign bit(s) for the given type.
static int64_t getSignMask(MVT VT) {
  switch (VT.SimpleTy) {
  case MVT::f16:
    return 0x8000;
  case MVT::f32:
    return 0x80000000L;
  case MVT::f64:
    return 0x8000000000000000LL;
  case MVT::v4f16:
    return 0x8000800080008000LL;
  case MVT::v2f32:
    return 0x8000000080000000LL;
  default:
    llvm_unreachable("Unsupported type");
  }
}

namespace {

/// This class holds all pre-computed/efficient FMA patterns/DAGs encoded in
/// 64-bit integer values.
class CSAFMAPatterns final : public FMAPatterns {
public:
  CSAFMAPatterns() : FMAPatterns() {
#include "CSAGenMAPatterns.inc"
    init();
  }
};

/// This class describes the known MUL/ADD/SUB/FMA/NEG/etc operations and
/// utility methods for working with such operations.
class FMAOpcodes final {
public:
  /// Enum defining the known classes of operations interesting to FMA
  /// optimization.
  enum FMAOpcodeKind {
    ADDOpc,
    SUBOpc,
    MULOpc,
    NEGOpc,
    FMAOpc,
    FMSOpc,
    FMRSOpc
  };

private:
  /// A structure describing one operation.
  struct FMAOpcodeDesc {
    /// Register opcode.
    unsigned Opcode;

    /// Machine value type.
    MVT VT;

    /// Opcode kind.
    FMAOpcodeKind Kind;
  };

  /// Known opcodes.
  static const FMAOpcodeDesc Descs[];

  /// Maps for fast opcode lookups.
  using FMAKindType = std::pair<FMAOpcodeKind, MVT>;
  std::map<unsigned, FMAKindType> Opcode2KindType;
  std::map<FMAKindType, unsigned> KindType2Opcode;

public:
  FMAOpcodes();

  /// This function returns true iff the given opcode \p Opcode should be
  /// recognized by the FMA optimization. Also, if the opcode is recognized,
  /// then machine value type associated with the opcode is returned in \p VT,
  /// the opcode kind is returned in \p OpcodeKind.
  /// It is assumed here that all recognized opcodes can be represented as
  /// FMA operations having 3 operands: ((MulSign)(Op1 * Op2) + (AddSign)Op3),
  /// where the MulSign is the sign of the product of the first 2 operands
  /// and AddSign is the sign of the 3rd operand. The MulSign and AddSign signs
  /// are returned in the corresponding parameters \p MulSign and \p AddSign,
  /// and each parameter is set to true iff the corresponding sign is negative.
  /// For example, SUB(a,b) can be represented as (+a*1.0 - c). In this case
  /// \p MulSign must be set to false, and AddSign must be set to true.
  bool recognizeOpcode(unsigned Opcode, MVT &VT, FMAOpcodeKind &Kind,
                       bool &MulSign, bool &AddSign) const {
    auto It = Opcode2KindType.find(Opcode);
    if (It == Opcode2KindType.end())
      return false;

    Kind = It->second.first;
    VT = It->second.second;
    MulSign = Kind == FMRSOpc;
    AddSign = Kind == FMSOpc || Kind == SUBOpc || Kind == NEGOpc;
    return true;
  }

  /// Returns opcode of the given opcode kind \p OpcodeKind and machine value
  /// type \p VT.
  unsigned getOpcode(FMAOpcodeKind Kind, MVT VT) const {
    auto It = KindType2Opcode.find({Kind, VT});
    if (It != KindType2Opcode.end())
      return It->second;
    llvm_unreachable("Unsupported machine value type or opcode kind.");
  }

  /// Returns an opcode kind with the given signs of the product of 1st and 2nd
  /// FMA operands \p MulSign and the sign of the 3rd FMA operand \p AddSign.
  unsigned getFMAOpcode(bool MulSign, bool AddSign, MVT VT) const {
    assert(!(AddSign && MulSign) &&
           "Unsupported Mul/Add sign combination for FMA");
    auto Kind = MulSign ? FMRSOpc : AddSign ? FMSOpc : FMAOpc;
    return getOpcode(Kind, VT);
  }
};

const FMAOpcodes::FMAOpcodeDesc FMAOpcodes::Descs[] = {
    // ADD
    {CSA::ADDF16,    MVT::f16,   ADDOpc},
    {CSA::ADDF32,    MVT::f32,   ADDOpc},
    {CSA::ADDF64,    MVT::f64,   ADDOpc},
    {CSA::ADDF16X4,  MVT::v4f16, ADDOpc},
    {CSA::ADDF32X2,  MVT::v2f32, ADDOpc},
    // SUB
    {CSA::SUBF16,    MVT::f16,   SUBOpc},
    {CSA::SUBF32,    MVT::f32,   SUBOpc},
    {CSA::SUBF64,    MVT::f64,   SUBOpc},
    {CSA::SUBF16X4,  MVT::v4f16, SUBOpc},
    {CSA::SUBF32X2,  MVT::v2f32, SUBOpc},
    // MUL
    {CSA::MULF16,    MVT::f16,   MULOpc},
    {CSA::MULF32,    MVT::f32,   MULOpc},
    {CSA::MULF64,    MVT::f64,   MULOpc},
    {CSA::MULF16X4,  MVT::v4f16, MULOpc},
    {CSA::MULF32X2,  MVT::v2f32, MULOpc},
    // NEG
    {CSA::NEGF16,    MVT::f16,   NEGOpc},
    {CSA::NEGF32,    MVT::f32,   NEGOpc},
    {CSA::NEGF64,    MVT::f64,   NEGOpc},
    // FMA
    {CSA::FMAF16,    MVT::f16,   FMAOpc},
    {CSA::FMAF32,    MVT::f32,   FMAOpc},
    {CSA::FMAF64,    MVT::f64,   FMAOpc},
    {CSA::FMAF16X4,  MVT::v4f16, FMAOpc},
    {CSA::FMAF32X2,  MVT::v2f32, FMAOpc},
    // FMS
    {CSA::FMSF16,    MVT::f16,   FMSOpc},
    {CSA::FMSF32,    MVT::f32,   FMSOpc},
    {CSA::FMSF64,    MVT::f64,   FMSOpc},
    {CSA::FMSF16X4,  MVT::v4f16, FMSOpc},
    {CSA::FMSF32X2,  MVT::v2f32, FMSOpc},
    // FMRS
    {CSA::FMRSF16,   MVT::f16,   FMRSOpc},
    {CSA::FMRSF32,   MVT::f32,   FMRSOpc},
    {CSA::FMRSF64,   MVT::f64,   FMRSOpc},
    {CSA::FMRSF16X4, MVT::v4f16, FMRSOpc},
    {CSA::FMRSF32X2, MVT::v2f32, FMRSOpc}
};

FMAOpcodes::FMAOpcodes() {
  for (auto &D : Descs) {
    Opcode2KindType.insert({D.Opcode, {D.Kind, D.VT}});
    KindType2Opcode.insert({{D.Kind, D.VT}, D.Opcode});
  }
}

/// This class does all the optimization work, it goes through the functions,
/// searches for the optimizable expressions and replaces then with more
/// efficient equivalents.
class CSAGlobalFMA final : public GlobalFMA {
public:
  CSAGlobalFMA() : GlobalFMA(ID), ST(nullptr), TII(nullptr), MRI(nullptr) {}

  StringRef getPassName() const override { return "CSA GlobalFMA"; }

  bool runOnMachineFunction(MachineFunction &MFunc) override;

private:
  /// Pass identification, replacement for typeid.
  static char ID;

  /// This field is used to get information about available target instruction
  /// sets.
  const CSASubtarget *ST;

  /// This field is used to get information about available target operations.
  const CSAInstrInfo *TII;

  /// Machine register information.
  MachineRegisterInfo *MRI;

  /// FMA opcodes.
  std::unique_ptr<FMAOpcodes> Opcodes;

  /// Walks through all instructions in the machine basic block, finds
  /// MUL/ADD/FMA operations and creates FMA expressions (FMAExpr) for them.
  /// Returns the number of optimizable expressions found in the block.
  /// The parameter \p MRI is passed to this method to make it possible
  /// to find virtual registers associated with FMARegisterTerms and
  /// having uses that are not recognized as FMAExpr operations.
  std::unique_ptr<FMABasicBlock>
  parseBasicBlock(MachineBasicBlock &MBB) override;

  /// Generates output IR for the FMA expression \p Expr. The given DAG \p Dag
  /// gives the efficient version of the generated expression tree.
  /// The new instruction is inserted into the machine basic block \p MBB.
  /// The exact insertion point for the new instructions is taken from
  /// the field MI of the given FMA expression \p Expr.
  /// The parameter \p FMABB is passed to the routine as a storage of existing
  /// special const terms that may be needed during code-generation.
  ///
  /// Note that the parameter \p Expr does not have 'const' attribute because
  /// it may be changed during the code-gen phase. For example, virtual
  /// registers created and assigned to operands of \p Expr may be written
  /// to those operands.
  void generateOutputIR(FMAExpr &Expr, const FMADag &Dag) override;
};

char CSAGlobalFMA::ID = 0;

/// CSA specific variant of the immediate term.
class CSAFMAImmediateTerm final : public FMAImmediateTerm {
public:
  CSAFMAImmediateTerm(MVT VT, FMABasicBlock *BB, int64_t Imm)
      : FMAImmediateTerm(VT, BB, Imm) {}

  bool isZero() const override { return Imm == 0; }
  bool isOne() const override { return Imm == getOne(getVT()); }

  /// Prints the FMA expression or term to the given stream \p OS.
  /// The parameter \p PrintAttributes specifies if the caller wants to see
  /// more information and some of FMA node attributes should be printed out.
  void print(raw_ostream &OS, bool PrintAttributes) const override {
    OS << format_hex(Imm, 2u + VT.getSizeInBits() / 4);
    if (PrintAttributes)
      OS << " // Type: " << EVT(VT).getEVTString();
  }
};

/// This class represents one optimizable basic block. It holds all FMAExpr
/// objects created for operations in one MachineBasicBlock.
/// It also keeps references to special terms 0.0 and 1.0 created only once and
/// returned when they are used again.
class CSAFMABasicBlock final : public FMABasicBlock {
  /// Register terms used or defined by FMA expressions in the basic block
  /// are stored into map to avoid creation of duplicated terms and
  /// to have quick search through already existing terms using virtual
  /// registers as keys.
  SmallDenseMap<unsigned, std::unique_ptr<FMARegisterTerm>> RegToRegTerm;

  /// Terms for immediate values.
  std::map<std::pair<MVT, int64_t>, std::unique_ptr<FMAImmediateTerm>> Imms;

public:
  /// Creates an FMA basic block for the given MachineBasicBlock \p MBB.
  CSAFMABasicBlock(MachineBasicBlock &MBB) : FMABasicBlock(MBB) {}

  FMAImmediateTerm *createImm(MVT VT, int64_t Imm) {
    auto &Term = Imms[{VT, Imm}];
    if (!Term)
      Term = std::make_unique<CSAFMAImmediateTerm>(VT, this, Imm);
    return Term.get();
  }

  FMAImmediateTerm *createZero(MVT VT) { return createImm(VT, 0); }

  FMAImmediateTerm *createOne(MVT VT) { return createImm(VT, getOne(VT)); }

  /// Creates an FMA term associated with the virtual register used in
  /// the passed machine operand \p MO. The parameter \p VT specifies
  /// the type of the created term.
  FMARegisterTerm *createReg(MVT VT, const MachineOperand &MO) {
    assert(MO.isReg() && "Cannot create an FMA term for MachineOperand.");
    unsigned Reg = MO.getReg();

    // If there is a term created for this machine operand (or identical to it)
    // then just return the existing term. Otherwise, create a new term.
    auto &Term = RegToRegTerm[Reg];
    if (!Term)
      Term =
          std::make_unique<FMARegisterTerm>(VT, this, Reg, RegToRegTerm.size());
    return Term.get();
  }

  /// Creates an FMA term associated with the virtual register used in
  /// the passed machine operand \p MO. The parameter \p VT specifies
  /// the type of the created term.
  FMATerm *createRegOrImm(MVT VT, const MachineOperand &MO) {
    if (MO.isReg())
      return createReg(VT, MO);
    assert(MO.isImm() && "Unexpected MachineOperand kind.");
    return createImm(VT, MO.getImm());
  }

  /// Walks through all instructions in the machine basic block, finds
  /// MUL/ADD/FMA operations and creates FMA expressions (FMAExpr) for them.
  /// Returns the number of optimizable expressions found in the block.
  /// The parameter \p MRI is passed to this method to make it possible
  /// to find virtual registers associated with FMARegisterTerms and
  /// having uses that are not recognized as FMAExpr operations.
  unsigned parseBasicBlock(const FMAOpcodes &Opcodes, MachineRegisterInfo *MRI);

  /// For CSA we do not care about the killed attribute.
  void setIsKilledAttributeForTerms() override {}

  /// Prints the basic block to the given stream \p OS.
  void print(raw_ostream &OS) const override;
};

unsigned CSAFMABasicBlock::parseBasicBlock(const FMAOpcodes &Opcodes,
                                           MachineRegisterInfo *MRI) {
  LLVM_DEBUG(dbgs() << "FMA-STEP1: FIND FMA OPERATIONS:\n");

  for (auto &MI : getMBB()) {
    if (!MI.getFlag(MachineInstr::FmReassoc) ||
        !MI.getFlag(MachineInstr::FmContract))
      continue;

    MVT VT;
    FMAOpcodes::FMAOpcodeKind Kind;
    bool MulSign, AddSign;

    if (!Opcodes.recognizeOpcode(MI.getOpcode(), VT, Kind, MulSign, AddSign))
      continue;

    // For vector forms make sure that instruction does not have swizzles.
    if (VT.isVector()) {
      unsigned FirstSwizzleOp = 0u;
      switch (Kind) {
      case FMAOpcodes::ADDOpc:
      case FMAOpcodes::SUBOpc:
      case FMAOpcodes::MULOpc:
        FirstSwizzleOp = 3u;
        break;
      case FMAOpcodes::FMAOpc:
      case FMAOpcodes::FMSOpc:
      case FMAOpcodes::FMRSOpc:
        FirstSwizzleOp = 4u;
        break;
      default:
        llvm_unreachable("Unsupported opcode kind.");
      }
      for (unsigned I = 0u; I < 3u; ++I)
        if (MI.getOperand(FirstSwizzleOp + I).getImm())
          continue;
    }

    std::array<FMANode *, 3u> Ops;
    switch (Kind) {
    case FMAOpcodes::MULOpc: // op1 * op2 + 0
      Ops[0] = createRegOrImm(VT, MI.getOperand(1));
      Ops[1] = createRegOrImm(VT, MI.getOperand(2));
      Ops[2] = createZero(VT);
      break;
    case FMAOpcodes::ADDOpc: // op1 * 1 + op2
    case FMAOpcodes::SUBOpc: // op1 * 1 - op2
      Ops[0] = createRegOrImm(VT, MI.getOperand(1));
      Ops[1] = createOne(VT);
      Ops[2] = createRegOrImm(VT, MI.getOperand(2));
      break;
    case FMAOpcodes::NEGOpc: // 0 * 0 - op1
      Ops[0] = createZero(VT);
      Ops[1] = createZero(VT);
      Ops[2] = createRegOrImm(VT, MI.getOperand(1));
      break;
    case FMAOpcodes::FMAOpc:  //  op1 * op2 + op3
    case FMAOpcodes::FMSOpc:  //  op1 * op2 - op3
    case FMAOpcodes::FMRSOpc: // -op1 * op2 + op3
      Ops[0] = createRegOrImm(VT, MI.getOperand(1));
      Ops[1] = createRegOrImm(VT, MI.getOperand(2));
      Ops[2] = createRegOrImm(VT, MI.getOperand(3));
      break;
    default:
      llvm_unreachable("Unsupported opcode kind.");
    }

    // Canonize immediates.
    auto CanonizeImm = [this, VT](FMANode *&Op, bool &Sign) {
      if (auto *Term = dyn_cast<FMAImmediateTerm>(Op)) {
        CSAFMAImmediateTerm NegTerm(VT, this, Term->getImm() ^ getSignMask(VT));
        if (NegTerm.isOne()) {
          Op = createOne(VT);
          Sign ^= true;
        } else if (NegTerm.isZero())
          Op = createZero(VT);
      }
    };
    CanonizeImm(Ops[0], MulSign);
    CanonizeImm(Ops[1], MulSign);
    CanonizeImm(Ops[2], AddSign);

    // Create a new register term for the result of the FMA operation and
    // the FMAExpr node for this operation.
    createFMA(VT, &MI, createReg(VT, MI.getOperand(0)), Ops, MulSign, AddSign);
  }
  setDefHasUnknownUsersForRegisterTerms(MRI);

  LLVM_DEBUG(dbgs() << *this << "FMA-STEP1 DONE.\n");
  return getFMAs().size();
}

void CSAFMABasicBlock::print(raw_ostream &OS) const {
  OS << "\nFMA REGISTER TERMs:\n  ";
  for (const auto &T : RegToRegTerm) {
    T.second->print(OS, true /* PrintAttributes */);
    OS << "\n  ";
  }

  OS << "\nFMA EXPRESSIONs:\n";
  unsigned Index = 0;
  for (const auto &E : getFMAs())
    if (!E->isFullyConsumed()) {
      OS << "  " << Index++ << ": ";
      E->print(OS, true /* PrintAttributes */);
      OS << "\n";
    }
}

/// Loop over all of the basic blocks, performing the FMA optimization for
/// each block separately.
bool CSAGlobalFMA::runOnMachineFunction(MachineFunction &MF) {
  if (!EnableFMAOpt)
    return false;

  ST = &MF.getSubtarget<CSASubtarget>();
  TII = ST->getInstrInfo();
  MRI = &MF.getRegInfo();

  // Target must support FMA ISA.
  if (!ST->hasFMA())
    return false;

  // Compilation options must allow FP contraction and FP expression
  // re-association.
  const TargetOptions &Options = MF.getTarget().Options;
  if (Options.AllowFPOpFusion != FPOpFusion::Fast || !Options.UnsafeFPMath)
    return false;

  // Initialize patterns and opcodes if it has not yet been done.
  if (!Patterns)
    Patterns = std::make_unique<CSAFMAPatterns>();
  if (!Opcodes)
    Opcodes = std::make_unique<FMAOpcodes>();

  // Init insturction latencies.
  AddSubLatency = 4;
  MulLatency = 4;
  FMALatency = 4;

  Control.ForceFMAs = checkFMAControl(FMAControlForceFMAs);
  Control.TuneForLatency = checkFMAControl(FMAControlTuneForLatency);
  Control.TuneForThroughput = checkFMAControl(FMAControlTuneForThroughput);

  // And finally do the transformation.
  return GlobalFMA::runOnMachineFunction(MF);
}

std::unique_ptr<FMABasicBlock>
CSAGlobalFMA::parseBasicBlock(MachineBasicBlock &MBB) {
  // Find MUL/ADD/SUB/FMA/etc operations in the input machine instructions
  // and create internal FMA structures for them.
  // Exit if there are not enough optimizable expressions.
  auto FMABB = std::make_unique<CSAFMABasicBlock>(MBB);
  if (FMABB->parseBasicBlock(*Opcodes, MRI) < 2)
    return nullptr;
  return FMABB;
}

void CSAGlobalFMA::generateOutputIR(FMAExpr &Expr, const FMADag &Dag) {
  auto *FMABB = static_cast<CSAFMABasicBlock*>(Expr.getFMABB());
  auto &MBB = FMABB->getMBB();
  auto *MI = Expr.getMI();

  auto GenMOForTerm = [](const FMATerm *Term) {
    if (auto *RT = dyn_cast<FMARegisterTerm>(Term)) {
      assert(RT->getReg() && "RegTerm with no register");
      return MachineOperand::CreateReg(RT->getReg(), false);
    }
    return MachineOperand::CreateImm(cast<FMAImmediateTerm>(Term)->getImm());
  };

  std::array<unsigned, FMADagCommon::MaxNumOfNodesInDAG> ResultRegs;
  SmallVector<MachineOperand, 6u> MOs;

  for (unsigned NodeInd = Dag.getNumNodes() - 1; NodeInd != ~0U; NodeInd--) {
    MOs.clear();

    bool AIsTerm = false, BIsTerm = false, CIsTerm = false;
    unsigned A = Dag.getOperand(NodeInd, 0, &AIsTerm);
    unsigned B = Dag.getOperand(NodeInd, 1, &BIsTerm);
    unsigned C = Dag.getOperand(NodeInd, 2, &CIsTerm);

    MVT VT = Expr.getVT();
    bool IsAddOrSub = false, IsMul = false;
    bool NegateResult = false, SwapAC = false;

    if (AIsTerm) {
      // If A is 0.0, then the DAG is inefficient, emit error.
      // If A is 1.0, then the DAG is either inefficient or non-standard:
      //   (1.0 * B + C) is non-standard; (B * 1.0 + C) would be Ok.
      //   (1.0 * 0.0 + C) is inefficient;
      //   (1.0 * 1.0 + C) is non-standard; (C * 1.0 + 1.0) would be Ok.
      assert(A != FMADagCommon::TermZERO && A != FMADagCommon::TermONE &&
             "Bad FMA DAG: the operand A cannot be equal to 0.0 or 1.0.");
      MOs.push_back(GenMOForTerm(Expr.getUsedTermByIndex(A)));
    } else
      MOs.push_back(MachineOperand::CreateReg(ResultRegs[A], false));

    if (BIsTerm) {
      assert(B != FMADagCommon::TermZERO &&
             "Bad FMA DAG: the operand B cannot be equal to 0.0.");
      if (B == FMADagCommon::TermONE)
        IsAddOrSub = true;
      else
        MOs.push_back(GenMOForTerm(Expr.getUsedTermByIndex(B)));
    } else
      MOs.push_back(MachineOperand::CreateReg(ResultRegs[B], false));

    if (CIsTerm) {
      if (C == FMADagCommon::TermZERO)
        IsMul = true;
      else {
        auto *Term = C == FMADagCommon::TermONE ? FMABB->createOne(VT)
                                                : Expr.getUsedTermByIndex(C);
        MOs.push_back(GenMOForTerm(Term));
      }
    } else
      MOs.push_back(MachineOperand::CreateReg(ResultRegs[C], false));

    unsigned Opcode = 0;
    if (IsAddOrSub) {
      // Generate ADD(A,C) or SUB(A,C).
      // The ADD(-A,C) case is replaced with SUB(C,A), i.e. just swap A and C.
      // The ADD(-A,-C) case is replaced with ADD(A,C), but we also remember
      // that the result of this FMA node must be negated.
      bool AddSign = Dag.getAddSign(NodeInd);
      if (Dag.getMulSign(NodeInd)) {
        if (AddSign) {
          AddSign = false;
          NegateResult = true;
        } else {
          AddSign = true;
          SwapAC = true;
        }
      }
      Opcode = Opcodes->getOpcode(
          AddSign ? FMAOpcodes::SUBOpc : FMAOpcodes::ADDOpc, VT);
    } else if (IsMul) {
      // Generate MUL(A,B).
      if (!Dag.getMulSign(NodeInd))
        Opcode = Opcodes->getOpcode(FMAOpcodes::MULOpc, VT);
      else {
        // Instead of (0 - MUL(A,B)) it is better to have FMRS(A,B,0).
        Opcode = Opcodes->getOpcode(FMAOpcodes::FMRSOpc, VT);
        MOs.push_back(GenMOForTerm(FMABB->createZero(VT)));
      }
    } else
      Opcode = Opcodes->getFMAOpcode(Dag.getMulSign(NodeInd),
                                     Dag.getAddSign(NodeInd), VT);

    if (SwapAC)
      std::swap(MOs[0], MOs[1]);

    if (VT.isVector())
      std::fill_n(std::back_inserter(MOs), 3u, MachineOperand::CreateImm(0));

    // Create the new instructions.
    auto OldDst = MI->getOperand(0).getReg();
    auto NewDst =
        !NodeInd && !NegateResult ? OldDst : MRI->cloneVirtualRegister(OldDst);

    auto NewMI = BuildMI(MBB, MI, MI->getDebugLoc(), TII->get(Opcode), NewDst)
                     .setMIFlags(MI->getFlags())
                     .add(MOs);
    LLVM_DEBUG(dbgs() << "  GENERATE NEW INSTRUCTION:\n    " << *NewMI);
    (void)NewMI;

    if (NegateResult) {
      unsigned Opc = 0u;
      switch (VT.SimpleTy) {
      case MVT::f16:
        Opc = CSA::XOR16;
        break;
      case MVT::f32:
        Opc = CSA::XOR32;
        break;
      case MVT::f64:
      case MVT::v4f16:
      case MVT::v2f32:
        Opc = CSA::XOR64;
        break;
      default:
        llvm_unreachable("Unsupported type");
      }

      auto XorMI = BuildMI(MBB, MI, MI->getDebugLoc(), TII->get(Opc), OldDst)
                       .addUse(NewDst)
                       .addImm(getSignMask(VT));
      LLVM_DEBUG(dbgs() << "  GENERATE NEW INSTRUCTION:\n    " << *XorMI);
      (void)XorMI;
    }

    if (NodeInd)
      ResultRegs[NodeInd] = NewDst;
  }

  auto DeleteMI = [&MBB](MachineInstr *MI) {
    LLVM_DEBUG(dbgs() << "  DELETE the MI (it is replaced): \n    " << *MI);
    MBB.erase(MI);
  };
  for (auto *CMI : Expr.getConsumedMIs())
    DeleteMI(CMI);
  DeleteMI(MI);
}

} // End anonymous namespace.

MachineFunctionPass *llvm::createCSAGlobalFMAPass() {
  return new CSAGlobalFMA();
}
