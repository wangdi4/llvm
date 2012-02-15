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

/*
*
* File cpu_dev_limits.h
* Declares constants (max values) of device properties
*
*/
#pragma once

#define CPU_DEV_MAX_WI_SIZE				1024			// Maximum values that could be specified for WI in one dimension
#define CPU_DEV_LCL_MEM_SIZE			(32*1024)
#define CPU_DEV_DCU_LINE_SIZE			64
#define CPU_DEV_MAXIMUM_ALIGN			128

#define ADJUST_SIZE_TO_DCU_LINE(X)			( ((X)+CPU_DEV_DCU_LINE_SIZE-1) & (~(CPU_DEV_DCU_LINE_SIZE-1)))
#define ADJUST_SIZE_TO_MAXIMUM_ALIGN(X)		( ((X)+CPU_DEV_MAXIMUM_ALIGN-1) & (~(CPU_DEV_MAXIMUM_ALIGN-1)))
#define MIN_PARAM(X,Y) ((X)<(Y)?(X):(Y))

// Maximum number of arguments to be passed to the kernel
#define CPU_MAX_PARAMETER_SIZE			(4096-256)
#define CPU_MAX_PARAM_COUNT				(CPU_MAX_PARAMETER_SIZE/8)
#define CPU_KERNEL_MAX_ARG_COUNT		(MIN_PARAM((CPU_MAX_PARAMETER_SIZE/sizeof(void*)), CPU_MAX_PARAM_COUNT))
#define CPU_MAX_SAMPLERS				(MIN_PARAM((CPU_MAX_PARAMETER_SIZE/sizeof(void*)), CPU_MAX_PARAM_COUNT))
#define CPU_IMAGE3D_MAX_DIM_SIZE		2048
#define CPU_IMAGE2D_MAX_DIM_SIZE		8192
#define CPU_MAX_READ_IMAGE_ARGS			(MIN_PARAM((CPU_MAX_PARAMETER_SIZE/sizeof(void*)), CPU_MAX_PARAM_COUNT))
#define CPU_MAX_WRITE_IMAGE_ARGS		(MIN_PARAM((CPU_MAX_PARAMETER_SIZE/sizeof(void*)), CPU_MAX_PARAM_COUNT))
#define CPU_MAX_CONSTANT_BUFFER_SIZE	(128*1024)
#define CPU_MAX_CONSTANT_ARGS			(MIN_PARAM((CPU_MAX_PARAMETER_SIZE/sizeof(void*)), CPU_MAX_PARAM_COUNT))
#define CPU_MAX_LOCAL_ARGS				(MIN_PARAM((CPU_MAX_PARAMETER_SIZE/sizeof(void*)), CPU_MAX_PARAM_COUNT))
#define CPU_MEM_BASE_ADDR_ALIGN			(CPU_DEV_MAXIMUM_ALIGN*8) // In bits
#define CPU_MAX_WORK_ITEM_DIMENSIONS	MAX_WORK_DIM
// Assuming MAX_WORK_DIM == 3
#define CPU_MAX_WI_DIM_POW_OF_2	(MAX_WORK_DIM+1)
#define CPU_MAX_WORK_GROUP_SIZE			1024			// Must be power of 2, No API to get max number of fibers
#define CPU_DEFAULT_WG_SIZE				32
#define CPU_MIN_ACTUAL_PARAM_SIZE		sizeof(size_t)
#define CPU_MIN_ACTUAL_PARAM_PTR		size_t*
#define CPU_MIN_VECTOR_SIZE				16				// Minimum vector size, XMM == 16bytes

// Minimum memory size allocate for single WI instance
#define CPU_DEV_MIN_WI_PRIVATE_SIZE		(1024*sizeof(size_t))
// Maximum private memory size that could be allocated for WG execution
#define CPU_DEV_MAX_WG_PRIVATE_SIZE		(CPU_DEV_MIN_WI_PRIVATE_SIZE*CPU_MAX_WORK_GROUP_SIZE)
// Maximum memory size that could be allocated for WG execution. This is the sum of
// WG Private memory size +
// Kernel parameters size (twice to cover the hidden parameters) +
// Local IDs buffer
#define CPU_DEV_MAX_WG_TOTAL_SIZE		  (CPU_DEV_MAX_WG_PRIVATE_SIZE + (2*CPU_MAX_PARAMETER_SIZE) + \
  (CPU_MAX_PARAMETER_SIZE*CPU_MAX_WI_DIM_POW_OF_2*sizeof(size_t)))
