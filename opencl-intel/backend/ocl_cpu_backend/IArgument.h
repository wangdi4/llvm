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

File Name:  IArgument.h

\*****************************************************************************/

#ifndef __IARGUMENT_H__
#define __IARGUMENT_H__

#include <cstddef>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  /// @brief  IArgument interface class descibes the functionality needed
  ///         to be implemented by arguments classes
  /// @Author Marina Yatsina
  class IArgument {
  
  public:
    
    /// @brief Returns the size of this argument
    /// @returns  The size of this argument
    virtual size_t getSize() const = 0;
    
    /// @brief Returns the alignment of this argument
    /// @returns  The alignment of this argument
    virtual size_t getAlignment() const = 0;
    
    /// @brief  Returns the size of this argument with needed alignment considurations 
    ///         (i.e. considarating alignment of the destination pointer of this argument)
    /// @returns  The aligned size of this argument
    virtual size_t getAlignedSize() const = 0;
    
    /// @brief Sets the value of this argument
    /// @param pValue       The src from which to copy the value
    virtual void setValue(char* pValue) = 0;
  
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __IARGUMENT_H__
