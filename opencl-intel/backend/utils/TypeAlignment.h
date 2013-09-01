/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#ifndef __TYPE_ALIGNMRENT_H__
#define __TYPE_ALIGNMRENT_H__

#include "cl_kernel_arg_type.h"
#include "common_dev_limits.h"
#include <cstring>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  /// @brief  TypeAlignment class used to provide alignment and size information
  ///         for cl_kernel_argument and general alignment utilities 
  ///         (aligning offsets and pointers)
  class TypeAlignment
  {
  public:
    
    /// @brief Returns the size of the given argument
    /// @param arg         The argument for which to return its size
    /// @returns The size of the given argument
    static size_t getSize(const cl_kernel_argument& arg);
    
    /// @brief Returns the alignment of the given argument
    /// @param arg         The argument for which to return its alignment
    /// @returns The alignment of the given argument
    static size_t getAlignment(const cl_kernel_argument& arg);
    
    /// @brief Returns offest aligned based on the given alignment
    /// @param alignment    The alignment
    /// @param offset       The offset to align
    /// @returns The offest aligned based on the given alignment
    static size_t align(size_t alignment, size_t offset);
    
    /// @brief Returns pointer aligned based on the given alignment
    /// @param alignment    The alignment
    /// @param pointer       The pointer to align
    /// @returns The pointer aligned based on the given alignment
    static char* align(size_t alignment, const char* ptr);
    
  public:
    // Represents the maximum alignment
    static const size_t MAX_ALIGNMENT = DEV_MAXIMUM_ALIGN;
  };
  
}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {


#endif // __TYPE_ALIGNMRENT_H__
