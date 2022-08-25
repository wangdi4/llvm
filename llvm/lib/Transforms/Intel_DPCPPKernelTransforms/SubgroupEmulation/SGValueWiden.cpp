//===------------------ SGValueWiden.cpp - Widen values ------------------===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SubgroupEmulation/SGValueWiden.h"
#include "llvm/Analysis/VectorUtils.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"
#include "llvm/PassRegistry.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/LegacyPasses.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SubgroupEmulation/SGFunctionWiden.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/LoopUtils.h"

using namespace llvm;
using namespace CompilationUtils;
using namespace LoopUtils;

extern bool DPCPPEnableSubGroupEmulation;

#define DEBUG_TYPE "dpcpp-kernel-sg-emu-value-widen"

static std::string encodeVectorVariant(Function *F, unsigned EmuSize) {
  using namespace DPCPPKernelMetadataAPI;
  auto Kernels = KernelList(F->getParent()).getList();
  auto KernelRange = make_range(Kernels.begin(), Kernels.end());
  std::string Buffer;
  llvm::raw_string_ostream Out(Buffer);

  // The vector variant name starts with: prefix (_ZGV) + ISA (b: XMM) + Mask(N:
  // not masked)
  Out << "_ZGVbN" << EmuSize;

  // Encode. TODO: We can enable uniformity analysis to mark some parameters
  // as uniform.
  const char *KindStr = find(KernelRange, F) != Kernels.end() ? "u" : "v";
  SmallVector<StringRef, 4> KindStrs(F->arg_size(), KindStr);
  Out << join(KindStrs.begin(), KindStrs.end(), "");
  Out << '_' << F->getName();
  Out.flush();
  return std::string(Out.str());
}

bool SGValueWidenPass::runImpl(Module &M, const SGSizeInfo *SSI) {
  if (!DPCPPEnableSubGroupEmulation)
    return false;

  Helper.initialize(M);
  FunctionsToBeWidened = Helper.getAllFunctionsNeedEmulation();

  if (FunctionsToBeWidened.empty())
    return false;

  ConstZero = Helper.getZero();

  for (auto *Fn : FunctionsToBeWidened) {
    LLVM_DEBUG(dbgs() << "Adding vector-variants for " << Fn->getName()
                      << "\n");
    // Add vector-variants attribute.
    const std::set<unsigned> Sizes = SSI->getEmuSizes(Fn);
    SmallVector<std::string, 4> Variants;
    for (auto I = Sizes.begin(), E = Sizes.end(); I != E; I++) {
      Variants.push_back(encodeVectorVariant(Fn, *I));
    }
    Fn->addFnAttr(KernelAttribute::VectorVariants, join(Variants, ","));
    LLVM_DEBUG(dbgs() << "  vector-variants: "
                      << Fn->getFnAttribute(KernelAttribute::VectorVariants)
                             .getValueAsString()
                      << "\n");
  }

  // Add sub-group functions to FunctionsToBeWidened.
  for (auto &Fn : M)
    if (Fn.isDeclaration() && Fn.getName().contains("sub_group") &&
        Fn.hasFnAttribute(KernelAttribute::VectorVariants))
      FunctionsToBeWidened.insert(&Fn);

  FunctionWidener FWImpl;
  FWImpl.run(FunctionsToBeWidened, FuncMap);

  // Initialize barrier utils.
  Utils.init(&M);

  // Process dummy region.
  // 1) Move original instructions located in dummybarrier region (if exists).
  //    to WGExcludeBB
  // 2) Move the first dummybarrier (if exists) to the begin of SGExcludeBB;
  //    Move the second dummybarrier (if exists) to the begin of WGExcludeBB.
  for (auto &Pair : FuncMap) {
    for (auto *EmulateFunc : Pair.second) {
      // Skip sub-group functions.
      if (EmulateFunc->isDeclaration())
        continue;
      auto *SGExcludeBB = &EmulateFunc->getEntryBlock();

      auto *SGLoopHeader = SGExcludeBB->getSingleSuccessor();
      assert(SGLoopHeader && "Get single successor failed");
      auto *FirstI = &*SGLoopHeader->begin();
      SGExcludeBBMap[EmulateFunc] = SGExcludeBB;

      if (Utils.isDummyBarrierCall(FirstI)) {
        auto DummyRegion = Utils.findDummyRegion(*EmulateFunc);
        if (!DummyRegion.empty()) {
          BasicBlock *WGExcludeBB = SGExcludeBB;
          std::string SGExcludeBBName = SGExcludeBB->getName().str();
          WGExcludeBB->setName("wg.loop.exclude");
          SGExcludeBB = SGExcludeBB->splitBasicBlock(&*SGExcludeBB->begin(),
                                                     SGExcludeBBName);
          SGExcludeBBMap[EmulateFunc] = SGExcludeBB;
          WGExcludeBBMap[EmulateFunc] = WGExcludeBB;
          Instruction *IP = WGExcludeBB->getTerminator();
          InstVec InstsToMove;
          for (auto &Inst : DummyRegion)
            InstsToMove.push_back(&Inst);
          for (auto *Inst : InstsToMove)
            Inst->moveBefore(IP);
          // Move the dummyBarrier to the begin of WGExcludeBB.
          InstsToMove.back()->moveBefore(&*WGExcludeBB->begin());
        }
        // Move the dummyBarrier to the begin of SGExcludeBB.
        FirstI->moveBefore(&*SGExcludeBB->begin());
      }
    }
  }
  collectWideCalls(M);

  // Re-initialize for emulated functions.
  Helper.initialize(M);

  // Kernels to be emulated must be scalar kernels, no need to
  // find all kernels.
  using namespace DPCPPKernelMetadataAPI;
  auto Kernels = KernelList(M).getList();
  auto KernelRange = make_range(Kernels.begin(), Kernels.end());
  for (auto *Fn : FunctionsToBeWidened) {
    for (auto *WideFn : FuncMap[Fn]) {
      // Skip sub-group functions.
      if (WideFn->isDeclaration())
        continue;
      LLVM_DEBUG(dbgs() << "Widen function: " << WideFn->getName() << "\n");
      VFInfo Variant = VFABI::demangleForVFABI(WideFn->getName());
      runOnFunction(*WideFn, Variant.getVF());
      auto It = find(KernelRange, Fn);
      if (It != Kernels.end()) {
        Kernels.erase(It);
        Kernels.push_back(WideFn);
        std::string FName = Fn->getName().str();
        Fn->setName(FName + "_after_value_widen");
        WideFn->setName(FName);
      }
    }
  }
  KernelList(&M).set(Kernels);

  // Re-initialize barrier utils.
  Utils.init(&M);

  widenCalls();

  // If all the conditions are met:
  //   1) kernel A calls function B and kernel D calls same function B;
  //   2) function B calls function C;
  //   3) only the function body of kernel D has sub-group and function C has
  //      barrier.
  // Then the scalar version of both B and C should be preserved since kernel A
  // is not widened.
  // The better solution should be do not to widen functions only call
  // work_group_barrier.
  SmallVector<Function *, 4> FnToRemove;
  for (Function *Fn : FunctionsToBeWidened) {
    for (auto &I : instructions(Fn)) {
      if (Helper.isBarrier(&I) || Helper.isDummyBarrier(&I))
        InstsToBeRemoved.push_back(&I);
    }
    if (Fn->getName().endswith("_after_value_widen"))
      FnToRemove.push_back(Fn);
  }

  for (auto *I : InstsToBeRemoved)
    I->eraseFromParent();

  for (auto *Fn : FnToRemove)
    Fn->eraseFromParent();

  return true;
}

void SGValueWidenPass::collectWideCalls(Module &) {
  FuncSet EmulateFuncs;
  for (auto &Pair : FuncMap)
    for (auto EmuFunc : Pair.second)
      EmulateFuncs.insert(EmuFunc);

  for (Function *F : FunctionsToBeWidened) {
    for (User *U : F->users())
      if (CallInst *CI = dyn_cast<CallInst>(U))
        if (EmulateFuncs.count(CI->getCaller()))
          WideCalls.insert(CI);
  }
}

bool SGValueWidenPass::isCrossBarrier(Instruction *I,
                                      const InstSet &SyncInsts) const {
  BasicBlock *DefBB = I->getParent();

  for (User *U : I->users()) {
    auto *UI = dyn_cast<Instruction>(U);
    if (!UI)
      continue;
    BasicBlock *UseBB = UI->getParent();
    if (BarrierUtils::isCrossedByBarrier(SyncInsts, UseBB, DefBB))
      return true;
  }

  return false;
}

bool SGValueWidenPass::isWIRelated(Value *V) {
  if (CallInst *CI = dyn_cast<CallInst>(V)) {
    if (Function *F = CI->getCalledFunction()) {
      const std::string Name = F->getName().str();
      // Once WIRelatedAnalysis is ported to SGEmulation, we can remove
      // isWorkGroupUniform.
      if (isSubGroupUniform(Name) || isWorkGroupUniform(Name))
        return false;
    }
  }

  return true;
  // TODO: WIRelatedAnalysis pass should be improved to analyze uniformity for
  // sub-group emulation.
  // return WIRelatedAnalysis->isWIRelated(V);
}

void SGValueWidenPass::runOnFunction(Function &F, const unsigned &Size) {
  LLVM_DEBUG(dbgs() << "Begin widening: " << F.getName()
                    << " with size: " << Size << "\n");
  InstSet SyncInsts = Helper.getSyncInstsForFunction(&F);
  assert(!SyncInsts.empty() && "Can't find sync insts!");

  DebugAIMap.clear();

  BasicBlock *WGExcludeBB =
      WGExcludeBBMap.count(&F) ? WGExcludeBBMap[&F] : nullptr;
  BasicBlock *SGExcludeBB = SGExcludeBBMap[&F];
  Instruction *FirstI = SGExcludeBB->getTerminator();

  InstVec WorkList;

  // Collect all instructions to be handled first, we can't process the
  // instructions while iterating function because we will insert new
  // instructions.
  for (auto &I : instructions(F)) {
    // Skip WGExcludeBB and SGExcludeBB, they are not in sub-group loop.
    if (I.getParent() == WGExcludeBB || I.getParent() == SGExcludeBB ||
        I.use_empty())
      continue;
    WorkList.push_back(&I);
  }

  for (auto *I : WorkList) {
    if (isa<AllocaInst>(I)) {
      widenAlloca(I, FirstI, Size);
    } else if (isCrossBarrier(I, SyncInsts)) {
      if (isWIRelated(I))
        widenValue(I, FirstI, Size);
      else {
        Instruction *IP = FirstI;
        // For WG uniform calls, we need to create an alloca instruction in
        // WGExcludeBB to hold the result value.
        if (CallInst *CI = dyn_cast<CallInst>(I)) {
          if (Function *F = CI->getCalledFunction()) {
            const std::string Name = F->getName().str();
            if (isWorkGroupUniform(Name)) {
              assert(WGExcludeBB && "WGExcludeBB doesn't exist");
              IP = WGExcludeBB->getTerminator();
            }
          }
        }
        hoistUniformValue(I, IP);
      }
    }
  }

  // For every local variables with debug info attached, we will store
  // it's new address to DebugAI in every sub-group loop.
  for (auto &Pair : DebugAIMap) {
    AllocaInst *AI = Pair.first;
    AllocaInst *DebugAI = Pair.second;
    AllocaInst *WideAI = cast<AllocaInst>(VecValueMap[AI]);

    auto *ArraySize = dyn_cast<ConstantInt>(AI->getArraySize());
    assert(ArraySize && "Array Size is not constant");
    for (Instruction *SyncInst : SyncInsts) {
      auto *IP = SyncInst->getNextNode();
      // Skip non-loop region.
      // FIXME: isa<ReturnInst>(IP) doesn't work because wide loads for return
      // values are here.
      if (isWideCall(IP) || isa<ReturnInst>(IP))
        continue;
      auto *Idx = Helper.createGetSubGroupLId(IP);
      IRBuilder<> Builder(IP);
      auto *Offset = ArraySize->getZExtValue() > 1
                         ? Builder.CreateMul(Idx, ArraySize)
                         : Idx;
      auto *NewV = Builder.CreateGEP(WideAI->getAllocatedType(), WideAI,
                                     {ConstZero, Offset});
      Builder.CreateStore(NewV, DebugAI);
    }
  }
}

void SGValueWidenPass::hoistUniformValue(Instruction *V, Instruction *FirstI) {
  LLVM_DEBUG(dbgs() << "Update Uniform Instruction" << *V << "\n");
  IRBuilder<> Builder(FirstI);
  Type *VType = V->getType();
  auto *VPtr = Builder.CreateAlloca(VType, nullptr, "u." + V->getName());
  UniValueMap[V] = VPtr;

  // Update Use.
  InstSet UsersToUpdate;
  for (auto *U : V->users()) {
    auto *UI = dyn_cast<Instruction>(U);
    // ReturnInst has been processed in FunctionWidener
    if (!UI || isWideCall(UI) || isa<ReturnInst>(UI))
      continue;
    UsersToUpdate.insert(UI);
  }

  for (auto *UI : UsersToUpdate) {
    LLVM_DEBUG(dbgs() << "Updating Use:" << *UI << "\n");
    Builder.SetInsertPoint(getInsertPoint(UI, V));
    auto *NewV = Builder.CreateLoad(VType, VPtr);
    UI->replaceUsesOfWith(V, NewV);
  }

  // We will store the vector return value in WidenCalls.
  if (!isWideCall(V))
    setWIValue(V);
}

Value *SGValueWidenPass::getVectorValuePtr(Value *V, unsigned Size,
                                           Instruction *IP) {
  assert(VecValueMap.count(V) && "Can't find value in VecValueMap");
  auto *WideValuePtr = cast<AllocaInst>(VecValueMap[V]);
  auto *WideType = WideValuePtr->getAllocatedType();
  Type *DestType = SGHelper::getVectorType(V, Size);
  IRBuilder<> Builder(IP);

  // eg. V = alloca i32 DestType shold be <16 x i32*>
  if (isa<AllocaInst>(V)) {
    // Generate sequential vector 0 ... Size-1.
    SmallVector<Constant *, 16> IndVec;
    for (unsigned Ind = 0; Ind < Size; ++Ind)
      IndVec.push_back(ConstantInt::get(Builder.getInt32Ty(), Ind));
    Constant *SequenceVec = ConstantVector::get(IndVec);

    return Builder.CreateGEP(WideType, WideValuePtr, {ConstZero, SequenceVec});
  }

  if (WideType->isVectorTy())
    return WideValuePtr;

  auto *VectorValType = cast<FixedVectorType>(V->getType());
  unsigned OrigNumElem = VectorValType->getNumElements();
  // For type3 shuffle, look like this cast doesn't work.
  if ((OrigNumElem & (OrigNumElem - 1)) == 0)
    return Builder.CreateBitCast(WideValuePtr, PointerType::get(DestType, 0));
  return nullptr;
}

static void storeVectorByVecElement(Value *Addr, Value *Data, Type *OrigValType,
                                    unsigned VF, IRBuilderBase &Builder) {
  Value *ConstZero = Builder.getInt32(0);
  auto *VectorValType = cast<FixedVectorType>(OrigValType);
  unsigned OrigNumElem = VectorValType->getNumElements();
  for (unsigned Idx = 0; Idx < VF; ++Idx) {
    for (unsigned Id = 0; Id < OrigNumElem; ++Id) {
      Value *ElemVal =
          Builder.CreateExtractElement(Data, Idx * OrigNumElem + Id);
      Value *ElemPtr = Builder.CreateGEP(
          Addr->getType()->getScalarType()->getPointerElementType(), Addr,
          {ConstZero, Builder.getInt32(Idx), Builder.getInt32(Id)});
      Builder.CreateStore(ElemVal, ElemPtr);
    }
  }
}

static Value *loadVectorByVecElement(Value *Addr, Type *OrigValType,
                                     unsigned VF, IRBuilderBase &Builder) {
  Value *ConstZero = Builder.getInt32(0);
  auto *VectorValType = cast<FixedVectorType>(OrigValType);
  Type *ScalarEleType = VectorValType->getScalarType();
  unsigned OrigNumElem = VectorValType->getNumElements();
  Type *DestType = SGHelper::getVectorType(OrigValType, VF);
  Value *Res = UndefValue::get(DestType);
  for (unsigned Idx = 0; Idx < VF; ++Idx) {
    for (unsigned Id = 0; Id < OrigNumElem; ++Id) {
      Value *ElemPtr = Builder.CreateGEP(
          Addr->getType()->getScalarType()->getPointerElementType(), Addr,
          {ConstZero, Builder.getInt32(Idx), Builder.getInt32(Id)});
      Value *ElemVal = Builder.CreateLoad(ScalarEleType, ElemPtr);
      Res = Builder.CreateInsertElement(Res, ElemVal, Idx * OrigNumElem + Id);
    }
  }
  return Res;
}

void SGValueWidenPass::setVectorValue(Value *Data, Value *V, unsigned Size,
                                      Instruction *IP) {
  IRBuilder<> Builder(IP);
  Type *OrigValType = V->getType();

  if (UniValueMap.count(V)) {
    Value *ElemVal =
        isa<FixedVectorType>(OrigValType)
            ? generateExtractSubVector(Data, 1, 16, Builder, "extract.sub.")
            : Builder.CreateExtractElement(Data, ConstZero);
    Builder.CreateStore(ElemVal, UniValueMap[V]);
    return;
  }
  assert(VecValueMap.count(V) && "Can't find Value in VecValueMap");
  Value *Ptr = getVectorValuePtr(V, Size, IP);
  if (Ptr != nullptr) {
    Builder.CreateStore(Data, Ptr);
  } else {
    // Do we need to fix vector memory access here?
    storeVectorByVecElement(VecValueMap[V], Data, OrigValType, Size, Builder);
  }
}

Value *SGValueWidenPass::getVectorValue(Value *V, unsigned Size,
                                        Instruction *IP) {
  auto *OrigValType = V->getType();
  assert(!OrigValType->isVoidTy() && "Get vector counterpart of void type!");

  if (isa<AllocaInst>(V))
    return getVectorValuePtr(V, Size, IP);

  IRBuilder<> Builder(IP);
  if (VecValueMap.count(V)) {
    Value *WideValPtr = getVectorValuePtr(V, Size, IP);
    // Process type3 shuffle.
    if (WideValPtr == nullptr)
      return loadVectorByVecElement(VecValueMap[V], OrigValType, Size, Builder);
    return Builder.CreateLoad(SGHelper::getVectorType(V, Size), WideValPtr);
  }
  if (UniValueMap.count(V))
    V = Builder.CreateLoad(OrigValType, UniValueMap[V]);
  if (isa<FixedVectorType>(OrigValType))
    return replicateVector(V, Size, Builder, "splat.");
  return Builder.CreateVectorSplat(Size, V);
}

Value *SGValueWidenPass::getScalarValue(Value *V, Instruction *IP) {
  IRBuilder<> Builder(IP);
  auto *OrigValType = V->getType();
  if (UniValueMap.count(V))
    return Builder.CreateLoad(OrigValType, UniValueMap[V]);

  // This value should be constant.
  if (!VecValueMap.count(V))
    return V;

  auto *WideValuePtr = cast<AllocaInst>(VecValueMap[V]);
  auto *ScalarValuePtr = Builder.CreateGEP(
      WideValuePtr->getAllocatedType(), WideValuePtr, {ConstZero, ConstZero});
  return Builder.CreateLoad(OrigValType, ScalarValuePtr);
}

void SGValueWidenPass::setWIValue(Value *V) {
  Instruction *IP = cast<Instruction>(V)->getNextNode();
  // It's safe to do this, because if the use of V is before this store then
  // the use itself must be a PHINode in this BB, so we won't load from
  // ValueMap for that use.
  if (isa<PHINode>(IP))
    IP = IP->getParent()->getFirstNonPHI();
  IRBuilder<> Builder(IP);
  if (!VecValueMap.count(V) && !UniValueMap.count(V)) {
    LLVM_DEBUG(dbgs() << "Value: " << *V << '\n');
    llvm_unreachable("Can't find value in ValueMap!");
  }
  auto *Offset =
      VecValueMap.count(V) ? getWIOffset(IP, VecValueMap[V]) : UniValueMap[V];
  // Offset's pointee type might be the promoted widen type of V.
  // In such cases, we need to zext V to promoted type before store.
  auto *Promoted = SGHelper::createZExtOrTruncProxy(
      V, Offset->getType()->getPointerElementType(), Builder);
  Builder.CreateStore(Promoted, Offset);
}

Value *SGValueWidenPass::getWIValue(Instruction *U, Value *V) {
  // If all the path from V to U don't cross barrier, we can return V. But for
  // wide calls, they will be removed later, so we can't use the scalar one.
  // TODO: Can we replace this approximate analyis (Use and Def are in the same
  // BB) with Cross-Barrier analysis?
  if (U->getParent() == cast<Instruction>(V)->getParent() && !isWideCall(V))
    return V;
  assert(VecValueMap.count(V) && "Can't find value in VecValueMap");
  Value *Src = VecValueMap[V];
  Instruction *IP = getInsertPoint(U, V);
  IRBuilder<> Builder(IP);
  auto *Offset = getWIOffset(IP, Src);
  auto *LI =
      Builder.CreateLoad(Offset->getType()->getPointerElementType(), Offset);
  // Src might be the promoted widen type of V.
  // In such cases, we need to trunc LI to V's original type.
  return SGHelper::createZExtOrTruncProxy(LI, V->getType(), Builder);
}

Value *SGValueWidenPass::getWIOffset(Instruction *IP, Value *Src) {
  assert(Src->getType()->isPointerTy() && "get offest for a non-pointer value");
  auto *Idx = Helper.createGetSubGroupLId(IP);
  IRBuilder<> Builder(IP);
  return Builder.CreateGEP(Src->getType()->getPointerElementType(), Src,
                           {ConstZero, Idx});
}

Instruction *SGValueWidenPass::getInsertPoint(Instruction *I, Value *V) {

  if (isWideCall(I) || isa<ReturnInst>(I)) {
    auto *IP = I->getPrevNode();
    // The ending barrier may be deleted before
    // sg_barrier
    // %0 = sub_group_all(1)
    // dummy_sg_barrier
    // sg_barrier
    // ret %0
    if (!Helper.isBarrier(IP))
      IP = Helper.insertBarrierBefore(I);
    return getInsertPoint(IP, V);
  }

  // Barrier and dummy barrier should always be the first instruction in a
  // BasicBlock.
  if (Helper.isBarrier(I) || Helper.isDummyBarrier(I)) {
    auto *SyncBB = I->getParent();
    std::string BBName = SyncBB->getName().str();
    SyncBB->setName("");
    SyncBB->splitBasicBlock(I, BBName);
    return SyncBB->getTerminator();
  }

  if (!isa<PHINode>(I))
    return I;

  auto *PHI = cast<PHINode>(I);
  auto *BB = PHI->getParent();
  for (auto BI = pred_begin(BB), BE = pred_end(BB); BI != BE; BI++) {
    auto *BV = PHI->getIncomingValueForBlock(*BI);
    if (BV == V)
      return (*BI)->getTerminator();
  }
  llvm_unreachable("Can't find incoming basicblock for value");
}

static std::pair<StringRef, unsigned> selectVariantAndEmuSize(CallInst *CI) {
  // Get parent function's vector-variants
  Function *ParentF = CI->getFunction();
  LLVM_DEBUG(dbgs() << "  Parent function name: " << ParentF->getName()
                    << "\n");
  assert(ParentF->hasFnAttribute(KernelAttribute::VectorVariants) &&
         "Parent function doesn't have vector-variants attribute");
  unsigned EmuSize = 0;
  if (auto Variant = VFABI::tryDemangleForVFABI(ParentF->getName())) {
    EmuSize = Variant->getVF();
  } else {
    StringRef ParentVariantStringValue =
        ParentF->getFnAttribute(KernelAttribute::VectorVariants)
            .getValueAsString();
    assert(ParentVariantStringValue.find(',') == StringRef::npos &&
           "Unexpected multiple vector variant string here!");
    LLVM_DEBUG(dbgs() << "  Parent function variant string: "
                      << ParentVariantStringValue << "\n");
    EmuSize = VFABI::demangleForVFABI(ParentVariantStringValue).getVF();
  }

  // Get vector-variants attribute
  StringRef VecVariantStringValue =
      CI->getCallSiteOrFuncAttr(KernelAttribute::VectorVariants)
          .getValueAsString();
  LLVM_DEBUG(dbgs() << "  Call instruction vector variant string: "
                    << VecVariantStringValue << "\n");
  SmallVector<StringRef, 4> VariantStrs;
  VecVariantStringValue.split(VariantStrs, ",");
  StringRef VariantStringValue;
  // Select Variant and emulation size
  for (auto VarStr : VariantStrs) {
    unsigned Vlen = VFABI::demangleForVFABI(VarStr).getVF();
    if (Vlen == EmuSize) {
      VariantStringValue = VarStr;
      break;
    }
  }
  assert(VariantStringValue.size() != 0 &&
         "Vector variant with same vector length is not found!");
  LLVM_DEBUG(dbgs() << "  Select vector variant string: " << VariantStringValue
                    << ", emulation size: " << EmuSize << "\n");

  return std::make_pair(VariantStringValue, EmuSize);
}

void SGValueWidenPass::widenCalls() {
  for (auto *I : WideCalls) {
    auto *CI = cast<CallInst>(I);
    LLVM_DEBUG(dbgs() << "Widening Call: " << *CI << "\n");
    assert(CI->hasFnAttr(KernelAttribute::VectorVariants) &&
           "wide call doesn't have vector-variants attribute");

    unsigned Size = 0;
    StringRef VariantStr;
    std::tie(VariantStr, Size) = selectVariantAndEmuSize(CI);
    auto Variant = VFABI::demangleForVFABI(VariantStr);
    Function *WideFunc = CI->getModule()->getFunction(Variant.VectorName);
    assert(WideFunc != nullptr && "No widen function!");
    assert(FuncMap[CI->getCalledFunction()].count(WideFunc) &&
           "Invalid vector variant function!");

    // If this function is not WG sync function, we can insert the new
    // widened call and load parameters / store return value before
    // the old call, otherwise, we need to load parameters before barrier call
    // and store return value after dummybarrier call.
    Instruction *ParamIP = CI, *RetValIP = CI;
    if (Utils.getAllFunctionsWithSynchronization().count(WideFunc)) {
      Instruction *PrevInst = CI->getPrevNonDebugInstruction();
      Instruction *NextInst = CI->getNextNonDebugInstruction();
      assert(Utils.isBarrierCall(PrevInst) &&
             "There should be a barrier before widened call");
      assert(Utils.isDummyBarrierCall(NextInst) &&
             "There should be a dummybarrier after widened call");
      ParamIP = PrevInst;
      RetValIP = NextInst->getNextNode();
    }

    IRBuilder<> Builder(CI);
    ArrayRef<VFParameter> Params = Variant.Shape.Parameters;

    SmallVector<Value *, 4> NewArgs;
    SmallVector<Type *, 4> NewArgTypes;
    for (auto &Pair : enumerate(CI->args())) {
      VFParameter Param = Params[Pair.index()];
      Value *Arg = Pair.value(), *NewArg = nullptr;
      if (Param.isUniform()) {
        NewArg = getScalarValue(Arg, ParamIP);
      } else {
        assert(Param.isVector() && "Unsupported VectorKind");
        NewArg = getVectorValue(Arg, Size, ParamIP);
      }
      assert(NewArg && "Vector value is null!");
      // Function argument might be promoted, create a zext.
      NewArg = SGHelper::createZExtOrTruncProxy(
          NewArg, WideFunc->getArg(Pair.index())->getType(), Builder);
      NewArgs.push_back(NewArg);
      NewArgTypes.push_back(NewArg->getType());
    }

    // Add mask argument
    if (Variant.isMasked()) {
      // Currently all mask is <i32 x VF>.
      auto *SGSize = Helper.createGetSubGroupSize(ParamIP);
      Value *Mask = generateRemainderMask(Size, SGSize, ParamIP);
      NewArgs.push_back(Mask);
      NewArgTypes.push_back(Mask->getType());
    }

    // Create new call.
    auto *NewReturnVal = Builder.CreateCall(WideFunc, NewArgs);
    NewReturnVal->setCallingConv(CI->getCallingConv());
    LLVM_DEBUG(dbgs() << "  -> " << *NewReturnVal << "\n");
    if (UniValueMap.count(CI) || VecValueMap.count(CI)) {
      // Store the vectorized return value.
      setVectorValue(NewReturnVal, CI, Size, RetValIP);
    }

    CI->dropAllReferences();
    InstsToBeRemoved.push_back(CI);
  }
}

void SGValueWidenPass::widenValue(Instruction *V, Instruction *FirstI,
                                  unsigned Size) {
  // Skip value w/o any user.
  if (V->user_empty())
    return;

  LLVM_DEBUG(dbgs() << "Widening Value:" << *V << "\n");
  IRBuilder<> Builder(FirstI);

  Type *VType = V->getType();
  std::string NewName = "w." + V->getName().str();

  AllocaInst *WideValueAddr = nullptr;
  if (VType->isAggregateType() || VType->isVectorTy()) {
    auto *Promoted = SGHelper::getPromotedIntVecType(VType);
    Type *WideType = ArrayType::get(Promoted, Size);
    WideValueAddr = Builder.CreateAlloca(WideType, nullptr, NewName);
    // We may convert this addr to pointer of vector and then load the vector
    // from it. This behaviour will cause aligned vector move, so we need to
    // update the alignment here.
    WideValueAddr->setAlignment(
        V->getModule()->getDataLayout().getPrefTypeAlign(Promoted) * Size);
  } else {
    Type *WideType = SGHelper::getVectorType(VType, Size);
    WideValueAddr = Builder.CreateAlloca(WideType, nullptr, NewName);
  }

  VecValueMap[V] = WideValueAddr;
  LLVM_DEBUG(dbgs() << "To:" << *WideValueAddr << "\n");

  // Update Use.
  InstSet UsersToUpdate;
  for (auto *U : V->users()) {
    auto *UI = dyn_cast<Instruction>(U);
    if (!UI || isWideCall(UI) || isa<ReturnInst>(UI))
      continue;
    UsersToUpdate.insert(UI);
  }

  for (auto *UI : UsersToUpdate) {
    LLVM_DEBUG(dbgs() << "Updating Use:" << *UI << "\n");
    auto *NewV = getWIValue(UI, V);
    UI->replaceUsesOfWith(V, NewV);
  }

  // We will store the vector return value in WidenCalls.
  if (!isWideCall(V))
    setWIValue(V);
}

void SGValueWidenPass::widenAlloca(Instruction *V, Instruction *FirstI,
                                   unsigned Size) {
  // Skip value w/o any user.
  if (V->user_empty())
    return;

  LLVM_DEBUG(dbgs() << "Widening Value(alloca):" << *V << "\n");
  AllocaInst *AI = cast<AllocaInst>(V);

  auto *ArraySize = dyn_cast<ConstantInt>(AI->getArraySize());
  assert(ArraySize && "Array Size is not constant");

  IRBuilder<> Builder(FirstI);
  unsigned NewSize = Size * ArraySize->getZExtValue();
  Type *AllocatedType = AI->getAllocatedType();
  auto AddrSpace = AI->getType()->getAddressSpace();
  std::string NewName = "w." + V->getName().str();

  AllocaInst *WideValue = nullptr;
  if (AllocatedType->isAggregateType() || AllocatedType->isVectorTy()) {
    auto *Promoted = SGHelper::getPromotedIntVecType(AllocatedType);
    Type *WideType = ArrayType::get(Promoted, NewSize);
    WideValue = Builder.CreateAlloca(WideType, AddrSpace, nullptr, NewName);
    // We may convert this addr to pointer of vector and then load the vector
    // from it. This behaviour will cause aligned vector move, so we need to
    // update the alignment here.
    WideValue->setAlignment(
        V->getModule()->getDataLayout().getPrefTypeAlign(Promoted) * Size);
  } else {
    Type *WideType = SGHelper::getVectorType(AllocatedType, NewSize);
    WideValue = Builder.CreateAlloca(WideType, AddrSpace, nullptr, NewName);
  }
  VecValueMap[V] = WideValue;
  LLVM_DEBUG(dbgs() << "To:" << *WideValue << "\n");

  InstSet UsersToUpdate;
  for (auto *U : AI->users()) {
    auto *UI = dyn_cast<Instruction>(U);
    if (!UI || isWideCall(UI) || isa<ReturnInst>(UI))
      continue;
    UsersToUpdate.insert(UI);
  }

  // Create a new llvm.dbg.declare which describes the address of original
  // local variable. And so we should prepend deref to DIExpression.
  auto DbgDeclares = FindDbgDeclareUses(AI);
  if (!DbgDeclares.empty()) {
    Builder.SetInsertPoint(FirstI);
    AllocaInst *AIAddrDbg =
        Builder.CreateAlloca(AI->getType(), nullptr, "dbg." + AI->getName());

    DbgDeclareInst *DI = DbgDeclares.front();
    DIBuilder DIB(*FirstI->getModule(), false);
    DILocalVariable *Variable = DI->getVariable();
    DIExpression *Expression =
        DIExpression::prepend(DI->getExpression(), DIExpression::DerefBefore);
    const DILocation *Location = DI->getDebugLoc().get();
    DIB.insertDeclare(AIAddrDbg, Variable, Expression, Location,
                      AIAddrDbg->getNextNode());
    DebugAIMap[AI] = AIAddrDbg;
  }

  // We always need to do this even if we disable debug, otherwise,
  // when we remove AI, these debug intrinsics are still using it.
  for (auto *DI : DbgDeclares)
    DI->eraseFromParent();

  for (auto *UI : UsersToUpdate) {
    LLVM_DEBUG(dbgs() << "Updating Use:" << *UI << "\n");
    auto *IP = getInsertPoint(UI, V);
    auto *Idx = Helper.createGetSubGroupLId(IP);
    // This action also sets Debug Location.
    Builder.SetInsertPoint(IP);
    auto *Offset =
        ArraySize->getZExtValue() > 1 ? Builder.CreateMul(Idx, ArraySize) : Idx;
    auto *NewV = Builder.CreateGEP(WideValue->getAllocatedType(), WideValue,
                                   {ConstZero, Offset});
    // NewV's pointee type might be the promoted type of V.
    // e.g. "alloca i1, align 1" will be widen to
    //      "alloca <16 x i8>, align 16"
    // In such cases, we need to cast the GEP back to original pointer type.
    NewV = Builder.CreatePointerCast(NewV, V->getType());
    UI->replaceUsesOfWith(V, NewV);
  }

  InstsToBeRemoved.push_back(AI);
}

namespace {
/// Legacy SGValueWiden pass.
class SGValueWidenLegacy : public ModulePass {
public:
  static char ID;

  SGValueWidenLegacy() : ModulePass(ID) {
    initializeSGValueWidenLegacyPass(*PassRegistry::getPassRegistry());
  }

  StringRef getPassName() const override { return "SGValueWidenLegacy"; }

  bool runOnModule(Module &M) override;

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addRequired<SGSizeAnalysisLegacy>();
  }

private:
  SGValueWidenPass Impl;
};
} // namespace

char SGValueWidenLegacy::ID = 0;

INITIALIZE_PASS_BEGIN(SGValueWidenLegacy, DEBUG_TYPE,
                      "Insert sub_group_barrier and vector-variants attribute",
                      false, false)
INITIALIZE_PASS_DEPENDENCY(SGSizeAnalysisLegacy)
INITIALIZE_PASS_END(SGValueWidenLegacy, DEBUG_TYPE,
                    "Insert sub_group_barrier and vector-variants attribute",
                    false, false)

bool SGValueWidenLegacy::runOnModule(Module &M) {
  const SGSizeInfo *SSI = &getAnalysis<SGSizeAnalysisLegacy>().getResult();

  return Impl.runImpl(M, SSI);
}

ModulePass *llvm::createSGValueWidenLegacyPass() {
  return new SGValueWidenLegacy();
}
