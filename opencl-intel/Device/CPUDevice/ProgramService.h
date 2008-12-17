
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
//  ProgramService.h
//  Implementation of the Class ProgramService
//  Class Object is responsible on programs and kernels binaries
//  Intercats with the compiler backend
///////////////////////////////////////////////////////////

#pragma once

#include"cl_device_api.h"


namespace Intel { namespace OpenCL { namespace CPUDevice {

class ProgramService
{

public:
	ProgramService(cl_int devId, cl_dev_log_descriptor *logDesc);
	virtual ~ProgramService();

	cl_int checkProgramBinary (size_t IN bin_size, const void* IN bin);
    cl_int buildProgram( size_t IN bin_size,
									   const void* IN bin,
									   const cl_char* IN options,
									   void* IN user_data,
									   cl_dev_binary_prop IN prop,
									   cl_dev_program* OUT prog
									   );

	cl_int unloadCompiler();
    cl_int getProgramBinary( cl_dev_program IN prog,
										 size_t IN size,
										 void* OUT binary,
										 size_t* OUT size_ret
										 );

	cl_int getBuildLog( cl_dev_program IN prog,
									  size_t IN size,
									  char* OUT log,
									  size_t* OUT size_ret
									  );
	cl_int getSupportedBinaries( cl_uint IN count,
										   cl_prog_binary_desc* OUT types,
										   size_t* OUT size_ret
										   );


protected:
	cl_int					m_devId;
	cl_dev_log_descriptor*	m_logDesc;
};

}}}