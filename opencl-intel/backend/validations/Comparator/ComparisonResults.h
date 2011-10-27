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

File Name:  ComparisonResults.h

\*****************************************************************************/
#ifndef COMPARISON_RESULTS_H
#define COMPARISON_RESULTS_H

#include "IComparisonResults.h"
#include <vector>
#include <string>
#include <map>

namespace Validation
{
    /// Comparison data for one buffer
    struct CompStatistics
    {
        IMemoryObjectDescPtr pDesc;
        uint64_t numMismatches;
        long double maxDiff;
        float maxInterval;

        CompStatistics() : pDesc(NULL), numMismatches(0), maxDiff(0.0), maxInterval(0.0)
        {}

        CompStatistics(const IMemoryObjectDesc * in_pDesc) :
        pDesc(in_pDesc), numMismatches(0), maxDiff(0.0), maxInterval(0.0)
        {}
    };


    /// @brief  Simple implementation of IComparisonResults interface. Stores mismatches information in the vector of values.
    class ComparisonResults : public IComparisonResults
    {
    public:

        static const uint32_t MAX_MISMATCHES = 100;

        ComparisonResults(const std::string& kerName, bool statDetail):m_kernelName(kerName) {}

        ComparisonResults() {}

        virtual ~ComparisonResults();

        /// @brief Adds mismatch information
        /// @param  [in]  in_Val            Mismatch value information        template<typename T>
        void AddMismatch(const MismatchedVal& in_Val);

        /// @brief Gets mismatch information by given mismatch index
        /// @param [in] index   Index of mismatch value in mismatch container
        MismatchedVal GetMismatch(size_t index);

        /// @brief Number of mismatched values in mismatch container
        size_t GetMismatchCount();

        void Report();

        void ReportDetail();

        void Clear();

        /// @brief returns pointer to object that contains statistics data
        StatisticsCollector* GetStatistics()
        {
            return &mStatistics;
        }

    private:
        std::string m_kernelName;
        /// @brief  Mismatched values container
        std::vector<MismatchedVal> mismatches;
        /// @brief  Number of mismatched values
        typedef std::map<uint32_t, CompStatistics> CompStatMap;
        CompStatMap         m_statMap;
        /// @brief  Object for gathering statistical information
        StatisticsCollector mStatistics;
    };
} // namespace Validation

#endif // COMPARISON_RESULTS_H
