/*=================================================================================
Copyright (c) 2012, Intel Corporation
Subject to the terms and conditions of the Master Development License
Agreement between Intel and Apple dated August 26, 2005; under the Category 2 Intel
OpenCL CPU Backend Software PA/License dated November 15, 2012 ; and RS-NDA #58744
==================================================================================*/

#include "FunctionArgument.h"
#include "TypeAlignment.h"

#include <cstring>
#include <algorithm>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

FunctionArgument::FunctionArgument(const char* pValue, size_t size, size_t alignment)
  : 
    m_size(size), 
    m_alignment(alignment) {

    // Get aligned destination
    m_pValue = TypeAlignment::align(m_alignment, pValue);
    
    m_alignedSize = (m_pValue - pValue);  // Get actual alignment
    m_alignedSize += m_size;              // Add size
}
    
void FunctionArgument::setValue(const char* pValue) {
  
    // TODO : assert pointers not null?
    // Copy value from given src to dest
    std::copy(pValue, pValue+m_size, m_pValue);
    //memcpy(m_pValue, pValue, m_size);
}

    
}}} // namespace Intel { namespace OpenCL { namespace DeviceBackend {