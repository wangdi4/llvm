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

File Name:  OpenCLComparator.h

\*****************************************************************************/
#ifndef OPENCL_COMPARATOR_H
#define OPENCL_COMPARATOR_H

#include <vector>
#include <string>
#include "cl_device_api.h"
#include "cl_dev_backend_api.h"
#include "IRunResultComparator.h"

using namespace Intel::OpenCL::DeviceBackend;

namespace Validation
{
    class OpenCLProgramConfiguration;
    class ComparatorRunOptions;
    class ReferenceRunOptions;
    class IRunResultComparison;
    class IRunResult;
    class IProgram;

    /// @brief Responsible for comparing the run results of the given program
    class OpenCLComparator: public IRunResultComparator
    {
    public:
        // TODO: At the moment, RunResult objects doesn't know if it contains valid NEAT data, so we forced to pass
        // Reference run options and assume if NEAT is enabled then RunResult must contain NEAT data. To fix this 
        // design bug we need to re-design RunResult object (including interface).
        OpenCLComparator(const OpenCLProgramConfiguration* pProgramConfig,
                         const ComparatorRunOptions* pComparatorConfig,
                         const ReferenceRunOptions* pRefConfig);

        /// @brief Compares the result of two runs
        virtual IRunResultComparison* Compare( IRunResult* volcanoRunResults, IRunResult* referenceRunResults ) const;

    private:
        bool    m_useNeat;
        double  m_ULPTolerance;
        bool    m_detailedStat;
        std::vector<std::string> m_kernels;
    };
}

#endif // RUN_RESULTS_H
