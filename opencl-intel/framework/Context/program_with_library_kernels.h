// INTEL CONFIDENTIAL
//
// Copyright 2020 Intel Corporation.
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

#ifndef FRAMEWORK_CONTEXT_PROGRAM_WITH_LIBRARY_KERNELS_H
#define FRAMEWORK_CONTEXT_PROGRAM_WITH_LIBRARY_KERNELS_H

#include "cl_shared_ptr.hpp"
#include "kernel.h"
#include "program.h"
#include <string>

namespace Intel {
namespace OpenCL {
namespace Framework {

class ProgramWithLibraryKernels : public Program {
public:
  PREPARE_SHARED_PTR(ProgramWithLibraryKernels)

  static SharedPtr<ProgramWithLibraryKernels>
  Allocate(SharedPtr<Context> Ctx, cl_uint NumDevices,
           SharedPtr<FissionableDevice> *Devices, std::string &KernelNames,
           cl_int *Ret) {
    return SharedPtr<ProgramWithLibraryKernels>(new ProgramWithLibraryKernels(
        Ctx, NumDevices, Devices, KernelNames, Ret));
  }

protected:
  ProgramWithLibraryKernels(SharedPtr<Context> Ctx, cl_uint NumDevices,
                            SharedPtr<FissionableDevice> *Devices,
                            std::string &KernelNames, cl_int *Ret);

  virtual ~ProgramWithLibraryKernels();
};

} // namespace Framework
} // namespace OpenCL
} // namespace Intel

#endif // #ifndef FRAMEWORK_CONTEXT_PROGRAM_WITH_LIBRARY_KERNELS_H
