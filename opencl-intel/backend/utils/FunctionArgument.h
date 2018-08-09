// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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
    FunctionArgument(const char* pValue, size_t size, size_t alignment);
    
    /// @brief Interface implementation
    /// @brief Returns the size of this argument
    /// @returns  The size of this argument
    virtual size_t getSize() const { return m_size; }
    
    /// @brief Interface implementation
    /// @brief Returns the alignment of this argument
    /// @returns  The alignment of this argument
    virtual size_t getAlignment() const { return m_alignment; }
    
    /// @brief Interface implementation
    /// @brief  Returns the size with alignments needed to be done 
    ///         to destination pointer of this argument
    /// @returns  The aligned size of this argument
    virtual size_t getAlignedSize() const { return m_alignedSize; }
    
    /// @brief Interface implementation
    /// @brief Sets the value of this argument
    /// @param pValue       The src from which to copy the value 
    virtual void setValue(const char* pValue);

    /// @brief Gets the value of this argument
    /// @return The pointer to the value 
    virtual void* getValue() { return *((void**)m_pValue); }
    
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
