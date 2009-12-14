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
