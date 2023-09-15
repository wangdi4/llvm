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

/*
 *
 * File plugin_interface.h
 * IPlugin is an interface that each plugin should implement
 *
 */
#pragma once

#include <cstdlib>

#ifdef __cplusplus
extern "C" {
#endif

struct _cl_prog_container_header;
struct _cl_work_description_type;

#ifdef __cplusplus
}
#endif

namespace llvm {
class Function;
}

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {
class ICLDevBackendKernel_;
class ICLDevBackendProgram_;

/**
 * ICLDevBackendPlugin is an interface that each plugin should implement
 */
class ICLDevBackendPlugin {
public:
  virtual ~ICLDevBackendPlugin() {}

  virtual void OnCreateBinary(const ICLDevBackendKernel_ *pKernel,
                              const _cl_work_description_type *pWorkDesc,
                              size_t bufSize, void *pArgsBuffer) = 0;

  virtual void OnCreateKernel(const ICLDevBackendProgram_ *pProgram,
                              const ICLDevBackendKernel_ *pKernel,
                              const void *pFunction) = 0;

  virtual void OnCreateProgram(const void *pBinary, size_t uiBinarySize,
                               const ICLDevBackendProgram_ *pProgram) = 0;

  virtual void OnReleaseProgram(const ICLDevBackendProgram_ *pProgram) = 0;
};
} // namespace DeviceBackend

namespace Frontend {
class LinkData;
class CompileData;
/*
 * interface for Front end pluging to implement
 */
struct ICLFrontendPlugin {
  //
  // invoked when a program is being linked
  virtual void OnLink(const LinkData *linkData) = 0;
  //
  // invoked when a program is being compiled
  virtual void OnCompile(const CompileData *compileData) = 0;

  virtual ~ICLFrontendPlugin() {}
};
} // namespace Frontend

// Serves as a query system for BE/FE plugin
struct IPlugin {
  // When implemented in derived classes, should return a pointer to a backend
  // pluging. Note: NULL valude is not legal.
  virtual DeviceBackend::ICLDevBackendPlugin *getBackendPlugin() = 0;
  // When implemented in derived classes, should return a pointer to a frontend
  // pluging. Note: NULL valude is not legal.
  virtual Frontend::ICLFrontendPlugin *getFrontendPlugin() = 0;
  virtual ~IPlugin() {}
};
} // namespace OpenCL
} // namespace Intel

#ifdef __cplusplus
extern "C" {
#endif

//
// BE factory/release methods
//
typedef Intel::OpenCL::IPlugin *(*PLUGIN_CREATE_FUNCPTR)(void);
typedef void (*PLUGIN_RELEASE_FUNCPTR)(Intel::OpenCL::IPlugin *);

#ifdef __cplusplus
}
#endif
