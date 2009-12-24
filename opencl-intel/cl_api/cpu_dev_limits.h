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

#define CPU_DEV_MAX_WI_SIZE		255			// Maximum values that could be specified for WI in one dimension
#define CPU_DEV_MAX_WG_SIZE		255			// As maximum number of fibers in the thread
#define CPU_DEV_LCL_MEM_SIZE	(24*1024)	// Total DCU size is 32k, we will leave 8k for internal data
#define CPU_DCU_LINE_SIZE		64

#define ADJUST_SIZE_TO_DCU_LINE(X) ( ((X)+CPU_DCU_LINE_SIZE-1) & (~(CPU_DCU_LINE_SIZE-1)))

// Maximum number of arguments to be passed to the kernel
#define CPU_KERNEL_MAX_ARG_COUNT	256

#define CPU_MAX_SAMPLERS				16
#define CPU_MAX_PARAMETER_SIZE			1024
#define CPU_IMAGE3D_MAX_DIM_SIZE		2048
#define CPU_IMAGE2D_MAX_DIM_SIZE		8192
#define CPU_MAX_READ_IMAGE_ARGS			128
#define CPU_MAX_WRITE_IMAGE_ARGS		16
#define CPU_MAX_CONSTANT_BUFFER_SIZE	128000
#define CPU_MAX_CONSTANT_ARGS			128
#define CPU_MEM_BASE_ADDR_ALIGN			0
#define CPU_MAX_WORK_ITEM_DIMENSIONS	MAX_WORK_DIM
#define CPU_MAX_WORK_GROUP_SIZE			256 // Must be power of 2, No API to get max number of fibers
#define CPU_PROFILING_TIMER_RESOLUTION  1
#define CPU_MIN_ACTUAL_PARAM_SIZE		(sizeof(void*))
#define CPU_MIN_VECTOR_SIZE				8
