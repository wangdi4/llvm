//==----------------------- DPCPPChannelPipeUtils.h -----------------------==//
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

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_CHANNEL_PIPE_UTILS_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_CHANNEL_PIPE_UTILS_H

#include "llvm/IR/DiagnosticInfo.h"
#include "llvm/IR/DiagnosticPrinter.h"
#include "llvm/IR/GlobalVariable.h"

namespace llvm {
namespace DPCPPChannelPipeUtils {

struct ChannelPipeMD {
  int PacketSize;
  int PacketAlign;
  int Depth;
  std::string IO;
};

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

/// Extract pipe metadata attached to the SYCL pipe storage GV.
void getSYCLPipeMetadata(GlobalVariable *SYCLPipeStorageVar, ChannelPipeMD &MD);

/// Attach pipe metadata to the pipe storage GV.
void setPipeMetadata(GlobalVariable *PipeStorageVar, const ChannelPipeMD &MD);

/// Create __pipe_global_ctor() function body.
Function *createPipeGlobalCtor(Module &M);

void initializeGlobalPipeScalar(GlobalVariable *PipeStorageVar,
                                const ChannelPipeMD &MD, Function *GlobalCtor,
                                Function *PipeInit);

} // namespace DPCPPChannelPipeUtils
} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_CHANNEL_PIPE_UTILS_H
