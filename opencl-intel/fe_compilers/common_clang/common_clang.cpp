// Copyright (c) 2006-2009 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

//
// common_clang.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "common_clang.h"
#include "pch_mgr.h"
#include "resource.h"
#include "exceptions.h"
#include "binary_result.h"
#include "options.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/Threading.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Mutex.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Metadata.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Linker.h"
#include "clang/Basic/LangOptions.h"
#include "clang/Basic/Diagnostic.h"
#include "clang/Basic/DiagnosticIDs.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/FrontendTool/Utils.h"
#include "clang/Driver/ArgList.h"
#include "clang/Driver/Arg.h"
#include "clang/Driver/DriverDiagnostic.h"
#include "CL/cl.h"
#include "assert.h"
#include <list>
#include <iosfwd>
#include <sstream>
#include <iterator>
#include <algorithm>
#ifdef _WIN32
#include <ctype.h>
#endif

using namespace Intel::OpenCL::ClangFE;

static volatile bool lazyCCInit = true; // the flag must be 'volatile' to prevent caching in a CPU register
static llvm::sys::Mutex lazyCCInitMutex;

// This function mustn't be invoked from a static object constructor,
// from a DllMain function (Windows specific), or from a function
// w\ __attribute__ ((constructor)) (Linux specific).
void CommonClangInitialize()
{
    if(lazyCCInit) {
      llvm::sys::ScopedLock lock(lazyCCInitMutex);

        if(lazyCCInit) {
            llvm::llvm_start_multithreaded();
            llvm::InitializeAllTargets();
            llvm::InitializeAllAsmPrinters();
            llvm::InitializeAllAsmParsers();
            llvm::InitializeAllTargetMCs();
            lazyCCInit = false;
        }
    }
}

void CommonClangTerminate()
{
    llvm::llvm_shutdown();
}

void LLVMErrorHandler(void *pUserData, const std::string &message, bool gen_crash_diag)
{
#ifndef _WIN32
    clang::DiagnosticsEngine &de = *static_cast<clang::DiagnosticsEngine*>(pUserData);
    de.Report(clang::diag::note_drv_command_failed_diag_msg) << message;
#endif
    // We cannot recover from llvm errors.
    exit(1);
}

class MemoryBufferCache
{
public:
    ~MemoryBufferCache()
    {
        std::vector<llvm::MemoryBuffer*>::iterator i = m_cache.begin();
        std::vector<llvm::MemoryBuffer*>::iterator e = m_cache.end();

        for(; i != e; ++i)
            delete *i;
    }

    llvm::MemoryBuffer* getMemBuffer(llvm::StringRef InputData,
                                     llvm::StringRef BufferName = "",
                                     bool RequiresNullTerminator = true)
    {
        llvm::MemoryBuffer *pBuffer = llvm::MemoryBuffer::getMemBuffer(InputData, BufferName, RequiresNullTerminator);
        m_cache.push_back(pBuffer);
        return pBuffer;
    }

private:
    std::vector<llvm::MemoryBuffer*> m_cache;
};


static void GetCpuHeaders(
                           std::vector< ResourceProp >& vPCHBuffers,
                           std::vector< ResourceProp >& vHeaderWithDefs,
                           const char*               pszOptions)
{
    bool clStd20 = std::string(pszOptions).find("-cl-std=CL2.0") != std::string::npos;

    unsigned int uiNumPCHBuffers      = 3;
    unsigned int uiNumHeadersWithDefs = 1;

    vPCHBuffers.resize(uiNumPCHBuffers);
    vHeaderWithDefs.resize(uiNumHeadersWithDefs);

    const char* OCLVer = clStd20 ? IDR_CPU_SPEC_20 : IDR_CPU_SPEC_12;
    const char* resource_library = "libcommon_clang.so"; // Library name use Linux and Android only.

    // Get pointer to PCHs.
    vPCHBuffers[0].m_data = ResourceManager::instance().get_resource(IDR_COMMON          , "PCH", false, vPCHBuffers[0].m_size, resource_library);
    vPCHBuffers[1].m_data = ResourceManager::instance().get_resource(IDR_CPU             , "PCH", false, vPCHBuffers[1].m_size, resource_library);
    vPCHBuffers[2].m_data = ResourceManager::instance().get_resource(OCLVer              , "PCH", false, vPCHBuffers[2].m_size, resource_library);
    //Get pointer to header with OpenCL defines.
    vHeaderWithDefs[0].m_data = ResourceManager::instance().get_resource(IDR_HEADER_WITH_DEFS_CPU    , "PCH", true, vHeaderWithDefs[0].m_size, resource_library);
    // Name headers and PCHs.

    vPCHBuffers[0].m_name  = "opencl_common.h.pch";
    vPCHBuffers[1].m_name  = "cpu_opencl_.h.pch";
    vPCHBuffers[2].m_name  = "PCH_CPU.pch";
    vHeaderWithDefs[0].m_name = "header_with_defs_cpu.h";
    vHeaderWithDefs[0].m_size -= 1; //String length without null terminator.
}

#ifdef _WIN32
class SE_Exception
{
private:
  unsigned int nSE;
public:
  SE_Exception( unsigned int n ) : nSE( n )
  {}
  unsigned int getSeNumber()
  {
      return nSE;
  }
};

void trans_func( unsigned int u, EXCEPTION_POINTERS* pExp )
{
    throw SE_Exception(u);
}
#endif

extern "C" CC_DLL_EXPORT int Compile(const char*   pszProgramSource,
                                     const char**  pInputHeaders,
                                     unsigned int  uiNumInputHeaders,
                                     const char**  pInputHeadersNames,
                                     const char*   pPCHBuffer,
                                     size_t        uiPCHBufferSize,
                                     const char*   pszOptions,
                                     const char*   pszOptionsEx,
                                     const char*   pszDeviceSupportedExtensions,
                                     const char*   pszOpenCLVer,
                                     IOCLFEBinaryResult** pBinaryResult)
{
    // Lazy initialization
    CommonClangInitialize();
#ifdef _WIN32
    _set_se_translator( trans_func );
#endif
    try
    {
        // create a new scope to make sure the mutex will be released last

        MemoryBufferCache bufferCache;
        llvm::OwningPtr<OCLFEBinaryResult> pResult ( new OCLFEBinaryResult() );

        // Parse options
        CompileOptionsParser optionsParser(pszDeviceSupportedExtensions, pszOpenCLVer);
        optionsParser.processOptions(pszOptions, pszOptionsEx);

        // Prepare output buffer
        llvm::raw_svector_ostream ir_ostream(pResult->getIRBufferRef());

        // Prepare error log
        llvm::raw_string_ostream err_ostream( pResult->getLogRef());

        // Prepare our diagnostic client.
        llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> DiagID(new clang::DiagnosticIDs());
        llvm::IntrusiveRefCntPtr<clang::DiagnosticOptions> DiagOpts(new clang::DiagnosticOptions());
        DiagOpts->ShowCarets = 0;
        DiagOpts->ShowPresumedLoc = true;
        clang::TextDiagnosticPrinter *DiagsPrinter = new clang::TextDiagnosticPrinter(err_ostream, DiagOpts.getPtr());
        llvm::IntrusiveRefCntPtr<clang::DiagnosticsEngine> Diags(new clang::DiagnosticsEngine(DiagID, &*DiagOpts, DiagsPrinter));

        // Create the clang compiler
        llvm::OwningPtr<clang::CompilerInstance> compiler(new clang::CompilerInstance());

        // Set buffers
        // CompilerInstance takes ownership over input buffer
        compiler->SetInputBuffer(llvm::MemoryBuffer::getMemBuffer( llvm::StringRef(pszProgramSource), optionsParser.getSourceName()));
        compiler->SetOutputStream(&ir_ostream);
        compiler->setDiagnostics(Diags.getPtr());

        // Create compiler invocation from user args before trickering with it
        clang::CompilerInvocation::CreateFromArgs( compiler->getInvocation(),
                                                   optionsParser.beginArgs(),
                                                   optionsParser.endArgs(),
                                                   *Diags);
        // Pre processor options
        clang::PreprocessorOptions &ppOptions = compiler->getPreprocessorOpts();
        optionsParser.getSupportedPragmas(ppOptions.SupportedPragmas);
        // We'll manage the remapped files buffers
        ppOptions.RetainRemappedFileBuffers = true;

        std::vector< ResourceProp > vPCHBuffers;
        std::vector< ResourceProp > vHeaderWithDefs;

        if(0 != pPCHBuffer)
        {
            vPCHBuffers.push_back( ResourceProp( pPCHBuffer, uiPCHBufferSize, std::string("PCHHeader.pch")));
            ppOptions.ImplicitPCHInclude = vPCHBuffers[0].m_name;
        }
        else
        {
            for(const char* const* I = optionsParser.beginArgs(); I != optionsParser.endArgs(); I++)
            {
                if( !strcmp( "PCH_CPU.pch", *I) )
                {
                    GetCpuHeaders(vPCHBuffers, vHeaderWithDefs, pszOptions);
                    break;
                }
            }
        }
        // Set an error handler, so that any LLVM backend diagnostics go through our error handler.
        // (currently commented out since setting the llvm error handling in multi-threaded environment is unsupported)
        //llvm::remove_fatal_error_handler();
        //llvm::install_fatal_error_handler(LLVMErrorHandler, static_cast<void*>(&compiler->getDiagnostics()));

        // Input Headers
        for (unsigned int i = 0; i < uiNumInputHeaders; ++i)
        {
            llvm::MemoryBuffer *header = bufferCache.getMemBuffer(pInputHeaders[i], pInputHeadersNames[i]);
            compiler->AddInMemoryHeader(header, pInputHeadersNames[i]);
        }

        // Input header with OpenCL defines.
        for(unsigned int i = 0; i < vHeaderWithDefs.size(); ++i)
        {
            llvm::MemoryBuffer *header = bufferCache.getMemBuffer(llvm::StringRef( vHeaderWithDefs[i].m_data, vHeaderWithDefs[i].m_size), "", true);
            compiler->AddInMemoryHeader(header, vHeaderWithDefs[i].m_name);
        }

        // PCH buffers.
        for(unsigned int i = 0; i < vPCHBuffers.size(); ++i)
        {
            llvm::MemoryBuffer *pchBuffer = bufferCache.getMemBuffer(llvm::StringRef( vPCHBuffers[i].m_data, vPCHBuffers[i].m_size), "", false);
            ppOptions.addRemappedFile( vPCHBuffers[i].m_name, pchBuffer);
        }

        // Execute the frontend actions.
        bool success = false;
        try
        {
            success = clang::ExecuteCompilerInvocation(compiler.get());
        }
        catch (const std::exception&)
        {
            //LOG_ERROR(TEXT("CompileTask::Execute() - caught an exception during compilation"), "");
        }
#ifdef _WIN32
        catch(SE_Exception& seException)
        {
          if(seException.getSeNumber() == EXCEPTION_STACK_OVERFLOW)
          {
            pResult->setLog("Error! Stack overflow during compilation of the program. "
                            "Size of the stack should be increased\n"
                            "Note! Further execution can be unstable. Memory leak is possible.\n");
          }
          pResult->setResult(CL_COMPILE_PROGRAM_FAILURE);
          success = false;
        }
#endif
        pResult->setIRType(IR_TYPE_COMPILED_OBJECT);
        pResult->setIRName(optionsParser.getSourceName());
        // Our error handler depends on the Diagnostics object, which we're
        // potentially about to delete. Uninstall the handler now so that any
        // later errors use the default handling behavior instead.
        // (currently commented out since setting the llvm error handling in multi-threaded environment is unsupported)
        // llvm::remove_fatal_error_handler();
        ir_ostream.flush();
        err_ostream.flush();

        //LOG_INFO(TEXT("%s"), TEXT("Finished"));
        if( pBinaryResult )
        {
            *pBinaryResult = pResult.take();
        }
        return success ? CL_SUCCESS : CL_COMPILE_PROGRAM_FAILURE;
    }
    catch( std::bad_alloc& )
    {
        if( pBinaryResult )
        {
            *pBinaryResult = NULL;
        }
        return CL_OUT_OF_HOST_MEMORY;
    }
    catch(internal_error& a)
    {
        if( pBinaryResult )
        {
            *pBinaryResult = NULL;
        }
        assert( false && " Common_clang. Something wrong with PCHs." );
        return CL_COMPILE_PROGRAM_FAILURE;
    }
#ifdef _WIN32
    catch(SE_Exception&)
    {
      return CL_COMPILE_PROGRAM_FAILURE;
    }
#endif
}
