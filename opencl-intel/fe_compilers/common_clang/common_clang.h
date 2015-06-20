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

#pragma once
#include "CL/cl.h"
#include "assert.h"

#if defined (_WIN32)
#if defined (COMMON_CLANG_EXPORTS)
#define CC_DLL_EXPORT _declspec(dllexport)
#else
#define CC_DLL_EXPORT _declspec(dllimport)
#endif
#else
#define CC_DLL_EXPORT
#endif

namespace Intel { namespace OpenCL { namespace ClangFE {
    //
    // Type of the binary returned after compilation and/or link
    //
    enum IR_TYPE
    {
        IR_TYPE_UNKNOWN,
        IR_TYPE_EXECUTABLE,
        IR_TYPE_LIBRARY,
        IR_TYPE_COMPILED_OBJECT
    };

    //
    // Compilation results interface
    // Returned by Compile method
    //
    struct IOCLFEBinaryResult
    {
        // Returns the size in bytes of the IR buffer
        virtual size_t      GetIRSize() const = 0;
        // Returns the pointer to the IR buffer or NULL if no IR buffer is present
        virtual const void* GetIR() const = 0;
        // Returns the name of the program
        virtual const char* GetIRName() const = 0;
        // Returns the type of the resulted binary
        virtual IR_TYPE GetIRType() const = 0;
        // Returns the pointer to the compilation log string or NULL if not log was created
        virtual const char* GetErrorLog() const = 0;
        // Releases the result object
        virtual void        Release() = 0;
        protected:
        virtual             ~IOCLFEBinaryResult(){}
    };

    //
    // GetKernelArgInfo function result structure
    //
    struct IOCLFEKernelArgInfo
    {
    public:
        virtual unsigned int getNumArgs() const = 0;
        virtual const char* getArgName(unsigned int index) const = 0;
        virtual const char* getArgTypeName(unsigned int index) const = 0;
        virtual cl_kernel_arg_address_qualifier getArgAdressQualifier(unsigned int index) const = 0;
        virtual cl_kernel_arg_access_qualifier getArgAccessQualifier(unsigned int index) const = 0;
        virtual cl_kernel_arg_type_qualifier getArgTypeQualifier(unsigned int index) const = 0;

        // release result
        virtual void Release() = 0;
        protected:
        virtual             ~IOCLFEKernelArgInfo(){}
    };
}}}

//
//Verifies the given OpenCL application supplied compilation options
//Params:
//    pszOptions - compilation options string
//    pszUnknownOptions - optional outbound pointer to the space separated unrecognized options
//    uiUnknownOptionsSize - size of the pszUnknownOptions buffer
//Returns:
//    true if the options verification was successful, false otherwise
//
extern "C" CC_DLL_EXPORT bool CheckCompileOptions(
    // A string for compile options
    const char*     pszOptions,
    // buffer to get the list of unknown options
    char* pszUnknownOptions,
    // size of the buffer for unknown options
    size_t uiUnknownOptionsSize
    );

//
//Verifies the given OpenCL application supplied link options
//Params:
//    pszOptions - compilation options string
//    pszUnknownOptions - optional outbound pointer to the space separated unrecognized options
//    uiUnknownOptionsSize - size of the pszUnknownOptions buffer
//Returns:
//    true if the options verification was successful, false otherwise
//
extern "C" CC_DLL_EXPORT bool CheckLinkOptions(
    // A string for compile options
    const char*     pszOptions,
    // buffer to get the list of unknown options
    char* pszUnknownOptions,
    // size of the buffer for unknown options
    size_t uiUnknownOptionsSize
    );

//
//Compiles the given OpenCL program to the LLVM IR
//Params:
//    pProgramSource - OpenCL source program to compile
//    pInputHeaders - array of the header buffers
//    uiNumInputHeader - size of the pInputHeaders array
//    pszInputHeadersNames - array of the headers names
//    pPCHBuffer - optional pointer to the pch buffer
//    uiPCHBufferSize - size of the pch buffer
//    pszOptions - OpenCL application supplied options
//    pszOptionsEx - optional extra options string usually supplied by runtime
//    pszDeviceSupportedExtentions - space separated list of the supported OpenCL extensions string
//    pszOpenCLVer - OpenCL version string - "120" for OpenCL 1.2, "200" for OpenCL 2.0, ...
//    pBinaryResult - optional outbound pointer to the compilation results
//Returns:
//    Compilation Result as int:  0 - success, error otherwise.
//
extern "C" CC_DLL_EXPORT int Compile(
    // A pointer to main program's source (null terminated string)
    const char*     pszProgramSource,
    // array of additional input headers to be passed in memory (each null terminated)
    const char**    pInputHeaders,
    // the number of input headers in pInputHeaders
    unsigned int    uiNumInputHeaders,
    // array of input headers names corresponding to pInputHeaders
    const char**    pInputHeadersNames,
    // optional pointer to the pch buffer
    const char*     pPCHBuffer,
    // size of the pch buffer
    size_t          uiPCHBufferSize,
    // OpenCL application supplied options
    const char*     pszOptions,
    // optional extra options string usually supplied by runtime
    const char*     pszOptionsEx,
    // space separated list of the supported OpenCL extensions string
    const char*     pszDeviceSupportedExtentions,
    // OpenCL version string - "120" for OpenCL 1.2, "200" for OpenCL 2.0, ...
    const char*     pszOpenCLVer,
    // optional outbound pointer to the compilation results
    Intel::OpenCL::ClangFE::IOCLFEBinaryResult** pBinaryResult
    );

//
//Links the given OpenCL binaries
//Params:
//    pInputHeaders - array of the header buffers
//    uiNumInputHeader - size of the pInputHeaders array
//    pszInputHeadersNames - array of the headers names
//    pPCHBuffer - optional pointer to the pch buffer
//    uiPCHBufferSize - size of the pch buffer
//    pszOptions - OpenCL application supplied options
//    pszOptionsEx - optional extra options string usually supplied by runtime
//    pszDeviceSupportedExtentions - space separated list of the supported OpenCL extensions string
//    pszOpenCLVer - OpenCL version string - "120" for OpenCL 1.2, "200" for OpenCL 2.0, ...
//    pBinaryResult - optional outbound pointer to the compilation results
//Returns:
//    Link Result as int:  0 - success, error otherwise.
//
extern "C" CC_DLL_EXPORT int Link(
    // array of additional input headers to be passed in memory
    const void**    pInputBinaries,
    // the number of input binaries
    unsigned int    uiNumBinaries,
    // the size in bytes of each binary
    const size_t*   puiBinariesSizes,
    // OpenCL application supplied options
    const char*     pszOptions,
    // optional outbound pointer to the compilation results
    Intel::OpenCL::ClangFE::IOCLFEBinaryResult** pBinaryResult
    );

//
//Gets the kernel argument information for the specific kernel
//Params:
//    pBin - pointer to the binary to query
//    uiBinarySize - size of the binary pointed by pBin
//    szKernelName - name of the kernel to query the info about
//    ppResult - pointer to the result structure - allocated by function
//               client should call Release method to free the memory
//Returns:
//    Link Result as int:  0 - success, error otherwise.
//
extern "C" CC_DLL_EXPORT int GetKernelArgInfo(
    const void *pBin,
    size_t uiBinarySize,
    const char *szKernelName,
    Intel::OpenCL::ClangFE::IOCLFEKernelArgInfo** ppResult);
