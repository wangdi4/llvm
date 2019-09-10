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

#pragma once

#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/ToolOutputFile.h"

#include <memory>
#include <string>

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

/// This class is responsible for asm JIT compilation.
class AsmCompiler {
public:

  /// @brief Compiles assembly code to an object file. Currently, only msvc targets are supported.
  /// @param InAsm - In-memory text of assembly code
  /// @param OutObject - Object's output file stream
  /// @param Triple - Target triple. It can only be "x86_64-pc-windows-msvc" or "i686-pc-windows-msvc"
  /// @returns - Returns the compiler's return value
  static int compileAsmToObjectFile(std::unique_ptr<llvm::MemoryBuffer> InAsm,
                                    llvm::raw_fd_ostream *OutObject,
                                    const std::string &Triple);

private:
  static const llvm::Target *getTarget(const std::string &TripleName);
  static int assembleInput(const llvm::Target *TheTarget,
                           llvm::SourceMgr &SrcMgr, llvm::MCContext &Ctx,
                           llvm::MCStreamer &Str, llvm::MCAsmInfo &MAI,
                           llvm::MCSubtargetInfo &STI, llvm::MCInstrInfo &MCII,
                           llvm::MCTargetOptions &MCOptions);
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel