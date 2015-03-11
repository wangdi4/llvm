//===------- HIRPrint.cpp - Pretty print for HIR  ---------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is for Pretty Print HIR.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Support/Debug.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Operator.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/ScalarEvolutionExpander.h"

using namespace llvm;
using namespace loopopt;

static const int BASE = 1;
static const int STRIDE = 2;
static const int SUBSCRIPT = 3;

struct HLNodePrinter {

  int Indent;
  int loopLevel;
  raw_ostream &OS = dbgs();
  void visit(const HLRegion *Region);
  void postVisit(const HLRegion *Region);
  void visit(const HLLoop *Loop);
  void postVisit(const HLLoop *Loop);
  void visit(const HLInst *Inst);
  void visit(const HLLabel *Label);
  void visit(const HLGoto *Goto);
  void visit(const HLSwitch *Switch);
  void visit(const HLIf *If){};
  void postVisit(const HLIf *If){};
  bool isDone() const { return false; }
  HLNodePrinter() : Indent(0) {}

  void printIndentSpaces() {
    for (int i = 0; i < Indent; i++) {
      OS << " ";
    }
  }

  void printIndentLoop(bool for_loop = true) {
    int count = (for_loop ? loopLevel - 1 : loopLevel);
    for (int i = 0; i < count; i++) {
      OS << "|   ";
    }
  }

  SCEV *getBlobSCEV(int BlobIdx) {
    //  TBC waiting for blob table
    // return const_cast<SCEV *>(Blobs[BlobIdx]);
    return nullptr;
  }

  void printCanon(CanonExpr *CE, int type);
  void printCanonExpr(CanonExpr *CE);
  void printRegDDRef(RegDDRef *Ref, const HLInst *Inst);
  void printDDRefInst(DDRef *Ref, const HLInst *Inst);
  void printDDRef(DDRef *Ref);
};

void HLNodePrinter::printCanon(CanonExpr *CE, int type) {

  int level = 1;
  bool printed = false;

  for (auto CurIVPair = CE->iv_cbegin(), E = CE->iv_cend(); CurIVPair != E;
       CurIVPair++, level++) {
    if (CurIVPair->Coeff != 0) {
      if (CurIVPair->Coeff != 1)
        OS << CurIVPair->Coeff << "*"
           << "i" << level;
      else
        OS << "i" << level;

      if (printed)
        OS << "+";
      else
        printed = true;
    }
  }
  int C0 = CE->getConstant();
  if (C0 != 0) {
    if (printed)
      OS << "+" << C0;
    else
      OS << C0;
  }

  if (CE->hasBlob()) {

    for (auto BlobPair = CE->blob_cbegin(), E = CE->blob_cend(); BlobPair != E;
         BlobPair++) {

      if (BlobPair->Coeff != 0) {
        if (BlobPair->Coeff != 1) {
          OS << BlobPair->Coeff << "*";
          OS << getBlobSCEV(BlobPair->Index);
        }
        OS << getBlobSCEV(BlobPair->Index);
        if (printed)
          OS << "+";
        else
          printed = true;
      }
    }
  }
}

void HLNodePrinter::printCanonExpr(CanonExpr *CE) {

  int64_t x = 0;
  x = CE->getConstant();
  OS << x;
}

void HLNodePrinter::printRegDDRef(RegDDRef *Ref, const HLInst *Inst) {

  Instruction *I = const_cast<Instruction *>(Inst->getLLVMInstruction());

  StoreInst *storeI = dyn_cast<StoreInst>(I);

  // TBC: get baseCE, then blobidx to get blob name
  // waiting for real Blob table to be created

  if (Ref->hasGEPInfo()) {

    printCanon(Ref->getBaseCE(), BASE);

    OS << "[";
  }

  // TBC:  not right now..

  auto R = Inst->ddref_begin();
  R++;

  if (RegDDRef *RegRef = dyn_cast<RegDDRef>(*R)) {
    bool afterFirstSubs = false;
    for (auto CE = RegRef->canon_begin(), E = RegRef->canon_end(); CE != E;
         CE++) {
      if (afterFirstSubs) {
        OS << "[";
      }
      printCanon(*CE, SUBSCRIPT);
      afterFirstSubs = true;
    }
  }

  if (Ref->hasGEPInfo()) {
    OS << "]";
  }
}

void HLNodePrinter::printDDRefInst(DDRef *Ref, const HLInst *Inst) {

  if (ConstDDRef *CRef = dyn_cast<ConstDDRef>(Ref)) {
    printCanonExpr(CRef->getCanonExpr());
  } else if (BlobDDRef *BRef = dyn_cast<BlobDDRef>(Ref)) {
    printCanonExpr(BRef->getCanonExpr());
  } else if (RegDDRef *RegRef = dyn_cast<RegDDRef>(Ref)) {
    printRegDDRef(RegRef, Inst);
  }
}

void HLNodePrinter::printDDRef(DDRef *Ref) {

  if (ConstDDRef *CRef = dyn_cast<ConstDDRef>(Ref)) {
    printCanonExpr(CRef->getCanonExpr());
  } else if (BlobDDRef *BRef = dyn_cast<BlobDDRef>(Ref)) {
    printCanonExpr(BRef->getCanonExpr());
  } else if (RegDDRef *RegRef = dyn_cast<RegDDRef>(Ref)) {
    for (auto CE = RegRef->canon_begin(), E = RegRef->canon_end(); CE != E;
         CE++) {
      printCanon(*CE, SUBSCRIPT);
    }
  }
}

void HLNodePrinter::visit(const HLRegion *Region) {
  OS << "BEGIN_REGION\n";
  Indent = 0;
}

void HLNodePrinter::postVisit(const HLRegion *Region) {
  OS << "END_REGION\n";
  Indent = 0;
}

void HLNodePrinter::postVisit(const HLLoop *Loop) {
  printIndentLoop();
  printIndentSpaces();
  OS << "+ END_LOOP\n";
  Indent -= 3;
  loopLevel -= 1;
}

void HLNodePrinter::visit(const HLLabel *Label) { OS << "Label?\n"; }

void HLNodePrinter::visit(const HLGoto *Goto) { OS << "goto?\n"; }

void HLNodePrinter::visit(const HLSwitch *Switch) { OS << "switch?\n"; }

void HLNodePrinter::visit(const HLLoop *Loop) {

  DDRef *LB = const_cast<DDRef *>(Loop->getLowerDDRef());
  DDRef *Tripcnt = const_cast<DDRef *>(Loop->getTripCountDDRef());
  DDRef *Stride = const_cast<DDRef *>(Loop->getStrideDDRef());

  int level = Loop->getNestingLevel();
  loopLevel = level;

  printIndentLoop();

  Indent += 3;

  printIndentSpaces();

  OS << "+ LOOP i" << level << "=";

  printDDRef(LB);
  OS << ",";
  printDDRef(Tripcnt);
  OS << ",";
  printDDRef(Stride);
  OS << " // lb,tripCnt,step\n";
}

void HLNodePrinter::visit(const HLInst *Inst) {

  Instruction *I = const_cast<Instruction *>(Inst->getLLVMInstruction());

  SmallVector<DDRef *, 5> ddref;
  int i = 0;

  for (auto R = Inst->ddref_begin(), E = Inst->ddref_end(); R != E; i++, R++) {
    ddref.push_back(*R);
  }

  printIndentSpaces();
  printIndentLoop(false);
  OS << "  ";

  if (isa<StoreInst>(*I)) {
    printDDRefInst(ddref[1], Inst);
    OS << " = ";
    printDDRefInst(ddref[0], Inst);
  } else if (isa<LoadInst>(*I)) {
    printDDRefInst(ddref[0], Inst);
    OS << " = ";
    printDDRefInst(ddref[1], Inst);
  } else {
    i = 0;
    for (auto R = Inst->ddref_begin(), E = Inst->ddref_end(); R != E;
         i++, R++) {
      if (i > 1) {
        OS << " , ";
      }
      printDDRefInst(ddref[i], Inst);
      if (i == 0) {
        OS << " = " << I->getOpcodeName() << " ";
      }
    }
  }

  OS << " //" << *I << "\n";
}

void HLNode::print() const {

  HLNodePrinter HIRP;

  if (isa<HLRegion>(this)) {
    auto Reg = cast<HLRegion>(this);
    HLNodeUtils::visit<HLNodePrinter>(&HIRP, const_cast<HLRegion *>(Reg));
  } else if (isa<HLLoop>(this)) {
    auto L = cast<HLLoop>(this);
    HLNodeUtils::visit<HLNodePrinter>(&HIRP, const_cast<HLLoop *>(L));
  } else if (isa<HLInst>(this)) {
    auto Inst = cast<HLInst>(this);
    HLNodeUtils::visit<HLNodePrinter>(&HIRP, const_cast<HLInst *>(Inst));
  }
}
