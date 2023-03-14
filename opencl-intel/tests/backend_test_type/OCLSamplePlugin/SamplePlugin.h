/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  SamplePlugin.h

\*****************************************************************************/

#ifndef SAMPLE_PLUGIN_H
#define SAMPLE_PLUGIN_H

#include "cl_dev_backend_api.h"
#include "cl_device_api.h"
#include "plugin_interface.h"
#include <cstddef>
#include <memory>

#define OCL_SAMPLEPLUGIN_EXPORTS

// defines the SamplePlugin APIs
#if defined(_WIN32)
#define OCL_SAMPLEPLUGIN_API __declspec(dllexport)
#else
#define OCL_SAMPLEPLUGIN_API
#endif

// FIXME 'using namespace' shouldn't be used in header file.
using namespace Intel::OpenCL::DeviceBackend;

/** @brief SamplePlugin used for testing the loading of the plugins in the
 * backend tests we test if the plugin is loaded by checking the 'pluginWorked'
 * flag before and after the call of OnCreateProgram
 */

class SamplePlugin : public ICLDevBackendPlugin {
public:
  SamplePlugin() {}
  ~SamplePlugin() {}

  /// @brief dummy implemention of the interface's method
  void OnCreateBinary(const ICLDevBackendKernel_ *pKernel,
                      const _cl_work_description_type *pWorkDesc,
                      size_t bufSize, void *pArgsBuffer) override {
    // do nothing
  }

  /// @brief dummy implemention of the interface's method
  void OnCreateKernel(const ICLDevBackendProgram_ *pProgram,
                      const ICLDevBackendKernel_ *pKernel,
                      const void *pFunction) override {
    // do nothing
  }

  /// @brief implementing the interface's method, the ONLY method that changes
  /// pluginWorked
  void OnCreateProgram(const void *pBinary, size_t uiBinarySize,
                       const ICLDevBackendProgram_ *pProgram) override {
    // we are using this event to check if the plugin is loaded (see backend
    // plugin tests)
    pluginWorked = true;
  }

  /// @brief dummy implemention of the interface's method
  void OnReleaseProgram(const ICLDevBackendProgram_ *pProgram) override {
    // do nothing
  }

  /// @brief we call this after we created a program to check if the plugin is
  /// loaded
  static bool DidPluginWork() { return pluginWorked; }

private:
  static bool pluginWorked;
};

#ifdef __cplusplus
extern "C" {
#endif
OCL_SAMPLEPLUGIN_API void ReleasePlugin(Intel::OpenCL::IPlugin *pPlugin);
#ifdef __cplusplus
}
#endif
/** @brief OclSamplePlugin used for returning the backend plugin SamplePlugin
 *         to the plugin manager
 */
class OclSamplePlugin : public Intel::OpenCL::IPlugin {
  friend void ReleasePlugin(Intel::OpenCL::IPlugin *);

public:
  /// @brief return the singleton instance
  static OclSamplePlugin *Instance();
  /// @brief return the backend plugin SamplePlugin to the plugin manager
  ICLDevBackendPlugin *getBackendPlugin() override;
  /// @brief dummy implemntion, SamplePlugin is only for testing in the backend
  Intel::OpenCL::Frontend::ICLFrontendPlugin *getFrontendPlugin() override;
  std::unique_ptr<SamplePlugin> ret;

private:
  /// @brief singleton instance
  static OclSamplePlugin *instance;

  OclSamplePlugin() {}
};

#endif // SAMPLE_PLUGIN_H
