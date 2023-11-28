// INTEL CONFIDENTIAL
//
// Copyright 2006 Intel Corporation.
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

#include "UserLoggerOutputParams.h"

using namespace Intel::OpenCL::Framework;
using std::string;

void OutputParamsValueProvider::Print2Logger() {
  if (CL_SUCCEEDED(m_apiLogger.GetLastRetVal())) {
    for (std::vector<ParamInfo>::const_iterator iter =
             m_outputParamsVec.begin();
         iter != m_outputParamsVec.end(); ++iter) {
      m_apiLogger.PrintOutputParam(iter->m_name, iter->m_paramName,
                                   iter->m_addr, iter->m_size,
                                   iter->m_bIsPtr2Ptr, iter->m_bIsUnsigned);
    }
    if (nullptr != m_specialPrinter) {
      const string str2Print = m_specialPrinter->GetStringToPrint();
      if (!str2Print.empty()) {
        m_apiLogger.PrintOutputParamStr((string(", ") + str2Print).c_str());
      }
    }
  }
}
