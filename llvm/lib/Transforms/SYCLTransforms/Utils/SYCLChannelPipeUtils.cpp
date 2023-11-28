//==---------------------- SYCLChannelPipeUtils.cpp ----------------------==//
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

#include "llvm/Transforms/SYCLTransforms/Utils/SYCLChannelPipeUtils.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/MetadataAPI.h"
#include "llvm/Transforms/Utils/ModuleUtils.h"
#include <tuple>

#define DEBUG_TYPE "sycl-kernel-channel-pipe-utils"

using namespace llvm::CompilationUtils;

constexpr static int ChannelSizeLimit = 256 * 1024;
constexpr static int ChannelArraySizeLimit = 256 * 1024 * 1024;

unsigned SYCLChannelDepthEmulationMode = llvm::SYCLChannelPipeUtils::Strict;
static llvm::cl::opt<unsigned, true> SYCLChannelDepthEmulationModeOpt(
    "sycl-channel-depth-emulation-mode", llvm::cl::Hidden,
    llvm::cl::desc("Channel depth emulation mode"),
    llvm::cl::location(SYCLChannelDepthEmulationMode));

namespace llvm {
namespace SYCLChannelPipeUtils {

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

typedef struct __hostpipe_packet {
  void *data;
  struct __hostpipe_packet *next;
} __hostpipe_packet;

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
  // Access mode protocol
  int protocol;
  // Two access pointers for host pipe.
  alignas(64) struct __hostpipe_packet *hp_read_ptr;
  alignas(64) struct __hostpipe_packet *hp_write_ptr;
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

DiagnosticKind LargeChannelPipeWarningDiagInfo::Kind =
    static_cast<DiagnosticKind>(getNextAvailablePluginDiagnosticKind());

bool isGlobalPipe(GlobalVariable *GV) {
  auto MD = SYCLKernelMetadataAPI::GlobalVariableMetadataAPI(GV);
  return MD.PipePacketSize.hasValue() && MD.PipePacketAlign.hasValue();
}

void getSYCLPipeMetadata(GlobalVariable *StorageVar, ChannelPipeMD &PipeMD) {
  LLVM_DEBUG(dbgs() << "Extracting pipe metadata from: " << *StorageVar
                    << "\n");

  // StorageVar is a struct whose first 3 members are integers: size, alignment
  // and capacity (depth). Explore its initializer to find out these parameters
  // (they are guaranteed to be constants).
  ConstantStruct *Initializer =
      cast<ConstantStruct>(StorageVar->getInitializer());

  assert(Initializer->getNumOperands() >= 3 &&
         "Pipe storage initializer have to contain at least 3 integer values");
  ConstantInt *Size = cast<ConstantInt>(Initializer->getOperand(0));
  ConstantInt *Align = cast<ConstantInt>(Initializer->getOperand(1));
  ConstantInt *Capacity = cast<ConstantInt>(Initializer->getOperand(2));
  ConstantInt *Protocol =
      ConstantInt::get(Type::getInt32Ty(Initializer->getContext()), -1);
  if (Initializer->getNumOperands() >= 8) {
    Protocol = cast<ConstantInt>(Initializer->getOperand(7));
  }

  LLVM_DEBUG(dbgs() << "Got size(" << *Size << "), align(" << *Align
                    << "), capacity(" << *Capacity << "), protocol("
                    << *Protocol << ")\n");

  if (MDNode *IOMetadata = StorageVar->getMetadata("io_pipe_id")) {
    assert(IOMetadata->getNumOperands() == 1 &&
           "IO metadata is expected to have a single argument");
    int ID =
        llvm::mdconst::dyn_extract<llvm::ConstantInt>(IOMetadata->getOperand(0))
            ->getZExtValue();
    const std::string IOStrName = std::to_string(ID);
    LLVM_DEBUG(dbgs() << "I/O pipe id is(" << IOStrName << ")\n");

    PipeMD = {(int)Size->getSExtValue(), (int)Align->getSExtValue(),
              (int)Capacity->getSExtValue(), IOStrName,
              (int)Protocol->getSExtValue()};
    return;
  }

  PipeMD = {(int)Size->getSExtValue(), (int)Align->getSExtValue(),
            (int)Capacity->getSExtValue(), "", (int)Protocol->getSExtValue()};
}

void setPipeMetadata(GlobalVariable *StorageVar, const ChannelPipeMD &PipeMD) {
  auto MD = SYCLKernelMetadataAPI::GlobalVariableMetadataAPI(StorageVar);
  MD.PipePacketSize.set(PipeMD.PacketSize);
  MD.PipePacketAlign.set(PipeMD.PacketAlign);
  MD.PipeDepth.set(PipeMD.Depth);
  MD.PipeIO.set(PipeMD.IO);
  MD.PipeProtocol.set(PipeMD.Protocol);
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

Function *createPipeGlobalDtor(Module &M) {
  auto *DtorTy = FunctionType::get(Type::getVoidTy(M.getContext()),
                                   ArrayRef<Type *>(), false);
  Function *Dtor = cast<Function>(
      M.getOrInsertFunction("__pipe_global_dtor", DtorTy).getCallee());
  Dtor->setLinkage(GlobalValue::ExternalLinkage);
  auto *EntryBB = BasicBlock::Create(M.getContext(), "entry", Dtor);
  ReturnInst::Create(M.getContext(), EntryBB);
  appendToGlobalDtors(M, Dtor, /*Priority=*/65535);
  return Dtor;
}

GlobalVariable *createPipeBackingStore(GlobalVariable *GV,
                                       const ChannelPipeMD &MD) {
  Module *M = GV->getParent();
  Type *Int8Ty = IntegerType::getInt8Ty(M->getContext());

  size_t BSSize = OpenCLInterface::__pipe_get_total_size_fpga(
      MD.PacketSize, MD.Depth, SYCLChannelDepthEmulationMode);
  size_t ChanArrayNum = 0;
  size_t SingleChanSize = BSSize;
  if (auto *PipePtrArrayTy = dyn_cast<ArrayType>(GV->getValueType())) {
    ChanArrayNum = getNumElementsOfNestedArray(PipePtrArrayTy);
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
      new GlobalVariable(ArrayTy, /*isConstant=*/false, GV->getLinkage(),
                         /*initializer=*/nullptr, GV->getName() + ".bs",
                         GlobalValue::NotThreadLocal, ADDRESS_SPACE_GLOBAL);

  BS->setInitializer(ConstantAggregateZero::get(ArrayTy));
  BS->setAlignment(MaybeAlign(MD.PacketAlign));
  M->insertGlobalVariable(std::next(GV->getIterator()), BS);

  return BS;
}

ChannelKind getChannelKind(const StringRef Name) {
  ChannelKind Kind;

  std::tie(Kind.Access, Kind.Blocking) =
      StringSwitch<std::pair<ChannelKind::AccessKind, bool>>(Name)
          .StartsWith("_Z18read_channel_intel", {ChannelKind::READ, true})
          .StartsWith("_Z21read_channel_nb_intel", {ChannelKind::READ, false})
          .StartsWith("_Z19write_channel_intel", {ChannelKind::WRITE, true})
          .StartsWith("_Z22write_channel_nb_intel", {ChannelKind::WRITE, false})
          .Default({ChannelKind::NONE, false});

  return Kind;
}

ChannelPipeMD getChannelPipeMetadata(GlobalVariable *Channel) {
  auto GVMetadata = SYCLKernelMetadataAPI::GlobalVariableMetadataAPI(Channel);

  assert(GVMetadata.PipePacketSize.hasValue() &&
         GVMetadata.PipePacketAlign.hasValue() &&
         "Channel metadata must contain packet_size and packet_align");

  ChannelPipeMD CMD;
  CMD.PacketSize = GVMetadata.PipePacketSize.get();
  CMD.PacketAlign = GVMetadata.PipePacketAlign.get();
  CMD.Depth = GVMetadata.PipeDepth.hasValue() ? GVMetadata.PipeDepth.get() : 0;
  CMD.IO = GVMetadata.PipeIO.hasValue() ? GVMetadata.PipeIO.get() : "";
  CMD.Protocol =
      GVMetadata.PipeProtocol.hasValue() ? GVMetadata.PipeProtocol.get() : -1;

  return CMD;
}

void initializeGlobalPipeScalar(GlobalVariable *PipeGV, const ChannelPipeMD &MD,
                                Function *GlobalCtor, Function *PipeInit) {
  auto *BS = createPipeBackingStore(PipeGV, MD);

  IRBuilder<> Builder(GlobalCtor->getEntryBlock().getTerminator());

  Value *PacketSize = Builder.getInt32(MD.PacketSize);
  int depth = MD.Depth;
  if (PipeGV->getName().starts_with(
          "_ZN4sycl3_V13ext5intel12experimental4pipe")) // Hostpipe
    depth = -1;
  Value *Depth = Builder.getInt32(depth);
  Value *Mode = Builder.getInt32(SYCLChannelDepthEmulationMode);

  SmallVector<Value *, 5> CallArgs = {
      Builder.CreateBitCast(BS, PipeInit->getFunctionType()->getParamType(0)),
      PacketSize, Depth, Mode};

  if (MD.Protocol >= 0) {
    CallArgs.push_back(Builder.getInt32(MD.Protocol));
  }

  Builder.CreateCall(PipeInit, CallArgs);
  Builder.CreateStore(Builder.CreateBitCast(BS, PipeGV->getValueType()),
                      PipeGV);
}

void initializeGlobalPipeReleaseCall(Function *GlobalDtor,
                                     Function *PipeReleaseFunc,
                                     GlobalVariable *PipeGV) {
  IRBuilder<> Builder(GlobalDtor->getEntryBlock().getTerminator());
  Value *CallArgs[] = {Builder.CreateBitCast(
      PipeGV, PipeReleaseFunc->getFunctionType()->getParamType(0))};
  Builder.CreateCall(PipeReleaseFunc, CallArgs);
}

PipeTypesHelper::PipeTypesHelper(Module &M) {
  for (auto &GV : M.globals()) {
    if (isGlobalPipe(&GV)) {
      GlobalPipeTy = GV.getValueType();
      break;
    }
  }

  for (Function *F : getKernels(M)) {
    SYCLKernelMetadataAPI::KernelInternalMetadataAPI KIMD(F);
    if (KIMD.ArgTypeNullValList.hasValue()) {
      llvm::for_each(KIMD.ArgTypeNullValList, [&](Constant *C) {
        if (auto *TETy = dyn_cast<TargetExtType>(C->getType());
            TETy && TETy->getName() == "spirv.Pipe") {
          switch (TETy->getIntParameter(0)) {
          case 0:
            OpaquePipeROTy = TETy;
            break;
          case 1:
            OpaquePipeWOTy = TETy;
            break;
          case 2:
            OpaquePipeRWTy = TETy;
            break;
          default:
            llvm_unreachable("unexpected pipe type");
          }
        }
      });
    }
  }
}

Function *getPipeBuiltin(Module &M, RuntimeService &RTS, const PipeKind &Kind) {
  if (Kind.Blocking) {
    // There are no declarations and definitions of blocking pipe built-ins in
    // RTL's.
    // Calls to blocking pipe built-ins will be resolved in PipeSupport,
    // so we just need to insert declarations here.
    PipeKind NonBlockingKind = Kind;
    NonBlockingKind.Blocking = false;

    Function *NonBlockingBuiltin = importFunctionDecl(
        &M, RTS.findFunctionInBuiltinModules(getPipeName(NonBlockingKind)));
    Function *BlockingBuiltin = cast<Function>(
        M.getOrInsertFunction(getPipeName(Kind),
                              NonBlockingBuiltin->getFunctionType())
            .getCallee());
    return BlockingBuiltin;
  }

  return importFunctionDecl(
      &M, RTS.findFunctionInBuiltinModules(getPipeName(Kind)));
}

} // namespace SYCLChannelPipeUtils
} // namespace llvm
