/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  OpenCLMICFactory.cpp

\*****************************************************************************/
#include "OpenCLMICFactory.h"

#include "RunResult.h"
#include "OpenCLProgram.h"
#include "OpenCLMICBackendRunner.h"
#include "OpenCLReferenceRunner.h"
#include "OpenCLRunConfiguration.h"
#include "OpenCLMICBackendWrapper.h"
#include "OpenCLComparator.h"

using namespace Validation;
using std::string;

OpenCLMICFactory::OpenCLMICFactory(void)
{
}

OpenCLMICFactory::~OpenCLMICFactory(void)
{
}

IProgram * OpenCLMICFactory::CreateProgram(IProgramConfiguration* programConfig,
                                           IRunConfiguration* pRunConfiguration)
{
    OpenCLProgramConfiguration *openCLProgramConfig = static_cast<OpenCLProgramConfiguration*>(programConfig);
    OpenCLRunConfiguration *runConfig = static_cast<OpenCLRunConfiguration*>(pRunConfiguration);
    const BERunOptions *beConfig = static_cast<const BERunOptions*>(runConfig->GetBackendRunnerConfiguration());
    std::string cpuArchitecture = beConfig->GetValue<std::string>(RC_BR_CPU_ARCHITECTURE,"");
    return new OpenCLProgram(openCLProgramConfig, cpuArchitecture);
}

IProgramConfiguration * OpenCLMICFactory::CreateProgramConfiguration(const string& configFile, const string& baseDir)
{
    return new OpenCLProgramConfiguration(configFile, baseDir);
}

IRunConfiguration * OpenCLMICFactory::CreateRunConfiguration()
{
    return new OpenCLRunConfiguration();
}

IProgramRunner * OpenCLMICFactory::CreateProgramRunner(const IRunComponentConfiguration* pRunConfiguration)
{
    return new OpenCLMICBackendRunner(pRunConfiguration);
}

IProgramRunner * OpenCLMICFactory::CreateReferenceRunner(const IRunComponentConfiguration* pRunConfiguration)
{
    return new OpenCLReferenceRunner(static_cast<const ReferenceRunOptions*>(pRunConfiguration)->GetValue<bool>(RC_REF_USE_NEAT, false));
}

IRunResultComparator* OpenCLMICFactory::CreateComparator(IProgramConfiguration* pProgramConfiguration,
                                                      IRunConfiguration* pRunConfiguration)
{
    return new OpenCLComparator(static_cast<OpenCLProgramConfiguration*>(pProgramConfiguration),
        static_cast<const ComparatorRunOptions*>(pRunConfiguration->GetComparatorConfiguration()),
        static_cast<const ReferenceRunOptions*>(pRunConfiguration->GetReferenceRunnerConfiguration())
        );
}

