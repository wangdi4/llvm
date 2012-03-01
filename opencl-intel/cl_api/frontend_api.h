// Copyright (c) 2006-2011 Intel Corporation
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

#include <stddef.h>
#include <CL/cl.h>

namespace Intel { namespace OpenCL { namespace FECompilerAPI {

#define CL_FE_INTERNAL_ERROR_OHNO -1    //thanks to doron for the awesome name

class FEKernelArgInfo
{
public:
    virtual unsigned int getNumArgs() const = 0;
    virtual const char* getArgName(unsigned int index) const = 0;
    virtual const char* getArgTypeName(unsigned int index) const = 0;
    virtual cl_kernel_arg_address_qualifier getArgAdressQualifier(unsigned int index) const = 0;
    virtual cl_kernel_arg_access_qualifier getArgAccessQualifier(unsigned int index) const = 0;
    virtual cl_kernel_arg_type_qualifier getArgTypeQualifier(unsigned int index) const = 0;

    // release result
	virtual long Release() = 0;
};

// Compile task descriptor, contains FE compilation info
struct	FECompileProgramDescriptor
{
    // A pointer to main program's source (assumed one nullterminated string)
	const char*		pProgramSource;		
    // the number of input headers in pInputHeaders
    unsigned int    uiNumInputHeaders; 
    // array of additional input headers to be passed in memory
    const char**    pInputHeaders;
    // array of input headers names corresponding to pInputHeaders
    const char**    pszInputHeadersNames;  
        // A string for compile options
	const char *	pszOptions;	
};

// Link task descriptor, contains FE Linking info
struct	FELinkProgramsDescriptor
{
    // array of binary containers
	const void**    pBinaryContainers;	
    // the number of input binaries in pBinaryContainers
    unsigned int    uiNumBinaries;
    // the size in bytes of each container in pBinaryContainers
    const size_t*   puiBinariesSizes;
    // A string for link options
	const char *	pszOptions;
};

// This interface represents the FE compiler build result
class IOCLFEBinaryResult
{
public:
	virtual size_t	GetIRSize() = 0;
	virtual const void*	GetIR() = 0;
	virtual const char* GetErrorLog() = 0;
	// release result
	virtual long Release() = 0;

    // Will be true if link is called with "-create-library"
    virtual bool IsLibrary() { return false; }
};

// This interface represent FE compiler instance
class IOCLFECompiler
{
public:
    // Synchronous function
	// Input: pProgDesc - descriptor of the program to compile
	// Output: The inerface to build result
	// Returns: Compile status
    virtual int CompileProgram(FECompileProgramDescriptor* pProgDesc, IOCLFEBinaryResult* *pBinaryResult) = 0;

    // Synchronous function
	// Input: pProgDesc - descriptor of the programs to link
	// Output: The inerface to link result
	// Returns: Link status
    virtual int LinkPrograms(FELinkProgramsDescriptor* pProgDesc, IOCLFEBinaryResult* *pBinaryResult) = 0;

    // Synchronous function
	// Input: pBin - the program's binary including the header
    //        szKernelName - the name of the kernel for which we query the arg info
	// Output: The interface to kernelArgInfo result
	// Returns: CL_SUCCESS in case of success
    //          CL_KERNEL_ARG_INFO_NOT_AVAILABLE if binary was built without -cl-kernel-arg-info
    //          CL_OUT_OF_HOST_MEMORY for out of host memory
    //          CL_FE_INTERNAL_ERROR_OHNO for internal errors (should never happen)
    virtual int GetKernelArgInfo(const void*        pBin,
                                 const char*        szKernelName,
                                 FEKernelArgInfo*   *pArgInfo) = 0;

	// release compiler instance
	virtual void Release() = 0;
};

// Create an instance of the FE compiler tagged to specific device
// Input: pDeviceInfo - device Specific information
typedef int fnCreateFECompilerInstance(const void* pDeviceInfo, size_t devInfoSize, IOCLFECompiler* *pFECompiler);

}}}
