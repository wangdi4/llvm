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

#include "compile_data.h"
#include "plugin_interface.h"
#include "source_file.h"
#include <cstdio>
#include <cstdlib>

using namespace Intel::OpenCL;
using namespace Intel::OpenCL::Frontend;
using namespace Intel::OpenCL::DeviceBackend;

struct FePluginMock : ICLFrontendPlugin {
  FePluginMock() {}

  void OnLink(const LinkData *data) override { printf("on link was called\n"); }

  void OnCompile(const CompileData *data) override {
    SourceFile file("my name is", "slim shady", "");
    const_cast<CompileData *>(data)->sourceFile(file);
  }
};

struct NullPlugin : ICLDevBackendPlugin {
  void OnCreateBinary(const ICLDevBackendKernel_ *pKernel,
                      const _cl_work_description_type *pWorkDesc,
                      size_t bufSize, void *pArgsBuffer) override {}

  void OnCreateKernel(const ICLDevBackendProgram_ *pProgram,
                      const ICLDevBackendKernel_ *pKernel,
                      const void *pFunction) override {}

  void OnCreateProgram(const void *pBinary, size_t uiBinarySize,
                       const ICLDevBackendProgram_ *pProgram) override {}

  void OnReleaseProgram(const ICLDevBackendProgram_ *pProgram) override {}
};

struct DummyPlugin : IPlugin {
  FePluginMock *pfe;
  DeviceBackend::ICLDevBackendPlugin *pbe;

  DummyPlugin() : pfe(NULL), pbe(NULL) {}

  ICLFrontendPlugin *getFrontendPlugin() override {
    if (pfe)
      return pfe;
    pfe = new FePluginMock();
    return pfe;
  }

  DeviceBackend::ICLDevBackendPlugin *getBackendPlugin() override {
    if (pbe)
      return pbe;
    pbe = new NullPlugin();
    return pbe;
  }

  ~DummyPlugin() {
    if (pfe)
      delete pfe;
    if (pbe)
      delete pbe;
  }
};

extern "C" {
IPlugin *CreatePlugin() {
  printf("creating plugin\n");
  return new DummyPlugin();
}
void ReleasePlugin(IPlugin *p) {
  printf("releasing plugin\n");
  if (p)
    delete p;
}
}
