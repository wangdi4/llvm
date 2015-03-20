//===------- HIRPrint.cpp - Pretty print for HIR  ---------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file is for Pretty Printing HIR.
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
using namespace llvm::loopopt;

static const int BASE = 1;
static const int STRIDE = 2;
static const int SUBSCRIPT = 3;

namespace llvm {

namespace loopopt {

class HLNodePrinter {

  int Indent;
  int LoopLevel;
  raw_ostream &OS = dbgs();

public:
  void visit(const HLRegion *Region);
  void postVisit(const HLRegion *Region);
  void visit(const HLLoop *Loop);
  void postVisit(const HLLoop *Loop);
  void visit(const HLInst *Inst);
  void visit(const HLLabel *Label);
  void visit(const HLGoto *Goto);
  void visit(const HLSwitch *Switch);
  void visit(const HLIf *If);
  void postVisit(const HLIf *If);
  bool isDone() const { return false; }
  HLNodePrinter() : Indent(0), LoopLevel(0) {}

  void printIndentSpaces() {
    for (int i = 0; i < Indent; i++) {
      OS << " ";
    }
  }

  void printIndentLoop(bool for_loop = true) {
    int count = (for_loop ? LoopLevel - 1 : LoopLevel);
    for (int i = 0; i < count; i++) {
      OS << "|   ";
    }
  }

  void printCanonExpr(CanonExpr *CE, int type = BASE);
  void printRegDDRef(RegDDRef *Ref);
  void printDDRef(DDRef *Ref);
  // void printDDRef(DDRef *Ref);
};

} // End namespace loopopt

} // End namespace llvm

void HLNodePrinter::printCanonExpr(CanonExpr *CE, int type) {

  int level = 1;
  bool printed = false;
  if (CE->hasIV()) {
    for (auto CurIVPair = CE->iv_cbegin(), E = CE->iv_cend(); CurIVPair != E;
         CurIVPair++, level++) {
      if (CurIVPair->Coeff != 0) {
        if (printed) {
          OS << "+";
        } else {
          printed = true;
        }

        if (CurIVPair->IsBlobCoeff) {
          CanonExprUtils::getBlob(CurIVPair->Coeff)->print(OS);
          OS << " * ";
        } else if (CurIVPair->Coeff != 1) {
          OS << CurIVPair->Coeff << "*";
        }
        OS << "i" << level;
      }
    }
  }

  if (CE->hasBlob()) {
    for (auto BlobPair = CE->blob_cbegin(), E = CE->blob_cend(); BlobPair != E;
         BlobPair++) {

      if (BlobPair->Coeff != 0) {
        if (printed) {
          OS << "+";
        } else {
          printed = true;
        }

        if (BlobPair->Coeff != 1) {
          OS << BlobPair->Coeff << "*";
        }

        CanonExprUtils::getBlob(BlobPair->Index)->print(OS);
      }
    }
  }

  int C0 = CE->getConstant();
  if (C0 != 0 || !printed) {
    if (printed)
      OS << "+" << C0;
    else
      OS << C0;
  }
}

void HLNodePrinter::printRegDDRef(RegDDRef *RegRef) {

  // TBC: get baseCE, then blobidx to get blob name
  // Instruction *I =  const_cast<Instruction *>(Inst->getLLVMInstruction());
  // StoreInst *storeI = dyn_cast<StoreInst>(I);
  // waiting for real Blob table to be created

  if (RegRef->hasGEPInfo()) {
    printCanonExpr(RegRef->getBaseCE(), BASE);
    OS << "[";
  }

  // TBC:  not right now..

  bool afterFirstSubs = false;
  for (auto CE = RegRef->canon_begin(), E = RegRef->canon_end(); CE != E;
       CE++) {
    if (afterFirstSubs) {
      OS << "[";
    }
    printCanonExpr(*CE, SUBSCRIPT);
    afterFirstSubs = true;
  }

  if (RegRef->hasGEPInfo()) {
    OS << "]";
  }
}

void HLNodePrinter::printDDRef(DDRef *Ref) {

  if (ConstDDRef *CRef = dyn_cast<ConstDDRef>(Ref)) {
    printCanonExpr(CRef->getCanonExpr());
  } else if (BlobDDRef *BRef = dyn_cast<BlobDDRef>(Ref)) {
    printCanonExpr(BRef->getCanonExpr());
  } else if (RegDDRef *RegRef = dyn_cast<RegDDRef>(Ref)) {
    printRegDDRef(RegRef);
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
  LoopLevel -= 1;
}

void HLNodePrinter::visit(const HLLabel *Label) {
  OS << "Label print not done!\n";
}

void HLNodePrinter::visit(const HLGoto *Goto) { OS << "goto print not done\n"; }

void HLNodePrinter::visit(const HLSwitch *Switch) {
  OS << "switch print not done\n";
}

void HLNodePrinter::visit(const HLIf *If) { OS << "if  print not done\n"; }
void HLNodePrinter::postVisit(const HLIf *If) {
  OS << "postif  print not done\n";
}

void HLNodePrinter::visit(const HLLoop *Loop) {

  DDRef *LB = const_cast<DDRef *>(Loop->getLowerDDRef());
  DDRef *Tripcnt = const_cast<DDRef *>(Loop->getTripCountDDRef());
  DDRef *Stride = const_cast<DDRef *>(Loop->getStrideDDRef());

  int level = Loop->getNestingLevel();
  LoopLevel = level;

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

void HLNodePrinter::visit(const HLInst *HInst) {

  Instruction *Inst = const_cast<Instruction *>(HInst->getLLVMInstruction());
  int Count = 0;

  printIndentSpaces();

  if (LoopLevel > 0)
    printIndentLoop(false);

  OS << "  ";

  /// TODO: Add beautification logic based on instruction types
  for (auto I = HInst->ddref_begin(), E = HInst->ddref_end(); I != E;
       I++, Count++) {

    if (Count > 1) {
      OS << " , ";
    }

    if (Count == 0) {
      if (HInst->hasLval()) {
        printDDRef(*I);

        OS << " = ";

        if (!isa<LoadInst>(Inst) && !isa<StoreInst>(Inst)) {
          OS << Inst->getOpcodeName() << " ";
        }
      } else {
        OS << Inst->getOpcodeName() << " ";
        printDDRef(*I);
      }
    } else {
      printDDRef(*I);
    }
  }

  OS << " //" << *Inst << "\n";
}

void HLNode::print() const {

  HLNodePrinter HIRP;
  HLNodeUtils::visit<HLNodePrinter>(&HIRP, const_cast<HLNode *>(this));
}

void DDRef::print() const {

  HLNodePrinter HIRP;
  HIRP.printDDRef(const_cast<DDRef *>(this));
}

void CanonExpr::print() const {

  HLNodePrinter HIRP;
  HIRP.printCanonExpr(const_cast<CanonExpr *>(this));
}
