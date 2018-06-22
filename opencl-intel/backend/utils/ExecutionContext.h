// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
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

#ifndef __EXECUTION_CONTEXT_H__
#define __EXECUTION_CONTEXT_H__

#include "cpu_dev_limits.h"
#include "cl_dev_backend_api.h"
#include <cstddef>
#include <set>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  /// @brief  Work Info structure, contains these information:
  ///         uiWorkDim       - work dimension
  ///         GlobalOffset    - global offset (for each dimension)
  ///         GlobalSize      - global size (for each dimension)
  ///         LocalSize       - local sizes (for each dimension)
  ///         WGNumber        - number of work groups (for each dimension)
  ///         WGLoopIterCount - number of iteration the loop barrier executes
  struct sWorkInfo
  {
      unsigned int    uiWorkDim;
      size_t          GlobalOffset[MAX_WORK_DIM];
      size_t          GlobalSize[MAX_WORK_DIM];
      size_t          LocalSize[WG_SIZE_NUM][MAX_WORK_DIM];
      size_t          WGNumber[MAX_WORK_DIM];
      size_t          WGLoopIterCount;
  };
}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __EXECUTION_CONTEXT_H__
