// INTEL CONFIDENTIAL
//
// Copyright 2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#include "PipeIOTransformation.h"

#include <llvm/IR/IRBuilder.h>
#include <llvm/Transforms/Utils/ModuleUtils.h>

#include <BuiltinLibInfo.h>
#include <CompilationUtils.h>
#include <InitializePasses.h>
#include <MetadataAPI.h>
#include <OCLAddressSpace.h>
#include <OCLPassSupport.h>

using namespace llvm;
using namespace Intel::MetadataAPI;
using namespace Intel::OpenCL::DeviceBackend;
using namespace Intel::OpenCL::DeviceBackend::ChannelPipeMetadata;

namespace intel {
char PipeIOTransformation::ID = 0;
OCL_INITIALIZE_PASS_BEGIN(PipeIOTransformation, "pipe-io-transformation",
                          "Transform pipes with io attributes", false, true)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
OCL_INITIALIZE_PASS_END(PipeIOTransformation, "pipe-io-transformation",
                        "Transform pipes with io attributes", false, true)
}

#define DEBUG_TYPE "pipe-io-transformation"

namespace {

typedef SmallVector<std::pair<Value *, StringRef>, 4> PipesWithMDVector;
typedef SmallVector<CallInst *, 4> PipesBuiltinsVector;

} // anonymous namespace

namespace intel {

static bool isPipe(const GlobalValue *GV, const Type *PipeTy) {
  auto *GVValueTy = GV->getType()->getElementType();

  if (PipeTy == GVValueTy)
    return true;

  if (auto *GVArrTy = dyn_cast<ArrayType>(GVValueTy)) {
    if (PipeTy == CompilationUtils::getArrayElementType(GVArrTy)) {
      return true;
    }
  }

  return false;
}

static GlobalVariable *createGlobalTextConstant(Module &M,
                                                const StringRef Name) {
  ArrayType *Ty =
      ArrayType::get(Type::getInt8Ty(M.getContext()), Name.size() + 1);
  auto *ConstStringGV = new GlobalVariable(
      M, Ty, /*isConstant=*/true, GlobalValue::PrivateLinkage,
      /*Initializer=*/ConstantDataArray::getString(M.getContext(), Name),
      Name + ".str", /*InsertBefore=*/nullptr);

  ConstStringGV->setUnnamedAddr(GlobalValue::UnnamedAddr::Global);
  const auto &DL = M.getDataLayout();
  ConstStringGV->setAlignment(DL.getPrefTypeAlignment(Ty));
  return ConstStringGV;
}

PipeIOTransformation::PipeIOTransformation() : ModulePass(ID) {}

static void initializeGlobalPipeReleaseCall(Module &M, Function *GlobalDtor,
                                            Function *PipeReleaseFunc,
                                            GlobalVariable *PipeGV) {
  IRBuilder<> Builder(GlobalDtor->getEntryBlock().getTerminator());

  Value *CallArgs[] = {Builder.CreateBitCast(
      PipeGV, PipeReleaseFunc->getFunctionType()->getParamType(0))};

  Builder.CreateCall(PipeReleaseFunc, CallArgs);
}

static Function *createGlobalPipeDtor(Module &M) {
  auto *DtorTy = FunctionType::get(Type::getVoidTy(M.getContext()),
                                   ArrayRef<Type *>(), false);

  Function *Dtor =
      cast<Function>(M.getOrInsertFunction("__pipe_global_dtor", DtorTy));

  Dtor->setLinkage(GlobalValue::ExternalLinkage);

  auto *EntryBB = BasicBlock::Create(M.getContext(), "entry", Dtor);
  ReturnInst::Create(M.getContext(), EntryBB);

  appendToGlobalDtors(M, Dtor, /*Priority=*/65535);

  return Dtor;
}

static bool processGlobalIOPipes(Module &M, Type *PipeTy,
                                 PipesWithMDVector &PipesWithMDVec,
                                 OCLBuiltins &Builtins) {
  bool Changed = false;
  Function *GlobalDtor = nullptr;

  for (auto &PipeGV : M.globals()) {
    if (!isPipe(&PipeGV, PipeTy))
      continue;
    if (PipeGV.hasMetadata() && !PipeGV.getMetadata("io"))
      continue;

    if (!GlobalDtor)
      GlobalDtor = createGlobalPipeDtor(M);

    initializeGlobalPipeReleaseCall(
        M, GlobalDtor, Builtins.get("__pipe_release_intel"), &PipeGV);

    ChannelPipeMD MD = getChannelPipeMetadata(&PipeGV);
    PipesWithMDVec.push_back(std::make_pair(&PipeGV, MD.IO));
    Changed = true;
  }

  return Changed;
}

static bool processIOPipesFromKernelArg(Module &M,
                                        PipesWithMDVector &PipesWithMDVec) {
  bool Changed = false;

  auto KernelsVec = KernelList(M).getList();
  for (auto *Kernel : KernelsVec) {
    auto ArgIOAttributeList = KernelMetadataAPI(Kernel).ArgIOAttributeList;
    if (!ArgIOAttributeList.hasValue())
      continue;
    auto IOList = ArgIOAttributeList.getList();
    auto *it = Kernel->arg_begin();
    for (auto &IO : IOList) {
      Value *Pipe = it++;
      StringRef IOName = IO;
      if (IOName.empty())
        continue;
      PipesWithMDVec.push_back(std::make_pair(Pipe, IOName));
      Changed = true;
    }
  }

  return Changed;
}

static void getPipeBuiltinCalls(PipesBuiltinsVector &PBV, Value *V) {
  for (auto *U : V->users()) {
    if (CallInst *Call = dyn_cast<CallInst>(U)) {
      Function *CF = Call->getCalledFunction();
      assert(CF && "Indirect function call?");
      if (CompilationUtils::isPipeBuiltin(CF->getName())) {
        PBV.push_back(Call);
      }
    } else {
      getPipeBuiltinCalls(PBV, U);
    }
  }
}

static void replacePipeBuiltinCall(CallInst *PipeCall, GlobalVariable *TC,
                                   OCLBuiltins &Builtins) {
  Function *CF = PipeCall->getCalledFunction();
  assert(CF && "Indirect function call");
  PipeKind PK = CompilationUtils::getPipeKind(CF->getName());
  PK.IO = true;

  Function *Builtin = Builtins.get(CompilationUtils::getPipeName(PK));
  FunctionType *FTy = Builtin->getFunctionType();
  Value *Args[] = {
      PipeCall->getArgOperand(0), PipeCall->getArgOperand(1),
      CastInst::CreatePointerCast(TC, FTy->getParamType(2), "", PipeCall)};

  auto PC = CallInst::Create(Builtin, Args, PipeCall->getName(), PipeCall);
  PipeCall->replaceAllUsesWith(PC);
  PipeCall->eraseFromParent();
}

static void replaceIOPipesBuiltins(Module &M, PipesWithMDVector &PipesWithMDVec,
                                   OCLBuiltins &Builtins) {
  for (auto &PipeWithMD : PipesWithMDVec) {
    auto *TC = createGlobalTextConstant(M, PipeWithMD.second);
    PipesBuiltinsVector PBV;
    getPipeBuiltinCalls(PBV, PipeWithMD.first);
    for (auto &PC : PBV) {
      replacePipeBuiltinCall(PC, TC, Builtins);
    }
  }
}

bool PipeIOTransformation::runOnModule(Module &M) {
  auto *PipeValueTy = CompilationUtils::getStructByName("opencl.pipe_t", &M);
  if (!PipeValueTy)
    return false;
  auto *PipeTy = PointerType::get(PipeValueTy, Utils::OCLAddressSpace::Global);
  PipesWithMDVector PipesWithMDVec;
  bool Changed = false;
  BuiltinLibInfo &BLI = getAnalysis<BuiltinLibInfo>();
  OCLBuiltins Builtins(M, BLI.getBuiltinModules());

  Changed |= processGlobalIOPipes(M, PipeTy, PipesWithMDVec, Builtins);
  Changed |= processIOPipesFromKernelArg(M, PipesWithMDVec);

  replaceIOPipesBuiltins(M, PipesWithMDVec, Builtins);

  return Changed;
}

void PipeIOTransformation::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<BuiltinLibInfo>();
}

} // namespace intel

extern "C" {
ModulePass *createPipeIOTransformationPass() {
  return new intel::PipeIOTransformation();
}
}
