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
#include "ChannelPipeUtils.h"

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"
#include <llvm/ADT/SmallString.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Transforms/Utils/ModuleUtils.h>

#include <CompilationUtils.h>

int Intel::OpenCL::DeviceBackend::ChannelDepthEmulationMode;

namespace Intel { namespace OpenCL { namespace DeviceBackend {

static cl::opt<int, true> ChannelDepthEmulationModeOpt(
    "channel-depth-emulation-mode", cl::Hidden,
    cl::desc("Channel depth emulation mode"),
    cl::location(Intel::OpenCL::DeviceBackend::ChannelDepthEmulationMode),
    cl::init(CHANNEL_DEPTH_MODE_STRICT));

// TODO: Using static string can only handle non-parallel compilaton.
// Need to use better solution for it.
std::string ChannelPipesErrorLog;
#define CHANNEL_SIZE_LIMIT 256 * 1024
#define CHANNEL_ARRAY_SIZE_LIMIT 256 * 1024 * 1024


GlobalVariable *
createPipeBackingStore(GlobalVariable *GV,
                       const ChannelPipeMetadata::ChannelPipeMD &MD) {
  Module *M = GV->getParent();
  Type *Int8Ty = IntegerType::getInt8Ty(M->getContext());

  size_t BSSize = __pipe_get_total_size_fpga(MD.PacketSize, MD.Depth,
      ChannelDepthEmulationMode);
  size_t chanArrayNum = 0;
  size_t singleChanSize = BSSize;
  if (auto *PipePtrArrayTy =
          dyn_cast<ArrayType>(GV->getType()->getElementType())) {
    chanArrayNum = CompilationUtils::getArrayNumElements(PipePtrArrayTy);
    BSSize *= chanArrayNum;
  }
  // If channel size exceeds the threshold, it may leads to potential
  // memory allocation failure later. This channel info will be recorded
  // and be shown when memory allocation fails.
  if (singleChanSize > CHANNEL_SIZE_LIMIT || BSSize > CHANNEL_ARRAY_SIZE_LIMIT) {
    ChannelPipesErrorLog.append("Channel name: " +
        GV->getName().drop_back(5).str() + "\n");
    ChannelPipesErrorLog.append("PacketSize: " +
        std::to_string(MD.PacketSize) + "\n");

    if (chanArrayNum > 0) {
      ChannelPipesErrorLog.append("Channel array Nums: " +
          std::to_string(chanArrayNum) +  "\n");
    }

    ChannelPipesErrorLog.append("Total Channel size: " +
        std::to_string(BSSize) + "\n");
  }

  auto *ArrayTy = ArrayType::get(Int8Ty, BSSize);

  SmallString<16> NameStr;
  auto Name = (GV->getName() + ".bs").toStringRef(NameStr);

  auto *BS =
      new GlobalVariable(*M, ArrayTy, /*isConstant=*/false, GV->getLinkage(),
                         /*initializer=*/nullptr, Name,
                         /*InsertBefore=*/nullptr, GlobalValue::NotThreadLocal,
                         Utils::OCLAddressSpace::Global);

  BS->setInitializer(ConstantAggregateZero::get(ArrayTy));
  BS->setAlignment(MaybeAlign(MD.PacketAlign));

  return BS;
}

void
initializeGlobalPipeScalar(GlobalVariable *PipeGV,
                           const ChannelPipeMetadata::ChannelPipeMD &MD,
                           Function *GlobalCtor,
                           Function *PipeInit) {
  auto *BS = createPipeBackingStore(PipeGV, MD);

  IRBuilder<> Builder(GlobalCtor->getEntryBlock().getTerminator());

  Value *PacketSize = Builder.getInt32(MD.PacketSize);
  Value *Depth = Builder.getInt32(MD.Depth);
  Value *Mode = Builder.getInt32(ChannelDepthEmulationMode);

  Value *CallArgs[] = {
      Builder.CreateBitCast(BS, PipeInit->getFunctionType()->getParamType(0)),
      PacketSize, Depth, Mode
  };

  Builder.CreateCall(PipeInit, CallArgs);
  Builder.CreateStore(
      Builder.CreateBitCast(BS, PipeGV->getType()->getElementType()), PipeGV);
}

Function *createGlobalPipeCtor(Module &M) {
  auto *CtorTy = FunctionType::get(Type::getVoidTy(M.getContext()),
                                   ArrayRef<Type *>(), false);

  Function *Ctor =
      cast<Function>(M.getOrInsertFunction("__pipe_global_ctor",
                                           CtorTy).getCallee());

  // The function will be called from the RT
  Ctor->setLinkage(GlobalValue::ExternalLinkage);

  auto *EntryBB = BasicBlock::Create(M.getContext(), "entry", Ctor);
  ReturnInst::Create(M.getContext(), EntryBB);

  appendToGlobalCtors(M, Ctor, /*Priority=*/65535);

  return Ctor;
}

void
setPipeMetadata(GlobalVariable *GV,
                const ChannelPipeMetadata::ChannelPipeMD &MD) {
  auto PipeMD = DPCPPKernelMetadataAPI::GlobalVariableMetadataAPI(GV);
  PipeMD.PipePacketSize.set(MD.PacketSize);
  PipeMD.PipePacketAlign.set(MD.PacketAlign);
  PipeMD.PipeDepth.set(MD.Depth);
  PipeMD.PipeIO.set(MD.IO);
}

}}} // namespace Intel::OpenCL::DeviceBackend
