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

File Name:  SATest.h

\*****************************************************************************/
#ifndef SATest_H
#define SATest_H

#include "IProgramRunner.h"
#include "IProgram.h"
#include "IRunConfiguration.h"

#include "RunnerFactoryGenerator.h"
#include "BufferContainerList.h"
#include "RunResult.h"

#include <string>

namespace Validation
{
    /// @brief The main class that runs a single test 
    class SATest
    {
    public:

        /// @brief Constructor
        /// @param [IN] programType Type of the test program (OpenCL / DirectX)
        /// @param [IN] configFileName Name of file that contains run configurations
        /// @param [IN] baseDirectory Directory to use in data file lookup
        /// @param [IN] pRunConfiguration Run configuration file
        SATest( RunnerFactory::PROGRAM_TYPE programType,
                const std::string& configFileName,
                const std::string& baseDirectory,
                IRunConfiguration* pRunConfiguration);

        /// @brief Destructor
        virtual ~SATest(void);

        /// @brief Runs a single test
        /// @throws TestFailException In case the test failed
        void Run(TEST_MODE mode, IRunConfiguration* pRunConfiguration);

    private:
        /// @brief Loads or re-generate reference output.
        void LoadOrGenerateReference(IRunConfiguration* pRunConfiguration, IRunResult* pResult);

        /// @brief re-generate reference output.
        void RunReference(const IRunComponentConfiguration* pRunConfiguration);

        /// @brief Runs a single test and measure its performance
        void RunPerformance(const IRunComponentConfiguration* pRunConfiguration);

        /// @brief Runs a single test and measure its performance
        void RunValidation(IRunConfiguration* pRunConfiguration);

        /// @brief Generate the reference output
        void GenerateReference(IRunResult* pResult,
                               IProgramRunner* pRunner,
                               const IRunComponentConfiguration* pRunConfiguration);

        /// @brief Runs a single test in build only mode
        void RunBuildOnly(const IRunComponentConfiguration* pRunConfiguration);

    private:

        /// @brief Runner factory generator
        IRunnerFactory& m_factory;

        /// @brief Test run configuration
        IProgramConfiguration * m_pProgramConfiguration;

        /// @brief Test program
        IProgram * m_pProgram;
    };
}

#endif // SATest_H