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

#ifndef OCL_CPU_BACKEND_LIBRARY_PROGRAM_MANAGER_H
#define OCL_CPU_BACKEND_LIBRARY_PROGRAM_MANAGER_H

#include "Program.h"
#include "cl_device_api.h"
#include <memory>
#include <string>

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {
class IAbstractBackendFactory;
class CPUProgramBuilder;

class LibraryProgramManager {
public:
  LibraryProgramManager(const LibraryProgramManager &) = delete;
  void operator=(const LibraryProgramManager &) = delete;

  static void Init();

  static void Terminate();

  static LibraryProgramManager *getInstance();

  void createProgram(IAbstractBackendFactory *Factory, CPUProgramBuilder &PB);

  void release() { m_program.reset(nullptr); }

  Program *getProgram() { return m_program.get(); }

  const char *getKernelNames() { return m_kernelNames.c_str(); }

private:
  LibraryProgramManager(){};
  ~LibraryProgramManager(){};

  static LibraryProgramManager *m_instance;

  // Holds program object.
  std::unique_ptr<Program> m_program;

  // List of kernel names separated by ";"
  std::string m_kernelNames;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel

#endif // #ifndef OCL_CPU_BACKEND_LIBRARY_PROGRAM_MANAGER_H
