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

File Name:  DXReferenceRunner.h

\*****************************************************************************/
#ifndef DX_REFERENCE_RUNNER_H
#define DX_REFERENCE_RUNNER_H

#include "IProgramRunner.h"

namespace Validation
{
    /// @brief This class enables to run a single DirectX reference test
    class DXReferenceRunner : public IProgramRunner
    {
    public:

        // @brief Constructor
        DXReferenceRunner(void);

        /// @brief Destructor
        virtual ~DXReferenceRunner(void);

        /// @brief Executes a single test program
        /// @param [OUT] runResult Result of test program execution
        /// @param [IN] program Test program to execute
        /// @param [IN] programConfig Test program configuration options
        /// @param [IN] runConfig Configuration of the test run.
        virtual void Run(IRunResult* runResult, 
            IProgram* program,
            IProgramConfiguration* programConfig, 
            const IRunComponentConfiguration* runConfig);

        /// @brief Load the output from file
        /// @param [OUT] runResult Result of test program execution
        /// @param [IN] config Configuration of the test run containing the output file path.
        virtual void LoadOutput(IRunResult* pRunResult, IProgramConfiguration* pConfig);

        /// @brief Store the output to file
        /// @param [IN] runResult Result of test program execution
        /// @param [IN] config Configuration of the test run containing the output file path.
        virtual void StoreOutput(IRunResult* pRunResult, IProgramConfiguration* pConfig);
    };

}

#endif // DX_REFERENCE_RUNNER_H
