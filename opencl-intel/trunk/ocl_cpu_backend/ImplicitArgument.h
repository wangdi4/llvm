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

File Name:  ImplicitArgument.h

\*****************************************************************************/

#ifndef __IMPLICIT_ARGUMENT_H__
#define __IMPLICIT_ARGUMENT_H__

#include "FunctionArgument.h"
#include "ImplicitArgProperties.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  class ImplicitArgument : public FunctionArgument {
  
  public:
    /// @brief Constructor
    /// @param pValue           Implict argument's value destination pointer
    /// @param implicitArgProps Implicit argument properties
    ImplicitArgument(char* pValue, const ImplicitArgProperties& implicitArgProps)
    : FunctionArgument(pValue, implicitArgProps.getSize(), implicitArgProps.getAlignment()) { }
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __IMPLICIT_ARGUMENT_H__
