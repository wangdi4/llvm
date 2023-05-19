//===- KernelTargetExtTypeLower.cpp ---------------------------------------===//
//
// Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/KernelTargetExtTypeLower.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

using namespace llvm;
using namespace CompilationUtils;
using namespace SYCLKernelMetadataAPI;

#define DEBUG_TYPE "sycl-kernel-target-ext-type-lower"

namespace {

/// Type mapper for target extension type.
class TargetExtTypeMapTy : public ValueMapTypeRemapper {
  /// Map from source type to destination type. Source type contains target
  /// extension type, while destination type doesn't.
  DenseMap<Type *, Type *> MappedTypes;

public:
  bool empty() { return MappedTypes.empty(); }

  void addMapping(Type *SrcTy, Type *DstTy) {
    if (!hasMapping(SrcTy))
      MappedTypes[SrcTy] = DstTy;
  }

  bool hasMapping(Type *SrcTy) const { return MappedTypes.contains(SrcTy); }

  Type *remapType(Type *SrcTy) override {
    auto It = MappedTypes.find(SrcTy);
    return It != MappedTypes.end() ? It->second : SrcTy;
  }

  /// Recursively check if source type contains target extension type. If yes,
  /// add its mapping.
  /// Return mapped type if mapping exists, or source type otherwise.
  Type *get(Type *Ty, SmallPtrSetImpl<Type *> &Visited) {
    if (Visited.contains(Ty))
      return remapType(Ty);

    Visited.insert(Ty);

    if (auto It = MappedTypes.find(Ty); It != MappedTypes.end())
      return It->second;

    if (auto *TETy = dyn_cast<TargetExtType>(Ty)) {
      addMapping(Ty, TETy->getLayoutType());
      return TETy->getLayoutType();
    }

    bool Changed = false;
    SmallVector<Type *, 8> NewEltTypes;
    for (auto *SubTy : Ty->subtypes()) {
      auto *MappedTy = get(SubTy, Visited);
      Changed |= MappedTy != SubTy;
      NewEltTypes.push_back(MappedTy);
    }

    if (!Changed)
      return Ty;

    Type *NewTy;
    switch (Ty->getTypeID()) {
    case Type::ArrayTyID:
      NewTy = ArrayType::get(NewEltTypes[0], Ty->getArrayNumElements());
      break;
    case Type::FixedVectorTyID:
    case Type::ScalableVectorTyID:
      NewTy = VectorType::get(NewEltTypes[0],
                              cast<VectorType>(Ty)->getElementCount());
      break;
    case Type::StructTyID:
      if (cast<StructType>(Ty)->isLiteral())
        NewTy = StructType::get(Ty->getContext(), NewEltTypes,
                                cast<StructType>(Ty)->isPacked());
      else
        NewTy = StructType::create(NewEltTypes, Ty->getStructName(),
                                   cast<StructType>(Ty)->isPacked());
      break;
    default:
      llvm_unreachable("Unhandled derived type to remap");
    }

    addMapping(Ty, NewTy);
    return NewTy;
  }
};

} // namespace

/// Save function parameter's type, including target extension type, to
/// metadata.
static void formArgTypeNullValMetadata(SmallVectorImpl<Function *> &Funcs,
                                       ValueToValueMapTy &VMap) {
  for (Function *F : Funcs) {
    SmallVector<Constant *, 8> ArgTypesMD;
    llvm::transform(F->args(), std::back_inserter(ArgTypesMD), [](Argument &A) {
      return Constant::getNullValue(A.getType());
    });
    if (auto VMapIt = VMap.find(F); VMapIt != VMap.end())
      F = cast<Function>(VMapIt->second);
    SYCLKernelMetadataAPI::KernelInternalMetadataAPI KIMD(F);
    KIMD.ArgTypeNullValList.set(std::move(ArgTypesMD));
  }
}

/// Find all users that are instructions.
static void findInstUsers(Value *Val, SmallPtrSetImpl<Instruction *> &Users,
                          DenseSet<Value *> &Visited) {
  if (Visited.contains(Val))
    return;

  if (auto *I = dyn_cast<Instruction>(Val))
    Users.insert(I);

  if (isa<AllocaInst>(Val)) {
    auto DbgUses = findDbgUses(Val);
    Users.insert(DbgUses.begin(), DbgUses.end());
  }

  SmallVector<Value *, 16> WorkList{Val};
  while (!WorkList.empty()) {
    Value *V = WorkList.pop_back_val();
    Visited.insert(V);
    if (auto *SI = dyn_cast<StoreInst>(V)) {
      auto *Op = SI->getPointerOperand();
      if (!Visited.contains(Op)) {
        WorkList.push_back(Op);
        if (auto *I = dyn_cast<Instruction>(Op))
          Users.insert(I);
      }
    }
    for (User *U : V->users()) {
      for (auto It = df_begin(U), E = df_end(U); It != E;) {
        if (auto *I = dyn_cast<Instruction>(*It)) {
          if (Visited.contains(I)) {
            It.skipChildren();
            continue;
          }
          WorkList.push_back(I);
          Users.insert(I);
        }
        ++It;
      }
    }
  }
}

static Type *getArgType(Argument &A) {
  if (auto *Ty = A.getPointeeInMemoryValueType())
    return Ty;
  return A.getType();
}

static Attribute
getPointeeInMemoryAttrWithNewType(Argument &Arg,
                                  TargetExtTypeMapTy &TETypeMap) {
  auto &Ctx = Arg.getContext();
  if (auto *Ty = Arg.getParamByValType())
    return Attribute::getWithByValType(Ctx, TETypeMap.remapType(Ty));
  if (auto *Ty = Arg.getParamStructRetType())
    return Attribute::getWithStructRetType(Ctx, TETypeMap.remapType(Ty));
  if (auto *Ty = Arg.getParamByRefType())
    return Attribute::getWithByRefType(Ctx, TETypeMap.remapType(Ty));
  if (auto *Ty = Arg.getParamInAllocaType())
    return Attribute::getWithInAllocaType(Ctx, TETypeMap.remapType(Ty));
  llvm_unreachable("unhandled in-memory ABI type");
}

/// Replace target extension type with its layout type.
/// Assume target extension type, e.g. pipe and image2d_t, is only allowed for
/// function parameter.
/// FPGA channel global variables are not handled in this function. They'll be
/// handled in ChannelPipeTransformationPass.
static bool materializeTargetExtType(Module &M,
                                     SmallVectorImpl<Function *> &Kernels) {
  // Retrieve address space for TargetExtType pointer argument.
  TargetExtTypeMapTy TETypeMap;
  SmallPtrSet<Type *, 16> VisitedType;
  for (Function &F : M) {
    SYCLKernelMetadataAPI::KernelMetadataAPI KMD(&F);
    if (KMD.ArgAddrSpaceList.hasValue()) {
      // OpenCL or spirv kernels. Read from kernel_arg_addr_space metadata.
      for (const auto &[Idx, A] : llvm::enumerate(F.args())) {
        if (auto *TETy = dyn_cast<TargetExtType>(getArgType(A));
            TETy && !TETypeMap.hasMapping(TETy)) {
          auto *Ty = TETy->getLayoutType();
          if (isa<PointerType>(TETy->getLayoutType()))
            Ty = PointerType::get(Ty, KMD.ArgAddrSpaceList.getItem(Idx));
          TETypeMap.addMapping(TETy, Ty);
        }
      }
      std::ignore = TETypeMap.get(F.getReturnType(), VisitedType);
    } else {
      // nonspirv kernels and non-kernel functions.
      auto AddMapping = [&](Type *Ty) {
        if (auto *TETy = dyn_cast<TargetExtType>(Ty);
            TETy && !TETypeMap.hasMapping(TETy)) {
          auto *LTy = TETy->getLayoutType();
          if (isa<PointerType>(LTy)) {
            unsigned AS = StringSwitch<unsigned>(TETy->getName())
                              .Case("spirv.DeviceEvent", ADDRESS_SPACE_PRIVATE)
                              .Case("spirv.Event", ADDRESS_SPACE_PRIVATE)
                              .Case("spirv.Queue", ADDRESS_SPACE_PRIVATE)
                              .Case("spirv.Sampler", ADDRESS_SPACE_CONSTANT)
                              .Default(ADDRESS_SPACE_GLOBAL);
            LTy = PointerType::get(LTy, AS);
          }
          TETypeMap.addMapping(TETy, LTy);
        }
      };
      for (auto &A : F.args())
        AddMapping(getArgType(A));
      AddMapping(F.getReturnType());
    }
  }

  // Handle named struct type.
  for (auto *Ty : M.getIdentifiedStructTypes())
    std::ignore = TETypeMap.get(Ty, VisitedType);

  // Find functions with argument of target extension type. Create a new
  // function with new argument of layout type.
  SmallVector<Function *, 32> WorkList;
  for (Function &F : M) {
    if (!TETypeMap.hasMapping(F.getReturnType()) &&
        llvm::all_of(F.args(), [&](Argument &A) {
          return !TETypeMap.hasMapping(getArgType(A));
        })) {
      continue;
    }
    WorkList.push_back(&F);
  }
  ValueToValueMapTy VMap;
  SmallVector<Function *, 16> FuncsToRemove;
  for (Function *F : WorkList) {
    SmallVector<Type *> NewArgTypes(F->arg_size());
    for (const auto &[Idx, A] : llvm::enumerate(F->args())) {
      Type *Ty = getArgType(A);
      if (!TETypeMap.hasMapping(Ty) || A.hasPointeeInMemoryValueAttr())
        Ty = A.getType();
      else
        Ty = TETypeMap.remapType(Ty);
      NewArgTypes[Idx] = Ty;
    }
    Type *RetTy = TETypeMap.remapType(F->getReturnType());
    auto *NewFnTy = FunctionType::get(RetTy, NewArgTypes, F->isVarArg());
    auto *NewF = Function::Create(NewFnTy, F->getLinkage(), "", M);
    NewF->setCallingConv(F->getCallingConv());
    NewF->copyMetadata(F, 0);
    NewF->setDSOLocal(F->isDSOLocal());
    NewF->setComdat(F->getComdat());
    NewF->setAttributes(F->getAttributes());
    NewF->splice(NewF->begin(), F);
    NewF->takeName(F);
    VMap[F] = NewF;
    FuncsToRemove.push_back(F);

    // Add argument with TargetExtType to VMap.
    for (auto It = F->arg_begin(), E = F->arg_end(), NewIt = NewF->arg_begin();
         It != E; ++It, ++NewIt) {
      NewIt->takeName(&*It);
      if (TETypeMap.hasMapping(getArgType(*It))) {
        if (!NewF->isDeclaration())
          VMap[&*It] = &*NewIt;
        if (It->hasPointeeInMemoryValueAttr())
          NewIt->addAttr(getPointeeInMemoryAttrWithNewType(*It, TETypeMap));
      } else if (!NewF->isDeclaration()) {
        It->replaceAllUsesWith(&*NewIt);
      }
    }
  }

  if (TETypeMap.empty()) {
    formArgTypeNullValMetadata(Kernels, VMap);
    return !Kernels.empty();
  }

  // Handle each function.
  ValueMapper VMapper(VMap, RF_NoModuleLevelChanges | RF_IgnoreMissingLocals,
                      &TETypeMap);
  SmallVector<Function *, 32> FuncsToAddMD;
  for (Function &F : M) {
    if (F.isDeclaration()) {
      if (llvm::find(Kernels, &F) == Kernels.end())
        if (auto It = VMap.find(&F);
            It != VMap.end() && !cast<Function>(It->second)->isDeclaration())
          FuncsToAddMD.push_back(&F);
      continue;
    }

    DenseSet<Value *> Visited;
    SmallPtrSet<Instruction *, 32> ToRemap;

    // Find instructions that need to be remapped.
    for (const auto &Pair : VMap) {
      if (dyn_cast<Function>(Pair.second) == &F)
        for (auto &Arg : cast<Function>(Pair.first)->args())
          if (VMap.count(&Arg))
            findInstUsers(const_cast<Argument *>(&Arg), ToRemap, Visited);
    }
    if (F.getName().startswith("__") && F.getName().endswith("_block_invoke"))
      for (auto &Arg : F.args())
        findInstUsers(&Arg, ToRemap, Visited);
    for (Instruction &I : instructions(&F)) {
      if (auto *AI = dyn_cast<AllocaInst>(&I)) {
        Type *AllocTy = AI->getAllocatedType();
        if (TETypeMap.get(AllocTy, VisitedType) != AllocTy)
          findInstUsers(&I, ToRemap, Visited);
      } else if (auto *LI = dyn_cast<LoadInst>(&I)) {
        if (TETypeMap.hasMapping(LI->getType()))
          findInstUsers(&I, ToRemap, Visited);
      } else if (auto *CI = dyn_cast<CallInst>(&I)) {
        if (VMap.count(CI->getCalledFunction()))
          findInstUsers(&I, ToRemap, Visited);
      } else if (auto *GEP = dyn_cast<GetElementPtrInst>(&I)) {
        if (TETypeMap.hasMapping(GEP->getSourceElementType()))
          findInstUsers(&I, ToRemap, Visited);
      }
    }

    // Fix operands with TargetExtType.
    for (auto *I : ToRemap)
      VMapper.remapInstruction(*I);
  }

  // Save target extention type to kernels.
  // Also save for non-kernel functions with argument of target extension type.
  // E.g., fpga channel can be used as argument of non-kernel function.
  FuncsToAddMD.append(Kernels.begin(), Kernels.end());
  formArgTypeNullValMetadata(FuncsToAddMD, VMap);

  for (Function *F : FuncsToRemove) {
    assert(F->use_empty() && "function still has use");
    const_cast<Function *>(F)->eraseFromParent();
  }

  return !FuncsToAddMD.empty() || !TETypeMap.empty();
}

static auto findKernels(Module &M) {
  SmallVector<Function *, 16> Kernels;
  for (auto &F : M)
    if (!F.isDeclaration() && F.getCallingConv() == CallingConv::SPIR_KERNEL)
      Kernels.push_back(&F);
  return Kernels;
}

PreservedAnalyses KernelTargetExtTypeLowerPass::run(Module &M,
                                                    ModuleAnalysisManager &AM) {
  // Find kernel list in the module.
  auto Kernels = findKernels(M);

  // Materialize target extension type.
  bool Changed = materializeTargetExtType(M, Kernels);

  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}
