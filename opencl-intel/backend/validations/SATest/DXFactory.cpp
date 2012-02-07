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

File Name:  DXFactory.cpp

\*****************************************************************************/
#include "DXFactory.h"

#include "DXProgram.h"
#include "DXProgramRunner.h"
#include "DXreferenceRunner.h"
#include "DXRunConfiguration.h"
#include "DXProgramConfiguration.h"

using std::string;
using namespace Validation;

DXFactory::DXFactory(void)
{

}

DXFactory::~DXFactory(void)
{

}

IProgram * DXFactory::CreateProgram(IProgramConfiguration* programConfig,
                                    IRunConfiguration* pRunConfiguration)
{
  return new DXProgram(programConfig->GetProgramFilePath());
}

IProgramConfiguration * DXFactory::CreateProgramConfiguration(const string& configFile, const string& )
{
  return new DXProgramConfiguration(configFile);
}

IRunConfiguration * DXFactory::CreateRunConfiguration()
{
  return new DXRunConfiguration();
}

IProgramRunner * DXFactory::CreateProgramRunner(const IRunComponentConfiguration* pRunConfiguration)
{
  return new DXProgramRunner();
}

IProgramRunner * DXFactory::CreateReferenceRunner(const IRunComponentConfiguration* pRunConfiguration)
{
  return new DXReferenceRunner();
}

IRunResultComparator* DXFactory::CreateComparator(IProgramConfiguration* pProgramConfiguration,
                                                  IRunConfiguration* pRunConfiguration)
{
    throw Exception::NotImplemented(
        "DXFactory::CreateComparator is not implemented");
}
