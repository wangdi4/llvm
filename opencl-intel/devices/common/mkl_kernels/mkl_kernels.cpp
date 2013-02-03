// Copyright (c) 2006-2012 Intel Corporation
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
//  mkl_kernels.cpp
///////////////////////////////////////////////////////////


#include "stdafx.h"

#include "mkl_kernels.h"
#include "export/mkl_builtins.h"
#include <cl_dynamic_lib.h>

using namespace Intel::OpenCL::MKLKernels;

/////////////////////////////////////////////////////////////////////////////
// Create registration classes for MKL functions
#define REGISTER_MKL_FUNCTION(MKL_FUNCTION_NAME,MKL_CLASS_TYPE,DATA_TYPE) \
	template<> const MKL_##MKL_CLASS_TYPE##_Parameters<DATA_TYPE> MKL_##MKL_CLASS_TYPE##_Executor<DATA_TYPE>::m_sParams = MKL_##MKL_CLASS_TYPE##_Parameters<DATA_TYPE>();

#include "mkl_kernels.inc"

#undef REGISTER_MKL_FUNCTION
/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

Intel::OpenCL::Utils::OclDynamicLib g_mklRT;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Register MKL functions inside the Builtin kernels map
#define REGISTER_MKL_FUNCTION(MKL_FUNCTION_NAME,MKL_CLASS_TYPE,DATA_TYPE) \
	struct MKL_FUNCTION_NAME##CreatorClass\
	{\
		static cl_dev_err_code MKL_FUNCTION_NAME##Creator(Intel::OpenCL::BuiltInKernels::IBuiltInKernel* *ppBIKernel)\
		{\
			void* pFunc = g_mklRT.GetFunctionPtrByName(#MKL_FUNCTION_NAME);\
			if ( NULL==pFunc ) return CL_DEV_NOT_SUPPORTED;\
		    *ppBIKernel = new MKLKernel< MKL_##MKL_CLASS_TYPE##_Executor<DATA_TYPE > >(#MKL_FUNCTION_NAME, pFunc);\
			return CL_DEV_SUCCESS;\
		}\
	};\
 	REGISTER_BUILTIN_KERNEL(MKL_FUNCTION_NAME, MKL_FUNCTION_NAME##CreatorClass::MKL_FUNCTION_NAME##Creator)

bool Intel::OpenCL::MKLKernels::InitLibrary()
{
	// Check if MKL library in the system path
#ifdef WIN32	
	if ( !g_mklRT.Load("mkl_rt.dll") )
#else
	if ( !g_mklRT.Load("libmkl_rt.so") )
#endif
	{
		return false;
	}

	// Import set of exposed MKL functions
	#include"mkl_kernels.inc"

	return true;
}

#undef REGISTER_MKL_FUNCTION

