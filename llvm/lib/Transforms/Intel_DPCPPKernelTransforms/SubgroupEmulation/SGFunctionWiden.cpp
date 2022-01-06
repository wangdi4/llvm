//===--------------- SGFunctionWiden.cpp - Widen functions ---------------===//
//
// Copyright (C) 2021 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===---------------------------------------------------------------------===//

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/SubgroupEmulation/SGFunctionWiden.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/DIBuilder.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Transforms/Utils/Local.h"

#define DEBUG_TYPE "dpcpp-kernel-sg-emu-function-widen"

using namespace llvm;
using namespace DPCPPKernelCompilationUtils;

void FunctionWidener::run(FuncSet &Functions,
                          DenseMap<Function *, Function *> &FuncMap) {
  if (Functions.empty())
    return;

  FunctionsToWiden = Functions;
  Module &M = *(Functions[0]->getParent());

  // Find all WG sync functions.
  Utils.init(&M);
  WGSyncFunctions = Utils.getAllFunctionsWithSynchronization();

  using namespace DPCPPKernelMetadataAPI;
  auto Kernels = KernelList(M).getList();
  auto KernelRange = make_range(Kernels.begin(), Kernels.end());

  for (auto *Fn : Functions) {
    LLVM_DEBUG(dbgs() << "Expanding Function: " << Fn->getName() << "\n");
    Attribute Attr = Fn->getFnAttribute("vector-variants");
    StringRef VariantsStr = Attr.getValueAsString();
    SmallVector<StringRef, 8> VecVariants;
    VariantsStr.split(VecVariants, ',');
    // TODO: Need to support more than one variants.
    VectorVariant Variant(VecVariants[0]);
    std::string VariantName = Variant.getName().getValue();
    Function *FnWiden = Fn->getParent()->getFunction(VariantName);
    if (FnWiden != nullptr) {
      LLVM_DEBUG(dbgs() << "Skip widening function as the widened version "
                        << "exists: " << VariantName << "\n");
      FuncMap[Fn] = FnWiden;
      continue;
    }

    if (find(KernelRange, Fn) == Kernels.end() && !Fn->isDeclaration())
      removeByValAttr(*Fn);
    ValueToValueMapTy VMap;
    FnWiden = cloneFunction(*Fn, Variant, VMap);
    FuncMap[Fn] = FnWiden;

    // Skip SIMD Intrisincs.
    if (FnWiden->isDeclaration())
      continue;

    // Invalidate helper for Cloned functions.
    Helper.initialize(M);

    // Make sub-group exclude BB, we will insert sth there.
    auto &EntryBB = FnWiden->getEntryBlock();
    std::string BBName = EntryBB.getName().str();
    EntryBB.setName("sg.loop.exclude");
    EntryBB.splitBasicBlock(&*EntryBB.begin(), BBName);

    expandVectorParameters(FnWiden, Variant, VMap);
    bool IsWGSyncFunction = WGSyncFunctions.count(Fn);
    expandReturn(FnWiden, IsWGSyncFunction);
    LLVM_DEBUG(FnWiden->dump());
  }
}

// TODO: We need to add a function(sth like processIncompatibleAttrs) OR a pass
// to handle all incompatible attributes.
void FunctionWidener::removeByValAttr(Function &F) {
  for (auto &Pair : enumerate(F.args())) {
    if (!F.hasParamAttribute(Pair.index(), Attribute::ByVal))
      continue;

    // Create a copy for this argument and update uses.
    Type *PointeeType = F.getParamByValType(Pair.index());

    // Remove byval attribute.
    F.removeParamAttr(Pair.index(), Attribute::ByVal);

    Argument *Arg = &Pair.value();

    if (Arg->user_empty())
      continue;

    Instruction *IP = Helper.getFirstDummyBarrier(&F)->getNextNode();
    assert(IP && "Function doesn't have dummy_sg_barrier");

    PointerType *ArgType = cast<PointerType>(Arg->getType());
    IRBuilder<> Builder(IP);
    auto *ArgPtr =
        Builder.CreateAlloca(PointeeType, ArgType->getAddressSpace());
    auto *ArgPointeeVal = Builder.CreateLoad(PointeeType, Arg);
    Builder.CreateStore(ArgPointeeVal, ArgPtr);
    Arg->replaceUsesWithIf(ArgPtr, [ArgPointeeVal](Use &U) {
      return U.getUser() != ArgPointeeVal;
    });

    // Fix DebugInfo. llvm.dbg.declare attached this parameter describes
    // the original argument.
    DIBuilder DIB(*F.getParent(), false);
    replaceDbgDeclare(Arg, ArgPtr, DIB, 0, 0);
  }
}

Function *FunctionWidener::cloneFunction(Function &F, VectorVariant &V,
                                         ValueToValueMapTy &VMap) {
  FunctionType *OrigFunctionType = F.getFunctionType();
  Type *ReturnType = F.getReturnType();

  // Expand return type to vector.
  if (!ReturnType->isVoidTy())
    ReturnType = SGHelper::getVectorType(ReturnType, V.getVlen());

  std::vector<VectorKind> ParamKinds = V.getParameters();
  SmallVector<Type *, 4> ParamTypes;
  FunctionType::param_iterator ParamIt = OrigFunctionType->param_begin();
  FunctionType::param_iterator ParamEnd = OrigFunctionType->param_end();
  std::vector<VectorKind>::iterator VKIt = ParamKinds.begin();
  for (; ParamIt != ParamEnd; ++ParamIt, ++VKIt) {
    if (VKIt->isVector()) {
      Type *WidenedParamType = SGHelper::getVectorType(*ParamIt, V.getVlen());
      ParamTypes.push_back(WidenedParamType);
    } else {
      ParamTypes.push_back(*ParamIt);
    }
  }

  if (V.isMasked()) {
    // TODO: Use characteristic type once built-ins updated.
    Type *MaskType =
        FixedVectorType::get(Type::getInt32Ty(F.getContext()), V.getVlen());
    ParamTypes.push_back(MaskType);
  }

  FunctionType *CloneFuncType =
      FunctionType::get(ReturnType, ParamTypes, false);

  std::string VariantName = V.getName().getValue();
  Function *Clone = Function::Create(
      CloneFuncType, GlobalValue::ExternalLinkage, VariantName, F.getParent());

  Clone->copyAttributesFrom(&F);

  Function::arg_iterator ArgIt = F.arg_begin();
  Function::arg_iterator ArgEnd = F.arg_end();
  Function::arg_iterator NewArgIt = Clone->arg_begin();
  for (; ArgIt != ArgEnd; ++ArgIt, ++NewArgIt) {
    NewArgIt->setName(ArgIt->getName());
    VMap[&*ArgIt] = &*NewArgIt;
  }

  SmallVector<ReturnInst *, 8> Returns;
  CloneFunctionInto(Clone, &F, VMap, CloneFunctionChangeType::LocalChangesOnly,
                    Returns);

  auto &Context = Clone->getContext();
  auto Attrs = Clone->getAttributes();

  // Remove incompatible argument attributes.
  SmallVector<AttributeSet, 4> ParamAttrs;
  for (auto &Pair : enumerate(Clone->args())) {
    Type *ArgType = Pair.value().getType();
    AttributeMask AB = AttributeFuncs::typeIncompatible(ArgType);
    // The following attributes are rejected in community code.
    // But they are accepted via INTEL_CUSTOMIZATION macro. CodeGen
    // can't process these attributes for vector value.
    // For <VF x i1> zeroext.
    AB.addAttribute(Attribute::ZExt);
    AB.addAttribute(Attribute::SExt);
    // For <VF x pointer> sret.
    AB.addAttribute(Attribute::StructRet);
    AttributeSet ParamAttr = Attrs.getParamAttrs(Pair.index());
    ParamAttrs.push_back(ParamAttr.removeAttributes(Context, AB));
  }

  // FIXME
  auto FnAttrs =
      Attrs.getFnAttrs().removeAttribute(Context, "min-legal-vector-width");

  // Remove incompatible return attributes.
  auto RetAttrs = Attrs.getRetAttrs()
                      .removeAttribute(Context, Attribute::ZExt)
                      .removeAttribute(Context, Attribute::SExt);

  // Update Attributes.
  Clone->setAttributes(
      AttributeList::get(Context, FnAttrs, RetAttrs, ParamAttrs));
  return Clone;
}

bool FunctionWidener::isWideCall(Instruction *I) {
  CallInst *CI = dyn_cast<CallInst>(I);
  if (!CI)
    return false;

  Function *CF = CI->getCalledFunction();
  if (FunctionsToWiden.count(CF))
    return true;

  return false;
}

Instruction *FunctionWidener::getInsertPoint(Instruction *I, Value *V) {

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

void FunctionWidener::expandVectorParameters(Function *Clone, VectorVariant &V,
                                             ValueToValueMapTy &VMap) {

  IRBuilder<> Builder(&*Clone->getEntryBlock().begin());
  ArrayRef<VectorKind> ParamKinds = V.getParameters();

  InstSet SyncInsts = Helper.getSyncInstsForFunction(Clone);

  for (auto &ArgIt : enumerate(Clone->args())) {
    // TODO: For alloca instructions for uniform parameters, we can move the
    // alloca to SGExcludeBB.
    if (!ParamKinds[ArgIt.index()].isVector())
      continue;

    Argument *Arg = &ArgIt.value();

    LLVM_DEBUG(dbgs() << "Expanding vector arg:" << *Arg << "\n");

    // Can't just getScalarType of Arg, original type may be a vector too.
    Type *OrigArgType = nullptr;
    for (const auto &Pair : VMap)
      if (Pair.second == Arg) {
        OrigArgType = Pair.first->getType();
        break;
      }
    assert(OrigArgType && "Can't find original type");

    InstSet InstUsers;
    for (User *U : Arg->users())
      InstUsers.insert(cast<Instruction>(U));

    if (auto *VecOrigType = dyn_cast<FixedVectorType>(OrigArgType)) {
      unsigned OrigNumElements = VecOrigType->getNumElements();
      for (Instruction *I : InstUsers) {
        LLVM_DEBUG(dbgs() << "Updating Use:" << *I << "\n");
        Instruction *IP = getInsertPoint(I, Arg);
        Builder.SetInsertPoint(IP);
        auto *Idx = Helper.createGetSubGroupLId(IP);
        auto *EleId = Builder.CreateMul(Idx, Builder.getInt32(OrigNumElements));
        Value *Val = UndefValue::get(VecOrigType);
        // TODO: Replace this with bitcast and load.
        for (unsigned EleIdx = 0; EleIdx < OrigNumElements; ++EleIdx) {
          auto *EleVal = Builder.CreateExtractElement(Arg, EleId);
          // EleVal might be promoted, create a trunc to original scalar type.
          auto *Trunc = SGHelper::createZExtOrTruncProxy(
              EleVal, OrigArgType->getScalarType(), Builder);
          Val = Builder.CreateInsertElement(Val, Trunc, EleIdx);
          EleId = Builder.CreateAdd(EleId, Helper.getOne());
        }

        I->replaceUsesOfWith(Arg, Val);
        LLVM_DEBUG(dbgs() << "To:" << *I << "\n");
      }
    } else {
      for (Instruction *I : InstUsers) {
        LLVM_DEBUG(dbgs() << "Updating Use:" << *I << "\n");
        Instruction *IP = getInsertPoint(I, Arg);
        Builder.SetInsertPoint(IP);
        auto *Idx = Helper.createGetSubGroupLId(IP);
        auto *Val = Builder.CreateExtractElement(Arg, Idx);
        // The argument element Val might be promoted, create a trunc.
        auto *Trunc =
            SGHelper::createZExtOrTruncProxy(Val, OrigArgType, Builder);
        I->replaceUsesOfWith(Arg, Trunc);
        LLVM_DEBUG(dbgs() << "To:" << *I << "\n");
      }
    }

    // Create a new llvm.dbg.declare which describes the address of original
    // pointer-type argument.
    auto DbgDeclares = FindDbgDeclareUses(Arg);
    if (!DbgDeclares.empty()) {
      IRBuilder<> Builder(&*Clone->getEntryBlock().begin());
      AllocaInst *AddrDbg = Builder.CreateAlloca(OrigArgType, nullptr,
                                                 "dbg.param." + Arg->getName());

      DbgDeclareInst *DI = DbgDeclares.front();
      DIBuilder DIB(*Clone->getParent(), false);
      DILocalVariable *Variable = DI->getVariable();
      DIExpression *Expression =
          DIExpression::prepend(DI->getExpression(), DIExpression::DerefBefore);
      const DILocation *Location = DI->getDebugLoc().get();
      DIB.insertDeclare(AddrDbg, Variable, Expression, Location,
                        AddrDbg->getNextNode());

      for (Instruction *SyncInst : SyncInsts) {
        Instruction *IP = SyncInst->getNextNode();
        // Skip non-loop region.
        if (isWideCall(IP) || isa<ReturnInst>(IP))
          continue;
        Builder.SetInsertPoint(IP);
        auto *Idx = Helper.createGetSubGroupLId(IP);
        auto *EleVal = Builder.CreateExtractElement(Arg, Idx);
        Builder.CreateStore(EleVal, AddrDbg);
      }
    }
    // Remove invalid debug intrinsics.
    for (auto *DI : DbgDeclares)
      DI->eraseFromParent();
  }
}

void FunctionWidener::expandReturn(Function *Clone, bool IsWGSyncFunction) {
  Type *ReturnType = Clone->getReturnType();
  if (ReturnType->isVoidTy())
    return;

  IRBuilder<> Builder(&*Clone->getEntryBlock().begin());

  auto *ReturnPtr = Builder.CreateAlloca(ReturnType, nullptr, "w.ret");

  InstVec RIs;
  for (auto &BB : *Clone) {
    auto *RI = dyn_cast<ReturnInst>(BB.getTerminator());
    if (!RI)
      continue;
    RIs.push_back(RI);
  }

  Type *OrigReturnType = (*RIs.begin())->getOperand(0)->getType();
  auto *VecReturnType = dyn_cast<FixedVectorType>(OrigReturnType);
  unsigned OrigReturnNumElements =
      VecReturnType ? VecReturnType->getNumElements() : 1;
  for (auto *RI : RIs) {
    LLVM_DEBUG(dbgs() << "Expanding return value:" << *RI << "\n");
    Value *OrigVal = RI->getOperand(0);
    Instruction *IP = getInsertPoint(RI, OrigVal);
    Builder.SetInsertPoint(IP);
    auto *Idx = Helper.createGetSubGroupLId(IP);
    // If the original return type is already a vector type, scale the offset.
    Idx = Builder.CreateMul(Idx, Builder.getInt32(OrigReturnNumElements));
    auto *ElePtr =
        Builder.CreateGEP(ReturnType, ReturnPtr, {Helper.getZero(), Idx});
    // ElePtr's pointee type might be different with OrigReturnType if original
    // return type is vector.
    ElePtr =
        Builder.CreatePointerCast(ElePtr, PointerType::get(OrigReturnType, 0));
    Builder.CreateStore(OrigVal, ElePtr);
    // Reset insert point to the terminator.
    Builder.SetInsertPoint(RI);
    auto *WideRet = Builder.CreateLoad(ReturnType, ReturnPtr);
    RI->setOperand(0, WideRet);
  }

  // If this function is a WG sync function, we need to restore the ending
  // barrier before return instruction.
  if (IsWGSyncFunction)
    for (auto *RI : RIs)
      Utils.createBarrier(RI);
}
