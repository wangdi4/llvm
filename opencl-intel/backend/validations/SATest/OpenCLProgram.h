/*****************************************************************************\

Copyright (c) Intel Corporation (2011-2012).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name: OpenCLProgram.h

\*****************************************************************************/
#ifndef OPENCL_PROGRAM_H
#define OPENCL_PROGRAM_H

#include "IProgram.h"
#include "cl_device_api.h"
#include "cl_dev_backend_api.h"
#include "OpenCLProgramConfiguration.h"
#include "llvm/IR/Module.h"

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

    /// @brief Extracts LLVM program from 'program' and parse it
    ///        and create a LLVM module.
    /// @return module
    llvm::Module* ParseToModule(void) const;

private:
    /// @brief create OpenCL program from BC type
    /// @param [IN] programFile Name of OpenCL test program file
    void BCOpenCLProgram(const std::string& programFile);

    /// @brief sets the pContainer's arguments
    void setContainer();

private:

    /// @brief OpenCL program byte code container
    cl_prog_container_header* pContainer;

    /// @brief Size of OpenCL program container
    unsigned int containerSize;

};

class ProgramHolder
{
public:
    ProgramHolder(const ICLDevBackendCompilationService* pService):
        m_pService(pService),
        m_pProgram(NULL)
    {
        assert(m_pService);
    }

    ProgramHolder(const ICLDevBackendCompilationService* pService, ICLDevBackendProgram_* pProgram ):
        m_pService(pService),
        m_pProgram(pProgram)
    {
        assert(m_pService);
    }

    ~ProgramHolder()
    {
        m_pService->ReleaseProgram(m_pProgram);
    }

    ICLDevBackendProgram_* getProgram() const
    {   
        return m_pProgram;
    }

    void setProgram(ICLDevBackendProgram_* pProgram)
    {   
        // Disabling assertion to allow setting SATest build iterations > 1
        assert( NULL == m_pProgram && "Resetting the program is not supported");
        m_pProgram = pProgram;
    }


private:
    const ICLDevBackendCompilationService* m_pService;
    ICLDevBackendProgram_ * m_pProgram;
};


}




#endif // OPENCL_PROGRAM_H
