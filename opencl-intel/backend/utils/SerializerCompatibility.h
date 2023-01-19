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
//
// This file provides compatibility utilities that help to support old program
// binary that was serialized from legacy data structures.

#ifndef __OPENCL_BACKEND_SERIALIZER_COMPATIBILITY__
#define __OPENCL_BACKEND_SERIALIZER_COMPATIBILITY__

#include "llvm/Transforms/SYCLTransforms/KernelArgType.h"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

/// Legacy kernel argument info struct. It is replaced by llvm::KernelArgument.
struct KernelArgumentLegacy {
  llvm::KernelArgumentType Ty; //!< Type of the argument.
  unsigned SizeInBytes;        //!< Size of the argument in bytes
  unsigned Access;             //!< Access type for pointers
  unsigned OffsetInBytes;      //!< Offset of the argument in argument buffer

  llvm::KernelArgument toNew();
};

/// Convert from llvm::KernelArgument to KernelArgumentLegacy.
KernelArgumentLegacy cvtToKernelArgumentLegacy(const llvm::KernelArgument &Arg);

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#endif // __OPENCL_BACKEND_SERIALIZER_COMPATIBILITY__
