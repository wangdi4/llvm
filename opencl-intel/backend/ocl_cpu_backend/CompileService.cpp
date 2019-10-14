// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

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

        if(nullptr == pBinary || uiBinarySize == 0 || nullptr == ppProgram)
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
    std::lock_guard<llvm::sys::Mutex> lock(m_buildLock);
#ifdef OCL_DEV_BACKEND_PLUGINS
    m_pluginManager.OnReleaseProgram(pProgram);
#endif
    delete pProgram;
}

cl_dev_err_code CompileService::BuildProgram( ICLDevBackendProgram_* pProgram,
                                              const ICLDevBackendOptions* pOptions,
                                              const char* pBuildOpts)
{
    if(nullptr == pProgram)
    {
        return CL_DEV_INVALID_VALUE;
    }

    // if an exception is caught, this mutex should be unlocked safely
    // on windows, this mutex will remain unlocked if it is locked in the try{} code block
    std::lock_guard<llvm::sys::Mutex> lock(m_buildLock);

    try
    {
        return GetProgramBuilder()->BuildProgram(static_cast<Program*>(pProgram), pOptions, pBuildOpts);
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
            llvm::raw_fd_ostream ostr(fname.c_str(), ec,
                                      llvm::sys::fs::FA_Write);
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
