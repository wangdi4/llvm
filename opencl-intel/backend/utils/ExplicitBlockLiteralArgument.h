/*****************************************************************************\

Copyright (c) Intel Corporation (2010-2013).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  ExplicitBlockLiteralArgument.h

\*****************************************************************************/

#ifndef __EXPLICITBLOCKLITERAL_ARGUMENT_H__
#define __EXPLICITBLOCKLITERAL_ARGUMENT_H__

#include "FunctionArgument.h"
#include "TypeAlignment.h"
#include "cl_device_api.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  struct BlockLiteral;

  /// OpenCL2.0.
  /// BlockLiteral argument. Passed as 0th argument in enqueued kernel
  class ExplicitBlockLiteralArgument : public ExplicitArgument {
    const BlockLiteral *m_pBL;
  public:
    /// @brief Ctor saves BlockLiteral ptr to be used in setValue
    /// @param pValue           Implict argument's value destination pointer
    /// @param arg              OpenCL argument
    /// @param pBL              BlockLiteral pointers
    ExplicitBlockLiteralArgument (char* pValue, const cl_kernel_argument& arg, const BlockLiteral* pBL)
      : ExplicitArgument(pValue, arg), m_pBL(pBL) { }

    /// @brief Set argument as pointer to BlockLiteral structure. 
    ///        Hack for using inside Binary::InitParams():
    ///        actual BlockLiteral pointer is obtained via ctor
    /// @param   is ignored
    virtual void setValue(const char* ) {
      ExplicitArgument::setValue((const char*)&m_pBL);
    }
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __EXPLICITBLOCKLITERAL_ARGUMENT_H__
