// INTEL CONFIDENTIAL
//
// Copyright 2008 Intel Corporation.
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

#pragma once

#include "cl_device_api.h"
#include "cl_types.h"

namespace Intel {
namespace OpenCL {
namespace Framework {

// Pure interface class
// implement this interface if you want to be notified when program build
// process was finished in order to register the observer, call BuildProgram
// method in Device object
class IBuildDoneObserver {
public:
  virtual cl_err_code NotifyBuildDone(cl_device_id device,
                                      cl_build_status build_status) = 0;

  virtual ~IBuildDoneObserver(){}; // Virtual D'tor
};

class IFrontendBuildDoneObserver {
public:
  virtual cl_err_code NotifyFEBuildDone(cl_device_id device, size_t szBinSize,
                                        const void *pBinData,
                                        const char *pBuildLog) = 0;

  virtual ~IFrontendBuildDoneObserver(){}; // Virtual D'tor
};

// Pure interface class
// implement this interface if you want to be notified when specific command's
// status was changed in order to register the observer, call
class ICmdStatusChangedObserver {
public:
  virtual cl_err_code NotifyCmdStatusChanged(cl_int iCmdStatus,
                                             cl_int iCompletionResult,
                                             cl_ulong ulTimer) = 0;

  virtual ~ICmdStatusChangedObserver(){}; // Virtual D'tor
};
} // namespace Framework
} // namespace OpenCL
} // namespace Intel
