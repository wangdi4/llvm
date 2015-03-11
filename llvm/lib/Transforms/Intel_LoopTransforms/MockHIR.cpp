#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/Intel_LoopTransforms/MockHIR.h"

#include "llvm/Transforms/Intel_LoopTransforms/Utils/HLNodeUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/CanonExprUtils.h"
#include "llvm/Transforms/Intel_LoopTransforms/Utils/DDRefUtils.h"

#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/Support/Debug.h"

#include "llvm/IR/LLVMContext.h"
#define DEBUG_TYPE "mhir"

using namespace llvm;
using namespace llvm::loopopt;

char MockHIR::ID = 0;
static RegisterPass<MockHIR> X("MockHIR", "Mock HIR Construction", false, true);

static std::set<BasicBlock *> OrigBBs;

void MockHIR::createMockHIRSimpleLoop() {

  auto curBlock = F->begin();
  BasicBlock *LoopBlock = ++curBlock;

  OrigBBs.insert(LoopBlock);

  // set up basic region
  HLRegion *Region = HLNodeUtils::createHLRegion(OrigBBs, LoopBlock, LoopBlock);

  TopRegion = Region;
  HLRegions.clear();
  HLRegions.push_back(Region);
  // not today
  // set up ztt if
  HLIf *Ztt = nullptr;
  // HLNodeUtils::createHLIf(Region);
  // HLNodeUtils::setSimpleLoopZtt(Ztt, PrehdrBlock);

  // set up loop
  HLLoop *Loop = HLNodeUtils::createHLLoop(Ztt);
  // set loop bounds, znorm
  Type *Int64Type = IntegerType::get(getGlobalContext(), 64);
  CanonExpr *LBCE = CanonExprUtils::createCanonExpr(Int64Type, true, 1, 0, 1);
  DDRef *LBRef = DDRefUtils::createConstDDRef(LBCE);

  CanonExpr *TCCE = CanonExprUtils::createCanonExpr(Int64Type, true, 1, 4, 1);
  DDRef *TCRef = DDRefUtils::createConstDDRef(TCCE);

  CanonExpr *StrideCE =
      CanonExprUtils::createCanonExpr(Int64Type, true, 1, 1, 1);
  DDRef *StrideRef = DDRefUtils::createConstDDRef(StrideCE);

  Loop->setLowerDDRef(LBRef);
  Loop->setTripCountDDRef(TCRef);
  Loop->setStrideDDRef(StrideRef);

  // regions child is loop
  HLNodeUtils::insertAsFirstChild(Region, Loop);

  // set up instruction(s) inside
  BasicBlock::iterator CurInst = ++LoopBlock->begin();

  Instruction *LoadGEP = CurInst;
  Instruction *LoadI = ++CurInst;
  Instruction *StoreGEP = ++CurInst;
  Instruction *StoreI = ++CurInst;

  DEBUG(errs() << "The Load GEP is " << *LoadGEP << "\n");
  DEBUG(errs() << "The Load is " << *LoadI << "\n");
  DEBUG(errs() << "The Store GEP is " << *StoreGEP << "\n");
  DEBUG(errs() << "The Store is " << *StoreI << "\n");

  // set up inst refs
  Value *LoadPtr, *StorePtr;
  if (LoadInst *I = dyn_cast<LoadInst>(LoadI)) {
    LoadPtr = I->getPointerOperand();
  }
  if (StoreInst *I = dyn_cast<StoreInst>(StoreI)) {
    StorePtr = I->getPointerOperand();
  }

  HLInst *InstNode = HLNodeUtils::createHLInst(StoreI);

  HLNodeUtils::insertAsFirstChild(Loop, InstNode);

  GEPOperator *SrcGEP = dyn_cast<GEPOperator>(StorePtr);
  GEPOperator *DstGEP = dyn_cast<GEPOperator>(LoadPtr);
  const SCEV *SrcPtrSCEV;
  const SCEV *DstPtrSCEV;
  if (SrcGEP && DstGEP &&
      SrcGEP->getPointerOperandType() == DstGEP->getPointerOperandType()) {
    SrcPtrSCEV = SE->getSCEV(SrcGEP->getPointerOperand());
    DstPtrSCEV = SE->getSCEV(DstGEP->getPointerOperand());

    DEBUG(errs() << "The load scev is " << *DstPtrSCEV << "\n");
    DEBUG(errs() << "The store scev is " << *SrcPtrSCEV << " with type "
                 << *(SrcGEP->getPointerOperandType()) << " \n");
  }

  RegDDRef *StoreRef = DDRefUtils::createRegDDRef(2);
  RegDDRef *LoadRef = DDRefUtils::createRegDDRef(4);
  // store loaded_val, store_loc
  InstNode->setOperandDDRef(LoadRef, 1);
  InstNode->setLvalDDRef(StoreRef);

  // load ptr is dst scev and blob idx 0, store is src and 1
  MockBlobTable.push_back(DstPtrSCEV);
  MockBlobTable.push_back(SrcPtrSCEV);
  // create blob refs
  // Blob ddrefs aren't required for now, commenting out
  // BlobDDRef *StoreBlobRef =
  //    DDRefUtils::createBlobDDRef(1, StoreBlobCE, StoreRef);
  // DDRefUtils::dbgPushBlobDDRef(StoreRef, StoreBlobRef);
  // BlobDDRef *LoadBlobRef = DDRefUtils::createBlobDDRef(3, LoadBlobCE,
  // LoadRef);
  // DDRefUtils::dbgPushBlobDDRef(LoadRef, LoadBlobRef);

  CanonExpr *LoadBlobCE =
      CanonExprUtils::createCanonExpr(DstGEP->getPointerOperandType());
  LoadBlobCE->addBlob(0, 1);
  // finish reg ddref
  // Canon expr for stride
  // 1 + i_0*1, 0 norm in subscript form(ie not byte addressed)
  CanonExpr *LoadLinearCE = CanonExprUtils::createCanonExpr(Int64Type);
  LoadLinearCE->addIV(1, 1);
  /// Setting null stride for the first dimension, revisit later
  LoadRef->addDimension(LoadLinearCE, nullptr);
  LoadRef->setBaseCE(LoadBlobCE);
  LoadRef->setInBounds(DstGEP->isInBounds());

  CanonExpr *StoreLinearCE = CanonExprUtils::createCanonExpr(Int64Type);
  StoreLinearCE->addIV(1, 1);
  CanonExpr *StoreBlobCE =
      CanonExprUtils::createCanonExpr(SrcGEP->getPointerOperandType());
  StoreBlobCE->addBlob(1, 1);
  /// Setting null stride for the first dimension, revisit later
  StoreRef->addDimension(StoreLinearCE, nullptr);
  StoreRef->setBaseCE(StoreBlobCE);
  StoreRef->setInBounds(SrcGEP->isInBounds());

  // SE->dump();
}

/*
lookinig for a function like this
pretty much for(..) {A[i] = B[i]}
*** IR Dump After Loop-Closed SSA Form Pass ***
; Function Attrs: nounwind uwtable
define i32 @_Z3foov() #0 {
entry:
  br label %for.body

for.body:                                         ; preds = %entry, %for.body
  %i.05 = phi i64 [ 1, %entry ], [ %inc, %for.body ]
  %arrayidx = getelementptr inbounds [10 x i32]* @B, i64 0, i64 %i.05
  %0 = load i32* %arrayidx, align 4, !tbaa !1
  %arrayidx1 = getelementptr inbounds [10 x i32]* @A, i64 0, i64 %i.05
  store i32 %0, i32* %arrayidx1, align 4, !tbaa !1
  %inc = add nsw i64 %i.05, 1
  %cmp = icmp slt i64 %inc, 5
  br i1 %cmp, label %for.body, label %for.end

for.end:                                          ; preds = %for.body
  %1 = load i32* getelementptr inbounds ([10 x i32]* @A, i64 0, i64 2), align 8,
!tbaa !1
  ret i32 %1
}
*/
bool MockHIR::functionMatchesSimpleLoop() {
  int numBlocks = 0;

  for (Function::iterator i = F->begin(), e = F->end(); i != e; ++i) {
    errs() << "Basic block (name=" << i->getName() << ") has " << i->size()
           << " instructions.\n";
    numBlocks++;
  }
  if (numBlocks != 3) {
    DEBUG(errs() << "Incorrect number of blocks " << numBlocks << "\n");
    return false;
  }

  BasicBlock *first = (F->begin());
  BasicBlock *second = ++(F->begin());
  BasicBlock *third = ++(++(F->begin()));
  // first block has only branch
  if (first->size() != 1) {
    DEBUG(errs() << "first block did not meet criteria " << first->size()
                 << "\n");
    return false;
  }

  // second block is pre header
  if (second->size() != 8) {
    DEBUG(errs() << "second block did not meet criteria " << second->size()
                 << "\n");
    return false;
  }

  // third block is body
  if (third->size() != 2) {
    DEBUG(errs() << "third block did not meet criteria " << third->size()
                 << "\n");
    return false;
  }

  DEBUG(errs() << "function matched simple loop\n");
  return true;
}

FunctionPass *llvm::createMockHIRPass() { return new MockHIR(); }
