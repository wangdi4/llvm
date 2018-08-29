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

#ifndef RUN_RESULTS_H
#define RUN_RESULTS_H

#include "IRunResult.h"
#include "IComparisonResults.h"
#include "IBufferContainerList.h"
#include "IPerformance.h"

#include "BufferContainerList.h"
#include "Performance.h"

#include <map>
#include <vector>
#include <string>

namespace Validation
{
    /// @brief Responsible for holding the result of comparison
    ///        for entire test run ( possible for multiple kernels)
    class RunResultComparison: public IRunResultComparison
    {
    public:
        RunResultComparison(): m_failed(false){}

        ~RunResultComparison();

        IComparisonResults* GetComparison( const char* name ) ;

        bool isFailed() const { return m_failed; }

    public:
        void AddComparison( const char* name, IComparisonResults* comparison);

        void SetIsFailedStatus( bool failed ) { m_failed = failed; }

    private:
        typedef std::map<std::string, IComparisonResults*> CompResultsMap;
        CompResultsMap m_comparisons;
        bool           m_failed;
    };


    /// @brief This class is responsible for containing test output and 
    ///        performance measurements for OpenCL program 
    // This class is used for storing Run results both for 
    /// ocl_backend and OpenCL Reference
    class RunResult : public IRunResult
    {
    public:
        /// @brief Constructor
        RunResult();

        /// @brief Destructor
        virtual ~RunResult(void);

        /// @brief Returns test execution output
        /// @return Test output
        virtual IBufferContainerList& GetOutput(const char * name);
        virtual const IBufferContainerList& GetOutput(const char * name) const;

        /// @brief Returns test execution NEAT output
        /// @return Test NEAT output
        virtual IBufferContainerList& GetNEATOutput(const char* name);
        virtual const IBufferContainerList& GetNEATOutput(const char* name) const;

        /// @brief Returns pointer to the list of boolean values that signal comparator if it can omit corresponding kernel argument.
        /// @param name Name of the kernel.
        virtual const std::vector<bool>* GetComparatorIgnoreList(const char* name);

        /// @brief Sets ignore list for the kernel with the provided name.
        /// @param name Name of the kernel.
        /// @param ignoreList The list of boolean values that signal comparator if it can omit corresponding kernel argument.
        virtual void SetComparatorIgnoreList(const char* name, const std::vector<bool>& ignoreList);

        /// @brief Returns test execution performance measurements
        /// @return Test performance measurements
        virtual IPerformance& GetPerformance();

        /// @brief Returns number of output buffers
        virtual size_t GetOutputsCount() const;

    private:
        /// hide copy constructor
        RunResult(const RunResult& );
        /// hide assignment operator
        void operator =(RunResult&);

        typedef std::map<std::string, BufferContainerList*> OutputsMap;
        /// @brief Output buffers
        OutputsMap m_refOutputs;
        OutputsMap m_neatOutputs;
        std::map<std::string, std::vector<bool> > m_comparatorIgnoreList;

        /// @brief Output performance
        Performance m_performance;
    };
}

#endif // RUN_RESULTS_H
