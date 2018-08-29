// INTEL CONFIDENTIAL
//
// Copyright 2011-2018 Intel Corporation.
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
