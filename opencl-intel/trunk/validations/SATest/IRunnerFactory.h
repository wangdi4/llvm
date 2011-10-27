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

File Name:  IRunnerFactory.h

\*****************************************************************************/
#ifndef I_RUNNER_FACTORY_H
#define I_RUNNER_FACTORY_H

#include "IProgram.h"
#include "IProgramRunner.h"
#include "IRunConfiguration.h"
#include "IRunResultComparator.h"
#include "IProgramConfiguration.h"

#include <string>

namespace Validation
{
  /// @brief This class is a factory of program, program runner and reference runner
  class IRunnerFactory
  {
  public:
    /// @brief Creates new program
    virtual IProgram* CreateProgram(IProgramConfiguration* programConfig,
                                    IRunConfiguration* pRunConfiguration) = 0;

    /// @brief Creates new program configuration
    virtual IProgramConfiguration* CreateProgramConfiguration(const std::string& configFile, const std::string& baseDir) = 0;

    /// @brief Creates new program configuration
    virtual IRunConfiguration* CreateRunConfiguration() = 0;

    /// @brief Creates new program runner
    virtual IProgramRunner* CreateProgramRunner() = 0;

    /// @brief Creates new reference runner
    virtual IProgramRunner* CreateReferenceRunner(const IRunComponentConfiguration* pRunConfiguration) = 0;

    /// @brief Creates new run results comparator
    // TODO: Once RunResult object will be re-designed, change IRunConfiguration to IRunComponentConfiguration.
    virtual IRunResultComparator* CreateComparator(IProgramConfiguration* pProgramConfiguration,
                                                   IRunConfiguration* pRunConfiguration) = 0;

  };
}

#endif // I_RUNNER_FACTORY_H