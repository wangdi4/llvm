//===-- DDTests.h - Data dependence testing between two DDRefs --*- C++ -*-===//
//
// Copyright (C) 2015-2016 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This file is modified from DependenceAnalysis.h
// Thanks for Preston Briggs,  the contributor of DependenceAnalysis.cpp
//
// DependenceAnalysis is an LLVM pass that analyses dependences between memory
// accesses. Currently, it is an implementation of the approach described in
//
//            Practical Dependence Testing
//            Goff, Kennedy, Tseng
//            PLDI 1991
//
// There's a single entry point that analyzes the dependence between a pair
// of memory references in a function, returning either NULL, for no dependence,
// or a more-or-less detailed description of the dependence between them.
//
// This pass exists to support the DependenceGraph pass. There are two separate
// passes because there's a useful separation of concerns. A dependence exists
// if two conditions are met:
//
//    1) Two instructions reference the same memory location, and
//    2) There is a flow of control leading from one instruction to the other.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_DDTEST_H
#define LLVM_ANALYSIS_DDTEST_H

#include "llvm/ADT/SmallBitVector.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intel_LoopIR/CanonExpr.h"
#include "llvm/IR/Operator.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

#include <array>

namespace llvm {

class AAResults;

namespace loopopt {

class DDRef;
class RegDDRef;
class HLLoop;
class HLNodeUtils;
class HIRLoopStatistics;

/// Dependences - This class represents a dependence between two memory
/// memory references in a function. It contains minimal information and
/// is used in the very common situation where the compiler is unable to
/// determine anything beyond the existence of a dependence; that is, it
/// represents a confused dependence (see also FullDependence). In most
/// cases (for output, flow, and anti dependences), the dependence implies
/// an ordering, where the source must precede the destination; in contrast,
/// input dependences are unordered.
///

enum DVKind : unsigned char {
  NONE = 0,
  LT = 1,
  EQ = 2,
  LE = 3,
  GT = 4,
  NE = 5,
  GE = 6,
  ALL = 7,
};

typedef signed char DistTy;
const DistTy UnknownDistance = SCHAR_MIN;
const DistTy MaxDistance = SCHAR_MAX;
const DistTy MinDistance = UnknownDistance + 1;

inline constexpr DVKind operator|(DVKind Lhs, DVKind Rhs) {
  return static_cast<DVKind>(static_cast<unsigned char>(Lhs) |
                             static_cast<unsigned char>(Rhs));
}

inline constexpr DVKind operator&(DVKind Lhs, DVKind Rhs) {
  return static_cast<DVKind>(static_cast<unsigned char>(Lhs) &
                             static_cast<unsigned char>(Rhs));
}

inline constexpr DVKind operator~(DVKind Arg) {
  return static_cast<DVKind>(~static_cast<unsigned char>(Arg));
}

inline const DVKind &operator&=(DVKind &Lhs, DVKind Rhs) {
  return Lhs = Lhs & Rhs;
}

inline const DVKind &operator|=(DVKind &Lhs, DVKind Rhs) {
  return Lhs = Lhs | Rhs;
}

struct DirectionVector : public std::array<DVKind, MaxLoopNestLevel> {
  // Print DV from level 1 to Level
  void print(raw_ostream &OS, unsigned Level,
             bool ShowLevelDetail = false) const;

  // Print the entire DirectionVector
  void print(raw_ostream &OS, bool ShowLevelDetail = false) const;

  /// Is  DV all ( = = = .. =)?
  bool isEQ() const;

  /// Is DV imply INDEP for level L on
  /// e.g.  DV = (< *)   implies INDEP for innermost loop
  /// In this example, isDVIndepFromLevel(2) return true
  bool isIndepFromLevel(unsigned Level) const;

  /// Returns true if DV shows cross iter dependence at Level.
  bool isCrossIterDepAtLevel(unsigned Level) const {
    return !((*this)[Level - 1] == DVKind::EQ || isIndepFromLevel(Level));
  }

  /// Returns true if DV refinement for Level makes sense.
  /// Be sure to also call isIndepFromLevel() before attempting to refine.
  bool isRefinableAtLevel(unsigned Level) const {
    auto &DV = *this;
    for (unsigned L = 1; L < Level - 1; ++L) {
      // DV::NE would result in indep after refinement. Should be
      // handled by isDVIndepFromLevel() instead.
      if (DV[L - 1] == DVKind::GE || DV[L - 1] == DVKind::LE ||
          DV[L - 1] == DVKind::ALL)
        return true;
    }
    return false;
  }

  // Returns last level in DV .e.g.  (= = =) return 3
  unsigned getLastLevel() const;

  // Fill in input direction vector for demand driven DD
  // startLevel, toLevel
  // e.g. DV.initInput(3,3)
  // will fill in (= = *)
  // which is testing for innermost loop only
  void setAsInput(const unsigned int StartLevel = 1,
                  const unsigned int EndLevel = MaxLoopNestLevel);

  // Construct all 0
  void setZero();
};

struct DistanceVector : public std::array<DistTy, MaxLoopNestLevel> {
  // Print DistVec from level 1 to Level
  void print(raw_ostream &OS, unsigned Level) const;

  // Print the entire DistanceVector
  void print(raw_ostream &OS) const;

  // Set as all 0
  void setZero();
};

class Dependences {
public:
  Dependences(DDRef *Source, DDRef *Destination)
      : Src(Source), Dst(Destination) {}
  virtual ~Dependences() {}

  /// Dependences::DVEntry - Each level in the distance/direction vector
  /// has a direction (or perhaps a union of several directions), and
  /// perhaps a distance.
  struct DVEntry {
    DVKind Direction;   // Init to ALL, then refine.
    bool Scalar : 1;    // Init to true.
    bool PeelFirst : 1; // Peeling the first iteration will break dependence.
    bool PeelLast : 1;  // Peeling the last iteration will break the dependence.
    bool Splitable : 1; // Splitting the loop will break dependence.
    const CanonExpr *Distance; // NULL implies no distance available.
    DVEntry()
        : Direction(DVKind::ALL), Scalar(true), PeelFirst(false),
          PeelLast(false), Splitable(false), Distance(nullptr) {}
  };

  /// getSrc - Returns the source instruction for this dependence.
  ///
  DDRef *getSrc() const { return Src; }

  /// getDst - Returns the destination instruction for this dependence.
  ///
  DDRef *getDst() const { return Dst; }

  /// isLoopIndependent - Returns true if this is a loop-independent
  /// dependence.
  virtual bool isLoopIndependent() const { return true; }

  /// isConfused - Returns true if this dependence is confused
  /// (the compiler understands nothing and makes worst-case
  /// assumptions).
  // virtual bool isConfused() const { return true; }

  /// isConsistent - Returns true if this dependence is consistent
  /// (occurs every time the source and destination are executed).
  // virtual bool isConsistent() const { return false; }

  /// getLevels - Returns the number of common loops surrounding the
  /// source and destination of the dependence.

  virtual unsigned getLevels() const { return 0; }

  /// isReversed: DD edge computed in reverse order
  /// see example below for details

  virtual bool isReversed() const { return false; }

  /// getDirection - Returns the direction associated with a particular
  /// level.
  virtual DVKind getDirection(unsigned Level) const { return DVKind::ALL; }

  /// getDistance - Returns the distance (or NULL) associated with a
  /// particular level.
  virtual const CanonExpr *getDistance(unsigned Level) const { return nullptr; }

  /// isPeelFirst - Returns true if peeling the first iteration from
  /// this loop will break this dependence.
  virtual bool isPeelFirst(unsigned Level) const { return false; }

  /// isPeelLast - Returns true if peeling the last iteration from
  /// this loop will break this dependence.
  virtual bool isPeelLast(unsigned Level) const { return false; }

  /// isSplitable - Returns true if splitting this loop will break
  /// the dependence.
  virtual bool isSplitable(unsigned Level) const { return false; }

  /// isScalar - Returns true if a particular level is scalar; that is,
  /// if no subscript in the source or destination mention the induction
  /// variable associated with the loop at this level.
  virtual bool isScalar(unsigned Level) const;

  /// dump - For debugging purposes, dumps a dependence to OS.
  ///
  void dump(raw_ostream &OS) const;

private:
  DDRef *Src, *Dst;
  friend class DDTest;
};

/// FullDependence - This class represents a dependence between two memory
/// references in a function. It contains detailed information about the
/// dependence (direction vectors, etc.) and is used when the compiler is
/// able to accurately analyze the interaction of the references; that is,
/// it is not a confused dependence (see Dependence). In most cases
/// (for output, flow, and anti dependences), the dependence implies an
/// ordering, where the source must precede the destination; in contrast,
/// input dependences are unordered.

/// The class has more information that  put in the DD Edge which contains
/// the DV.  These detail info are avaiable through calls to Depends

class FullDependences : public Dependences {
public:
  FullDependences(DDRef *SrcDDRef, DDRef *DstDDRef, unsigned Levels);
  ~FullDependences();

  /// isLoopIndependent - Returns true if this is a loop-independent
  /// dependence.
  bool isLoopIndependent() const override { return LoopIndependent; }

  /// isConfused - Returns true if this dependence is confused
  /// (the compiler understands nothing and makes worst-case
  /// assumptions).
  // bool isConfused() const override { return false; }

  /// isConsistent - Returns true if this dependence is consistent
  /// (occurs every time the source and destination are executed).
  // bool isConsistent() const override { return Consistent; }

  /// getLevels - Returns the number of common loops surrounding the
  /// source and destination of the dependence.
  unsigned getLevels() const override { return Levels; }

  bool isReversed() const override { return Reversed; }

  /// getDirection - Returns the direction associated with a particular
  /// level.
  DVKind getDirection(unsigned Level) const override;

  /// getDistance - Returns the distance (or NULL) associated with a
  /// particular level.
  const CanonExpr *getDistance(unsigned Level) const override;

  /// isPeelFirst - Returns true if peeling the first iteration from
  /// this loop will break this dependence.
  bool isPeelFirst(unsigned Level) const override;

  /// isPeelLast - Returns true if peeling the last iteration from
  /// this loop will break this dependence.
  bool isPeelLast(unsigned Level) const override;

  /// isSplitable - Returns true if splitting the loop will break
  /// the dependence.
  bool isSplitable(unsigned Level) const override;

  /// isScalar - Returns true if a particular level is scalar; that is,
  /// if no subscript in the source or destination mention the induction
  /// variable associated with the loop at this level.
  bool isScalar(unsigned Level) const override;

private:
  unsigned short Levels; // commonLevels
  bool LoopIndependent;
  bool Consistent; // Init to true, then refine.

  /// isReversed
  /// if true, caller should note that the result is reversed
  ///  from DstDDRef to  SrcDDef.
  ///  e.g.  for a[i] = a[i+1]    (SrcDDref , DstDDref).
  /// Result returned:  anti (>), IsReversed=true,
  //  DD edge is from Dst->Src
  bool Reversed;
  DVEntry *DV;
  void setDirection(const unsigned Level, const DVKind dv) const;
  void setDistance(const unsigned Level, const CanonExpr *CE) const;
  friend class DDTest;
};

/// DDtest - This class is the main dependence-analysis driver.
///

class DDTest {
  friend class HIRDDAnalysis;

  AAResults &AAR;
  HLNodeUtils &HNU;
  HIRLoopStatistics &HLS;

  DDTest(AAResults &AAR, HLNodeUtils &HNU, HIRLoopStatistics &HLS);
  ~DDTest();

  /// \brief Tests for a dependence between the Src and Dst DDRefs
  /// Returns NULL if no dependence; otherwise, returns a Dependence (or a
  /// FullDependence) with as much information as can be gleaned.
  std::unique_ptr<Dependences> depends(DDRef *SrcDDRef, DDRef *DstDDRef,
                                       const DirectionVector &InputDV,
                                       bool fromFusion = false);

  /// findDependences  - return true if there is a dependence, otherwise INDEP
  /// for Srce and Dest DDRefs
  /// fills in ForwardDV and backwardDV
  //  In most cases. only one DV  is neeed.
  //  e.g. with source A[i1][i2],  Dest  A[i1][i2+1]
  //
  //  true returned
  //  forwardDV[0:1]  is (0, 0)
  //  backwardDV[0:1] is (= ,<)

  bool findDependences(DDRef *SrcDDRef, DDRef *DstDDRef,
                       const DirectionVector &InputDV,
                       DirectionVector &ForwardDV, DirectionVector &BackwardDV,
                       DistanceVector &ForwardDistV,
                       DistanceVector &BackwardDistV, bool *IsLoopIndepDepTemp);

  /// getSplitIteration - Give a dependence that's splittable at some
  /// particular level, return the iteration that should be used to split
  /// the loop.
  ///
  /// Generally, the dependence analyzer will be used to build
  /// a dependence graph for a function (basically a map from instructions
  /// to dependences). Looking for cycles in the graph shows us loops
  /// that cannot be trivially vectorized/parallelized.
  ///
  /// We can try to improve the situation by examining all the dependences
  /// that make up the cycle, looking for ones we can break.
  /// Sometimes, peeling the first or last iteration of a loop will break
  /// dependences, and there are flags for those possibilities.
  /// Sometimes, splitting a loop at some other iteration will do the trick,
  /// and we've got a flag for that case. Rather than waste the space to
  /// record the exact iteration (since we rarely know), we provide
  /// a method that calculates the iteration. It's a drag that it must work
  /// from scratch, but wonderful in that it's possible.
  ///
  /// Here's an example:
  ///
  ///    for (i = 0; i < 10; i++)
  ///        A[i] = ...
  ///        ... = A[11 - i]
  ///
  /// There's a loop-carried flow dependence from the store to the load,
  /// found by the weak-crossing SIV test. The dependence will have a flag,
  /// indicating that the dependence can be broken by splitting the loop.
  /// Calling getSplitIteration will return 5.
  /// Splitting the loop breaks the dependence, like so:
  ///
  ///    for (i = 0; i <= 5; i++)
  ///        A[i] = ...
  ///        ... = A[11 - i]
  ///    for (i = 6; i < 10; i++)
  ///        A[i] = ...
  ///        ... = A[11 - i]
  ///
  /// breaks the dependence and allows us to vectorize/parallelize
  /// both loops.

  const CanonExpr *getSplitIteration(const Dependences &Dep, unsigned Level);

  ///
  ///  Split DV for forward/backward edges. ForwardDV may get modified for
  ///  leading <>
  ///

  void splitDVForForwardBackwardEdge(DirectionVector &ForwardDV,
                                     DirectionVector &BackwardDV,
                                     unsigned MaxLevel) const;

  /// \brief Query LLVM Alias Analysis to check if there is no aliasing between
  /// \p SrcDDRef and \p DstDDref (ex. due to TBAA or AliasScopes)
  bool queryAAIndep(RegDDRef *SrcDDRef, RegDDRef *DstDDRef);

  /// Set DV for various cases.
  /// PeelFirst && Reversed
  void setDVForPeelFirstAndReversed(DirectionVector &ForwardDV,
                                    DirectionVector &BackwardDV,
                                    const Dependences &Result, unsigned Levels);

  ///  BiDirectional
  void setDVForBiDirection(DirectionVector &ForwardDV,
                           DirectionVector &BackwardDV,
                           const Dependences &Result, unsigned Levels,
                           unsigned LTGTLevel);
  ///  LoopIndependent (= =)
  void setDVForLoopIndependent(DirectionVector &ForwardDV,
                               DirectionVector &BackwardDV,
                               const Dependences &Result, unsigned Levels,
                               unsigned SrcNum, unsigned DstNum);

  ///  Breakup <= into <, =

  void setDVForLE(DirectionVector &ForwardDV, DirectionVector &BackwardDV,
                  const Dependences &Result, unsigned Levels);

  /// Distance: from CE to INT

  void populateDistanceVector(const DirectionVector &ForwardDV,
                              const DirectionVector &BackwardDV,
                              const Dependences &Result,
                              DistanceVector &ForwardDistV,
                              DistanceVector &BackwardDistV, unsigned Levels);

  /// Map DV to distance

  DistTy mapDVToDist(DVKind DV, unsigned Level, const Dependences &Result);

  /// Subscript - This private struct represents a pair of subscripts from
  /// a pair of potentially multi-dimensional array references. We use a
  /// vector of them to guide subscript partitioning.
  struct Subscript {
    const CanonExpr *Src;
    const CanonExpr *Dst;
    enum ClassificationKind { ZIV, SIV, RDIV, MIV, NonLinear } Classification;
    SmallBitVector Loops;
    SmallBitVector GroupLoops;
    SmallBitVector Group;
  };

  struct CoefficientInfo {
    const CanonExpr *Coeff;
    const CanonExpr *PosPart;
    const CanonExpr *NegPart;
    const CanonExpr *Iterations;
  };

  struct BoundInfo {
    const CanonExpr *Iterations;
    const CanonExpr *Upper[MaxLoopNestLevel];
    const CanonExpr *Lower[MaxLoopNestLevel];
    DVKind Direction;
    DVKind DirSet;
  };

  SmallVector<CanonExpr *, 10> WorkCE;
  void push(CanonExpr *CE) { WorkCE.push_back(CE); }

  /// Constraint - This private class represents a constraint, as defined
  /// in the paper
  ///
  ///           Practical Dependence Testing
  ///           Goff, Kennedy, Tseng
  ///           PLDI 1991
  ///
  /// There are 5 kinds of constraint, in a hierarchy.
  ///   1) Any - indicates no constraint, any dependence is possible.
  ///   2) Line - A line ax + by = c, where a, b, and c are parameters,
  ///             representing the dependence equation.
  ///   3) Distance - The value d of the dependence distance;
  ///   4) Point - A point <x, y> representing the dependence from
  ///              iteration x to iteration y.
  ///   5) Empty - No dependence is possible.
  class Constraint {
  private:
    enum ConstraintKind { Empty, Point, Distance, Line, Any } Kind;
    const CanonExpr *A;
    const CanonExpr *B;
    const CanonExpr *C;
    const HLLoop *AssociatedLoop;

  public:
    /// isEmpty - Return true if the constraint is of kind Empty.
    bool isEmpty() const { return Kind == Empty; }

    /// isPoint - Return true if the constraint is of kind Point.
    bool isPoint() const { return Kind == Point; }

    /// isDistance - Return true if the constraint is of kind Distance.
    bool isDistance() const { return Kind == Distance; }

    /// isLine - Return true if the constraint is of kind Line.
    /// Since Distance's can also be represented as Lines, we also return
    /// true if the constraint is of kind Distance.
    bool isLine() const { return Kind == Line || Kind == Distance; }

    /// isAny - Return true if the constraint is of kind Any;
    bool isAny() const { return Kind == Any; }

    /// getX - If constraint is a point <X, Y>, returns X.
    /// Otherwise assert.
    const CanonExpr *getX() const;

    /// getY - If constraint is a point <X, Y>, returns Y.
    /// Otherwise assert.
    const CanonExpr *getY() const;

    /// getA - If constraint is a line AX + BY = C, returns A.
    /// Otherwise assert.
    const CanonExpr *getA() const;

    /// getB - If constraint is a line AX + BY = C, returns B.
    /// Otherwise assert.
    const CanonExpr *getB() const;

    /// getC - If constraint is a line AX + BY = C, returns C.
    /// Otherwise assert.
    const CanonExpr *getC() const;

    /// getD - If constraint is a distance, returns D.
    /// Otherwise assert.
    const CanonExpr *getD() const;

    /// getAssociatedLoop - Returns the loop associated with this constraint.
    const HLLoop *getAssociatedLoop() const;

    /// setPoint - Change a constraint to Point.
    void setPoint(const CanonExpr *X, const CanonExpr *Y,
                  const HLLoop *CurrentLoop);

    /// setLine - Change a constraint to Line.
    void setLine(const CanonExpr *A, const CanonExpr *B, const CanonExpr *C,
                 const HLLoop *CurrentLoop);

    /// setDistance - Change a constraint to Distance.
    void setDistance(const CanonExpr *D, const HLLoop *CurrentLoop);

    /// setEmpty - Change a constraint to Empty.
    void setEmpty();

    /// setAny - Change a constraint to Any.
    void setAny();

    /// dump - For debugging purposes. Dumps the constraint
    /// out to OS.
    void dump(raw_ostream &OS) const;
  };

  unsigned CommonLevels, SrcLevels, MaxLevels;
  HLLoop *DeepestLoop;

  /// establishNestingLevels - Examines the loop nesting of the Src and Dst
  /// instructions and establishes their shared loops. Sets the variables
  /// CommonLevels, SrcLevels, and MaxLevels.
  /// The source and destination instructions needn't be contained in the same
  /// loop. The routine establishNestingLevels finds the level of most deeply
  /// nested loop that contains them both, CommonLevels. An instruction that's
  /// not contained in a loop is at level = 0. MaxLevels is equal to the level
  /// of the source plus the level of the destination, minus CommonLevels.
  /// This lets us allocate vectors MaxLevels in length, with room for every
  /// distinct loop referenced in both the source and destination subscripts.
  /// The variable SrcLevels is the nesting depth of the source instruction.
  /// It's used to help calculate distinct loops referenced by the destination.
  /// Here's the map from loops to levels:
  ///            0 - unused
  ///            1 - outermost common loop
  ///          ... - other common loops
  /// CommonLevels - innermost common loop
  ///          ... - loops containing Src but not Dst
  ///    SrcLevels - innermost loop containing Src but not Dst
  ///          ... - loops containing Dst but not Src
  ///    MaxLevels - innermost loop containing Dst but not Src
  /// Consider the follow code fragment:
  ///    for (a = ...) {
  ///      for (b = ...) {
  ///        for (c = ...) {
  ///          for (d = ...) {
  ///            A[] = ...;
  ///          }
  ///        }
  ///        for (e = ...) {
  ///          for (f = ...) {
  ///            for (g = ...) {
  ///              ... = A[];
  ///            }
  ///          }
  ///        }
  ///      }
  ///    }
  /// If we're looking at the possibility of a dependence between the store
  /// to A (the Src) and the load from A (the Dst), we'll note that they
  /// have 2 loops in common, so CommonLevels will equal 2 and the direction
  /// vector for Result will have 2 entries. SrcLevels = 4 and MaxLevels = 7.
  /// A map from loop names to level indices would look like
  ///     a - 1
  ///     b - 2 = CommonLevels
  ///     c - 3
  ///     d - 4 = SrcLevels
  ///     e - 5
  ///     f - 6
  ///     g - 7 = MaxLevels
  void establishNestingLevels(const DDRef *Src, const DDRef *Dst);

  /// mapSrcLoop - Given one of the loops containing the source, return
  /// its level index in our numbering scheme.
  unsigned mapSrcLoop(const HLLoop *SrcLoop) const;

  /// mapDstLoop - Given one of the loops containing the destination,
  /// return its level index in our numbering scheme.
  unsigned mapDstLoop(const HLLoop *DstLoop) const;
  unsigned mapDstLoop(unsigned NestingLevel) const;

  /// isLoopInvariant - Returns true if Expression is loop invariant
  /// in LoopNest.
  bool isLoopInvariant(const CanonExpr *Expression,
                       const HLLoop *LoopNest) const;

  /// Makes sure both subscripts (i.e. Pair->Src and Pair->Dst) share the same
  /// integer type by sign-extending one of them when necessary.
  /// Sign-extending a subscript is safe because getelementptr assumes the
  /// array subscripts are signed.
  void unifySubscriptType(Subscript *Pair);

  /// removeMatchingExtensions - Examines a subscript pair.
  /// If the source and destination are identically sign (or zero)
  /// extended, it strips off the extension in an effort to
  /// simplify the actual analysis.
  void removeMatchingExtensions(Subscript *Pair);

  /// collectCommonLoops - Finds the set of loops from the LoopNest that
  /// have a level <= CommonLevels and are referred to by the CanonExpr
  /// Expression.
  void collectCommonLoops(const CanonExpr *Expression, const HLLoop *LoopNest,
                          SmallBitVector &Loops) const;

  /// checkSrcSubscript - Examines the CanonExpr Src, returning true iff it's
  /// linear. Collect the set of loops mentioned by Src.
  bool checkSrcSubscript(const CanonExpr *Src, const HLLoop *LoopNest,
                         SmallBitVector &Loops);

  /// checkDstSubscript - Examines the CanonExpr Dst, returning true iff it's
  /// linear. Collect the set of loops mentioned by Dst.
  bool checkDstSubscript(const CanonExpr *Dst, const HLLoop *LoopNest,
                         SmallBitVector &Loops);

  /// isKnownPredicate - Compare X and Y using the predicate Pred.
  /// Basically a wrapper for CanonExpr::isKnownPredicate,
  /// but tries harder, especially in the presence of sign and zero
  /// extensions and symbolics.
  bool isKnownPredicate(ICmpInst::Predicate Pred, const CanonExpr *X,
                        const CanonExpr *Y);

  /// collectUpperBound - All subscripts are the same type (on my machine,
  /// an i64). The loop bound may be a smaller type. collectUpperBound
  /// find the bound, if available, and zero extends it to the Type T.
  /// (I zero extend since the bound should always be >= 0.)
  /// If no upper bound is available, return NULL.
  const CanonExpr *collectUpperBound(const HLLoop *L, Type *T) const;

  /// collectConstantUpperBound - Calls collectUpperBound(), then
  /// attempts to cast it to CanonExprConstant. If the cast fails,
  /// returns NULL.
  const CanonExpr *collectConstantUpperBound(const HLLoop *L, Type *T) const;

  /// classifyPair - Examines the subscript pair (the Src and Dst CanonExprs)
  /// and classifies it as either ZIV, SIV, RDIV, MIV, or Nonlinear.
  /// Collects the associated loops in a set.
  Subscript::ClassificationKind classifyPair(const CanonExpr *Src,
                                             const HLLoop *SrcLoopNest,
                                             const CanonExpr *Dst,
                                             const HLLoop *DstLoopNest,
                                             SmallBitVector &Loops);

  /// testZIV - Tests the ZIV subscript pair (Src and Dst) for dependence.
  /// Returns true if any possible dependence is disproved.
  /// If there might be a dependence, returns false.
  /// If the dependence isn't proven to exist,
  /// marks the Result as inconsistent.
  bool testZIV(const CanonExpr *Src, const CanonExpr *Dst,
               FullDependences &Result);

  /// testSIV - Tests the SIV subscript pair (Src and Dst) for dependence.
  /// Things of the form [c1 + a1*i] and [c2 + a2*j], where
  /// i and j are induction variables, c1 and c2 are loop invariant,
  /// and a1 and a2 are constant.
  /// Returns true if any possible dependence is disproved.
  /// If there might be a dependence, returns false.
  /// Sets appropriate direction vector entry and, when possible,
  /// the distance vector entry.
  /// If the dependence isn't proven to exist,
  /// marks the Result as inconsistent.
  bool testSIV(const CanonExpr *Src, const CanonExpr *Dst, unsigned &Level,
               FullDependences &Result, Constraint &NewConstraint,
               const CanonExpr *&SplitIter, const HLLoop *SrcParentLoop,
               const HLLoop *DstParentLoop);

  /// testRDIV - Tests the RDIV subscript pair (Src and Dst) for dependence.
  /// Things of the form [c1 + a1*i] and [c2 + a2*j]
  /// where i and j are induction variables, c1 and c2 are loop invariant,
  /// and a1 and a2 are constant.
  /// With minor algebra, this test can also be used for things like
  /// [c1 + a1*i + a2*j][c2].
  /// Returns true if any possible dependence is disproved.
  /// If there might be a dependence, returns false.
  /// Marks the Result as inconsistent.
  bool testRDIV(const CanonExpr *Src, const CanonExpr *Dst,
                FullDependences &Result, const HLLoop *SrcParentLoop,
                const HLLoop *DstParentLoop);

  /// testMIV - Tests the MIV subscript pair (Src and Dst) for dependence.
  /// Returns true if dependence disproved.
  /// Can sometimes refine direction vectors.
  bool testMIV(const CanonExpr *Src, const CanonExpr *Dst,
               const DirectionVector &InputDV, const SmallBitVector &Loops,
               FullDependences &Result, const HLLoop *SrcParentLoop,
               const HLLoop *DstParentLoop);

  /// strongSIVtest - Tests the strong SIV subscript pair (Src and Dst)
  /// for dependence.
  /// Things of the form [c1 + a*i] and [c2 + a*i],
  /// where i is an induction variable, c1 and c2 are loop invariant,
  /// and a is a constant
  /// Returns true if any possible dependence is disproved.
  /// If there might be a dependence, returns false.
  /// Sets appropriate direction and distance.
  bool strongSIVtest(const CanonExpr *Coeff, const CanonExpr *SrcConst,
                     const CanonExpr *DstConst, const HLLoop *CurrentLoop,
                     unsigned Level, FullDependences &Result,
                     Constraint &NewConstraint);

  /// weakCrossingSIVtest - Tests the weak-crossing SIV subscript pair
  /// (Src and Dst) for dependence.
  /// Things of the form [c1 + a*i] and [c2 - a*i],
  /// where i is an induction variable, c1 and c2 are loop invariant,
  /// and a is a constant.
  /// Returns true if any possible dependence is disproved.
  /// If there might be a dependence, returns false.
  /// Sets appropriate direction entry.
  /// Set consistent to false.
  /// Marks the dependence as splitable.
  bool weakCrossingSIVtest(const CanonExpr *SrcCoeff, const CanonExpr *SrcConst,
                           const CanonExpr *DstConst, const HLLoop *CurrentLoop,
                           unsigned Level, FullDependences &Result,
                           Constraint &NewConstraint,
                           const CanonExpr *&SplitIter);

  /// ExactSIVtest - Tests the SIV subscript pair
  /// (Src and Dst) for dependence.
  /// Things of the form [c1 + a1*i] and [c2 + a2*i],
  /// where i is an induction variable, c1 and c2 are loop invariant,
  /// and a1 and a2 are constant.
  /// Returns true if any possible dependence is disproved.
  /// If there might be a dependence, returns false.
  /// Sets appropriate direction entry.
  /// Set consistent to false.
  bool exactSIVtest(const CanonExpr *SrcCoeff, const CanonExpr *DstCoeff,
                    const CanonExpr *SrcConst, const CanonExpr *DstConst,
                    const HLLoop *CurrentLoop, unsigned Level,
                    FullDependences &Result, Constraint &NewConstraint);

  /// weakZeroSrcSIVtest - Tests the weak-zero SIV subscript pair
  /// (Src and Dst) for dependence.
  /// Things of the form [c1] and [c2 + a*i],
  /// where i is an induction variable, c1 and c2 are loop invariant,
  /// and a is a constant. See also weakZeroDstSIVtest.
  /// Returns true if any possible dependence is disproved.
  /// If there might be a dependence, returns false.
  /// Sets appropriate direction entry.
  /// Set consistent to false.
  /// If loop peeling will break the dependence, mark appropriately.
  bool weakZeroSrcSIVtest(const CanonExpr *DstCoeff, const CanonExpr *SrcConst,
                          const CanonExpr *DstConst, const HLLoop *CurrentLoop,
                          unsigned Level, FullDependences &Result,
                          Constraint &NewConstraint);

  /// weakZeroDstSIVtest - Tests the weak-zero SIV subscript pair
  /// (Src and Dst) for dependence.
  /// Things of the form [c1 + a*i] and [c2],
  /// where i is an induction variable, c1 and c2 are loop invariant,
  /// and a is a constant. See also weakZeroSrcSIVtest.
  /// Returns true if any possible dependence is disproved.
  /// If there might be a dependence, returns false.
  /// Sets appropriate direction entry.
  /// Set consistent to false.
  /// If loop peeling will break the dependence, mark appropriately.
  bool weakZeroDstSIVtest(const CanonExpr *SrcCoeff, const CanonExpr *SrcConst,
                          const CanonExpr *DstConst, const HLLoop *CurrentLoop,
                          unsigned Level, FullDependences &Result,
                          Constraint &NewConstraint);

  /// exactRDIVtest - Tests the RDIV subscript pair for dependence.
  /// Things of the form [c1 + a*i] and [c2 + b*j],
  /// where i and j are induction variable, c1 and c2 are loop invariant,
  /// and a and b are constants.
  /// Returns true if any possible dependence is disproved.
  /// Marks the result as inconsistent.
  /// Works in some cases that symbolicRDIVtest doesn't,
  /// and vice versa.
  bool exactRDIVtest(const CanonExpr *SrcCoeff, const CanonExpr *DstCoeff,
                     const CanonExpr *SrcConst, const CanonExpr *DstConst,
                     const HLLoop *SrcLoop, const HLLoop *DstLoop,
                     FullDependences &Result);

  /// symbolicRDIVtest - Tests the RDIV subscript pair for dependence.
  /// Things of the form [c1 + a*i] and [c2 + b*j],
  /// where i and j are induction variable, c1 and c2 are loop invariant,
  /// and a and b are constants.
  /// Returns true if any possible dependence is disproved.
  /// Marks the result as inconsistent.
  /// Works in some cases that exactRDIVtest doesn't,
  /// and vice versa. Can also be used as a backup for
  /// ordinary SIV tests.
  bool symbolicRDIVtest(const CanonExpr *SrcCoeff, const CanonExpr *DstCoeff,
                        const CanonExpr *SrcConst, const CanonExpr *DstConst,
                        const HLLoop *SrcLoop, const HLLoop *DstLoop);

  /// gcdMIVtest - Tests an MIV subscript pair for dependence.
  /// Returns true if any possible dependence is disproved.
  /// Marks the result as inconsistent.
  /// Can sometimes disprove the equal direction for 1 or more loops.
  //  Can handle some symbolics that even the SIV tests don't get,
  /// so we use it as a backup for everything.
  bool gcdMIVtest(const CanonExpr *Src, const CanonExpr *Dst,
                  const HLLoop *srcParentLoop, const HLLoop *dstParentLoop,
                  FullDependences &Result);

  /// banerjeeMIVtest - Tests an MIV subscript pair for dependence.
  /// Returns true if any possible dependence is disproved.
  /// Marks the result as inconsistent.
  /// Computes directions.
  bool banerjeeMIVtest(const CanonExpr *Src, const CanonExpr *Dst,
                       const DirectionVector &InputDV,
                       const SmallBitVector &Loops, FullDependences &Result,
                       const HLLoop *SrcLoop, const HLLoop *DstLoop);

  /// collectCoefficientInfo - Walks through the subscript,
  /// collecting each coefficient, the associated loop bounds,
  /// and recording its positive and negative parts for later use.
  CoefficientInfo *collectCoeffInfo(const CanonExpr *Subscript, bool SrcFlag,
                                    const CanonExpr *&Constant,
                                    const HLLoop *SrcLoop,
                                    const HLLoop *DstLoop);

  ///  New CE are always constructed for member functions here for  arith.
  ///  operations
  ///  They are recorded in a small vector and freed for each call of DDtest

  /// getPositivePart - X^+ = max(X, 0).
  ///
  const CanonExpr *getPositivePart(const CanonExpr *X);

  /// getNegativePart - X^- = min(X, 0).
  ///
  const CanonExpr *getNegativePart(const CanonExpr *X);

  /// getLowerBound - Looks through all the bounds info and
  /// computes the lower bound given the current direction settings
  /// at each level.
  const CanonExpr *getLowerBound(BoundInfo *Bound);

  /// getUpperBound - Looks through all the bounds info and
  /// computes the upper bound given the current direction settings
  /// at each level.
  const CanonExpr *getUpperBound(BoundInfo *Bound);

  //
  /// return CE for srcCE  - dstCE
  const CanonExpr *getMinus(const CanonExpr *SrcConst,
                            const CanonExpr *DstConst);

  /// return CE for srcCE  + dstCE
  const CanonExpr *getAdd(const CanonExpr *SrcConst, const CanonExpr *DstConst);

  /// return true if 2 CE are equal
  bool areCEEqual(const CanonExpr *CE1, const CanonExpr *CE2,
                  bool RelaxedMode = true) const;

  /// return negation of CE
  const CanonExpr *getNegative(const CanonExpr *CE);

  //  Next interface is for distance.
  //  distance == nullptr are allowed whuch means undefined
  const CanonExpr *getNegativeDist(const CanonExpr *CE);

  ///  CE1 * CE2
  const CanonExpr *getMulExpr(const CanonExpr *CE1, const CanonExpr *CE2);

  /// return CE from apint
  const CanonExpr *getConstantfromAPInt(Type *Ty, APInt Value);

  /// return CE from int with type
  const CanonExpr *getConstantWithType(Type *SrcTy, Type *DestTy, bool IsSExt,
                                       int64_t Val);

  /// CE1 / CE2
  const CanonExpr *getUDivExpr(const CanonExpr *CE1, const CanonExpr *CE2);
  ///   max(CE2, CE2)
  const CanonExpr *getSMaxExpr(const CanonExpr *CE1, const CanonExpr *CE2);
  ///   min(CE1, CE2)
  const CanonExpr *getSMinExpr(const CanonExpr *CE1, const CanonExpr *CE2);

  ///  Blob + constant
  const CanonExpr *getInvariant(const CanonExpr *CE);

  ///  Coeff w.r.t. to ordering (not level)
  const CanonExpr *getCoeff(const CanonExpr *CE, unsigned int IVnum = 1,
                            bool checkSingleIV = true);
  ///  get first coeff
  const CanonExpr *getFirstCoeff(const CanonExpr *CE);

  /// get 2nd coeff
  const CanonExpr *getSecondCoeff(const CanonExpr *CE);

  /// exploreDirections - Hierarchically expands the direction vector
  /// search space, combining the directions of discovered dependences
  /// in the DirSet field of Bound. Returns the number of distinct
  /// dependences discovered. If the dependence is disproved,
  /// it will return 0.
  unsigned exploreDirections(unsigned Level, CoefficientInfo *A,
                             CoefficientInfo *B, BoundInfo *Bound,
                             const SmallBitVector &Loops,
                             unsigned &DepthExpanded, const CanonExpr *Delta,
                             const DirectionVector &InputDV);

  /// testBounds - Returns true iff the current bounds are plausible.
  ///
  bool testBounds(DVKind DirKind, unsigned Level, BoundInfo *Bound,
                  const CanonExpr *Delta, const DirectionVector &InputDV);

  /// findBoundsALL - Computes the upper and lower bounds for level K
  /// using the * direction. Records them in Bound.
  void findBoundsALL(CoefficientInfo *A, CoefficientInfo *B, BoundInfo *Bound,
                     unsigned K);

  /// findBoundsLT - Computes the upper and lower bounds for level K
  /// using the < direction. Records them in Bound.
  void findBoundsLT(CoefficientInfo *A, CoefficientInfo *B, BoundInfo *Bound,
                    unsigned K);

  /// findBoundsGT - Computes the upper and lower bounds for level K
  /// using the > direction. Records them in Bound.
  void findBoundsGT(CoefficientInfo *A, CoefficientInfo *B, BoundInfo *Bound,
                    unsigned K);

  /// findBoundsEQ - Computes the upper and lower bounds for level K
  /// using the = direction. Records them in Bound.
  void findBoundsEQ(CoefficientInfo *A, CoefficientInfo *B, BoundInfo *Bound,
                    unsigned K);

  /// intersectConstraints - Updates X with the intersection
  /// of the Constraints X and Y. Returns true if X has changed.
  bool intersectConstraints(Constraint *X, const Constraint *Y);

  /// propagate - Review the constraints, looking for opportunities
  /// to simplify a subscript pair (Src and Dst).
  /// Return true if some simplification occurs.
  /// If the simplification isn't exact (that is, if it is conservative
  /// in terms of dependence), set consistent to false.
  bool propagate(const CanonExpr *&Src, const CanonExpr *&Dst,
                 SmallBitVector &Loops,
                 SmallVectorImpl<Constraint> &Constraints, bool &Consistent);

  /// propagateDistance - Attempt to propagate a distance
  /// constraint into a subscript pair (Src and Dst).
  /// Return true if some simplification occurs.
  /// If the simplification isn't exact (that is, if it is conservative
  /// in terms of dependence), set consistent to false.
  bool propagateDistance(const CanonExpr *&Src, const CanonExpr *&Dst,
                         Constraint &CurConstraint, bool &Consistent);

  /// propagatePoint - Attempt to propagate a point
  /// constraint into a subscript pair (Src and Dst).
  /// Return true if some simplification occurs.
  bool propagatePoint(const CanonExpr *&Src, const CanonExpr *&Dst,
                      Constraint &CurConstraint);

  /// propagateLine - Attempt to propagate a line
  /// constraint into a subscript pair (Src and Dst).
  /// Return true if some simplification occurs.
  /// If the simplification isn't exact (that is, if it is conservative
  /// in terms of dependence), set consistent to false.
  bool propagateLine(const CanonExpr *&Src, const CanonExpr *&Dst,
                     Constraint &CurConstraint, bool &Consistent);

  /// findCoefficient - Given a linear CanonExpr,
  /// return the coefficient corresponding to specified loop.
  /// If there isn't one, return the CanonExpr constant 0.
  /// For example, given a*i + b*j + c*k, returning the coefficient
  /// corresponding to the j loop would yield b.
  const CanonExpr *findCoefficient(const CanonExpr *Expr,
                                   const HLLoop *TargetLoop);

  /// zeroCoefficient - Given a linear CanonExpr,
  /// return the CanonExpr given by zeroing out the coefficient
  /// corresponding to the specified loop.
  /// For example, given a*i + b*j + c*k, zeroing the coefficient
  /// corresponding to the j loop would yield a*i + c*k.
  const CanonExpr *zeroCoefficient(const CanonExpr *Expr,
                                   const HLLoop *TargetLoop);

  /// addToCoefficient - Given a linear CanonExpr Expr,
  /// return the CanonExpr given by adding some Value to the
  /// coefficient corresponding to the specified TargetLoop.
  /// For example, given a*i + b*j + c*k, adding 1 to the coefficient
  /// corresponding to the j loop would yield a*i + (b+1)*j + c*k.
  const CanonExpr *addToCoefficient(const CanonExpr *Expr,
                                    const HLLoop *TargetLoop,
                                    const CanonExpr *Value);

  /// updateDirection - Update direction vector entry
  /// based on the current constraint.
  void updateDirection(Dependences::DVEntry &Level,
                       const Constraint &CurConstraint) const;

  bool tryDelinearize(const CanonExpr *SrcCanonExpr,
                      const CanonExpr *DstCanonExpr,
                      SmallVectorImpl<Subscript> &Pair,
                      const CanonExpr *ElementSize);

public:
}; // class DDtest

/// createDDtestPass - This creates an instance of the
/// DDtest pass.
FunctionPass *createDDtest();
}

} // namespace llvm

#endif
