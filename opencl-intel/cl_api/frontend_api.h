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

namespace Intel { namespace OpenCL { namespace FECompilerAPI {

// Build task descriptor, contains FE compilation info
struct	FEBuildProgramDescriptor
{
	const char*		pInput;		// A pointer source lines
	size_t			pInputLen;	// source line length
	const char *	pszOptions;	// A string for build options
};

// This interface represents the FE compiler build result
class IOCLFEBuildProgramResult
{
public:
	virtual size_t	GetIRSize() = 0;
	virtual const void*	GetIR() = 0;
	virtual const char* GetErrorLog() = 0;
	// release result
	virtual void Release() = 0;

};

// This interface represent FE compiler instance
class IOCLFECompiler
{
public:
	// Syncronious function
	// Input: pProgDesc - descriptor of the program to build
	// Output: The inerface to build result
	// Returns: Build status
	virtual int BuildProgram(FEBuildProgramDescriptor* pProgDesc, IOCLFEBuildProgramResult* *pBuildResult) = 0;

	// release compiler instance
	virtual void Release() = 0;
};

// Create an instance of the FE compiler tagged to specific device
// Input: pDeviceInfo - device Specific information
typedef int fnCreateFECompilerInstance(const void* pDeviceInfo, size_t devInfoSize, IOCLFECompiler* *pFECompiler);

}}}
