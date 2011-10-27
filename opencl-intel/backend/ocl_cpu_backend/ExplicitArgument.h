/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  ExplicitArgument.h

\*****************************************************************************/

#ifndef __EXPLICIT_ARGUMENT_H__
#define __EXPLICIT_ARGUMENT_H__

#include "FunctionArgument.h"
#include "TypeAlignment.h"
#include "cl_device_api.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  class ExplicitArgument : public FunctionArgument {
  
  public:
    /// @brief Constructor
    /// @param pValue           Explicit argument's value destination pointer
    /// @param arg              OpenCL argument
    ExplicitArgument(char* pValue, const cl_kernel_argument& arg)
     : FunctionArgument(pValue, TypeAlignment::getSize(arg), TypeAlignment::getAlignment(arg)) { }
  
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __EXPLICIT_ARGUMENT_H__
