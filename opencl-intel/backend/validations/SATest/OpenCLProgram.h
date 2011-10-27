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

File Name:  OpenCLProgram.h

\*****************************************************************************/
#ifndef OPENCL_PROGRAM_H
#define OPENCL_PROGRAM_H

#include "IProgram.h"
#include "cl_device_api.h"
#include "cl_dev_backend_api.h"
#include "OpenCLProgramConfiguration.h"

#include <string>

namespace Validation
{
  /// @brief This class contains OpenCL test program information
  class OpenCLProgram : public IProgram
  {
  public:

    /// @brief Constructor
    /// @param [IN] Configuration file of the OpenCL test file
    /// @param [IN] Cpu architecture of the OpenCL program
    OpenCLProgram(OpenCLProgramConfiguration * oclProgramConfig,
        const std::string cpuArch);

    /// @brief destructor
    virtual ~OpenCLProgram(void);

    /// @brief Returns program container
    /// @return Program container of this OpenCL program
    cl_prog_container_header* GetProgramContainer() const;

    /// @brief Returns size of program container
    /// @return Size of program container of this OpenCL program
    unsigned int GetProgramContainerSize() const;

    /// @brief Returns back-end program interface from the latest build
    ICLDevBackendProgram_* GetIProgram() const { return m_pProgram; }

    /// @brief Sets the program interface from the latest build
    void SetIProgram( ICLDevBackendProgram_* pProgram) { m_pProgram = pProgram; }

  private:

    /// @brief OpenCL program byte code container
    cl_prog_container_header* pContainer;

    /// @brief Size of OpenCL program container
    unsigned int containerSize;

    /// Compiled program pointer
    ICLDevBackendProgram_* m_pProgram;

    /// @brief create OpenCL program from BC type
    /// @param [IN] programFile Name of OpenCL test program file
    void BCOpenCLProgram(const std::string& programFile);

    /// @brief sets the pContainer's arguments
    void setContainer();
  };
}

#endif // OPENCL_PROGRAM_H
