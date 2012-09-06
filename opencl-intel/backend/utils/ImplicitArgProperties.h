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

File Name:  ImplicitArgProperties.h

\*****************************************************************************/

#ifndef __IMPLICIT_ARG_PROPERTIES_H__
#define __IMPLICIT_ARG_PROPERTIES_H__

#include <string>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  /// @brief  ImplicitArgProperties struct used to describe each implicit argument
  struct ImplicitArgProperties {
    /// @brief Implicit argument's name
    std::string m_name;
      
    /// @brief Implicit argument's size
    size_t m_size; 
      
    /// @brief Implicit argument's alignment
    size_t m_alignment; 
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __IMPLICIT_ARG_PROPERTIES_H__
