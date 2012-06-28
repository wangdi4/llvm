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

#ifdef __INCLUDE_MKL__

#include<tbb/tbb.h>
#include "mkl_kernels.h"

using namespace Intel::OpenCL::CPUDevice;

/////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////

REGISTER_MKL_FUNCTION(cblas_sgemm, GEMM, float)
REGISTER_MKL_FUNCTION(cblas_dgemm, GEMM, double)

cl_kernel_arg_address_qualifier Intel::OpenCL::CPUDevice::ArgType2AddrQual(cl_kernel_arg_type type)
{
	switch(type)
	{
	case CL_KRNL_ARG_INT: case CL_KRNL_ARG_UINT: case CL_KRNL_ARG_FLOAT: case CL_KRNL_ARG_DOUBLE:
	case CL_KRNL_ARG_VECTOR: case CL_KRNL_ARG_SAMPLER: case CL_KRNL_ARG_COMPOSITE:
		return CL_KERNEL_ARG_ADDRESS_PRIVATE;

	case CL_KRNL_ARG_PTR_LOCAL:
		return CL_KERNEL_ARG_ADDRESS_LOCAL;

	case CL_KRNL_ARG_PTR_CONST:
		return CL_KERNEL_ARG_ADDRESS_CONSTANT;

	case CL_KRNL_ARG_PTR_GLOBAL:
	default:
		return CL_KERNEL_ARG_ADDRESS_GLOBAL;

	}
}

#endif // __INCLUDE_MKL__