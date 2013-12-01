/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  CPUCompileService.cpp

\*****************************************************************************/

#include "exceptions.h"
#include "CPUCompileService.h"
#include "ProgramBuilder.h"
#include "Program.h"
#include "BitCodeContainer.h"
#include "plugin_manager.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "BitCodeContainer.h"
#include "CPUDeviceBackendFactory.h"

#include "llvm/Target/TargetMachine.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/MemoryObject.h"
#include "llvm/MC/MCDisassembler.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/PassManager.h"


#include <sstream>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

CPUCompileService::CPUCompileService(const ICompilerConfig& config)
    :m_programBuilder(CPUDeviceBackendFactory::GetInstance(), config)
{
     m_backendFactory = CPUDeviceBackendFactory::GetInstance();
}

cl_dev_err_code CPUCompileService::DumpJITCodeContainer( const ICLDevBackendCodeContainer* pCodeContainer,
                                              const std::string& filename) const
{
    try
    {
        const BitCodeContainer* pContainer = static_cast<const BitCodeContainer*>(pCodeContainer);
        llvm::Module*           pModule    = reinterpret_cast<llvm::Module*>(pContainer->GetModule());
        const CPUCompiler*      pCompiler  = static_cast<const CPUCompiler*>(m_programBuilder.GetCompiler());

        pCompiler->DumpJIT( pModule, filename);
        return CL_DEV_SUCCESS;
    }
    catch( Exceptions::DeviceBackendExceptionBase& e )
    {
        return e.GetErrorCode();
    }
    catch( std::bad_alloc& )
    {
        return CL_DEV_OUT_OF_MEMORY; 
    }
}

}}}
