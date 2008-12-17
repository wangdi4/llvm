// Copyright (c) 2006-2008 Intel Corporation
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

///////////////////////////////////////////////////////////
//  ProgramService.cpp
//  Implementation of the Class ProgramService
//  Class Object is responsible on programs and kernels binaries
//  Intercats with the compiler backend
///////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ProgramService.h"

#include<stdlib.h>

using namespace Intel::OpenCL::CPUDevice;

ProgramService::ProgramService(cl_int devId, cl_dev_log_descriptor *logDesc) :
	m_devId(devId), m_logDesc(logDesc)
{

}



ProgramService::~ProgramService()
{

}

cl_int ProgramService::checkProgramBinary (size_t IN bin_size, const void* IN bin)
{
	return CL_DEV_INVALID_VALUE;
}
cl_int ProgramService::buildProgram( size_t IN bin_size,
								   const void* IN bin,
								   const cl_char* IN options,
								   void* IN user_data,
								   cl_dev_binary_prop IN prop,
								   cl_dev_program* OUT prog
								   )
{
	return CL_DEV_INVALID_BINARY;
}

cl_int ProgramService::unloadCompiler()
{
	return CL_DEV_SUCCESS;
}
cl_int ProgramService::getProgramBinary( cl_dev_program IN prog,
									 size_t IN size,
									 void* OUT binary,
									 size_t* OUT size_ret
									 )
{
	return CL_DEV_INVALID_VALUE;
}

cl_int ProgramService::getBuildLog( cl_dev_program IN prog,
								  size_t IN size,
								  char* OUT log,
								  size_t* OUT size_ret
								  )
{
	return CL_DEV_INVALID_VALUE;
}
cl_int ProgramService::getSupportedBinaries( cl_uint IN count,
									   cl_prog_binary_desc* OUT types,
									   size_t* OUT size_ret
									   )
{
		return CL_DEV_INVALID_VALUE;
}

