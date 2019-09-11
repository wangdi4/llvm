//====-- Intel_FMACGCommon.h - Fused Multiply Add optimization -----------====//
//
//      Copyright (c) 2016-2019 Intel Corporation.
//      All rights reserved.
//
//        INTEL CORPORATION PROPRIETARY INFORMATION
//
// This software is supplied under the terms of a license
// agreement or nondisclosure agreement with Intel Corp.
// and may not be copied or disclosed except in accordance
// with the terms of that agreement.
//
//===----------------------------------------------------------------------===//
//
// This file defines the common global FMA optimization part that is shared
// by all targets.
//
//===----------------------------------------------------------------------===//
//
// Authors:
// --------
// Vyacheslav Klochkov (vyacheslav.n.klochkov@intel.com)
//

#ifndef LLVM_CODEGEN_INTEL_FMACGCOMMON_H
#define LLVM_CODEGEN_INTEL_FMACGCOMMON_H

#include "llvm/ADT/SetVector.h"
#include "llvm/ADT/SmallBitVector.h"
#include "llvm/CodeGen/Intel_FMACommon.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/Support/raw_ostream.h"

namespace llvm {

class FMABasicBlock;
class FMAExpr;
class FMATerm;
class FMARegisterTerm;
class FMAMemoryTerm;
class FMAImmediateTerm;
class FMAExprSP;
class FMADag;
class FMAPerfDesc;
class FMAPatterns;

/// This class provides the methods returning output streams
/// for debug messages which can be turned ON/OFF depending
/// on their categories.
class FMADbg {
  static const int Main = 1;
  static const int Matching = 2;
  static const int FWS = 4;
public:
  /// Return output stream if the general purpose debug messages are enabled.
  static raw_ostream &dbgs();

  /// Return output stream if the debug messages from pattern matching
  /// are enabled.
  static raw_ostream &match();

  /// Return output stream if the debug messages from expressions-fusing
  /// module are enabled.
  static raw_ostream &fws();
};

/// This class describes the performance metrics of some expression tree.
/// In particular, it keeps information about the number of various operations
/// and latency of the expression tree.
class FMAPerfDesc final {
private:
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
};

/// Prints the FMA Performance Descriptor \p PerfDesc to the given stream \p OS.
inline raw_ostream &operator<<(raw_ostream &OS, const FMAPerfDesc &PerfDesc) {
  PerfDesc.print(OS);
  return OS;
}

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

/// Prints the FMA node \p Node to the given stream \p OS.
inline raw_ostream &operator<<(raw_ostream &OS, const FMADagCommon &Dag) {
  Dag.print(OS);
  return OS;
}

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
  std::unique_ptr<unsigned[]> getTermsMappingToCompactTerms();
};

/// Prints the FMA SP \p SP to the given stream \p OS.
inline raw_ostream &operator<<(raw_ostream &OS, const FMAExprSP &SP) {
  SP.print(OS);
  return OS;
}

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
protected:
  /// Represents a set of FMA patterns that all have the same SHAPE.
  using FMAPatternsSet = ArrayRef<uint64_t>;

  /// All FMA patterns are stored as a vector of references to groups of Dags
  /// where each of the groups has the same SHAPE.
  /// It is also supposed that the groups of Dags are sorted by the SHAPE.
  SmallVector<FMAPatternsSet, 0> Dags;

  /// The largest available shape that corresponds to some
  /// pattern kept in the patterns storage.
  uint64_t LargestAvailableShape;

  /// Returns the number of shapes (i.e. the number of Dag/pattern sets).
  unsigned getNumShapes() const { return Dags.size(); }

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

  /// Constructor.
  FMAPatterns() : LargestAvailableShape(0) {}

  /// Initialize the patterns storage.
  /// Currently it is assumed that there is only one set of patterns for
  /// the target CPU. It may be changed in future, for example, there may
  /// be 2 separate pattern sets: for AVX and for AVX2. In such cases
  /// the init() method may get some input arguments and become more
  /// complex.
  void init();

public:
  /// Returns an FMA DAG that would be the most efficient equivalent of the
  /// given sum of products \p SP.
  std::unique_ptr<FMADag> getDagForBestSPMatch(const FMAExprSP &SP);

  /// Returns the largest available shape available in the patterns storage.
  uint64_t getLargestShape() const { return LargestAvailableShape; }
};

/// This class represents FMA expressions and terms. It works as a bridge
/// between input IR and internal FMA structures, in particular it helps
/// to connect instances of MachineInstr/MachineOperand classes with machine
/// independent classes FMAExprSP/FMADag, get the most efficient representation
/// of input expression and to generate output IR.
class FMANode {
public:
  enum FMANodeKind {
    NK_Expr,
    NK_RegisterTerm,
    NK_MemoryTerm,
    NK_ImmediateTerm
  };
  FMANodeKind getKind() const { return Kind; }

private:
  FMANodeKind Kind;

protected:
  /// Parent basic block.
  FMABasicBlock *BB;

  /// Machine value type of the FMA expression or term.
  MVT VT;

public:
  /// Constructor. Initializes the only field \p VT available in this class.
  FMANode(FMANodeKind Kind, FMABasicBlock *BB, MVT VT)
      : Kind(Kind), BB(BB), VT(VT) {}
  virtual ~FMANode() = default;

  /// Utility functions for checking if the current FMA node is a special term
  /// that represents zero or one.
  bool isZero() const;
  bool isOne() const;

  /// Returns parent FMA basic block for this node.
  FMABasicBlock *getFMABB() const { return BB; }

  /// Returns machine value type of the FMA node.
  MVT getVT() const { return VT; }

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

  /// The reference to the last machine instruction that is using this FMA term
  /// and is generated by FMA optimization. In fact there still may be other
  /// non-FMA instructions using the register associated with this FMA term.
  MachineInstr *LastUseMI;

public:
  /// Creates FMATerm node for a term associated with a virtual register \p Reg.
  /// The parameter \p VT specifies the type of the created expression.
  FMATerm(FMANodeKind Kind, MVT VT, FMABasicBlock *BB, unsigned Reg)
      : FMANode(Kind, BB, VT), Reg(Reg), LastUseMI(nullptr) {}

  /// Returns the virtual register associated with FMA term.
  unsigned getReg() const { return Reg; }

  /// Binds FMA term with a virtual register \p Reg.
  void setReg(unsigned Reg) { this->Reg = Reg; }

  void setLastUseMI(MachineInstr *MI) { LastUseMI = MI; }

  /// The last instruction using this term must have <kill> attribute
  /// set for the first operand that is using this term.
  virtual void setIsKilledAttribute() {
    if (!LastUseMI)
      return;

    for (MachineOperand &MO : LastUseMI->operands()) {
      if (MO.isReg() && MO.getReg() == Reg) {
        MO.setIsKill(true);
        return;
      }
    }
  }

  // Method for type inquiry through isa, cast and dyn_cast.
  static bool classof(const FMANode *N) {
    return N->getKind() >= NK_RegisterTerm && N->getKind() <= NK_ImmediateTerm;
  }
};

/// This class represents an FMA term associated with a virtual register.
class FMARegisterTerm final : public FMATerm {
private:
  /// The order number of the term in the FMABasicBlock.
  /// This field is used only for having convenient dumps of FMA basic block
  /// and FMA expressions.
  unsigned TermIndexInBB;

  /// Indicates if the last use of the virtual register associated with this
  /// FMA register term was seen in one of recognized ADD/SUB/MUL/FMA
  /// expressions.
  bool IsEverKilled;

  /// This field is set to true iff the virtual register associated with this
  /// term is defined by an instruction recognized by this FMA optimization and
  /// is used by at least one instruction not recognized by the optimization.
  bool DefHasUnknownUsers;

public:
  /// Creates FMARegisterTerm node for a term associated with a virtual
  /// register \p Reg.
  /// The parameter \p VT specifies the type of the created expression.
  /// The parameter \p TermIndexInBB defines the order number of the term in
  /// the FMA basic block being currently optimized.
  FMARegisterTerm(MVT VT, FMABasicBlock *BB, unsigned Reg,
                  unsigned TermIndexInBB)
      : FMATerm(NK_RegisterTerm, VT, BB, Reg), TermIndexInBB(TermIndexInBB),
        IsEverKilled(false), DefHasUnknownUsers(false) {}

  /// Returns true iff this term was seen with IsKill attribute set in one
  /// of recognized ADD/SUB/MUL/FMA expressions.
  bool isEverKilled() const { return IsEverKilled; }

  /// Sets the IsEverKilled property. This method must be called when the
  /// virtual register associated with this FMA register term is seen in
  /// recognized ADD/SUB/MUL/FMA operation.
  void setIsEverKilled() { IsEverKilled = true; }

  /// Marks this register term as one having uses not detected as
  /// FMAExpr. So, if register term t1 defined by some FMAExpr
  /// (t1 = FMAExpr(...)) has been marked, then the expression defining
  /// this term cannot be deleted in this optimization.
  void setDefHasUnknownUsers() { DefHasUnknownUsers = true; }

  /// Returns true iff the register term is marked as one having
  /// unknown (i.e. non-FMAExpr) users.
  bool getDefHasUnknownUsers() const { return DefHasUnknownUsers; }

  /// The last instruction using this register term must have <kill> attribute
  /// set for the first operand that is using this term.
  void setIsKilledAttribute() {
    if (IsEverKilled)
      FMATerm::setIsKilledAttribute();
  }

  /// Prints the FMA term to the given stream \p OS.
  /// The parameter \p PrintAttributes specifies if the caller wants to see
  /// more information and some of FMA node attributes should be printed out.
  void print(raw_ostream &OS, bool PrintAttributes) const override {
    OS << "T" << TermIndexInBB << "%%vreg"
       << Register::virtReg2Index(Reg);
    if (PrintAttributes) {
      OS << " // Type: " << EVT(VT).getEVTString();
      if (IsEverKilled)
        OS << "; IsEverKilled = 1";
      if (DefHasUnknownUsers)
        OS << "; DefHasUknownUsers = 1!";
    }
  }

  static bool classof(const FMANode *N) {
    return N->getKind() == NK_RegisterTerm;
  }
};

/// This class represents an FMA term associated with a load from memory.
class FMAMemoryTerm final : public FMATerm {
private:
  /// A reference to machine instruction having a memory operand usually
  /// represented as several machine operands used for memory base, offset, etc.
  MachineInstr *MI;

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
  FMAMemoryTerm(MVT VT, FMABasicBlock *BB, MachineInstr *MI, unsigned IndexInBB)
      : FMATerm(NK_MemoryTerm, VT, BB, 0), MI(MI), TermIndexInBB(IndexInBB) {}

  /// Returns the reference to machine instruction associated with FMA term.
  MachineInstr *getMI() const { return MI; }

  /// Prints the FMA term to the given stream \p OS.
  /// The parameter \p PrintAttributes specifies if the caller wants to see
  /// more information and some of FMA node attributes should be printed out.
  void print(raw_ostream &OS, bool PrintAttributes) const override {
    OS << "T" << TermIndexInBB << "_mem";
    if (PrintAttributes)
      OS << " // Type: " << EVT(VT).getEVTString() << "\n  MI: " << *MI;
  }

  // Method for type inquiry through isa, cast and dyn_cast.
  static bool classof(const FMANode *N) {
    return N->getKind() == NK_MemoryTerm;
  }
};

/// This class represents a special FMA term associated with a floating point
/// constant, e.g. 0.0 or 1.0.
class FMAImmediateTerm : public FMATerm {
protected:
  ///  representation of the value kept by this special term.
  int64_t Imm;

public:
  /// Creates FMAExpr for a special term 0.0 or 1.0 of the type \p VT.
  /// The parameter \p SpecialValue can be equal to either 0 or 1 values.
  FMAImmediateTerm(MVT VT, FMABasicBlock *BB, int64_t Imm)
      : FMATerm(NK_ImmediateTerm, VT, BB, 0), Imm(Imm) {}

  int64_t getImm() const { return Imm; }

  virtual bool isZero() const = 0;
  virtual bool isOne() const = 0;

  /// Prints the FMA expression or term to the given stream \p OS.
  /// The parameter \p PrintAttributes specifies if the caller wants to see
  /// more information and some of FMA node attributes should be printed out.
  void print(raw_ostream &OS, bool PrintAttributes) const override {
    OS << format_hex(Imm, 2u + VT.getSizeInBits() / 4);
    if (PrintAttributes)
      OS << " // Type: " << EVT(VT).getEVTString();
  }

  // Method for type inquiry through isa, cast and dyn_cast.
  static bool classof(const FMANode *N) {
    return N->getKind() == NK_ImmediateTerm;
  }
};

/// This class represents an FMA expression having 3 operands:
///   (MulSign)(A * B) + (AddSign)C;
/// The operands A, B, C can be of any class derived from FMANode class,
/// i.e. A, B, C can point to instances of FMAExpr, FMATerm, FMAImmediateTerm,
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
class FMAExpr final : public FMANode {
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
  std::array<FMANode *, 3u> Operands;

  /// This field is set to true only if there is at least one FMA expression
  /// using 'this' FMA expression directly and there are no FMA expressions
  /// that would use the term defined by 'this' expression.
  bool IsFullyConsumedByKnownExpressions;

  /// This field is set to true only if it must be re-generated at the
  /// code-generation phase.
  bool IsMustBeOptimized;

  /// A reference to an FMA term defined by 'this' FMA expression.
  FMARegisterTerm *ResultTerm;

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
  unsigned IndexInBB;

  /// Returns an index for the given term \p Term. It is asserted that the
  /// provided term is used as an operand of one of FMA operations included
  /// into the expression tree referenced by 'this' FMA expression.
  ///
  /// Conversion of the term to an unsigned index may be needed to bind
  /// FMA terms represented as FMAExpr and terms represented as unsigned
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
  template <typename R> void addToUsedTerms(R &&Terms) {
    for (auto *T : Terms)
      addToUsedTerms(T);
  }

  /// Removes the given term \p Term from the list of used terms.
  /// This method is going to be used only when one expression gets consumed
  /// by another expression and the term defined by the consumed expression
  /// stops being used by the consuming expression. Thus the removed term
  /// can be only an FMARegisterTerm.
  void removeFromUsedTerms(FMARegisterTerm *Term);

  /// Recursively walks through the expression nodes, builds sums of products
  /// for them and puts the created SPs to the given map \p ExprToSPMap.
  /// Returns the sum of products generated for 'this' FMA expression.
  /// The parameter \p RootFMAExpr is a reference to a root FMA expression,
  /// which holds the container with all used terms, which is needed to
  /// convert terms into unsigned indices/terms used in the result
  /// sum of products.
  FMAExprSP *generateSPRecursively(
      const FMAExpr *RootFMAExpr,
      SmallDenseMap<const FMANode *, std::unique_ptr<FMAExprSP>> &Node2SP)
      const;

  /// Returns true if the current expression has too big shape and thus it
  /// does not have any suitable patterns available in the patterns storage
  /// \p Patterns.
  bool isExprTooLarge(const FMAPatterns &Patterns) const;

public:
  /// Create FMAExpr for ADD/SUB/FMA instruction.
  /// The parameter \p VT specifies the type of the created operation.
  /// The parameter \p MI passes a reference to machine instruction for which
  /// this FMAExpr is created.
  /// The parameter \p ResultTerm gives the reference to an FMARegisterTerm
  /// node created for the result of the FMAExpr operation being created here.
  /// The parameters \p Op1, \p Op2, \p Op3 give the references to FMA nodes
  /// used as operands of the created FMA expression.
  FMAExpr(MVT VT, FMABasicBlock *BB, MachineInstr *MI,
          FMARegisterTerm *ResultTerm, const std::array<FMANode *, 3u> &Ops,
          bool MulSign, bool AddSign, unsigned Index);

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
  FMANode *getOperand(unsigned Index) { return Operands[Index]; }

  /// Returns the const operand of FMA operation with the index \p Index.
  const FMANode *getOperand(unsigned Index) const { return Operands[Index]; }

  /// Returns the term associated with the result of this FMA expression.
  FMARegisterTerm *getResultTerm() const { return ResultTerm; }

  /// Returns the reference to machine instruction associated with
  /// FMA expression.
  MachineInstr *getMI() const { return MI; }

  /// Returns the reference to the DBG_VALUE machine instruction associated
  /// with this FMA expression.
  MachineInstr *getDbgValMI() const { return DbgValMI; }

  /// Associates the DBG_VALUE machine instruction with the current
  /// FMA expression.
  void setDbgValMI(MachineInstr *I) { DbgValMI = I; }

  /// Returns expression index in the parent FMABasicBlock.
  unsigned getIndexInBB() const { return IndexInBB; }

  /// This method puts all immediate parents/users of \p Node to \p ExprSet.
  void insertImmediateUsersOfTo(
          const FMANode *Node, SmallPtrSetImpl<const FMAExpr *> &ExprSet) const;

  /// This method puts 'this' node and all subexpressions to the given
  /// set \p ExprSet. It may be needed when each expression node must be
  /// visited only once.
  void putExprToExprSet(SmallPtrSetImpl<const FMAExpr *> &ExprSet) const;

  /// Returns the latency of the FMA expression tree.
  /// The parameters \p AddSubLatency, \p MulLatency, \p FMALatency specify
  /// the latencies of add/subtract, multiply, FMA operations.
  unsigned getLatency(unsigned AddSubLatency, unsigned MulLatency,
                      unsigned FMALatency) const;

  /// Returns true if 'this' expression uses the given term \p Term.
  bool isUserOf(const FMATerm *Term) const {
    // Strange, but it does not compile unless const qualifier is stripped.
    return UsedTerms.count(const_cast<FMATerm*>(Term));
  }

  /// Returns true iff there are users of this expression not visible to
  /// the FMA optimization.
  bool hasUnknownUsers() const { return ResultTerm->getDefHasUnknownUsers(); }

  /// Replaces the uses of the node \p Old with \p New.
  void replaceAllUsesOfWith(FMANode *Old, FMANode *New);

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

  /// Returns true iff the expression MUST  be re-generated/optimized
  /// during the code-generation phase of this optimization.
  bool isMustBeOptimized() const { return IsMustBeOptimized; }

  /// Marks the expression as an expression that MUST be re-generated/optimized
  /// during the code-generation phase of this optimization.
  void markAsMustBeOptimized() { IsMustBeOptimized = true; }

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

  /// For all used terms sets the field 'LastUseMI' to nullptr, which means
  /// that the previously registered 'LastUseMI' value for them is not valid
  /// because a new expression using those terms was found. It happened that
  /// the new user (MI) is not replaced by the FMA optimization and thus
  /// the <isKill> attribute is not going to be set for such terms, unless
  /// some other FMA expression that is replaced by FMA optimization is met
  /// later.
  void unsetLastUseMIsForRegisterTerms() {
    for (auto *T : UsedTerms)
      if (isa<FMARegisterTerm>(T))
        T->setLastUseMI(nullptr);
  }

  /// Looks for an expression in the given basic block \p FMABB that could be
  /// included into 'this' expression. Returns a reference to such expression
  /// or nullptr if no consumable expressions were found. It also returns
  /// a set of users of the returned expression in \p UsersSet container.
  /// The parameters \p BadFWSCandidates and \p InefficientFWS candidates
  /// specify the sets of bad (or non fusible) and inefficient (fusible, but
  /// not giving any performance benefits) expressions.
  /// The parameter \p CanConsumeIfOneUser specifies if the searched candidates
  /// may have only one user.
  FMAExpr *findFWSCandidate(
      const SmallPtrSetImpl<FMAExpr *> &BadFWSCandidates,
      const SmallPtrSetImpl<FMAExpr *> &InefficientFWSCandidates,
      bool CanConsumeIfOneUser,
      SmallPtrSetImpl<FMAExpr *> &UsersSet);

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
  void compactTerms(FMAExprSP &SP);

  /// Generates and returns a sum of products for 'this' FMA expression.
  std::unique_ptr<FMAExprSP> generateSP() const;

  /// Prints the FMA expression or term to the given stream \p OS.
  /// The parameter \p PrintAttributes specifies if the caller wants to see
  /// more information and some of FMA node attributes should be printed out.
  void print(raw_ostream &OS, bool PrintAttributes) const override;

  /// Method for type inquiry through isa, cast and dyn_cast.
  static bool classof(const FMANode *N) { return N->getKind() == NK_Expr; }
};

/// This class represents one optimizable basic block. It holds all FMAExpr
/// objects created for operations in one MachineBasicBlock.
/// It also keeps references to special terms 0.0 and 1.0 created only once and
/// returned when they are used again.
class FMABasicBlock {
private:
  /// A reference to the machine basic block that is being optimized.
  MachineBasicBlock &MBB;

  /// All FMA expressions available in the basic block.
  SmallVector<std::unique_ptr<FMAExpr>, 16u> FMAs;

  /// This field maps terms to expressions defining those terms.
  /// For example, for any expression T1 = FMA1(...) there should be a pair
  /// <T1, FMA1> in this map.
  SmallDenseMap<FMARegisterTerm *, FMAExpr *> TermToDefFMA;

public:
  /// Creates an FMA basic block for the given MachineBasicBlock \p MBB.
  FMABasicBlock(MachineBasicBlock &MBB) : MBB(MBB) {}
  virtual ~FMABasicBlock() = default;

  /// Returns machine basic block this FMA block was created for.
  MachineBasicBlock &getMBB() { return MBB; }
  const MachineBasicBlock &getMBB() const { return MBB; }

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
  ///   \p Ops - are the operands of created FMA(A, B, C).
  FMAExpr *createFMA(MVT VT, MachineInstr *MI, FMARegisterTerm *ResTerm,
                     const std::array<FMANode *, 3u> &Ops, bool MulSign,
                     bool AddSign);

  /// Returns a reference to FMA expression defining the given \p Term.
  FMAExpr *findDefiningFMA(const FMATerm *Term) const {
    if (auto *Reg = dyn_cast<FMARegisterTerm>(Term))
      return TermToDefFMA.lookup(Reg);
    return nullptr;
  }

  /// Returns the vector containing all FMAs available in this basic block.
  const SmallVectorImpl<std::unique_ptr<FMAExpr>> &getFMAs() const {
    return FMAs;
  }

  /// For the given term \p Term it puts all the users of that term into
  /// \p UsersSet.
  void findKnownUsers(const FMATerm *Term,
                      SmallPtrSetImpl<FMAExpr *> &UsersSet) const;

  /// Marks the FMARegisterTerms defined by some known FMAExpr expressions
  /// with special attributes if such terms have uses now recognized as
  /// FMAExpr expressions.
  void setDefHasUnknownUsersForRegisterTerms(MachineRegisterInfo *MRI) {
    SmallPtrSet<const MachineInstr *, 16u> MIsSet;

    // 1st pass: Add all known machine instructions to the set.
    for (auto &T2E : TermToDefFMA)
      MIsSet.insert(T2E.second->getMI());

    // 2nd pass: Mark the register terms having defining FMA expressions and
    // having users/MIs not recognized as FMAs in this basic block.
    for (auto &T2E : TermToDefFMA) {
      FMARegisterTerm *Term = T2E.first;
      unsigned Reg = Term->getReg();
      for (MachineInstr &I : MRI->reg_instructions(Reg)) {
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

  /// Sets the <isKill> attribute to machine operands associated with the
  /// last uses of terms.
  virtual void setIsKilledAttributeForTerms() = 0;

  /// Prints the basic block to the given stream \p OS.
  virtual void print(raw_ostream &OS) const = 0;
};

/// Prints the FMA basic block \p FMABB to the given stream \p OS.
inline raw_ostream &operator<<(raw_ostream &OS, const FMABasicBlock &FMABB) {
  FMABB.print(OS);
  return OS;
}

/// This class does all the optimization work, it goes through the functions,
/// searches for the optimizable expressions and replaces then with more
/// efficient equivalents.
class GlobalFMA : public MachineFunctionPass {
public:
  GlobalFMA(char &ID) : MachineFunctionPass(ID), Patterns(nullptr) {}

  bool runOnMachineFunction(MachineFunction &MFunc) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.setPreservesCFG();
    MachineFunctionPass::getAnalysisUsage(AU);
  }

protected:
  /// The latency of ADD and SUB operations in the target CPU.
  unsigned AddSubLatency;

  /// The latency of MUL operations in the target CPU.
  unsigned MulLatency;

  /// The latency of FMA operations in the target CPU.
  unsigned FMALatency;

  /// Flags controlling pass behavior.
  struct {
    bool ForceFMAs = false;
    bool TuneForLatency = false;
    bool TuneForThroughput = false;
  } Control;

  /// A storage with pre-computed/efficient FMA patterns.
  std::unique_ptr<FMAPatterns> Patterns;

  /// Do the FMA optimization in one basic block.
  /// Return true iff any changes in the IR were made.
  bool optBasicBlock(MachineBasicBlock &MBB);

  /// Walks through all instructions in the machine basic block, finds
  /// MUL/ADD/FMA operations and creates FMA expressions (FMAExpr) for them.
  /// Returns the number of optimizable expressions found in the block.
  /// The parameter \p MRI is passed to this method to make it possible
  /// to find virtual registers associated with FMARegisterTerms and
  /// having uses that are not recognized as FMAExpr operations.
  virtual std::unique_ptr<FMABasicBlock>
  parseBasicBlock(MachineBasicBlock &MBB) = 0;

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

  /// Checks if fusing of \p T into all of \p GoodUsersSet is safe,
  /// we need to check that FMA expressions from \p GoodUsersSet do not
  /// intersect (as result of FWS) with FMA expressions from \p BadUsersSet.
  ///     T   A       T - is the candidate for fusing it may be replaced
  ///      \ /            with some expression during FWS.
  ///   Z   N   X     U - is immediate user of T, included into "good" expr G
  ///    \ / \ /          and "bad" expr B.
  ///     G   B       Replacing T would cause changes in both G and B,
  ///                 and such cases are considered unsafe.
  bool isSafeToFuse(const FMATerm *T,
                    const SmallPtrSetImpl<const FMAExpr *> &GoodUsersSet,
                    const SmallPtrSetImpl<const FMAExpr *> &BadUsersSet) const;

  /// If fusing the expression \p FWSExpr and users of it is profitable and
  /// gives good opportunities for pattern matching then this routine does
  /// such fusing and returns true. Otherwise, it just returns false.
  /// The parameter \p UsersSet gives the set of users of \p FWSExpr.
  /// The parameter \p BadUsersSet is output and is populated with the users
  /// of \p FWSExpr that cannot or should not be fused with \p FWSExpr.
  bool
  doFWSAndConsumeIfProfitable(FMAExpr &FWSExpr,
                              const SmallPtrSetImpl<FMAExpr *> &UsersSet,
                              SmallPtrSetImpl<FMAExpr *> &BadUsersSet,
                              SmallPtrSetImpl<FMAExpr *> &InefficientUsersSet,
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
  virtual void generateOutputIR(FMAExpr &Expr, const FMADag &Dag) = 0;

  /// Returns true if the DAG \p Dag seems more efficient than the initial
  /// expression \p Expr being optimized now.
  /// The parameter \p TuneForLatency specifies if the latency aspect has
  /// the priority over the throughput.
  /// The parameter \p TuneForThroughput specifies if the throughput aspect has
  /// the priority over the latency.
  /// If \p TuneForLatency and \p TuneForThroughput are both set or both unset,
  /// then both aspects are the same important and the final decision depends
  /// on some heuristics.
  bool isDagBetterThanInitialExpr(const FMADag &Dag, const FMAExpr &Expr) const;

  /// For the given DAG \p Dag this method returns the descriptor describing
  /// various performance metrics of the DAG.
  FMAPerfDesc getDagPerfDesc(const FMADag &Dag) const;

  /// For the given expression \p Expr this method returns the descriptor
  /// describing various performance metrics of the expression.
  FMAPerfDesc getExprPerfDesc(const FMAExpr &Expr) const;
};

} // End llvm namespace

#endif // LLVM_CODEGEN_INTEL_FMACGCOMMON_H
