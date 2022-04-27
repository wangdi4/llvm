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

#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/InitializePasses.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/BuiltinLibInfoAnalysis.h"

#include "ChannelPipeTransformation/ChannelPipeUtils.h"
#include "CompilationUtils.h"
#include "InitializePasses.h"
#include "OCLAddressSpace.h"
#include "OCLPassSupport.h"
#include "PipeCommon.h"

#include <string>
#include <set>

using namespace llvm;
using namespace Intel::OpenCL::DeviceBackend;

#define DEBUG_TYPE "sycl-pipes-hack"

namespace intel {
char SYCLPipesHack::ID = 0;
OCL_INITIALIZE_PASS_BEGIN(SYCLPipesHack, "sycl-pipes-hack",
                    "Hack SYCL pipe objects and wire them to OpenCL pipes",
                    false, false)
OCL_INITIALIZE_PASS_DEPENDENCY(BuiltinLibInfoAnalysisLegacy)
OCL_INITIALIZE_PASS_END(SYCLPipesHack, "sycl-pipes-hack",
                    "Hack SYCL pipe objects and wire them to OpenCL pipes",
                    false, false)

const char *CreatePipeFromPipeStorageWriteName =
    "_Z39__spirv_CreatePipeFromPipeStorage_write";
const char *CreatePipeFromPipeStorageReadName =
    "_Z38__spirv_CreatePipeFromPipeStorage_read";
const char *CreatePipeFromPipeStorageWriteTargetName =
    "_Z39__spirv_CreatePipeFromPipeStorage_writePU3AS427"
    "__spirv_ConstantPipeStorage";
const char *CreatePipeFromPipeStorageReadTargetName =
    "_Z38__spirv_CreatePipeFromPipeStorage_readPU3AS427"
    "__spirv_ConstantPipeStorage";

static void
findPipeStorageGlobals(Module *M,
                       SmallVector<GlobalVariable *, 8> &StorageVars) {
  for (auto &F : *M) {
    StringRef FName = F.getName();
    if (!FName.startswith(CreatePipeFromPipeStorageWriteName) &&
        !FName.startswith(CreatePipeFromPipeStorageReadName))
      continue;

    for (auto *User : F.users()) {
      auto *Call = dyn_cast<CallInst>(User);
      if (!Call)
        continue;
      assert(Call->arg_size() == 1 &&
             "Expect __spirv_CreatePipeFromPipeStorage to have 1 argument");
      // Get PipeStorage GV, it might be hidden by several pointer casts.
      // Strip them.
      Value *PipeStorageArg = Call->getArgOperand(0);
      assert(PipeStorageArg && "Failed to obtain an argument");
      GlobalVariable *PipeStorageGV =
          dyn_cast<GlobalVariable>(PipeStorageArg->stripPointerCasts());
      assert(PipeStorageGV && "PipeStorage should be a GV");
      LLVM_DEBUG(dbgs() << "Found SYCL pipe storage: " << *PipeStorageGV <<
                           "\n");
      StorageVars.emplace_back(PipeStorageGV);
    }

    if (FName.startswith(CreatePipeFromPipeStorageWriteName))
      F.setName(CreatePipeFromPipeStorageWriteTargetName);
    if (FName.startswith(CreatePipeFromPipeStorageReadName))
      F.setName(CreatePipeFromPipeStorageReadTargetName);
  }
}

static void getSYCLPipeMetadata(GlobalVariable *StorageVar,
                                ChannelPipeMetadata::ChannelPipeMD& PipeMD) {
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

  if (MDNode *IOMetadata = StorageVar->getMetadata("io_pipe_id")) {
    assert(IOMetadata->getNumOperands() == 1 &&
           "IO metadata is expected to have a single argument");
    int ID = llvm::mdconst::dyn_extract<llvm::ConstantInt>(
        IOMetadata->getOperand(0))->getZExtValue();
    const std::string IOStrName = std::to_string(ID);
    LLVM_DEBUG(dbgs() << "I/O pipe id is(" << IOStrName << ")\n");

    PipeMD = { (int) Size->getSExtValue(), (int) Align->getSExtValue(),
               (int) Capacity->getSExtValue(), IOStrName };
    return;
  }

  PipeMD =  { (int) Size->getSExtValue(), (int) Align->getSExtValue(),
              (int) Capacity->getSExtValue(), "" };
}

SYCLPipesHack::SYCLPipesHack() : ModulePass(ID) {
  initializeSYCLPipesHackPass(*llvm::PassRegistry::getPassRegistry());
}

bool SYCLPipesHack::runOnModule(Module &M) {
  BuiltinLibInfo &BLI = getAnalysis<BuiltinLibInfoAnalysisLegacy>().getResult();
  OCLBuiltins Builtins(M, BLI.getBuiltinModules());

  // SYCL Program scope pipes are currently represented by global struct with 3
  // i32 values: size, align and capacity. We need to find these global structs
  // and replace with %opencl.pipe_rw_t objects to utilize the rest of pipe
  // related passes without any modifications.
  SmallVector<GlobalVariable *, 8> StorageVars;
  findPipeStorageGlobals(&M, StorageVars);
  if (StorageVars.empty()) {
    return false;
  }

  auto RWPipeTyName = "opencl.pipe_rw_t";
  auto *RWPipeValueTy = StructType::getTypeByName(M.getContext(), RWPipeTyName);
  if (!RWPipeValueTy)
    RWPipeValueTy = StructType::create(M.getContext(), RWPipeTyName);
  auto *RWPipeTy = PointerType::get(RWPipeValueTy, Utils::OCLAddressSpace::Global);

  Function *GlobalCtor = nullptr;

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
    RWPipeGV->setAlignment(M.getDataLayout().getPreferredAlign(RWPipeGV));

    // Pipe parameters are hidden inside of the {i32, i32, i32} struct, so we
    // deconstruct it and set it as a metadata so other passes can check it as
    // with FPGA OpenCL pipes.
    ChannelPipeMetadata::ChannelPipeMD PipeMD;
    getSYCLPipeMetadata(GV, PipeMD);
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
      assert(isa<CastInst>(U) ||
             cast<ConstantExpr>(U)->getOpcode() == Instruction::AddrSpaceCast);
      for (auto *CastU : U->users()) {
        CallInst *CI = cast<CallInst>(CastU);
        Function *F = CI->getCalledFunction();
        assert(F && "Indirect call is not expected");
        assert(F->getName().find(CreatePipeFromPipeStorageWriteName) !=
                   StringRef::npos ||
               F->getName().find(CreatePipeFromPipeStorageReadName) !=
                   StringRef::npos);
      }
    }
#endif

    Constant *Bitcast =
        ConstantExpr::getBitCast(RWPipeGV, GV->getType());
    LLVM_DEBUG(dbgs() << "Replacing pipe storage pointer (" << *GV
               << ") with read-write pipe (" << *Bitcast << "\n");
    GV->replaceAllUsesWith(Bitcast);
  }
  return true;
}

void SYCLPipesHack::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<BuiltinLibInfoAnalysisLegacy>();
}

} // namespace intel

extern "C" ModulePass *createSYCLPipesHackPass() {
  return new intel::SYCLPipesHack();
}
