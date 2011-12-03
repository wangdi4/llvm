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

File Name:  CompileService.cpp

\*****************************************************************************/

#include "exceptions.h"
#include "CompileService.h"
#include "ProgramBuilder.h"
#include "Program.h"
#include "BitCodeContainer.h"
#include "plugin_manager.h"
#include "llvm/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "BitCodeContainer.h"

#include "llvm/Target/TargetRegistry.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetData.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/Support/MemoryObject.h"
#include "llvm/MC/MCDisassembler.h"
#include "llvm/MC/MCInstPrinter.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/System/Path.h"
#include "llvm/PassManager.h"

#include <sstream>


namespace Intel { namespace OpenCL { namespace DeviceBackend {

CompileService::CompileService()
{}

cl_dev_err_code CompileService::CreateProgram( const cl_prog_container_header* pByteCodeContainer, 
                                               ICLDevBackendProgram_** ppProgram)
{
    try
    {
        std::auto_ptr<Program> spProgram(m_backendFactory->CreateProgram());
        BitCodeContainer* bitCodeContainer = new BitCodeContainer(pByteCodeContainer);
        spProgram->SetBitCodeContainer(bitCodeContainer);
        *ppProgram = spProgram.release();
#ifdef OCL_DEV_BACKEND_PLUGINS  
        // Notify the plugin manager
        PluginManager::Instance().OnCreateProgram( pByteCodeContainer, 
                                                   *ppProgram);
#endif
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
    
cl_dev_err_code CompileService::BuildProgram( ICLDevBackendProgram_* pProgram, 
                                              const ICLDevBackendOptions* pOptions )
{
    try
    {
        //TODO: build the CompilerBuildOptions from the supplied pOptions
        return GetProgramBuilder()->BuildProgram(static_cast<Program*>(pProgram), NULL);
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

cl_dev_err_code CompileService::DumpCodeContainer( const ICLDevBackendCodeContainer* pCodeContainer,
                                                   const ICLDevBackendOptions* pOptions ) const
{
    try
    {
        assert(pCodeContainer);
        assert(pOptions);

        const BitCodeContainer* pContainer = static_cast<const BitCodeContainer*>(pCodeContainer);
        llvm::Module* pModule = (llvm::Module*)pContainer->GetModule();
        assert(pModule);

        std::string fname = pOptions->GetStringValue( CL_DEV_BACKEND_OPTION_DUMPFILE, "");
        
        if( fname.empty() )
        {
            llvm::outs() << *pModule;
        }
        else
        {
            std::string err;
            llvm::raw_fd_ostream ostr(fname.c_str(), err);
            if(err.empty())
            {
                ((llvm::Module*)pContainer->GetModule())->print(ostr, 0);
            }
            else
            {
                throw Exceptions::DeviceBackendExceptionBase(std::string("Can't open the dump file ") + fname + ":" + err);
            }
        }
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


void CompileService::Release()
{
    delete this;
}

//prints the JIT file in assembly x86
void CompileService::DumpJITCodeContainer( const ICLDevBackendCodeContainer* pCodeContainer,
                    const std::string dumpJIT,
                    const std::string baseDirectory) const
{
    const BitCodeContainer* pContainer = static_cast<const BitCodeContainer*>(pCodeContainer);
    llvm::Module* pModule = (llvm::Module*)pContainer->GetModule();
    llvm::Triple triple(pModule->getTargetTriple());
    std::string err;
    const llvm::Target *target = llvm::TargetRegistry::lookupTarget(triple.getTriple(), err);
    std::string FeaturesStr;
    TargetMachine* TM = target->createTargetMachine(triple.getTriple(), FeaturesStr);
    
    // Build up all of the passes that we want to do to the module.
    PassManager PM;

    // Create the output file.
    std::string fileName;
    llvm::sys::Path filePath(dumpJIT.c_str(), dumpJIT.size());

    if( !filePath.isAbsolute() && !baseDirectory.empty())
    {
        llvm::sys::Path absFilePath(baseDirectory.c_str(), baseDirectory.size());
        absFilePath.appendComponent(filePath.c_str());
        fileName = absFilePath.str();
    }
    else
    {
        filePath.makeAbsolute();
        fileName = filePath.str();
    }
    std::string errorInfo;
    llvm::raw_fd_ostream out(fileName.c_str(), errorInfo,
                llvm::raw_fd_ostream::F_Binary);
    if (!errorInfo.empty()) { return; }
    llvm::formatted_raw_ostream FOS(out);

    TM->addPassesToEmitFile(PM, FOS, TargetMachine::CGFT_AssemblyFile, CodeGenOpt::Default);
    PM.run(*pModule);
}

}}}