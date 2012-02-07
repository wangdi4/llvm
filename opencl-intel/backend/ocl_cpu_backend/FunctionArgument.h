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

File Name:  FunctionArgument.h

\*****************************************************************************/

#ifndef __FUNCTION_ARGUMENT_H__
#define __FUNCTION_ARGUMENT_H__

#include "IArgument.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  class FunctionArgument : public IArgument {
  
  public:
  
    /// @brief Constructor
    /// @param pValue           Implict argument's value destination pointer
    /// @param size             Implict argument's size
    /// @param alignment        Implict argument's alignment
    FunctionArgument(char* pValue, size_t size, size_t alignment);
    
    /// @brief Interface implementation
    /// @brief Returns the size of this argument
    /// @returns  The size of this argument
    virtual size_t getSize() const { return m_size; }
    
    /// @brief Interface implementation
    /// @brief Returns the alignment of this argument
    /// @returns  The alignment of this argument
    virtual size_t getAlignment() const { return m_alignment; };
    
    /// @brief Interface implementation
    /// @brief  Returns the size with alignments needed to be done 
    ///         to destination pointer of this argument
    /// @returns  The aligned size of this argument
    virtual size_t getAlignedSize() const { return m_alignedSize; };
    
    /// @brief Interface implementation
    /// @brief Sets the value of this argument
    /// @param pValue       The src from which to copy the value 
    virtual void setValue(char* pValue);
    
  protected:
    
    /// @brief Implict argument's value destination pointer
    char* m_pValue;
    
    /// @brief Implict argument's size
    size_t m_size;
    
    /// @brief Implict argument's alignment
    size_t m_alignment;
    
    /// @brief Implict argument's size + destination pointer alignment
    size_t m_alignedSize;
  
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __FUNCTION_ARGUMENT_H__