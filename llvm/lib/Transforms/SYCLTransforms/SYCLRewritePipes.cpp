//==- SYCLRewritePipes.cpp - Rewrite SYCL pipe structs to OpenCL structs ==//
//
// Copyright (C) 2022 Intel Corporation
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
// ===--------------------------------------------------------------------===//

#include "llvm/Transforms/SYCLTransforms/SYCLRewritePipes.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Casting.h"
#include "llvm/Transforms/SYCLTransforms/BuiltinLibInfoAnalysis.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/SYCLTransforms/Utils/RuntimeService.h"
#include "llvm/Transforms/SYCLTransforms/Utils/SYCLChannelPipeUtils.h"

using namespace llvm;
using namespace SYCLChannelPipeUtils;
using namespace CompilationUtils;

#define DEBUG_TYPE "sycl-kernel-rewrite-pipes"

const StringRef CreatePipeFromPipeStorageWriteName =
    "_Z39__spirv_CreatePipeFromPipeStorage_write";
const StringRef CreatePipeFromPipeStorageReadName =
    "_Z38__spirv_CreatePipeFromPipeStorage_read";
const StringRef CreatePipeFromPipeStorageWriteTargetName =
    "_Z39__spirv_CreatePipeFromPipeStorage_writePU3AS427"
    "__spirv_ConstantPipeStorage";
const StringRef CreatePipeFromPipeStorageReadTargetName =
    "_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS427"
    "__spirv_ConstantPipeStorage";

static void
collectCreatePipeFuncs(Module &M,
                       SmallVectorImpl<Function *> &CreatePipeFuncs) {
  for (auto &F : M) {
    StringRef Name = F.getName();
    if (Name.startswith(CreatePipeFromPipeStorageWriteName) ||
        Name.startswith(CreatePipeFromPipeStorageReadName))
      CreatePipeFuncs.push_back(&F);
  }
}

static void
fixCreatePipeFuncName(SmallVectorImpl<Function *> &CreatePipeFuncs) {
  for (auto *F : CreatePipeFuncs) {
    if (F->getName().startswith(CreatePipeFromPipeStorageWriteName))
      F->setName(CreatePipeFromPipeStorageWriteTargetName);
    if (F->getName().startswith(CreatePipeFromPipeStorageReadName))
      F->setName(CreatePipeFromPipeStorageReadTargetName);
  }
}

static void
collectSYCLPipeStorageGlobals(SmallVectorImpl<Function *> &CreatePipeFuncs,
                              SmallVectorImpl<GlobalVariable *> &StorageVars) {
  for (auto *F : CreatePipeFuncs) {
    for (auto *U : F->users()) {
      if (!isa<CallInst>(U))
        continue;
      auto *CI = cast<CallInst>(U);
      assert(CI->arg_size() == 1 &&
             "Expect __spirv_CreatePipeFromPipeStorage to have 1 argument");
      // Get PipeStorage GV, it might be hidden by several pointer casts.
      // Strip them.
      auto *PipeStorageArg = CI->getArgOperand(0);
      assert(PipeStorageArg && "Failed to obtain an argument");
      auto *PipeStorageGV =
          cast<GlobalVariable>(PipeStorageArg->stripPointerCasts());
      LLVM_DEBUG(dbgs() << "Found SYCL pipe storage: " << *PipeStorageGV
                        << "\n");
      StorageVars.emplace_back(PipeStorageGV);
    }
  }
}

static void
rewritePipeStorageVars(Module &M,
                       SmallVectorImpl<GlobalVariable *> &StorageVars,
                       RuntimeService &RTS) {
  if (StorageVars.empty())
    return;

  const StringRef OCLPipeRWTypeName = "opencl.pipe_rw_t";
  auto *OCLPipeRWType =
      StructType::getTypeByName(M.getContext(), OCLPipeRWTypeName);
  if (!OCLPipeRWType)
    OCLPipeRWType = StructType::create(M.getContext(), OCLPipeRWTypeName);
  auto *OCLPipeRWPtrType =
      OCLPipeRWType->getPointerTo(AddressSpace::ADDRESS_SPACE_GLOBAL);

  Function *GlobalCtor = nullptr;

  for (auto *StorageVar : StorageVars) {
    // For each SYCL program scope pipe storage we create an opaque pointer that
    // is going to point to an implementation defined memory.
    auto *OCLPipeGV = new GlobalVariable(
        M, OCLPipeRWPtrType, /*isConstant*/ false, GlobalValue::ExternalLinkage,
        /*Initializer*/ nullptr, StorageVar->getName() + ".syclpipe",
        /*InsertBefore*/ nullptr, GlobalValue::ThreadLocalMode::NotThreadLocal,
        AddressSpace::ADDRESS_SPACE_GLOBAL);
    OCLPipeGV->setInitializer(ConstantPointerNull::get(OCLPipeRWPtrType));
    OCLPipeGV->setAlignment(M.getDataLayout().getPreferredAlign(OCLPipeGV));

    // Pipe parameters are hidden inside of the {i32, i32, i32} struct, so we
    // deconstruct it and set it as a metadata so other passes can check it as
    // with FPGA OpenCL pipes.
    ChannelPipeMD PipeMD;
    getSYCLPipeMetadata(StorageVar, PipeMD);
    setPipeMetadata(OCLPipeGV, PipeMD);

    // Program scope Pipe object has to be initialized at runtime after a
    // program is loaded into memory. We use a global ctor function to do that.
    if (!GlobalCtor)
      GlobalCtor = createPipeGlobalCtor(M);

    // Emit a code that adds necessary initialization to the global ctor.
    Function *PipeInitFunc = importFunctionDecl(
        &M, RTS.findFunctionInBuiltinModules(PipeMD.Protocol < 0
                                                 ? "__pipe_init_fpga"
                                                 : "__pipe_init_ext_fpga"));
    initializeGlobalPipeScalar(OCLPipeGV, PipeMD, GlobalCtor, PipeInitFunc);

    // Finally we replace all usages of {i32, i32, i32} struct with our new
    // %opencl.pipe_rw_t global. It is expected that all usages are the calls to
    // __spirv_CreatePipeFromPipeStorage.
    //
    // This is not always the case, especially when IR is optimized, but we are
    // not going to handle all cases.
    //
    // If you see any of the following asserts firing, then the time to replace
    // this hack with a proper solution has finally come.
#ifndef NDEBUG
    std::function<bool(Value *, int)> ReplaceChecker = [&](Value *Value,
                                                           int Depth) {
      if (Depth == 0) {
        return false;
      }
      if (auto *CI = dyn_cast<CallInst>(Value)) {
        Function *F = CI->getCalledFunction();
        assert(F && "Indirect call is not expected");
        assert(F->getName().find(CreatePipeFromPipeStorageWriteName) !=
                   StringRef::npos ||
               F->getName().find(CreatePipeFromPipeStorageReadName) !=
                   StringRef::npos);
        return true;
      }
      for (auto *U : Value->users()) {
        if (!ReplaceChecker(U, Depth - 1)) {
          return false;
        }
      }
      return true;
    };
    assert(ReplaceChecker(StorageVar, 5) &&
           "The usage of pipe storage is not expected");
#endif

    Constant *Bitcast =
        ConstantExpr::getBitCast(OCLPipeGV, StorageVar->getType());
    LLVM_DEBUG(dbgs() << "Replacing pipe storage pointer (" << *StorageVar
                      << ") with read-write pipe (" << *Bitcast << "\n");
    StorageVar->replaceAllUsesWith(Bitcast);
  }
}

bool SYCLRewritePipesPass::runImpl(Module &M, BuiltinLibInfo *BLI) {
  assert(BLI && "Invalid BuiltinLibInfo!");
  SmallVector<Function *, 2> CreatePipeFuncs;
  collectCreatePipeFuncs(M, CreatePipeFuncs);
  if (CreatePipeFuncs.empty())
    return false;
  // FIXME: this workaround should be removed when llvm-spirv can translate the
  // name correctly.
  fixCreatePipeFuncName(CreatePipeFuncs);

  SmallVector<GlobalVariable *, 2> StorageVars;
  collectSYCLPipeStorageGlobals(CreatePipeFuncs, StorageVars);
  rewritePipeStorageVars(M, StorageVars, BLI->getRuntimeService());
  return true;
}

PreservedAnalyses SYCLRewritePipesPass::run(Module &M,
                                             ModuleAnalysisManager &AM) {
  auto *BLI = &AM.getResult<BuiltinLibInfoAnalysis>(M);
  if (!runImpl(M, BLI))
    return PreservedAnalyses::all();
  return PreservedAnalyses::none();
}
