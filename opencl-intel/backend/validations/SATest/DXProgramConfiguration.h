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

File Name:  DXProgramConfiguration.h

\*****************************************************************************/
#ifndef DX_PROGRAM_CONFIGURATION_H
#define DX_PROGRAM_CONFIGURATION_H

#include "IProgramConfiguration.h"

namespace Validation
{
    /// @brief This class contains DirectX test run configuration
    class DXProgramConfiguration : public IProgramConfiguration
    {
    public:

        /// @brief Constructor
        /// @param [IN] configFile Name of DirectX test run configuration file
        DXProgramConfiguration(const std::string& configFile);

        /// @brief Destructor
        virtual ~DXProgramConfiguration(void);

        /// @brief Returns the program file path 
        std::string GetProgramFilePath() const
        {
            throw "Not implemented";
        }

        /// @brief Returns the program name
        std::string GetProgramName() const
        {
            throw "Default";
        }
    };

}

#endif // DX_RUN_CONFIGURATION_H
