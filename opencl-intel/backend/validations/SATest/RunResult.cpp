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

File Name:  RunResult.cpp

\*****************************************************************************/
#include "Comparator.h"
#include "RunResult.h"
#include "IComparisonResults.h"
#include "ComparisonResults.h"
#include "llvm/Support/CommandLine.h"
#include "OpenCLProgramRunner.h"

// Command line options
#include "llvm/Support/CommandLine.h"

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

IBufferContainerList& RunResult::GetOutput(const char * kernelName)
{
    if( m_refOutputs.find( kernelName ) == m_refOutputs.end() )
    {
        m_refOutputs[kernelName] = new BufferContainerList();
    }
    return *m_refOutputs[kernelName];
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
    DEBUG({
    if( m_comparatorIgnoreList.find( kernelName ) != m_comparatorIgnoreList.end() )
    {
        llvm::dbgs() << "Ignore list for " << kernelName << " was reset!";
    }
    });
    m_comparatorIgnoreList[kernelName] = ignoreList;
}
