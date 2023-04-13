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
#include <iterator>
#include <sstream>

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

LibraryProgramManager *LibraryProgramManager::m_instance = nullptr;

void LibraryProgramManager::Init() {
  assert(!m_instance && "instance already initialized");
  m_instance = new LibraryProgramManager();
}

void LibraryProgramManager::Terminate() {
  if (m_instance) {
    delete m_instance;
    m_instance = nullptr;
  }
}

LibraryProgramManager *LibraryProgramManager::getInstance() {
  assert(m_instance && "instance doesn't exist");
  return m_instance;
}

void LibraryProgramManager::createProgram(IAbstractBackendFactory *Factory,
                                          CPUProgramBuilder &PB) {
  m_program.reset(Factory->CreateProgram());
  PB.BuildLibraryProgram(m_program.get(), m_kernelNames);
}

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
