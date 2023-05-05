//===- SYCLEqualizer.cpp - DPC++ kernel equalizer --------------------===//
//
// Copyright (C) 2021-2022 Intel Corporation. All rights reserved.
//
// The information and source code contained herein is the exclusive property
// of Intel Corporation and may not be disclosed, examined or reproduced in
// whole or in part without explicit written authorization from the company.
//
//===----------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/SYCLEqualizer.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/Analysis/IteratedDominanceFrontier.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/SYCLTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/LoopUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Utils/ValueMapper.h"

using namespace llvm;
using namespace llvm::CompilationUtils;
using namespace SYCLKernelMetadataAPI;

#define DEBUG_TYPE "sycl-kernel-equalizer"

static cl::opt<bool>
    RemoveFPGAReg("sycl-remove-fpga-reg", cl::init(false), cl::Hidden,
                  cl::desc("Remove __builtin_fpga_reg built-in calls."));

static cl::opt<bool>
    DemangleFPGAPipes("sycl-demangle-fpga-pipes", cl::init(false), cl::Hidden,
                      cl::desc("Remove custom mangling from pipe built-ins"));

namespace {

/// Base class for all functors, which supports immutability query.
class AbstractFunctor {
protected:
  bool IsChanged;

public:
  AbstractFunctor() : IsChanged(false) {}

  virtual ~AbstractFunctor() {}

  bool isChanged() const { return IsChanged; }
};

class FunctionFunctor : public AbstractFunctor {
public:
  virtual void operator()(Function &) = 0;
};

class BlockFunctor : public AbstractFunctor {
public:
  virtual void operator()(BasicBlock &) = 0;
};

// Basic block functors, to be applied on each block in the module.
class MaterializeBlockFunctor : public BlockFunctor {
public:
  MaterializeBlockFunctor(ArrayRef<Module *> BuiltinModules,
                          SmallPtrSetImpl<Function *> &FuncDeclToRemove)
      : BuiltinModules(BuiltinModules), FuncDeclToRemove(FuncDeclToRemove) {}

  void operator()(BasicBlock &BB) override {
    SmallVector<Instruction *, 4> InstToRemove;

    for (auto &I : BB) {
      if (CallInst *CI = dyn_cast<CallInst>(&I)) {
        IsChanged |= changeCallingConv(CI);

        if (RemoveFPGAReg)
          IsChanged |= removeFPGARegInst(CI, InstToRemove, FuncDeclToRemove);

        if (DemangleFPGAPipes)
          IsChanged |=
              demangleFPGAPipeBICall(CI, InstToRemove, FuncDeclToRemove);
      }
    }

    // Remove unused instructions.
    for (auto *I : InstToRemove)
      I->eraseFromParent();
  }

private:
  bool changeCallingConv(CallInst *CI) {
    if ((CallingConv::SPIR_FUNC == CI->getCallingConv()) ||
        (CallingConv::SPIR_KERNEL == CI->getCallingConv())) {
      CI->setCallingConv(CallingConv::C);
      return true;
    }

    return false;
  }

  bool demangleFPGAPipeBICall(CallInst *CI,
                              SmallVectorImpl<Instruction *> &InstToRemove,
                              SmallPtrSetImpl<Function *> &FuncDeclToRemove) {
    auto *F = CI->getCalledFunction();
    if (!F)
      return false;

    StringRef FName = F->getName();
    bool PipeBI = StringSwitch<bool>(FName)
                      .Case("__read_pipe_2", true)
                      .Case("__write_pipe_2", true)
                      .Case("__read_pipe_2_bl", true)
                      .Case("__write_pipe_2_bl", true)
                      .Case("__read_pipe_2_AS0", true)
                      .Case("__read_pipe_2_AS1", true)
                      .Case("__read_pipe_2_AS3", true)
                      .Case("__read_pipe_2_bl_AS0", true)
                      .Case("__read_pipe_2_bl_AS1", true)
                      .Case("__read_pipe_2_bl_AS3", true)
                      .Case("__write_pipe_2_AS0", true)
                      .Case("__write_pipe_2_AS1", true)
                      .Case("__write_pipe_2_AS2", true)
                      .Case("__write_pipe_2_AS3", true)
                      .Case("__write_pipe_2_bl_AS0", true)
                      .Case("__write_pipe_2_bl_AS1", true)
                      .Case("__write_pipe_2_bl_AS2", true)
                      .Case("__write_pipe_2_bl_AS3", true)
                      .Default(false);

    if (!PipeBI)
      return false;

    Module *PipesModule = nullptr;
    for (auto *M : BuiltinModules) {
      if (StructType::getTypeByName(M->getContext(), "struct.__pipe_t")) {
        PipesModule = M;
        break;
      }
    }
    assert(PipesModule && "Module containing pipe built-ins not found");

    assert(CI->arg_size() == 4 && "Unexpected number of arguments");
    SmallVector<Value *, 4> NewArgs;
    NewArgs.push_back(CI->getArgOperand(0));

    IRBuilder<> Builder(CI);

    if (FName.contains("_AS")) {
      FName = FName.drop_back(4);
      auto *Int8Ty = IntegerType::getInt8Ty(PipesModule->getContext());
      // We need to do a cast from global/local/private address spaces to
      // generic due to in backend we have pipe built-ins only with generic
      // address space.
      auto *I8PTy = PointerType::get(Int8Ty, ADDRESS_SPACE_GENERIC);
      auto *ResArg = Builder.CreatePointerBitCastOrAddrSpaceCast(
          CI->getArgOperand(1), I8PTy);
      NewArgs.push_back(ResArg);
    } else {
      // Copy packet argument as-is.
      NewArgs.push_back(CI->getArgOperand(1));
    }

    // Copy rest arguments.
    for (size_t I = 2; I < CI->arg_size(); ++I)
      NewArgs.push_back(CI->getArgOperand(I));

    // Add _fpga suffix to pipe built-ins.
    PipeKind Kind = getPipeKind(FName.str());
    Kind.FPGA = true;
    auto NewFName = getPipeName(Kind);

    Module *M = CI->getModule();
    Function *NewF = M->getFunction(NewFName);
    if (!NewF) {
      if (Kind.Blocking) {
        // Blocking built-ins are not declared in RTL, they are resolved in
        // PipeSupport instead.
        PipeKind NonBlockingKind = Kind;
        NonBlockingKind.Blocking = false;

        // Blocking built-ins differ from non-blocking only by name, so we
        // import a non-blocking function to get a declaration ...
        NewF = importFunctionDecl(
            M, PipesModule->getFunction(getPipeName(NonBlockingKind)),
            /*DuplicateIfExists*/ true);
        NewF->setName(getPipeName(Kind));
      } else {
        NewF = importFunctionDecl(M, PipesModule->getFunction(NewFName));
      }
    }

    for (size_t Idx = 0; Idx < NewArgs.size(); ++Idx) {
      if (auto *PTy = dyn_cast<PointerType>(NewArgs[Idx]->getType())) {
        auto *FArgTy = NewF->getArg(Idx)->getType();
        if (PTy->getAddressSpace() != FArgTy->getPointerAddressSpace())
          NewArgs[Idx] = Builder.CreateAddrSpaceCast(NewArgs[Idx], FArgTy);
      }
    }

    // With materialization of fpga pipe built-in calls, we import new
    // declarations for them, leaving old declarations unused. Add these unused
    // declarations with avoiding of duplications to the list of functions to
    // remove.
    FuncDeclToRemove.insert(F);

    auto *NewCI = Builder.CreateCall(NewF, NewArgs);
    NewCI->setCallingConv(CI->getCallingConv());
    NewCI->setAttributes(CI->getAttributes());
    if (CI->isTailCall())
      NewCI->setTailCall();

    // Replace old call instruction with updated one.
    InstToRemove.push_back(CI);
    if (CI->getType()->isVoidTy()) {
      // SYCL blocking pipe built-ins unlike OpenCL have no return type, so
      // instead of replacing uses of the old instruction - just create a new
      // one.
      assert(Kind.Blocking && "Only blocking pipes can have void return type!");
      return true;
    }
    CI->replaceAllUsesWith(NewCI);

    return true;
  }

  bool removeFPGARegInst(CallInst *CI,
                         SmallVectorImpl<Instruction *> &InstToRemove,
                         SmallPtrSetImpl<Function *> &FuncDeclToRemove) {
    auto *F = CI->getCalledFunction();
    if (!F)
      return false;

    StringRef FName = F->getName();
    if (!FName.startswith("llvm.fpga.reg"))
      return false;

    if (!FName.startswith("llvm.fpga.reg.struct."))
      CI->replaceAllUsesWith(CI->getArgOperand(0));
    else {
      Value *Dst = CI->getArgOperand(0);
      Value *Src = CI->getArgOperand(1);
      Dst->replaceAllUsesWith(Src);
    }

    FuncDeclToRemove.insert(F);
    InstToRemove.push_back(CI);
    return true;
  }

private:
  ArrayRef<Module *> BuiltinModules;
  SmallPtrSetImpl<Function *> &FuncDeclToRemove;
};

// Function functor, to be applied for every function in the module.
// Delegates call to basic-block functors.
class MaterializeFunctionFunctor : public FunctionFunctor {
public:
  MaterializeFunctionFunctor(ArrayRef<Module *> BuiltinModules,
                             SmallPtrSetImpl<Function *> &FuncDeclToRemove)
      : BuiltinModules(BuiltinModules), FuncDeclToRemove(FuncDeclToRemove) {}

  void operator()(Function &F) override {
    CallingConv::ID CConv = F.getCallingConv();
    if (CallingConv::SPIR_FUNC == CConv || CallingConv::SPIR_KERNEL == CConv) {
      F.setCallingConv(CallingConv::C);
      IsChanged = true;
    }
    MaterializeBlockFunctor BBMaterializer(BuiltinModules, FuncDeclToRemove);
    std::for_each(F.begin(), F.end(), BBMaterializer);
    IsChanged |= BBMaterializer.isChanged();
  }

private:
  ArrayRef<Module *> BuiltinModules;
  SmallPtrSetImpl<Function *> &FuncDeclToRemove;
};

/// Type mapper for target extension type.
class TargetExtTypeMapTy : public ValueMapTypeRemapper {
  /// Map from source type to destination type. Source type contains target
  /// extension type, while destination type doesn't.
  DenseMap<Type *, Type *> MappedTypes;

public:
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

// Set block-literal-size attribute for enqueued kernels.
static void setBlockLiteralSizeMetadata(Function &F) {
  SYCLKernelMetadataAPI::KernelInternalMetadataAPI KIMD(&F);
  // Find all enqueue_kernel and kernel query calls.
  for (const auto &EEF : *(F.getParent())) {
    if (!EEF.isDeclaration())
      continue;

    StringRef EEFName = EEF.getName();
    if (!(isEnqueueKernel(EEFName.str()) ||
          EEFName.equals("__get_kernel_work_group_size_impl") ||
          EEFName.equals(
              "__get_kernel_preferred_work_group_size_multiple_impl")))
      continue;

    unsigned BlockInvokeIdx = (EEFName.startswith("__enqueue_kernel_"))
                                  ? (EEFName.contains("_events") ? 6 : 3)
                                  : 0;
    unsigned BlockLiteralIdx = BlockInvokeIdx + 1;

    for (auto *U : EEF.users()) {
      auto *EECall = dyn_cast<CallInst>(U);
      if (!EECall)
        continue;
      Value *BlockInvoke =
          EECall->getArgOperand(BlockInvokeIdx)->stripPointerCasts();
      if (BlockInvoke != &F)
        continue;
      Value *BlockLiteral =
          EECall->getArgOperand(BlockLiteralIdx)->stripPointerCasts();
      int64_t BlockSize = 0;
      if (auto *BlockAlloca = dyn_cast<AllocaInst>(BlockLiteral)) {
        BlockSize = F.getParent()->getDataLayout().getTypeAllocSize(
            BlockAlloca->getAllocatedType());
      } else if (auto *BlockGlobal = dyn_cast<Constant>(BlockLiteral)) {
        auto *BlockGlobalConst = cast<Constant>(BlockGlobal->getOperand(0));
        auto *Size = cast<ConstantInt>(BlockGlobalConst->getOperand(0));
        BlockSize = Size->getZExtValue();
      } else {
        llvm_unreachable("Unexpected instruction");
      }

      KIMD.BlockLiteralSize.set(BlockSize);
      return;
    }
  }
}

// Find kernel and set external linkage.
static auto findKernels(Module &M) {
  assert(!M.getNamedMetadata("sycl.kernels") &&
         "Do not expect sycl.kernels Metadata");

  KernelList::KernelVectorTy Kernels;

  for (auto &F : M) {
    if (F.isDeclaration())
      continue;
    if (F.getCallingConv() != CallingConv::SPIR_KERNEL)
      continue;

    Kernels.push_back(&F);

    // OpenCL/SYCL/SPIR-V kernel could have internal linkage since spec doesn't
    // mandate kernel to have external linkage.
    F.setLinkage(GlobalValue::ExternalLinkage);

    if (F.getName().contains("_block_invoke_") &&
        F.getName().endswith("_kernel")) {
      // Set block-literal-size attribute for enqueued kernels.
      setBlockLiteralSizeMetadata(F);
    }
  }

  return Kernels;
}

// Rename builtin functions that may alias to other functions.
// e.g. intel_sub_group_broadcast --> sub_group_broadcast
static bool renameAliasingBuiltins(Module &M) {
  const static std::unordered_map<std::string, std::string> TrivialMappings = {
      {"sub_group_non_uniform_broadcast", "sub_group_broadcast"},
      {"intel_sub_group_broadcast", "sub_group_broadcast"},
  };

  bool Changed = false;
  for (auto &F : M) {
    // Parse function name with StringRef operations directly.
    // We don't use the demangle API intentionally as we don't need to know
    // type infos here.
    StringRef Name = F.getName();
    if (!Name.consume_front("_Z"))
      continue;
    unsigned EncodedLen = 0;
    if (Name.consumeInteger(10, EncodedLen))
      continue;
    StringRef RawName = Name.substr(0, EncodedLen);
    auto It = TrivialMappings.find(RawName.str());
    if (It == TrivialMappings.end())
      continue;

    LLVM_DEBUG(dbgs() << "Renaming function " << F.getName());
    const std::string &Replacement = It->second;
    F.setName(Twine("_Z") + Twine(Replacement.length()) + Twine(Replacement) +
              Name.substr(EncodedLen));
    LLVM_DEBUG(dbgs() << " as " << F.getName() << '\n');
    Changed = true;
  }

  return Changed;
}

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

/// Replace target extension type with its layout type.
/// Assume target extension type, e.g. pipe and image2d_t, is only allowed for
/// function parameter.
/// FPGA channel global variables are not handled in this function. They'll be
/// handled in ChannelPipeTransformationPass.
static void materializeTargetExtType(Module &M,
                                     KernelList::KernelVectorTy &Kernels) {
  // Retrieve address space for TargetExtType pointer argument.
  TargetExtTypeMapTy TETypeMap;
  for (Function &F : M) {
    SYCLKernelMetadataAPI::KernelMetadataAPI KMD(&F);
    if (KMD.ArgAddrSpaceList.hasValue()) {
      // OpenCL or spirv kernels. Read from kernel_arg_addr_space metadata.
      for (const auto &[Idx, A] : llvm::enumerate(F.args())) {
        if (auto *TETy = dyn_cast<TargetExtType>(A.getType());
            TETy && !TETypeMap.hasMapping(TETy)) {
          auto *Ty = TETy->getLayoutType();
          if (isa<PointerType>(TETy->getLayoutType()))
            Ty = PointerType::get(Ty, KMD.ArgAddrSpaceList.getItem(Idx));
          TETypeMap.addMapping(TETy, Ty);
        }
      }
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
        AddMapping(A.getType());
      AddMapping(F.getReturnType());
    }
  }

  // Find functions with argument of target extension type. Create a new
  // function with new argument of layout type.
  ValueToValueMapTy VMap;
  SmallVector<Function *, 16> FuncsToRemove;
  for (Function &F : M) {
    if (!isa<TargetExtType>(F.getReturnType()) &&
        !llvm::any_of(F.args(), [](Argument &Arg) {
          return isa<TargetExtType>(Arg.getType());
        }))
      continue;

    SmallVector<Type *, 8> NewArgTypes;
    llvm::transform(
        F.args(), std::back_inserter(NewArgTypes),
        [&](Argument &Arg) { return TETypeMap.remapType(Arg.getType()); });
    Type *RetTy = TETypeMap.remapType(F.getReturnType());
    auto *NewFnTy = FunctionType::get(RetTy, NewArgTypes, F.isVarArg());
    auto *NewF = Function::Create(NewFnTy, F.getLinkage(), "", M);
    NewF->setCallingConv(F.getCallingConv());
    NewF->copyMetadata(&F, 0);
    NewF->setDSOLocal(F.isDSOLocal());
    NewF->setComdat(F.getComdat());
    NewF->setAttributes(F.getAttributes());
    NewF->splice(NewF->begin(), &F);
    NewF->takeName(&F);
    VMap[&F] = NewF;
    FuncsToRemove.push_back(&F);

    // Add argument with TargetExtType to VMap.
    if (!NewF->isDeclaration()) {
      for (auto It = F.arg_begin(), E = F.arg_end(), NewIt = NewF->arg_begin();
           It != E; ++It, ++NewIt) {
        NewIt->takeName(&*It);
        auto *TETy = dyn_cast<TargetExtType>(It->getType());
        if (TETy)
          VMap[&*It] = &*NewIt;
        else
          It->replaceAllUsesWith(&*NewIt);
      }
    }
  }

  if (FuncsToRemove.empty()) {
    formArgTypeNullValMetadata(Kernels, VMap);
    return;
  }

  ValueMapper VMapper(VMap, RF_NoModuleLevelChanges | RF_IgnoreMissingLocals,
                      &TETypeMap);
  SmallPtrSet<Type *, 16> VisitedType;

  // Handle each function.
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
    for (auto &Arg : F.args())
      if (VMap.count(&Arg))
        findInstUsers(&Arg, ToRemap, Visited);
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

  // Replace old kernels with new kernels.
  llvm::for_each(Kernels, [&](Function *&F) {
    if (auto It = VMap.find(F); It != VMap.end())
      F = cast<Function>(It->second);
  });

  for (Function *F : FuncsToRemove) {
    assert(F->use_empty() && "function still has use");
    F->eraseFromParent();
  }
}

#if INTEL_CUSTOMIZATION
/// Find functions containing divergent subgroup builtin call where only some of
/// the workitems in a subgroup are active.
FuncSet getFuncWithDivergentSGBuiltin(
    Module &M, function_ref<PostDominatorTree &(Function &)> GetPDT) {
  FuncSet Result;
  Function *GetSGLocalId = nullptr;
  SmallVector<Function *, 8> NonUniformSGFuncs;
  for (auto &F : M) {
    if (!F.isDeclaration())
      continue;
    StringRef FName = F.getName();
    if (isGetSubGroupLocalId(FName))
      GetSGLocalId = &F;
    // TODO check only isSubGroupNonUniformFlow. Pending on CMPLRLLVM-47407
    else if (isSubGroupNonUniformFlow(FName) || isSubGroupAll(FName) ||
             isSubGroupAny(FName) || isSubGroupBroadCast(FName) ||
             isSubGroupReduceAdd(FName) || isSubGroupReduceMin(FName) ||
             isSubGroupReduceMax(FName))
      NonUniformSGFuncs.push_back(&F);
  }
  if (!GetSGLocalId)
    return Result;

  SmallPtrSet<BasicBlock *, 8> BBWithNonUniformSGCalls;
  for (auto *F : NonUniformSGFuncs)
    for (User *U : F->users())
      BBWithNonUniformSGCalls.insert(cast<Instruction>(U)->getParent());

  // Find basic blocks with a conditional branch and the condition is dependent
  // on get_sub_group_local_id call.
  // Only check the case that get_sub_group_local_id call is an operand of the
  // condition instruction, since we're yet to see other use cases.
  SmallVector<BasicBlock *, 8> DivergentRegionStartBB;
  for (User *U : GetSGLocalId->users()) {
    auto *CI = cast<CallInst>(U);
    auto *BB = CI->getParent();
    if (auto *Br = dyn_cast<BranchInst>(BB->getTerminator());
        Br && Br->isConditional())
      if (auto *Cmp = dyn_cast<ICmpInst>(Br->getCondition()))
        if (Cmp->getOperand(0) == CI || Cmp->getOperand(1) == CI)
          DivergentRegionStartBB.push_back(BB);
  }
  // Only one workitem is active in kmp critical region.
  if (Function *KMP_CRITICAL = M.getFunction("__kmpc_critical")) {
    for (User *U : KMP_CRITICAL->users()) {
      auto *BB = cast<CallInst>(U)->getParent();
      DivergentRegionStartBB.push_back(BB);
      if (BBWithNonUniformSGCalls.contains(BB))
        Result.insert(BB->getParent());
    }
  }

  for (auto *BB : DivergentRegionStartBB) {
    Function *F = BB->getParent();
    if (Result.contains(F))
      continue;
    auto &PDT = GetPDT(*F);
    SmallPtrSet<BasicBlock *, 16> SuccNonUniformSG;
    for (auto *Succ : successors(BB))
      if (BBWithNonUniformSGCalls.contains(Succ) && !PDT.dominates(Succ, BB))
        SuccNonUniformSG.insert(Succ);
    if (SuccNonUniformSG.empty())
      continue;
    SmallVector<BasicBlock *, 16> IDFBlocks;
    ReverseIDFCalculator IDF(PDT);
    IDF.setDefiningBlocks(SuccNonUniformSG);
    IDF.calculate(IDFBlocks);
    if (llvm::find(IDFBlocks, BB) != IDFBlocks.end())
      Result.insert(F);
  }

  return Result;
}

/// For OpenMP constructs that aren't supported by vectorizer, set their user
/// kernels' intel_vec_len_hint MD to 1 so that the kernels won't be vectorized.
/// This is a temporary fix until there is a bail out in vectorizer.
static void setNotVectorizeForUnsupportedOmpConstructs(
    Module &M, KernelList::KernelVectorTy &Kernels, TargetLibraryInfo &TLI,
    function_ref<PostDominatorTree &(Function &)> GetPDT) {
  // Refer to LoopVectorizationPlanner::isInvalidOMPConstructInSIMD
  auto IsInvalidOMPConstructInSIMD = [](LibFunc Func) {
    switch (Func) {
    case LibFunc_kmpc_atomic_compare_exchange:
    case LibFunc_kmpc_atomic_fixed4_add:
    case LibFunc_kmpc_atomic_float8_add:
    case LibFunc_kmpc_atomic_load:
    case LibFunc_kmpc_atomic_store:
      return false;
    default:
      return true;
    }
  };
  FuncSet UnsupportedOMPFuncs;
  for (auto &F : M) {
    LibFunc Func;
    if (TLI.getLibFunc(F, Func) && TLI.isOMPLibFunc(Func) &&
        IsInvalidOMPConstructInSIMD(Func))
      UnsupportedOMPFuncs.insert(&F);
  }
  if (UnsupportedOMPFuncs.empty())
    return;

  FuncSet UnsupportedOMPKernels;
  LoopUtils::fillFuncUsersSet(UnsupportedOMPFuncs, UnsupportedOMPKernels);
  if (!UnsupportedOMPKernels.empty()) {
    auto DivergentSGFuncs = getFuncWithDivergentSGBuiltin(M, GetPDT);
    FuncSet DivergentSGKernels;
    LoopUtils::fillFuncUsersSet(DivergentSGFuncs, DivergentSGKernels);
    for (Function *F : Kernels) {
      if (UnsupportedOMPKernels.contains(F) && !DivergentSGFuncs.contains(F) &&
          !DivergentSGKernels.contains(F)) {
        SYCLKernelMetadataAPI::KernelMetadataAPI KMD(F);
        KMD.VecLenHint.set(1);
      }
    }
  }
}
#endif // INTEL_CUSTOMIZATION

PreservedAnalyses SYCLEqualizerPass::run(Module &M, ModuleAnalysisManager &AM) {
  // Find kernel list in the module.
  auto Kernels = findKernels(M);

  // Materialize target extension type.
  materializeTargetExtType(M, Kernels);

  // Set sycl.kernels metadata.
  SYCLKernelMetadataAPI::KernelList KernelList(M);
  KernelList.set(Kernels);

  auto BuiltinModules =
      AM.getResult<BuiltinLibInfoAnalysis>(M).getBuiltinModules();
  SmallPtrSet<Function *, 4> FuncDeclToRemove;
  MaterializeFunctionFunctor FuncMaterializer(BuiltinModules, FuncDeclToRemove);
  // Take care of calling conventions.
  std::for_each(M.begin(), M.end(), FuncMaterializer);
  // Remove unused declarations.
  for (auto *FDecl : FuncDeclToRemove)
    FDecl->eraseFromParent();
  std::ignore = FuncMaterializer.isChanged();

#if INTEL_CUSTOMIZATION
  // Set intel_vec_len_hint metadata before unsupported OpenMP functions are
  // inlined at a later stage.
  if (isGeneratedFromOMP(M) && !Kernels.empty()) {
    auto &FAM =
        AM.getResult<FunctionAnalysisManagerModuleProxy>(M).getManager();
    TargetLibraryInfo TLI = FAM.getResult<TargetLibraryAnalysis>(*Kernels[0]);
    auto GetPDT = [&](Function &F) -> PostDominatorTree & {
      return FAM.getResult<PostDominatorTreeAnalysis>(F);
    };
    setNotVectorizeForUnsupportedOmpConstructs(M, Kernels, TLI, GetPDT);
  }
#endif // INTEL_CUSTOMIZATION

  std::ignore = renameAliasingBuiltins(M);

  // Module is always changed.
  return PreservedAnalyses::none();
}
