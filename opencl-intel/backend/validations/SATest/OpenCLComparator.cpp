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

File Name:  OpenCLComparator.cpp

\*****************************************************************************/

#include <memory>
#include "OpenCLComparator.h"
#include "OpenCLProgramConfiguration.h"
#include "OpenCLRunConfiguration.h"
#include "RunResult.h"
#include "Comparator.h"
#include "ComparisonResults.h"


using namespace Validation;

OpenCLComparator::OpenCLComparator(const OpenCLProgramConfiguration* pProgramConfig,
                                   const ComparatorRunOptions* pComparatorConfig,
                                   const ReferenceRunOptions* pRefConfig):
    m_useNeat(pRefConfig->GetValue<bool>(RC_REF_USE_NEAT, false)),
    m_ULPTolerance(pComparatorConfig->GetValue<double>(RC_COMP_ULP_TOLERANCE, 0.0)),
    m_detailedStat(pComparatorConfig->GetValue<bool>(RC_COMP_DETAILED_STAT, false))
{
    for(OpenCLProgramConfiguration::KernelConfigList::const_iterator it = pProgramConfig->beginKernels();
        it != pProgramConfig->endKernels();
        ++it )
    {
        m_kernels.push_back( (*it)->GetKernelName() );
    }
}

/// @brief Compares the result of two runs
IRunResultComparison* OpenCLComparator::Compare( IRunResult* volcanoRunResults, IRunResult* referenceRunResults ) const
{
    std::auto_ptr<RunResultComparison> spResult(new RunResultComparison());

    if( volcanoRunResults->GetOutputsCount() != referenceRunResults->GetOutputsCount() )
    {
        spResult->SetIsFailedStatus(true);
    }

    Comparator comparator;
    comparator.SetULPTolerance(m_ULPTolerance);

    for( std::vector<std::string>::const_iterator i = m_kernels.begin(), e = m_kernels.end(); i != e; ++i )
    {
        IBufferContainerList& volcanoOutput = volcanoRunResults->GetOutput( i->c_str() );
        IBufferContainerList& referenceOutput = referenceRunResults->GetOutput( i->c_str() );
        IBufferContainerList* neatOutput = m_useNeat? &referenceRunResults->GetNEATOutput(i->c_str() ) : NULL;

        // run comparison for OpenCL kernel
        std::auto_ptr<ComparisonResults> spres(new ComparisonResults(*i, m_detailedStat));
        COMP_RESULT compRes = comparator.Compare(*spres,
                                                 volcanoRunResults->GetComparatorIgnoreList(i->c_str()),
                                                 volcanoOutput,
                                                 &referenceOutput,
                                                 neatOutput);
        spres->Report();
        if(m_detailedStat)
        {
            spres->ReportDetail();
        }
        if (compRes == NOT_PASSED)
        {
            spResult->SetIsFailedStatus(true);
        }

        spResult->AddComparison(i->c_str(), spres.release());
    }
    return spResult.release();


}
