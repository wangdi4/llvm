//===---------- HIRParser.h - Parses SCEVs into CanonExprs --*- C++ -*-----===//
//
// Copyright (C) 2015-2019 Intel Corporation. All rights reserved.
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

#include "llvm/ADT/DenseSet.h"
#include "llvm/ADT/SmallVector.h"

#include "llvm/IR/InstrTypes.h"
#include "llvm/IR/Module.h"

#include "llvm/Analysis/Intel_LoopAnalysis/IR/CanonExpr.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/HLNode.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefUtils.h"
#include "llvm/Analysis/ScalarEvolution.h"

namespace llvm {

class Type;
class IntegerType;
class ArrayType;
class Function;
class PHINode;
class SCEV;
class ScalarEvolution;
class SCEVConstant;
class SCEVAddRecExpr;
class SCEVMulExpr;
class SCEVUnknown;
class GEPOrSubsOperator;
class Value;
class ConstantFP;
class Loop;
class LoopInfo;
class DominatorTree;
class IntrinsicInst;

namespace loopopt {

class HIRRegionIdentification;
class HIRCreation;
class HIRScalarSymbaseAssignment;
class HIRFramework;
class HIRLoopFormation;
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
class HLNodeUtils;

/// This analysis creates DDRefs and parses SCEVs into CanonExprs for HLNodes
/// inside HIR regions. It eliminates HLNodes useless to HIR.
///
/// It is also responsible for assigning symbases to non livein/liveout scalars.
class HIRParser {
  // CanonExprUtils provides wrapper functions on top of private functions of
  // HIRParser like createBlob() etc.
  friend class BlobUtils;
  // Needs to access getMaxScalarSymbase().
  friend class HIRSymbaseAssignment;
  // Provides access to hir_begin() to HIR transformations.
  friend class HIRFramework;

  typedef std::pair<HLInst *, unsigned> InstLevelPair;
  typedef std::map<unsigned, SmallVector<InstLevelPair, 4>> SymbaseToInstMap;

  typedef std::pair<BlobTy, unsigned> BlobSymbasePairTy;
  typedef SmallVector<BlobSymbasePairTy, 64> BlobTableTy;

  // DDRefUtils object for the framework.
  DDRefUtils DDRU;

  DominatorTree &DT;
  LoopInfo &LI;
  ScalarEvolution &SE;
  const HIRRegionIdentification &RI;
  std::reference_wrapper<HIRFramework> HIRF;
  HIRCreation &HIRC;
  HIRLoopFormation &LF;
  HIRScalarSymbaseAssignment &ScalarSA;
  HLNodeUtils &HNU;

  /// CurNode - The node we are operating on.
  HLDDNode *CurNode;

  /// CurRegion - The region we are operating on.
  HLRegion *CurRegion;

  /// CurOutermostLoop - The outermost loop of the loopnest CurNode belongs to.
  const Loop *CurOutermostLoop;

  /// CurLevel - The loop level we are operating on.
  unsigned CurLevel;

  // True when the parsing phase is done.
  bool IsReady;

  /// True when we are parsing scalar lval.
  bool ParsingScalarLval;

  /// UnclassifiedSymbaseInsts - Contains non-essential symbases (and associated
  /// instructions) as determined by the first phase of parsing.
  SymbaseToInstMap UnclassifiedSymbaseInsts;

  /// RequiredSymbases - Set of symbases determined as required by parsing.
  DenseSet<unsigned> RequiredSymbases;

  /// CurBlobLevelMap - Maps temp blob indices to nesting levels for the current
  /// DDRef.
  SmallDenseMap<unsigned, unsigned, 8> CurTempBlobLevelMap;

  // BlobTable - vector containing blobs and corresponding symbases for the
  // function.
  BlobTableTy BlobTable;

  // BlobToIndexMap - stores a mapping of blobs to corresponding indices for
  // faster lookup.
  DenseMap<BlobTy, unsigned> BlobToIndexMap;

  // SymbaseToIndexMap - maps temp symbases to blob indices for faster lookup.
  DenseMap<unsigned, unsigned> SymbaseToIndexMap;

  /// Maps all the GEP refs to their pointer values. This allows symbase
  /// assignment to do a better job by providing more information about the
  /// reference.
  DenseMap<const RegDDRef *, Value *> GEPRefToPointerMap;

  /// Stores the DistributePoints intrinsics which are populated in phase1 and
  /// processed/discarded at the end of phase2.
  SmallVector<HLInst *, 4> DistributePoints;

  /// Represents information about dimensions of a single chain of GEP
  /// operators and Subscripts instructions.
  class GEPChain;

  /// BlobProcessor - Performs necessary processing for a blob being added to a
  /// CanonExpr.
  class BlobProcessor;

  /// TempBlobCollector - Collects temp blobs within a blob.
  class TempBlobCollector;

  /// BlobPrinter - Used to print blobs.
  class BlobPrinter;

  /// PointerBlobFinder - Used to find pointer type blobs.
  class PointerBlobFinder;

  /// ScopeSCEVValidator - Validates SCEV returned by getSCEVAtScope().
  class ScopeSCEVValidator;

  /// Visits HIR and calls HIRParser's parse*() utilities. Parsing for
  /// non-essential HLInsts is postponed for phase2. Refer to isEssential().
  struct Phase1Visitor;

  /// Returns the node being parsed.
  HLDDNode *getCurNode() const { return CurNode; }

  /// Sets the node being parsed.
  void setCurNode(HLDDNode *Node) { CurNode = Node; }

  /// Returns the instruction which best represents the current node.
  const Instruction *getCurInst() const;

  // Returns true if we are in the middle of parsing scalar lval.
  bool parsingScalarLval() const { return ParsingScalarLval; }

  /// Phase1 parser functions
  /// parse(HLInst *, ...) is the only function used during phase2.
  void parse(HLRegion *Reg);
  void postParse(HLRegion *Reg) {}

  void parse(HLLoop *HLoop);
  void postParse(HLLoop *HLoop);

  // Non-null loop paramter indicates If is a Ztt.
  void parse(HLIf *If, HLLoop *HLoop = nullptr);
  void postParse(HLIf *If);

  void parse(HLSwitch *Switch);
  void postParse(HLSwitch *Switch) {}

  void parse(HLInst *HInst, bool IsPhase1, unsigned Phase2Level);
  void parse(HLLabel *Label) {}
  void parse(HLGoto *Goto) {}

  /// Implements phase1 of parsing.
  void phase1Parse(HLNode *Node);

  /// Implements phase2 of parsing.
  void phase2Parse();

  /// Returns the number of rval operands of Inst w.r.t HIR setup.
  static unsigned getNumRvalOperands(const Instruction *Inst);

  /// Returns true if this instruction has a user outside the region.
  bool isRegionLiveOut(const Instruction *Inst) const;

  /// Returns true if this instruction is essential (e.g. load/store) and cannot
  /// be eliminated.
  bool isEssential(const Instruction *Inst) const;

  /// Wrapper over ScalarEvolution's getSCEVForHIR().
  const SCEV *getSCEV(Value *Val) const;

  /// Returns the integer constant contained in ConstSCEV.
  int64_t getSCEVConstantValue(const SCEVConstant *ConstSCEV) const;

  /// Parses a SCEVConstant expr into CE's constant or denominator field based
  /// on IsDenom flag.
  void parseConstOrDenom(const SCEVConstant *ConstSCEV, CanonExpr *CE,
                         bool IsDenom);

  /// Parses a SCEVConstant expr into CE's constant field.
  void parseConstant(const SCEVConstant *ConstSCEV, CanonExpr *CE);

  /// Parses a SCEVConstant expr into CE's denominator field.
  void parseDenominator(const SCEVConstant *ConstSCEV, CanonExpr *CE);

  /// Sets the DefinedAtLevel for the Canon Expr.
  void setCanonExprDefLevel(CanonExpr *CE, unsigned NestingLevel,
                            unsigned DefLevel) const;

  /// Wrapper to find/insert Blob in the blob table by retrieving its symbase,
  /// if applicable. Returns the blob index.
  /// This is only used during parsing.
  unsigned findOrInsertBlobWrapper(BlobTy Blob);

  /// Caches temp blob level for later reuse in population of BlobDDRefs.
  void cacheTempBlobLevel(unsigned Index, unsigned NestingLevel,
                          unsigned DefLevel);

  /// In addition to calling ScalarSA's getOrAssignScalarSymbase(), it updates
  /// existing temp blob in the blob table if required. Blob index of the temp
  /// is returned if it was updated.
  unsigned getOrAssignSymbase(const Value *Temp, unsigned *BlobIndex = nullptr);

  /// Adds \p Inst in region livein set and loop livein/liveout sets as
  /// applicable and returns its def level.
  unsigned processInstBlob(const Instruction *Inst, const Instruction *BaseInst,
                           unsigned Symbase);

  /// Performs necessary processing for adding TempBlob to CE. This includes
  /// updating the defined at level of CE, adding an entry in the blob table and
  /// marking livein temps.
  const SCEVUnknown *processTempBlob(const SCEVUnknown *TempBlob, CanonExpr *CE,
                                     unsigned NestingLevel);

  /// Recusively breaks mul blobs such as (2 * n) into multiplier 2 and new blob
  /// n. Returns false if no such multiplier is found.
  bool breakConstantMultiplierMulBlob(const SCEVMulExpr *MulBlob,
                                      int64_t *Multiplier, BlobTy *NewBlob);

  /// Recusively breaks commutative blobs such as (2 + 2 * n) into multiplier 2
  /// and new blob (1 + n). Returns false if no such multiplier is found. \p
  /// IsTop is true for the topmost call.
  bool breakConstantMultiplierCommutativeBlob(BlobTy Blob, int64_t *Multiplier,
                                              BlobTy *NewBlob,
                                              bool IsTop = false);

  /// Breaks multiplication blobs such as (2 * n) into multiplier 2 and new blob
  /// n, otherwise sets the multiplier to 1. Also returns new or the orignal
  /// blob, as applicable.
  void breakConstantMultiplierBlob(BlobTy Blob, int64_t *Multiplier,
                                   BlobTy *NewBlob);

  /// Parses a blob into CE. If IVLevel is non-zero, blob is parsed as an IV
  /// coeff. If IndicateFailure is set, the function returns true/false
  /// indicating parsing success/failure instead of asserting on failures.
  bool parseBlob(BlobTy Blob, CanonExpr *CE, unsigned Level,
                 unsigned IVLevel = 0, bool IndicateFailure = false);

  /// Returns true if the passed in SCEV is valid for use in HIR.
  bool isValidScopeSCEV(const SCEV *SC) const;

  /// Calls SE->getSCEVAtScope() based on the location of CurNode.
  const SCEV *getSCEVAtScope(const SCEV *SC) const;

  /// Parses an AddRec into CanonExpr. This and parseRecursive() can call each
  /// other.
  bool parseAddRec(const SCEVAddRecExpr *AddRec, CanonExpr *CE, unsigned Level,
                   bool IndicateFailure);

  /// Parses a Mul into CanonExpr. It can create an IV term by calling
  /// parseAddRec().
  bool parseMul(const SCEVMulExpr *MulSCEV, CanonExpr *CE, unsigned Level,
                bool IndicateFailure);

  /// Recursively parses SCEV tree into CanonExpr. IsTop is true when we are at
  /// the top of the tree and UnderCast is true if we are under a cast type
  /// SCEV. If IndicateFailure is set, the function returns true/false
  /// indicating parsing success/failure instead of asserting on failures.
  bool parseRecursive(const SCEV *SC, CanonExpr *CE, unsigned Level,
                      bool IsTop = true, bool UnderCast = false,
                      bool IndicateFailure = false);

  /// Returns true if \p CI's SCEV contains a SCEVCastExpr whose operand is an
  /// AddRec with the same type as \p CI's operand.
  bool containsCastedAddRec(const CastInst *CI, const SCEV *SC) const;

  /// Returns true if the src type of \p CI is same as parent loop's IV type and
  /// its SCEV form is not a cast.
  bool isCastedFromLoopIVType(const CastInst *CI, const SCEV *SC) const;

  /// Returns true if we should parse \p CI by explicitly hiding the cast in the
  /// instruction and recursively parsing the cast operand.
  bool shouldParseWithoutCast(const CastInst *CI, bool IsTop) const;

  /// Forces incoming value to be parsed as a blob. \p FinalIntTy is the
  /// eventual type of the value we want to return. This is done by either
  /// truncating or sign extending \p Val.
  CanonExpr *parseAsBlob(const Value *Val, unsigned Level,
                         IntegerType *FinalIntTy = nullptr);

  /// Forms and returns a CanonExpr representing Val. IsTop is passed to
  /// parseRecursive(). \p FinalIntTy is the eventual type of the value we want
  /// to return. This is done by either truncating or sign extending \p Val.
  CanonExpr *parse(const Value *Val, unsigned Level, bool IsTop = true,
                   IntegerType *FinalIntTy = nullptr);

  /// Returns fast math flags if \p Cmp is a FPMathOperator. Returns default
  /// FMF otherwise.
  static FastMathFlags parseFMF(const CmpInst *Cmp);

  /// Parses llvm.dbg.* intrinsic and store info in the parent HLRegion.
  bool parsedDebugIntrinsic(const IntrinsicInst *Intrin);

  /// Returns true if \p HInst was processed as a removable intrinsic for HIR.
  bool processedRemovableIntrinsic(HLInst *HInst);

  /// Parses LLVM metadata and updates RegDDRef \p Ref.
  static void parseMetadata(const Instruction *Inst, RegDDRef *Ref);

  /// Parses LLVM instruction metadata and updates canon expr \p CE.
  static void parseMetadata(const Instruction *Inst, CanonExpr *CE);

  /// Helper function to call parseMetadata(const Instruction *, CanonExpr *).
  static void parseMetadata(const Value *Val, CanonExpr *CE);

  /// Parses the i1 condition associated with conditional branches and select
  /// instructions and returns predicates in \p Preds and DDRefs in \p Refs. \p
  /// AllowMultiplePreds indicates whether we should break '&&' conditions into
  /// different predicates.
  void parseCompare(const Value *Cond, unsigned Level,
                    SmallVectorImpl<HLPredicate> &Preds,
                    SmallVectorImpl<RegDDRef *> &Refs, bool AllowMultiplePreds);

  /// Parses the i1 condition associated with conditional branches and select
  /// instructions into a single predicate.
  void parseCompare(const Value *Cond, unsigned Level, HLPredicate *Pred,
                    RegDDRef **LHSDDRef, RegDDRef **RHSDDRef);

  /// Clears blob level map populated for the previous DDRef.
  void clearTempBlobLevelMap();

  /// populates blob DDRefs for Ref based on CurTempBlobLevelMap.
  void populateBlobDDRefs(RegDDRef *Ref, unsigned Level);

  /// Returns a RegDDRef representing loop lower (constant 0).
  RegDDRef *createLowerDDRef(Type *IVType) {
    return getDDRefUtils().createConstDDRef(IVType, 0);
  }

  /// Returns a RegDDRef representing loop stride (constant 1).
  RegDDRef *createStrideDDRef(Type *IVType) {
    return getDDRefUtils().createConstDDRef(IVType, 1);
  }

  /// Returns a RegDDRef representing loop upper.
  RegDDRef *createUpperDDRef(const SCEV *BETC, unsigned Level, Type *IVType,
                             const Loop *Lp);

  /// Returns the size of the contained type in bytes. Incoming type is expected
  /// to be a pointer type. May return zero if the pointer element type is
  /// not-sized.
  unsigned getPointerElementSize(Type *Ty) const;

  /// Check if \p GEPOp is compatible with existing chain represented by \p
  /// StridesV. If compatible, GEPOp strides will merged into StrideV.
  bool isCompatibleGEOpWithChain(const GEPOrSubsOperator *GEPOp,
                                 SmallVector<Value *, 8> &StridesV) const;

  // Returns true if it is valid to parse this GEPOrSubsOperator.
  bool isValidGEPOp(const GEPOrSubsOperator *GEPOp,
                    bool SkipLiveRangeCheck = false) const;

  /// Returns the base(earliest) GEP in case there are multiple GEPs associated
  /// with this load/store.
  const GEPOrSubsOperator *getBaseGEPOp(const GEPOrSubsOperator *GEPOp) const;

  /// Returns either the inital or update operand of header phi corresponding to
  /// the passed in boolean argument.
  const Value *getHeaderPhiOperand(const PHINode *Phi, bool IsInit) const;

  /// Returns the header phi operand which corresponds to the initiala value of
  /// phi (value coming from outside the loop).
  const Value *getHeaderPhiInitVal(const PHINode *Phi) const;

  /// Returns the header phi operand which corresponds to phi update (value
  /// coming from loop's backedge).
  const Value *getHeaderPhiUpdateVal(const PHINode *Phi) const;

  /// Creates a canon expr which represents the index of header phi.
  CanonExpr *createHeaderPhiIndexCE(const PHINode *Phi, unsigned Level);

  /// Wrapper for merging IndexCE2 into IndexCE1.
  static void mergeIndexCE(CanonExpr *IndexCE1, const CanonExpr *IndexCE2);

  /// Populates GEPOp's indices which represent structure field offsets into
  /// Offsets vector. Other GEP indices are marked with -1.
  static void populateOffsets(const GEPOrSubsOperator *GEPOp,
                              SmallVectorImpl<int64_t> &Offsets);

  /// Returns true if the last index of \p GEPOp is a structure offset.
  static bool representsStructOffset(const GEPOrSubsOperator *GEPOp);

  /// Populates \p Ref's dimensions by processing GEPOrSubsOperator starting
  /// from \p GEPOp till we hit the base GEP. \p RequiresIndexMerging is set for
  /// phi base GEPs to indicate that the inductive dimension of the base will be
  /// merged into the populated highest dimension.
  void populateRefDimensions(RegDDRef *Ref, const GEPOrSubsOperator *GEPOp,
                             unsigned Level, bool RequiresIndexMerging);

  /// Creates and adds dimensions for a phi base GEP.
  /// \p InitGEPOp is the GEPOrSubsOperator obtained from base phi's initial
  /// value. \p IndexCE is merged with the highest \p Ref dimension.
  void addPhiBaseGEPDimensions(const GEPOrSubsOperator *GEPOp,
                               const GEPOrSubsOperator *InitGEPOp,
                               RegDDRef *Ref, CanonExpr *IndexCE,
                               CanonExpr *StrideCE, Type *DimType,
                               unsigned Level);

  /// Given the initial value of a header phi, it returns a value which can act
  /// as the base of the Ref formed using the header phi. A null value indicates
  /// that the phi cannot be decomposed into intial and stride. It also returns
  /// the GEPOrSubsOperator associated with the initial value, if applicable.
  const Value *getValidPhiBaseVal(const Value *PhiInitVal,
                                  const GEPOrSubsOperator **InitGEPOp) const;

  /// Creates a GEP RegDDRef for a GEP whose base pointer ia a phi node.
  RegDDRef *createPhiBaseGEPDDRef(const PHINode *BasePhi,
                                  const GEPOrSubsOperator *GEPOp,
                                  unsigned Level);

  /// Creates a GEP RegDDRef representing a single element. For example- %t[0].
  RegDDRef *createSingleElementGEPDDRef(const Value *GEPVal, unsigned Level);

  /// Creates a GEP RegDDRef for this GEPOrSubsOperator.
  RegDDRef *createRegularGEPDDRef(const GEPOrSubsOperator *GEPOp,
                                  unsigned Level);

  /// Returns a RegDDRef containing GEPInfo. IsUse indicates whether this is a
  /// definition or a use of GEP.
  RegDDRef *createGEPDDRef(const Value *Val, unsigned Level, bool IsUse);

  /// Returns a RegDDRef representing this scalar value.
  /// \p LvalInst is non-null if this value represents an Lval.
  RegDDRef *createScalarDDRef(const Value *Val, unsigned Level,
                              HLInst *LvalInst = nullptr);

  /// Populates all symbases which are required to be kept in HIR based on the
  /// parsing for \p Ref. These are basically temp blobs contained in \p Ref.
  void populateRequiredSymbases(const RegDDRef *Ref);

  /// Returns an rval DDRef created from Val.
  RegDDRef *createRvalDDRef(const Instruction *Inst, unsigned OpNum,
                            unsigned Level);

  /// Returns an lval DDRef created from HInst.
  RegDDRef *createLvalDDRef(HLInst *HInst, unsigned Level);

  /// Helper to insert newly created blobs.
  void insertBlobHelper(BlobTy Blob, unsigned Symbase, bool Insert,
                        unsigned *NewBlobIndex);

  /// Updates OldBlob by NewBlob in the blob table and returns the blob index.
  unsigned updateTempBlob(BlobTy OldBlob, BlobTy NewBlob, unsigned Symbase);

  // Adds an lval or rval fake ref to \p HInst formed by cloning \p AddressRef.
  void addFakeRef(HLInst *HInst, const RegDDRef *AddressRef, bool IsRval);

  // Clears all the per-region data structures.
  void clearRegionData();

  // ---------------------------------------------------------------------
  // External interface follows. The following functions are called by the
  // framework utilities or other passes.
  // ---------------------------------------------------------------------

  /// Internal method to check blob index range.
  bool isBlobIndexValid(unsigned Index) const;

  /// Returns true if Symbase is assigned to some temp blob.
  bool foundInBlobTable(unsigned Symbase) const;

  /// Validates blob/symbase pair.
  bool validBlobSymbasePair(BlobTy Blob, unsigned Symbase) const;

  /// Implements find()/insert() functionality.
  /// ReturnSymbase indicates whether to return blob index or symbase.
  unsigned findOrInsertBlobImpl(BlobTy Blob, unsigned Symbase, bool Insert,
                                bool ReturnSymbase, BlobTy NewBlob = nullptr);

  /// Returns the index of Blob in the blob table. Index range is [1, UINT_MAX].
  /// Returns InvalidBlobIndex if the blob is not present in the table.
  unsigned findBlob(BlobTy Blob);

  /// Returns symbase corresponding to \p Blob. Returns InvalidSymbase if not
  /// found.
  unsigned findTempBlobSymbase(BlobTy Blob);

  /// Returns blob index corresponding to \p Symbase. Returns InvalidBlobIndex
  /// if blob is not found.
  unsigned findTempBlobIndex(unsigned Symbase) const;

  /// Finds or inserts a temp blob index corresponding to \p Symbase and returns
  /// it.
  unsigned findOrInsertTempBlobIndex(unsigned Symbase);

  /// Returns the index of Blob in the blob table. Blob is first inserted, if it
  /// isn't already present in the blob table. Index range is [1, UINT_MAX].
  unsigned findOrInsertBlob(BlobTy Blob, unsigned Symbase);

  /// Returns blob corresponding to Index.
  BlobTy getBlob(unsigned Index) const;

  /// Returns symbase corresponding to \p Index. Asserts if a valid symbase is
  /// not found.
  unsigned getTempBlobSymbase(unsigned Index) const;

  /// Maps blobs in Blobs to their corresponding indices and inserts them in
  /// Indices.
  void mapBlobsToIndices(const SmallVectorImpl<BlobTy> &Blobs,
                         SmallVectorImpl<unsigned> &Indices);

  /// Returns a new blob created from passed in Val.
  BlobTy createBlob(Value *Val, unsigned Symbase, bool Insert,
                    unsigned *NewBlobIndex);

  /// Returns a new blob created from a constant value.
  BlobTy createBlob(int64_t Val, Type *Ty, bool Insert, unsigned *NewBlobIndex);

  /// Returns a blob which represents (LHS + RHS). If Insert is true its index
  /// is returned via NewBlobIndex argument.
  BlobTy createAddBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                       unsigned *NewBlobIndex);

  /// Returns a blob which represents (LHS - RHS). If Insert is true its index
  /// is returned via NewBlobIndex argument.
  BlobTy createMinusBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                         unsigned *NewBlobIndex);
  /// Returns a blob which represents (LHS * RHS). If Insert is true its index
  /// is returned via NewBlobIndex argument.
  BlobTy createMulBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                       unsigned *NewBlobIndex);
  /// Returns a blob which represents (LHS / RHS). If Insert is true its index
  /// is returned via NewBlobIndex argument.
  BlobTy createUDivBlob(BlobTy LHS, BlobTy RHS, bool Insert,
                        unsigned *NewBlobIndex);
  /// Returns a blob which represents (trunc Blob to Ty). If Insert is true its
  /// index is returned via NewBlobIndex argument.
  BlobTy createTruncateBlob(BlobTy Blob, Type *Ty, bool Insert,
                            unsigned *NewBlobIndex);
  /// Returns a blob which represents (zext Blob to Ty). If Insert is true its
  /// index is returned via NewBlobIndex argument.
  BlobTy createZeroExtendBlob(BlobTy Blob, Type *Ty, bool Insert,
                              unsigned *NewBlobIndex);
  /// Returns a blob which represents (sext Blob to Ty). If Insert is true its
  /// index is returned via NewBlobIndex argument.
  BlobTy createSignExtendBlob(BlobTy Blob, Type *Ty, bool Insert,
                              unsigned *NewBlobIndex);
  /// Returns a new blob with appropriate cast (SExt, ZExt, Trunc) applied on
  /// top of \p Blob. If Insert is true its index is returned via NewBlobIndex
  /// argument.
  BlobTy createCastBlob(BlobTy Blob, bool IsSExt, Type *Ty, bool Insert,
                        unsigned *NewBlobIndex);

  /// Returns a new blob represented smin of BlobA and BlobB. If Insert is
  /// true its index is returned via NewBlobIndex argument. Blob types should
  /// match each other.
  BlobTy createSMinBlob(BlobTy BlobA, BlobTy BlobB, bool Insert,
                        unsigned *NewBlobIndex);

  /// Returns a new blob represented smax of BlobA and BlobB. If Insert is
  /// true its index is returned via NewBlobIndex argument. Blob types should
  /// match each other.
  BlobTy createSMaxBlob(BlobTy BlobA, BlobTy BlobB, bool Insert,
                        unsigned *NewBlobIndex);

  /// Returns a new blob represented umin of BlobA and BlobB. If Insert is
  /// true its index is returned via NewBlobIndex argument. Blob types should
  /// match each other.
  BlobTy createUMinBlob(BlobTy BlobA, BlobTy BlobB, bool Insert,
                        unsigned *NewBlobIndex);

  /// Returns a new blob represented umax of BlobA and BlobB. If Insert is
  /// true its index is returned via NewBlobIndex argument. Blob types should
  /// match each other.
  BlobTy createUMaxBlob(BlobTy BlobA, BlobTy BlobB, bool Insert,
                        unsigned *NewBlobIndex);

  /// Returns true if Blob contains SubBlob or if Blob == SubBlob.
  bool contains(BlobTy Blob, BlobTy SubBlob) const;

  /// Collects and returns temp blobs present inside Blob.
  void collectTempBlobs(BlobTy Blob, SmallVectorImpl<BlobTy> &TempBlobs) const;

  /// Replaces \p OldTempIndex by \p NewTempIndex in \p BlobIndex and returns
  /// the new blob in \p NewBlobIndex. Returns true if blob was replaced.
  bool replaceTempBlob(unsigned BlobIndex, unsigned TempIndex, BlobTy ByBlob,
                       unsigned &NewBlobIndex, int64_t &SimplifiedConstant);

  /// Replaces \p TempIndex by \p Constant in the \p BlobIndex blob. If the blob
  /// becomes constant the \p NewBlobIndex will be assigned to InvalidBlobIndex
  /// and \p SimplifiedConstant will contain a constant value.
  bool replaceTempBlobByConstant(unsigned BlobIndex, unsigned TempIndex,
                                 int64_t Constant, unsigned &NewBlobIndex,
                                 int64_t &SimplifiedConstant);

  /// Prints scalar corresponding to Symbase.
  void printScalar(raw_ostream &OS, unsigned Symbase) const;

  /// Prints blob.
  void printBlob(raw_ostream &OS, BlobTy Blob) const;

  /// Returns true if this is a temp blob.
  static bool isTempBlob(BlobTy Blob);

  /// Returns true if minimum value of blob is known and sets it in \p Val.
  bool getMinBlobValue(BlobTy Blob, int64_t &Val) const;

  /// Returns true if maximum value of blob is known and sets it in \p Val.
  bool getMaxBlobValue(BlobTy Blob, int64_t &Val) const;

  /// Returns true if \p HInst is a livein copy.
  bool isLiveinCopy(const HLInst *HInst);

  /// Returns true if \p HInst is a liveout copy.
  bool isLiveoutCopy(const HLInst *HInst);

  Value *getGEPRefPtr(const RegDDRef *Ref) const {
    auto It = GEPRefToPointerMap.find(Ref);
    assert((It != GEPRefToPointerMap.end()) &&
           "Could not find Ref's underlying pointer!");
    return It->second;
  }

  // Tries to trace back a pointer type instruction to the array type from which
  // it was created. This can happen if the base originated from an alloca.
  //
  // For example, if \p Ptr is %1 in the example below we will return [12 x i8].
  //
  // %1 = getelementptr inbounds [12 x i8], [12 x i8]* %tmpbuf.i, i32 0, i32 0
  // Returns number of elements in outer dimension.
  unsigned getPointerDimensionSize(const Value *Ptr) const;

  /// Returns HLNodeUtils object.
  HLNodeUtils &getHLNodeUtils() { return HNU; }

  /// Returns DDRefUtils object.
  DDRefUtils &getDDRefUtils() { return DDRU; }

  /// Returns CanonExprUtils object.
  CanonExprUtils &getCanonExprUtils() { return DDRU.getCanonExprUtils(); }
  const CanonExprUtils &getCanonExprUtils() const {
    return DDRU.getCanonExprUtils();
  }

  /// Returns BlobUtils object.
  BlobUtils &getBlobUtils() { return DDRU.getBlobUtils(); }

  struct DelinearizedCoeffBlobIndex {
    unsigned DimIndex;
    int64_t Coeff;
    unsigned BlobIndex;

    DelinearizedCoeffBlobIndex() : Coeff(0) {}
    DelinearizedCoeffBlobIndex(unsigned DimIndex, int64_t Coeff,
                               unsigned BlobIndex)
        : DimIndex(DimIndex), Coeff(Coeff), BlobIndex(BlobIndex) {}
  };

  /// Return the dimension index with the new blob and coeff after blob with \p
  /// BlobIndex being mapped using \p DimSizes.
  Optional<DelinearizedCoeffBlobIndex>
  delinearizeBlobIndex(Type *IndexType, unsigned BlobIndex,
                       SmallVectorImpl<BlobTy> &DimSizes);

  RegDDRef *delinearizeSingleRef(const RegDDRef *Refs,
                                 SmallVectorImpl<BlobTy> &Strides,
                                 SmallVectorImpl<BlobTy> &DimSizes);

public:
  HIRParser(DominatorTree &DT, LoopInfo &LI, ScalarEvolution &SE,
            HIRRegionIdentification &RI, HIRFramework &HIRF, HIRCreation &HIRC,
            HIRLoopFormation &LF, HIRScalarSymbaseAssignment &ScalarSA,
            HLNodeUtils &HNU)
      : DDRU(*this), DT(DT), LI(LI), SE(SE), RI(RI), HIRF(HIRF), HIRC(HIRC),
        LF(LF), ScalarSA(ScalarSA), HNU(HNU), CurNode(nullptr),
        CurRegion(nullptr), CurOutermostLoop(nullptr), CurLevel(0),
        IsReady(false), ParsingScalarLval(false) {
    // Connect contained DDRefUtils to HLNodeUtils object.
    HNU.DDRU = &DDRU;
  };

  void run();

  // Returns thre whenever the parsing phase is completed.
  bool isReady() { return IsReady; }

  /// Returns pointer to HIRFramework.
  HIRFramework &getHIRFramework() { return HIRF; }
  const HIRFramework &getHIRFramework() const { return HIRF; }

  /// Returns Function object.
  Function &getFunction() const { return getHIRFramework().getFunction(); }

  /// Returns Module object.
  Module &getModule() const { return getHIRFramework().getModule(); }

  /// Returns LLVMContext object.
  LLVMContext &getContext() const { return getHIRFramework().getContext(); }

  /// Returns DataLayout object.
  const DataLayout &getDataLayout() const {
    return getHIRFramework().getDataLayout();
  }

  /// Refer to DDRefUtils::delinearizeRefs().
  bool delinearizeRefs(ArrayRef<const loopopt::RegDDRef *> GepRefs,
                       SmallVectorImpl<loopopt::RegDDRef *> &OutRefs,
                       SmallVectorImpl<BlobTy> *DimSizes = nullptr);
};

} // End namespace loopopt

} // End namespace llvm

#endif
