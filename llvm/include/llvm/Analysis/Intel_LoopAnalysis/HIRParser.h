//===-------- HIRParser.h - Parses SCEVs into CanonExprs ------*-- C++ --*-===//
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
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRPARSER_H
#define LLVM_ANALYSIS_INTEL_LOOPANALYSIS_HIRPARSER_H

#include "llvm/Pass.h"
#include "llvm/IR/Intel_LoopIR/CanonExpr.h"

#include "llvm/Analysis/Intel_LoopAnalysis/HIRCreation.h"

namespace llvm {

class Type;
class Function;
class SCEV;
class ScalarEvolution;
class SCEVConstant;
class SCEVUnknown;
class GEPOperator;
class Value;
class LoopInfo;

namespace loopopt {

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
class HIRParser : public FunctionPass {
private:
  /// Func - The function we are analyzing.
  Function *Func;

  /// HIR - HIR for the function.
  HIRCreation *HIR;

  /// LI - The loop information for the function we are currently analyzing.
  LoopInfo *LI;

  /// SE - Scalar Evolution analysis for the function.
  ScalarEvolution *SE;

  /// LF - Loop formation analysis of HIR.
  LoopFormation *LF;

  /// CurRegion - The region we are operating on.
  HLRegion *CurRegion;

  /// CurLevel - The loop level we are operating on.
  unsigned CurLevel;

  /// EraseSet - Contains HLNodes to be erased.
  SmallVector<HLNode *, 32> EraseSet;

  /// \brief Visits HIR and calls HIRParser utilities.
  struct Visitor {
    HIRParser *HIRP;

    Visitor(HIRParser *Parser) : HIRP(Parser) {}

    void visit(HLRegion *Reg) { HIRP->parse(Reg); }
    void postVisit(HLRegion *Reg) { HIRP->postParse(Reg); }

    void visit(HLLoop *HLoop) { HIRP->parse(HLoop); }
    void postVisit(HLLoop *HLoop) { HIRP->postParse(HLoop); }

    void visit(HLIf *If) { HIRP->parse(If); }
    void postVisit(HLIf *If) { HIRP->postParse(If); }

    void visit(HLSwitch *Switch) { HIRP->parse(Switch); }
    void postVisit(HLSwitch *Switch) { HIRP->postParse(Switch); }

    void visit(HLInst *HInst) { HIRP->parse(HInst); }
    void visit(HLLabel *Label) { HIRP->parse(Label); }
    void visit(HLGoto *Goto) { HIRP->parse(Goto); }

    bool isDone() { return false; }
  };

  /// Main parser functions
  void parse(HLRegion *Reg) { CurRegion = Reg; }
  void postParse(HLRegion *Reg) {}

  void parse(HLLoop *HLoop);
  void postParse(HLLoop *HLoop) { CurLevel--; }

  void parse(HLIf *If) { assert(false && "If not handled yet!"); }
  void postParse(HLIf *If) {}

  void parse(HLSwitch *Switch) { assert(false && "Switch not handled yet!"); }
  void postParse(HLSwitch *Switch) {}

  void parse(HLInst *HInst);
  void parse(HLLabel *Label) {}
  void parse(HLGoto *Goto) {}

  /// \brief Returns the integer constant contained in ConstSCEV.
  int64_t getSCEVConstantValue(const SCEVConstant *ConstSCEV) const;

  /// \brief Parses a SCEVConstant expr into CE.
  void parseConstant(const SCEVConstant *ConstSCEV, CanonExpr *CE);

  /// \brief Parses a SCEVUnknown expr into CE.
  void parseBlob(const SCEV *BlobSCEV, CanonExpr *CE, unsigned CurLevel);

  /// \brief Parses the passed in SCEV into the CanonExpr CE.
  RegDDRef *parseRecursive(const SCEV *SC, const SCEV *ElementSize,
                           unsigned CurLevel, bool IsErasable,
                           bool IsTop = true);

  /// \brief Returns a RegDDRef containing GEPInfo.
  RegDDRef *createGEPRegDDRef(const SCEV *SC, const SCEV *ElementSize,
                              const GEPOperator *GEPOp, unsigned Level,
                              bool IsErasable);

  /// \brief Returns an rval DDRef created from Val.
  RegDDRef *createRvalDDRef(const Value *Val, unsigned Level);

  /// \brief Returns true if the Value is region live out.
  bool isRegionLiveOut(const Value *Val, bool IsCompare = false);

  /// \brief Erases HLNodes which are deemed useless by the parser.
  void eraseUselessNodes();

public:
  static char ID; // Pass identification
  HIRParser();

  bool runOnFunction(Function &F) override;
  void releaseMemory() override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
  void print(raw_ostream &OS, const Module * = nullptr) const override;
  void verifyAnalysis() const override;

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

  /// \brief Returns the index of Blob in the blob table. Index range is [1,
  /// UINT_MAX]. Returns 0
  /// if the blob is not present in the table.
  unsigned findBlob(CanonExpr::BlobTy Blob);
  /// \brief Returns the index of Blob in the blob table. Blob is first
  /// inserted, if it isn't
  /// already present in the blob table. Index range is [1, UINT_MAX].
  unsigned findOrInsertBlob(CanonExpr::BlobTy Blob);

  /// \brief Returns blob corresponding to BlobIndex.
  CanonExpr::BlobTy getBlob(unsigned BlobIndex);

  /// \brief Checks if the blob is constant or not
  /// If blob is constant, sets the return value in Val
  bool isConstIntBlob(CanonExpr::BlobTy Blob, int64_t *Val);

  /// \brief Returns a new blob created from a constant value.
  CanonExpr::BlobTy createBlob(int64_t Val, bool Insert = true,
                               unsigned *NewBlobIndex = nullptr);

  /// \brief Returns a blob which represents (LHS + RHS). If Insert is true its
  /// index is returned
  /// via NewBlobIndex argument.
  CanonExpr::BlobTy createAddBlob(CanonExpr::BlobTy LHS, CanonExpr::BlobTy RHS,
                                  bool Insert = true,
                                  unsigned *NewBlobIndex = nullptr);

  /// \brief Returns a blob which represents (LHS - RHS). If Insert is true its
  /// index is returned
  /// via NewBlobIndex argument.
  CanonExpr::BlobTy createMinusBlob(CanonExpr::BlobTy LHS,
                                    CanonExpr::BlobTy RHS, bool Insert = true,
                                    unsigned *NewBlobIndex = nullptr);
  /// \brief Returns a blob which represents (LHS * RHS). If Insert is true its
  /// index is returned
  /// via NewBlobIndex argument.
  CanonExpr::BlobTy createMulBlob(CanonExpr::BlobTy LHS, CanonExpr::BlobTy RHS,
                                  bool Insert = true,
                                  unsigned *NewBlobIndex = nullptr);
  /// \brief Returns a blob which represents (LHS / RHS). If Insert is true its
  /// index is returned
  /// via NewBlobIndex argument.
  CanonExpr::BlobTy createUDivBlob(CanonExpr::BlobTy LHS, CanonExpr::BlobTy RHS,
                                   bool Insert = true,
                                   unsigned *NewBlobIndex = nullptr);
  /// \brief Returns a blob which represents (trunc Blob to Ty). If Insert is
  /// true its index is
  /// returned via NewBlobIndex argument.
  CanonExpr::BlobTy createTruncateBlob(CanonExpr::BlobTy Blob, Type *Ty,
                                       bool Insert = true,
                                       unsigned *NewBlobIndex = nullptr);
  /// \brief Returns a blob which represents (zext Blob to Ty). If Insert is
  /// true its index is
  /// returned via NewBlobIndex argument.
  CanonExpr::BlobTy createZeroExtendBlob(CanonExpr::BlobTy Blob, Type *Ty,
                                         bool Insert = true,
                                         unsigned *NewBlobIndex = nullptr);
  /// \brief Returns a blob which represents (sext Blob to Ty). If Insert is
  /// true its index is
  /// returned via NewBlobIndex argument.
  CanonExpr::BlobTy createSignExtendBlob(CanonExpr::BlobTy Blob, Type *Ty,
                                         bool Insert = true,
                                         unsigned *NewBlobIndex = nullptr);

  /// TODO handle min/max blobs.
};

} // End namespace loopopt

} // End namespace llvm

#endif
