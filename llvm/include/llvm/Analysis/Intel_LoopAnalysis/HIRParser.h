//===---------- HIRParser.h - Parses SCEVs into CanonExprs --*- C++ -*-----===//
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

#include "llvm/IR/Module.h"

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
class Loop;

namespace loopopt {

class ScalarSymbaseAssignment;
class LoopFormation;
class HLNode;
class HLDDNode;
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

typedef const SCEV *BlobTy;
const unsigned InvalidBlobIndex = 0;

/// \brief This analysis creates DDRefs and parses SCEVs into CanonExprs for
/// HLNodes inside HIR regions. It eliminates HLNodes useless to HIR.
///
/// It is also responsible for assigning symbases to non livein/liveout scalars.
class HIRParser : public FunctionPass {
private:
  // CanonExprUtils provides wrapper functions on top of private functions of
  // HIRParser like createBlob() etc.
  friend class BlobUtils;
  // Needs to access getMaxScalarSymbase().
  friend class SymbaseAssignment;
  // Provides access to hir_begin() to HIR transformations.
  friend class HIRFramework;

  typedef std::pair<HLInst *, unsigned> InstLevelPair;
  typedef std::map<unsigned, SmallVector<InstLevelPair, 4>> SymbaseToInstMap;

  typedef std::pair<BlobTy, unsigned> BlobSymbasePairTy;
  typedef SmallVector<BlobSymbasePairTy, 64> BlobTableTy;

  typedef std::pair<BlobTy, unsigned> BlobPtrIndexPairTy;
  typedef SmallVector<BlobPtrIndexPairTy, 64> BlobToIndexTy;

  /// LI - The loop information for the function we are currently analyzing.
  LoopInfo *LI;

  /// SE - Scalar Evolution analysis for the function.
  ScalarEvolution *SE;

  /// RI - The region identification pass.
  const RegionIdentification *RI;

  /// HIR - HIR for the function.
  HIRCreation *HIR;

  /// LF - Loop formation analysis of HIR.
  LoopFormation *LF;

  /// ScalarSA - Scalar Symbase Assignment Analysis.
  ScalarSymbaseAssignment *ScalarSA;

  /// Func - The function we are operating on.
  Function *Func;

  /// CurNode - The node we are operating on.
  HLDDNode *CurNode;

  /// CurRegion - The region we are operating on.
  HLRegion *CurRegion;

  /// CurOutermostLoop - The outermost loop of the loopnest CurNode belongs to.
  const Loop *CurOutermostLoop;

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
  /// DDRef. 
  SmallDenseMap<unsigned, unsigned, 8> CurTempBlobLevelMap;

  // BlobTable - vector containing blobs and corresponding symbases for the
  // function.
  BlobTableTy BlobTable;

  // BlobToIndexMap - stores a mapping of blobs to corresponding indices for
  // faster lookup.
  BlobToIndexTy BlobToIndexMap;

  // Used as comparators to sort blobs.
  struct BlobPtrCompareLess;
  struct BlobPtrCompareEqual;

  /// BaseSCEVCreator - Creates a base version of SCEV by replacing values by
  /// base values. Base value is representative of a symbase. This is to create
  /// a 1:1 mapping between blob index and symbase eliminating comparison issues
  /// between different blob indices associated with the same symbase.
  class BaseSCEVCreator;

  /// TempBlobCollector - Collects temp blobs within a blob.
  class TempBlobCollector;

  /// BlobLevelSetter - Used to set blob levels in the canon expr.
  class BlobLevelSetter;

  /// BlobPrinter - Used to print blobs.
  class BlobPrinter;

  /// PointerBlobFinder - Used to find pointer type blobs.
  class PointerBlobFinder;

  /// \brief Visits HIR and calls HIRParser's parse*() utilities. Parsing for
  /// non-essential HLInsts is postponed for phase2. Refer to isEssential().
  struct Phase1Visitor;

  /// \brief Returns the node being parsed.
  HLDDNode *getCurNode() const { return CurNode; }

  /// \brief Sets the node being parsed.
  void setCurNode(HLDDNode *Node) { CurNode = Node; }

  /// \brief Returns the instruction which best represents the current node.
  const Instruction *getCurInst() const;

  /// Phase1 parser functions
  /// parse(HLInst *, ...) is the only function used during phase2.
  void parse(HLRegion *Reg) { CurRegion = Reg; }
  void postParse(HLRegion *Reg) {}

  void parse(HLLoop *HLoop);
  void postParse(HLLoop *HLoop);

  // Non-null loop paramter indicates If is a Ztt.
  void parse(HLIf *If, HLLoop *HLoop = nullptr);
  void postParse(HLIf *If) {}

  void parse(HLSwitch *Switch);
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

  /// \brief Returns true if this instruction has a user outside the region.
  bool isRegionLiveOut(const Instruction *Inst) const;

  /// \brief Returns true if this instruction is essential (e.g. load/store) and
  /// cannot be eliminated.
  bool isEssential(const Instruction *Inst) const;

  /// \brief Wrapper over ScalarEvolution's getSCEVForHIR().
  const SCEV *getSCEV(Value *Val) const;

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
  unsigned findOrInsertBlobWrapper(BlobTy Blob, unsigned *SymbasePtr = nullptr);

  /// \brief Adds an entry for the temp blob in blob maps.
  void addTempBlobEntry(unsigned Index, unsigned NestingLevel,
                        unsigned DefLevel);

  /// \brief Overrides CE's DefinedAtLevel if the temp blob has a deeper level.
  void setTempBlobLevel(const SCEVUnknown *TempBlobSCEV, CanonExpr *CE,
                        unsigned NestingLevel);

  /// \brief Breaks multiplication blobs such as (2 * n) into multiplier 2 and
  /// new blob n, otherwise sets the multiplier to 1. Also returns new or
  /// the orignal blob, as applicable.
  void breakConstantMultiplierBlob(BlobTy Blob, int64_t *Multiplier,
                                   BlobTy *NewBlob);

  /// \brief Parses a blob into CE. If IVLevel is non-zero, blob is parsed as an
  /// IV coeff.
  void parseBlob(BlobTy Blob, CanonExpr *CE, unsigned Level,
                 unsigned IVLevel = 0);

  /// \brief Recursively parses SCEV tree into CanonExpr. IsTop is true when we
  /// are at the top of the tree and UnderCast is true if we are under a cast
  /// type SCEV.
  void parseRecursive(const SCEV *SC, CanonExpr *CE, unsigned Level,
                      bool IsTop = true, bool UnderCast = false);

  /// \brief Forces incoming value to be parsed as a blob.
  CanonExpr *parseAsBlob(const Value *Val, unsigned Level);

  /// \brief Forms and returns a CanonExpr representing Val. IsTop is passed to
  /// parseRecursive().
  CanonExpr *parse(const Value *Val, unsigned Level, bool IsTop = true);

  /// \brief Parses the i1 condition associated with conditional branches and
  /// select instructions.
  void parseCompare(const Value *Cond, unsigned Level, CmpInst::Predicate *Pred,
                    RegDDRef **LHSDDRef, RegDDRef **RHSDDRef);

  /// \brief Clears blob level map populated for the previous DDRef.
  void clearTempBlobLevelMap();

  /// \brief populates blob DDRefs for Ref based on CurTempBlobLevelMap.
  void populateBlobDDRefs(RegDDRef *Ref);

  /// \brief Returns a RegDDRef representing loop lower (constant 0).
  RegDDRef *createLowerDDRef(Type *IVType);

  /// \brief Returns a RegDDRef representing loop stride (constant 1).
  RegDDRef *createStrideDDRef(Type *IVType);

  /// \brief Returns a RegDDRef representing loop upper.
  RegDDRef *createUpperDDRef(const SCEV *BETC, unsigned Level, Type *IVType);

  /// \brief Returns the number of dimensions for a GEP base pointer type.
  unsigned getNumDimensions(Type *GEPType) const;

  /// \brief Returns the size of the contained type in bits. Incoming type is
  /// expected to be a pointer type.
  unsigned getBitElementSize(Type *Ty) const;

  /// \brief Returns the base(earliest) GEP in case there are multiple GEPs
  /// associated with this load/store.
  const GEPOperator *getBaseGEPOp(const GEPOperator *GEPOp) const;

  /// \brief Finds pointer blobs in PtrSCEV.
  const SCEV *findPointerBlob(const SCEV *PtrSCEV) const;

  /// \brief Returns either the inital or update operand of header phi
  /// corresponding to the passed in boolean argument.
  const Value *getHeaderPhiOperand(const PHINode *Phi, bool IsInit) const;

  /// \brief Returns the header phi operand which corresponds to the initial
  /// value of phi (value coming from outside the loop).
  const Value *getHeaderPhiInitVal(const PHINode *Phi) const;

  /// \brief Returns the header phi operand which corresponds to phi update
  /// (value coming from loop's backedge).
  const Value *getHeaderPhiUpdateVal(const PHINode *Phi) const;

  /// \brief Creates a canon expr which represents the initial value of header
  /// phi.
  CanonExpr *createHeaderPhiInitCE(const PHINode *Phi, unsigned Level);

  /// \brief Creates a canon expr which represents the index of header phi.
  CanonExpr *createHeaderPhiIndexCE(const PHINode *Phi, unsigned Level);

  /// \brief Wrapper for merging IndexCE2 into IndexCE1.
  static void mergeIndexCE(CanonExpr *IndexCE1, const CanonExpr *IndexCE2);

  /// \brief Creates and adds dimensions for a phi base GEP into Ref.
  /// LastIndexCE is merged with the highest phi dimension.
  /// PhiDims is the number of dimensions in the phi type.
  /// IsInBounds is set to true, if applicable.
  void addPhiBaseGEPDimensions(const GEPOperator *GEPOp, RegDDRef *Ref,
                               CanonExpr *LastIndexCE, unsigned Level,
                               unsigned PhiDims, bool &IsInBounds);

  /// \brief Creates a GEP RegDDRef for a GEP whose base pointer ia a phi node.
  RegDDRef *createPhiBaseGEPDDRef(const PHINode *BasePhi,
                                  const GEPOperator *GEPOp, unsigned Level);

  /// \brief Creates a GEP RegDDRef representing a single element. For example-
  /// %t[0].
  RegDDRef *createSingleElementGEPDDRef(const Value *GEPVal, unsigned Level);

  /// \brief Creates a GEP RegDDRef for this GEPOperator.
  RegDDRef *createRegularGEPDDRef(const GEPOperator *GEPOp, unsigned Level);

  /// \brief Returns a RegDDRef containing GEPInfo. IsUse indicates whether this
  /// is a definition or a use of GEP.
  RegDDRef *createGEPDDRef(const Value *Val, unsigned Level, bool IsUse);

  /// \brief Returns a RegDDRef representing this Null value.
  RegDDRef *createUndefDDRef(Type *Type);

  /// \brief Returns a RegDDRef representing this scalar value.
  RegDDRef *createScalarDDRef(const Value *Val, unsigned Level,
                              bool IsLval = false);

  /// \brief Returns an rval DDRef created from Val.
  RegDDRef *createRvalDDRef(const Instruction *Inst, unsigned OpNum,
                            unsigned Level);

  /// \brief Returns an lval DDRef created from Inst.
  RegDDRef *createLvalDDRef(const Instruction *Inst, unsigned Level);

  /// \brief Helper to insert newly created blobs.
  void insertBlobHelper(BlobTy Blob, unsigned Symbase, bool Insert,
                        unsigned *NewBlobIndex);

  // External interface follows. The following functions are called by the
  // framework utilities or other passes.

  /// \brief Internal method to check blob index range.
  bool isBlobIndexValid(unsigned Index) const;

  /// \brief Implements find()/insert() functionality.
  /// ReturnSymbase indicates whether to return blob index or symbase.
  unsigned findOrInsertBlobImpl(BlobTy Blob, unsigned Symbase, bool Insert,
                                bool ReturnSymbase);

  /// \brief Returns the index of Blob in the blob table. Index range is [1,
  /// UINT_MAX]. Returns InvalidBlobIndex if the blob is not present in the
  /// table.
  unsigned findBlob(BlobTy Blob);

  /// \brief Returns symbase corresponding to Blob. Returns invalid value for
  /// non-temp or non-present blobs.
  unsigned findBlobSymbase(BlobTy Blob);

  /// \brief Returns the index of Blob in the blob table. Blob is first
  /// inserted, if it isn't already present in the blob table. Index range is
  /// [1, UINT_MAX].
  unsigned findOrInsertBlob(BlobTy Blob, unsigned Symbase);

  /// \brief Returns blob corresponding to Index.
  BlobTy getBlob(unsigned Index) const;

  /// \brief Returns symbase corresponding to Index. Returns invalid value for
  /// non-temp non-present blobs.
  unsigned getBlobSymbase(unsigned Index) const;

  /// \brief Maps blobs in Blobs to their corresponding indices and inserts
  /// them in Indices.
  void mapBlobsToIndices(const SmallVectorImpl<BlobTy> &Blobs,
                         SmallVectorImpl<unsigned> &Indices);

  /// \brief Returns a new blob created from passed in Val.
  BlobTy createBlob(Value *Val, unsigned Symbase, bool Insert,
                    unsigned *NewBlobIndex);

  /// \brief Returns a new blob created from a constant value.
  BlobTy createBlob(int64_t Val, Type *Ty, bool Insert, unsigned *NewBlobIndex);

  /// \brief Returns a blob which represents (LHS + RHS). If Insert is true its
  /// index is returned via NewBlobIndex argument.
  BlobTy createAddBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                       unsigned *NewBlobIndex);

  /// \brief Returns a blob which represents (LHS - RHS). If Insert is true its
  /// index is returned via NewBlobIndex argument.
  BlobTy createMinusBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                         unsigned *NewBlobIndex);
  /// \brief Returns a blob which represents (LHS * RHS). If Insert is true its
  /// index is returned via NewBlobIndex argument.
  BlobTy createMulBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                       unsigned *NewBlobIndex);
  /// \brief Returns a blob which represents (LHS / RHS). If Insert is true its
  /// index is returned via NewBlobIndex argument.
  BlobTy createUDivBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                        unsigned *NewBlobIndex);
  /// \brief Returns a blob which represents (trunc Blob to Ty). If Insert is
  /// true its index is returned via NewBlobIndex argument.
  BlobTy createTruncateBlob(BlobTy Blob, Type *Ty, bool Insert,
                            unsigned *NewBlobIndex);
  /// \brief Returns a blob which represents (zext Blob to Ty). If Insert is
  /// true its index is returned via NewBlobIndex argument.
  BlobTy createZeroExtendBlob(BlobTy Blob, Type *Ty, bool Insert,
                              unsigned *NewBlobIndex);
  /// \brief Returns a blob which represents (sext Blob to Ty). If Insert is
  /// true its index is returned via NewBlobIndex argument.
  BlobTy createSignExtendBlob(BlobTy Blob, Type *Ty, bool Insert,
                              unsigned *NewBlobIndex);

  // TODO handle min/max blobs.

  /// \brief Returns true if Blob contains SubBlob or if Blob == SubBlob.
  bool contains(BlobTy Blob, BlobTy SubBlob) const;

  /// \brief Collects and returns temp blobs present inside Blob.
  void collectTempBlobs(BlobTy Blob, SmallVectorImpl<BlobTy> &TempBlobs) const;

  /// \brief Returns the max symbase assigned to any temp.
  unsigned getMaxScalarSymbase() const;

  /// \brief Registers new lval/symbase pairs created by HIR transformations.
  /// Only used for printing.
  void insertHIRLval(const Value *Lval, unsigned Symbase);

  /// \brief Prints scalar corresponding to Symbase.
  void printScalar(raw_ostream &OS, unsigned Symbase) const;

  /// \brief Prints blob.
  void printBlob(raw_ostream &OS, BlobTy Blob) const;

  /// \brief Checks if the blob is constant or not.
  /// If blob is constant, sets the return value in Val.
  bool isConstantIntBlob(BlobTy Blob, int64_t *Val) const;

  /// \brief Returns true if this is a temp blob.
  bool isTempBlob(BlobTy Blob) const;

  /// \brief Returns true if TempBlob always has a defined at level of zero.
  bool isGuaranteedProperLinear(BlobTy TempBlob) const;

  /// \brief Returns true if this is an UndefValue blob.
  bool isUndefBlob(BlobTy Blob) const;

  /// \brief Returns true if Blob represents a constant FP value.
  bool isConstantFPBlob(BlobTy Blob) const;

  /// \brief Returns Function object.
  Function &getFunction() const { return *Func; }

  /// \brief Returns Module object.
  Module &getModule() const { return *(getFunction().getParent()); }

  /// \brief Returns LLVMContext object.
  LLVMContext &getContext() const { return getFunction().getContext(); }

  /// \brief Returns DataLayout object.
  const DataLayout &getDataLayout() const {
    return getModule().getDataLayout();
  }

  /// Region iterator methods
  HIRCreation::iterator hir_begin() { return HIR->begin(); }
  HIRCreation::const_iterator hir_cbegin() const { return HIR->begin(); }
  HIRCreation::iterator hir_end() { return HIR->end(); }
  HIRCreation::const_iterator hir_cend() const { return HIR->end(); }

  HIRCreation::reverse_iterator hir_rbegin() { return HIR->rbegin(); }
  HIRCreation::const_reverse_iterator hir_crbegin() const {
    return HIR->rbegin();
  }
  HIRCreation::reverse_iterator hir_rend() { return HIR->rend(); }
  HIRCreation::const_reverse_iterator hir_crend() const { return HIR->rend(); }

public:
  static char ID; // Pass identification
  HIRParser();

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void print(raw_ostream &OS, const Module * = nullptr) const override;
  void verifyAnalysis() const override;
};

} // End namespace loopopt

} // End namespace llvm

#endif
