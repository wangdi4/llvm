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

File Name:  DXRunConfiguration.h

\*****************************************************************************/
#ifndef DX_RUN_CONFIGURATION_H
#define DX_RUN_CONFIGURATION_H

#include "IRunConfiguration.h"

namespace Validation
{
    /// @brief This class contains DirectX test run configuration
    class DXRunConfiguration : public IRunConfiguration
    {
    public:

        /// @brief Constructor
        /// @param [IN] configFile Name of DirectX test run configuration file
        DXRunConfiguration();

        /// @brief Init the configuration from the command line parameters
        virtual void InitFromCommandLine();

        /// @brief Returns true if reference should be used in validation mode
        virtual bool UseReference() const;

        /// @brief Returns true if reference should unconditionally generated
        virtual bool ForceReference() const;

        /// @brief Returns current test mode.
        virtual TEST_MODE TestMode() const;

        /// @brief Returns pointer to the object with comparator configuration
        virtual const IRunComponentConfiguration* GetComparatorConfiguration() const;

        /// @brief Returns pointer to the object with reference runner configuration
        virtual const IRunComponentConfiguration* GetReferenceRunnerConfiguration() const;

        /// @brief Returns pointer to the object with back-end runner configuration
        virtual const IRunComponentConfiguration* GetBackendRunnerConfiguration() const;
    };

}

#endif // DX_RUN_CONFIGURATION_H
