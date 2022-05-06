//==---------------------- DPCPPChannelPipeUtils.cpp ----------------------==//
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

#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/DPCPPChannelPipeUtils.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/DPCPPKernelCompilationUtils.h"
#include "llvm/Transforms/Intel_DPCPPKernelTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include <tuple>

#define DEBUG_TYPE "dpcpp-kernel-channel-pipe-utils"

constexpr static int ChannelSizeLimit = 256 * 1024;
constexpr static int ChannelArraySizeLimit = 256 * 1024 * 1024;

namespace llvm {
namespace DPCPPChannelPipeUtils {

/// This namespace includes structs and helper functions that are defined in
/// OpenCL C language (pipes-defines.h, pipes-internal.h, pipes-info.cl).
namespace OpenCLInterface {

enum ChannelDepth { Strict = 0, Default = 1, IgnoreDepth = 2 };

constexpr static int MaxVLSupportedByPipes = 16;
constexpr static int PipeWriteBufPreferredLimit = 256;

/// __pipe_get_total_size_fpga wants to know the size of __pipe_t struct.
/// So the only thing we really care about is the struct field alignment.
/// The real meaning of each field doesn't matter.
namespace {

struct __pipe_internal_buf {
  int end;
  int size;
  int limit;
};

struct __pipe_t {
  int packet_size;
  int max_packets;
  alignas(64) volatile std::atomic_int
      head; // The original type is 'volatile atomic_int' in OpenCL C,
            // but we don't have native 'atomic_int' type in C++, so
            // replace it with 'volatile std::atomic_int'.
            // This hacking won't change the total size of struct __pipe_t.
  alignas(64) volatile std::atomic_int
      tail; // The original type is 'volatile atomic_int' in OpenCL C,
            // but we don't have native 'atomic_int' type in C++, so
            // replace it with 'volatile std::atomic_int'.
            // This hacking won't change the total size of struct __pipe_t.
  alignas(64) struct __pipe_internal_buf read_buf;
  alignas(64) struct __pipe_internal_buf write_buf;
  FILE *io;
};

} // namespace

int __pipe_get_max_packets_fpga(int depth, int mode) {
  // STRICT mode:
  //   if user specifies the depth -> the exact depth the user asked for is used
  //   if user doesn't specify the depth -> use depth = 1
  // DEFAULT mode:
  //   if user specifies the depth -> the exact depth the user asked for is used
  //   if user doesn't specify the depth -> use whataver we want to achieve max
  //     performance
  // IGNORE_DEPTH mode:
  //    Use whatever we want to achieve max performance, ignore user-provided
  //      values
  //
  //  NOTE: in any mode we need to ensure that at least 'depth' packets can be
  //  written without blocking.
  if (mode == ChannelDepth::Strict) {
    if (depth == 0)
      depth = 1;
    // reserve one extra element b/w head and tail to distinguish full/empty
    // conditions
    return depth + 1;
  }

  if (mode == ChannelDepth::Default && depth != 0) {
    // reserve one extra element b/w head and tail to distinguish full/empty
    // conditions
    return depth + 1;
  }

  // if (mode == ChannelDepth::IgnoreDepth ||
  //    (mode == ChannelDepth::Default && depth == 0))
  // pipe max_packets should be more than maximum of supported VL
  int max_packets =
      depth > MaxVLSupportedByPipes ? depth : MaxVLSupportedByPipes;

  // reserve one extra element b/w head and tail to distinguish full/empty
  // conditions
  max_packets += 1;

  // We must ensure that at least 'depth' packets can be written without
  // blocking. Write cache can block us doing so, because we try to reserve at
  // least 'limit' packets for writing, before performing an actual write.
  //
  // If we have 'depth - 1' packets written, and max_packets == 'depth + 1' (see
  // above), we cannot write last packet, because we wait until 'limit' packets
  // would be available.
  max_packets += PipeWriteBufPreferredLimit - 1;

  return max_packets;
}

int __pipe_get_total_size_fpga(int packet_size, int depth, int mode) {
  size_t total = sizeof(struct __pipe_t) +
                 packet_size * __pipe_get_max_packets_fpga(depth, mode);
  return total;
}

} // namespace OpenCLInterface

unsigned DPCPPChannelDepthEmulationMode = OpenCLInterface::ChannelDepth::Strict;
static cl::opt<unsigned, true> DPCPPChannelDepthEmulationModeOpt(
    "dpcpp-channel-depth-emulation-mode", cl::Hidden,
    cl::desc("Channel depth emulation mode"),
    cl::location(DPCPPChannelDepthEmulationMode));

DiagnosticKind LargeChannelPipeWarningDiagInfo::Kind =
    static_cast<DiagnosticKind>(getNextAvailablePluginDiagnosticKind());

void getSYCLPipeMetadata(GlobalVariable *StorageVar, ChannelPipeMD &PipeMD) {
  LLVM_DEBUG(dbgs() << "Extracting pipe metadata from: " << *StorageVar
                    << "\n");

  // StorageVar is a struct of 3 integers: size, alignment and capacity
  // (depth). Explore its initializer to find out these parameters (they are
  // guaranteed to be constants).
  ConstantStruct *Initializer =
      cast<ConstantStruct>(StorageVar->getInitializer());

  assert(Initializer->getNumOperands() == 3 &&
         "Pipe storage initializer have to contain 3 integer values");
  ConstantInt *Size = cast<ConstantInt>(Initializer->getOperand(0));
  ConstantInt *Align = cast<ConstantInt>(Initializer->getOperand(1));
  ConstantInt *Capacity = cast<ConstantInt>(Initializer->getOperand(2));

  LLVM_DEBUG(dbgs() << "Got size(" << *Size << "), align(" << *Align
                    << ") and capacity(" << *Capacity << ")\n");

  if (MDNode *IOMetadata = StorageVar->getMetadata("io_pipe_id")) {
    assert(IOMetadata->getNumOperands() == 1 &&
           "IO metadata is expected to have a single argument");
    int ID =
        llvm::mdconst::dyn_extract<llvm::ConstantInt>(IOMetadata->getOperand(0))
            ->getZExtValue();
    const std::string IOStrName = std::to_string(ID);
    LLVM_DEBUG(dbgs() << "I/O pipe id is(" << IOStrName << ")\n");

    PipeMD = {(int)Size->getSExtValue(), (int)Align->getSExtValue(),
              (int)Capacity->getSExtValue(), IOStrName};
    return;
  }

  PipeMD = {(int)Size->getSExtValue(), (int)Align->getSExtValue(),
            (int)Capacity->getSExtValue(), ""};
}

void setPipeMetadata(GlobalVariable *StorageVar, const ChannelPipeMD &PipeMD) {
  auto MD = DPCPPKernelMetadataAPI::GlobalVariableMetadataAPI(StorageVar);
  MD.PipePacketSize.set(PipeMD.PacketSize);
  MD.PipePacketAlign.set(PipeMD.PacketAlign);
  MD.PipeDepth.set(PipeMD.Depth);
  MD.PipeIO.set(PipeMD.IO);
}

Function *createPipeGlobalCtor(Module &M) {
  auto *CtorTy = FunctionType::get(Type::getVoidTy(M.getContext()),
                                   ArrayRef<Type *>(), false);
  Function *Ctor = cast<Function>(
      M.getOrInsertFunction("__pipe_global_ctor", CtorTy).getCallee());
  // The function will be called from the RT
  Ctor->setLinkage(GlobalValue::ExternalLinkage);
  auto *EntryBB = BasicBlock::Create(M.getContext(), "entry", Ctor);
  ReturnInst::Create(M.getContext(), EntryBB);
  appendToGlobalCtors(M, Ctor, /*Priority=*/65535);
  return Ctor;
}

static GlobalVariable *createPipeBackingStore(GlobalVariable *GV,
                                              const ChannelPipeMD &MD) {
  Module *M = GV->getParent();
  Type *Int8Ty = IntegerType::getInt8Ty(M->getContext());

  size_t BSSize = OpenCLInterface::__pipe_get_total_size_fpga(
      MD.PacketSize, MD.Depth, DPCPPChannelDepthEmulationMode);
  size_t ChanArrayNum = 0;
  size_t SingleChanSize = BSSize;
  if (auto *PipePtrArrayTy = dyn_cast<ArrayType>(GV->getValueType())) {
    ChanArrayNum = DPCPPKernelCompilationUtils::getNumElementsOfNestedArray(
        PipePtrArrayTy);
    BSSize *= ChanArrayNum;
  }
  // If channel size exceeds the threshold, it may leads to potential
  // memory allocation failure later. This channel info will be recorded
  // and be shown when memory allocation fails.
  if (SingleChanSize > ChannelSizeLimit || BSSize > ChannelArraySizeLimit) {
    StringRef StorageOriginalName;
    std::tie(StorageOriginalName, std::ignore) = GV->getName().split('.');
    M->getContext().diagnose(LargeChannelPipeWarningDiagInfo(
        StorageOriginalName, MD.PacketSize, ChanArrayNum, BSSize));
  }

  auto *ArrayTy = ArrayType::get(Int8Ty, BSSize);

  auto *BS =
      new GlobalVariable(*M, ArrayTy, /*isConstant=*/false, GV->getLinkage(),
                         /*initializer=*/nullptr, GV->getName() + ".bs",
                         /*InsertBefore=*/nullptr, GlobalValue::NotThreadLocal,
                         DPCPPKernelCompilationUtils::ADDRESS_SPACE_GLOBAL);

  BS->setInitializer(ConstantAggregateZero::get(ArrayTy));
  BS->setAlignment(MaybeAlign(MD.PacketAlign));

  return BS;
}

void initializeGlobalPipeScalar(GlobalVariable *PipeGV, const ChannelPipeMD &MD,
                                Function *GlobalCtor, Function *PipeInit) {
  auto *BS = createPipeBackingStore(PipeGV, MD);

  IRBuilder<> Builder(GlobalCtor->getEntryBlock().getTerminator());

  Value *PacketSize = Builder.getInt32(MD.PacketSize);
  Value *Depth = Builder.getInt32(MD.Depth);
  Value *Mode = Builder.getInt32(DPCPPChannelDepthEmulationMode);

  Value *CallArgs[] = {
      Builder.CreateBitCast(BS, PipeInit->getFunctionType()->getParamType(0)),
      PacketSize, Depth, Mode};

  Builder.CreateCall(PipeInit, CallArgs);
  Builder.CreateStore(
      Builder.CreateBitCast(BS, PipeGV->getType()->getElementType()), PipeGV);
}

} // namespace DPCPPChannelPipeUtils
} // namespace llvm
