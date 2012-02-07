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

File Name:  IProgramRunner.h

\*****************************************************************************/
#ifndef I_PROGRAM_RUNNER_H
#define I_PROGRAM_RUNNER_H

#include "IRunResult.h"
#include "IProgram.h"
#include "IRunConfiguration.h"
#include "IProgramConfiguration.h"
#include "IBufferContainerList.h"

namespace Validation
{
    /// @brief Interface for the test program runners.
    class IProgramRunner
    {
    public:

        virtual ~IProgramRunner(void) {}

        /// @brief Executes a single test program
        /// @param [OUT] runResult Result of test program execution
        /// @param [IN] program Test program to execute
        /// @param [IN] programConfig Test program configuration options
        /// @param [IN] runConfig Configuration of the test run.
        virtual void Run(IRunResult* pRunResult, 
                         IProgram * pProgram, 
                         IProgramConfiguration* programConfig, 
                         const IRunComponentConfiguration* runConfig) = 0;
    
        /// @brief Load pre-computed test output data from the file specified in program configuration.
        /// @param [OUT] runResult Result of test program execution
        /// @param [IN] config Configuration of the test run
        virtual void LoadOutput(IRunResult* pRunResult, IProgramConfiguration* pConfig) = 0;

        /// @brief Saves test execution results to the file specified in program configuration.
        /// @param [IN] runResult Result of test program execution
        /// @param [IN] config Configuration of the test run
        virtual void StoreOutput(IRunResult* pRunResult, IProgramConfiguration* pConfig) = 0;
    };
}


#endif // I_PROGRAM_RUNNER_H
