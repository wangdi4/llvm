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

File Name:  DXProgram.h

\*****************************************************************************/
#ifndef DX_PROGRAM_H
#define DX_PROGRAM_H

#include "IProgram.h"
#include <string>

namespace Validation
{
  /// @brief This class contains DirectX test program information
  class DXProgram : public IProgram
  {
  public:

    /// @brief Constructor
    /// @param [IN] programFile Name of DirectX test program file
    DXProgram(const std::string& programFile);

    /// @brief Destructor
    virtual ~DXProgram(void);
  };

}

#endif // DX_PROGRAM_H
