/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/
#ifndef __IMPLICIT_ARGUMENT_H__
#define __IMPLICIT_ARGUMENT_H__

#include "FunctionArgument.h"
#include "ImplicitArgProperties.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

  class ImplicitArgument : public FunctionArgument {
  
  public:
    /// @brief Constructor
    /// @param pValue           Implict argument's value destination pointer
    /// @param implicitArgProps Implicit argument properties
    ImplicitArgument(char* pValue, const ImplicitArgProperties& implicitArgProps)
    : FunctionArgument(pValue, implicitArgProps.m_size, implicitArgProps.m_alignment) { }

    /// @brief Empty Constructor
    ImplicitArgument() : FunctionArgument(nullptr, 0, 0) {}
  };

}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {

#endif // __IMPLICIT_ARGUMENT_H__
