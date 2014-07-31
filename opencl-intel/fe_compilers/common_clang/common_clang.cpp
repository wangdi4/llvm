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
#include <list>
#include <iosfwd>
#include <sstream>
#include <iterator>
#include <algorithm>
#ifdef _WIN32
#include <ctype.h>
#endif

using namespace Intel::OpenCL::ClangFE;

void CommonClangInitialize()
{
    llvm::llvm_start_multithreaded();
    llvm::InitializeAllTargets();
    llvm::InitializeAllAsmPrinters();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllTargetMCs();
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

extern "C" CC_DLL_EXPORT  int Compile(const char*   pszProgramSource,
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
    try
    {   // create a new scope to make sure the mutex will be released last
        llvm::OwningPtr<OCLFEBinaryResult> pResult ( new OCLFEBinaryResult() );

        // Parse options
        CompileOptionsParser optionsParser(pszDeviceSupportedExtensions, pszOpenCLVer);
        optionsParser.processOptions(pszOptions, pszOptionsEx);

        // Prepare input Buffer
        llvm::StringRef programSource(pszProgramSource);
        llvm::OwningPtr<llvm::MemoryBuffer> inputBuffer( llvm::MemoryBuffer::getMemBuffer( programSource, optionsParser.getSourceName()));

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

        // Set buffers (CompilerInstance takes ownership over its buffers)
        compiler->SetInputBuffer(inputBuffer.take());
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

        // Set an error handler, so that any LLVM backend diagnostics go through our error handler.
        // (currently commented out since setting the llvm error handling in multi-threaded environment is unsupported)
        //llvm::remove_fatal_error_handler();
        //llvm::install_fatal_error_handler(LLVMErrorHandler, static_cast<void*>(&compiler->getDiagnostics()));

        // Input Headers
        for (unsigned int i = 0; i < uiNumInputHeaders; ++i)
        {
            llvm::StringRef headerBuffer = llvm::StringRef(pInputHeaders[i]);
            llvm::MemoryBuffer *header = llvm::MemoryBuffer::getMemBuffer(pInputHeaders[i], pInputHeadersNames[i]);
            // CompilerInstance takes ownership over its buffers
            compiler->AddInMemoryHeader(header, pInputHeadersNames[i]);
        }

        // PCH buffer
        if( pPCHBuffer && uiPCHBufferSize > 0)
        {
            llvm::MemoryBuffer *pchBuffer = llvm::MemoryBuffer::getMemBuffer(llvm::StringRef(pPCHBuffer,uiPCHBufferSize), "", false);
            ppOptions.addRemappedFile("PCHeader.pch", pchBuffer);
            ppOptions.ImplicitPCHInclude = "PCHeader.pch";
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
}