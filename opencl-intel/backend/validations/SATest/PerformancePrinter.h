// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
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

#ifndef PERFORMANCEPRINTER_H
#define PERFORMANCEPRINTER_H

#include "IPerformance.h"
#include "IProgramConfiguration.h"
#include "IRunConfiguration.h"
#include <fstream>
#include <iostream>
#include <string>

namespace Validation {

class PerformancePrinter : public IPerformanceVisitor {
public:
  PerformancePrinter(const IProgramConfiguration *pProgramConfiguration,
                     const IRunComponentConfiguration *pRunConfiguration);

private:
  void OnKernelSample(const std::string &kernel, unsigned int vectorSize,
                      cl_long buildTicks, double buildSDMean,
                      cl_long executionTicks, double executionSDMean,
                      cl_long serializationTicks, double serializationSDMean,
                      cl_long deserializationTicks,
                      double deserializationSDMean) override;

private:
  std::string m_IRFilename;
  std::string m_JITFilename;
  std::string m_programName;
  std::fstream m_fOutStream;
  std::ostream *m_pOutStream;
};

} // namespace Validation
#endif // PERFORMANCE_H
