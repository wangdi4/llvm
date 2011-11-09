/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  IPerformance.h

\*****************************************************************************/
#ifndef I_PERFORMANCE_H
#define I_PERFORMANCE_H

#include "CL/cl_platform.h"
#include <string>

namespace Validation
{
  /// @brief This class enables test execution performance measurements.
  /// The measurements consist of build time and execution time.
  class IPerformance
  {
  public:
    /// @brief Returns build time
    /// @return Build time 
    /// returns -1 otherwise
    virtual cl_long GetBuildTime() const = 0;

    /// @brief Returns execution time
    /// @return Execution time for specified kernel
    /// returns -1 otherwise
    virtual cl_long GetExecutionTime(const std::string& name) const = 0;
  };
}

#endif // I_PERFORMANCE_H
