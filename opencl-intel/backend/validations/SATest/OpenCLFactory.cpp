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

#include "OpenCLFactory.h"
#include "OpenCLBackendWrapper.h"
#include "OpenCLCPUBackendRunner.h"
#include "OpenCLComparator.h"
#include "OpenCLProgram.h"
#include "OpenCLReferenceRunner.h"
#include "OpenCLRunConfiguration.h"
#include "RunResult.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

// debug macro
#define DEBUG_TYPE "OpenCLFactory"

using namespace Validation;
using std::string;

OpenCLFactory::OpenCLFactory(void) {}

OpenCLFactory::~OpenCLFactory(void) {}

IProgram *OpenCLFactory::CreateProgram(IProgramConfiguration *programConfig,
                                       IRunConfiguration *runConfig) {
  OpenCLProgramConfiguration *pOCLProgramConfig =
      static_cast<OpenCLProgramConfiguration *>(programConfig);
  BERunOptions *pBERunConfig =
      static_cast<BERunOptions *>(runConfig->GetBackendRunnerConfiguration());
  std::string arch = pBERunConfig->GetValue<std::string>(RC_BR_CPU_ARCHITECTURE,
                                                         std::string(""));
  pBERunConfig->SetValue<int>(RC_BR_DEVICE_MODE,
                              pOCLProgramConfig->GetDeviceMode());
  return new OpenCLProgram(pOCLProgramConfig, arch);
}

IProgramConfiguration *
OpenCLFactory::CreateProgramConfiguration(const string &configFile,
                                          const string &baseDir) {
  return new OpenCLProgramConfiguration(configFile, baseDir);
}

IRunConfiguration *OpenCLFactory::CreateRunConfiguration() {
  return new OpenCLRunConfiguration();
}

IProgramRunner *OpenCLFactory::CreateProgramRunner(
    const IRunComponentConfiguration *pRunConfiguration) {
  const BERunOptions *runConfig =
      static_cast<const BERunOptions *>(pRunConfiguration);
  return new OpenCLCPUBackendRunner(*runConfig);
}

IProgramRunner *OpenCLFactory::CreateReferenceRunner(
    const IRunComponentConfiguration *pRunConfiguration) {
  return new OpenCLReferenceRunner(
      static_cast<const ReferenceRunOptions *>(pRunConfiguration)
          ->GetValue<bool>(RC_REF_USE_NEAT, false));
}

IRunResultComparator *
OpenCLFactory::CreateComparator(IProgramConfiguration *pProgramConfiguration,
                                IRunConfiguration *pRunConfiguration) {
  return new OpenCLComparator(
      static_cast<OpenCLProgramConfiguration *>(pProgramConfiguration),
      static_cast<const ComparatorRunOptions *>(
          pRunConfiguration->GetComparatorConfiguration()),
      static_cast<const ReferenceRunOptions *>(
          pRunConfiguration->GetReferenceRunnerConfiguration()));
}
