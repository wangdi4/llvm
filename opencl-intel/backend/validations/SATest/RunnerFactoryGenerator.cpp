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

File Name:  RunnerFactoryGenerator.cpp

\*****************************************************************************/
#include "RunnerFactoryGenerator.h"
#include "OpenCLFactory.h"
#ifdef MIC_ENABLE
#include "OpenCLMICFactory.h"
#endif
#include "DXFactory.h"

using namespace Validation;

IRunnerFactory* RunnerFactory::s_dxFactory = NULL;
IRunnerFactory* RunnerFactory::s_oclFactory = NULL;

RunnerFactory::RunnerFactory()
{
}

IRunnerFactory& RunnerFactory::GetInstance(PROGRAM_TYPE programType)
{
    IRunnerFactory* retNullFactory = NULL;
    // Create appropriate runner factory based on program type
    if (programType == OPEN_CL) 
    {
        if( NULL == s_oclFactory)
        {
#ifdef MIC_ENABLE
            s_oclFactory = new OpenCLMICFactory();
#else
            s_oclFactory = new OpenCLFactory();
#endif
        }
        return *s_oclFactory;
    } else if (programType == DIRECT_X) 
    {
        if( NULL == s_dxFactory)
        {
            s_dxFactory = new DXFactory();
        }
        return *s_dxFactory;
    } else {
        throw Exception::InvalidArgument("Unrecognized program type");
    }
    return *retNullFactory;
}
