#if INTEL_FEATURE_SW_ADVANCED
//===--- HIRInterLoopBlocking.h - Utils for Inter Loop Blocking -------===//
//
// Copyright (C) 2023 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive
// property of Intel Corporation and may not be disclosed, examined
// or reproduced in whole or in part without explicit written authorization
// from the company.
//
//===--------------------------------------------------------------------===//
//

#ifndef LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRINTERLOOPBLOCK_H
#define LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRINTERLOOPBLOCK_H

#include "HIRPrintDiag.h"
#include "llvm/Analysis/Intel_LoopAnalysis/IR/RegDDRef.h"
#include "llvm/Analysis/Intel_LoopAnalysis/Utils/DDRefGrouping.h"

using namespace llvm;
using namespace llvm::loopopt;

namespace llvm {

namespace loopopt {
// Per-dimension information
// Records the matching loop to a dimension as an offset of levels from
// innermost loop. For example, with a following example
// DO K    // L1
//  DO J   // L2
//   DO I  // L3
//     H[K][J][I]
// A DimInfoTy is described as
// Dim-3 [K] -- LevelOffset is 2
// Dim-2 [J] -- LevelOffset is 1
// Dim-1 [I] -- LevelOffset is 0
// It is assumed that a dimension CE is among the following tree forms.
//  BLOB - blobs + optional constants - LevelOffset is a negative number.
//  KONST - pure constansts - LevelOffset is a negative number.
//  has IV - LevelOffset is valid
// See "Kind" below
class DimInfoTy {
public:
  // BLOB - blobs + optional constants
  // KONST - pure constansts
  // INVALID - dimension form not analyzable.
  enum Kind {
    BLOB = -3,
    KONST = -2,
    INVALID = -1,
  };

  DimInfoTy() : LevelOffset(INVALID) {}

  // Offset from InnermostLevel - (loop level of the IV appearing in this array
  // dimension) Always non-negative because innermost loop has the
  // largest Level. When this dimension does not have IV, but constant and
  // blobs, this field will have negative values.
  int LevelOffset;

  bool hasIV() const { return LevelOffset >= 0; }

  operator int() const { return LevelOffset; }
  void operator=(int Val) { LevelOffset = Val; }

  bool operator==(const DimInfoTy &Other) const {
    return LevelOffset == Other.LevelOffset;
  }

  bool operator<=(const DimInfoTy &Other) const {
    return LevelOffset <= Other.LevelOffset;
  }

  bool operator<(const DimInfoTy &Other) const {
    return LevelOffset < Other.LevelOffset;
  }

  bool operator>(const DimInfoTy &Other) const { return !operator<=(Other); }

  // Used for transformation
  bool isConstant() const { return LevelOffset == KONST; }

  bool isBlob() const { return LevelOffset == BLOB; }
};

typedef SmallVector<DimInfoTy, 4> DimInfoVecTy;
typedef SmallVectorImpl<DimInfoTy> DimInfoVecImplTy;

typedef DenseMap<unsigned, const RegDDRef *> BaseIndexToLowersAndStridesTy;
typedef SmallVector<SmallVector<int64_t, 64>, MaxLoopNestLevel>
    InnermostLoopToShiftTy;
typedef DDRefGatherer<RegDDRef, MemRefs> MemRefGatherer;

typedef DDRefGrouping::RefGroupVecTy<RegDDRef *> RefGroupVecTy;
typedef DDRefGrouping::RefGroupTy<RegDDRef *> RefGroupTy;

typedef std::pair<HLLoop *, SmallVector<DimInfoTy, 4>> LoopAndDimInfoTy;
typedef std::vector<LoopAndDimInfoTy> LoopToDimInfoTy;
typedef std::map<const HLLoop *, RegDDRef *> LoopToRefTy;
typedef std::map<const HLLoop *, const RegDDRef *> LoopToConstRefTy;

class Transformer {
  struct TopSortCompare {
    bool operator()(const HLInst *Inst1, const HLInst *Inst2) const {
      return Inst1->getTopSortNum() < Inst2->getTopSortNum();
    }
  };

  typedef std::set<const HLInst *, TopSortCompare> InstsToCloneSetTy;

  // Subtract AdjustingRef from a ref in loop's body.
  // by the same def location.
  // Example:
  // Input loop before alignment
  // for i = 0, N
  //  for j = 0, M
  //   a[i][j+1] = b[i][j] + 3;
  //
  // After alignment
  // for i = 0, N
  //  for j = 1, M + 1
  //   a[i][j] = b[i][j - 1] + 3;
  //
  // This function only takes care of loop's body. Loop bounds are taken care
  // of another function.
  // The alignment is achieved by subtracting AdjustingRef, base[0][1] from
  // a memref.
  class LoopBodyAligner final : public HLNodeVisitorBase {
  private:
    HLNode *SkipNode;

    // Loop to update
    HLLoop *Loop;
    const RegDDRef *AdjustingRef;
    const DenseMap<unsigned, unsigned> &MapFromLevelToDim;

  public:
    LoopBodyAligner(HLLoop *Loop, const RegDDRef *AdjustingRef,
                    const DenseMap<unsigned, unsigned> &MapFromLevelToDim)
        : SkipNode(nullptr), Loop(Loop), AdjustingRef(AdjustingRef),
          MapFromLevelToDim(MapFromLevelToDim) {}

    void update() {
      HLNodeUtils::visitRange(*this, Loop->child_begin(), Loop->child_end());
    }

    // Skip any inner level loops. This visitor is supposed to take care of
    // only the refs in its self body, not in bodies of its children.
    void visit(HLLoop *Lp) { SkipNode = Lp; };
    void visit(HLNode *Node) {}
    void postVisit(HLNode *Node) {}

    bool skipRecursion(HLNode *Node) { return SkipNode == Node; }

    // Main logic: update all ddrefs of a HLDDNode
    void visit(HLDDNode *Node) {
      for (auto *Ref : make_range(Node->ddref_begin(), Node->ddref_end())) {

        if (!Ref->hasGEPInfo())
          continue;

        std::unique_ptr<RegDDRef> OrigRef(Ref->clone());

        // For a Ref, go through dimensions, and get IV Level
        // Get the dim and get the CE from RepRef
        for (auto *CE : make_range(Ref->canon_begin(), Ref->canon_end())) {

          assert(CE->numIVs() <= 1);

          // get IV Level from CE
          unsigned FoundIVLevel = 0;
          for (auto Level : make_range(AllLoopLevelRange::begin(),
                                       AllLoopLevelRange::end())) {
            int64_t Coeff = 0;
            unsigned Index = 0;
            CE->getIVCoeff(Level, &Index, &Coeff);
            if (Coeff != 0) {
              FoundIVLevel = Level;
              break;
            }
          }

          if (!FoundIVLevel)
            continue;

          auto It = MapFromLevelToDim.find(FoundIVLevel);
          assert(It != MapFromLevelToDim.end());
          unsigned Dim = It->second;
          const CanonExpr *AdjustCE = AdjustingRef->getDimensionIndex(Dim);

          CanonExprUtils::subtract(CE, AdjustCE);
        }

#if 0
        // Initial makeConsistent using original refs.
        // After this point, nonlinear is truly nonlinear.
        // Also, IV + const should have Def@Level zero.
        //llvm::loopopt::printMarker("Orig Ref: ", {OrigRef.get()}, true);
        printMarker("AdjustingRef: ", {AdjustingRef}, true);
        printMarker("Ref: ", {Ref}, true);
#endif

        // Cannot use RepRef here as an auxiliary ref
        // because RepRef itself is aligned by this function
        // and get changed. Instead, use AdjustingRef,
        // since it is a RepRef minus IVs.
        // It contains all the blobs.
        Ref->makeConsistent({OrigRef.get(), AdjustingRef});
      }
    }

  private:
    static void alignSpatialLoopBounds(RegDDRef *Ref,
                                       const CanonExpr *AdjustingCE,
                                       const RegDDRef *AdjustingRef);
  };

public:
  HIRDDAnalysis &DDA;

  // Entry value 0 denotes no-blocking.
  // Size of StripmineSizes should be the same as global NumDims
  Transformer(ArrayRef<unsigned> StripmineSizes,
              const LoopToDimInfoTy &InnermostLoopToDimInfos,
              const LoopToConstRefTy &InnermostLoopToRepRef,
              const InnermostLoopToShiftTy &InnermostLoopToShift,
              HLNode *NodeOutsideByStrip, HIRDDAnalysis &DDA, StringRef Func,
              bool CloneDVLoads = true);

  static unsigned getNumByStripLoops(ArrayRef<unsigned> StripmineSizes) {
    return count_if(StripmineSizes, [](unsigned Size) { return Size; });
  }

  // Make sure every dimension has a target loop.
  bool checkDimsToLoops(ArrayRef<unsigned> StripmineSizes,
                        const LoopToDimInfoTy &InnermostLoopToDimInfos);

  // A spatial loopnest's depth is as many as the number of dimensions in refs.
  // (e.g. i, j, k-loop for A[i][j][k] ref). All these spatial loops are
  // considered dimension-matching loops.
  // If there are loops between NodeOutsideByStrip and outermost spatial loop
  // (in the previous example i-loop is the outermost spatial loop),
  // these loops are loops that doesn't correspond to any dimension of a memref
  // and can be called nonDimMatchingLoop.
  //
  // This function sees if there are nonDimMatchingLoops by comparing levels of
  // NodeOutside and outermost spatial loop.
  //
  // See the following example.
  //
  // Region                      <-- NodeOutsideByStrip level = 0
  // DO i1 = 0, N
  //    DO i2 = 0, M             <-- Outermost Spatial Loop Level = 2
  //       A[i2] = A[i2] + ...
  //
  // NumDims = 1, with only one-dimension and is the number of ByStrip loops.
  //
  //  NodeOutsideByStrip        Lv0_0
  //      NonDimMatchinLoops    Lv2_0, Lv2_1
  //        SpatialLoops (blocked unit-stride loops) Lv3_0, Lv3_1, ...
  //
  // Here, (Lv3_0 - Lv0_0) -  NumDims > 0 means the existence of
  // NonDimMatchingLoops. For the example above, (2 - 0) - 1 > 0 shows that
  // i1-loop is not a dimension-matching, but a nonDimMatchingLoop.
  //
  // This function is supposed to be called in the initialization stage
  // before ByStripLoops are added.
  // During transformation, ByStripLoops will be added before
  // outermost NonDimMatchingLoop.
  //
  // After transformation.
  //  NodeOutsideByStrip        Lv0_0
  //    ByStripLoops (as many as NumDims)           Lv1_0, Lv1_1, Lv1_2
  //      NonDimMatchinLoops    Lv2_0, Lv2_1
  //        SpatialLoops (blocked unit-stride loops) Lv3_0, Lv3_1, ...
  bool hasNonDimMatchingLoop(unsigned NumDims,
                             const LoopToDimInfoTy &InnermostLoopToDimInfo,
                             const HLNode *NodeOutsideByStripLevel) const;

  // Checks minimal assumptions when HasNonDimMatchingLoop is true.
  bool verifyAssumptionsWithNonDimMatchingLoop(const HLNode *AnchorNode) const;

  bool rewrite();

private:
  // Given a representative ref, RepRef, come up with a ref where IVs are
  // cleared. Example: RepRef = A[i][j + 1] --> an AdjustingRef = A[0][1]
  //
  // Later, resulting AdjustingRefs are used for alignment of loops, where
  // a memref B[i + 1][j] will adjusted by A[0][1] being subtracted from
  // and become B[i+1][j-1].
  void prepareAdjustingRefs(LoopToRefTy &InnermostLoopToAdjustingRef) const;

  void collectLiveInsToByStripLoops(HLNode *AnchorNode,
                                    HLNode *LastByStripNode);

  SmallVector<unsigned, 16>
  collectLiveOutsOfByStripLoops(HLNode *AnchorNode,
                                HLNode *LastByStripNode) const;
  SmallVector<unsigned, 16>
  quickCollectLiveInOutsOfByStrip(const HLLoop *AnchorNode,
                                  HLLoop::const_live_in_iterator begin,
                                  HLLoop::const_live_in_iterator end) const;

  // Collect LiveIns and LiveOuts.
  // [DefBeginIt, DefEndIt) is the range where Lvals are found.
  // [UseStartTopSortNum, UseTopSortNum] is the range of TopSortNumbers where
  // uses are found. If an edge from the def-range to use-range exists, the
  // symbase of the corresponding lval(ddref)'s symbase is populated into
  // LiveInOrOut. Being LiveIn or LiveOut are dependent on the caller site of
  // this function.
  template <bool IsAllRefer = false>
  void collectLiveInOutForByStripLoops(
      HLContainerTy::iterator DefBeginIt, HLContainerTy::iterator DefEndIt,
      unsigned UseStartTopSortNum, unsigned UseLastTopSortNum, DDGraph DDG,
      SmallVectorImpl<unsigned> &LiveInOrOut) const;

  void updateDefAtLevelOfSpatialLoops(HLNode *Node, unsigned LowestLevel) const;

  // Increase def@level of Ref by Increase if current def@level is
  // greater than equal to LevelThreshold.
  static void incDefinedAtLevelBy(RegDDRef *Ref, unsigned Increase,
                                  unsigned LowestLevel);

  // Add AdjustingRef to loop's bounds.
  CanonExpr *alignSpatialLoopBounds(RegDDRef *Ref, const RegDDRef *AdjustingRef,
                                    unsigned DimNum) const;

  // Adjust a loops LB, and UB and subscripts so that
  // all Lval ddrefs has only IVs but no constant of blob
  // in every dimension.
  // For example, if a lval was a[i + const1][j + blob]
  // it will become a[i][j] by subtraction const1, blob
  // Example:
  // Input loop before alignment
  // for i = 0, N
  //  for j = 0, M
  //   a[i][j+1] = b[i][j] + 3;
  //
  // After alignment
  // for i = 0, N
  //  for j = 1, M + 1
  //   a[i][j] = b[i][j - 1] + 3;
  void alignSpatialLoops(const LoopToRefTy &InnermostLoopToAdjustingRef);

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DEBUG)
  static void printDDEdges(const HLInst *LoadInst, DDGraph DDG);
#endif

  bool checkInvariance(const HLInst *HInst) const;

  // Start from the RHS of Copy or any other instruction
  // to find the eventual load instruction or the instruction
  // whose rvals are all liveIn to the region.
  // If not found or meet an unexpected situation, return false.
  // Example 1)
  //   %t1 = %a[..] -- (1)
  //   %t2 = %t1    -- (2)
  // Starting from %t1, a load instruction (1) is found.
  // Example 2)
  //   %t1 = %a[..] -- (1)
  //   %t3 = %b[..] -- (2)
  //   %t2 = %t1 + %t3   -- (3)
  // Starting from  %t1 and %t3, loads (1) and (2) are found.
  // Example 3)
  //   %t1 = %liveIn0 < %liveIn1; --(1)
  //   %t2 = (%liveIn2 != 1) ? -1 : %t1; --(2)
  // From %t1 in (2) inst (1) is found.
  bool tracebackToLoad(const RegDDRef *Rval, DDGraph DDG,
                       SmallVectorImpl<const HLInst *> &Res) const;

  // Find the load instruction starting from SrcNode.
  // If SrcNode is a load, return it.
  // If it is a copy, trace back to a load and return it.
  bool findLoad(
      const HLDDNode *SrcNode, DDGraph DDG,
      SmallVectorImpl<std::pair<const HLInst *, const HLInst *>> &Res) const;

  template <typename IteratorTy>
  bool findLoadsOfTemp(
      DDGraph DDG, IteratorTy begin, IteratorTy end,
      unsigned AnchorNodeTopSortNum, InstsToCloneSetTy &LoadInsts,
      std::map<const HLInst *, const HLInst *> &CopyToLoadMap) const;

  bool collectLoadsToClone(
      const HLNode *AnchorNode, InstsToCloneSetTy &LoadInstsToClone,
      SmallVectorImpl<std::pair<unsigned, unsigned>> &CopyToLoadIndexMap) const;

  // LB(UB) of a By-strip loop is the "min"("max") of all lower bounds of
  // original spatial loop corresponding to the same DimNum.
  // This function collects all LBs(UBs) of original spatial loops, and
  // generate min(max) blobs of them.
  bool computeByStripLoopBounds(
      const DenseMap<unsigned, unsigned> &OrigToCloneIndexMap,
      SmallVectorImpl<const RegDDRef *> &AuxRefs);

  // Replace all use with new Lval
  void cloneAndAddLoadInsts(
      InstsToCloneSetTy &LoadInstsToClone, HLNode *AnchorNode,
      DenseMap<unsigned, unsigned> &OrigToCloneIndexMap,
      SmallVectorImpl<const RegDDRef *> &AuxRefsForByStripBounds);

  // Generate by-strip loops and insert before AnchorNode.
  // Returns the innermost by-strip loop, where spatial loops will be added.
  // Add ByStrip loops, and also compute UBs of unit-strided Loop
  //   e.g. DO IV = by_strip_lb, by_strip_ub, by_strip_step
  //          tile_end = min(IV + by_strip_step - 1, by_strip_ub)
  //   IV is the tile's begin.
  //   tile_end is the last element of tile, not the past the last.
  HLLoop *addByStripLoops(HLNode *AnchorNode,
                          const InstsToCloneSetTy &LoadInstsToClone,
                          const SmallVectorImpl<unsigned> &LiveOutsOfByStrip,
                          ArrayRef<const RegDDRef *> AuxRefsFromSpatialLoops);

  // IV update caused by stripmining.
  // Increase all IV levels greater than equal to LowestSpatialLoopLevel
  // by ByStripLoopDepth.
  // For example, if ByStripLoopDepth = 3, and LowestSpatialLoopLevel = 2
  // i1, i2, i3, i4 becomes
  // --> i1, i5, i6, i7
  void updateSpatialIVs(HLNode *Node, unsigned ByStripLoopDepth,
                        unsigned LowestLevel) const;

  std::pair<const RegDDRef *, unsigned>
  findAuxRefWithCE(const HLLoop *InnermostLoop, const CanonExpr *TargetCE);

  // Add blocking guards to loop bounds or as an if-stmt.
  // When index CEs are constant or blob, i.e., no loop exists corresponding to
  // that dimension, if-stmt is added.
  // Also, update live-in temps.
  void applyBlockingGuardsToSpatialLoops(
      const LoopToRefTy &InnermostLoopToAdjustingRef);

  static bool isNoBlockDim(unsigned DimNum, ArrayRef<unsigned> StripmineSizes) {
    return (DimNum > StripmineSizes.size()) || StripmineSizes[DimNum - 1] == 0;
  }

  // For a dimension, where blocking is not done for the corresponding loop,
  // stripmine size is set to zero.
  bool isNoBlockDim(unsigned DimNum) const {
    return isNoBlockDim(DimNum, StripmineSizes);
  }

  static void removeDupCanonExprs(SmallVectorImpl<CanonExpr *> &CEs) {
    std::sort(CEs.begin(), CEs.end(),
              [](const CanonExpr *R1, const CanonExpr *R2) {
                return CanonExprUtils::compare(R1, R2);
              });

    auto Last = std::unique(CEs.begin(), CEs.end(),
                            [](const CanonExpr *R1, const CanonExpr *R2) {
                              return CanonExprUtils::areEqual(R1, R2);
                            });

    CEs.erase(Last, CEs.end());
  }

  void calcLoopMatchingDimNum();

  // Return the loop matching DimNum.
  // InnermostLoop and DimInfos are data to consult with.
  void calcLoopMatchingDimNum(unsigned DimNum, ArrayRef<DimInfoTy> DimInfos,
                              const HLLoop *InnermostLoop);

  // Return the loop matching DimNum.
  // InnermostLoop and DimInfos are data to consult with.
  const HLLoop *getLoopMatchingDimNum(unsigned DimNum, unsigned DimInfoSize,
                                      const HLLoop *InnermostLoop) const;

  HLLoop *getLoopMatchingDimNum(unsigned DimNum, unsigned DimInfoSize,
                                HLLoop *InnermostLoop) {
    return const_cast<HLLoop *>(
        static_cast<const Transformer &>(*this).getLoopMatchingDimNum(
            DimNum, DimInfoSize, const_cast<HLLoop *>(InnermostLoop)));
  }

  // Sweep through all rvals by calling makeConsistent().
  // Added to take care of operands of createMin/Max
  // createMin/Max generates extra operands than passed arguments.
  // Temp blobs defAtLevels are all up to date already.
  void MakeConsistentRvals(HLInst *HInst) const {
    for (auto *Ref :
         make_range(HInst->op_ddref_begin(), HInst->op_ddref_end())) {
      Ref->makeConsistent({});
    }
  }

  // Add "if (ByStripIV <= Ref <= TileEndRef)"
  // Ref: dimension index, either constant or a blob.
  // ByStripIV: tile begin,
  // TileEndRef : last element of the tile.
  void addIfGuards(RegDDRef *Ref, const HLLoop *ByStripLoop,
                   HLNode *NodeToEnclose, int64_t Shift,
                   const RegDDRef *AuxRef = nullptr) const;

  // Add guards to the original spatial loop
  // DO i = LB', UB'
  // -->
  // Do i = max(LB', by-strip-loop's IV), min(UB', min(by-strip-loop's IV + step
  // - 1, by-strip-loop's UB)) TileEnd =
  // min(by-strip-loop's IV + step - 1,
  // by-strip-loop's UB) is already available as the first child of the
  // corresponding by-strip loop.
  // TODO: consider make it a lambda to its caller. It is used only in that
  // context
  unsigned addLoopBoundsGuards(HLLoop *Loop, unsigned DimNum,
                               int64_t Shift) const;

  // Given a CE, get a corresponding blob.
  static std::pair<BlobTy, unsigned>
  getConstantOrSingleBlob(const CanonExpr *CE) {
    BlobUtils &BU = CE->getBlobUtils();

    int64_t Val;
    if (CE->isIntConstant(&Val)) {
      return {BU.createBlob(Val, CE->getDestType()), InvalidBlobIndex};
    } else {
      return {BU.getBlob(CE->getSingleBlobIndex()), CE->getSingleBlobIndex()};
    }
  }

  // Given an array of CEs, get a min(max) blob of all CEs.
  template <bool IsMin>
  static std::pair<BlobTy, unsigned>
  getGlobalMinMaxBlob(ArrayRef<CanonExpr *> Bounds);

  // Given a ByStrip Loop, calculate the UB of the inner unit-strided loop
  //    min (IV + step - 1, UB)
  // where IV, step, and UB are induction var, loop step, and upperbound of
  // the ByStrip loop.
  // TODO: A candidate for a lambda function
  HLInst *createTileEnd(HLLoop *ByStrip) const;

  // Notice that ByStripLoops are not normalized.
  // Only the children loops of ByStripLoops are normalized.
  void normalizeSpatialLoops();

  // This function add ByStripLoop-definitions to NonDimMatching Loops,
  // if NonDimMatchingLoops exist.
  // Notice that for all spatial loops, i.e., DimMatchingLoops,
  // ByStripLoop-definitions are already added as LiveIns.
  // e.g)
  //
  //  + DO i1 = 0, // ByStrip loop
  //  |   %tile_e_min =  ...
  //  |
  //  |   // NonDimMatching loop <-- LiveIn info should have %tile_e_min!
  //  |   + DO i2 = 0,
  //  |   |      %lb_max = (0 <= i1) ? i1 : 0;
  //  |   |      %ub_min = ( ub <= %tile_e_min) ? ub : %tile_e_min;
  //  |   |
  //  |   |   // Spatial loop -- contains %tile, %lb_,%ub_ as live-in
  //  |   |    DO i3 = %lb_max, %ub_min
  //
  void addLiveInToNonDimMatchingLoops(HLLoop *OutermostNonDimMatchingLoop);

  bool init();
#if !defined(NDEBUG) || defined(LLVM_ENABLE_DEBUG)
  void dump();
#endif

private:
  // In the order of DimNum, [DimNum = 1][DimNum = 2] .. and so on.
  ArrayRef<unsigned> StripmineSizes;

  const LoopToDimInfoTy &InnermostLoopToDimInfos;
  const LoopToConstRefTy &InnermostLoopToRepRef;
  const InnermostLoopToShiftTy &InnermostLoopToShift;
  // Loop enclosing all the spatial loopnests.
  // Inside NodeOutsideByStrip, by-strip loops are generated.
  HLNode *NodeOutsideByStrip;

  SmallVector<std::pair<BlobTy, unsigned>, 4> ByStripLoopLowerBlobs;
  SmallVector<std::pair<BlobTy, unsigned>, 4> ByStripLoopUpperBlobs;

  // Newly generated by-strip loops
  SmallVector<HLLoop *, 4> ByStripLoops;
  SmallVector<unsigned, 4> LiveInsOfAllSpatialLoop;

  // Number of ByStrip loops. Could be different from StripmineSizes.size()
  // because StripmineSizes can contain zeros.
  unsigned NumByStripLoops;

  StringRef Func;

  // Related to Fortrans DVs.
  // Variables used in the LB/UB of loops, which are loaded after
  // the non-leading spatial loops, are cloned at the beginning of the body of
  // the outermost loop in order to be used in the LB/UB of by-strip loops.
  bool CloneDVLoads;

  // Represents there is a loop that won't be stripmined, but byStrip loops
  // of other loop will placed outside of this loop.
  // For example, i1-loop is a NonDimMatching loop. Notice that there is no
  // memref where i1 appears in an index. Also notice that i1-loop still
  // will be placed within the by-strip loop of i2-loop. If a NonDimMatching
  // loop exists, HasNonDimMatchingLoop is true. Otherwise, false.
  //
  // Before transformation:
  // DO i1 = 0, N
  //    DO i2 = 0, M
  //       A[i2] = A[i2] + ...
  //
  // After transformation
  // DO II = 0, M, S -- ByStripLoop of i2 loop
  //    DO i1 = 0, N
  //      DO i2 = II, min(M, II+S)
  //        A[i2] = A[i2] + ...
  //
  bool HasNonDimMatchingLoop;

  // A map from an innermost loop to its outer enclosing loops
  // matching to dimnum (includes the innermost loop).
  std::unordered_map<const HLLoop *, SmallVector<const HLLoop *, 4>>
      Innermost2TargetLoop;
};

} // namespace loopopt

} // namespace llvm

#endif /* LLVM_TRANSFORMS_INTEL_LOOPTRANSFORMS_HIRINTERLOOPBLOCK_H */
#endif // INTEL_FEATURE_SW_ADVANCED
