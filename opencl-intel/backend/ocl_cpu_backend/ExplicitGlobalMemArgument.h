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

File Name:  ExplicitGlobalMemArgument.h

\*****************************************************************************/

#ifndef __EXPLICIT_GLOBAL_MEM_ARGUMENT_H__
#define __EXPLICIT_GLOBAL_MEM_ARGUMENT_H__

#include "ExplicitArgument.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  // TODO : rename this class? it is used both for global and constsant mem args
  class ExplicitGlobalMemArgument : public ExplicitArgument {
  
  public:
  
    /// @brief Constructor
    /// @param pValue           Implict argument's value destination pointer
    /// @param arg              OpenCL argument
    ExplicitGlobalMemArgument(char* pValue, const cl_kernel_argument& arg)
    : ExplicitArgument(pValue, arg) { }
    
    /// @brief Overriding implementation
    /// @brief Sets the value of this argument
    /// @param pValueSrc       The src from which to copy the value 
    virtual void setValue(char* pValue) {
      // The src is a mem descriptor and we need to extract the pointer to memory
      void* pGlobalData = (*(cl_mem_obj_descriptor**)pValue)->pData;
  
      ExplicitArgument::setValue(reinterpret_cast<char *>(&pGlobalData));
    }
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __EXPLICIT_GLOBAL_MEM_ARGUMENT_H__
