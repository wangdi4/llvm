//===----- RemoveDTransTypeMetadata.cpp - Remove DTrans type metadata -----===//
//
// Copyright (C) 2022-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "Intel_DTrans/Transforms/RemoveDTransTypeMetadata.h"

#include "Intel_DTrans/Analysis/DTransTypeMetadataBuilder.h"
#include "Intel_DTrans/Analysis/DTransTypeMetadataConstants.h"
#include "Intel_DTrans/Analysis/DTransTypes.h"
#include "Intel_DTrans/Analysis/DTransUtils.h"
#include "Intel_DTrans/Analysis/TypeMetadataReader.h"
#include "Intel_DTrans/DTransCommon.h"
#include "llvm/Analysis/Intel_WP.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Operator.h"
#include "llvm/Pass.h"
#include "llvm/Support/Debug.h"

#define TRACE_DEAD "remove-dead-dtranstypemetadata"

using namespace llvm;
using namespace dtransOP;

namespace {
class RemoveAllDTransTypeMetadata {
public:
  bool run(Module &M) {
    NamedMDNode *DTMDTypes = TypeMetadataReader::getDTransTypesMetadata(M);
    if (!DTMDTypes)
      return false;

    for (GlobalVariable &GV : M.globals())
      DTransTypeMetadataBuilder::addDTransMDNode(GV, nullptr);

    for (Function &F : M) {
      DTransTypeMetadataBuilder::setDTransFuncMetadata(&F, nullptr);

      for (auto &I : instructions(F)) {
        if (auto *AI = dyn_cast<AllocaInst>(&I)) {
          DTransTypeMetadataBuilder::addDTransMDNode(*AI, nullptr);
        } else if (auto *Call = dyn_cast<CallBase>(&I)) {
          if (Call->isIndirectCall())
            DTransTypeMetadataBuilder::addDTransMDNode(*Call, nullptr);
        }
      }
    }

    M.eraseNamedMetadata(DTMDTypes);
    return true;
  }
};

class RemoveDeadDTransTypeMetadata {
public:
public:
  bool run(Module &M) {
    NamedMDNode *DTMDTypes = TypeMetadataReader::mapStructsToMDNodes(
        M, MDStructDescriptorMap, /*IncludeOpaque=*/true);
    if (!DTMDTypes)
      return false;

    DTransTypeManager TM(M.getContext());
    TypeMetadataReader Reader(TM);
    // We are intentionally ignoring the return value from the initialize call
    // here because is just reading the elements that have DTrans type metadata
    // without trying to interpret the types, so it is ok if there is incomplete
    // metadata.
    (void)Reader.initialize(M);

    // Find all the structure types used in the IR to collect which
    // DTransStructTypes will need to be preserved when generating a new
    // !intel.dtrans.types metadata node.
    for (GlobalVariable &GV : M.globals()) {
      // The type for the global may need to be taken from either the
      // value type, or the metadata used to describe a global pointer.
      incorporateType(TM, GV.getValueType());
      incorporateValue(Reader, &GV);

      // We also need to look at ConstantExpr users of the variable because the
      // variable may be used with a GEPOperator as a different type than what
      // it was declared as.
      for (auto *U : GV.users())
        if (auto *CE = dyn_cast<ConstantExpr>(U))
          visitConstExpr(TM, CE);
    }

    for (Function &F : M) {
      incorporateType(TM, F.getFunctionType());
      incorporateValue(Reader, &F);

      // Check types used within attributes (sret, byval, ...)
      auto Attrs = F.getAttributes();
      for (unsigned i = 0; i < Attrs.getNumAttrSets(); ++i) {
        for (int AttrIdx = Attribute::FirstTypeAttr;
             AttrIdx <= Attribute::LastTypeAttr; AttrIdx++) {
          Attribute::AttrKind TypedAttr = (Attribute::AttrKind)AttrIdx;
          if (Attrs.hasAttributeAtIndex(i, TypedAttr))
            if (Type *Ty =
                    Attrs.getAttributeAtIndex(i, TypedAttr).getValueAsType())
              incorporateType(TM, Ty);
        }
      }

      for (auto &I : instructions(F)) {
        if (auto *AI = dyn_cast<AllocaInst>(&I)) {
          // The type from the alloca may need to be taken from either the
          // Alloca result type, or the metadata used to describe it.
          incorporateType(TM, AI->getAllocatedType());
          incorporateValue(Reader, AI);
        } else if (auto *GEP = dyn_cast<GetElementPtrInst>(&I)) {
          incorporateType(TM, GEP->getSourceElementType());
        } else if (auto *LI = dyn_cast<LoadInst>(&I)) {
          incorporateType(TM, LI->getType());
        } else if (auto *SI = dyn_cast<StoreInst>(&I)) {
          incorporateType(TM, SI->getValueOperand()->getType());
        } else if (auto *Call = dyn_cast<CallBase>(&I)) {
          if (Call->isIndirectCall())
            incorporateValue(Reader, Call);
        }
      }
    }

    DEBUG_WITH_TYPE(TRACE_DEAD, printResults());

    // Rewrite the !intel.dtrans.types metadata node.
    DTMDTypes->clearOperands();
    for (auto &KV : MDStructDescriptorMap)
      if (IRReferencedStructs.contains(KV.first))
        DTMDTypes->addOperand(KV.second);

    return true;
  }

private:
  // Update 'IRReferencedStructs' with structure types reachable from 'Ty'
  void incorporateType(DTransTypeManager &TM, llvm::Type *Ty) {
    SmallVector<llvm::Type *, 16> Worklist;
    Worklist.push_back(Ty);
    while (!Worklist.empty()) {
      llvm::Type *BaseType = Worklist.pop_back_val();
      while (BaseType->isArrayTy())
        BaseType = BaseType->getArrayElementType();

      // For a FunctionType, add the types used to the worklist, in case a
      // whole structure is passed or returned. For a StructType, we will walk
      // the DTransType metadata to determine identify all the effective types
      // that can be referenced from them.
      if (auto *FTy = dyn_cast<FunctionType>(BaseType)) {
        Worklist.push_back(FTy->getReturnType());
        for (const auto &ParmTy : FTy->params())
          Worklist.push_back(ParmTy);
      } else if (auto *StTy = dyn_cast<StructType>(BaseType)) {
        if (StTy->hasName()) {
          DTransType *DStTy = TM.getStructType(StTy->getName());
          if (DStTy)
            incorporateDTransType(DStTy);
        }
      }
    }
  }

  // Add any structure types used from GEPOperators
  void visitConstExpr(DTransTypeManager &TM, ConstantExpr *CE) {
    if (auto *GEPOp = dyn_cast<GEPOperator>(CE))
      incorporateType(TM, GEPOp->getSourceElementType());

    for (auto *U : CE->users())
      if (auto *UCE = dyn_cast<ConstantExpr>(U))
        visitConstExpr(TM, UCE);
  }

  // Traverse the DTransType, updating 'IRReferencedStructs' to contain all
  // StructTypes that are reachable.
  void incorporateDTransType(DTransType *DTy) {
    if (!Visited.insert(DTy).second)
      return;

    if (auto *DStTy = dyn_cast<DTransStructType>(DTy)) {
      IRReferencedStructs.insert(cast<llvm::StructType>(DStTy->getLLVMType()));
      for (auto &Field : DStTy->elements()) {
        DTransType *FieldTy = Field.getType();
        if (!FieldTy)
          continue;
        incorporateDTransType(FieldTy);
      }
    } else if (auto *DPtrTy = dyn_cast<DTransPointerType>(DTy)) {
      DTransType *DPointeeTy = DPtrTy->getPointerElementType();
      incorporateDTransType(DPointeeTy);
    } else if (auto *DSeqTy = dyn_cast<DTransSequentialType>(DTy)) {
      DTransType *DElemTy = DSeqTy->getElementType();
      incorporateDTransType(DElemTy);
    } else if (auto *DFuncTy = dyn_cast<DTransFunctionType>(DTy)) {
      DTransType *DRetTy = DFuncTy->getReturnType();
      if (DRetTy)
        incorporateDTransType(DRetTy);
      for (auto *DArgTy : DFuncTy->args()) {
        if (DArgTy)
          incorporateDTransType(DArgTy);
      }
    }
  }

  // If 'V' has DTrans type metadata attached to it, update
  // 'IRReferencedStructs'
  void incorporateValue(TypeMetadataReader &Reader, Value *V) {
    DTransType *DType = Reader.getDTransTypeFromMD(V);
    if (!DType)
      return;
    incorporateDTransType(DType);
  }

#if !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)
  void printResults() const {
    std::vector<std::pair<StructType *, MDNode *>> StructsAndMD;
    StructsAndMD.reserve(MDStructDescriptorMap.size());
    std::copy(MDStructDescriptorMap.begin(), MDStructDescriptorMap.end(),
              std::back_inserter(StructsAndMD));
    llvm::sort(StructsAndMD,
               [](const std::pair<StructType *, MDNode *> &Entry1,
                  const std::pair<StructType *, MDNode *> &Entry2) {
                 StructType *Ty1 = Entry1.first;
                 StructType *Ty2 = Entry2.first;
                 return dtrans::compareStructName(Ty1, Ty2);
               });

    dbgs() << "Remove dead DTrans type metadata results:\n";
    for (auto &KV : StructsAndMD) {
      bool Found = IRReferencedStructs.contains(KV.first);
      dbgs() << (Found ? "Kept     : " : "Discarded: ") << *KV.first << "\n";
    }
    dbgs() << "\n";
  }
#endif // !defined(NDEBUG) || defined(LLVM_ENABLE_DUMP)

  // Mapping from structure types to the DTrans metadata node that describes the
  // structure. These need to be kept in a MapVector to preserve the iteration
  // order for creating the new !intel.dtrans.types list for testing purposes.
  MapVector<StructType *, MDNode *> MDStructDescriptorMap;

  // Set of structure types that we need to maintain the DTrans type metadta
  // for. These are structures that are directly used in the IR, or can be
  // reached from a structure in the IR.
  DenseSet<StructType *> IRReferencedStructs;

  // Set to avoid traversing a type more than once.
  DenseSet<DTransType *> Visited;
};

class RemoveAllDTransTypeMetadataWrapper : public ModulePass {
public:
  static char ID;

  RemoveAllDTransTypeMetadataWrapper() : ModulePass(ID) {
    initializeRemoveAllDTransTypeMetadataWrapperPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override { return Impl.runImpl(M); }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<WholeProgramWrapperPass>();
  }

private:
  dtransOP::RemoveAllDTransTypeMetadataPass Impl;
};

class RemoveDeadDTransTypeMetadataWrapper : public ModulePass {
public:
  static char ID;

  RemoveDeadDTransTypeMetadataWrapper() : ModulePass(ID) {
    initializeRemoveDeadDTransTypeMetadataWrapperPass(
        *PassRegistry::getPassRegistry());
  }

  bool runOnModule(Module &M) override { return Impl.runImpl(M); }

  void getAnalysisUsage(AnalysisUsage &AU) const override {
    AU.addPreserved<WholeProgramWrapperPass>();
  }

private:
  dtransOP::RemoveDeadDTransTypeMetadataPass Impl;
};

} // end anonymous namespace

namespace llvm {
namespace dtransOP {

PreservedAnalyses
RemoveAllDTransTypeMetadataPass::run(Module &M, ModuleAnalysisManager &MAM) {
  bool Changed = runImpl(M);
  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

bool RemoveAllDTransTypeMetadataPass::runImpl(Module &M) {
  RemoveAllDTransTypeMetadata Transform;
  return Transform.run(M);
}

PreservedAnalyses
RemoveDeadDTransTypeMetadataPass::run(Module &M, ModuleAnalysisManager &MAM) {
  bool Changed = runImpl(M);
  if (!Changed)
    return PreservedAnalyses::all();

  PreservedAnalyses PA;
  PA.preserve<WholeProgramAnalysis>();
  return PA;
}

bool RemoveDeadDTransTypeMetadataPass::runImpl(Module &M) {
  RemoveDeadDTransTypeMetadata Transform;
  return Transform.run(M);
}

} // end namespace dtransOP
} // end namespace llvm

char RemoveAllDTransTypeMetadataWrapper::ID = 0;
INITIALIZE_PASS(RemoveAllDTransTypeMetadataWrapper,
                "remove-all-dtranstypemetadata",
                "Remove all DTrans type metadata", false, false)

ModulePass *llvm::createRemoveAllDTransTypeMetadataWrapperPass() {
  return new RemoveAllDTransTypeMetadataWrapper();
}

char RemoveDeadDTransTypeMetadataWrapper::ID = 0;
INITIALIZE_PASS(RemoveDeadDTransTypeMetadataWrapper,
                "remove-dead-dtranstypemetadata",
                "Remove dead DTrans type metadata", false, false)

ModulePass *llvm::createRemoveDeadDTransTypeMetadataWrapperPass() {
  return new RemoveDeadDTransTypeMetadataWrapper();
}
