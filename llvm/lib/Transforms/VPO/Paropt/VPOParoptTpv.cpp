#if INTEL_COLLAB
//===------- VPOParoptTpv.cpp - Paropt Threadprivate Transformations ------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
/// \file
/// This file implements the threadprivate support for OpenMP
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/VPO/Paropt/VPOParoptTpv.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/IR/MDBuilder.h"
#include "llvm/InitializePasses.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/VPO/VPOPasses.h"
#include "llvm/Transforms/VPO/Utils/VPOUtils.h"
#include "llvm/Transforms/VPO/Paropt/VPOParopt.h"
#include "llvm/Transforms/VPO/Paropt/VPOParoptTransform.h"
#include <algorithm>
using namespace llvm;
using namespace llvm::vpo;

#define DEBUG_TYPE "VPOParoptTpv"

// The driver to support the threadprivate intel legacy mode.

class VPOParoptTpvLegacy {
public:
  VPOParoptTpvLegacy() {};
  ~VPOParoptTpvLegacy() {}
  bool processTpvInModule(Module &M,
         const DataLayout &DL);

private:
  /// Map table between threadprivate global and threadprivate local pointer
  DenseMap<Value*, Value *> TpvTable;

  /// Map table between the theread-private globals and threadprivate local
  /// pointer dereference per function.
  DenseMap<std::pair<Value *, Function*>, Value *> TpvAcc;

  /// Map table for global thread id per function
  DenseMap<Function*, Instruction *> TidTable;

  /// Returns the instruction which holds the value of global thread
  /// id. It can be a call instruction __kmpc_global_thread_num or
  /// a load instruction.
  Instruction* getThreadNum(Value *V, Function *F);

  /// Transforms the given threadprivate variable in legacy mode.
  void processTpv(Value *V,
      const DataLayout &DL);

  /// Returns the threadprivate local pointer dereference given a thread
  /// private variable and the accessed function.
  Value *getTpvRef(Value *V, Instruction *I,
      const DataLayout &DL);

  /// Returns the threadprivate local pointer given a threadprivate
  /// variable and the accessed function.
  Value *getTpvPtr(Value *V, Function *F, PointerType *GlobalType);

  /// Generate the code to a threadprivate local pointer deference given
  /// a threadprivate variable and the accessed function.
  void genTpvRef(Value *V,
                 Function *F,
                 Instruction *TidV,
                 const DataLayout &DL);

  /// Utility to collect the instructions which use the incoming value V
  /// recursively.
  void collectGlobalVarRecursively(Value *V,
                                   SmallVectorImpl<Instruction *> &RewriteCons,
                                   bool ExpectConstExprFlag);
};

class VPOParoptTpv : public ModulePass {
public:
  static char ID;
  VPOParoptTpv() : ModulePass(ID) {
    initializeVPOParoptTpvPass(*PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override {
    if (skipModule(M))
      return false;

    auto &DL = M.getDataLayout();

    VPOParoptTpvLegacy Tpv;
    return Tpv.processTpvInModule(M, DL);
  }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<TargetLibraryInfoWrapperPass>();
    AU.addRequired<DominatorTreeWrapperPass>();
  }

};

// For each theread private global, chooses intel compatible implementation.
bool VPOParoptTpvLegacy::processTpvInModule(Module &M,
      const DataLayout &DL) {
  bool Changed = false;
  for (Module::global_iterator GVI = M.global_begin(),
       E = M.global_end(); GVI != E;) {
    GlobalVariable *GV = &*(GVI++);
    if (GV && !GV->isThreadLocal() && GV->isThreadPrivate()) {
      processTpv(GV, DL);
      GV->setThreadPrivate(false);
      Changed = true;
    }
  }

  return Changed;
}

// If the thread-id is available, uses the first argument of outlined function.
// Otherwise, generates the call __kmpc_global_thread_num
Instruction* VPOParoptTpvLegacy::getThreadNum(Value *V, Function *F) {
  DenseMap<Function *, Instruction *>::iterator I =
                                 TidTable.find(F);
  if (I == TidTable.end()) {
    BasicBlock *EntryBB = &(F->getEntryBlock());
    BasicBlock::iterator I = EntryBB->getFirstInsertionPt();
    Instruction *AI = &*I;
    BasicBlock *T = SplitBlock(EntryBB, AI);
    T->setName("tid.bb");

    if (F->getFnAttribute("mt-func").getValueAsString() == "true") {
      IRBuilder<> Builder(EntryBB);
      Builder.SetInsertPoint(EntryBB->getTerminator());
      LoadInst *NewLoad = Builder.CreateLoad(&*(F->arg_begin()));
      TidTable[F] = NewLoad;
    }
    else {
      StructType *IdentTy = VPOParoptUtils::getIdentStructType(F);
      CallInst *RI = VPOParoptUtils::genKmpcGlobalThreadNumCall(
          F, &*(EntryBB->getFirstInsertionPt()), IdentTy);
      TidTable[F] = RI;
      RI->insertBefore(EntryBB->getTerminator());
    }
  }

  return TidTable[F];
}

// Generates the threadprivate local global pointer for threadprivate global
Value *VPOParoptTpvLegacy::getTpvPtr(Value *V, Function *F, PointerType *GlobalType){
  if (TpvTable.find(V) == TpvTable.end()) {
    GlobalVariable *NewGV = new GlobalVariable(
                            *(F->getParent()),
                            GlobalType,
                            false,
                            GlobalValue::InternalLinkage,
                            Constant::getNullValue(GlobalType),
                            "__tpv_ptr_"+V->getName(), nullptr);
    TpvTable[V] = NewGV;
  }
  return TpvTable[V];
}

// Generates the pointer dereference for threadprivate local global pointer.
void VPOParoptTpvLegacy::genTpvRef(Value *V,
                             Function *F,
                             Instruction *TidV,
                             const DataLayout &DL) {
  static int count=0;
  count ++;
  BasicBlock *B = &(F->getEntryBlock());
  BasicBlock::reverse_iterator InstIt = B->rbegin();
  BasicBlock::reverse_iterator LastInstIt;

  // Identifies the next instruction after the insruction tidV
  // There are two cases for the omp_tid.
  // case 1:
  //  %tid.val = tail call i32 @__kmpc_global_thread_num(..}
  //
  // case 2:
  //  define void @foo(i32* %.global_tid)
  //  entry:
  //     %0 = load i32, i32* %.global_tid
  //
  while (dyn_cast<Instruction>(&*InstIt) != TidV) {
    LastInstIt = InstIt;
    ++InstIt;
  }
  assert(isa<Instruction>(&*LastInstIt) &&
         "genTpvRef: Expect non-empty instruction.");
  Instruction* LastI = cast<Instruction>(&*LastInstIt);


  IRBuilder<> Builder(B);
  PointerType *Int8PtrTy = Builder.getInt8PtrTy();
  PointerType *Int8PtrPtrTy = Int8PtrTy->getPointerTo();

  // Generates a stack variable to store the dereference of threadprivate
  // local global pointer
  // Example:
  //     %0 = alloca i8*
  //     ....
  //     %7 = call i8* @__kmpc_threadprivate_cached(...)
  //     store i8* %7, i8** %0
  //
  AllocaInst *TpvPtrRef = new AllocaInst(Int8PtrTy, DL.getAllocaAddrSpace(),
                                         "", &*(B->begin()));
  TpvAcc[std::make_pair(V, F)]= TpvPtrRef;


  // Generates the code to check whether threadprivate local global pointer is empty.
  // Example:
  //   %2 = load i8**, i8*** @__tpv_ptr_a
  //   %3 = icmp ne i8** %2, null
  //
  Value *TpvGV = getTpvPtr(V, F, Int8PtrPtrTy);
  Builder.SetInsertPoint(LastI);

#if 0 // CMPLRS-47746: These if/else Instructions create a race with the
      // runtime since the runtime does a store to the cache array, and
      // these if/else Instructions do a load from the array,
      // without any synchronization/lock to ensure atomicity.
  LoadInst *NewLoad = Builder.CreateLoad(TpvGV);

  Value *PtrCompare = Builder.CreateICmp(ICmpInst::ICMP_NE,
                                         NewLoad,
         Constant::getNullValue(NewLoad->getType()));

  // Geneates the if-then control flow structure
  TerminatorInst *ThenTerm =
      SplitBlockAndInsertIfThen(PtrCompare,
                                LastI,
                                false,
                                MDBuilder(F->getContext()).createBranchWeights(99999,100000));
  BasicBlock *ThenBB = ThenTerm->getParent();
  ThenBB->setName("then.bb."+Twine(count));
  B->getTerminator()->getSuccessor(1)->setName("then.else.bb."+Twine(count));

  IRBuilder<> BuilderThen(ThenBB);
  BuilderThen.SetInsertPoint(ThenBB->getTerminator());

  // Generates the dereference of the threadprivate local global and compares whether
  // it is empty or not.
  //
  // Example:
  // %mul.1 = mul nsw i32 %1, 8
  // %4 = sext i32 %mul.1 to i64
  // %gep.1 = getelementptr i8*, i8** %2, i64 %4
  // %ld.1 = load i8*, i8** %gep.1
  // store i8* %ld.1, i8** %0
  // %5 = icmp eq i8* %ld.1, null
  unsigned PtrWidth = DL.getIntPtrType(Int8PtrTy)->getIntegerBitWidth();
  Value *MulV =
      BuilderThen.CreateMul(TidV,
                            ConstantInt::get(TidV->getType(),PtrWidth/8),
                            "mul."+Twine(count));
  cast<BinaryOperator>(MulV)->setHasNoSignedWrap(true);

  Value *ExtV = MulV;
  if (PtrWidth == 64) {
    ExtV =
      BuilderThen.CreateSExt(MulV, DL.getIntPtrType(Int8PtrTy));
  }

  Value *OffsetV =
    BuilderThen.CreateGEP(NewLoad, ExtV, "gep."+Twine(count));

  Value *LV =
    BuilderThen.CreateLoad(OffsetV, "ld."+Twine(count));

  BuilderThen.CreateStore(LV, TpvPtrRef);

  Value *Cmp =
    BuilderThen.CreateICmp(ICmpInst::ICMP_EQ,
                           LV,
                           Constant::getNullValue(LV->getType()));

  TerminatorInst *ElseTerm =
    SplitBlockAndInsertIfThen(Cmp,
                              ThenTerm,
                              false,
                              MDBuilder(F->getContext()).createBranchWeights(1,100000));
  BasicBlock *ElseBB = ElseTerm->getParent();
  ElseBB->setName("else.bb."+Twine(count));
  ThenBB->getTerminator()->getSuccessor(1)->setName("else.else.bb."+Twine(count));

  B->getTerminator()->setSuccessor(1, ElseBB);

  IRBuilder<> BuilderElse(ElseBB);
  BuilderElse.SetInsertPoint(ElseBB->getTerminator());
#else
  BasicBlock *ElseBB = B;
  IRBuilder<> BuilderElse(B);
  BuilderElse.SetInsertPoint(LastI);
#endif

  // Generates the call __kmpc_threadprivate_cached() if threadprivate local global pointer
  // is empty.
  // Example:
  //   %6 = bitcast i32* @a to i8*
  //   %7 = call i8* @__kmpc_threadprivate_cached(...)
  //   store i8* %7, i8** %0
  Instruction *AI = ElseBB->getTerminator();
  LLVMContext &C = F->getContext();
  StructType *IdentTy = VPOParoptUtils::getIdentStructType(F);

  PointerType *GVPtrType = cast<PointerType>(V->getType());
  if (V->getType() != Type::getInt8PtrTy(C))
    V = CastInst::CreatePointerCast(V, Type::getInt8PtrTy(C),
                                    Twine(""), ElseBB->getTerminator());

  Value *SizeV;

  if (DL.getIntPtrType(Int8PtrTy)->getIntegerBitWidth() == 64)
    SizeV = BuilderElse.getInt64(
                DL.getTypeAllocSize(GVPtrType->getPointerElementType()));
  else
     SizeV = BuilderElse.getInt32(
                DL.getTypeAllocSize(GVPtrType->getPointerElementType()));

  CallInst *TC = VPOParoptUtils::genKmpcThreadPrivateCachedCall(
      F, AI, IdentTy, TidV, V, SizeV, TpvGV);

  TC->insertBefore(AI);
  IRBuilder<> BuilderStore(AI);
  BuilderStore.CreateStore(TC, TpvPtrRef);
}

// Return the threadprivate local global pointer dereferece.
Value *VPOParoptTpvLegacy::getTpvRef(Value *V, Instruction *I,
               const DataLayout &DL) {
  Function *F = I->getParent()->getParent();
  Instruction *TidV = getThreadNum(V, F);
  if (TpvAcc.find(std::make_pair(V, F)) == TpvAcc.end()) {
    genTpvRef(V, F, TidV, DL);
  }

  return TpvAcc[std::make_pair(V, F)];
}

// Utility to collect the instructions which use the incoming value V
// recursively.
void VPOParoptTpvLegacy::collectGlobalVarRecursively(
    Value *V, SmallVectorImpl<Instruction *> &RewriteCons,
    bool ExpectConstExprFlag) {
  Instruction *I;

  for (auto IB = V->user_begin(), IE = V->user_end(); IB != IE; ++IB) {
    if (ConstantExpr *CE = dyn_cast<ConstantExpr>(*IB)) {
      for (Use &U1 : CE->uses()) {
        User *UR = U1.getUser();
        I = dyn_cast<Instruction>(UR);
        if (I)
          RewriteCons.push_back(I);
        else
          collectGlobalVarRecursively(UR, RewriteCons, false);
      }
    }
    else if (!ExpectConstExprFlag) {
      I = dyn_cast<Instruction>(*IB);
      if (I)
        RewriteCons.push_back(I);
    }
  }
}

// For each threadprivate globals, converts the reference into
// the reference of threadprivate local global pointer.
//
void VPOParoptTpvLegacy::processTpv(Value *V,
              const DataLayout &DL) {
  Value *New;
  Instruction *LoadI, *CastI;
  SmallVector<Instruction*, 8> RewriteCons;
  SmallVector<Instruction*, 8> RewriteIns;

  collectGlobalVarRecursively(V, RewriteCons, true);

  while (!RewriteCons.empty()) {
    Instruction *I = RewriteCons.pop_back_val();
    GeneralUtils::breakExpressions(I);
  }

  for (auto IB = V->user_begin(), IE = V->user_end();
       IB != IE; IB++) {
    if (Instruction *User = dyn_cast<Instruction>(*IB))
      RewriteIns.push_back(User);
  }

  while (!RewriteIns.empty()) {
    Instruction *User = RewriteIns.pop_back_val();
    New = getTpvRef(V, User, DL);
    IRBuilder<> BuilderCE(User->getParent());
    BuilderCE.SetInsertPoint(User);
    LoadI = BuilderCE.CreateLoad(New);
    if (V->getType() != LoadI->getType())
      CastI = CastInst::CreatePointerCast(LoadI, V->getType(), Twine(""), User);
    else
      CastI = LoadI;
    User->replaceUsesOfWith(V, CastI);
  }

}

PreservedAnalyses VPOParoptTpvLegacyPass::run(Module &M, AnalysisManager<Module> &AM) {
    auto &DL = M.getDataLayout();
    VPOParoptTpvLegacy Tpv;
    if (!Tpv.processTpvInModule(M, DL))
      return PreservedAnalyses::all();
    return PreservedAnalyses::none();
}

char VPOParoptTpv::ID = 0;
INITIALIZE_PASS_BEGIN(VPOParoptTpv, "vpo-paropt-tpv",
                      "Paropt TPV Transformation", false, false)
INITIALIZE_PASS_DEPENDENCY(TargetLibraryInfoWrapperPass)
INITIALIZE_PASS_DEPENDENCY(DominatorTreeWrapperPass)
INITIALIZE_PASS_END(VPOParoptTpv, "vpo-paropt-tpv",
                    "Paropt Tpv Transformation", false, false)

ModulePass *llvm::createVPOParoptTpvPass() {
  return new VPOParoptTpv();
}

#endif // INTEL_COLLAB
