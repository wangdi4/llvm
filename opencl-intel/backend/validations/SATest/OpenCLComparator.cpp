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

#include "OpenCLComparator.h"
#include "Comparator.h"
#include "ComparisonResults.h"
#include "OpenCLProgramConfiguration.h"
#include "OpenCLRunConfiguration.h"
#include "RunResult.h"
#include <memory>

using namespace Validation;

OpenCLComparator::OpenCLComparator(
    const OpenCLProgramConfiguration *pProgramConfig,
    const ComparatorRunOptions *pComparatorConfig,
    const ReferenceRunOptions *pRefConfig)
    : m_useNeat(pRefConfig->GetValue<bool>(RC_REF_USE_NEAT, false)),
      m_ULPTolerance(
          pComparatorConfig->GetValue<double>(RC_COMP_ULP_TOLERANCE, 0.0)),
      m_detailedStat(
          pComparatorConfig->GetValue<bool>(RC_COMP_DETAILED_STAT, false)) {
  for (OpenCLProgramConfiguration::KernelConfigList::const_iterator it =
           pProgramConfig->beginKernels();
       it != pProgramConfig->endKernels(); ++it) {
    m_kernels.push_back((*it)->GetKernelName());
  }
}

/// @brief Compares the result of two runs
IRunResultComparison *
OpenCLComparator::Compare(IRunResult *runResults,
                          IRunResult *referenceRunResults) const {
  std::unique_ptr<RunResultComparison> spResult(new RunResultComparison());

  if (runResults->GetOutputsCount() != referenceRunResults->GetOutputsCount()) {
    spResult->SetIsFailedStatus(true);
  }

  Comparator comparator;
  comparator.SetULPTolerance(m_ULPTolerance);

  for (std::vector<std::string>::const_iterator i = m_kernels.begin(),
                                                e = m_kernels.end();
       i != e; ++i) {
    IBufferContainerList &output = runResults->GetOutput(i->c_str());
    IBufferContainerList &referenceOutput =
        referenceRunResults->GetOutput(i->c_str());
    IBufferContainerList *neatOutput =
        m_useNeat ? &referenceRunResults->GetNEATOutput(i->c_str()) : NULL;

    // run comparison for OpenCL kernel
    std::unique_ptr<ComparisonResults> spres(
        new ComparisonResults(*i, m_detailedStat));
    COMP_RESULT compRes = comparator.Compare(
        *spres, runResults->GetComparatorIgnoreList(i->c_str()), output,
        &referenceOutput, neatOutput);
    spres->Report();
    if (m_detailedStat) {
      spres->ReportDetail();
    }
    if (compRes == NOT_PASSED) {
      spResult->SetIsFailedStatus(true);
    }

    spResult->AddComparison(i->c_str(), spres.release());
  }
  return spResult.release();
}
