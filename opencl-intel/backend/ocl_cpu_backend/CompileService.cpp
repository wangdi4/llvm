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
#include "Program.h"
#include "BitCodeContainer.h"
#include "ObjectCodeContainer.h"
#include "elf_binary.h"
#include "cache_binary_handler.h"

#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/MutexGuard.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCDisassembler/MCDisassembler.h"
#include "llvm/MC/MCInstPrinter.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {
using namespace Intel::OpenCL::ELFUtils;

CompileService::CompileService()
{}

cl_dev_err_code CompileService::CreateProgram( const void* pBinary,
                                               size_t uiBinarySize,
                                               ICLDevBackendProgram_** ppProgram)
{
    assert(m_backendFactory);

    try
    {
        const char* pBinaryData = (const char*)pBinary;
        size_t uiBinaryDataSize = uiBinarySize;

        if(NULL == pBinary || uiBinarySize == 0 || NULL == ppProgram)
        {
            return CL_DEV_INVALID_VALUE;
        }

        std::auto_ptr<Program> spProgram(m_backendFactory->CreateProgram());

        //check if it is Binary object
        if( OCLElfBinaryReader::IsValidOpenCLBinary((const char*)pBinary, uiBinarySize))
        {
            OCLElfBinaryReader reader((const char*)pBinary,uiBinarySize);
            reader.GetIR(const_cast<char*&>(pBinaryData), uiBinaryDataSize);
            spProgram->SetBitCodeContainer(new BitCodeContainer(pBinaryData, uiBinaryDataSize, "main"));
            GetProgramBuilder()->ParseProgram(spProgram.get());
        }
        else if(CacheBinaryReader::IsValidCacheObject((const char*)pBinary, uiBinarySize))
        {
            spProgram->SetObjectCodeContainer(new ObjectCodeContainer(pBinaryData, uiBinaryDataSize));
        }
        //check if it is LLVM IR object
        else if ( !memcmp(_CL_LLVM_BITCODE_MASK_, pBinary, sizeof(_CL_LLVM_BITCODE_MASK_) - 1) )
        {
            spProgram->SetBitCodeContainer(new BitCodeContainer(pBinaryData, uiBinaryDataSize, "main"));
            GetProgramBuilder()->ParseProgram(spProgram.get());
        }
        else
        {
            throw Exceptions::DeviceBackendExceptionBase("Unknown binary type", CL_DEV_INVALID_BINARY);
        }
#ifdef OCL_DEV_BACKEND_PLUGINS
        // Notify the plugin manager
        m_pluginManager.OnCreateProgram(pBinaryData, uiBinaryDataSize, spProgram.get());
#endif
        *ppProgram = spProgram.release();
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

void CompileService::ReleaseProgram(ICLDevBackendProgram_* pProgram) const
{
    llvm::MutexGuard lock(m_buildLock);
#ifdef OCL_DEV_BACKEND_PLUGINS
    m_pluginManager.OnReleaseProgram(pProgram);
#endif
    delete pProgram;
}

cl_dev_err_code CompileService::BuildProgram( ICLDevBackendProgram_* pProgram,
                                              const ICLDevBackendOptions* pOptions )
{
    try
    {
        if(NULL == pProgram)
        {
            return CL_DEV_INVALID_VALUE;
        }

        llvm::MutexGuard lock(m_buildLock);

        return GetProgramBuilder()->BuildProgram(static_cast<Program*>(pProgram), pOptions);
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
    assert(pCodeContainer);
    assert(pOptions);

    try
    {
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
            std::error_code ec;
            llvm::raw_fd_ostream ostr(fname.c_str(), ec, llvm::sys::fs::F_RW);
            if(!ec)
            {
                ((llvm::Module*)pContainer->GetModule())->print(ostr, 0);
            }
            else
            {
                throw Exceptions::DeviceBackendExceptionBase(std::string("Can't open the dump file ") + fname + ":" + ec.message());
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
cl_dev_err_code CompileService::DumpJITCodeContainer( const ICLDevBackendCodeContainer* pCodeContainer,
                                           const std::string& filename) const
{
    assert(false);
    return CL_DEV_NOT_SUPPORTED;
}
}}}
