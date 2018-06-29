// INTEL CONFIDENTIAL
//
// Copyright 2009-2018 Intel Corporation.
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

#pragma once

#include <stddef.h>
#include <CL/cl.h>

namespace Intel { namespace OpenCL { namespace ClangFE {
    struct IOCLFEBinaryResult;
    struct IOCLFEKernelArgInfo {
        virtual size_t getNumArgs() const = 0;
        virtual const char *getArgName(size_t index) const = 0;
        virtual const char *getArgTypeName(size_t index) const = 0;
        virtual cl_kernel_arg_address_qualifier
        getArgAdressQualifier(size_t index) const = 0;
        virtual cl_kernel_arg_access_qualifier
        getArgAccessQualifier(size_t index) const = 0;
        virtual cl_kernel_arg_type_qualifier
        getArgTypeQualifier(size_t index) const = 0;
        virtual cl_bool
        getArgHostAccessible(size_t index) const = 0;
        virtual cl_int getArgLocalMemSize(size_t index) const = 0;
        // release result
        virtual void Release() = 0;
    protected:
        virtual ~IOCLFEKernelArgInfo() {}
    };
}}}

using namespace Intel::OpenCL::ClangFE;

namespace Intel { namespace OpenCL { namespace Utils {

class FrameworkUserLogger;

}}}

namespace Intel { namespace OpenCL { namespace FECompilerAPI {
#define CL_FE_INTERNAL_ERROR_OHNO -1    //thanks to doron for the awesome name

// Compile task descriptor, contains FE compilation info
struct FECompileProgramDescriptor
{
    // A pointer to main program's source (assumed one nullterminated string)
    const char*     pProgramSource;
    // the number of input headers in pInputHeaders
    unsigned int    uiNumInputHeaders;
    // array of additional input headers to be passed in memory
    const char**    pInputHeaders;
    // array of input headers names corresponding to pInputHeaders
    const char**    pszInputHeadersNames;
    // A string for compile options
    const char*     pszOptions;
    // Fpga emulator indicator
    bool            bFpgaEmulator;
};

// Link task descriptor, contains FE Linking info
struct FELinkProgramsDescriptor
{
    // array of binary containers
    const void**    pBinaryContainers;
    // the number of input binaries in pBinaryContainers
    unsigned int    uiNumBinaries;
    // the size in bytes of each container in pBinaryContainers
    const size_t*   puiBinariesSizes;
    // A string for link options
    const char*     pszOptions;
};

struct FESPIRVProgramDescriptor
{
    // binary container for SPIRV program
    const void*     pSPIRVContainer;
    // the size in bytes of the container pSPIRVContainer
    size_t          uiSPIRVContainerSize;
    // A string for compile options
    const char*     pszOptions;
};

struct FESPIRProgramDescriptor
{
    // binary container for SPIR 1.2 program
    const void*     pSPIRContainer;
    // the size in bytes of the container
    unsigned int    uiSPIRContainerSize;
};

// This interface represent FE compiler instance
class IOCLFECompiler
{
public:
    // Synchronous function
    // Input: pProgDesc - descriptor of the program to compile
    // Output: pBinaryResult - The interface to build result
    // Returns: Compile status
    virtual int CompileProgram(FECompileProgramDescriptor* pProgDesc, IOCLFEBinaryResult* *pBinaryResult) = 0;

    // Synchronous function
    // Input: pProgDesc - descriptor of the programs to link
    // Output: pBinaryResult - The interface to link result
    // Returns: Link status
    virtual int LinkPrograms(FELinkProgramsDescriptor* pProgDesc, IOCLFEBinaryResult* *pBinaryResult) = 0;

    // Synchronous function
    // Input: pProgDesc - descriptor of the program to parse
    // Output: pBinaryResult - The interface to parse result
    // Returns: SPIR-V parsing status
    virtual int ParseSPIRV(FESPIRVProgramDescriptor* pProgDesc, IOCLFEBinaryResult* *pBinaryResult) = 0;

    // Synchronous function
    // Input: pProgDesc - descriptor of the program to materialize
    // Output: pBinaryResult - The interface to parse result
    // Returns: SPIR 1.2 materialization status
    virtual int MaterializeSPIR(FESPIRProgramDescriptor *pProgDesc,
                                IOCLFEBinaryResult **pBinaryResult) = 0;

    // Synchronous function
    // Input: pBin - the program's binary including the header
    //        szKernelName - the name of the kernel for which we query the arg info
    // Output: pArgInfo - The interface to kernelArgInfo result
    // Returns: CL_SUCCESS in case of success
    //          CL_KERNEL_ARG_INFO_NOT_AVAILABLE if binary was built without -cl-kernel-arg-info
    //          CL_OUT_OF_HOST_MEMORY for out of host memory
    //          CL_FE_INTERNAL_ERROR_OHNO for internal errors (should never happen)
    virtual int GetKernelArgInfo(const void*        pBin,
                                 size_t             uiBinarySize,
                                 const char*        szKernelName,
                                 IOCLFEKernelArgInfo*   *pArgInfo) = 0;

    // Synchronous function
    // Input: szOptions - a string representing the compile options
    //        uiUnrecognizedOptionsSize - size of the szUnrecognizedOptions buffer
    // Output: szUnrecognizedOptions - a new string containing the unrecognized options separated by spaces
    // Returns: 'true' if the compile options are legal and 'false' otherwise
    virtual bool CheckCompileOptions(const char*  szOptions,
                                     char*        szUnrecognizedOptions,
                                     size_t       uiUnrecognizedOptionsSize) = 0;

    // Synchronous function
    // Input: szOptions - a string representing the link options
    //        uiUnrecognizedOptionsSize - size of the szUnrecognizedOptions buffer
    // Output: szUnrecognizedOptions - a new string containing the unrecognized options separated by spaces
    // Returns: 'true' if the link options are legal and 'false' otherwise
    virtual bool CheckLinkOptions(const char*  szOptions,
                                     char*        szUnrecognizedOptions,
                                     size_t       uiUnrecognizedOptionsSize) = 0;

    // release compiler instance
    virtual void Release() = 0;
};

// Create an instance of the FE compiler tagged to specific device
// Input: pDeviceInfo - device Specific information
typedef int fnCreateFECompilerInstance(const void* pDeviceInfo, size_t devInfoSize, IOCLFECompiler* *pFECompiler, Intel::OpenCL::Utils::FrameworkUserLogger* pUserLogger);
}}}

