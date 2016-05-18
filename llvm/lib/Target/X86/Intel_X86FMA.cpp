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
#include "llvm/CodeGen/Passes.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "Intel_X86FMACommon.h"
using namespace llvm;

#define DEBUG_TYPE "x86-global-fma"

namespace {
// This internal switch can be used to turn off the Global FMA optimization.
// Currently the optimization is turned OFF by default.
static cl::opt<bool> DoFMAOpt("do-x86-global-fma",
                              cl::desc("Enable the global FMA opt."),
                              cl::init(false), cl::Hidden);
// This internal switch regulates the amount of debug messages printed
// by the Global FMA optimization.
static cl::opt<bool> DebugFMAOpt("debug-x86-global-fma",
                                 cl::desc("Control FMA debug printings."),
                                 cl::init(false), cl::Hidden);

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

/// This class holds all pre-computed/efficient FMA patterns/DAGs encoded in
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
};

/// This class does all the optimization work, it goes through the functions,
/// searches for the optimizable expressions and replaces then with more
/// efficient equivalents.
class X86GlobalFMA : public MachineFunctionPass {
public:
  X86GlobalFMA()
      : MachineFunctionPass(ID), MF(nullptr), TII(nullptr), Patterns(nullptr) {}
  ~X86GlobalFMA() { delete Patterns; }

  const char *getPassName() const override { return "X86 GlobalFMA"; }

  bool runOnMachineFunction(MachineFunction &MFunc) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

private:
  /// Pass identification, replacement for typeid.
  static char ID;

  /// A reference to the function being currently compiled.
  MachineFunction *MF;

  /// This field is used to get information about available target operations.
  const TargetInstrInfo *TII;

  /// A storage with pre-computed/efficient FMA patterns.
  FMAPatterns *Patterns;

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
};

char X86GlobalFMA::ID = 0;

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


  /// Returns the reference to machine instruction associated with
  /// FMA expression.
  const MachineInstr *getMI() const { return MI; }

  /// Returns true iff 'this' FMA expression is included into some bigger
  /// FMA expression and does not show up as independent expression.
  bool isConsumed() const { return !IsRootExpr; }

  /// Marks the expression as consumed meaning that it exists only as
  /// a sub-expression of some bigger FMA expression.
  void markAsConsumed() { IsRootExpr = false; }

  // Returns the number of used register and memory terms.
  unsigned getNumUsedTerms() { return UsedTerms.size(); }

  /// Returns the used term by the index \p Index.
  FMATerm *getUsedTermByIndex(unsigned Index) const { return UsedTerms[Index]; }

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

  OS << "\nFMA EXPRESSIONAs:\n";
  unsigned Index = 0;
  for (auto E : FMAs) {
    if (!E->isConsumed()) {
      OS << "  " << Index++ << ": ";
      E->print(OS, true /* PrintType */);
      OS << "\n";
    }
  }
}

/// Loop over all of the basic blocks, performing the FMA optimization for
/// each block separately.
bool X86GlobalFMA::runOnMachineFunction(MachineFunction &MFunc) {
  if (!DoFMAOpt)
    return false;

  MF = &MFunc;
  const X86Subtarget &ST = MF->getSubtarget<X86Subtarget>();
  TII = ST.getInstrInfo();

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
  // Not implemented yet.
  return false;
}

} // End anonymous namespace.

FunctionPass *llvm::createX86GlobalFMAPass() { return new X86GlobalFMA(); }
