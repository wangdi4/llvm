/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

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