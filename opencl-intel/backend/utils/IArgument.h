/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __IARGUMENT_H__
#define __IARGUMENT_H__

#include <cstddef>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  /// @brief  IArgument interface class descibes the functionality needed
  ///         to be implemented by arguments classes
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
    virtual void setValue(const char* pValue) = 0;
  
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __IARGUMENT_H__
