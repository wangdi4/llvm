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

File Name:  OpenCLMICFactory.h

\*****************************************************************************/
#ifndef OPENCL_MIC_FACTORY_H
#define OPENCL_MIC_FACTORY_H

#include "IRunnerFactory.h"

#include <string>

namespace Validation
{
        /// @brief This class is a factory of OpenCL program, program runner and reference runner
    class OpenCLMICFactory : public IRunnerFactory
    {

    public:
        OpenCLMICFactory(void);
        ~OpenCLMICFactory(void);

        /// @brief Creates new OpenCL program
        virtual IProgram * CreateProgram(IProgramConfiguration* programConfig,
                                         IRunConfiguration* pRunConfiguration);

        /// @brief Creates new program configuration
        virtual IProgramConfiguration * CreateProgramConfiguration(const std::string& configFile, const std::string& baseDir);

        /// @brief Creates new run configuration
        virtual IRunConfiguration * CreateRunConfiguration();

        /// @brief Creates new OpenCL program runner
        virtual IProgramRunner * CreateProgramRunner(const IRunComponentConfiguration* pRunConfiguration);

        /// @brief Creates new OpenCL reference runner
        virtual IProgramRunner * CreateReferenceRunner(const IRunComponentConfiguration* pRunConfiguration);

        /// @brief Creates new run results comparator for given platform
        virtual IRunResultComparator* CreateComparator(IProgramConfiguration* pProgramConfiguration,
            IRunConfiguration* pRunConfiguration);
    };
}

#endif // OPENCL_MIC_FACTORY_H
