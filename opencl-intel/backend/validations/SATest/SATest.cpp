// INTEL CONFIDENTIAL
//
// Copyright 2010 Intel Corporation.
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

#include "SATest.h"
#include "Comparator.h"
#include "ComparisonResults.h"
#include "IComparisonResults.h"
#include "IProgram.h"
#include "IProgramRunner.h"
#include "IRunResult.h"
#include "IRunnerFactory.h"
#include "OpenCLProgramConfiguration.h"
#include "OpenCLRunConfiguration.h"
#include "OpenCLStamp.h"
#include "PerformancePrinter.h"
#include "SATestException.h"
#include "cl_env.h"

#include <cassert>
#include <iostream>
#include <map>
#include <memory>
#include <string>

using namespace Validation;
using std::string;

SATest::SATest(const string &configFileName, const string &baseDirectory,
               IRunConfiguration *pRunConfiguration)
    : m_factory(RunnerFactory::GetInstance()), m_pProgramConfiguration(NULL),
      m_pProgram(NULL) {
  // validate execution environment
  ValidateEnvironment();
  m_pProgramConfiguration =
      m_factory.CreateProgramConfiguration(configFileName, baseDirectory);
  m_pProgram =
      m_factory.CreateProgram(m_pProgramConfiguration, pRunConfiguration);

  const OpenCLProgramConfiguration *pProgramConfig =
      static_cast<const OpenCLProgramConfiguration *>(m_pProgramConfiguration);

  for (OpenCLProgramConfiguration::KernelConfigList::const_iterator it =
           pProgramConfig->beginKernels();
       it != pProgramConfig->endKernels(); ++it) {
    if (Random == (*it)->GetInputFileType()) {
      pRunConfiguration->SetForceReference(true);
      break;
    }
  }
}

SATest::~SATest(void) {
  delete m_pProgram;
  delete m_pProgramConfiguration;
}

void SATest::Run(TEST_MODE mode, IRunConfiguration *pRunConfiguration) {
  switch (mode) {
  case REFERENCE:
    RunReference(pRunConfiguration->GetReferenceRunnerConfiguration());
    break;
  case VALIDATION:
    RunValidation(pRunConfiguration);
    break;
  case PERFORMANCE:
    RunPerformance(pRunConfiguration->GetBackendRunnerConfiguration());
    break;
  case BUILD:
    RunBuildOnly(pRunConfiguration->GetBackendRunnerConfiguration());
    break;
  }
}

void SATest::RunValidation(IRunConfiguration *pRunConfiguration) {
  std::unique_ptr<IProgramRunner> spRunner(m_factory.CreateProgramRunner(
      pRunConfiguration->GetBackendRunnerConfiguration()));
  std::unique_ptr<IRunResultComparator> spComparator(
      m_factory.CreateComparator(m_pProgramConfiguration, pRunConfiguration));

  RunResult runResult;
  spRunner->Run(&runResult, m_pProgram, m_pProgramConfiguration,
                pRunConfiguration->GetBackendRunnerConfiguration());

  if (pRunConfiguration->UseReference()) {
    RunResult refResult;
    LoadOrGenerateReference(pRunConfiguration, &refResult);
    std::unique_ptr<IRunResultComparison> spCompResult(
        spComparator->Compare(&runResult, &refResult));

    if (spCompResult->isFailed()) {
      const OpenCLProgramConfiguration *pProgramConfig =
          static_cast<const OpenCLProgramConfiguration *>(
              m_pProgramConfiguration);
      for (OpenCLProgramConfiguration::KernelConfigList::const_iterator it =
               pProgramConfig->beginKernels();
           it != pProgramConfig->endKernels(); ++it) {
        if ((*it)->GetInputFileType() == Random &&
            spCompResult->GetComparison((*it)->GetKernelName().c_str())
                ->isFailed()) {
          std::cout << "Seed = "
                    << static_cast<const ReferenceRunOptions *>(
                           pRunConfiguration->GetReferenceRunnerConfiguration())
                           ->GetValue<uint64_t>(RC_COMMON_RANDOM_DG_SEED, 0)
                    << std::endl;
          break;
        } else if ((*it)->GetInputFileType() == Config &&
                   spCompResult->GetComparison((*it)->GetKernelName().c_str())
                       ->isFailed()) {
          std::cout << "Seed = " << (*it)->GetGeneratorConfig()->getSeed()
                    << std::endl;
          break;
        }
      }
      throw Exception::TestFailException("Comparison failed.");
    }
  }
  std::cout << "Test Passed." << std::endl;
}

void SATest::RunPerformance(
    const IRunComponentConfiguration *pRunConfiguration) {
  std::unique_ptr<IProgramRunner> spRunner(
      m_factory.CreateProgramRunner(pRunConfiguration));

  RunResult runResult;
  spRunner->Run(&runResult, m_pProgram, m_pProgramConfiguration,
                pRunConfiguration);

  PerformancePrinter printer(m_pProgramConfiguration, pRunConfiguration);
  runResult.GetPerformance().Visit(&printer);
}

void SATest::RunReference(const IRunComponentConfiguration *pRunConfiguration) {
  RunResult runResult;

  std::unique_ptr<IProgramRunner> spRunner(
      m_factory.CreateReferenceRunner(pRunConfiguration));
  GenerateReference(&runResult, spRunner.get(), pRunConfiguration);
  std::cout << "Reference output generated successfully" << std::endl;
}

void SATest::RunBuildOnly(const IRunComponentConfiguration *pRunConfiguration) {
  std::unique_ptr<IProgramRunner> spRunner(
      m_factory.CreateProgramRunner(pRunConfiguration));

  RunResult runResult;
  spRunner->Run(&runResult, m_pProgram, m_pProgramConfiguration,
                pRunConfiguration);
  std::cout << "Test program was successfully built." << std::endl;
}

void SATest::GenerateReference(
    IRunResult *pResult, IProgramRunner *pRunner,
    const IRunComponentConfiguration *pRunConfiguration) {
  // execute reference
  pRunner->Run(pResult, m_pProgram, m_pProgramConfiguration, pRunConfiguration);
  pRunner->StoreOutput(pResult, m_pProgramConfiguration);
}

void SATest::LoadOrGenerateReference(IRunConfiguration *pRunConfiguration,
                                     IRunResult *pResult) {
  std::unique_ptr<IProgramRunner> spRefRunner(m_factory.CreateReferenceRunner(
      pRunConfiguration->GetReferenceRunnerConfiguration()));
#if STAMP_ENABLED
  OCLStamp stamp(pRunConfiguration->GetReferenceRunnerConfiguration(),
                 m_pProgramConfiguration, m_pProgram);

  stamp.generateStamps();
#endif
  if (pRunConfiguration->GetForceReference()) {
    GenerateReference(pResult, spRefRunner.get(),
                      pRunConfiguration->GetReferenceRunnerConfiguration());
  } else {
    try {
      spRefRunner->LoadOutput(pResult, m_pProgramConfiguration);
    } catch (Exception::IOError &) {
      GenerateReference(pResult, spRefRunner.get(),
                        pRunConfiguration->GetReferenceRunnerConfiguration());
    }
  }
}

void SATest::ValidateEnvironment() {
  // fail if environment variable OCLBACKEND_PLUGINS is enabled
  // this will cause improper functionality of backend
  // Variable string is defined in
  // trunk/src/backend/ocl_cpu_backend/plugin_manager.cpp
  std::string Env;
  if (Intel::OpenCL::Utils::getEnvVar(Env, "OCLBACKEND_PLUGINS")) {
    throw Exception::InvalidArgument(
        "Environment variable OCLBACKEND_PLUGINS exists. "
        "For correct SATest functionality please remove it.");
  }
}
