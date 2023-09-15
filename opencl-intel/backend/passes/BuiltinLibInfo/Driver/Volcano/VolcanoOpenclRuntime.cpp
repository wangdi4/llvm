// INTEL CONFIDENTIAL
//
// Copyright 2010 Intel Corporation.
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

#include "VolcanoOpenclRuntime.h"
#include "Logger.h"

namespace intel {
const char *volacanoScalarSelect[] = {
    "_Z6selectccc", "_Z6selectcch", "_Z6selecthhc", "_Z6selecthhh",
    "_Z6selectsss", "_Z6selectsst", "_Z6selecttts", "_Z6selectttt",
    "_Z6selectiii", "_Z6selectiij", "_Z6selectjji", "_Z6selectjjj",
    "_Z6selectlll", "_Z6selectllm", "_Z6selectmml", "_Z6selectmmm",
    "_Z6selectffi", "_Z6selectffj", "_Z6selectddl", "_Z6selectddm",
    nullptr};

bool VolcanoOpenclRuntime::needPreVectorizationFakeFunction(
    const std::string & /*funcName*/) const {
  return false;
}

bool VolcanoOpenclRuntime::isWriteImage(
    const std::string & /*funcName*/) const {
  return false;
}

bool VolcanoOpenclRuntime::isFakeWriteImage(
    const std::string & /*funcName*/) const {
  return false;
}

VolcanoOpenclRuntime::VolcanoOpenclRuntime(ArrayRef<Module *> runtimeModuleList)
    : OpenclRuntime(runtimeModuleList, volacanoScalarSelect) {}

bool VolcanoOpenclRuntime::isTransposedReadImg(
    const std::string & /*funcName*/) const {
  return false;
}

Function *VolcanoOpenclRuntime::getWriteStream(bool /*isPointer64Bit*/) const {
  return nullptr;
}

bool VolcanoOpenclRuntime::isTransposedWriteImg(
    const std::string & /*funcName*/) const {
  return false;
}

Function *VolcanoOpenclRuntime::getReadStream(bool /*isPointer64Bit*/) const {
  return nullptr;
}

bool VolcanoOpenclRuntime::isStreamFunc(
    const std::string & /*funcName*/) const {
  return false;
}
} // namespace intel

/// Support for static linking of modules for Windows
/// This pass is called by a modified Opt.exe
extern "C" {
intel::RuntimeServices *
createVolcanoOpenclRuntimeSupport(ArrayRef<Module *> runtimeModuleList) {
  return new intel::VolcanoOpenclRuntime(runtimeModuleList);
}
}
