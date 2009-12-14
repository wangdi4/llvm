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

// FrontEnd callback function prototype
// This function is called upon FE compilation process comleted
// Output:
//		pData		-	the unique data passed within build function
//		pBuffer		-	A pointer to the buffer wich contains compiled binary
//		stSize		-	Size of the buffer
//		iStatus		-	Compilation status (return code)
//		szBuildLog	-	NULL terminates string with build process log
typedef void	(fn_BuildNotifyCallBack)(void* pData, void* pBuffer,
										size_t stBufferSize, int iStatus, const char* szErrLog);

// Build task descriptor, contains FE compilation info
struct	FEBuildProgramDesc
{
	unsigned int			uiLineCount;	//	A number of source lines in the program
	const char **			ppsLineArray;	//	A pointer to array of source lines
	size_t *				pLengths;		//	A pointer to array of source line lengths
	const char *			pszOptions;		//	A string for build options
	fn_BuildNotifyCallBack*	pCallBack;		//	A pointer to callback function,
											//	which will be called when
	void*					pData;			//	A unique data that will be returned with callback
};

// FrontEnd compiler invocation routine prototype
// Input:
//	pBuildDesc		- Build task descriptor
typedef int	(fn_FEBuildProgram)(FEBuildProgramDesc* pDesc);
