/*****************************************************************************\

Copyright (c) Intel Corporation (2010, 2011).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  SATest.cpp

\*****************************************************************************/
#include "Comparator.h"
#include "SATest.h"
#include "SATestException.h"

#include "IProgramRunner.h"
#include "IProgram.h"
#include "IRunResult.h"
#include "IRunnerFactory.h"

#include "IComparisonResults.h"
#include "ComparisonResults.h"

#include <memory>
#include <string>
#include <iostream>
#include <map>
#include <cassert>

using namespace Validation;
using std::string;

SATest::SATest(RunnerFactory::PROGRAM_TYPE programType,
                 const string& configFileName,
                 const string& baseDirectory,
                 IRunConfiguration* pRunConfiguration):
    m_factory(RunnerFactory::GetInstance(programType)),
    m_pProgramConfiguration(NULL),
    m_pProgram(NULL)
{
    m_pProgramConfiguration = m_factory.CreateProgramConfiguration(configFileName, baseDirectory);
    m_pProgram = m_factory.CreateProgram( m_pProgramConfiguration, pRunConfiguration);
}

SATest::~SATest(void)
{
    delete m_pProgram;
    delete m_pProgramConfiguration;
}

void SATest::Run(TEST_MODE mode, IRunConfiguration* pRunConfiguration)
{
    switch( mode)
    {
        case REFERENCE:
            RunReference( pRunConfiguration->GetReferenceRunnerConfiguration() );
            break;
        case VALIDATION:
            RunValidation( pRunConfiguration );
            break;
        case PERFORMANCE:
            RunPerformance( pRunConfiguration->GetBackendRunnerConfiguration() );
            break;
        case BUILD:
            RunBuildOnly( pRunConfiguration->GetBackendRunnerConfiguration() );
            break;
        default:
            throw Exception::TestFailException("Unsupported test mode.");
    }
}

void SATest::RunValidation(IRunConfiguration* pRunConfiguration)
{
    std::auto_ptr<IProgramRunner> spRunner(m_factory.CreateProgramRunner(pRunConfiguration->GetBackendRunnerConfiguration()));
    std::auto_ptr<IRunResultComparator> spComparator(
        m_factory.CreateComparator(m_pProgramConfiguration, pRunConfiguration));

    RunResult runResult;
    spRunner->Run(&runResult, m_pProgram,
        m_pProgramConfiguration, pRunConfiguration->GetBackendRunnerConfiguration());

    if( pRunConfiguration->UseReference() )
    {
        RunResult refResult;
        LoadOrGenerateReference(pRunConfiguration, &refResult);
        std::auto_ptr<IRunResultComparison> spCompResult(
            spComparator->Compare( &runResult, &refResult ));

        if( spCompResult->isFailed() )
        {
            throw Exception::TestFailException("Comparison failed.");
        }
    }
    std::cout << "Test Passed." << endl;
}

void SATest::RunPerformance(const IRunComponentConfiguration* pRunConfiguration)
{
    std::auto_ptr<IProgramRunner> spRunner( m_factory.CreateProgramRunner(pRunConfiguration));

    RunResult runResult;
    spRunner->Run(&runResult, m_pProgram,
        m_pProgramConfiguration, pRunConfiguration);
    ((Performance&)runResult.GetPerformance()).Print(m_pProgramConfiguration->GetProgramName());
}

void SATest::RunReference(const IRunComponentConfiguration* pRunConfiguration)
{
    RunResult runResult;

    std::auto_ptr<IProgramRunner> spRunner( m_factory.CreateReferenceRunner(pRunConfiguration));
    GenerateReference( &runResult, spRunner.get(), pRunConfiguration);
    std::cout << "Reference output generated successfully" << endl;
}

void SATest::RunBuildOnly(const IRunComponentConfiguration* pRunConfiguration)
{
    std::auto_ptr<IProgramRunner> spRunner( m_factory.CreateProgramRunner(pRunConfiguration));

    RunResult runResult;
    spRunner->Run(&runResult, m_pProgram, m_pProgramConfiguration, pRunConfiguration);
}

void SATest::GenerateReference(IRunResult* pResult, IProgramRunner* pRunner, const IRunComponentConfiguration* pRunConfiguration)
{
    // execute reference
    pRunner->Run(pResult, m_pProgram, m_pProgramConfiguration, pRunConfiguration);
    pRunner->StoreOutput(pResult, m_pProgramConfiguration);
}

void SATest::LoadOrGenerateReference(IRunConfiguration* pRunConfiguration, IRunResult* pResult)
{
    std::auto_ptr<IProgramRunner> spRefRunner( m_factory.CreateReferenceRunner(pRunConfiguration->GetReferenceRunnerConfiguration()));

    if(pRunConfiguration->ForceReference())
    {
        GenerateReference(pResult, spRefRunner.get(), pRunConfiguration->GetReferenceRunnerConfiguration());
    }
    else
    {
        try
        {
            spRefRunner->LoadOutput(pResult, m_pProgramConfiguration);
        }
        catch( Exception::IOError& )
        {
            GenerateReference( pResult, spRefRunner.get(), pRunConfiguration->GetReferenceRunnerConfiguration());
        }
    }
}

