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
#include "llvm/ADT/SmallBitVector.h"
#include "llvm/CodeGen/Intel_FMACommon.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetInstrInfo.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "csa-global-fma"

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
  case MVT::f32:
    return 0x3f800000L;
  case MVT::f64:
    return 0x3ff0000000000000LL;
  case MVT::v2f32:
    return 0x3f8000003f800000LL;
  default:
    llvm_unreachable("Unsupported type");
  }
}

/// Returns immediate value representing sign bit(s) for the given type.
static int64_t getSignMask(MVT VT) {
  switch (VT.SimpleTy) {
  case MVT::f32:
    return 0x80000000L;
  case MVT::f64:
    return 0x8000000000000000LL;
  case MVT::v2f32:
    return 0x8000000080000000LL;
  default:
    llvm_unreachable("Unsupported type");
  }
}

namespace {

class FMABasicBlock;
class FMAExpr;
class FMATerm;
class FMARegTerm;
class FMAImmTerm;
class FMAExprSP;
class FMADag;
class FMAPerfDesc;

/// This class holds all pre-computed/efficient FMA patterns/DAGs encoded in
/// 64-bit integer values.
///
/// The DAGs are grouped by their SHAPEs (see FMAExprSPCommon::Shape
/// for details). The groups/sets of DAGs are also sorted by SHAPEs which makes
/// it possible to use binary search for finding of DAGs match-able with some
/// input and potentially inefficient expressions.
///
/// The class has quite simple external interface having only two methods:
///
///   // Loads pre-computed DAGs from an auto-generated header file.
///   void init();
///
///   // Returns the most efficient DAG for the given sum of products.
///   FMADag *getDagForBestSPMatch(const FMAExprSP &SP);
class FMAPatterns final {
  /// Represents a set of FMA patterns that all have the same SHAPE.
  using FMAPatternsSet = ArrayRef<uint64_t>;

  /// All FMA patterns are stored as a vector of references to groups of Dags
  /// where each of the groups has the same SHAPE.
  /// It is also supposed that the groups of Dags are sorted by the SHAPE.
  SmallVector<FMAPatternsSet, 0> Dags;

  /// The largest available shape that corresponds to some
  /// pattern kept in the patterns storage.
  uint64_t LargestAvailableShape;

  /// This map contains the sums of products created during the binary search
  /// performed on the pre-computed DAGs. The sums of products are saved to
  /// this map to speed up the searches that are performed not for the first
  /// time.
  DenseMap<uint64_t, std::unique_ptr<FMAExprSP>> EncodedDagToSPMap;

  /// Returns a set of 64-bit encoded DAGs for the given \p Shape.
  /// If such set cannot be found then nullptr is returned.
  const FMAPatternsSet *getDagsForShape(uint64_t Shape);

  /// Returns a sum of product generated for 64-bit int encoded DAG
  /// \p EncodedDag.
  const FMAExprSP *acquireSP(uint64_t EncodedDag);

public:
  FMAPatterns();

  /// Returns an FMA DAG that would be the most efficient equivalent of the
  /// given sum of products \p SP.
  std::unique_ptr<FMADag> getDagForBestSPMatch(const FMAExprSP &SP);

  /// Returns the largest available shape available in the patterns storage.
  uint64_t getLargestShape() const { return LargestAvailableShape; }
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
    {CSA::ADDF32,    MVT::f32,   ADDOpc},
    {CSA::ADDF64,    MVT::f64,   ADDOpc},
    {CSA::ADDF32X2,  MVT::v2f32, ADDOpc},
    // SUB
    {CSA::SUBF32,    MVT::f32,   SUBOpc},
    {CSA::SUBF64,    MVT::f64,   SUBOpc},
    {CSA::SUBF32X2,  MVT::v2f32, SUBOpc},
    // MUL
    {CSA::MULF32,    MVT::f32,   MULOpc},
    {CSA::MULF64,    MVT::f64,   MULOpc},
    {CSA::MULF32X2,  MVT::v2f32, MULOpc},
    // NEG
    {CSA::NEGF32,    MVT::f32,   NEGOpc},
    {CSA::NEGF64,    MVT::f64,   NEGOpc},
    // FMA
    {CSA::FMAF32,    MVT::f32,   FMAOpc},
    {CSA::FMAF64,    MVT::f64,   FMAOpc},
    {CSA::FMAF32X2,  MVT::v2f32, FMAOpc},
    // FMS
    {CSA::FMSF32,    MVT::f32,   FMSOpc},
    {CSA::FMSF64,    MVT::f64,   FMSOpc},
    {CSA::FMSF32X2,  MVT::v2f32, FMSOpc},
    // FMRS
    {CSA::FMRSF32,   MVT::f32,   FMRSOpc},
    {CSA::FMRSF64,   MVT::f64,   FMRSOpc},
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
class CSAGlobalFMA final : public MachineFunctionPass {
public:
  CSAGlobalFMA()
      : MachineFunctionPass(ID), ST(nullptr), TII(nullptr), MRI(nullptr),
        Patterns(nullptr) {}

  StringRef getPassName() const override { return "CSA GlobalFMA"; }

  bool runOnMachineFunction(MachineFunction &MFunc) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

  /// The latency of ADD and SUB operations in the target CPU.
  unsigned AddSubLatency;

  /// The latency of MUL operations in the target CPU.
  unsigned MulLatency;

  /// The latency of FMA operations in the target CPU.
  unsigned FMALatency;

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

  /// A storage with pre-computed/efficient FMA patterns.
  std::unique_ptr<FMAPatterns> Patterns;

  /// FMA opcodes.
  std::unique_ptr<FMAOpcodes> Opcodes;

  /// Do the FMA optimization in one basic block.
  /// Return true iff any changes in the IR were made.
  bool optBasicBlock(MachineBasicBlock &MBB);

  /// Do the FMA optimizations in one parsed basic block \p FMABB.
  /// In particular, it tries to combine simple MUL/ADD/FMA operations
  /// registered in \p FMABB into bigger expressions, to find efficient
  /// replacements for them and if any optimizations are doable then update
  /// the input machine basic block \p MBB.
  /// Return true iff any changes in the IR (i.e. in \p MBB) were made.
  bool optParsedBasicBlock(FMABasicBlock &FMABB);

  /// Returns FMADag for the given expression \p Expr.
  /// The parameter \p DoCompactTerms specifies if it is Ok to do terms
  /// compaction and removal of the optimizable terms from 'UsedTerms' if such
  /// terms happen to be unused in the returned DAG.
  /// For example: the term b can be eliminated from expression (a+b)-(b-c).
  /// DAG for such expression would be: (a+c). It is safe to compact terms
  /// only when the DAG is computed for the expression for the last time.
  /// The parameter may be set to true when the terms compaction is needed for
  /// more efficient and robust pattern matching. It also may be set to
  /// false when the returned DAG is not finalized yet and removing terms
  /// may cause problems.
  std::unique_ptr<FMADag> getDagForExpression(FMAExpr &Expr,
                                              bool DoCompactTerms) const;

  /// Returns FMADag for the result of fusing \p Expr and \p FWSExpr.
  /// \p FWSExpr must be an expression defining the term used by \p Expr.
  std::unique_ptr<FMADag> getDagForFusedExpression(FMAExpr &Expr,
                                                   FMAExpr &FWSExpr) const;

  /// If fusing the expression \p FWSExpr and users of it is profitable and
  /// gives good opportunities for pattern matching then this routine does
  /// such fusing and returns true. Otherwise, it just returns false.
  /// The parameter \p Users gives the set of users of \p FWSExpr.
  /// The parameter \p BadUsers is output and is populated with the users
  /// of \p FWSExpr that cannot or should not be fused with \p FWSExpr.
  bool doFWSAndConsumeIfProfitable(FMAExpr &FWSExpr,
                                   SmallPtrSetImpl<FMAExpr *> &Users,
                                   SmallPtrSetImpl<FMAExpr *> &BadUsers,
                                   SmallPtrSetImpl<FMAExpr *> &InefficientUsers,
                                   bool CanConsumeIfOneUser);

  /// Performs the Forward Substitution transformation of the FMA expressions
  /// in the given FMA basic block \p FMABB.
  /// This transformation fuses small FMA expressions into bigger expressions
  /// that can then be optimized.
  /// For example,
  ///   t1 = a * b;
  ///   t2 = t1 + c;
  /// -->
  ///   t2 = a * b + c;
  void doFWS(FMABasicBlock &FMABB);

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
  void generateOutputIR(const FMAExpr &Expr, const FMADag &Dag,
                        FMABasicBlock &FMABB);

  /// Returns true if the DAG \p Dag seems more efficient than the initial
  /// expression \p Expr being optimized now.
  /// The parameter \p TuneForLatency specifies if the latency aspect has
  /// the priority over the throughput.
  /// The parameter \p TuneForThroughput specifies if the throughput aspect has
  /// the priority over the latency.
  /// If \p TuneForLatency and \p TuneForThroughput are both set or both unset,
  /// then both aspects are the same important and the final decision depends
  /// on some heuristics.
  bool isDagBetterThanInitialExpr(const FMADag &Dag, const FMAExpr &Expr,
                                  bool TuneForLatency,
                                  bool TuneForThroughput) const;

  /// For the given DAG \p Dag this method returns the descriptor describing
  /// various performance metrics of the DAG.
  FMAPerfDesc getDagPerfDesc(const FMADag &Dag) const;

  /// For the given expression \p Expr this method returns the descriptor
  /// describing various performance metrics of the expression.
  FMAPerfDesc getExprPerfDesc(const FMAExpr &Expr) const;
};

char CSAGlobalFMA::ID = 0;

/// This class describes the performance metrics of some expression tree.
/// In particular, it keeps information about the number of various operations
/// and latency of the expression tree.
class FMAPerfDesc final {
  /// Latency of the expression tree.
  unsigned Latency;

  /// The number of ADD and SUB operations.
  unsigned NumAddSub;

  /// The number of MUL operations.
  unsigned NumMul;

  /// The number of FMA operations.
  unsigned NumFMA;

public:
  /// Default constructor.
  FMAPerfDesc() : Latency(0), NumAddSub(0), NumMul(0), NumFMA(0) {}

  /// Constructor that fully initializes the object.
  FMAPerfDesc(unsigned Latency, unsigned NumAddSub, unsigned NumMul,
              unsigned NumFMA)
      : Latency(Latency), NumAddSub(NumAddSub), NumMul(NumMul), NumFMA(NumFMA) {
  }

  /// Returns the latency of the expression tree.
  unsigned getLatency() const { return Latency; }

  /// Returns the number of ADD and SUB operations in the expression tree.
  unsigned getNumAddSub() const { return NumAddSub; }

  /// Returns the number of MUL operations in the expression tree.
  unsigned getNumMul() const { return NumMul; }

  /// Returns the number of FMA operations in the expression tree.
  unsigned getNumFMA() const { return NumFMA; }

  /// Returns the number of all operations in the expression tree.
  unsigned getNumOperations() const { return NumAddSub + NumMul + NumFMA; }

  /// Returns true if 'this' performance metrics seem better than \p OtherDesc.
  /// The parameters \p TuneForLatency and \p TuneForThroughput specify
  /// what aspect Latency or Throughput has the priority.
  bool isBetterThan(const FMAPerfDesc &OtherDesc, bool TuneForLatency,
                    bool TuneForThroughput) const;

  /// Prints the performance metrics to the given output stream \p OS.
  void print(raw_ostream &OS) const {
    OS << "#Operations = " << getNumOperations() << " (" << NumAddSub
       << "xADD|SUB + " << NumMul << "xMUL + " << NumFMA
       << "xFMA); Latency = " << Latency << ".";
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD void dump() const { print(dbgs()); }
#endif
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
/// Prints the FMA Performance Descriptor \p PerfDesc to the given stream \p OS.
static raw_ostream &operator<<(raw_ostream &OS, const FMAPerfDesc &PerfDesc) {
  PerfDesc.print(OS);
  return OS;
}
#endif

/// This class is derived from FMADagCommon representing FMA Directed Acyclic
/// Graphs. It adds some methods specific for code-generation.
class FMADag final : public FMADagCommon {
public:
  /// Creates an FMA DAG for 64-bit encoded DAG from precomputed FMA DAGs.
  FMADag(uint64_t Encoded64) : FMADagCommon(Encoded64) {}

  /// Creates a copy of the given DAG \p Dag.
  FMADag(const FMADagCommon &Dag) : FMADagCommon(Dag) {}

  /// Returns true if the term or expression used in \p NodeInd and \p OpndInd
  /// operand is used the last time in this DAG.
  /// The word "last" should be understood in the way how code-generation phase
  /// would understand it. Code-generation translate the DAG to IR starting
  /// from the node with maximum index. So the term with
  /// (\p NodeInd, \p OpndInd) coordinates is the last if it is not used
  /// in nodes with indices 0 to (\p NodeInd - 1), and is not used
  /// in the operands 0 to (\p OpndInd - 1) in the node with index \p NodeInd.
  bool isLastUse(unsigned NodeInd, unsigned OpndInd) const;

  /// Returns true iff the FMA DAG node \p NodeInd is a pure MUL operation,
  /// i.e. the addend is zero and neither multiplicand is zero or one.
  bool isMul(unsigned NodeInd) const;

  /// Returns true iff the FMA DAG node \p NodeInd is a pure ADD operation,
  /// i.e. neither of three operands is zero and one of multiplicands is equal
  /// to one.
  bool isAdd(unsigned NodeInd) const;

  /// Returns true iff the FMA DAG node \NodeInd is an FMA operation.
  bool isFMA(unsigned NodeInd) const;

  /// Returns the latency of the expression tree starting from the node
  /// \p NodeInd.
  /// The parameters \p MulLatency, \p AddSubLatency, FMALatency specify
  /// the latency of MUL, ADD-or-SUB, FMA operations.
  unsigned getLatency(unsigned MulLatency, unsigned AddSubLatency,
                      unsigned FMALatency, unsigned NodeInd = 0) const;
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
/// Prints the FMA node \p Node to the given stream \p OS.
static raw_ostream &operator<<(raw_ostream &OS, const FMADag &Dag) {
  Dag.print(OS);
  return OS;
}
#endif

/// This class is derived from FMAExprSPCommon representing expressions
/// consisting of MUL/ADD/SUB/FMA operations in canonical form, i.e. sum of
/// products. It adds only some methods specific for FMA optimization.
class FMAExprSP final : public FMAExprSPCommon {
public:
  /// Default constructor. Creates an empty sum of products.
  FMAExprSP() : FMAExprSPCommon() {}

  /// Creates a sum of products consisting of only one product with the
  /// only term \p Term.
  FMAExprSP(unsigned Term) : FMAExprSPCommon(Term) {}

  /// Initializes the sum of products using the given DAG \p EncodedDag
  /// encoded in 64-bit integer.
  void initForEncodedDag(uint64_t EncodedDag) {
    assert(!Dag && "initForEncodedDag() is applied to initialized SP");
    Dag = new FMADag(EncodedDag);
    bool isOk = initForDag(*Dag);
    (void)isOk;
    assert(isOk && "Could not initialize SP for 64-bit encoded DAG.");

    canonize();
    computeShape();
  }

  /// Canonizes the sum of products. Here that means that the terms in each
  /// of the products and the products itself must be lexicographically
  /// ordered. This method also can remove products which are identical but
  /// have opposite signs.
  void canonize() override;

  /// Returns a reference to an array that can be used to do terms mapping
  /// which may be needed to compact regular terms after some of such terms
  /// got removed completely from the sum of products.
  /// If this method cannot find any unused terms then it returns nullptr,
  /// which means that terms compression is not possible and not needed.
  /// The returned array TM (TermsMapping) has the elements defined such a way
  /// that the desired SP would be the result of using this rule:
  ///   NewTermIndex = TM[OldTermIndex];
  /// For the terms removed from SP a special value ~0U is written, i.e. if
  /// the term with index Ti is removed, then TM[Ti] is set to ~0U;
  ///
  /// Example. The method canonize() may remove 4 products from SP:
  ///   +aa-aa+bc+dd-dd+e;
  /// SP after canonize() is:
  ///   +bc+e;
  /// The terms 'a', and 'd' are unused. After this method discovers that, it
  /// returns the following terms mapping:
  ///   TM[] = {~0U, 0, 1, ~0U, 2}
  /// The caller of this method then can use this array such a way:
  ///   NewTerm = TM[OldTerm];
  /// The caller of this method can use this rule for the initial SP and
  /// transform it to:
  ///   +ab+c;
  std::unique_ptr<unsigned[]> getTermsMappingToCompactTerms();
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
/// Prints the FMA SP \p SP to the given stream \p OS.
static raw_ostream &operator<<(raw_ostream &OS, const FMAExprSP &SP) {
  SP.print(OS);
  return OS;
}
#endif

/// This class represents FMA expressions and terms. It works as a bridge
/// between input IR and internal FMA structures, in particular it helps
/// to connect instances of MachineInstr/MachineOperand classes with machine
/// independent classes FMAExprSP/FMADag, get the most efficient representation
/// of input expression and to generate output IR.
class FMANode {
public:
  enum FMANodeKind {
    NK_Expr,
    NK_RegTerm,
    NK_ImmTerm
  };
  FMANodeKind getKind() const { return Kind; }

private:
  FMANodeKind Kind;

protected:
  /// Machine value type of the FMA expression or term.
  MVT VT;

public:
  /// Constructor. Initializes the only field \p VT available in this class.
  FMANode(FMANodeKind Kind, MVT VT) : Kind(Kind), VT(VT) {}
  virtual ~FMANode() = default;

  bool isZero() const;
  bool isOne() const;

  /// Returns machine value type of the FMA node.
  MVT getVT() const { return VT; }

  /// Prints the FMA expression or term to the given stream \p OS.
  /// The parameter \p PrintAttributes specifies if the caller wants to see
  /// more information and some of FMA node attributes should be printed out.
  virtual void print(raw_ostream &OS, bool PrintAttributes = false) const = 0;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD void dump() const { print(dbgs()); }
#endif
};

/// Prints the FMA node \p Node to the given stream \p OS.
static raw_ostream &operator<<(raw_ostream &OS, const FMANode &Node) {
  Node.print(OS);
  return OS;
}

/// This class represents a term or a leaf of an FMA expression tree.
class FMATerm : public FMANode {
public:
  FMATerm(FMANodeKind Kind, MVT VT) : FMANode(Kind, VT) {}

  static bool classof(const FMANode *N) {
    return N->getKind() >= NK_RegTerm && N->getKind() <= NK_ImmTerm;
  }
};

/// This class represents an FMA term associated with a virtual register.
class FMARegTerm final : public FMATerm {
  /// A virtual register associated with the result of the FMA term.
  /// The special value 0 means that the virtual register has not been
  /// assigned yet. For memory terms this field is equal to 0 until the load
  /// is generated and this field gets assigned to the virtual register
  /// associated with the result of the load. Similarly for special const
  /// terms, this field is also equal to 0 until the code for the constant
  /// is generated and this fields is assigned to the virtual register
  /// associated with the const.
  unsigned Reg;

  /// The order number of the term in the FMABasicBlock.
  /// This field is used only for having convenient dumps of FMA basic block
  /// and FMA expressions.
  unsigned TermIndexInBB;

  /// This field is set to true iff the virtual register associated with this
  /// term is defined by an instruction recognized by this FMA optimization and
  /// is used by at least one instruction not recognized by the optimization.
  bool DefHasUnknownUsers;

public:
  /// Creates FMARegTerm node for a term associated with a virtual
  /// register \p Reg.
  /// The parameter \p VT specifies the type of the created expression.
  /// The parameter \p TermIndexInBB defines the order number of the term in
  /// the FMA basic block being currently optimized.
  FMARegTerm(MVT VT, unsigned Reg, unsigned TermIndexInBB)
      : FMATerm(NK_RegTerm, VT), Reg(Reg), TermIndexInBB(TermIndexInBB),
        DefHasUnknownUsers(false) {}

  /// Returns the virtual register associated with FMA term.
  unsigned getReg() const { return Reg; }

  /// Binds FMA term with a virtual register \p Reg.
  void setReg(unsigned NewReg) { Reg = NewReg; }

  /// Marks this register term as one having uses not detected as
  /// FMAExpr. So, if register term t1 defined by some FMAExpr
  /// (t1 = FMAExpr(...)) has been marked, then the expression defining
  /// this term cannot be deleted in this optimization.
  void setDefHasUnknownUsers() { DefHasUnknownUsers = true; }

  /// Returns true iff the register term is marked as one having
  /// unknown (i.e. non-FMAExpr) users.
  bool getDefHasUnknownUsers() const { return DefHasUnknownUsers; }

  /// Prints the FMA term to the given stream \p OS.
  /// The parameter \p PrintAttributes specifies if the caller wants to see
  /// more information and some of FMA node attributes should be printed out.
  void print(raw_ostream &OS, bool PrintAttributes) const override {
    OS << "T" << TermIndexInBB << ":%"
       << TargetRegisterInfo::virtReg2Index(Reg);
    if (PrintAttributes) {
      OS << " // Type: " << EVT(VT).getEVTString();
      if (DefHasUnknownUsers)
        OS << "; DefHasUknownUsers = 1!";
    }
  }

  static bool classof(const FMANode *N) { return N->getKind() == NK_RegTerm; }
};

/// This class represents a special FMA term associated with a floating point
/// constant, e.g. 0.0 or 1.0.
class FMAImmTerm final : public FMATerm {
  //// Immediate value.
  int64_t Imm;

public:
  FMAImmTerm(MVT VT, int64_t Imm) : FMATerm(NK_ImmTerm, VT), Imm(Imm) {}

  int64_t getImm() const { return Imm; }

  bool isZero() const { return Imm == 0; }

  bool isOne() const { return Imm == getOne(getVT()); }

  bool isNegativeZero() const {
    return FMAImmTerm(getVT(), getSignMask(getVT()) ^ Imm).isZero();
  }

  bool isNegativeOne() const {
    return FMAImmTerm(getVT(), getSignMask(getVT()) ^ Imm).isOne();
  }

  /// Prints the FMA expression or term to the given stream \p OS.
  /// The parameter \p PrintAttributes specifies if the caller wants to see
  /// more information and some of FMA node attributes should be printed out.
  void print(raw_ostream &OS, bool PrintAttributes) const override {
    OS << format_hex(Imm, 2u + VT.getSizeInBits() / 4);
    if (PrintAttributes)
      OS << " // Type: " << EVT(VT).getEVTString();
  }

  static bool classof(const FMANode *N) { return N->getKind() == NK_ImmTerm; }
};

/// This class represents an FMA expression having 3 operands:
///   (MulSign)(A * B) + (AddSign)C;
/// The operands A, B, C can be of any class derived from FMANode class,
/// i.e. A, B, C can point to instances of FMAExpr, FMATerm, FMAImmTerm,
/// etc. classes.
///
/// The key goals of having this class:
/// - To represent simple FMA operations and big FMA expression trees existing
///   in input IR and to simplify the analysis of such expression trees.
/// - To convert input IR to IR independent classes FMAExprSP/FMADag.
///   In particular, it must be possible to lower expression trees represented
///   with the help of FMAExpr to a canonical form (sum of products).
///   Also, the leaves of the optimizable expression trees must be represented
///   in such a way that it would be possible to order the terms/leaves and
///   correlate unique leaves represented with FMATerm and unsigned terms used
///   in FMAExprSP.
/// - To transform the input IR accordingly to found efficient patterns.
///   This goal requires having references to IR:
///   - MachineInstr to have references to IR operations and to have insertion
///     point for newly generated instructions. Also, references to
///     MachineInstr are needed to remove the old/inefficient IR operations.
///   - Other references to IR if that helps to make the code-generation more
///     efficient.
class FMAExpr final : public FMANode {
  /// Sign used for the product of the 1st and 2nd operands. The value 'true'
  /// is used for negative products.
  bool MulSign;

  /// Sign used for the 3rd operand of FMA. The value 'true' is used when
  /// the 3rd operand is subtracted from the product of the 1st and 2nd
  /// operands, and 'false' is used when the 3rd operand is added.
  bool AddSign;

  /// References to 3 operands of FMA operation. The first two operands are
  /// multiplied and third operand is added to the product of the first and
  /// second operands:   (MulSign)Operand[0]*Operand[1] + (AddSign)Operand[2].
  std::array<FMANode *, 3u> Operands;

  /// This field is set to true only if there is at least one FMA expression
  /// using 'this' FMA expression directly and there are no FMA expressions
  /// that would use the term defined by 'this' expression.
  bool IsFullyConsumedByKnownExpressions;

  /// A reference to an FMA term defined by 'this' FMA expression.
  FMARegTerm *ResultTerm;

  /// A vector of FMA Terms used in this FMA expression and subexpressions.
  /// Indices of terms in this vector are used as unsigned terms in FMAExprSP
  /// and FMADag objects created for this FMA expression later.
  /// This vector is maintained only for FMA expressions that are not fully
  /// consumed by other expressions or not having non-FMA related users,
  /// i.e. when !IsFullyConsumedByKnownExpressions || HasUnknownUsers.
  ///
  /// The following 3 operations are supposed to be the most common:
  /// a) Add a term to the vector if the term is not there yet.
  /// b) Get a term by an index.
  /// c) Get the index of some term that is known to be in the container.
  //
  /// FIXME: vector is not a perfect container. The complexity of the
  /// operations (a), (b), (c) are accordingly Linear, Constant, Linear.
  /// The size of this vector is expected to be not much bigger than
  /// FMADagCommon::MaxNumOfUniqueTermsInDAG as this optimization does not
  /// support efficient code generation for huge expressions. Efficient
  /// expression partitioning must be implemented to avoid that limit. That
  /// would be a good moment to fix the problem with Linear complexity for (a)
  /// and (c) operations. Note that such solution must handle the problem of
  /// non-determinism of code-generation which may be caused by the order in
  /// which the elements are stored in the container. The elements stored here
  /// are addresses and thus the elements order is really random.
  SmallSetVector<FMATerm *, 16u> UsedTerms;

  /// A reference to machine instruction which is used as a reference point to
  /// an original MUL/ADD/FMA operation; it is used in FWS (Forward
  /// Substitution) that creates bigger FMAs by pulling some of FMA operations
  /// down to their users.
  MachineInstr *MI;

  /// A reference to the DBG_VALUE instruction, which is associated with the
  /// register defined by the FMA instruction MI. If the current FMA expression
  /// gets consumed by some other FMA, then DbgValMI must be added
  /// to ConsumedMIs too. So, we would be sure that it gets removed when not
  /// used anymore.
  MachineInstr *DbgValMI;

  /// A set of machine instructions corresponding to other FMA expressions
  /// consumed by this FMA expression. The machine instructions in this list
  /// must be removed from IR when/if this expression is translated back to IR.
  /// This set is maintained only for root FMA expressions, i.e. when the field
  /// IsRootExpr is set to 'true'.
  std::list<MachineInstr *> ConsumedMIs;

  /// Expression index in the parent FMABasicBlock.
  unsigned Index;

  /// Returns an index for the given term \p Term. It is asserted that the
  /// provided term is used as an operand of one of FMA operations included
  /// into the expression tree referenced by 'this' FMA expression.
  ///
  /// Conversion of the term to an unsigned index may be needed to bind
  /// FMA terms represented as FMAExpr and terms represented as unsigned
  /// in FMAExprSP/FMADag classes.
  unsigned getUsedTermIndex(const FMATerm *Term) const {
    auto I = find(UsedTerms, Term);
    assert(I != UsedTerms.end() &&
           "Cannot find FMA term in the list of used terms.");
    return std::distance(UsedTerms.begin(), I);
  }

  /// Recursively walks through the expression nodes, builds sums of products
  /// for them and puts the created SPs to the given map \p ExprToSPMap.
  /// Returns the sum of products generated for 'this' FMA expression.
  /// The parameter \p RootFMAExpr is a reference to a root FMA expression,
  /// which holds the container with all used terms, which is needed to
  /// convert terms into unsigned indices/terms used in the result
  /// sum of products.
  FMAExprSP *generateSPRecursively(
      const FMAExpr *RootExpr,
      SmallDenseMap<const FMANode *, std::unique_ptr<FMAExprSP>> &Node2SP)
      const;

  /// Returns true if the current expression has too big shape and thus it
  /// does not have any suitable patterns available in the patterns storage
  /// \p Patterns.
  bool isExprTooLarge(const FMAPatterns &Patterns) const {
    auto SP = generateSP();
    return !SP || Patterns.getLargestShape() < SP->Shape;
  }

public:
  /// Create FMAExpr for ADD/SUB/FMA instruction.
  /// The parameter \p VT specifies the type of the created operation.
  /// The parameter \p MI passes a reference to machine instruction for which
  /// this FMAExpr is created.
  /// The parameter \p Res gives the reference to an FMARegTerm
  /// node created for the result of the FMAExpr operation being created here.
  /// The parameter \p Ops gives the references to FMA expression operands.
  FMAExpr(MVT VT, MachineInstr *MI, FMARegTerm *Res,
          const std::array<FMANode *, 3u> &Ops, bool MulSign, bool AddSign,
          unsigned Index);

  /// Returns the operand of FMA operation with the index \p Index.
  FMANode *getOperand(unsigned Index) { return Operands[Index]; }

  /// Returns the const operand of FMA operation with the index \p Index.
  const FMANode *getOperand(unsigned Index) const { return Operands[Index]; }

  /// Returns the term associated with the result of this FMA expression.
  FMARegTerm *getResultTerm() const { return ResultTerm; }

  /// Returns the reference to machine instruction associated with
  /// FMA expression.
  MachineInstr *getMI() const { return MI; }

  /// Associates the DBG_VALUE machine instruction with the current
  /// FMA expression.
  void setDbgValMI(MachineInstr *I) { DbgValMI = I; }

  /// Returns expression index in the parent FMABasicBlock.
  unsigned getIndex() const { return Index; }

  /// This method puts 'this' node and all subexpressions to the given
  /// set \p ExprSet. It may be needed when each expression node must be
  /// visited only once.
  void putExprToExprSet(SmallPtrSetImpl<const FMAExpr *> &ExprSet) const {
    SmallVector<const FMANode *, 16u> WorkList({this});
    do {
      auto *Expr = cast<FMAExpr>(WorkList.pop_back_val());
      if (ExprSet.insert(Expr).second)
        copy_if(Expr->Operands, std::back_inserter(WorkList),
                [](FMANode *N) { return isa<FMAExpr>(N); });
    } while (!WorkList.empty());
  }

  /// Returns the latency of the FMA expression tree.
  /// The parameters \p AddSubLatency, \p MulLatency, \p FMALatency specify
  /// the latencies of add/subtract, multiply, FMA operations.
  unsigned getLatency(unsigned AddSubLatency, unsigned MulLatency,
                      unsigned FMALatency) const;

  /// Returns true if 'this' expression uses the given term \p Term.
  bool isUserOf(FMATerm *Term) const {
    return UsedTerms.count(Term);
  }

  /// Returns true iff there are users of this expression not visible to
  /// the FMA optimization.
  bool hasUnknownUsers() const { return ResultTerm->getDefHasUnknownUsers(); }

  /// Replaces the uses of the node \p Old with \p New.
  void replaceAllUsesOfWith(const FMANode *Old, FMANode *New) {
    SmallVector<FMAExpr *, 16u> WorkList({this});
    do {
      for (auto &Opnd : WorkList.pop_back_val()->Operands)
        if (Opnd == Old)
          Opnd = New;
        else if (auto *Expr = dyn_cast<FMAExpr>(Opnd))
          WorkList.push_back(Expr);
    } while (!WorkList.empty());
  }

  /// Starts the fusing of 'this' expression and \p FWSExpr, keeping the
  /// opportunity to cancel the changes if necessary.
  /// The state of the vector 'UsedTerms' is saved in \p UsedTermsBackup before
  /// the fusing if two expressions.
  void startConsume(FMAExpr &FWSExpr,
                    SmallVectorImpl<FMATerm *> &UsedTermsBackUp);

  /// Reverts the fusing of 'this' and \p FWSExpr expressions.
  /// The parameter \p UsedTermsBackUp gives the terms that were used by
  /// 'this' expression before fusing of these two expressions.
  void cancelConsume(FMAExpr &FWSExpr,
                     SmallVectorImpl<FMATerm *> &UsedTermsBackUp);

  /// Commits the fusing of 'this' and \p FWSExpr expressions.
  /// The parameter \p HasOtherFMAUsers specifies if there are other
  /// FMA expressions using the term defined by \p FWSExpr.
  void commitConsume(FMAExpr &FWSExpr, bool HasOtherFMAUsers);

  /// Includes the given expression \p Expr into 'this' expression. It is
  /// assumed that the term defined by \p Expr is used in 'this' expression.
  /// Returns true iff the consumption happened successfully.
  /// The parameter \p HasOtherFMAUsers specifies if the term defined by
  /// \p Expr is used in some other FMA expressions ('this' is not counted).
  bool consume(FMAExpr &Expr, const FMAPatterns &Patterns,
               bool HasOtherFMAUsers = false);

  /// Returns a const reference to the list containing machine instructions
  /// consumed by this expression and that may need to be replaced by the
  /// code generated for this expression.
  const std::list<MachineInstr *> &getConsumedMIs() const {
    return ConsumedMIs;
  }

  /// Returns true iff 'this' FMA expression is included into bigger
  /// FMA expressions and does not show up as independent expression,
  /// i.e. it does not have any unknown users.
  bool isFullyConsumed() const {
    return IsFullyConsumedByKnownExpressions && !hasUnknownUsers();
  }

  /// Returns true iff 'this' FMA expression exists only as subexpression
  /// of other known FMA expressions, i.e. the term defined by 'this' expression
  /// is not used by any other known FMA expression.
  bool isFullyConsumedByKnownExpressions() const {
    return IsFullyConsumedByKnownExpressions;
  }

  /// Marks the expression as consumed by known expressions, which means
  /// that this expression exists only as a subexpression of other FMA
  /// expressions. BTW, this does not mean that there are no other users at all.
  void markAsFullyConsumedByKnownExpressions() {
    IsFullyConsumedByKnownExpressions = true;
  }

  // Returns true iff 'this' FMA expression has consumed some other expression.
  // In this case it is considered as optimizable.
  // Otherwise, 'this' expression is original and consists of only one operation
  // MUL, SUB, ADD, or FMA, which cannot be optimized.
  bool isOptimizable() const {
    for (auto *Op : Operands)
      if (isa<FMAExpr>(Op))
        // If IsFullyConsumed() then it must be optimized only as part
        // of the bigger expressions including 'this' one.
        return !isFullyConsumed();
    return false;
  }

  /// Looks for an expression in the given basic block \p FMABB that could be
  /// included into 'this' expression. Returns a reference to such expression
  /// or nullptr if no consumable expressions were found.
  /// Expressions that are not used or used not only by 'this' expression are
  /// not considered as candidates for being consumed.
  /// The parameter \p MRI is passed to this method to make it possible to
  /// count the number of users of registers associated with the results of
  /// candidates for consumption.
  FMAExpr *
  findFWSCandidate(FMABasicBlock &FMABB,
                   const SmallPtrSetImpl<FMAExpr *> &BadFWSCandidates,
                   const SmallPtrSetImpl<FMAExpr *> &InefficientFWSCandidates,
                   bool CanConsumeIfOneUser, SmallPtrSetImpl<FMAExpr *> &Users);

  /// Returns the used term by the index \p Index.
  FMATerm *getUsedTerm(unsigned Index) const { return UsedTerms[Index]; }

  /// Compacts terms in the given sum of products \p SP generated for 'this'
  /// FMA expression and does corresponding changes in 'this' FMA expression as
  /// well. Terms compaction may be needed after call of FMAExprSP::canonize()
  /// method that may remove some of products and remove some of terms.
  /// For example,
  ///     Before canonize(): +ab+c-c+d
  ///     After  canonize(): +ab+d
  /// The term 'c' got totally removed here.
  /// Terms compact will rename the term 'd' with 'c':
  ///     After compactTerms(): +ab+c.
  void compactTerms(FMAExprSP &SP);

  /// Generates and returns a sum of products for 'this' FMA expression.
  std::unique_ptr<FMAExprSP> generateSP() const;

  /// Prints the FMA expression or term to the given stream \p OS.
  /// The parameter \p PrintAttributes specifies if the caller wants to see
  /// more information and some of FMA node attributes should be printed out.
  void print(raw_ostream &OS, bool PrintAttributes) const override;

  static bool classof(const FMANode *N) { return N->getKind() == NK_Expr; }
};

bool FMANode::isZero() const {
  if (auto *Imm = dyn_cast<FMAImmTerm>(this))
    return Imm->isZero();
  return false;
}

bool FMANode::isOne() const {
  if (auto *Imm = dyn_cast<FMAImmTerm>(this))
    return Imm->isOne();
  return false;
}

FMAExpr::FMAExpr(MVT VT, MachineInstr *MI, FMARegTerm *Res,
                 const std::array<FMANode *, 3u> &Ops, bool MulSign,
                 bool AddSign, unsigned Index)
    : FMANode(NK_Expr, VT), MulSign(MulSign), AddSign(AddSign), Operands(Ops),
      IsFullyConsumedByKnownExpressions(false), ResultTerm(Res), MI(MI),
      DbgValMI(nullptr), Index(Index) {

  assert(ResultTerm && "Unexpected result term in FMAExpr constructor.");

  for (auto *Op : Operands) {
    assert(Op && "Unexpected operands in FMAExpr constructor.");
    if (auto *Term = dyn_cast<FMATerm>(Op))
      if (!Term->isZero() && !Term->isOne())
        UsedTerms.insert(Term);
  }
}

void FMAExpr::print(raw_ostream &OS, bool PrintAttributes) const {
  bool IsFullyConsumed = isFullyConsumed();
  if (!IsFullyConsumed)
    OS << *ResultTerm << " = ";
  OS << (MulSign ? "FNM" : "FM") << (AddSign ? "S(" : "A(") << *Operands[0]
     << "," << *Operands[1] << "," << *Operands[2] << ")";
  if (PrintAttributes) {
    OS << " // Type: " << EVT(VT).getEVTString();
    if (!IsFullyConsumed)
      OS << "\n  MI: " << *MI;
    OS << "  UsedTerms: ";
    for (auto *T : UsedTerms)
      OS << *T << ", ";
    OS << "\n";
  }
}

void FMAExprSP::canonize() {
  FMAExprSPCommon::canonize();

  if (NumProducts == 0)
    return;

  // The base version of the canonize() method sorted the products.
  // If there are some equal products but having opposite signs, then the
  // products with negative signs got placed after products with positive
  // signs. For example: +abc+ad+ad-ad-ad+c.
  //
  // The loop below looks for and removes such products with opposite signs.
  for (unsigned ProdInd = 1; ProdInd < NumProducts; ProdInd++) {
    const FMAExprProduct *PrevProd = &Products[ProdInd - 1];
    const FMAExprProduct *CurProd = &Products[ProdInd];
    if (CurProd->Sign && !PrevProd->Sign &&
        CurProd->NumTerms == PrevProd->NumTerms) {

      // Compare the products.
      unsigned TermInd;
      for (TermInd = 0; TermInd < CurProd->NumTerms; TermInd++) {
        if (PrevProd->Terms[TermInd] != CurProd->Terms[TermInd])
          break;
      }

      // If the products are equal, then just remove both products.
      if (TermInd == CurProd->NumTerms) {
        // Ok, just remove two products now.
        for (unsigned i = ProdInd + 1; i < NumProducts; i++)
          Products[i - 2] = Products[i];
        NumProducts -= 2;

        // Two products with indices (ProdInd - 1) and (ProdInd) have been
        // removed. Go to the next loop iteration. Adjust the loop variable
        // to remove more products with opposite signs and not to skip any
        // optimizable cases.
        // For example:
        //    +abc+ad+ad-ad-ad+c // ProdInd here is equal to 3,
        //               ^^
        //    SP after removal of the products with indices 2 and 3:
        //    +abc+ad-ad+c
        //    ProdInd must be set to 1 here.
        // So, subtract 2 from ProdInd, but do that carefully, i.e. do not make
        // ProdInd negative.
        ProdInd--;
        if (ProdInd)
          ProdInd--;
      }
    }
  }

  // Handle a special case. If all products got removed, then the result
  // sum of product is equal to zero.
  if (NumProducts == 0) {
    NumProducts = 1;
    Products[0].setSingleton(false, TermZERO);
  }
}

std::unique_ptr<unsigned[]> FMAExprSP::getTermsMappingToCompactTerms() {

  // First of all get the mask showing what terms are used.
  // For each of the used terms set the corresponding bit in the bit mask.
  SmallBitVector IsTermUsed(MaxNumOfUniqueTermsInSP);
  unsigned UsageMask = 0;

  for (unsigned ProdInd = 0; ProdInd < NumProducts; ProdInd++) {
    unsigned NumTerms = Products[ProdInd].NumTerms;
    uint8_t *Terms = Products[ProdInd].Terms;
    for (unsigned TermInd = 0; TermInd < NumTerms; TermInd++) {
      unsigned Term = Terms[TermInd];
      if (Term != TermZERO && Term != TermONE) {
        UsageMask |= 1 << Term;
        IsTermUsed[Term] = true;
      }
    }
  }

  // If the mask is full, then just return nullptr as the terms
  // mapping is not needed.
  if (((UsageMask + 1) & UsageMask) == 0)
    return nullptr;

  // Compact the term indices now.
  auto TermsMapping = make_unique<unsigned[]>(MaxNumOfUniqueTermsInSP);
  unsigned TheLastNewUsedTerm = 0;
  for (unsigned Term = 0; Term < MaxNumOfUniqueTermsInSP; Term++) {
    if (!IsTermUsed[Term])
      TermsMapping[Term] = ~0U;
    else {
      TermsMapping[Term] = TheLastNewUsedTerm;
      TheLastNewUsedTerm++;
    }
  }

  return TermsMapping;
}

bool FMADag::isLastUse(unsigned NodeInd, unsigned OpndInd) const {
  bool OpndIsTerm;
  unsigned SearchedOpnd = getOperand(NodeInd, OpndInd, &OpndIsTerm);

  for (unsigned OI = 0; OI < OpndInd; OI++) {
    bool IsTerm;
    unsigned ExprOrTerm = getOperand(NodeInd, OI, &IsTerm);
    if (IsTerm == OpndIsTerm && ExprOrTerm == SearchedOpnd)
      return false;
  }

  for (unsigned NI = 0; NI < NodeInd; NI++) {
    for (unsigned OI = 0; OI < 3; OI++) {
      bool IsTerm;
      unsigned ExprOrTerm = getOperand(NI, OI, &IsTerm);
      if (IsTerm == OpndIsTerm && ExprOrTerm == SearchedOpnd)
        return false;
    }
  }
  return true;
}

bool FMADag::isMul(unsigned NodeInd) const {
  bool AIsTerm, BIsTerm, CIsTerm;

  // If A is 0.0 or 1.0 then it is not a MUL operation.
  unsigned A = getOperand(NodeInd, 0, &AIsTerm);
  if (AIsTerm && (A == FMADagCommon::TermZERO || A == FMADagCommon::TermONE))
    return false;

  // If B is 0.0 or 1.0 then it is not a MUL operation.
  unsigned B = getOperand(NodeInd, 1, &BIsTerm);
  if (BIsTerm && (B == FMADagCommon::TermZERO || B == FMADagCommon::TermONE))
    return false;

  // If C is NOT 0.0 then it is not a MUL operation.
  unsigned C = getOperand(NodeInd, 2, &CIsTerm);
  if (!CIsTerm || C != FMADagCommon::TermZERO)
    return false;

  return true;
}

bool FMADag::isAdd(unsigned NodeInd) const {
  bool AIsTerm, BIsTerm, CIsTerm;

  // If A is 0.0 then it is not an ADD operation.
  unsigned A = getOperand(NodeInd, 0, &AIsTerm);
  bool AIsZero = false, AIsOne = false;
  if (AIsTerm) {
    AIsZero = A == FMADagCommon::TermZERO;
    AIsOne = A == FMADagCommon::TermONE;
  }
  if (AIsZero)
    return false;

  // If B is 0.0 then it is not an ADD operation.
  unsigned B = getOperand(NodeInd, 1, &BIsTerm);
  bool BIsZero = false, BIsOne = false;
  if (BIsTerm) {
    BIsZero = B == FMADagCommon::TermZERO;
    BIsOne = B == FMADagCommon::TermONE;
  }
  if (BIsZero)
    return false;

  // At least one of A and B must be equal to 1.0 in ADD operation.
  if (!AIsOne && !BIsOne)
    return false;

  // If C is 0.0 then it is not an ADD operation.
  unsigned C = getOperand(NodeInd, 2, &CIsTerm);
  if (CIsTerm && C == FMADagCommon::TermZERO)
    return false;

  return true;
}

bool FMADag::isFMA(unsigned NodeInd) const {
  bool AIsTerm, BIsTerm, CIsTerm;

  // If A is 0.0 or 1.0 then it is not an FMA operation.
  unsigned A = getOperand(NodeInd, 0, &AIsTerm);
  if (AIsTerm && (A == FMADagCommon::TermZERO || A == FMADagCommon::TermONE))
    return false;

  // If B is 0.0 or 1.0 then it is not an FMA operation.
  unsigned B = getOperand(NodeInd, 1, &BIsTerm);
  if (BIsTerm && (B == FMADagCommon::TermZERO || B == FMADagCommon::TermONE))
    return false;

  // If C is 0.0 then it is not an FMA operation.
  unsigned C = getOperand(NodeInd, 2, &CIsTerm);
  if (CIsTerm && C == FMADagCommon::TermZERO)
    return false;

  return true;
}

unsigned FMADag::getLatency(unsigned MulLatency, unsigned AddSubLatency,
                            unsigned FMALatency, unsigned NodeInd) const {
  unsigned Latency = 0;
  for (unsigned OpndInd = 0; OpndInd < 3; OpndInd++) {
    bool OpndIsTerm;
    unsigned Opnd = getOperand(NodeInd, OpndInd, &OpndIsTerm);
    if (!OpndIsTerm)
      Latency = std::max(
          Latency, getLatency(MulLatency, AddSubLatency, FMALatency, Opnd));
  }

  if (isMul(NodeInd))
    Latency += MulLatency;
  else if (isAdd(NodeInd))
    Latency += AddSubLatency;
  else if (isFMA(NodeInd))
    Latency += FMALatency;
  else
    llvm_unreachable("Dag has obvious inefficiencies.");

  return Latency;
}

FMAExprSP *FMAExpr::generateSPRecursively(
    const FMAExpr *RootExpr,
    SmallDenseMap<const FMANode *, std::unique_ptr<FMAExprSP>> &Node2SP) const {
  // If the sum of products is already initialized for 'this' FMA expression,
  // then return it.
  if (auto &SP = Node2SP[this])
    return SP.get();

  SmallVector<FMAExprSP *, 3u> OperandSP;
  for (auto *Opnd : Operands) {
    FMAExprSP *OpnSP = nullptr;
    if (auto *Term = dyn_cast<FMATerm>(Opnd)) {
      auto &TermSP = Node2SP[Term];
      if (!TermSP) {
        if (Term->isZero())
          TermSP = make_unique<FMAExprSP>(FMAExprSPCommon::TermZERO);
        else if (Term->isOne())
          TermSP = make_unique<FMAExprSP>(FMAExprSPCommon::TermONE);
        else
          TermSP = make_unique<FMAExprSP>(RootExpr->getUsedTermIndex(Term));
      }
      OpnSP = TermSP.get();
    } else if (auto *Expr = dyn_cast<FMAExpr>(Opnd))
      OpnSP = Expr->generateSPRecursively(RootExpr, Node2SP);
    else
      llvm_unreachable("Unsupported node kind.");

    // Sums of products must be available for all operands. Otherwise,
    // it is impossible to generate a sum of products for expression.
    if (!OpnSP)
      return nullptr;

    OperandSP.push_back(OpnSP);
  }

  FMAExprSP MulSP;
  if (!MulSP.initForMul(*OperandSP[0], *OperandSP[1]))
    return nullptr;

  auto &SP = Node2SP[this] = make_unique<FMAExprSP>();
  if (!SP->initForAdd(MulSP, *OperandSP[2], MulSign, AddSign))
    return nullptr;

  return SP.get();
}

std::unique_ptr<FMAExprSP> FMAExpr::generateSP() const {
  // Exit early if the number of terms is obviously too big and
  // SP cannot be built.
  if (UsedTerms.size() > FMAExprSP::MaxNumOfUniqueTermsInSP)
    return nullptr;

  SmallDenseMap<const FMANode *, std::unique_ptr<FMAExprSP>> Node2SP;
  if (!generateSPRecursively(this, Node2SP))
    return nullptr;

  auto SP = std::move(Node2SP[this]);
  assert(SP && "SP was not generated");
  SP->canonize();
  SP->computeShape();
  return SP;
}

// The canonize() method may remove some of terms completely,
// For example,
//     Before: +ab+c-c+d
//     After : +ab+d
// The term 'c' got totally removed here. Let's compact the terms
// in SP and in 'this' FMAExpr.
void FMAExpr::compactTerms(FMAExprSP &SP) {
  if (auto TermsMapping = SP.getTermsMappingToCompactTerms()) {
    LLVM_DEBUG(dbgs() << "  Need to compact terms in EXPR: " << *this << "\n");
    LLVM_DEBUG(dbgs() << "  SP before compact: " << SP << "\n");

    SP.doTermsMapping(TermsMapping.get());

    LLVM_DEBUG(dbgs() << "  SP after compact: " << SP << "\n");

    // Now delete the unused terms from the vector UsedTerms.
    unsigned TermsMappingIndex = 0;
    for (auto I = UsedTerms.begin(); I != UsedTerms.end();) {
      // Terms mapping has the value ~0U if the corresponding term must be
      // removed.
      if (TermsMapping[TermsMappingIndex] == ~0U) {
        LLVM_DEBUG(dbgs() << "  Remove the term from UsedTerms: " << **I);
        I = UsedTerms.erase(I);
      } else
        I++;
      TermsMappingIndex++;
    }
  }
}

void FMAExpr::startConsume(FMAExpr &FWSExpr,
                           SmallVectorImpl<FMATerm *> &UsedTermsBackUp) {
  // 1. Change the corresponding operands/terms with the reference to FWSExpr.
  FMARegTerm *FWSTerm = FWSExpr.getResultTerm();
  replaceAllUsesOfWith(FWSTerm, &FWSExpr);

  // 2. Save the list of used terms before the consumption to make it
  // possible to revert the changes in it done by this step 1.
  UsedTermsBackUp.assign(UsedTerms.begin(), UsedTerms.end());

  // 3. Remove FWSTerm from the set of used terms as it just got substituted by
  // the expression FWSExpr.
  auto Res = UsedTerms.remove(FWSTerm);
  assert(Res && "Cannot remove a term that is not in a list of used terms.");
  (void)Res;

  // 4. Add terms used by 'FWSExpr' to the list of terms used by the current
  // FMA expression.
  UsedTerms.insert(FWSExpr.UsedTerms.begin(), FWSExpr.UsedTerms.end());
}

void FMAExpr::cancelConsume(FMAExpr &FWSExpr,
                            SmallVectorImpl<FMATerm *> &UsedTermsBackUp) {
  replaceAllUsesOfWith(&FWSExpr, FWSExpr.getResultTerm());

  // Restore used terms from backup
  UsedTerms.clear();
  UsedTerms.insert(UsedTermsBackUp.begin(), UsedTermsBackUp.end());
}

void FMAExpr::commitConsume(FMAExpr &FWSExpr, bool HasOtherFMAUsers) {
  if (!HasOtherFMAUsers)
    FWSExpr.markAsFullyConsumedByKnownExpressions();

  if (!HasOtherFMAUsers && !FWSExpr.hasUnknownUsers()) {
    LLVM_DEBUG(dbgs() << "    !! Remove all USED terms from EXPR: " << FWSExpr
                      << "\n");
    // FWSExpr does not need to keep the list of used terms anymore.
    FWSExpr.UsedTerms.clear();

    // The last user of FWSExpr gets information about machine instructions
    // associated with the included/fused expression.
    ConsumedMIs.push_back(FWSExpr.getMI());
    ConsumedMIs.splice(ConsumedMIs.end(), FWSExpr.ConsumedMIs);
    if (FWSExpr.DbgValMI)
      ConsumedMIs.push_back(FWSExpr.DbgValMI);
  }
}

bool FMAExpr::consume(FMAExpr &FWSExpr, const FMAPatterns &Patterns,
                      bool HasOtherFMAUsers) {
  SmallVector<FMATerm *, 16u> UsedTermsBackUp;
  startConsume(FWSExpr, UsedTermsBackUp);

  // If the new expression is too big and cannot be be optimized, then
  // cancel the changes done at startConsume().
  if (isExprTooLarge(Patterns)) {
    cancelConsume(FWSExpr, UsedTermsBackUp);
    return false;
  }

  commitConsume(FWSExpr, HasOtherFMAUsers);

  LLVM_DEBUG(dbgs() << "  -->After consuming expr: " << *this << "\n");
  return true;
}

unsigned FMAExpr::getLatency(unsigned AddSubLatency, unsigned MulLatency,
                             unsigned FMALatency) const {
  unsigned MaxOperandLatency = 0;
  for (auto *Opnd : Operands)
    if (auto *Expr = dyn_cast<FMAExpr>(Opnd)) {
      unsigned OperandLatency =
          Expr->getLatency(AddSubLatency, MulLatency, FMALatency);
      MaxOperandLatency = std::max(MaxOperandLatency, OperandLatency);
    }

  if (Operands[0]->isZero() || Operands[1]->isZero())
    // This FMA is actually a term. It adds nothing to the returned latency.
    return MaxOperandLatency;

  if (Operands[0]->isOne() || Operands[1]->isOne()) {
    if (!Operands[2]->isZero())
      return MaxOperandLatency + AddSubLatency;
  } else if (Operands[2]->isZero())
    return MaxOperandLatency + MulLatency;
  else
    return MaxOperandLatency + FMALatency;

  return MaxOperandLatency;
}

/// This class represents one optimizable basic block. It holds all FMAExpr
/// objects created for operations in one MachineBasicBlock.
/// It also keeps references to special terms 0.0 and 1.0 created only once and
/// returned when they are used again.
class FMABasicBlock {
  /// A reference to the machine basic block that is being optimized.
  MachineBasicBlock &MBB;

  /// All FMA expressions available in the basic block.
  SmallVector<std::unique_ptr<FMAExpr>, 16u> FMAs;

  /// Register terms used or defined by FMA expressions in the basic block
  /// are stored into map to avoid creation of duplicated terms and
  /// to have quick search through already existing terms using virtual
  /// registers as keys.
  SmallDenseMap<unsigned, std::unique_ptr<FMARegTerm>> RegToRegTerm;

  /// This field maps terms to expressions defining those terms.
  /// For example, for any expression T1 = FMA1(...) there should be a pair
  /// <T1, FMA1> in this map.
  SmallDenseMap<FMARegTerm *, FMAExpr *> RegTermToDefExpr;

  /// Terms for immediate values.
  std::map<std::pair<MVT, int64_t>, std::unique_ptr<FMAImmTerm>> Imms;

public:
  /// Creates an FMA basic block for the given MachineBasicBlock \p MBB.
  FMABasicBlock(MachineBasicBlock &MBB) : MBB(MBB) {}

  /// Returns machine basic block.
  MachineBasicBlock &getMBB() { return MBB; }
  const MachineBasicBlock &getMBB() const { return MBB; }

  FMAImmTerm *createImm(MVT VT, int64_t Imm) {
    auto &Term = Imms[{VT, Imm}];
    if (!Term)
      Term = make_unique<FMAImmTerm>(VT, Imm);
    return Term.get();
  }

  FMAImmTerm *createZero(MVT VT) { return createImm(VT, 0); }

  FMAImmTerm *createOne(MVT VT) { return createImm(VT, getOne(VT)); }

  /// Creates an FMA term associated with the virtual register used in
  /// the passed machine operand \p MO. The parameter \p VT specifies
  /// the type of the created term.
  FMARegTerm *createReg(MVT VT, const MachineOperand &MO) {
    assert(MO.isReg() && "Cannot create an FMA term for MachineOperand.");
    unsigned Reg = MO.getReg();

    // If there is a term created for this machine operand (or identical to it)
    // then just return the existing term. Otherwise, create a new term.
    auto &Term = RegToRegTerm[Reg];
    if (!Term)
      Term = make_unique<FMARegTerm>(VT, Reg, RegToRegTerm.size());
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

  /// Creates an FMA expression for a statement like this:
  ///    Res = (MulSign) Op[0] * \p Op[1] + (AddSign) Op[2].
  FMAExpr *createFMA(MVT VT, MachineInstr *MI, FMARegTerm *Res,
                     const std::array<FMANode *, 3u> &Ops, bool MulSign,
                     bool AddSign) {
    auto *Expr = new FMAExpr(VT, MI, Res, Ops, MulSign, AddSign, FMAs.size());
    FMAs.emplace_back(Expr);
    RegTermToDefExpr[Res] = Expr;
    return Expr;
  }

  /// Walks through all instructions in the machine basic block, finds
  /// MUL/ADD/FMA operations and creates FMA expressions (FMAExpr) for them.
  /// Returns the number of optimizable expressions found in the block.
  /// The parameter \p MRI is passed to this method to make it possible
  /// to find virtual registers associated with FMARegTerms and
  /// having uses that are not recognized as FMAExpr operations.
  unsigned parseBasicBlock(const FMAOpcodes &Opcodes, MachineRegisterInfo *MRI);

  /// Returns a reference to FMA expression defining the given \p Term.
  FMAExpr *findDefiningFMA(const FMATerm *Term) const {
    if (auto *Reg = dyn_cast<FMARegTerm>(Term))
      return RegTermToDefExpr.lookup(Reg);
    return nullptr;
  }

  /// Returns the vector containing all FMAs available in this basic block.
  const SmallVectorImpl<std::unique_ptr<FMAExpr>> &getFMAs() const {
    return FMAs;
  }

  /// For the given term \p Term it puts all the users of that term into
  /// \p UsersSet.
  void findKnownUsers(FMATerm *Term,
                      SmallPtrSetImpl<FMAExpr *> &Users) const {
    Users.clear();
    for (auto &Expr : FMAs)
      if (Expr->isUserOf(Term))
        Users.insert(Expr.get());
  }

  /// Marks the FMARegTerms defined by some known FMAExpr expressions
  /// with special attributes if such terms have uses now recognized as
  /// FMAExpr expressions.
  void setDefHasUnknownUsersForRegTerms(MachineRegisterInfo *MRI) {
    SmallPtrSet<const MachineInstr *, 16u> MIsSet;

    // 1st pass: Add all known machine instructions to the set.
    for (auto &T2E : RegTermToDefExpr)
      MIsSet.insert(T2E.second->getMI());

    // 2nd pass: Mark the register terms having defining FMA expressions and
    // having users/MIs not recognized as FMAs in this basic block.
    for (auto &T2E : RegTermToDefExpr) {
      FMARegTerm *Term = T2E.first;
      for (MachineInstr &I : MRI->reg_instructions(Term->getReg())) {
        // Do not count DBG_VALUE as a real user.
        // Otherwise, -g would severely affect the optimization.
        // DBG_VALUE though must be registered. So it would be
        // deleted when gets unused.
        if (I.isDebugValue()) {
          T2E.second->setDbgValMI(&I);
          continue;
        }

        if (!MIsSet.count(&I))
          Term->setDefHasUnknownUsers();
      }
    }
  }

  /// Prints the basic block to the given stream \p OS.
  void print(raw_ostream &OS) const;

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  LLVM_DUMP_METHOD void dump() const { print(dbgs()); }
#endif
};

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
/// Prints the FMA basic block \p FMABB to the given stream \p OS.
static raw_ostream &operator<<(raw_ostream &OS, const FMABasicBlock &FMABB) {
  FMABB.print(OS);
  return OS;
}
#endif

unsigned FMABasicBlock::parseBasicBlock(const FMAOpcodes &Opcodes,
                                        MachineRegisterInfo *MRI) {
  LLVM_DEBUG(dbgs() << "FMA-STEP1: FIND FMA OPERATIONS:\n");

  for (auto &MI : MBB) {
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
      if (auto *Term = dyn_cast<FMAImmTerm>(Op)) {
        if (Term->isNegativeOne()) {
          Op = createOne(VT);
          Sign ^= true;
        } else if (Term->isNegativeZero())
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
  setDefHasUnknownUsersForRegTerms(MRI);

  LLVM_DEBUG(dbgs() << *this << "FMA-STEP1 DONE.\n");
  return FMAs.size();
}

void FMABasicBlock::print(raw_ostream &OS) const {
  OS << "\nFMA REGISTER TERMs:\n  ";
  for (const auto &T : RegToRegTerm) {
    T.second->print(OS, true /* PrintAttributes */);
    OS << "\n  ";
  }

  OS << "\nFMA EXPRESSIONs:\n";
  unsigned Index = 0;
  for (const auto &E : FMAs)
    if (!E->isFullyConsumed()) {
      OS << "  " << Index++ << ": ";
      E->print(OS, true /* PrintAttributes */);
      OS << "\n";
    }
}

FMAExpr *FMAExpr::findFWSCandidate(
    FMABasicBlock &FMABB, const SmallPtrSetImpl<FMAExpr *> &BadFWSCandidates,
    const SmallPtrSetImpl<FMAExpr *> &InefficientFWSCandidates,
    bool CanConsumeIfOneUser, SmallPtrSetImpl<FMAExpr *> &Users) {

  // Walk through all terms used by the current FMA, find those that are the
  // results of other FMAs.
  for (auto *Term : UsedTerms) {
    auto *Expr = FMABB.findDefiningFMA(Term);
    if (!Expr)
      continue;

    // This expression cannot be removed even if it gets consumed by another
    // one. It still may be possible to proceed and get some performance, but
    // those are very rare corner cases, which though complicate the
    // optimization alot and create bigger risks.
    if (Expr->hasUnknownUsers()) {
      LLVM_DEBUG(dbgs() << "  Candidate is skipped as having unknown users: "
                        << *Term << "\n");
      continue;
    }

    if (BadFWSCandidates.count(Expr)) {
      LLVM_DEBUG(dbgs() << "  Candidate is skipped as BAD candidate: " << *Term
                        << "\n");
      continue;
    }

    if (!CanConsumeIfOneUser && InefficientFWSCandidates.count(Expr)) {
      LLVM_DEBUG(dbgs() << "  Candidate is skipped as INEFFICIENT candidate: "
                        << *Term << "\n");
      continue;
    }

    // This place would be good for doing safety check verifying that it is Ok
    // to use the virtual register associated with 'Term' at the point where
    // the machine instruction associated with this FMA expression is located.
    // For FMARegTerm we can just use the virtual register as it is SSA
    // form and the register is virtual at this phase. For FMAMemoryTerm the
    // virtual register is not assigned yet, but it is going to be defined
    // right before the load instruction associated with FMAMemoryTerm and
    // it should be Ok to use such term in 'this' FMA expression.
    // So, no checks are performed here.

    FMABB.findKnownUsers(Term, Users);

    // Skip the candidate if it fusible into 2 or more users in
    // the mode that enables FWS only candidates with 1 user.
    if (CanConsumeIfOneUser && Users.size() > 1) {
      LLVM_DEBUG(dbgs() << "  Candidate is skipped as HAVING MANY USERS: "
                        << *Term << "\n");
      continue;
    }

    return Expr;
  }

  return nullptr;
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
    Patterns = make_unique<FMAPatterns>();
  if (!Opcodes)
    Opcodes = make_unique<FMAOpcodes>();

  // Init insturction latencies.
  AddSubLatency = 4;
  MulLatency = 4;
  FMALatency = 4;

  // Process all basic blocks.
  bool Changed = false;
  for (auto &MB : MF)
    if (optBasicBlock(MB))
      Changed = true;

  LLVM_DEBUG(dbgs() << "********** CSA Global FMA **********\n");
  if (Changed)
    LLVM_DEBUG(MF.print(dbgs()));
  return Changed;
}

/// Loop over all of the instructions in the basic block, optimizing
/// MUL/ADD/FMA expressions. Return true iff any changes in the machine
/// operation were done.
bool CSAGlobalFMA::optBasicBlock(MachineBasicBlock &MBB) {
  LLVM_DEBUG(dbgs() << "\n**** RUN FMA OPT FOR ANOTHER BASIC BLOCK ****\n");

  FMABasicBlock FMABB(MBB);

  // Find MUL/ADD/SUB/FMA/etc operations in the input machine instructions
  // and create internal FMA structures for them.
  // Exit if there are not enough optimizable expressions.
  if (FMABB.parseBasicBlock(*Opcodes, MRI) < 2)
    return false;

  // Save the dump of the basic block, we may want to print it after the basic
  // block is changed by this optimization.
  SmallString<256u> BBDump;
  {
    raw_svector_ostream BBS(BBDump);
    LLVM_DEBUG(BBS << MBB);
  }

  // Run the FMA optimization and dump the debug messages if the optimization
  // produced any changes in IR.
  if (!optParsedBasicBlock(FMABB))
    return false;

  LLVM_DEBUG(dbgs() << "Basic block before Global FMA opt:\n" << BBDump
                    << "\n\nBasic block after Global FMA opt:\n" << MBB
                    << "\n");
  return true;
}

std::unique_ptr<FMADag>
CSAGlobalFMA::getDagForExpression(FMAExpr &Expr, bool DoCompactTerms) const {
  LLVM_DEBUG(dbgs() << "  Find DAG for FMA EXPR:\n  " << Expr << "\n");
  auto SP = Expr.generateSP();
  if (!SP) {
    LLVM_DEBUG(dbgs() << "  Could not compute SP.\n");
    return nullptr;
  }

  // The returned SP might have some opportunities for terms compact.
  // For example, for initial FMAExpr expression
  //   +ab+c-c+d
  // the returned SP may be shorter and the term 'c' is not used anymore:
  //   +ab+d
  // Let's compact the terms in SP and in 'this' FMAExpr.
  if (DoCompactTerms)
    Expr.compactTerms(*SP);

  LLVM_DEBUG(dbgs() << "  Computed SP is: ");
  LLVM_DEBUG(SP->print(dbgs()));
  LLVM_DEBUG(dbgs() << "  SHAPE: " << format_hex(SP->Shape, 2) << "\n\n");

  return Patterns->getDagForBestSPMatch(*SP);
}

std::unique_ptr<FMADag>
CSAGlobalFMA::getDagForFusedExpression(FMAExpr &UserExpr,
                                       FMAExpr &FWSExpr) const {
  SmallVector<FMATerm *, 16u> UsedTermsBackUp;
  UserExpr.startConsume(FWSExpr, UsedTermsBackUp);

  auto Dag = getDagForExpression(UserExpr, false);
  UserExpr.cancelConsume(FWSExpr, UsedTermsBackUp);

  return Dag;
}

bool CSAGlobalFMA::optParsedBasicBlock(FMABasicBlock &FMABB) {
  bool Changed = false;
  doFWS(FMABB);

  LLVM_DEBUG(dbgs() << "\nFMA-STEP3: DO PATTERN MATCHING AND CODE-GEN:\n");
  for (auto &Expr : FMABB.getFMAs()) {
    if (!Expr->isOptimizable())
      continue;

    LLVM_DEBUG(dbgs() << "  Optimize FMA EXPR:\n  " << *Expr);
    auto Dag = getDagForExpression(*Expr, true);
    if (!Dag)
      continue;

    LLVM_DEBUG(dbgs() << "  CONGRATULATIONS! A searched DAG was found:\n    ");
    LLVM_DEBUG(Dag->print(dbgs()));

    // FIXME: Currently, the setting of the latency vs throughput priorities
    // is set only accordingly to the internal switch value.
    // For some target architectures (e.g. in-order targets) the throughput
    // aspects should be more important.
    // Also, this place should be updated after the latency vs throughput
    // analysis of the optimized basic block and the data dependencies analysis
    // in the optimized expression are implemented.
    bool TuneForLatency = checkFMAControl(FMAControlTuneForLatency);
    bool TuneForThroughput = checkFMAControl(FMAControlTuneForThroughput);
    if (!isDagBetterThanInitialExpr(*Dag, *Expr, TuneForLatency,
                                    TuneForThroughput)) {
      LLVM_DEBUG(dbgs() << "  DAG is NOT better than the initial EXPR.\n\n");
      continue;
    }
    LLVM_DEBUG(dbgs() << "  DAG IS better than the initial EXPR.\n");

    Changed = true;
    generateOutputIR(*Expr, *Dag, FMABB);
    LLVM_DEBUG(dbgs() << "\n");
  }

  LLVM_DEBUG(dbgs() << "\nFMA-STEP3 IS DONE. Machine basic block IS "
                    << (Changed ? "" : "NOT ") << "UPDATED.\n\n");
  return Changed;
}

FMAPatterns::FMAPatterns() : LargestAvailableShape(0) {
#include "CSAGenMAPatterns.inc"
  LargestAvailableShape = acquireSP(Dags.back().front())->Shape;
}

const FMAExprSP *FMAPatterns::acquireSP(uint64_t EncodedDag) {
  auto &SP = EncodedDagToSPMap[EncodedDag];
  if (!SP) {
    SP = make_unique<FMAExprSP>();
    SP->initForEncodedDag(EncodedDag);
  }
  return SP.get();
}

const FMAPatterns::FMAPatternsSet *
FMAPatterns::getDagsForShape(uint64_t Shape) {
  unsigned First = 0, Last = Dags.size() - 1;

  // If the passed 'Shape' is bigger than the biggest available shape in
  // the storage, then just exit early and skip the binary search.
  auto *SP = acquireSP(Dags[Last].front());
  if (Shape > SP->Shape)
    return nullptr;
  if (Shape == SP->Shape)
    return &Dags[Last];

  while (First < Last) {
    // Check the SHAPE of a set of DAGs in the middle of the search scope.
    unsigned Middle = (First + Last) / 2;
    SP = acquireSP(Dags[Middle].front());
    uint64_t CurShape = SP->Shape;

    // If the searched SHAPE is found, then return the whole set of DAGs having
    // the same SHAPE.
    if (Shape == CurShape)
      return &Dags[Middle];

    // Halve the search scope and continue the binary search.
    if (Shape < CurShape)
      Last = Middle;
    else
      First = Middle + 1;
  }

  return nullptr;
}

std::unique_ptr<FMADag> FMAPatterns::getDagForBestSPMatch(const FMAExprSP &SP) {

  const FMAPatternsSet *DagsSet = getDagsForShape(SP.Shape);
  if (!DagsSet)
    return nullptr;

  LLVM_DEBUG(dbgs() << "  MATCHING: could find a set of DAGs for SHAPE("
                    << format_hex(SP.Shape, 2) << ")\n");

  // Find the best DAG for the given SP.
  std::unique_ptr<FMADagCommon> BestDag;
  for (const auto Dag64 : *DagsSet) {
    // FIXME: TermONE in C position is not yet supported, e.g. (A*B+1). Fix it.
    FMADag Dag(Dag64);

    auto *CandidateSP = acquireSP(Dag64);

    LLVM_DEBUG(dbgs() << "  MATCHING: let's try to match 2 SPs:\n    actual: ");
    LLVM_DEBUG(SP.print(dbgs()));
    LLVM_DEBUG(dbgs() << "    formal: ");
    LLVM_DEBUG(CandidateSP->print(dbgs()));

    FMASPToSPMatcher SPMatcher;
    if (auto *CandidateDag = SPMatcher.getDagToMatchSPs(*CandidateSP, SP)) {
      // Ok, we found Sum Of Products. Let's do some heuristical checks
      // and choose the best alternative here.

      // FIXME: currently we just choose the first one and return it.
      BestDag.reset(CandidateDag);
      break;
    }
  }

  if (BestDag)
    return make_unique<FMADag>(*BestDag);
  return nullptr;
}

void CSAGlobalFMA::generateOutputIR(const FMAExpr &Expr, const FMADag &Dag,
                                    FMABasicBlock &FMABB) {
  auto &MBB = FMABB.getMBB();
  auto *MI = Expr.getMI();

  auto GenMOForTerm = [](const FMATerm *Term) {
    if (auto *RT = dyn_cast<FMARegTerm>(Term)) {
      assert(RT->getReg() && "RegTerm with no register");
      return MachineOperand::CreateReg(RT->getReg(), false);
    }
    return MachineOperand::CreateImm(cast<FMAImmTerm>(Term)->getImm());
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
      MOs.push_back(GenMOForTerm(Expr.getUsedTerm(A)));
    } else
      MOs.push_back(MachineOperand::CreateReg(ResultRegs[A], false));

    if (BIsTerm) {
      assert(B != FMADagCommon::TermZERO &&
             "Bad FMA DAG: the operand B cannot be equal to 0.0.");
      if (B == FMADagCommon::TermONE)
        IsAddOrSub = true;
      else
        MOs.push_back(GenMOForTerm(Expr.getUsedTerm(B)));
    } else
      MOs.push_back(MachineOperand::CreateReg(ResultRegs[B], false));

    if (CIsTerm) {
      if (C == FMADagCommon::TermZERO)
        IsMul = true;
      else {
        auto *Term = C == FMADagCommon::TermONE ? FMABB.createOne(VT)
                                                : Expr.getUsedTerm(C);
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
        MOs.push_back(GenMOForTerm(FMABB.createZero(VT)));
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
      case MVT::f32:
        Opc = CSA::XOR32;
        break;
      case MVT::f64:
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

FMAPerfDesc CSAGlobalFMA::getExprPerfDesc(const FMAExpr &Expr) const {
  unsigned NumAddSub = 0;
  unsigned NumMul = 0;
  unsigned NumFMA = 0;

  SmallPtrSet<const FMAExpr *, 16u> ExprSet;
  Expr.putExprToExprSet(ExprSet);

  for (const auto *E : ExprSet) {
    if (E->getOperand(0)->isZero() || E->getOperand(1)->isZero())
      // This FMA is actually a term. It adds nothing to the returned
      // statistics.
      continue;

    if (E->getOperand(0)->isOne() || E->getOperand(1)->isOne()) {
      if (!E->getOperand(2)->isZero())
        NumAddSub++;
    } else if (E->getOperand(2)->isZero())
      NumMul++;
    else
      NumFMA++;
  }

  unsigned Latency = Expr.getLatency(AddSubLatency, MulLatency, FMALatency);
  return FMAPerfDesc(Latency, NumAddSub, NumMul, NumFMA);
}

FMAPerfDesc CSAGlobalFMA::getDagPerfDesc(const FMADag &Dag) const {
  unsigned NumAddSub = 0;
  unsigned NumMul = 0;
  unsigned NumFMA = 0;

  unsigned NumNodes = Dag.getNumNodes();
  for (unsigned NodeInd = 0; NodeInd < NumNodes; NodeInd++) {
    bool AIsTerm, BIsTerm, CIsTerm;
    unsigned A = Dag.getOperand(NodeInd, 0, &AIsTerm);
    unsigned B = Dag.getOperand(NodeInd, 1, &BIsTerm);
    unsigned C = Dag.getOperand(NodeInd, 2, &CIsTerm);

    bool AIsZero = AIsTerm && A == FMADagCommon::TermZERO;
    bool BIsZero = BIsTerm && B == FMADagCommon::TermZERO;
    bool CIsZero = CIsTerm && C == FMADagCommon::TermZERO;

    (void)AIsZero;
    (void)BIsZero;
    assert(!AIsZero && !BIsZero && "DAG has obvious inefficiencies.");

    bool AIsOne = AIsTerm && A == FMADagCommon::TermONE;
    bool BIsOne = BIsTerm && B == FMADagCommon::TermONE;

    if (AIsOne || BIsOne) {
      assert(!CIsZero && "DAG has obvious inefficiencies.");
      NumAddSub++;
      // -A - C node requires 2 operations at the code-generation phase:
      //   T0 = A + C; T1 = 0 - T0;
      // Count the additional neg/subtract operation here.
      if (Dag.getMulSign(NodeInd) && Dag.getAddSign(NodeInd))
        NumAddSub++;
    } else if (CIsZero) {
      // A*B requires 1 MUL operation at the code-generation phase.
      // -A*B requires 1 FMA operation: -A*B+0.
      if (Dag.getMulSign(NodeInd))
        NumFMA++;
      else
        NumMul++;
    } else
      NumFMA++;
  }
  unsigned Latency = Dag.getLatency(MulLatency, AddSubLatency, FMALatency);
  return FMAPerfDesc(Latency, NumAddSub, NumMul, NumFMA);
}

bool FMAPerfDesc::isBetterThan(const FMAPerfDesc &OtherDesc,
                               bool TuneForLatency,
                               bool TuneForThroughput) const {

  // Tuning for latency AND throughput means that the caller does not have
  // strong preferences and the choice should be made heuristically.
  if (TuneForLatency && TuneForThroughput) {
    TuneForLatency = false;
    TuneForThroughput = false;
  }

  unsigned NumOperations = getNumOperations();
  unsigned OtherNumOperations = OtherDesc.getNumOperations();

  if (TuneForThroughput) {
    // If the number of operations in this descriptor is smaller than
    // the number of operations in 'OtherDesc', then just return true.
    if (NumOperations != OtherNumOperations)
      return NumOperations < OtherNumOperations;

    // If the numbers of operations are equal, then return true or false
    // depending on which descriptor has smaller latency.
    if (Latency != OtherDesc.Latency)
      return Latency < OtherDesc.Latency;

    // Less FMAs is preferred because they clobber one of operands and thus
    // are less flexible than MUL/ADD/SUB operations.
    return NumFMA < OtherDesc.NumFMA;
  }

  if (TuneForLatency) {
    // If the latencies are different, then return true or false depending on
    // which descriptor has smaller latency.
    if (Latency != OtherDesc.Latency)
      return Latency < OtherDesc.Latency;

    // If the latencies are identical, then compare the numbers of operations.
    if (NumOperations != OtherNumOperations)
      return NumOperations < OtherNumOperations;

    // Less FMAs is preferred because they clobber one of operands and thus
    // are less flexible than MUL/ADD/SUB operations.
    return NumFMA < OtherDesc.NumFMA;
  }

  double LatencyImprovement;
  if (Latency < OtherDesc.Latency)
    LatencyImprovement = (double)OtherDesc.Latency / (double)Latency - 1.0;
  else
    LatencyImprovement = -((double)Latency / (double)OtherDesc.Latency - 1.0);

  double ThroughputImprovement;
  if (NumOperations < OtherNumOperations)
    ThroughputImprovement =
        (double)OtherNumOperations / (double)NumOperations - 1.0;
  else
    ThroughputImprovement =
        -((double)NumOperations / (double)OtherNumOperations - 1.0);

  double Improvement = LatencyImprovement + ThroughputImprovement;
  if (Improvement == 0)
    // Prefer to have less FMAs as FMAs are less flexible when they are
    // processed by memory-folding, coalescing and register allocation
    // optimizations.
    return NumFMA < OtherDesc.NumFMA;
  return LatencyImprovement + ThroughputImprovement > 0;
}

bool CSAGlobalFMA::isDagBetterThanInitialExpr(const FMADag &Dag,
                                              const FMAExpr &Expr,
                                              bool TuneForLatency,
                                              bool TuneForThroughput) const {
  FMAPerfDesc DagDesc = getDagPerfDesc(Dag);
  FMAPerfDesc ExprDesc = getExprPerfDesc(Expr);

  LLVM_DEBUG(dbgs() << "  Compare DAG and initial EXPR:\n"
                    << "    DAG  has: " << DagDesc << "\n"
                    << "    EXPR has: " << ExprDesc << "\n");

  // If the internal switch requires FMAs, then just return true.
  // This code is placed after the printings of the DAG/Expr properties
  // as the last may be interesting even if FMAs are forced.
  if (checkFMAControl(FMAControlForceFMAs))
    return true;

  return DagDesc.isBetterThan(ExprDesc, TuneForLatency, TuneForThroughput);
}

bool CSAGlobalFMA::doFWSAndConsumeIfProfitable(
    FMAExpr &FWSExpr, SmallPtrSetImpl<FMAExpr *> &Users,
    SmallPtrSetImpl<FMAExpr *> &BadUsers,
    SmallPtrSetImpl<FMAExpr *> &InefficientUsers, bool CanConsumeIfOneUser) {

  SmallPtrSet<FMAExpr *, 16u> NeutralUsers;
  unsigned NumOtherFMAUsers = Users.size();

  int NumAdditionalAddSub = 0;
  int NumAdditionalMul = 0;
  int NumAdditionalFMA = 0;

  bool Consumed = false;

  // CanConsumeIfOneUser means a special mode is ON, on which
  // only consumptions of expressions with 1 user are done even
  // they may look inefficient at this moment.
  if (CanConsumeIfOneUser && Users.size() > 1)
    return false;

  for (auto *UserExpr : Users) {
    if (BadUsers.count(UserExpr) ||
        (!CanConsumeIfOneUser && InefficientUsers.count(UserExpr)))
      continue;

    auto OriginalDag = getDagForExpression(*UserExpr, false);
    if (!OriginalDag) {
      // User cannot consume anything as it cannot have a DAG even without
      // fusing UserExpr and FWSExpr.
      LLVM_DEBUG(dbgs() << "  User is marked as BAD now because "
                           "getDagForExpression() returned NULL: "
                        << *UserExpr << "\n");
      BadUsers.insert(UserExpr);
      continue;
    }

    auto FusedDag = getDagForFusedExpression(*UserExpr, FWSExpr);
    if (!FusedDag) {
      // DAG could not be created for the fused expression (UserExpr + FWSExpr).
      LLVM_DEBUG(dbgs() << "  User is marked as BAD now because "
                           "getDagForFusedExpression() returned NULL: "
                        << *UserExpr << "\n  FWSExpr: " << FWSExpr << "\n");
      BadUsers.insert(UserExpr);
      continue;
    }

    FMAPerfDesc OriginalDesc = getDagPerfDesc(*OriginalDag);
    FMAPerfDesc FusedDesc = getDagPerfDesc(*FusedDag);
    LLVM_DEBUG(dbgs() << "      DAG before FWS: " << *OriginalDag << "      "
                      << OriginalDesc << "\n");
    LLVM_DEBUG(dbgs() << "      DAG after FWS: " << *FusedDag << "      "
                      << FusedDesc << "\n");
    // TODO: vklochko: The 2nd and 3rd arguments of the next call must depend
    // on the existance of recurrent terms used by the fused expression.
    // Currently, such analysis are not available.
    if (!FusedDesc.isBetterThan(OriginalDesc, false /*TuneForLatency */,
                                false /*TuneForThroughput*/)) {
      // UserExpr and FWSExpr potentially can be fused, but the DAG for
      // the fused expression does NOT seem better than the DAG of UserExpr.
      NeutralUsers.insert(UserExpr);
      NumAdditionalAddSub +=
          FusedDesc.getNumAddSub() - OriginalDesc.getNumAddSub();
      NumAdditionalMul += FusedDesc.getNumMul() - OriginalDesc.getNumMul();
      NumAdditionalFMA += FusedDesc.getNumFMA() - OriginalDesc.getNumFMA();
      LLVM_DEBUG(dbgs() << "      Fusing does NOT seems efficient now. "
                           "Add the candidate to neutral users.\n");
    } else {
      // If the fused DAG is better than the original DAG, then just
      // fuse UserExpr and FWSExpr.
      // Example:
      //    t1 = a-b; // has 2 users: t2 and some other-unknown.
      //    t2 = a-t1;
      // -->
      //    t1 = a-b;
      //    t2 = a-(a-b); // Better because it is equal to t2 = b;
      LLVM_DEBUG(dbgs() << "      Fusing seems efficient. Dot it now.\n");
      NumOtherFMAUsers--;
      Consumed = UserExpr->consume(FWSExpr, *Patterns, NumOtherFMAUsers);
      assert(Consumed && "FWS/consume must be possible.");
    }
  }

  // If there is nothing more to fuse, then just exit.
  if (NeutralUsers.empty())
    return Consumed;

  // If the FWS expression cannot be removed due to existing users, then exit.
  if (FWSExpr.hasUnknownUsers() || !BadUsers.empty()) {
    LLVM_DEBUG(
        dbgs() << "      Fusing neutral users does NOT seem efficient.\n");
    InefficientUsers.insert(NeutralUsers.begin(), NeutralUsers.end());
    return Consumed;
  }

  // There were some UserExpr that could be fused with FWSExpr, but the fused
  // expressions are not better than the original UserExpr.
  //
  // Ok, if FWSExpr can be consumed by all such UserExpr expressions, then
  // FWSExpr can be just removed. It makes sense doing it ONLY if the benefit
  // from removing FWSExpr is bigger than the loses from fusing it to all users.
  // Example:
  //    t1 = a*b; // has 2 users: t2 and t3.
  //    t2 = t1+c;
  //    t3 = t1-c;
  // -->
  //    t2 = a*b+c;
  //    t3 = a*b-c;
  // t2 and t3 expressions did not get better (and did not get worse),
  // but consumption helped to eliminate the expression t1 = a*b.
  //
  // TODO: vklochko: The latency aspect is ignored for a while here:
  // 1. There are no loop dependencies analysis at this moment.
  // 2. Without loop dependencies, it is extremely hard to imagine a situation
  //    when latency aspect would have priority over throughput.
  int NumAdditionalOperations =
      NumAdditionalAddSub + NumAdditionalMul + NumAdditionalFMA;
  FMAPerfDesc FWSExprDesc = getExprPerfDesc(FWSExpr);

  if ((CanConsumeIfOneUser && NeutralUsers.size() == 1) ||
      (NumAdditionalOperations < (int)FWSExprDesc.getNumOperations() ||
       (NumAdditionalOperations == (int)FWSExprDesc.getNumOperations() &&
        NumAdditionalFMA <= (int)FWSExprDesc.getNumFMA()))) {
    // It is already known that the fusing FWSExpr and its only user does not
    // seem efficient (otherwise, the user of FWSExpr would not be in
    // NeutralUsers) and fusing would happen earlier.
    //
    // The fusing still may be useful as it may give more opportinuties for
    // further and more efficient fusing. Do the fusing if the fused
    // expression is at least the same good as the two expressions before
    // fusing.
    ;
  } else if (NumAdditionalOperations > (int)FWSExprDesc.getNumOperations() ||
             (NumAdditionalOperations == (int)FWSExprDesc.getNumOperations() &&
              NumAdditionalFMA >= (int)FWSExprDesc.getNumFMA())) {
    // Do not fuse if any of the following is true:
    // - fusing to all users produces more instructions than FWSExpr has;
    // - fusing to all users produces the same number of instructions as
    //   FWSExpr has, and the number of heavy FMA instructions did not decrease.
    InefficientUsers.insert(NeutralUsers.begin(), NeutralUsers.end());
    return Consumed;
  }

  // Fusing FWSExpr to all users seems efficient.
  LLVM_DEBUG(dbgs() << "      Fusing into " << NumOtherFMAUsers
                    << " neutral users seems efficient.\n");
  for (auto *E : NeutralUsers) {
    LLVM_DEBUG(dbgs() << "      Fuse with the neutral user #"
                      << NumOtherFMAUsers << ": " << *E << "\n");
    NumOtherFMAUsers--;
    Consumed = E->consume(FWSExpr, *Patterns, NumOtherFMAUsers);
    assert(Consumed && "FWS/consume must be possible.");
  }
  FWSExpr.markAsFullyConsumedByKnownExpressions();
  return Consumed;
}

void CSAGlobalFMA::doFWS(FMABasicBlock &FMABB) {

  LLVM_DEBUG(dbgs() << "\nFMA-STEP2: DO FWS:\n");

  using FMAExprSet = SmallPtrSet<FMAExpr *, 8u>;
  SmallVector<FMAExprSet, 8u> BadFWSCandidates(FMABB.getFMAs().size());
  SmallVector<FMAExprSet, 8u> InefficientFWSCandidates(FMABB.getFMAs().size());

  bool Consumed = true;
  bool CanConsumeIfOneUser = false;
  while (Consumed) {
    Consumed = false;

    const auto &FMAs = FMABB.getFMAs();
    for (unsigned I = 0; I < FMAs.size(); ++I) {
      auto &Expr = FMAs[I];
      if (Expr->isFullyConsumedByKnownExpressions())
        continue;

      LLVM_DEBUG(dbgs() << "\n  FWS: try to find terms that could be "
                           "substituted by expressions in: "
                        << *Expr << "\n");

      SmallPtrSet<FMAExpr *, 16u> Users;
      FMAExpr *FWSExpr = Expr->findFWSCandidate(FMABB, BadFWSCandidates[I],
                                                InefficientFWSCandidates[I],
                                                CanConsumeIfOneUser, Users);
      while (FWSExpr) {
        LLVM_DEBUG(dbgs() << "    Found a FWS candidate:\n    " << *FWSExpr
                          << " fusible to " << Users.size() << " user(s):\n");
        LLVM_DEBUG(for (auto *E : Users) { dbgs() << "      " << *E << "\n"; });

        SmallPtrSet<FMAExpr *, 16u> BadUsers;
        SmallPtrSet<FMAExpr *, 16u> InefficientUsers;
        if (doFWSAndConsumeIfProfitable(*FWSExpr, Users, BadUsers,
                                        InefficientUsers,
                                        CanConsumeIfOneUser)) {
          Consumed = true;

          // The expressions from Users got changed, thus the candidates
          // previously registered as bad for them may become good now.
          for (auto *E : Users) {
            BadFWSCandidates[E->getIndex()].clear();
            InefficientFWSCandidates[E->getIndex()].clear();
            // Also each other expression referencing E from Users as bad
            // needs to clean E from it's bad list.
            for (unsigned I = 0; I < FMABB.getFMAs().size(); ++I) {
              BadFWSCandidates[I].erase(E);
              InefficientFWSCandidates[I].erase(E);
            }
          }
        }

        for (auto *E : BadUsers)
          BadFWSCandidates[E->getIndex()].insert(FWSExpr);
        for (auto *E : InefficientUsers)
          InefficientFWSCandidates[E->getIndex()].insert(FWSExpr);

        FWSExpr = Expr->findFWSCandidate(FMABB, BadFWSCandidates[I],
                                         InefficientFWSCandidates[I],
                                         CanConsumeIfOneUser, Users);
      }
    }

    if (!Consumed && !CanConsumeIfOneUser) {
      // Give temporary permit to do FWS for expressions having only one use
      // if the fused expression is not worse than two separate expressions
      // before fusing.
      LLVM_DEBUG(dbgs() << "\n\n  FWS: Now set CanConsumeIfOneUser "
                           "to TRUE and repeat.\n\n");
      CanConsumeIfOneUser = true;
      Consumed = true;
    } else if (Consumed && CanConsumeIfOneUser) {
      LLVM_DEBUG(dbgs() << "\n\n  FWS: Now set CanConsumeIfOneUser "
                           "to FALSE to give priority to cases with "
                           "clean bonuses.\n\n");
      // 'CanConsumeIfOneUser' was a temporary permit to do FWS in cases where
      // it potentially gives more opportunities for doing beneficial FWS
      // cases. Revoke the permit and try to do efficient FWS cases now.
      CanConsumeIfOneUser = false;
    }
  } // end while (Consumed)

  LLVM_DEBUG(dbgs() << "\nFMA-STEP2 DONE. FMA basic block after FWS:\n"
                    << FMABB);
}

} // End anonymous namespace.

MachineFunctionPass *llvm::createCSAGlobalFMAPass() {
  return new CSAGlobalFMA();
}
