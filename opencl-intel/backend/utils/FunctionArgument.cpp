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
