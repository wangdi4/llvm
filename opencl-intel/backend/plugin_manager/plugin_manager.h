// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
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

#ifndef __PLUGIN_MANAGER_H__
#define __PLUGIN_MANAGER_H__

#include "cl_device_api.h"
#include "compile_data.h"
#include "link_data.h"
#include <cstdlib>
#include <list>
#include <stdexcept>
#include <string>

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {
class ICLDevBackendKernel_;
class ICLDevBackendProgram_;
} // namespace DeviceBackend

struct IPlugin;
class PluginInfo;

class PluginManagerException : public std::runtime_error {
public:
  virtual ~PluginManagerException() throw();
  PluginManagerException(std::string message);
};

/**
 * PluginManager presents the facade infterface through which the OCL Backend
 * could notify the plugins about specific events.
 *
 * PluginManager is responsible for loading the plugins from OS upon
 * initialization. It will pass each notification to each loaded plugin in
 * sequence.
 *
 * Each plugin will be deployed as a separate DLL. There are several proposed
 * method for loading such plugins:
 *
 * 1. Each plugin's dll will be named with special prefix such that
 * PluginManager could scan the current working directory and load all such dlls
 * 2. There will be a special environment variable that will specify the list of
 * plugins to load.
 * 3. OCL Backend will use the configuration file ( currently not implemented)
 * to specify the needed plugins.
 */
class PluginManager {
public:
  ~PluginManager();
  PluginManager();

  PluginManager(const PluginManager &) = delete;
  PluginManager &operator=(const PluginManager &) = delete;

  /////////////////////////////////////////////////
  // Description:
  //   invoked by the OCL Backend, when the runtime initializes an NDRange.
  //   (could be as a result by call to function such as clEnqueueBuffer
  /////////////////////////////////////////////////
  void OnCreateBinary(const DeviceBackend::ICLDevBackendKernel_ *pKernel,
                      const _cl_work_description_type *pWorkDesc,
                      size_t bufSize, void *pArgsBuffer);

  /////////////////////////////////////////////////
  // Description:
  //  invoked by the OCL BE, as a result of a call to clCreateKernel by the
  //  application
  /////////////////////////////////////////////////
  void OnCreateKernel(const DeviceBackend::ICLDevBackendProgram_ *pProgram,
                      const DeviceBackend::ICLDevBackendKernel_ *pKernel,
                      const void *pFunction);

  /////////////////////////////////////////////////
  // Description:
  //   invoked by OCL BE, when a OCL Program object is created.
  //   (Program objects may be created by several function such as
  //   'clCreateProgramWithSource' and clCreateProgramWithBinary).
  /////////////////////////////////////////////////
  void OnCreateProgram(const void *pBinary, size_t uiBinarySize,
                       const DeviceBackend::ICLDevBackendProgram_ *pProgram);

  /////////////////////////////////////////////////
  // Description:
  //   invoked by OCL BE, when clReleaseProgram is called by the application.
  /////////////////////////////////////////////////
  void OnReleaseProgram(const DeviceBackend::ICLDevBackendProgram_ *pProgram);

  /////////////////////////////////////////////////
  // Description:
  //   invoked by clang, (compiler's FE), when a program is linked by
  //   clLinkProgram. This callback is only relevant to OCL 1.2 onwards.
  /////////////////////////////////////////////////
  void OnLink(const Frontend::LinkData *linkData);

  /////////////////////////////////////////////////
  // Description:
  //   invoked by clang, when compiling an open CL source file to llvm bytecode.
  /////////////////////////////////////////////////
  void OnCompile(const Frontend::CompileData *compileData);

private:
  //
  // Load the plugins which corresponds the OCLXXX_PLUGINS environment variables
  void LoadPlugins();

private:
  typedef std::list<PluginInfo *> PluginsList;
  // list of registered plugins
  PluginsList m_listPlugins;
  // Initialization state;
  bool m_bInitialized;
}; // end class PluginManager
} // namespace OpenCL
} // namespace Intel

#endif
