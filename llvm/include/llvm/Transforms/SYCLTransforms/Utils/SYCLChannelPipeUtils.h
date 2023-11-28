//==----------------------- SYCLChannelPipeUtils.h -----------------------==//
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

#ifndef LLVM_TRANSFORMS_SYCLTRANSFORMS_CHANNEL_PIPE_UTILS_H
#define LLVM_TRANSFORMS_SYCLTRANSFORMS_CHANNEL_PIPE_UTILS_H

#include "llvm/ADT/StringRef.h"
#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/Transforms/SYCLTransforms/Utils/CompilationUtils.h"
#include "llvm/Transforms/SYCLTransforms/Utils/RuntimeService.h"

namespace llvm {
namespace SYCLChannelPipeUtils {

/// Suffix of new pipe global variable that replaces channel global variable.
static constexpr StringRef PipeGVSuffix = ".pipe";

struct ChannelPipeMD {
  int PacketSize;
  int PacketAlign;
  int Depth;
  std::string IO;
  int Protocol;
};

enum ChannelDepthMode { Strict, Default, IgnoreDepth };

ChannelPipeMD getChannelPipeMetadata(GlobalVariable *Channel);

struct ChannelKind {
  enum AccessKind {
    NONE, // not a channel
    READ,
    WRITE
  };

  AccessKind Access;
  bool Blocking;

  bool operator==(const ChannelKind &LHS) const {
    return Access == LHS.Access && Blocking == LHS.Blocking;
  }

  operator bool() const { return Access != AccessKind::NONE; }
};

ChannelKind getChannelKind(const StringRef);

/// Emitting warning messages of large-size channel declaration.
class LargeChannelPipeWarningDiagInfo : public DiagnosticInfo {
  const StringRef ChannelName;
  const unsigned PacketSize;
  const unsigned NumOfChannelArray;
  const unsigned TotalChannelSize;

public:
  static DiagnosticKind Kind;

  LargeChannelPipeWarningDiagInfo(const StringRef ChannelName,
                                  const unsigned PacketSize,
                                  const unsigned NumOfChannelArray,
                                  const unsigned TotalChannelSize)
      : DiagnosticInfo(Kind, DS_Warning), ChannelName(ChannelName),
        PacketSize(PacketSize), NumOfChannelArray(NumOfChannelArray),
        TotalChannelSize(TotalChannelSize) {}

  void print(DiagnosticPrinter &DP) const override {
    DP << "Large channel may lead to memory allocation failure:\n"
       << "  Channel name: " << ChannelName << "\n  Packet size: " << PacketSize
       << '\n';
    if (NumOfChannelArray > 0)
      DP << "  Number of channel array: " << NumOfChannelArray << '\n';
    DP << "  Total channel size: " << TotalChannelSize << '\n';
  }
};

/// Return true if 'GV' is global pipe.
bool isGlobalPipe(GlobalVariable *GV);

/// Extract pipe metadata attached to the SYCL pipe storage GV.
void getSYCLPipeMetadata(GlobalVariable *SYCLPipeStorageVar, ChannelPipeMD &MD);

/// Attach pipe metadata to the pipe storage GV.
void setPipeMetadata(GlobalVariable *PipeStorageVar, const ChannelPipeMD &MD);

/// Create __pipe_global_ctor() function body.
Function *createPipeGlobalCtor(Module &M);

/// Create __pipe_global_dtor() function body.
Function *createPipeGlobalDtor(Module &M);

void initializeGlobalPipeScalar(GlobalVariable *PipeStorageVar,
                                const ChannelPipeMD &MD, Function *GlobalCtor,
                                Function *PipeInit);

void initializeGlobalPipeReleaseCall(Function *GlobalDtor,
                                     Function *PipeReleaseFunc,
                                     GlobalVariable *PipeGV);

GlobalVariable *createPipeBackingStore(GlobalVariable *GV,
                                       const ChannelPipeMD &MD);

namespace OpenCLInterface {
int __pipe_get_total_size_fpga(int packet_size, int depth, int mode);
} // namespace OpenCLInterface

/// A helper class for checking pipe types.
class PipeTypesHelper {
  Type *OpaquePipeRWTy = nullptr;
  Type *OpaquePipeROTy = nullptr;
  Type *OpaquePipeWOTy = nullptr;
  Type *GlobalPipeTy = nullptr;

public:
  PipeTypesHelper(Module &M);

  bool hasPipeTypes() const {
    return OpaquePipeRWTy || OpaquePipeROTy || OpaquePipeWOTy || GlobalPipeTy;
  }
};

Function *getPipeBuiltin(Module &M, RuntimeService &RTS,
                         const CompilationUtils::PipeKind &Kind);

} // namespace SYCLChannelPipeUtils
} // namespace llvm

#endif // LLVM_TRANSFORMS_SYCLTRANSFORMS_CHANNEL_PIPE_UTILS_H
