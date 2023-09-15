// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
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

#include "cl_config.h"
#include "cl_user_logger.h"

namespace Intel {
namespace OpenCL {
namespace Framework {

/*******************************************************************************
 * Configuration keys
 ******************************************************************************/
#ifdef _WIN32
#define DEFAULT_LOG_FILE_NAME "C:\\cl.log"
#else
#define DEFAULT_LOG_FILE_NAME "~/intel_ocl.log"
#endif

// General configuration strings:
#define CL_CONFIG_LOG_FILE "CL_CONFIG_LOG_FILE" // string
#ifndef NDEBUG
#define CL_CONFIG_USE_LOGGER "CL_CONFIG_USE_LOGGER" // bool
#endif
#define CL_CONFIG_DEVICES                                                      \
  "CL_CONFIG_DEVICES" // string (use tokenize to get substrings)

#define CL_CONFIG_USE_ITT_API "CL_CONFIG_USE_ITT_API" // bool
#define CL_ITT_CONFIG_ENABLE_API_TRACING                                       \
  "CL_ITT_CONFIG_ENABLE_API_TRACING" // bool
#define CL_ITT_CONFIG_ENABLE_CONTEXT_TRACING                                   \
  "CL_ITT_CONFIG_ENABLE_CONTEXT_TRACING" // bool

// Used to Enable/Disable task state Markers in GPA Platform Analyzer
#define CL_ITT_CONFIG_SHOW_QUEUED_MARKER                                       \
  "CL_ITT_CONFIG_SHOW_QUEUED_MARKER" // bool
#define CL_ITT_CONFIG_SHOW_SUBMITTED_MARKER                                    \
  "CL_ITT_CONFIG_SHOW_SUBMITTED_MARKER" // bool
#define CL_ITT_CONFIG_SHOW_RUNNING_MARKER                                      \
  "CL_ITT_CONFIG_SHOW_RUNNING_MARKER" // bool
#define CL_ITT_CONFIG_SHOW_COMPLETED_MARKER                                    \
  "CL_ITT_CONFIG_SHOW_COMPLETED_MARKER" // bool

#define CL_CONFIG_ENABLE_PARALLEL_COPY "CL_CONFIG_ENABLE_PARALLEL_COPY"

/*******************************************************************************
 * Class name:  OCLConfig
 *
 * Description:  represents an OCLConfig object
 ******************************************************************************/
class OCLConfig : public Intel::OpenCL::Utils::BasicCLConfigWrapper {
public:
  OCLConfig();
  ~OCLConfig();

  string GetLogFile() const {
    return m_pConfigFile->Read<string>(CL_CONFIG_LOG_FILE,
                                       DEFAULT_LOG_FILE_NAME);
  }
  bool UseLogger() const {
#ifndef NDEBUG
    return m_pConfigFile->Read<bool>(CL_CONFIG_USE_LOGGER, false);
#else
    return false;
#endif
  }

  vector<string> GetDevices() const;
  string GetDefaultDevice() const;

  bool EnableAPITracing() const {
    return m_pConfigFile->Read<bool>(CL_ITT_CONFIG_ENABLE_API_TRACING, false);
  }
  bool EnableContextTracing() const {
    return m_pConfigFile->Read<bool>(CL_ITT_CONFIG_ENABLE_CONTEXT_TRACING,
                                     true);
  }

  bool ShowQueuedMarker() const {
    return m_pConfigFile->Read<bool>(CL_ITT_CONFIG_SHOW_QUEUED_MARKER, true);
  }
  bool ShowSubmittedMarker() const {
    return m_pConfigFile->Read<bool>(CL_ITT_CONFIG_SHOW_SUBMITTED_MARKER,
                                     false);
  }
  bool ShowRunningMarker() const {
    return m_pConfigFile->Read<bool>(CL_ITT_CONFIG_SHOW_RUNNING_MARKER, false);
  }
  bool ShowCompletedMarker() const {
    return m_pConfigFile->Read<bool>(CL_ITT_CONFIG_SHOW_COMPLETED_MARKER, true);
  }

  bool EnableITT() const {
    return m_pConfigFile->Read<bool>(CL_CONFIG_USE_ITT_API, false);
  }

  bool EnableParallelCopy() const {
    return m_pConfigFile->Read<bool>(CL_CONFIG_ENABLE_PARALLEL_COPY, true);
  }
};

} // namespace Framework
} // namespace OpenCL
} // namespace Intel
