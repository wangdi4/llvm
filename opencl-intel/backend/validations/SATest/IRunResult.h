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

File Name:  IRunResult.h

\*****************************************************************************/
#ifndef I_RUN_RESULT_H
#define I_RUN_RESULT_H

#include "IPerformance.h"
#include "IBufferContainerList.h"
#include "IComparisonResults.h"

namespace Validation
{
    /// @brief Contains the result of comparison for entire run result
    class IRunResultComparison
    {
    public:
        virtual ~IRunResultComparison(){}

        /// @brief Returns comparison for specific kernel
        /// @return comparison result
        virtual IComparisonResults* GetComparison( const char* name ) = 0;

        /// @brief Returns the overall result of comparison
        virtual bool isFailed() const = 0;
    };

    /// @brief Contains information about the execution of the test.
    /// The information consists of output and performence measurements.
    class IRunResult
    {
    public:
        virtual ~IRunResult(){}

        /// @brief Returns test execution output
        /// @return Test output
        virtual IBufferContainerList& GetOutput(const char* kernelName) = 0;

        /// @brief Returns test execution NEAT output
        /// @return NEAT output
        virtual IBufferContainerList& GetNEATOutput(const char* kernelName) = 0;

        /// @brief Returns vector of flags signaling comparator to omit corresponding argument.
        virtual const std::vector<bool>* GetComparatorIgnoreList(const char* kernelName) = 0;

        /// @brief Set vector of flags signaling comparator to omit corresponding argument.
        virtual void SetComparatorIgnoreList(const char* kernelName, const std::vector<bool>&) = 0;

        /// @brief Returns the count of output buffers
        /// @return Test output
        virtual size_t GetOutputsCount() const = 0;

        /// @brief Returns test execution performance measurements
        /// @return Test performance measurements
        virtual IPerformance& GetPerformance() = 0;

    };
}

#endif // I_RUN_RESULT_H
