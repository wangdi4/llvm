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

File Name:  IRunConfiguration.h

\*****************************************************************************/
#ifndef I_RUN_CONFIGURATION_H
#define I_RUN_CONFIGURATION_H

namespace Validation
{
    enum TEST_MODE {VALIDATION, REFERENCE, PERFORMANCE, BUILD};

    /// @brief  This class contains test run configuration for SATest components like comparator, reference runner
    ///         and back-end runner.
    class IRunComponentConfiguration
    {
    public:
        virtual ~IRunComponentConfiguration() {}

        /// @brief Init the configuration from the command line parameters
        virtual void InitFromCommandLine() = 0;
    };

    /// @brief This class contains test run configuration for SATest
    class IRunConfiguration
    {
    public:
        virtual ~IRunConfiguration() {}

        /// @brief Init the configuration from the command line parameters
        virtual void InitFromCommandLine() = 0;

        /// @brief Returns true if reference should be used in validation mode
        virtual bool UseReference() const = 0;

        /// @brief Returns true if reference should unconditionally generated
        virtual bool ForceReference() const = 0;

        /// @brief Returns current test mode.
        virtual TEST_MODE TestMode() const = 0;

        /// @brief Returns pointer to the object with comparator configuration
        virtual const IRunComponentConfiguration* GetComparatorConfiguration() const = 0;

        /// @brief Returns pointer to the object with reference runner configuration
        virtual const IRunComponentConfiguration* GetReferenceRunnerConfiguration() const = 0;

        /// @brief Returns pointer to the object with back-end runner configuration
        virtual const IRunComponentConfiguration* GetBackendRunnerConfiguration() const = 0;
    };

}

#endif // I_RUN_CONFIGURATION_H
