// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

/// @brief Get global Ctor/Dtor names sorted by priority. Set Ctor/Dtor linkage
///        to external.
#ifndef __BACKEND_UTILS_H__
#define __BACKEND_UTILS_H__

#include "llvm/IR/Module.h"
#include "llvm/Passes/OptimizationLevel.h"
#include <vector>

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

namespace BackendUtils {

/// Get optimization level based on
/// * module compile flag.
/// * kernel function attributes: optnone and sycl-optlevel.
llvm::OptimizationLevel getOptLevel(bool HasDisableOptFlag, llvm::Module &M);

void recordGlobalCtorDtors(llvm::Module &M, std::vector<std::string> &CtorNames,
                           std::vector<std::string> &DtorNames);

} // namespace BackendUtils

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#endif // __BACKEND_UTILS_H__
