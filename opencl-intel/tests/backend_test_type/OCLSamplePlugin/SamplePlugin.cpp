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

File Name:  SamplePlugin.cpp

\*****************************************************************************/
#include "SamplePlugin.h"
#include <assert.h>

using namespace Intel::OpenCL::DeviceBackend;

// return the singleton instance
OclSamplePlugin *OclSamplePlugin::Instance() {
  if (NULL == instance) {
    instance = new OclSamplePlugin();
  }
  return instance;
}

// return the backend plugin SamplePlugin to the plugin manager
ICLDevBackendPlugin *OclSamplePlugin::getBackendPlugin() {
  if (!ret) {
    ret.reset(new SamplePlugin());
  }
  return ret.get();
}

Intel::OpenCL::Frontend::ICLFrontendPlugin *
OclSamplePlugin::getFrontendPlugin() {
  // dummy
  return NULL;
}

OclSamplePlugin *OclSamplePlugin::instance = NULL;

// defines the exported functions for the DLL.
#ifdef __cplusplus
extern "C" {
#endif
OCL_SAMPLEPLUGIN_API Intel::OpenCL::IPlugin *CreatePlugin(void) {
  return OclSamplePlugin::Instance();
}

OCL_SAMPLEPLUGIN_API void ReleasePlugin(Intel::OpenCL::IPlugin *pPlugin) {
  if (OclSamplePlugin::instance == pPlugin)
    return; // don't delete singleton!! (other may have a fererence to it)
  assert(false && "where did this pointer came from??");
}

OCL_SAMPLEPLUGIN_API bool getTheFlag() { return SamplePlugin::DidPluginWork(); }
#ifdef __cplusplus
}
#endif

bool SamplePlugin::pluginWorked = false;
