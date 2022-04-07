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

#include "RunResult.h"
#include "Exception.h"

#define DEBUG_TYPE "SATest"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

using namespace Validation;

RunResultComparison::~RunResultComparison()
{
    for( CompResultsMap::iterator it = m_comparisons.begin(); it != m_comparisons.end(); ++it)
    {
        delete it->second;
    }
}

IComparisonResults* RunResultComparison::GetComparison( const char* name )
{
    if( m_comparisons.find( name ) == m_comparisons.end() )
    {
        throw Exception::InvalidArgument("name not found");
    }
    return m_comparisons[ std::string(name) ];
}

void RunResultComparison::AddComparison( const char* name, IComparisonResults* comparison)
{
    m_comparisons[std::string(name)] = comparison;
}

////////////////////////////////////////////////

RunResult::RunResult(void)
{
}

RunResult::~RunResult(void)
{
}

const IBufferContainerList& RunResult::GetOutput(const char * kernelName) const
{
    if( m_refOutputs.find( kernelName ) == m_refOutputs.end() )
    {
        throw Exception::InvalidArgument(std::string("There are no run results for the kernel: ") + kernelName);
    }
    return *m_refOutputs.find(kernelName)->second;
}

IBufferContainerList& RunResult::GetOutput(const char * kernelName)
{
    if( m_refOutputs.find( kernelName ) == m_refOutputs.end() )
    {
        m_refOutputs[kernelName] = new BufferContainerList();
    }
    return *m_refOutputs[kernelName];
}

const IBufferContainerList& RunResult::GetNEATOutput(const char * kernelName) const
{
    if( m_neatOutputs.find( kernelName ) == m_neatOutputs.end() )
    {
        throw Exception::InvalidArgument(std::string("There are no run NEAT results for the kernel: ") + kernelName);
    }
    return *m_neatOutputs.find(kernelName)->second;
}

IBufferContainerList& RunResult::GetNEATOutput(const char * kernelName)
{
    if( m_neatOutputs.find( kernelName ) == m_neatOutputs.end() )
    {
        m_neatOutputs[kernelName] = new BufferContainerList();
    }
    return *m_neatOutputs[kernelName];
}

const std::vector<bool>* RunResult::GetComparatorIgnoreList(const char * kernelName)
{
    if( m_comparatorIgnoreList.find( kernelName ) == m_comparatorIgnoreList.end() )
    {
        return 0;
    }
    return &(m_comparatorIgnoreList[kernelName]);
}

size_t  RunResult::GetOutputsCount() const
{
    return m_refOutputs.size();
}

IPerformance& RunResult::GetPerformance()
{
    return m_performance;
}

void RunResult::SetComparatorIgnoreList( const char* kernelName, const std::vector<bool>& ignoreList )
{
    LLVM_DEBUG({
    if( m_comparatorIgnoreList.find( kernelName ) != m_comparatorIgnoreList.end() )
    {
        llvm::dbgs() << "Ignore list for " << kernelName << " was reset!";
    }
    });
    m_comparatorIgnoreList[kernelName] = ignoreList;
}
