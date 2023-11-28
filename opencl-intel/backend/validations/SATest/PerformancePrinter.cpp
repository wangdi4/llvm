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

#include "PerformancePrinter.h"
#include "OpenCLRunConfiguration.h"

using namespace Validation;

PerformancePrinter::PerformancePrinter(
    const IProgramConfiguration *pProgramConfiguration,
    const IRunComponentConfiguration *pRunConfiguration) {
  const BERunOptions *pBERunConfig =
      static_cast<const BERunOptions *>(pRunConfiguration);

  m_programName = pProgramConfiguration->GetProgramName();
  m_IRFilename =
      pBERunConfig->GetValue<std::string>(RC_BR_DUMP_OPTIMIZED_LLVM_IR, "");
  m_JITFilename = pBERunConfig->GetValue<std::string>(RC_BR_DUMP_JIT, "");
  std::string csvFilename =
      pBERunConfig->GetValue<std::string>(RC_BR_PERF_LOG, "-");

  if (csvFilename == "-")
    m_pOutStream = &std::cout;
  else {
    m_fOutStream.open(csvFilename.c_str(), std::ios_base::out | std::ios::app);
    m_pOutStream = &m_fOutStream;
  }
}

void PerformancePrinter::OnKernelSample(
    const std::string &kernel, unsigned int vectorSize, cl_long buildTicks,
    double buildSDMean, cl_long executionTicks, double executionSDMean,
    cl_long serializationTicks, double serializationSDMean,
    cl_long deserializationTicks, double deserializationSDMean) {
  *m_pOutStream << m_programName << "." << kernel << "," << buildTicks << ","
                << buildSDMean << "," << executionTicks << ","
                << executionSDMean << "," << vectorSize
                << "," // Actualy vector size
                << m_IRFilename << "," << m_JITFilename << std::endl;
}
