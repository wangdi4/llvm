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

#include "LibraryProgramManager.h"
#include "CPUProgramBuilder.h"
#include "IAbstractBackendFactory.h"
#include "Program.h"
#include <iterator>
#include <sstream>

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

cl_dev_err_code
LibraryProgramManager::createProgram(IAbstractBackendFactory *Factory,
                                     CPUProgramBuilder &PB) {
  try {
    m_program.reset(Factory->CreateProgram());
    return PB.BuildLibraryProgram(m_program.get(), m_kernelNames);
  } catch (Exceptions::DeviceBackendExceptionBase &e) {
    return e.GetErrorCode();
  } catch (std::bad_alloc &) {
    return CL_DEV_OUT_OF_MEMORY;
  }
}

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
