//===---------- HIRParser.h - Parses SCEVs into CanonExprs --*- C++ -*-----===//
//
// Copyright (C) 2015 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===----------------------------------------------------------------------===//
//
// This analysis is used to create DDRefs and parse SCEVs into CanonExprs
// for HLNodes.
// It is also responsible for assigning symbases to non livein/liveout scalars
// and populating non-phi livein scalars. Some livein scalars can only be
// discovered during parsing because they appear in SCEVs but not in the IR for
// the region. Loop upper is an example.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRPARSER_H
#define LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRPARSER_H

#include <map>

#include "llvm/Pass.h"

#include "llvm/ADT/DenseSet.h"

#include "llvm/IR/Intel_LoopIR/CanonExpr.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRCreation.h"

namespace llvm {

class Type;
class Function;
class PHINode;
class SCEV;
class ScalarEvolution;
class SCEVConstant;
class SCEVUnknown;
class GEPOperator;
class Value;
class LoopInfo;

namespace loopopt {

class ScalarSymbaseAssignment;
class LoopFormation;
class HLNode;
class HLRegion;
class HLLoop;
class HLIf;
class HLInst;
class HLLabel;
class HLGoto;
class HLSwitch;
class RegDDRef;
class DDRef;
class CanonExpr;

/// \brief This analysis creates DDRefs and parses SCEVs into CanonExprs for
/// HLNodes inside HIR regions. It eliminates HLNodes useless to HIR.
///
/// It is also responsible for assigning symbases to non livein/liveout scalars.
class HIRParser : public FunctionPass {
private:
  // CanonExprUtils/DDRefUtils provides wrapper functions on top of private
  // functions of HIRParser like createBlob() etc.
  friend class CanonExprUtils;
  friend class DDRefUtils;

  // Needs to access getMaxScalarSymbase().
  friend class SymbaseAssignment;

  typedef std::pair<HLInst *, unsigned> InstLevelPair;
  typedef std::map<unsigned, SmallVector<InstLevelPair, 4>> SymbaseToInstMap;

  /// LI - The loop information for the function we are currently analyzing.
  LoopInfo *LI;

  /// SE - Scalar Evolution analysis for the function.
  ScalarEvolution *SE;

  /// HIR - HIR for the function.
  HIRCreation *HIR;

  /// LF - Loop formation analysis of HIR.
  LoopFormation *LF;

  /// ScalarSA - Scalar Symbase Assignment Analysis.
  ScalarSymbaseAssignment *ScalarSA;

  /// Func - The function we are operating on.
  Function *Func;

  /// CurRegion - The region we are operating on.
  HLRegion *CurRegion;

  /// CurLevel - The loop level we are operating on.
  unsigned CurLevel;

  /// UnclassifiedSymbaseInsts - Contains non-essential symbases (and associated
  /// instructions) as determined by the first phase of parsing.
  SymbaseToInstMap UnclassifiedSymbaseInsts;

  /// RequiredSymbases - Set of symbases determined as required by parsing.
  DenseSet<unsigned> RequiredSymbases;

  /// TODO:Commenting out as the usefulness of blob definition is not clear yet.
  ///
  /// TempBlobSymbases - Set of symbases which represent temp blobs.
  /// This can be used to query whether an HLInst is a blob definition and needs
  /// to be kept updated for new instructions created by HIR transformations.
  // SmallSet<unsigned, 64> TempBlobSymbases;

  /// CurBlobLevelMap - Maps temp blob indices to nesting levels for the current
  /// DDRef. Level of -1 denotes non-linear blobs.
  SmallDenseMap<unsigned, int, 8> CurTempBlobLevelMap;

  /// PolynomialFinder - Used to find non-affine(polynomial) sub-SCEVs in an
  /// SCEV.
  class PolynomialFinder;

  /// BaseSCEVCreater - Creates a base version of SCEV by replacing values by
  /// base values. Base value is representative of a symbase. This is to create
  /// a 1:1 mapping between blob index and symbase eliminating comparison issues
  /// between different blob indices associated with the same symbase.
  class BaseSCEVCreater;

  /// BlobLevelSetter - Used to set blob levels in the canon expr.
  class BlobLevelSetter;

  /// BlobPrinter - Used to print blobs.
  class BlobPrinter;

  /// \brief Visits HIR and calls HIRParser's parse*() utilities. Parsing for
  /// non-essential HLInsts is postponed for phase2. Refer to isEssential().
  struct Phase1Visitor;

  /// Phase1 parser functions
  /// parse(HLInst *, ...) is the only function used during phase2.
  void parse(HLRegion *Reg) { CurRegion = Reg; }
  void postParse(HLRegion *Reg) {}

  void parse(HLLoop *HLoop);
  void postParse(HLLoop *HLoop) { CurLevel--; }

  void parse(HLIf *If);
  void postParse(HLIf *If) {}

  void parse(HLSwitch *Switch) { assert(false && "Switch not handled yet!"); }
  void postParse(HLSwitch *Switch) {}

  void parse(HLInst *HInst, bool IsPhase1, unsigned Phase2Level);
  void parse(HLLabel *Label) {}
  void parse(HLGoto *Goto) {}


  /// \brief Implements phase1 of parsing.
  void phase1Parse(HLNode *Node);

  /// \brief Implements phase2 of parsing.
  void phase2Parse();

  /// \brief Returns the number of rval operands of HInst.
  static unsigned getNumRvalOperands(HLInst *HInst);

  /// \brief Returns true if this instruction has a polynomial representation
  /// and should be parsed as a blob (1 * t).
  bool isPolyBlobDef(const Instruction *Inst) const;

  /// \brief Returns true if this instruction has a user outside the region.
  bool isRegionLiveOut(const Instruction *Inst) const;

  /// \brief Returns true if this instruction is essential (e.g. load/store) and
  /// cannot be eliminated.
  bool isEssential(const Instruction *Inst) const;

  /// \brief Returns the integer constant contained in ConstSCEV.
  int64_t getSCEVConstantValue(const SCEVConstant *ConstSCEV) const;

  /// \brief Parses a SCEVConstant expr into CE's constant or denominator field
  /// based on IsDenom flag.
  void parseConstOrDenom(const SCEVConstant *ConstSCEV, CanonExpr *CE,
                         bool IsDenom);

  /// \brief Parses a SCEVConstant expr into CE's constant field.
  void parseConstant(const SCEVConstant *ConstSCEV, CanonExpr *CE);

  /// \brief Parses a SCEVConstant expr into CE's denominator field.
  void parseDenominator(const SCEVConstant *ConstSCEV, CanonExpr *CE);

  /// \brief Sets the DefinedAtLevel for the Canon Expr.
  void setCanonExprDefLevel(CanonExpr *CE, unsigned NestingLevel,
                            unsigned DefLevel) const;

  /// \brief Wrapper to find/insert Blob in the blob table and assign it a
  /// symbase, if applicable. Returns blob index and Symbase.
  /// This is only used during parsing.
  unsigned findOrInsertBlobWrapper(CanonExpr::BlobTy Blob,
                                   unsigned *SymbasePtr = nullptr);

  /// \brief Adds an entry for the temp blob in blob maps.
  void addTempBlobEntry(unsigned Index, unsigned NestingLevel,
                        unsigned DefLevel);

  /// \brief Overrides CE's DefinedAtLevel if the temp blob has a deeper level.
  void setTempBlobLevel(const SCEVUnknown *TempBlobSCEV, CanonExpr *CE,
                        unsigned NestingLevel);

  /// \brief Breaks multiplication blobs such as (2 * n) into multiplier 2 and
  /// new blob n, otherwise sets the multiplier to 1. Also returns the index for
  /// the new or the orignal blob, as applicable.
  void breakConstantMultiplierBlob(CanonExpr::BlobTy Blob, int64_t *Multiplier,
                                   unsigned *Index);

  /// \brief Parses a blob into CE. If IVLevel is non-zero, blob is parsed as an
  /// IV coeff.
  void parseBlob(CanonExpr::BlobTy Blob, CanonExpr *CE, unsigned Level,
                 unsigned IVLevel = 0);

  /// \brief Parses SCEV into CanonExpr.
  void parseRecursive(const SCEV *SC, CanonExpr *CE, unsigned Level,
                      bool IsTop);

  /// \brief Forces incoming value to be parsed as a blob.
  void parseAsBlob(const Value *Val, CanonExpr *CE, unsigned Level);

  /// \brief Forms and returns a CanonExpr representing Val.
  CanonExpr *parse(const Value *Val, unsigned Level);

  /// \brief Parses the i1 condition associated with conditional branches and
  /// select instructions.
  void parseCompare(const Value *Cond, unsigned Level, CmpInst::Predicate *Pred,
                    RegDDRef **LHSDDRef, RegDDRef **RHSDDRef);

  /// \brief Clears blob level map populated for the previous DDRef.
  void clearTempBlobLevelMap();

  /// \brief populates blob DDRefs for Ref based on CurTempBlobLevelMap.
  void populateBlobDDRefs(RegDDRef *Ref);

  /// \brief Returns a RegDDRef representing loop lower (constant 0).
  RegDDRef *createLowerDDRef(const SCEV *BETC);

  /// \brief Returns a RegDDRef representing loop stride (constant 1).
  RegDDRef *createStrideDDRef(const SCEV *BETC);

  /// \brief Returns a RegDDRef representing loop upper.
  RegDDRef *createUpperDDRef(const SCEV *BETC, unsigned Level);

  /// \brief collects strides for an ArrayType in the Strides vector.
  void collectStrides(Type *GEPType, SmallVectorImpl<uint64_t> &Strides);

  /// \brief Looks for GEPOperator associated with this pointer Phi in the phi
  /// operands.
  const GEPOperator *findGEPOperator(const PHINode *PtrPhi) const;

  /// \brief Returns the size of the contained type in bits. Incoming type is
  /// expected to be a pointer type.
  unsigned getBitElementSize(Type *Ty) const;

  /// \brief Creates a GEP RegDDRef for a GEP whose base pointer ia a phi node.
  RegDDRef *createPhiBaseGEPDDRef(const PHINode *BasePhi,
                                  const GEPOperator *GEPOp, unsigned Level);

  /// \brief Creates a GEP RegDDRef representing a single element. For example-
  /// %t[0].
  RegDDRef *createSingleElementGEPDDRef(const Value *GEPVal, unsigned Level);

  /// \brief Creates a GEP RegDDRef for this GEPOperator.
  RegDDRef *createRegularGEPDDRef(const GEPOperator *GEPOp, unsigned Level);

  /// \brief Returns a RegDDRef containing GEPInfo.
  RegDDRef *createGEPDDRef(const Value *Val, unsigned Level);

  /// \brief Returns a RegDDRef representing this scalar value.
  RegDDRef *createScalarDDRef(const Value *Val, unsigned Level, bool isLval);

  /// \brief Returns an rval DDRef created from Val.
  RegDDRef *createRvalDDRef(const Instruction *Inst, unsigned OpNum,
                            unsigned Level);

  /// \brief Returns an lval DDRef created from Inst.
  RegDDRef *createLvalDDRef(const Instruction *Inst, unsigned Level);

  /// \brief Helper to insert newly created blobs.
  void insertBlobHelper(CanonExpr::BlobTy Blob, unsigned Symbase, bool Insert,
                        unsigned *NewBlobIndex);

  // External interface follows. The following functions are called by the
  // framework utilities or other passes.

  /// \brief Returns a new blob created from passed in Val.
  CanonExpr::BlobTy createBlob(Value *Val, unsigned Symbase, bool Insert,
                               unsigned *NewBlobIndex);

  /// \brief Returns a new blob created from a constant value.
  CanonExpr::BlobTy createBlob(int64_t Val, bool Insert,
                               unsigned *NewBlobIndex);

  /// \brief Returns a blob which represents (LHS + RHS). If Insert is true its
  /// index is returned via NewBlobIndex argument.
  CanonExpr::BlobTy createAddBlob(CanonExpr::BlobTy LHS, CanonExpr::BlobTy RHS,
                                  bool Insert, unsigned *NewBlobIndex);

  /// \brief Returns a blob which represents (LHS - RHS). If Insert is true its
  /// index is returned via NewBlobIndex argument.
  CanonExpr::BlobTy createMinusBlob(CanonExpr::BlobTy LHS,
                                    CanonExpr::BlobTy RHS, bool Insert,
                                    unsigned *NewBlobIndex);
  /// \brief Returns a blob which represents (LHS * RHS). If Insert is true its
  /// index is returned via NewBlobIndex argument.
  CanonExpr::BlobTy createMulBlob(CanonExpr::BlobTy LHS, CanonExpr::BlobTy RHS,
                                  bool Insert, unsigned *NewBlobIndex);
  /// \brief Returns a blob which represents (LHS / RHS). If Insert is true its
  /// index is returned via NewBlobIndex argument.
  CanonExpr::BlobTy createUDivBlob(CanonExpr::BlobTy LHS, CanonExpr::BlobTy RHS,
                                   bool Insert, unsigned *NewBlobIndex);
  /// \brief Returns a blob which represents (trunc Blob to Ty). If Insert is
  /// true its index is returned via NewBlobIndex argument.
  CanonExpr::BlobTy createTruncateBlob(CanonExpr::BlobTy Blob, Type *Ty,
                                       bool Insert, unsigned *NewBlobIndex);
  /// \brief Returns a blob which represents (zext Blob to Ty). If Insert is
  /// true its index is returned via NewBlobIndex argument.
  CanonExpr::BlobTy createZeroExtendBlob(CanonExpr::BlobTy Blob, Type *Ty,
                                         bool Insert, unsigned *NewBlobIndex);
  /// \brief Returns a blob which represents (sext Blob to Ty). If Insert is
  /// true its index is returned via NewBlobIndex argument.
  CanonExpr::BlobTy createSignExtendBlob(CanonExpr::BlobTy Blob, Type *Ty,
                                         bool Insert, unsigned *NewBlobIndex);

  // TODO handle min/max blobs.

  /// \brief Returns the max symbase assigned to any temp.
  unsigned getMaxScalarSymbase() const;

  /// \brief Registers new lval/symbase pairs created by HIR transformations.
  /// Only used for printing.
  void insertHIRLval(const Value *Lval, unsigned Symbase);

  /// \brief Prints scalar corresponding to Symbase.
  void printScalar(raw_ostream &OS, unsigned Symbase) const;

  /// \brief Prints blob.
  void printBlob(raw_ostream &OS, CanonExpr::BlobTy Blob) const;

  /// \brief Checks if the blob is constant or not
  /// If blob is constant, sets the return value in Val
  bool isConstantIntBlob(CanonExpr::BlobTy Blob, int64_t *Val) const;

  /// \brief Returns true if this is a temp blob.
  bool isTempBlob(CanonExpr::BlobTy Blob) const;

public:
  static char ID; // Pass identification
  HIRParser();

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void print(raw_ostream &OS, const Module * = nullptr) const override;
  void verifyAnalysis() const override;

  LLVMContext &getContext() const;
  const DataLayout &getDataLayout() const;

  /// Region iterator methods
  HIRCreation::iterator hir_begin() { return HIR->begin(); }
  HIRCreation::const_iterator hir_begin() const { return HIR->begin(); }
  HIRCreation::iterator hir_end() { return HIR->end(); }
  HIRCreation::const_iterator hir_end() const { return HIR->end(); }

  HIRCreation::reverse_iterator hir_rbegin() { return HIR->rbegin(); }
  HIRCreation::const_reverse_iterator hir_rbegin() const {
    return HIR->rbegin();
  }
  HIRCreation::reverse_iterator hir_rend() { return HIR->rend(); }
  HIRCreation::const_reverse_iterator hir_rend() const { return HIR->rend(); }

  /// \brief Returns symbase representing constants.
  unsigned getSymBaseForConstants() const;
};

} // End namespace loopopt

} // End namespace llvm

#endif
