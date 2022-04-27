// Copyright 2021 Intel Corporation.
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

#include "SerializerCompatibility.h"

using namespace llvm;

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

KernelArgument KernelArgumentLegacy::toNew() {
  return KernelArgument{Ty, SizeInBytes, OffsetInBytes};
}

/// Convert from llvm::KernelArgument to legacy cl_kernel_argument.
KernelArgumentLegacy cvtToKernelArgumentLegacy(const KernelArgument &Arg) {
  unsigned DummyAccess = 0;
  return KernelArgumentLegacy{Arg.Ty, Arg.SizeInBytes, DummyAccess,
                              Arg.OffsetInBytes};
}

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
