/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  ExecutionContext.h

\*****************************************************************************/

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
  ///         LocalSize       - local size (for each dimension)
  ///         WGNumber        - number of work groups (for each dimension)
  ///         WGLoopIterCount - number of iteration the loop barrier executes
  struct sWorkInfo
  {
      unsigned int    uiWorkDim;
      size_t          GlobalOffset[MAX_WORK_DIM];
      size_t          GlobalSize[MAX_WORK_DIM];
      size_t          LocalSize[MAX_WORK_DIM];
      size_t          WGNumber[MAX_WORK_DIM];
      size_t          WGLoopIterCount;
  };
}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __EXECUTION_CONTEXT_H__
