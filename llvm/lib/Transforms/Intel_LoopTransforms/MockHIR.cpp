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

void MockHIR::createMockHIRSimpleLoop() {

  Function::iterator curBlock = F->begin();
  BasicBlock *EntryBlock = curBlock;
  BasicBlock *LoopBlock = ++curBlock;
  BasicBlock *ExitBlock = ++curBlock;

  std::set<BasicBlock *> OrigBBs;

  for (Function::iterator i = F->begin(), e = F->end(); i != e; ++i) {
    OrigBBs.insert(i);
  }

  // set up basic region
  HLRegion *Region =
      HLNodeUtils::createHLRegion(OrigBBs, EntryBlock, ExitBlock);

  TopRegion = Region;
  HLRegions.clear();
  HLRegions.push_back(Region);
  // not today
  // set up ztt if
  HLIf *Ztt = nullptr;
  // HLNodeUtils::createHLIf(Region);
  // HLNodeUtils::setSimpleLoopZtt(Ztt, PrehdrBlock);

  // set up loop
  HLLoop *Loop = HLNodeUtils::createHLLoop(Ztt, false, 1);
  // set loop bounds, znorm
  Type *Int64Type = IntegerType::get(getGlobalContext(), 64);
  CanonExpr *LBCE = CanonExprUtils::createCanonExpr(Int64Type, true, 1, 0, 1);
  DDRef *LBRef = DDRefUtils::createConstDDRef(LBCE, Loop);

  CanonExpr *TCCE = CanonExprUtils::createCanonExpr(Int64Type, true, 1, 4, 1);
  DDRef *TCRef = DDRefUtils::createConstDDRef(TCCE, Loop);

  CanonExpr *StrideCE =
      CanonExprUtils::createCanonExpr(Int64Type, true, 1, 1, 1);
  DDRef *StrideRef = DDRefUtils::createConstDDRef(StrideCE, Loop);

  HLNodeUtils::dbgPushDDRef(Loop, LBRef);
  HLNodeUtils::dbgPushDDRef(Loop, TCRef);
  HLNodeUtils::dbgPushDDRef(Loop, StrideRef);

  // regions child is loop
  ///HLNodeUtils::dbgPushBackChild(Region, Loop);
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

  HLNode *InstNode = HLNodeUtils::createHLInst(StoreI);

 // HLNodeUtils::dbgPushBackChild(Loop, InstNode);

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

  RegDDRef *StoreRef = DDRefUtils::createRegDDRef(2, InstNode);
  RegDDRef *LoadRef = DDRefUtils::createRegDDRef(4, InstNode);
  // store loaded_val, store_loc
  HLNodeUtils::dbgPushDDRef(InstNode, LoadRef);
  HLNodeUtils::dbgPushDDRef(InstNode, StoreRef);

  // load ptr is dst scev and blob idx 0, store is src and 1
  MockBlobTable.push_back(DstPtrSCEV);
  MockBlobTable.push_back(SrcPtrSCEV);
  // create blob refs
  CanonExpr *StoreBlobCE = CanonExprUtils::createCanonExpr(
      SrcGEP->getPointerOperandType(), true, 0, 0, 1);
  StoreBlobCE->addBlob(1, 1);
  BlobDDRef *StoreBlobRef =
      DDRefUtils::createBlobDDRef(1, StoreBlobCE, StoreRef);
  DDRefUtils::dbgPushBlobDDRef(StoreRef, StoreBlobRef);

  CanonExpr *LoadBlobCE = CanonExprUtils::createCanonExpr(
      DstGEP->getPointerOperandType(), true, 0, 0, 1);
  LoadBlobCE->addBlob(0, 1);
  BlobDDRef *LoadBlobRef = DDRefUtils::createBlobDDRef(3, LoadBlobCE, LoadRef);
  DDRefUtils::dbgPushBlobDDRef(LoadRef, LoadBlobRef);

  // finish reg ddref
  // Canon expr for stride
  // 1 + i_0*1, 0 norm in subscript form(ie not byte addressed)
  CanonExpr *LoadLinearCE =
      CanonExprUtils::createCanonExpr(Int64Type, true, 1, 1, 1);
  LoadLinearCE->addIV(1, 1);
  RegDDRef::StrideTy Strides;
  Strides.push_back(CanonExprUtils::createCanonExpr(Int64Type, true, 0, 0, 1));
  Strides.push_back(LoadLinearCE);
  DDRefUtils::setGEP(LoadRef, LoadBlobCE, Strides, true);

  CanonExpr *StoreLinearCE =
      CanonExprUtils::createCanonExpr(Int64Type, true, 1, 1, 1);
  StoreLinearCE->addIV(1, 1);
  RegDDRef::StrideTy Strides2;
  Strides2.push_back(CanonExprUtils::createCanonExpr(Int64Type, true, 0, 0, 1));
  Strides2.push_back(StoreLinearCE);
  DDRefUtils::setGEP(StoreRef, StoreBlobCE, Strides2, true);
  // Set refs into hlinst

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
