// INTEL CONFIDENTIAL
//
// Copyright 2019 Intel Corporation.
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
#include "SYCLPipesHack.h"

#include <llvm/ADT/SmallVector.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/Debug.h>

#include <PipeCommon.h>
#include <InitializePasses.h>
#include <OCLPassSupport.h>
#include <OCLAddressSpace.h>
#include <BuiltinLibInfo.h>
#include <CompilationUtils.h>

#include "ChannelPipeTransformation/ChannelPipeUtils.h"

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

#define DEBUG_TYPE "sycl-pipes-hack"

namespace intel {
char SYCLPipesHack::ID = 0;
OCL_INITIALIZE_PASS_BEGIN(SYCLPipesHack, "sycl-pipes-hack",
                    "Hack SYCL pipe objects and wire them to OpenCL pipes",
                    false, true)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfo)
OCL_INITIALIZE_PASS_END(SYCLPipesHack, "sycl-pipes-hack",
                    "Hack SYCL pipe objects and wire them to OpenCL pipes",
                    false, true)

static void findPipeStorageGlobals(Module *M,
                                   SmallVectorImpl<GlobalVariable *> &StorageVars) {
  const char *StorageTypeName = "struct._ZTS19ConstantPipeStorage.ConstantPipeStorage";
  Type *StorageTy = M->getTypeByName(StorageTypeName);
  if (!StorageTy) {
    LLVM_DEBUG(dbgs() << "Could not find pipe storage type \"" << StorageTypeName
                      << "\" in the module\n");
    // Module doesn't have any SYCL program scope pipes, so we have nothing to
    // do.
    return;
  }

  auto *StoragePtrTy =
    PointerType::get(StorageTy, Utils::OCLAddressSpace::Global);

  for (auto &GV : M->globals()) {
    if (GV.getType() == StoragePtrTy) {
      LLVM_DEBUG(dbgs() << "Found SYCL pipe storage: " << GV << "\n");
      StorageVars.push_back(&GV);
    }
  }
}

static ChannelPipeMetadata::ChannelPipeMD
getSYCLPipeMetadata(GlobalVariable *StorageVar) {
  LLVM_DEBUG(dbgs() << "Extracting pipe metadata from: "
             << *StorageVar << "\n");

  // StorageVar is a struct of 3 integers: size, alignment and capacity (depth).
  // Explore its initializer to find out these parameters (they are guaranteed
  // to be constants).
  ConstantStruct *Initializer =
    cast<ConstantStruct>(StorageVar->getInitializer());

  assert(Initializer->getNumOperands() == 3 &&
         "Pipe storage initializer have to contain 3 integer values");
  ConstantInt *Size     = cast<ConstantInt>(Initializer->getOperand(0));
  ConstantInt *Align    = cast<ConstantInt>(Initializer->getOperand(1));
  ConstantInt *Capacity = cast<ConstantInt>(Initializer->getOperand(2));

  LLVM_DEBUG(dbgs() << "Got size(" << *Size << "), align(" << *Align <<
             ") and capacity(" << *Capacity << ")\n");

  // IO is not handled yet: no SPIR-V <-> LLVM IR translation format is defined
  // for IO pipes.
  return { (int) Size->getSExtValue(),
           (int) Align->getSExtValue(),
           (int) Capacity->getSExtValue(),
           /*IO*/ "" };
}


SYCLPipesHack::SYCLPipesHack() : ModulePass(ID) {}

bool SYCLPipesHack::runOnModule(Module &M) {
  BuiltinLibInfo &BLI = getAnalysis<BuiltinLibInfo>();
  OCLBuiltins Builtins(M, BLI.getBuiltinModules());

  // SYCL Program scope pipes are currently represented by global struct with 3
  // i32 values: size, align and capacity. We need to find these global structs
  // and replace with %opencl.pipe_rw_t objects to utilize the rest of pipe
  // related passes without any modifications.
  SmallVector<GlobalVariable *, 16> StorageVars;
  findPipeStorageGlobals(&M, StorageVars);
  if (StorageVars.empty()) {
    return false;
  }

  auto RWPipeTyName = "opencl.pipe_rw_t";
  auto *RWPipeValueTy = M.getTypeByName(RWPipeTyName);
  if (!RWPipeValueTy)
    RWPipeValueTy = StructType::create(M.getContext(), RWPipeTyName);
  auto *RWPipeTy = PointerType::get(RWPipeValueTy, Utils::OCLAddressSpace::Global);

  Function *GlobalCtor = nullptr;

  bool Changed = false;
  for (auto *GV : StorageVars) {
    // For each SYCL program scope pipe storage we create an opaque pointer that
    // is going to point to an implementation defined memory.
    auto *RWPipeGV = new GlobalVariable(
        M, RWPipeTy, /*isConstant=*/false, GlobalValue::ExternalLinkage,
        /*Initializer=*/nullptr, GV->getName() + ".syclpipe",
        /*InsertBefore=*/nullptr,
        GlobalValue::ThreadLocalMode::NotThreadLocal,
        Utils::OCLAddressSpace::Global);
    RWPipeGV->setInitializer(ConstantPointerNull::get(cast<PointerType>(RWPipeTy)));
    RWPipeGV->setAlignment(M.getDataLayout().getPreferredAlignment(RWPipeGV));

    // Pipe parameters are hidden inside of the {i32, i32, i32} struct, so we
    // deconstruct it and set it as a metadata so other passes can check it as
    // with FPGA OpenCL pipes.
    ChannelPipeMetadata::ChannelPipeMD PipeMD = getSYCLPipeMetadata(GV);
    setPipeMetadata(RWPipeGV, PipeMD);

    // Program scope Pipe object has to be initialized at runtime after a
    // program is loaded into memory. We use a global ctor function to do that.
    if (!GlobalCtor) {
      GlobalCtor = createGlobalPipeCtor(M);
    }

    // Emit a code that adds necessary initialization to the global ctor.
    initializeGlobalPipeScalar(RWPipeGV,
                               PipeMD,
                               GlobalCtor,
                               Builtins.get("__pipe_init_fpga"));

    // Finally we replace all usages of {i32, i32, i32} struct with our new
    // %opencl.pipe_rw_t global. It is expected that all usages are the calls to
    // __spirv_CreatePipeFromPipeStorage.
    //
    // This is not always the case, especially when IR is optimized, but we are
    // not going to handle all cases, hence the name of the pass is
    // SYCLPipesHack.
    //
    // If you see any of the following asserts firing, then the time to replace
    // this hack with a proper solution has finally come.
#ifndef NDEBUG
    for (auto *U : GV->users()) {
      CastInst *Cast = cast<CastInst>(U);
      for (auto *CastU : Cast->users()) {
        CallInst *CI = cast<CallInst>(CastU);
        Function *F = CI->getCalledFunction();
        assert(F && "Indirect call is not expected");
        assert(F->getName().find("__spirv_CreatePipeFromPipeStorage") !=
               StringRef::npos);
      }
    }
#endif

    Constant *Bitcast =
        ConstantExpr::getBitCast(RWPipeGV, GV->getType());
    LLVM_DEBUG(dbgs() << "Replacing pipe storage pointer (" << *GV
               << ") with read-write pipe (" << *Bitcast << "\n");
    GV->replaceAllUsesWith(Bitcast);
    Changed = true;
  }
  return Changed;
}

void SYCLPipesHack::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<BuiltinLibInfo>();
}

} // namespace intel

extern "C" ModulePass *createSYCLPipesHackPass() {
  return new intel::SYCLPipesHack();
}
