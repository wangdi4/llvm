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

File Name:  RunnerFactoryGenerator.h

\*****************************************************************************/
#ifndef RUNNER_FACTORY_GENERATOR_H
#define RUNNER_FACTORY_GENERATOR_H

#include "IRunnerFactory.h"

namespace Validation
{
    /// @brief IRunnerFactory abstract factory implementation
    class RunnerFactory
    {
    public:
        /// @brief PROGRAM_TYPE indicates whether the test program is an OpelCL kernel
        /// or DirectX shader
        typedef enum {OPEN_CL, DIRECT_X} PROGRAM_TYPE;

        /// @brief Returns appropriate instance of a runner factory
        /// @return Instance of a runner factory
        static IRunnerFactory& GetInstance(PROGRAM_TYPE);

    private:
        /// @brief Ctor.
        RunnerFactory();

        /// @brief Instance of a runner factory
        static IRunnerFactory * s_dxFactory;
        static IRunnerFactory * s_oclFactory;
    };
}

#endif // RUNNER_FACTORY_GENERATOR_H