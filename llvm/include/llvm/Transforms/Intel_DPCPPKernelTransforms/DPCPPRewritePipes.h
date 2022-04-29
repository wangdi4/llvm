//==- DPCPPRewritePipes.h - Rewrite DPCPP pipe structs to OpenCL structs --==//
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

#ifndef LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_DPCPP_REWRITE_PIPES_H
#define LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_DPCPP_REWRITE_PIPES_H

#include "llvm/IR/PassManager.h"

namespace llvm {

class BuiltinLibInfo;

/// Rewrite DPCPP pipe structs to OpenCL pipe structs.
/// SYCL Program scope pipes are currently represented by global struct with 3
/// i32 values: size, align and capacity. We need to find these global structs
/// and replace with %opencl.pipe_rw_t objects to utilize the rest of pipe
/// related passes without any modifications.
///
/// SYCL Pipes are enabled as SPIR-V Program Scope pipes, but due to
/// various design problems, LLVM IR that we get from SPIR-V translator is
/// not flexible enough to make a robust implementation.
///
/// With the current design, program scope pipe is emitted as a global
/// struct (pipe storage) that consists of three i32 values: size,
/// alignment and capacity.
///
/// Following the design for OpenCL FPGA channels, DPCPPRewritePipes pass
/// creates new global variable of type %opencl.pipe_rw_t, initilizes it
/// with a backing store, and then replaces the original global struct
/// with this new opaque type.
///
/// This design is incomplete and fragile: if an IR heavily optimized, we
/// may not be able to replace one type with another properly. Proper
/// solution would be possible if we change IR representation of program
/// pipes, and add a payload to the storage struct.
class DPCPPRewritePipesPass : public PassInfoMixin<DPCPPRewritePipesPass> {
public:
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);

  bool runImpl(Module &M, BuiltinLibInfo *BLI);

  static bool isRequired() { return true; }
};

} // namespace llvm

#endif // LLVM_TRANSFORMS_INTEL_DPCPP_KERNEL_TRANSFORMS_DPCPP_REWRITE_PIPES_H
