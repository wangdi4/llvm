/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __IMPLICIT_ARG_PROPERTIES_H__
#define __IMPLICIT_ARG_PROPERTIES_H__

#include <string>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  /// @brief  ImplicitArgProperties struct used to describe each implicit argument
  struct ImplicitArgProperties {
    /// @brief Implicit argument's name
    const char* m_name;
      
    /// @brief Implicit argument's size
    size_t m_size; 
      
    /// @brief Implicit argument's alignment
    size_t m_alignment; 

    /// @brief Indicates if implicit argument is initialized by the wrapper
    bool m_bInitializedByWrapper; 
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __IMPLICIT_ARG_PROPERTIES_H__
