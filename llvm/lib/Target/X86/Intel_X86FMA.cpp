//====-- Intel_X86FMA.cpp - Fused Multiply Add optimization ---------------====
//
//      Copyright (c) 2016 Intel Corporation.
//      All rights reserved.
//
//        INTEL CORPORATION PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license
// agreement or nondisclosure agreement with Intel Corp.
// and may not be copied or disclosed except in accordance
// with the terms of that agreement.
//
// static char cvs_id[] = "$Id$";
//
// This file defines the pass which finds the best representations of
// the original expression trees consisting of MUL/ADD/SUB/FMA/UnarySub
// operations and performs transformations.
//
//  External interfaces:
//      FunctionPass *llvm::createX86GlobalFMAPass();
//      bool X86GlobalFMA::runOnMachineFunction(MachineFunction &MFunc);
//

#include "X86.h"
#include "X86InstrInfo.h"
#include "X86Subtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/Format.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "Intel_X86FMACommon.h"
using namespace llvm;

#define DEBUG_TYPE "x86-global-fma"

namespace {

/// This internal switch can be used to turn off the Global FMA optimization.
/// Currently the optimization is turned OFF by default.
static cl::opt<bool> DoFMAOpt("do-x86-global-fma",
                              cl::desc("Enable the global FMA opt."),
                              cl::init(false), cl::Hidden);

/// This internal switch regulates the amount of debug messages printed
/// by the Global FMA optimization.
static cl::opt<bool> DebugFMAOpt("debug-x86-global-fma",
                                 cl::desc("Control FMA debug printings."),
                                 cl::init(false), cl::Hidden);

/// The internal switch that is used to re-define FMA heuristics.
static cl::opt<unsigned> FMAControl("x86-global-fma-control",
                                    cl::desc("FMA heuristics control."),
                                    cl::init(0), cl::Hidden);

// This function was created to make it possible to generate DEBUG output
// with desired level of details. DEBUG_WITH_TYPE() macro does not let to do
// it as it is exclusive, i.e. you can specify only 1 level of detailization,
// while this optimization may want to have several levels of dump details.
raw_ostream &fmadbgs() { return (!DebugFMAOpt) ? nulls() : dbgs(); }

class FMABasicBlock;
class FMAExpr;
class FMATerm;
class FMARegisterTerm;
class FMAMemoryTerm;
class FMASpecialTerm;
class FMAExprSP;
class FMADag;

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
class FMAPatterns {
private:
  /// Represents a set of FMA patterns that all have the same SHAPE.
  struct FMAPatternsSet {
    const uint64_t *Dags;
    unsigned NumDags;

    /// Initializes a set of patterns using the given reference to an array
    /// \p Dags and the size of the array \p NumDags.
    FMAPatternsSet(const uint64_t *Dags, unsigned NumDags)
        : Dags(Dags), NumDags(NumDags) {}
  };

  /// All FMA patterns are stored as a vector of references to groups of Dags
  /// where each of the groups has the same SHAPE.
  /// It is also supposed that the groups of Dags are sorted by the SHAPE.
  std::vector<FMAPatternsSet *> Dags;

  /// Returns the number of shape (i.e. the number of Dag/pattern sets).
  unsigned getNumShapes() { return Dags.size(); }

  /// This map contains the sums of products created during the binary search
  /// performed on the pre-computed DAGs. The sums of products are saved to
  /// this map to speed up the searches that are performed not for the first
  /// time.
  std::map<uint64_t, FMAExprSP *> EncodedDagToSPMap;

  /// Returns a set of 64-bit encoded DAGs for the given \p Shape.
  /// If such set cannot be found then nullptr is returned.
  FMAPatternsSet *getDagsForShape(uint64_t Shape);

  /// Returns a sum of product generated for 64-bit int encoded DAG
  /// \p EncodedDag.
  FMAExprSP *acquireSP(uint64_t EncodedDag);

public:
  FMAPatterns(){};
  ~FMAPatterns(void) {
    for (auto D : Dags)
      delete D;
  }

  /// Initialize the patterns storage.
  /// Currently it is assumed that there is only one set of patterns for
  /// the target CPU. It may be changed in future, for example, there may
  /// be 2 separate pattern sets: for AVX and for AVX2. In such cases
  /// the init() method may get some input arguments and become more
  /// complex.
  void init() {
    // All the code that initializes the patterns storage is in the
    // following included header file.
#   include "X86GenMAPatterns.inc"
  }

  /// Returns an FMA DAG that would be the most efficient equivalent of the
  /// given sum of products \p SP.
  FMADag *getDagForBestSPMatch(const FMAExprSP &SP);
};

/// This class does all the optimization work, it goes through the functions,
/// searches for the optimizable expressions and replaces then with more
/// efficient equivalents.
class X86GlobalFMA : public MachineFunctionPass {
public:
  X86GlobalFMA()
      : MachineFunctionPass(ID), MF(nullptr), TII(nullptr), MRI(nullptr),
        Patterns(nullptr) {}
  ~X86GlobalFMA() { delete Patterns; }

  const char *getPassName() const override { return "X86 GlobalFMA"; }

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

  /// A reference to the function being currently compiled.
  MachineFunction *MF;

  /// This field is used to get information about available target operations.
  const X86InstrInfo *TII;

  /// Machine register information.
  MachineRegisterInfo *MRI;

  /// A storage with pre-computed/efficient FMA patterns.
  FMAPatterns *Patterns;

  /// This field if set to true means that the target CPU has AVX512 feature,
  /// and that the special AVX512 opcodes must be recognized in the input IR and
  /// generated to the output IR.
  bool HasAVX512;

  /// The bits that are used to define various FMA heuristics in the
  /// internal switch FMAControl/"-x86-global-fma-control".
  static const unsigned FMAControlHaswellFMAs = 0x1;
  static const unsigned FMAControlBroadwellFMAs = 0x2;
  static const unsigned FMAControlSkylakeFMAs = 0x4;
  static const unsigned FMAControlTargetFMAsMask = 0xFF;
  static const unsigned FMAControlForceFMAs = 0x100;

  /// Returns true iff all of the passed features \p F are enabled
  /// by the internal switch FMAControl/"-x86-global-fma-control".
  static bool checkAllFMAFeatures(unsigned F) {
    return (FMAControl & F) == F;
  }

  /// Returns true iff any of the passed features \p F is enabled
  /// by the internal switch FMAControl/"fma-control".
  static bool checkAnyOfFMAFeatures(unsigned F) {
    return (FMAControl & F) != 0;
  }

  /// Do the FMA optimization in one basic block.
  /// Return true iff any changes in the IR were made.
  bool optBasicBlock(MachineBasicBlock &MBB);

  /// Do the FMA optimizations in one parsed basic block \p FMABB.
  /// In particular, it tries to combine simple MUL/ADD/FMA operations
  /// registered in \p FMABB into bigger expressions, to find efficient
  /// replacements for them and if any optimizations are doable then update
  /// the input machine basic block \p MBB.
  /// Return true iff any changes in the IR (i.e. in \p MBB) were made.
  bool optParsedBasicBlock(FMABasicBlock &FMABB, MachineBasicBlock &MBB);

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
  void generateOutputIR(FMAExpr &Expr, const FMADag &Dag, FMABasicBlock &FMABB,
                        MachineBasicBlock &MBB);

  /// Returns true if the DAG \p Dag seems more efficient than the initial
  /// expression \p Expr being optimized now.
  /// The parameter \p TuneForLatency specifies if the latency aspect has
  /// the priority over the throughput.
  bool isDagBetterThanInitialExpr(const FMADag &Dag, const FMAExpr &Expr,
                                  bool TuneForLatency = true) const;
};

char X86GlobalFMA::ID = 0;

/// This class is derived from FMADagCommon representing FMA Directed Acyclic
/// Graphs. It adds some methods specific for code-generation.
class FMADag : public FMADagCommon {
public:
  /// Creates an FMA DAG for 64-bit encoded DAG from precomputed FMA DAGs.
  FMADag(uint64_t Encoded64) : FMADagCommon(Encoded64){};

  /// Creates a copy of the given DAG \p Dag.
  FMADag(const FMADagCommon &Dag) : FMADagCommon(Dag){};

  /// Returns true if the term or expression used in \p NodeInd and \p OpndInd
  /// operand is used the last time in this DAG.
  /// The word "last" should be understood in the way how code-generation phase
  /// would understand it. Code-geneneration translate the DAG to IR starting
  /// from the node with maximum index. So the term with
  /// (\p NodeInd, \p OpndInd) coordinates is the last if it is not used
  /// in nodes with indices 0 to (\p NodeInd - 1), and is not used
  /// in the operands 0 to (\p OpndInd - 1) in the node with index \p NodeInd.
  bool isLastUse(unsigned NodeInd, unsigned OpndInd) const;

  /// Returns true iff this FMA DAG has any node having the third operand
  /// equal to 1.0.
  bool hasPlusMinusOne() const;
};

/// This class is derived from FMAExprSPCommon representing expressions
/// consisting of MUL/ADD/SUB/FMA operations in canonical form, i.e. sum of
/// products. It adds only some methods specific for FMA optimization.
class FMAExprSP : public FMAExprSPCommon {
public:
  /// Default constructor. Creates an empty sum of products.
  FMAExprSP() : FMAExprSPCommon() {}

  /// Creates a sum of products consisting of only one product with the
  /// only term \p Term.
  FMAExprSP(unsigned Term) : FMAExprSPCommon(Term) {}

  /// Initializes the sum of products using the given DAG \p EncodedDag
  /// encoded in 64-bit integer.
  void initForEncodedDag(uint64_t EncodedDag);

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
  unsigned *getTermsMappingToCompactTerms();
};

/// This class represents FMA expressions and terms. It works as a bridge
/// between input IR and internal FMA structures, in particular it helps
/// to connect instances of MachineInstr/MachineOperand classes with machine
/// independent classes FMAExprSP/FMADag, get the most efficient representation
/// of input expression and to generate output IR.
class FMANode {
protected:
  /// Machine value type of the FMA expression or term.
  MVT VT;

public:
  /// Constructor. Initializes the only field \p VT available in this class.
  FMANode(MVT VT) : VT(VT) {}

  /// Destructor.
  virtual ~FMANode() {}

  /// A series of utility functions used to ask about the current FMA node
  /// kind, i.e. if it is an FMA expression, term, etc.
  /// All of the default versions return false. The derived classes must
  /// override some of these methods to identify themselves properly.
  virtual bool isFMA() const { return false; }
  virtual bool isTerm() const { return false; }
  virtual bool isZero() const { return false; }
  virtual bool isOne() const { return false; }
  virtual bool isRegisterTerm() const { return false; }
  virtual bool isMemoryTerm() const { return false; }
  virtual bool isSpecialTerm() const { return false; };

  /// Returns vector type of the FMA node.
  MVT getVT() const { return VT; }

  /// Downcast conversion from FMANode to FMAExpr.
  const FMAExpr *castToExpr() const {
    const FMAExpr *Expr = dyn_cast<FMAExpr>(this);
    assert(Expr && "Cannot downcast FMANode to FMAExpr.");
    return Expr;
  }
  /// Downcast conversion from FMANode to FMAExpr.
  FMAExpr *castToExpr() {
    FMAExpr *Expr = dyn_cast<FMAExpr>(this);
    assert(Expr && "Cannot downcast FMANode to FMAExpr.");
    return Expr;
  }

  /// Downcast conversion from FMANode to FMATerm.
  const FMATerm *castToTerm() const {
    const FMATerm *Term = dyn_cast<FMATerm>(this);
    assert(Term && "Cannot downcast FMANode to FMATerm.");
    return Term;
  }
  /// Downcast conversion from FMANode to FMATerm.
  FMATerm *castToTerm() {
    FMATerm *Term = dyn_cast<FMATerm>(this);
    assert(Term && "Cannot downcast FMANode to FMATerm.");
    return Term;
  }

  /// Prints the FMA expression or term to the given stream \p OS.
  /// The parameter \p PrintAttributes specifies if the caller wants to see
  /// more information and some of FMA node attributes should be printed out.
  virtual void print(raw_ostream &OS, bool PrintAttributes = false) const = 0;
};

/// Prints the FMA node \p Node to the given stream \p OS.
inline raw_ostream &operator<<(raw_ostream &OS, const FMANode &Node) {
  Node.print(OS);
  return OS;
}

/// This class represents a term or a leaf of an FMA expression tree.
class FMATerm : public FMANode {
protected:
  /// A virtual register associated with the result of the FMA term.
  /// The special value 0 means that the virtual register has not been
  /// assigned yet. For memory terms this field is equal to 0 until the load
  /// is generated and this field gets assigned to the virtual register
  /// associated with the result of the load. Similarly for special const
  /// terms, this field is also equal to 0 until the code for the constant
  /// is generated and this fields is assigned to the virtual register
  /// associated with the const.
  unsigned Reg;

public:
  /// Creates FMATerm node for a term associated with a virtual register \p Reg.
  /// The parameter \p VT specifies the type of the created expression.
  FMATerm(MVT VT, unsigned Reg) : FMANode(VT), Reg(Reg) {}

  /// Destructor.
  virtual ~FMATerm() {}

  /// This method overrides the parent implementation and just returns true.
  virtual bool isTerm() const override { return true; }

  /// Returns the virtual register associated with FMA term.
  unsigned getReg() const { return Reg; }

  /// Binds FMA term with a virtual register \p Reg.
  void setReg(unsigned Reg) { this->Reg = Reg; }

  /// Downcast conversion from FMATerm to FMAMemoryTerm.
  const FMAMemoryTerm *castToMemoryTerm() const {
    const FMAMemoryTerm *Term = dyn_cast<FMAMemoryTerm>(this);
    assert(Term && "Cannot downcast FMATerm to FMAMemoryTerm.");
    return Term;
  }
  /// Downcast conversion from FMATerm to FMAMemoryTerm.
  FMAMemoryTerm *castToMemoryTerm() {
    FMAMemoryTerm *Term = dyn_cast<FMAMemoryTerm>(this);
    assert(Term && "Cannot downcast FMATerm to FMAMemoryTerm.");
    return Term;
  }

  // Method for type inquiry through isa, cast and dyn_cast.
  static bool classof(const FMANode *Node) { return Node->isTerm(); }
};

/// This class represents an FMA term associated with a virtual register.
class FMARegisterTerm : public FMATerm {
private:
  /// The order number of the term in the FMABasicBlock.
  /// This field is used only for having convenient dumps of FMA basic block
  /// and FMA expressions.
  unsigned TermIndexInBB;

public:
  /// Creates FMARegisterTerm node for a term associated with a virtual
  /// register \p Reg.
  /// The parameter \p VT specifies the type of the created expression.
  /// The parameter \p TermIndexInBB defines the order number of the term in
  /// the FMA basic block being currently optimized.
  FMARegisterTerm(MVT VT, unsigned Reg, unsigned TermIndexInBB)
      : FMATerm(VT, Reg), TermIndexInBB(TermIndexInBB) {}

  /// This method overrides the parent implementation and just returns true.
  virtual bool isRegisterTerm() const override { return true; }

  /// Prints the FMA term to the given stream \p OS.
  /// The parameter \p PrintAttributes specifies if the caller wants to see
  /// more information and some of FMA node attributes should be printed out.
  void print(raw_ostream &OS, bool PrintAttributes) const override {
    OS << "T" << TermIndexInBB << "_vreg"
       << TargetRegisterInfo::virtReg2Index(Reg);
    if (PrintAttributes)
      OS << " // Type: " << EVT(VT).getEVTString();
  }
};

/// This class represents an FMA term associated with a load from memmory.
class FMAMemoryTerm : public FMATerm {
private:
  /// A reference to machine instruction having a memory operand usually
  /// represented as several machine operands used for memory base, offset, etc.
  const MachineInstr *MI;

  /// The order number of the term in the FMABasicBlock.
  /// This field is used only for having convenient dumps of FMA basic block
  /// and FMA expressions.
  unsigned TermIndexInBB;

public:
  /// Creates FMAMemoryTerm node for a term associated with a memory reference.
  /// The parameter \p VT specifies the type of the created term.
  /// The parameter \p MI passes a reference to machine instruction containing
  /// the load from memory.
  /// The parameter \p TermIndexInBB defines the order number of the term in
  /// the FMA basic block being currently optimized.
  FMAMemoryTerm(MVT VT, const MachineInstr *MI, unsigned IndexInBB)
      : FMATerm(VT, 0), MI(MI), TermIndexInBB(IndexInBB) {}

  /// This method overrides the parent implementation and just returns true.
  virtual bool isMemoryTerm() const override { return true; }

  /// Returns the reference to machine instruction associated with FMA term.
  const MachineInstr *getMI() const { return MI; }

  /// Prints the FMA term to the given stream \p OS.
  /// The parameter \p PrintAttributes specifies if the caller wants to see
  /// more information and some of FMA node attributes should be printed out.
  void print(raw_ostream &OS, bool PrintAttributes) const override {
    OS << "T" << TermIndexInBB << "_mem";
    if (PrintAttributes)
      OS << " // Type: " << EVT(VT).getEVTString() << "\n  MI: " << *MI;
  }

  // Method for type inquiry through isa, cast and dyn_cast.
  static bool classof(const FMATerm *Term) { return Term->isMemoryTerm(); }
};

/// This class represents a special FMA term associated with a floating point
/// constant, e.g. 0.0 or 1.0.
class FMASpecialTerm : public FMATerm {
private:
  unsigned SpecialValue;

public:
  /// Creates FMAExpr for a special term 0.0 or 1.0 of the type \p VT.
  /// The parameter \p SpecialValue can be equal to either 0 or 1 values.
  FMASpecialTerm(MVT VT, unsigned SpecialValue)
      : FMATerm(VT, 0), SpecialValue(SpecialValue) {}

  /// These methods override the parent implementations to identify the
  /// term properly.
  bool isZero() const override { return SpecialValue == 0; }
  bool isOne() const override { return SpecialValue == 1; }
  bool isSpecialTerm() const override { return true; }

  /// Prints the FMA expression or term to the given stream \p OS.
  /// The parameter \p PrintAttributes specifies if the caller wants to see
  /// more information and some of FMA node attributes should be printed out.
  void print(raw_ostream &OS, bool PrintAttributes) const override {
    OS << SpecialValue;
    if (PrintAttributes)
      OS << " // Type: " << EVT(VT).getEVTString();
  }
};

/// This class represents an FMA expression having 3 operands:
///   (MulSign)(A * B) + (AddSign)C;
/// The operands A, B, C can be of any class derived from FMANode class,
/// i.e. A, B, C can point to instances of FMAExpr, FMATerm, FMASpecialTerm,
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
///   - Sets of virtual registers marked as killed in the original IR, so such
///     registers must be marked as killed after replacement of the original
///     MachineInstr operations with the more efficient ones.
///   - Other references to IR if that helps to make the code-generation more
///     efficient.
class FMAExpr : public FMANode {
private:
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
  FMANode *Operands[3];

  /// This field is set to 'true' if the current FMA expression is a root
  /// expression, i.e. it is not a subexpression of some other FMA expression.
  bool IsRootExpr;

  /// A reference to an FMA term defined by 'this' FMA expression.
  FMARegisterTerm *ResultTerm;

  /// A vector of FMA Terms used in this FMA expression and subexpressions.
  /// Indices of terms in this vector are used as unsigned terms in FMAExprSP
  /// and FMADag objects created for this FMA expression later.
  /// This vector is maintained only for root FMA expressions, i.e. when
  /// the field IsRootExpr is set to 'true'.
  /// The following 3 operations are supposed to be the most common:
  /// a) Add a term to the vector if the term is not there yet.
  /// b) Get a term by an index.
  /// c) Get the index of some term that is known to be in the container.
  //
  /// FIXME: std::vector is not a perfect container. The complexity of the
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
  std::vector<FMATerm *> UsedTerms;

  /// A reference to machine instruction which is used as a reference point to
  /// an original MUL/ADD/FMA operation; it is used in FWS (Forward
  /// Substitution) that creates bigger FMAs by pulling some of FMA operations
  /// down to their users.
  const MachineInstr *MI;

  /// Virtual registers which are killed after being used in this
  /// FMA expression.
  /// This set is maintained only for root FMA expressions, i.e. when the field
  /// IsRootExpr is set to 'true'.
  std::set<unsigned> KilledRegs;

  /// A set of machine instructions corresponding to other FMA expressions
  /// consumed by this FMA expression. The machine instructions in this list
  /// must be removed from IR when/if this expression is translated back to IR.
  /// This set is maintained only for root FMA expressions, i.e. when the field
  /// IsRootExpr is set to 'true'.
  std::list<const MachineInstr *> ConsumedMIs;

  /// Returns an index for the given term \p Term. It is asserted that the
  /// provided term is used as an operand of one of FMA operations included
  /// into the expression tree referenced by 'this' FMA expression.
  ///
  /// Conversion of the term to an unsigned index may be needed to bind
  /// FMA terms represesnted as FMAExpr and terms represented as unsigned
  /// in FMAExprSP/FMADag classes.
  unsigned getUsedTermIndex(const FMATerm *Term) const;

  /// Registers the given term \p Term as a term used in 'this' FMA expression
  /// by adding the term to the container 'UsedTerms' if it is not already
  /// there.
  /// Usually, this method is used at the time of FMA expressions creation.
  void addToUsedTerms(FMATerm *Term);

  /// Register the terms from the given vector of terms \p Terms as used
  /// in 'this' FMA expression.
  /// Usually, this method is used when one FMA expression with several used
  /// terms gets included into another FMA expression. In such cases all terms
  /// used by the included FMA expression become terms used by the new bigger
  /// FMA expression.
  void addToUsedTerms(const std::vector<FMATerm *> &Terms) {
    for (auto T : Terms)
      addToUsedTerms(T);
  }

  /// Removes the given term \p Term from the list of used terms.
  /// This method is going to be used only when one expression gets consumed
  /// by another expression and the term defined by the consumed expression
  /// stops being used by the consuming expression. Thus the removed term
  /// can be only an FMARegisterTerm.
  void removeFromUsedTerms(FMARegisterTerm *Term);

  /// This method puts 'this' node and all subexpressions to the given
  /// set \p ExprSet. It may be needed when each expression node must be
  /// visited only once.
  void putExprToExprSet(std::set<const FMAExpr *> &ExprSet) const;

  /// Recursively walks through the expression nodes, builds sums of products
  /// for them and puts the created SPs to the given map \p ExprToSPMap.
  /// Returns the sum of products generated for 'this' FMA expression.
  /// The parameter \p RootFMAExpr is a reference to a root FMA expression,
  /// which holds the container with all used terms, which is needed to
  /// convert terms into unsigned indices/terms used in the result
  /// sum of products.
  FMAExprSP *generateSPRecursively(
      const FMAExpr *RootFMAExpr,
      std::map<const FMAExpr *, FMAExprSP *> &ExprToSPMap) const;

public:
  /// Create FMAExpr for ADD/SUB/FMA instruction.
  /// The parameter \p VT specifies the type of the created operation.
  /// The parameter \p MI passes a reference to machine instruction for which
  /// this FMAExpr is created.
  /// The parameter \p ResultTerm gives the reference to an FMARegisterTerm
  /// node created for the result of the FMAExpr operation being created here.
  /// The parameters \p Op1, \p Op2, \p Op3 give the references to FMA nodes
  /// used as operands of the created FMA expression.
  FMAExpr(MVT VT, const MachineInstr *MI, FMARegisterTerm *ResultTerm,
          FMANode *Op1, FMANode *Op2, FMANode *Op3);

  /// Destructor.
  virtual ~FMAExpr() {}

  /// This method overrides the parent implementation and simply returns true
  /// identifying 'this' FMA node as FMA expression.
  bool isFMA() const override { return true; }

  /// Returns the sign used for the product of the 1st and 2nd operands of
  /// the FMA operation. The returned value is true if the product is
  /// subtracted and false if it is added.
  bool getMulSign() const { return MulSign; }

  /// Returns the sign used for the 3rd operand of FMA expression.
  /// The returned value is true if the 3rd operand is subtracted.
  /// Otherwise, the returned value is false.
  bool getAddSign() const { return AddSign; }

  /// Sets the sign used for the product of the 1st and 2nd operands of FMA
  /// operation. The passed value 'true' of \p Sign is used when the product
  /// is subtracted, and the value 'false' is used when the product is added.
  void setMulSign(bool Sign) { MulSign = Sign; }

  /// Sets the sign used for the 3rd operand of FMA expression.
  /// The passed value 'true' of \p Sign is used when the 3rd operand is
  /// subtracted, and the value 'false' is used when the 3rd operand is added.
  void setAddSign(bool Sign) { AddSign = Sign; }

  /// Returns the operand of FMA operation with the index \p Index.
  FMANode *getOperand(unsigned Index) {
    assert(Index < 3 && "Operand index must be in the range 0 to 2.");
    return Operands[Index];
  };
  /// Returns the const operand of FMA operation with the index \p Index.
  const FMANode *getOperand(unsigned Index) const {
    assert(Index < 3 && "Operand index must be in the range 0 to 2.");
    return Operands[Index];
  };

  /// Returns the term associated with the result of this FMA expression.
  FMARegisterTerm *getResultTerm() const { return ResultTerm; }

  /// Returns the reference to machine instruction associated with
  /// FMA expression.
  const MachineInstr *getMI() const { return MI; }

  /// Returns the number of times the given term \p Term is used by 'this'
  /// expression and its subexpressions.
  unsigned countTermUses(const FMATerm *Term) const;

  /// Replaces the uses of the term \p Term with the uses of the expression
  /// \p Expr. Usually that is done when the expression \p Expr is consumed
  /// by 'this' expression.
  void replaceTermWithExpr(FMATerm *Term, FMAExpr *Expr);

  /// Includes the given expression \p Expr into 'this' expression. It is
  /// assumed that the term defined by \p Expr is used in 'this' expression.
  void consume(FMAExpr *Expr);

  /// Returns true iff 'this' FMA expression is included into some bigger
  /// FMA expression and does not show up as independent expression.
  bool isConsumed() const { return !IsRootExpr; }

  /// Marks the expression as consumed meaning that it exists only as
  /// a sub-expression of some bigger FMA expression.
  void markAsConsumed() { IsRootExpr = false; }

  /// Looks for an expression in the given basic block \p FMABB that could be
  /// included into 'this' expression. Returns a reference to such expression
  /// or nullptr if no consumable expressions were found.
  /// Expressions that are not used or used not only by 'this' expression are
  /// not considered as candidates for being consumed.
  /// The parameter \p MRI is passed to this method to make it possible to
  /// count the number of users of registers associated with the results of
  /// candidates for consumption.
  FMAExpr *findFWSCandidate(FMABasicBlock &FMABB,
                            const MachineRegisterInfo *MRI) const;

  /// Returns the number of used register and memory terms.
  unsigned getNumUsedTerms() const { return UsedTerms.size(); }

  /// Returns the used term by the index \p Index.
  FMATerm *getUsedTermByIndex(unsigned Index) const { return UsedTerms[Index]; }

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
  void compactTerms(FMAExprSP *SP);

  /// Generates and returns a sum of products for 'this' FMA expression.
  FMAExprSP *generateSP() const;

  /// Prints the FMA expression or term to the given stream \p OS.
  /// The parameter \p PrintAttributes specifies if the caller wants to see
  /// more information and some of FMA node attributes should be printed out.
  void print(raw_ostream &OS, bool PrintAttributes) const override;

  /// Method for type inquiry through isa, cast and dyn_cast.
  static bool classof(const FMANode *Node) { return Node->isFMA(); }
};

FMAExpr::FMAExpr(MVT VT, const MachineInstr *MI, FMARegisterTerm *ResultTerm,
                 FMANode *Op1, FMANode *Op2, FMANode *Op3)
    : FMANode(VT), MulSign(false), AddSign(false), IsRootExpr(true),
      ResultTerm(ResultTerm), MI(MI) {

  assert((Op1 && Op2 && Op3) && "Unexpected operands in FMAExpr constructor.");
  assert(ResultTerm && "Unexpected result term in FMAExpr constructor.");

  Operands[0] = Op1;
  Operands[1] = Op2;
  Operands[2] = Op3;
  if (Op1->isRegisterTerm() || Op1->isMemoryTerm())
    addToUsedTerms(Op1->castToTerm());
  if (Op2->isRegisterTerm() || Op2->isMemoryTerm())
    addToUsedTerms(Op2->castToTerm());
  if (Op3->isRegisterTerm() || Op3->isMemoryTerm())
    addToUsedTerms(Op3->castToTerm());

  // If the given MachineInstr has the last use for some of virtual register
  // terms, then add such registers to the list of registers that must
  // get isKill() attribute at the Machine IR emitting stage.
  unsigned NumOperands = MI->getNumOperands();
  const MachineRegisterInfo &MRI = MI->getParent()->getParent()->getRegInfo();
  const TargetRegisterClass *RC = MRI.getRegClass(ResultTerm->getReg());
  for (unsigned OpInd = 1; OpInd < NumOperands; OpInd++) {
    MachineOperand MO = MI->getOperand(OpInd);

    // It is assumed here that the result and all operands of FMA operation
    // have the same register class. The only exception from that rule is
    // the operand loaded from memory. The load from memory is represented by
    // the last several machine operands having different register class.
    // We do not want to add the last ones to the KilledRegs set as such
    // address registers would not be used and would only pollute KilledRegs.
    if (!MO.isReg())
      continue;
    unsigned Reg = MO.getReg();
    if (!TargetRegisterInfo::isVirtualRegister(Reg) ||
        RC != MRI.getRegClass(Reg))
      continue;

    if (MO.isKill())
      KilledRegs.insert(Reg);
  }
}

void FMAExpr::putExprToExprSet(std::set<const FMAExpr *> &ExprSet) const {
  if (ExprSet.find(this) != ExprSet.end())
    return;

  ExprSet.insert(this);
  for (auto Opnd : Operands)
    if (Opnd->isFMA())
      Opnd->castToExpr()->putExprToExprSet(ExprSet);
}

void FMAExpr::print(raw_ostream &OS, bool PrintType) const {
  if (IsRootExpr)
    OS << *ResultTerm << " = ";
  OS << (MulSign ? "FNM" : "FM") << (AddSign ? "S(" : "A(") << *Operands[0]
     << "," << *Operands[1] << "," << *Operands[2] << ")";
  if (PrintType)
    OS << " // Type: " << EVT(VT).getEVTString();
  if (IsRootExpr)
    OS << "\n  MI: " << *MI;
}

unsigned FMAExpr::getUsedTermIndex(const FMATerm *Term) const {
  std::vector<FMATerm *>::const_iterator B = UsedTerms.begin();
  std::vector<FMATerm *>::const_iterator E = UsedTerms.end();
  std::vector<FMATerm *>::const_iterator I = std::find(B, E, Term);
  assert(I != E && "Cannot find FMA term in the list of used terms.");
  return I - B;
}

void FMAExpr::addToUsedTerms(FMATerm *Term) {
  std::vector<FMATerm *>::iterator E = UsedTerms.end();
  if (std::find(UsedTerms.begin(), E, Term) == E)
    UsedTerms.push_back(Term);
}

void FMAExpr::removeFromUsedTerms(FMARegisterTerm *Term) {
  std::vector<FMATerm *>::iterator B = UsedTerms.begin();
  std::vector<FMATerm *>::iterator E = UsedTerms.end();
  std::vector<FMATerm *>::iterator I = std::find(B, E, Term);
  assert(I != E && "Cannot remove a term that is not in a list of used terms.");
  UsedTerms.erase(I);
}

void FMAExprSP::initForEncodedDag(uint64_t EncodedDag) {
  assert(Dag == nullptr && "initForEncodedDag() is applied to initialized SP");
  Dag = new FMADag(EncodedDag);
  bool isOk = initForDag(*Dag);
  assert(isOk && "Could not initialize SP for 64-bit encoded DAG.");

  canonize();
  computeShape();
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
        if (ProdInd != 0)
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

unsigned *FMAExprSP::getTermsMappingToCompactTerms() {

  // First of all get the mask showing what terms are used.
  // For each of the used terms set the corresponding bit in the bit mask.
  bool IsTermUsed[MaxNumOfUniqueTermsInSP] = {};
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
  unsigned *TermsMapping = new unsigned[MaxNumOfUniqueTermsInSP];
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

bool FMADag::hasPlusMinusOne() const {
  unsigned NumNodes = getNumNodes();
  for (unsigned NodeInd = 0; NodeInd < NumNodes; NodeInd++) {
    bool CIsTerm;
    unsigned C = getOperand(NodeInd, 2, &CIsTerm);
    if (CIsTerm && C == TermONE)
      return true;
  }
  return false;
}

FMAExprSP *FMAExpr::generateSPRecursively(
    const FMAExpr *RootFMAExpr,
    std::map<const FMAExpr *, FMAExprSP *> &ExprToSPMap) const {
  // If the sum of products is already initialized for 'this' FMA expression,
  // then return it.
  FMAExprSP *SP = ExprToSPMap[this];
  if (SP != nullptr)
    return SP;

  FMAExprSP *OperandSP[3];
  for (unsigned OpndInd = 0; OpndInd < 3; OpndInd++) {
    FMANode *Opnd = Operands[OpndInd];
    if (Opnd->isZero())
      SP = new FMAExprSP(FMAExprSPCommon::TermZERO);
    else if (Opnd->isOne())
      SP = new FMAExprSP(FMAExprSPCommon::TermONE);
    else if (Opnd->isTerm())
      SP = new FMAExprSP(RootFMAExpr->getUsedTermIndex(Opnd->castToTerm()));
    else if (Opnd->isFMA())
      SP = Opnd->castToExpr()->generateSPRecursively(RootFMAExpr, ExprToSPMap);
    else
      llvm_unreachable("Unsupported node kind.");

    // Sums of products must be available for all operands. Otherwise,
    // it is impossible to generate a sum of products for expression.
    if (!SP)
      return nullptr;

    OperandSP[OpndInd] = SP;
  }

  FMAExprSP MulSP;
  if (!MulSP.initForMul(*OperandSP[0], *OperandSP[1]))
    return nullptr;

  SP = new FMAExprSP();
  if (!SP->initForAdd(MulSP, *OperandSP[2], MulSign, AddSign)) {
    delete SP;
    return nullptr;
  }

  ExprToSPMap[this] = SP;
  return SP;
}

FMAExprSP *FMAExpr::generateSP() const {
  // Exit early if the number of terms is obviously too big and
  // SP cannot be built.
  if (UsedTerms.size() > FMAExprSP::MaxNumOfUniqueTermsInSP)
    return nullptr;

  std::map<const FMAExpr *, FMAExprSP *> ExprToSPMap;
  FMAExprSP *SP = generateSPRecursively(this, ExprToSPMap);

  if (SP) {
    SP->canonize();
    SP->computeShape();
  }

  // Free all temporarily allocated sums of products.
  for (auto S : ExprToSPMap) {
    if (S.second != SP)
      delete S.second;
  }
  return SP;
}

// The canonize() method may remove some of terms completely,
// For example,
//     Before: +ab+c-c+d
//     After : +ab+d
// The term 'c' got totally removed here. Let's compact the terms
// in SP and in 'this' FMAExpr.
void FMAExpr::compactTerms(FMAExprSP *SP) {
  unsigned *TermsMapping = SP->getTermsMappingToCompactTerms();
  if (TermsMapping) {
    SP->doTermsMapping(TermsMapping);

    // Now delete the unused terms from the vector UsedTerms.
    unsigned TermsMappingIndex = 0;
    for (auto I = UsedTerms.begin(); I != UsedTerms.end();) {
      // Terms mapping has the value ~0U if the corresponding term must be
      // removed.
      if (TermsMapping[TermsMappingIndex] == ~0U)
        I = UsedTerms.erase(I);
      else
        I++;
      TermsMappingIndex++;
    }
    delete[] TermsMapping;
  }
}

void FMAExpr::replaceTermWithExpr(FMATerm *Term, FMAExpr *Expr) {
  for (auto &Opnd : Operands)
    if (Opnd == Term)
      Opnd = Expr;
    else if (Opnd->isFMA())
      Opnd->castToExpr()->replaceTermWithExpr(Term, Expr);
}

void FMAExpr::consume(FMAExpr *FWSExpr) {
  FWSExpr->markAsConsumed();

  // Update the list of consumed machine instructions.
  // Also, FWSExpr does not need to keep the list of consumed machine
  // instructions anymore.
  ConsumedMIs.push_back(FWSExpr->getMI());
  ConsumedMIs.splice(ConsumedMIs.end(), FWSExpr->ConsumedMIs);

  // Term defined by the FWSExpr.
  FMARegisterTerm *FWSTerm = FWSExpr->getResultTerm();

  // Change the corresponding operands/terms with the reference to fws_expr.
  replaceTermWithExpr(FWSTerm, FWSExpr);

  // Remove FWSTerm from the set of used terms as it just got substituted by
  // the expression FWSExpr.
  removeFromUsedTerms(FWSTerm);

  // Add terms used by 'FWSExpr' to the list of terms used by the current
  // FMA expression.
  // Also, FWSExpr does not need to keep the list of used terms anymore.
  addToUsedTerms(FWSExpr->UsedTerms);
  FWSExpr->UsedTerms.clear();

  KilledRegs.insert(FWSExpr->KilledRegs.begin(), FWSExpr->KilledRegs.end());
  FWSExpr->KilledRegs.clear();

  DEBUG(fmadbgs() << "  -->After consuming expr: " << *this << "\n");
}

unsigned FMAExpr::countTermUses(const FMATerm *Term) const {
  std::set<const FMAExpr *> ExprSet;
  putExprToExprSet(ExprSet);

  unsigned NumUses = 0;
  for (auto E : ExprSet)
    for (auto Opnd : E->Operands)
      if (Opnd == Term)
        NumUses++;

  return NumUses;
}

/// This class represents one optimizable basic block. It holds all FMAExpr
/// objects created for operations in one MachineBasicBlock.
/// It also keeps references to special terms 0.0 and 1.0 created only once and
/// returned when they are used again.
class FMABasicBlock {
private:
  /// All FMA expressions available in the basic block.
  std::vector<FMAExpr *> FMAs;

  /// Register terms used or defined by FMA expressions in the basic block
  /// are stored into std::map to avoid creation of duplicated terms and
  /// to have quick search through already existing terms using virtual
  /// registers as keys.
  std::map<unsigned, FMARegisterTerm *> RegisterToFMARegisterTerm;

  /// This field maps terms to expressions defining those terms.
  /// For example, for any expression T1 = FMA1(...) there should be a pair
  /// <T1, FMA1> in this map.
  std::map<const FMATerm *, FMAExpr *> TermToDefFMA;

  /// Memory terms used by FMA expressions in the basic block are stored into
  /// std::map to avoid creation of duplicated terms and to have quick search
  /// through already existing terms using machine instructions as keys.
  std::map<const MachineInstr *, FMAMemoryTerm *> MIToFMAMemoryTerm;

  /// Special terms 0.0 and 1.0 created for the basic block.
  std::vector<FMASpecialTerm *> SpecialTerms;

  /// A reference to the machine basic block that is being optimized.
  const MachineBasicBlock &MBB;

public:
  /// Creates an FMA basic block for the given MachineBasicBlock \p MBB.
  FMABasicBlock(const MachineBasicBlock &MBB) : MBB(MBB) {}

  /// Destructor. Frees the references to FMAs, Terms, SpecialTerms.
  ~FMABasicBlock() {
    for (auto E : FMAs)
      delete E;
    for (auto T : RegisterToFMARegisterTerm)
      delete T.second;
    for (auto T : MIToFMAMemoryTerm)
      delete T.second;
    for (auto S : SpecialTerms)
      delete S;
  }

  /// Creates an FMA term for a special/const value of the given type \p VT.
  /// The parameter \p SpecialValue is an unsigned value representing
  /// a corresponding floating point value. For example, 0 and 1 values
  /// represent accordingly 0.0 and 1.0 floating point values.
  FMASpecialTerm *createSpecialTerm(MVT VT, unsigned SpecialValue);

  /// Creates an FMA term associated with the virtual register used in
  /// the passed machine operand \p MO. The parameter \p VT specifies
  /// the type of the created term.
  FMARegisterTerm *createRegisterTerm(MVT VT, const MachineOperand &MO);

  /// Creates an FMA term associated with a load from memory performed in
  /// the passed machine instruction \p MI. The parameter \p VT specifies
  /// the type of the created term.
  FMAMemoryTerm *createMemoryTerm(MVT VT, const MachineInstr *MI);

  /// Creates an FMA expression for a statement like this:
  ///   \p ResTerm = \p A * \p B + \p C.
  /// Returns a reference to the created FMA expression.
  /// The parameters:
  ///   \p MI - a reference to the MachineInstruction associated with the
  ///           created FMA node; usually it is an ADD operation.
  ///   \p VT - specifies the type information describing the number of
  ///           elements in the operation and the size of elements.
  ///   \p ResTerm - a reference to FMA term associated with the result
  ///                to where the result of created FMA is stored.
  ///   \p A, \pB, \p C - are the operands of created FMA(A, B, C).
  FMAExpr *createFMA(MVT VT, const MachineInstr *MI, FMARegisterTerm *ResTerm,
                     FMANode *A, FMANode *B, FMANode *C);

  /// Walks through all instructions in the machine basic block, finds
  /// MUL/ADD/FMA operations and creates FMA expressions (FMAExpr) for them.
  /// Returns the number of optimizable expressions found in the block.
  unsigned findFMAs();

  /// Returns a reference to FMA expression defining the given \p Term.
  FMAExpr *findDefiningFMA(const FMATerm *Term) const {
    auto I = TermToDefFMA.find(Term);
    if (I == TermToDefFMA.end())
      return nullptr;
    return I->second;
  }

  /// Returns the vector containing all FMAs available in this basic block.
  const std::vector<FMAExpr *> &getFMAs() const { return FMAs; };

  /// Prints the type to the given stream \p OS.
  void print(raw_ostream &OS) const;
};

/// Prints the FMA basic block \p FMABB to the given stream \p OS.
inline raw_ostream &operator<<(raw_ostream &OS, const FMABasicBlock &FMABB) {
  FMABB.print(OS);
  return OS;
}

FMASpecialTerm *FMABasicBlock::createSpecialTerm(MVT VT,
                                                 unsigned SpecialValue) {
  if (SpecialValue == 0) {
    for (auto T : SpecialTerms)
      if (T->isZero() && T->getVT() == VT)
        return T;
  } else if (SpecialValue == 1) {
    for (auto T : SpecialTerms)
      if (T->isOne() && T->getVT() == VT)
        return T;
  } else
    llvm_unreachable("Only special terms for 0.0 and 1.0 are supported now.");

  FMASpecialTerm *Term = new FMASpecialTerm(VT, SpecialValue);
  SpecialTerms.push_back(Term);
  return Term;
}

FMARegisterTerm *FMABasicBlock::createRegisterTerm(MVT VT,
                                                   const MachineOperand &MO) {
  assert(MO.isReg() && "Cannot create an FMA term for MachineOperand.");
  unsigned Reg = MO.getReg();

  // If there is a term created for this machine operand (or identical to it)
  // then just return the existing term. Otherwise, create a new term.
  FMARegisterTerm *Term = RegisterToFMARegisterTerm[Reg];
  if (!Term) {
    Term = new FMARegisterTerm(VT, Reg, RegisterToFMARegisterTerm.size() +
                                            MIToFMAMemoryTerm.size());
    RegisterToFMARegisterTerm[Reg] = Term;
  }
  return Term;
}

FMAMemoryTerm *FMABasicBlock::createMemoryTerm(MVT VT, const MachineInstr *MI) {
  // If there is an FMA term created for this memory reference then just
  // return the existing term. Otherwise, create a new term.
  FMAMemoryTerm *Term = MIToFMAMemoryTerm[MI];
  if (!Term) {
    // FIXME: If there are two different machine instructions having loads
    // from exactly the same memory and there are no stores to that memory
    // between those loads, then the memory term created for the first machine
    // instruction could be re-used in the next machine instruction.
    // Currently, we just create a new memory term.
    Term = new FMAMemoryTerm(VT, MI, RegisterToFMARegisterTerm.size() +
                                         MIToFMAMemoryTerm.size());
    MIToFMAMemoryTerm[MI] = Term;
  }
  return Term;
}

FMAExpr *FMABasicBlock::createFMA(MVT VT, const MachineInstr *MI,
                                  FMARegisterTerm *ResTerm, FMANode *Op1,
                                  FMANode *Op2, FMANode *Op3) {
  FMAExpr *Expr = new FMAExpr(VT, MI, ResTerm, Op1, Op2, Op3);
  FMAs.push_back(Expr);
  TermToDefFMA[ResTerm] = Expr;
  return Expr;
}

unsigned FMABasicBlock::findFMAs() {
  DEBUG(fmadbgs() << "FMA-STEP1: FIND FMA OPERATIONS:\n");

  for (const auto &MI : MBB) {
    unsigned Form = 0;
    bool AddSign = false;
    bool MulSign = false;
    bool IsMem = false;
    MVT VT;

    switch (MI.getOpcode()) {
    case X86::VADDSDrm:
      IsMem = true;
    case X86::VADDSDrr:
      Form = 13;
      VT = MVT::v1f64;
      break;
    case X86::VADDSSrm:
      IsMem = true;
    case X86::VADDSSrr:
      Form = 13;
      VT = MVT::v1f32;
      break;
    case X86::VSUBSDrm:
      IsMem = true;
    case X86::VSUBSDrr:
      Form = 13;
      VT = MVT::v1f64;
      AddSign = true;
      break;
    case X86::VSUBSSrm:
      IsMem = true;
    case X86::VSUBSSrr:
      Form = 13;
      VT = MVT::v1f32;
      AddSign = true;
      break;
    case X86::VMULSDrm:
      IsMem = true;
    case X86::VMULSDrr:
      Form = 12;
      VT = MVT::v1f64;
      break;
    case X86::VMULSSrm:
      IsMem = true;
    case X86::VMULSSrr:
      Form = 12;
      VT = MVT::v1f32;
      break;

    case X86::VFMADDSDr132m:
      IsMem = true;
    case X86::VFMADDSDr132r:
      Form = 132;
      VT = MVT::v1f64;
      break;
    case X86::VFMADDSSr132m:
      IsMem = true;
    case X86::VFMADDSSr132r:
      Form = 132;
      VT = MVT::v1f32;
      break;
    case X86::VFMSUBSDr132m:
      IsMem = true;
    case X86::VFMSUBSDr132r:
      Form = 132;
      VT = MVT::v1f64;
      AddSign = true;
      break;
    case X86::VFMSUBSSr132m:
      IsMem = true;
    case X86::VFMSUBSSr132r:
      Form = 132;
      VT = MVT::v1f32;
      AddSign = true;
      break;
    case X86::VFNMADDSDr132m:
      IsMem = true;
    case X86::VFNMADDSDr132r:
      Form = 132;
      VT = MVT::v1f64;
      MulSign = true;
      break;
    case X86::VFNMADDSSr132m:
      IsMem = true;
    case X86::VFNMADDSSr132r:
      Form = 132;
      VT = MVT::v1f32;
      MulSign = true;
      break;
    case X86::VFNMSUBSDr132m:
      IsMem = true;
    case X86::VFNMSUBSDr132r:
      Form = 132;
      VT = MVT::v1f64;
      AddSign = true;
      MulSign = true;
      break;
    case X86::VFNMSUBSSr132m:
      IsMem = true;
    case X86::VFNMSUBSSr132r:
      Form = 132;
      VT = MVT::v1f32;
      AddSign = true;
      MulSign = true;
      break;

    case X86::VFMADDSDr213m:
      IsMem = true;
    case X86::VFMADDSDr213r:
      Form = 213;
      VT = MVT::v1f64;
      break;
    case X86::VFMADDSSr213m:
      IsMem = true;
    case X86::VFMADDSSr213r:
      Form = 213;
      VT = MVT::v1f32;
      break;
    case X86::VFMSUBSDr213m:
      IsMem = true;
    case X86::VFMSUBSDr213r:
      Form = 213;
      VT = MVT::v1f64;
      AddSign = true;
      break;
    case X86::VFMSUBSSr213m:
      IsMem = true;
    case X86::VFMSUBSSr213r:
      Form = 213;
      VT = MVT::v1f32;
      AddSign = true;
      break;
    case X86::VFNMADDSDr213m:
      IsMem = true;
    case X86::VFNMADDSDr213r:
      Form = 213;
      VT = MVT::v1f64;
      MulSign = true;
      break;
    case X86::VFNMADDSSr213m:
      IsMem = true;
    case X86::VFNMADDSSr213r:
      Form = 213;
      VT = MVT::v1f32;
      MulSign = true;
      break;
    case X86::VFNMSUBSDr213m:
      IsMem = true;
    case X86::VFNMSUBSDr213r:
      Form = 213;
      VT = MVT::v1f64;
      AddSign = true;
      MulSign = true;
      break;
    case X86::VFNMSUBSSr213m:
      IsMem = true;
    case X86::VFNMSUBSSr213r:
      Form = 213;
      VT = MVT::v1f32;
      AddSign = true;
      MulSign = true;
      break;

    case X86::VFMADDSDr231m:
      IsMem = true;
    case X86::VFMADDSDr231r:
      Form = 231;
      VT = MVT::v1f64;
      break;
    case X86::VFMADDSSr231m:
      IsMem = true;
    case X86::VFMADDSSr231r:
      Form = 231;
      VT = MVT::v1f32;
      break;
    case X86::VFMSUBSDr231m:
      IsMem = true;
    case X86::VFMSUBSDr231r:
      Form = 231;
      VT = MVT::v1f64;
      AddSign = true;
      break;
    case X86::VFMSUBSSr231m:
      IsMem = true;
    case X86::VFMSUBSSr231r:
      Form = 231;
      VT = MVT::v1f32;
      AddSign = true;
      break;
    case X86::VFNMADDSDr231m:
      IsMem = true;
    case X86::VFNMADDSDr231r:
      Form = 231;
      VT = MVT::v1f64;
      MulSign = true;
      break;
    case X86::VFNMADDSSr231m:
      IsMem = true;
    case X86::VFNMADDSSr231r:
      Form = 231;
      VT = MVT::v1f32;
      MulSign = true;
      break;
    case X86::VFNMSUBSDr231m:
      IsMem = true;
    case X86::VFNMSUBSDr231r:
      Form = 231;
      VT = MVT::v1f64;
      AddSign = true;
      MulSign = true;
      break;
    case X86::VFNMSUBSSr231m:
      IsMem = true;
    case X86::VFNMSUBSSr231r:
      Form = 231;
      VT = MVT::v1f32;
      AddSign = true;
      MulSign = true;
      break;
    default:
      break;
    }

    if (Form == 0)
      continue;

    FMATerm *Op1, *Op2, *Op3;
    FMATerm *MemTerm = IsMem ? createMemoryTerm(VT, &MI) : nullptr;
    switch (Form) {
    case 12: // op1 * op2 + 0
      Op1 = createRegisterTerm(VT, MI.getOperand(1));
      Op2 = IsMem ? MemTerm : createRegisterTerm(VT, MI.getOperand(2));
      Op3 = createSpecialTerm(VT, 0);
      break;
    case 13: // op1 * 1 + op3, op1 * 1 - op3
      Op1 = createRegisterTerm(VT, MI.getOperand(1));
      Op2 = createSpecialTerm(VT, 1);
      Op3 = IsMem ? MemTerm : createRegisterTerm(VT, MI.getOperand(2));
      break;
    case 132: // op1 * op3 + op2,  -op1 * op3 + op2,
              // op1 * op3 - op2,  -op1 * op3 - op2
      Op1 = createRegisterTerm(VT, MI.getOperand(1));
      Op2 = IsMem ? MemTerm : createRegisterTerm(VT, MI.getOperand(3));
      Op3 = createRegisterTerm(VT, MI.getOperand(2));
      break;
    case 213: // op2 * op1 + op3,  -op2 * op1 + op3,
              // op2 * op1 - op3,  -op2 * op1 - op3
      Op1 = createRegisterTerm(VT, MI.getOperand(2));
      Op2 = createRegisterTerm(VT, MI.getOperand(1));
      Op3 = IsMem ? MemTerm : createRegisterTerm(VT, MI.getOperand(3));
      break;
    case 231: // op2 * op3 + op1,  -op2 * op3 + op1,
              // op2 * op3 - op1,  -op2 * op3 - op1
      Op1 = createRegisterTerm(VT, MI.getOperand(2));
      Op2 = IsMem ? MemTerm : createRegisterTerm(VT, MI.getOperand(3));
      Op3 = createRegisterTerm(VT, MI.getOperand(1));
      break;
    default:
      break;
    }
    FMARegisterTerm *ResTerm = createRegisterTerm(VT, MI.getOperand(0));
    FMAExpr *Expr = createFMA(VT, &MI, ResTerm, Op1, Op2, Op3);
    Expr->setMulSign(MulSign);
    Expr->setAddSign(AddSign);
  }
  DEBUG(print(fmadbgs()));
  DEBUG(fmadbgs() << "FMA-STEP1 DONE.\n");
  return FMAs.size();
}

void FMABasicBlock::print(raw_ostream &OS) const {
  OS << "\nFMA MEMORY TERMs:\n";
  for (auto T : MIToFMAMemoryTerm) {
    OS << "  ";
    T.second->print(OS, true /* PrintAttributes */);
  }

  OS << "\nFMA REGISTER TERMs:\n  ";
  for (auto T : RegisterToFMARegisterTerm) {
    T.second->print(OS, true /* PrintAttributes */);
    OS << "\n  ";
  }

  OS << "\nFMA EXPRESSIONs:\n";
  unsigned Index = 0;
  for (auto E : FMAs) {
    if (!E->isConsumed()) {
      OS << "  " << Index++ << ": ";
      E->print(OS, true /* PrintType */);
      OS << "\n";
    }
  }
}

FMAExpr *FMAExpr::findFWSCandidate(FMABasicBlock &FMABB,
                                   const MachineRegisterInfo *MRI) const {

  // Walk through all terms used by the current FMA, find those that are the
  // results of other FMAs.
  for (FMATerm *Term : UsedTerms) {
    FMAExpr *TermDefFMA = FMABB.findDefiningFMA(Term);
    if (TermDefFMA == nullptr)
      continue;

    // If TermDefFMA was consumed, then TermDefFMA expression would be part of
    // 'this' expression and the term defined by TermDefFMA would not be used
    // as an operand of 'this' expression.
    assert(!TermDefFMA->isConsumed() && "Cannot consume one expression twice");

    // This place would be good for doing safety check verifying that it is Ok
    // to use the virtual register associated with 'Term' at the point where
    // the machine instruction associated with this FMA expression is located.
    // For FMARegisterTerm we can just use the virtual register as it is SSA
    // form and the register is virtual at this phase. For FMAMemoryTerm the
    // virtual register is not assigned yet, but it is going to be defined
    // right before the load instruction associated with FMAMemoryTerm and
    // it should be Ok to use such term in 'this' FMA expression.
    // So, no checks are performed here.

    // Check that there is only one FMA expression using this term.
    //
    // TODO: In some cases it is profitable to let the expression TermDefFMA
    // be consumed by several independent expressions, but such transformation
    // should avoid unnecessary operations duplication.
    // Good test case:
    //   t1 = a * b; // TermDefFMA has 2 users.
    //   t2 = t1 + c;
    //   t3 = t1 + d;
    // Fusing may replace 3 operations with 2 FMA operations:
    //   t2 = a * b + c;
    //   t3 = a * b + d;
    // Bad test case:
    //   t1 = a * b * c; // TermDefFMA has 2 users.
    //   t2 = t1 * d;
    //   t3 = t1 * e;
    // Fusing just increases the number of operations from 4 to 6:
    //   t2 = a * b * c * d;
    //   t3 = a * b * c * e;
    // Consumption of expressions having two or more users requires having
    // in-flight pattern matching giving quick estimation of whether it is
    // efficient.
    unsigned Reg = Term->getReg();
    if (!MRI->hasOneNonDBGUse(Reg)) {
      // A. Count the number of 'Reg' uses in IR.
      iterator_range<MachineRegisterInfo::use_nodbg_iterator> RegUses =
        MRI->use_nodbg_operands(Reg);
      unsigned NumRegUses = std::distance(RegUses.begin(), RegUses.end());
      DEBUG(fmadbgs() << "  The register vreg"
                      << TargetRegisterInfo::virtReg2Index(Reg) << " has "
                      << NumRegUses << " uses.\n");

      // B. How many times 'Term' is used in 'this'?
      unsigned NumTermUses = countTermUses(Term);
      DEBUG(fmadbgs()
            << "  The term corresponding to register mentioned above is used "
            << NumTermUses << " times.\n");

      // C. Compare A and B.
      if (NumTermUses < NumRegUses)
        continue;
    }

    // TODO: this is a workaround for lack of efficient partitioning of
    // big and huge expressions. This check should be removed when
    // such partitioning is implemented.
    //
    // The consumption is possible, but before returning the expression
    // as a good candidate let's first check if the new expression after
    // such fuse would be optimizable. Remember that too big expressions are
    // not optimized now. In particular, expressions having more than
    // FMADagCommon::MaxNumOfUniqueTermsInDAG terms cannot be optimized.
    std::set<FMATerm *> NewTermsSet;
    NewTermsSet.insert(UsedTerms.begin(), UsedTerms.end());
    NewTermsSet.insert(TermDefFMA->UsedTerms.begin(),
                       TermDefFMA->UsedTerms.end());
    if (NewTermsSet.size() > FMADagCommon::MaxNumOfUniqueTermsInDAG)
      continue;

    return TermDefFMA;
  }

  return nullptr;
}

/// Loop over all of the basic blocks, performing the FMA optimization for
/// each block separately.
bool X86GlobalFMA::runOnMachineFunction(MachineFunction &MFunc) {
  if (!DoFMAOpt)
    return false;

  MF = &MFunc;
  const X86Subtarget &ST = MF->getSubtarget<X86Subtarget>();
  TII = ST.getInstrInfo();
  MRI = &MF->getRegInfo();

  // SubTarget must support FMA ISA.
  if (!ST.hasFMA())
    return false;

  // Compilation options must allow FP contraction and FP expression
  // re-association.
  const TargetOptions &Options = MF->getTarget().Options;
  if (Options.AllowFPOpFusion != FPOpFusion::Fast || !Options.UnsafeFPMath)
    return false;

  // Even though the compilation switches allow the Global FMA optimization it
  // still may be unsafe to do it as some of MUL/ADD/SUB/etc machine
  // instructions could be generated for LLVM IR operations with unset
  // 'fast-math' attributes. Such LLVM IR operations may be added to the
  // currently compiled function by the inlining optimization controlled by
  // -flto switch.
  // The 'fast-math' attributes get lost after LLVM IR to MachineInstr
  // translation. So, it is generally incorrect to do any unsafe algebra
  // transformations at the MachineInstr IR level.
  // FIXME: The ideal solution for this problem would be to have 'fast-math'
  // attributes defined for individual MachineInstr operations.
  // The currently used solution is rather temporary.
  //
  // Check the LLVM IR function. If there are some instructions in it with
  // attributes not allowing unsafe algebra, then exit.
  const Function *F = MF->getFunction();
  // If LLVM IR is not available, then just conservatively exit.
  if (!F)
    return false;
  for (auto &I : instructions(F)) {
    // isa<FPMathOperator>(&I) returns true for any operation having FP result.
    // In particular, it returns true for FP loads, which never have
    // the Fast-Math attributes set. Thus this opcode check is needed to
    // avoid mess with FP loads and other FMA opt unrelated operations.
    unsigned Opcode = I.getOpcode();
    bool CheckedOp =
        Opcode == Instruction::FAdd || Opcode == Instruction::FSub ||
        Opcode == Instruction::FMul || Opcode == Instruction::FDiv ||
        Opcode == Instruction::FRem || Opcode == Instruction::FCmp ||
        Opcode == Instruction::Call;
    if (CheckedOp && isa<FPMathOperator>(&I) && !I.hasUnsafeAlgebra()) {
      DEBUG(fmadbgs() << "Exit because found mixed fast-math settings.\n");
      return false;
    }
  }

  // The patterns storage initialization code is not cheap, so it is better
  // to call it only when FMA instructions have a chance to be generated.
  // Also, if the patterns storage is already created/initialized once, it
  // does not make sense to re-initialize it again.
  // This place may need to be updated if/when the patterns storage
  // initialization gets dependant on the target CPU settings. For example,
  // if the patterns are initialized one way for AVX, another way for AVX2,
  // and there are functions with different target CPU setttings.
  if (Patterns == nullptr) {
    Patterns = new FMAPatterns();
    Patterns->init();
  }

  HasAVX512 = ST.hasAVX512();
  if ((HasAVX512 && !checkAnyOfFMAFeatures(FMAControlTargetFMAsMask)) ||
      checkAllFMAFeatures(FMAControlSkylakeFMAs)) {
    AddSubLatency = 4;
    MulLatency = 4;
    FMALatency = 4;
  } else if (checkAllFMAFeatures(FMAControlBroadwellFMAs)) {
    AddSubLatency = 3;
    MulLatency = 3;
    FMALatency = 5;
  } else {
    // Haswell is the last available option.
    AddSubLatency = 3;
    MulLatency = 5;
    FMALatency = 5;
  }

  bool EverMadeChangeInFunc = false;

  // Process all basic blocks.
  for (MachineFunction::iterator I = MF->begin(), E = MF->end(); I != E; ++I)
    if (optBasicBlock(*I))
      EverMadeChangeInFunc = true;

  DEBUG(dbgs() << "********** X86 Global FMA **********\n");
  if (EverMadeChangeInFunc) {
    DEBUG(MF->print(dbgs()));
  }

  return EverMadeChangeInFunc;
}

/// Loop over all of the instructions in the basic block, optimizing
/// MUL/ADD/FMA expressions. Return true iff any changes in the machine
/// operation were done.
bool X86GlobalFMA::optBasicBlock(MachineBasicBlock &MBB) {
  DEBUG(fmadbgs() << "\n**** RUN FMA OPT FOR ANOTHER BASIC BLOCK ****\n");

  // Save the dump of the basic block, we may want to print it after the basic
  // block is changed by this optimization.
  std::string LogBBStr = "";
  raw_string_ostream LogBB(LogBBStr);
  DEBUG(LogBB << "Basic block before Global FMA opt:\n" << MBB << "\n");

  FMABasicBlock FMABB(MBB);

  // Find MUL/ADD/SUB/FMA/etc operations in the input machine instructions
  // and create internal FMA structures for them.
  // Exit if there are not enough optimizable expressions.
  if (FMABB.findFMAs() < 2)
    return false;

  // Run the FMA optimization and dump the debug messages if the optimization
  // produced any changes in IR.
  bool EverMadeChangeInBB = optParsedBasicBlock(FMABB, MBB);
  if (EverMadeChangeInBB) {
    DEBUG(fmadbgs() << LogBB.str());
    DEBUG(fmadbgs() << "\nBasic block after Global FMA opt:\n" << MBB << "\n");
  }
  return EverMadeChangeInBB;
}

bool X86GlobalFMA::optParsedBasicBlock(FMABasicBlock &FMABB,
                                       MachineBasicBlock &MBB) {
  bool EverMadeChangeInBB = false;
  doFWS(FMABB);

  DEBUG(fmadbgs() << "\nFMA-STEP3: DO PATTERN MATCHING AND CODE-GEN:\n");
  for (FMAExpr *Expr : FMABB.getFMAs()) {
    if (Expr->isConsumed())
      continue;

    DEBUG(fmadbgs() << "  Optimize FMA EXPR:\n  " << *Expr);
    FMAExprSP *SP = Expr->generateSP();
    if (!SP) {
      DEBUG(fmadbgs() << "  Could not compute SP.\n");
      continue;
    }

    // The returned SP might have some opportunities for terms compact.
    // For example, for initial FMAExpr expression
    //   +ab+c-c+d
    // the returned SP may be shorter and the term 'c' is not used anymore:
    //   +ab+d
    // Let's compact the terms in SP and in 'this' FMAExpr.
    Expr->compactTerms(SP);

    DEBUG(fmadbgs() << "  Computed SP is: ");
    DEBUG(SP->print(fmadbgs()));
    DEBUG(fmadbgs() << "  SHAPE: " << format_hex(SP->Shape, 2) << "\n\n");

    FMADag *Dag = Patterns->getDagForBestSPMatch(*SP);
    if (!Dag)
      continue;
    SP->Dag = Dag;

    DEBUG(fmadbgs() << "  CONGRATULATIONS! A searched DAG was found:\n    ");
    DEBUG(Dag->print(fmadbgs()));
    if (!isDagBetterThanInitialExpr(*Dag, *Expr, true)) {
      DEBUG(fmadbgs() << "  DAG is NOT better than the initial EXPR.\n\n");
      continue;
    }
    DEBUG(fmadbgs() << "  DAG IS better than the initial EXPR.\n");

    EverMadeChangeInBB = true;
    generateOutputIR(*Expr, *Dag, FMABB, MBB);
  }
  DEBUG(fmadbgs() << "\nFMA-STEP3 IS DONE. Machine basic block IS "
                  << (EverMadeChangeInBB ? "" : "NOT ") << "UPDATED.\n\n");
  return EverMadeChangeInBB;
}

FMAExprSP *FMAPatterns::acquireSP(uint64_t EncodedDag) {
  FMAExprSP *SP = EncodedDagToSPMap[EncodedDag];

  if (!SP) {
    SP = new FMAExprSP();
    SP->initForEncodedDag(EncodedDag);
    EncodedDagToSPMap[EncodedDag] = SP;
  }

  return SP;
}

FMAPatterns::FMAPatternsSet *FMAPatterns::getDagsForShape(uint64_t Shape) {
  unsigned First = 0, Last = getNumShapes() - 1;

  // If the passed 'Shape' is bigger than the biggest available shape in
  // the storage, then just exit early and skip the binary search.
  FMAExprSP *SP = acquireSP(Dags[Last]->Dags[0]);
  if (Shape > SP->Shape)
    return nullptr;
  if (Shape == SP->Shape)
    return Dags[Last];

  while (First < Last) {
    // Check the SHAPE of a set of DAGs in the middle of the search scope.
    unsigned Middle = (First + Last) / 2;
    SP = acquireSP(Dags[Middle]->Dags[0]);
    uint64_t CurShape = SP->Shape;

    // If the searched SHAPE is found, then return the whole set of DAGs having
    // the same SHAPE.
    if (Shape == CurShape)
      return Dags[Middle];

    // Halve the search scope and continue the binary search.
    if (Shape < CurShape)
      Last = Middle;
    else
      First = Middle + 1;
  }

  return nullptr;
}

// This routine checks if it is possible to match the current SP and the
// given SP. Such matching is often possible if the given SP is just a more
// general form of 'this' SP, i.e. the given SP has more terms than 'this' SP.
FMADag *FMAPatterns::getDagForBestSPMatch(const FMAExprSP &SP) {

  FMAPatternsSet *DagsSet = getDagsForShape(SP.Shape);
  if (!DagsSet)
    return nullptr;

  DEBUG(fmadbgs() << "  MATCHING: could find a set of DAGs for SHAPE("
                  << format_hex(SP.Shape, 2) << ")\n");

  // Find the best DAG for the given SP.
  FMADagCommon *BestDag = nullptr;
  for (unsigned i = 0; i < DagsSet->NumDags; i++) {
    uint64_t Dag64 = DagsSet->Dags[i];

    // FIXME: TermONE in C position is not yet supported, e.g. (A*B+1). Fix it.
    FMADag Dag(Dag64);
    if (Dag.hasPlusMinusOne())
      continue;

    FMAExprSP *CandidateSP = acquireSP(Dag64);

    DEBUG(fmadbgs() << "  MATCHING: let's try to match 2 SPs:\n    actual: ");
    SP.print(fmadbgs());
    DEBUG(fmadbgs() << "    formal: ");
    CandidateSP->print(fmadbgs());

    FMASPToSPMatcher SPMatcher;
    FMADagCommon *CandidateDag = SPMatcher.getDagToMatchSPs(*CandidateSP, SP);

    if (CandidateDag) {
      // Ok, we found Sum Of Products. Let's do some heuristical checks
      // and choose the best alternative here.

      // FIXME: currently we just choose the first one and return it.
      BestDag = CandidateDag;
      break;
    }
  }

  if (BestDag) {
    FMADag *Dag = new FMADag(*BestDag);
    delete BestDag;
    return Dag;
  }

  return nullptr;
}

void X86GlobalFMA::generateOutputIR(FMAExpr &Expr, const FMADag &Dag,
                                    FMABasicBlock &FMABB,
                                    MachineBasicBlock &MBB) {
  // Not implemented yet.
}

bool X86GlobalFMA::isDagBetterThanInitialExpr(const FMADag &Dag,
                                              const FMAExpr &Expr,
                                              bool TuneForLatency) const {
  if (checkAllFMAFeatures(FMAControlForceFMAs))
    return true;

  // Not implemented yet.
  return false;
}

void X86GlobalFMA::doFWS(FMABasicBlock &FMABB) {

  DEBUG(fmadbgs() << "\nFMA-STEP2: DO FWS:\n");

  bool Consumed = true;
  while (Consumed) {
    Consumed = false;

    for (auto Expr : FMABB.getFMAs()) {
      if (Expr->isConsumed())
        continue;

      DEBUG(fmadbgs() << "  FWS: try to find terms that could be substituted "
                         "by expressions in: "
                      << *Expr << "\n");
      FMAExpr *FWSExpr = Expr->findFWSCandidate(FMABB, MRI);
      while (FWSExpr) {
        DEBUG(fmadbgs() << "  -->Found such a term/expression:\n  " << *FWSExpr
                        << "    to\n  " << *Expr << "\n");
        Expr->consume(FWSExpr);
        Consumed = true;

        FWSExpr = Expr->findFWSCandidate(FMABB, MRI);
      }
    }
  }
  DEBUG(fmadbgs() << "\nFMA-STEP2 DONE. FMA basic block after FWS:\n" << FMABB);
}
} // End anonymous namespace.

FunctionPass *llvm::createX86GlobalFMAPass() { return new X86GlobalFMA(); }
