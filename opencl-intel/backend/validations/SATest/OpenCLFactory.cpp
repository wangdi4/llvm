/*****************************************************************************\

Copyright (c) Intel Corporation (2011-2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  OpenCLFactory.cpp

\*****************************************************************************/
#include "OpenCLFactory.h"

#include "RunResult.h"
#include "OpenCLProgram.h"
#include "OpenCLCPUBackendRunner.h"
#include "OpenCLReferenceRunner.h"
#include "OpenCLRunConfiguration.h"
#include "OpenCLBackendWrapper.h"
#include "OpenCLComparator.h"
#if defined(SATEST_INCLUDE_MIC_DEVICE)
#include "OpenCLMICBackendRunner.h"
#endif

#define DEBUG_TYPE "OpenCLFactory"
// debug macros
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace Validation;
using std::string;

OpenCLFactory::OpenCLFactory(void)
{
}

OpenCLFactory::~OpenCLFactory(void)
{
}

IProgram * OpenCLFactory::CreateProgram(IProgramConfiguration* programConfig,
                                        IRunConfiguration* runConfig)
{
    OpenCLProgramConfiguration* pOCLProgramConfig = static_cast<OpenCLProgramConfiguration*>(programConfig);
    const BERunOptions* pBERunConfig = static_cast<const BERunOptions*>(runConfig->GetBackendRunnerConfiguration());
    std::string arch = pBERunConfig->GetValue<std::string>(RC_BR_CPU_ARCHITECTURE,std::string(""));

    return new OpenCLProgram(pOCLProgramConfig, arch);
}

IProgramConfiguration * OpenCLFactory::CreateProgramConfiguration(const string& configFile, const string& baseDir)
{
    return new OpenCLProgramConfiguration(configFile, baseDir);
}

IRunConfiguration * OpenCLFactory::CreateRunConfiguration()
{
    return new OpenCLRunConfiguration();
}

IProgramRunner * OpenCLFactory::CreateProgramRunner(const IRunComponentConfiguration* pRunConfiguration)
{
    const BERunOptions *runConfig = static_cast<const BERunOptions*>(pRunConfiguration);
#if defined(SATEST_INCLUDE_MIC_DEVICE)
    std::string cpuArch = runConfig->GetValue<std::string>(RC_BR_CPU_ARCHITECTURE, "auto");
    bool isSDEmode = runConfig->GetValue<bool>(RC_BR_USE_SDE, false);
    if ((std::string("auto-remote") == cpuArch ||
        std::string("knc") == cpuArch) &&
        !isSDEmode
        )
    {
        DEBUG(llvm::dbgs() << "CreateProgramRunner created back-end runner for MIC!" << "\n");
        return new OpenCLMICBackendRunner(*runConfig);
    }
#endif
    return new OpenCLCPUBackendRunner(*runConfig);
}

IProgramRunner * OpenCLFactory::CreateReferenceRunner(const IRunComponentConfiguration* pRunConfiguration)
{
    return new OpenCLReferenceRunner(static_cast<const ReferenceRunOptions*>(pRunConfiguration)->GetValue<bool>(RC_REF_USE_NEAT, false));
}

IRunResultComparator* OpenCLFactory::CreateComparator(IProgramConfiguration* pProgramConfiguration,
                                                      IRunConfiguration* pRunConfiguration)
{
    return new OpenCLComparator(static_cast<OpenCLProgramConfiguration*>(pProgramConfiguration),
        static_cast<const ComparatorRunOptions*>(pRunConfiguration->GetComparatorConfiguration()),
        static_cast<const ReferenceRunOptions*>(pRunConfiguration->GetReferenceRunnerConfiguration())
        );
}

