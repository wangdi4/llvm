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
* File mic_dev_limits.h
* Declares constants (max values) of device properties
*
*/
#pragma once

#define MIC_DEV_MAX_WI_SIZE              1024            // Maximum values that could be specified for WI in one dimension
#define MIC_DEV_LCL_MEM_SIZE             (32*1024)
#define MIC_DEV_DCU_LINE_SIZE            64
#define MIC_DEV_MAXIMUM_ALIGN            512

#define ADJUST_SIZE_TO_DCU_LINE(X)       (((X)+MIC_DEV_DCU_LINE_SIZE-1) & (~(MIC_DEV_DCU_LINE_SIZE-1)))
#define ADJUST_SIZE_TO_MAXIMUM_ALIGN(X)  (((X)+MIC_DEV_MAXIMUM_ALIGN-1) & (~(MIC_DEV_MAXIMUM_ALIGN-1)))
#define MIN_PARAM(X,Y) ((X)<(Y)?(X):(Y))

// Minimum memory size allocate for single WI instance
#define MIC_DEV_MIN_WI_PRIVATE_SIZE      (1024*sizeof(size_t))

// Maximum number of arguments to be passed to the kernel
#define MIC_MAX_PARAMETER_SIZE           1024
#define MIC_MAX_PARAM_COUNT              (MIC_MAX_PARAMETER_SIZE/8)
#define MIC_KERNEL_MAX_ARG_COUNT         (MIN_PARAM((MIC_MAX_PARAMETER_SIZE/sizeof(void*)), MIC_MAX_PARAM_COUNT))
#define MIC_MAX_SAMPLERS                 (MIN_PARAM((MIC_MAX_PARAMETER_SIZE/sizeof(void*)), MIC_MAX_PARAM_COUNT))
#define MIC_IMAGE3D_MAX_DIM_SIZE         2048
#define MIC_IMAGE2D_MAX_DIM_SIZE         8192
#define MIC_MAX_READ_IMAGE_ARGS          (MIN_PARAM((MIC_MAX_PARAMETER_SIZE/sizeof(void*)), MIC_MAX_PARAM_COUNT))
#define MIC_MAX_WRITE_IMAGE_ARGS         (MIN_PARAM((MIC_MAX_PARAMETER_SIZE/sizeof(void*)), MIC_MAX_PARAM_COUNT))
#define MIC_MAX_CONSTANT_BUFFER_SIZE     (128*1024)
#define MIC_MAX_CONSTANT_ARGS            (MIN_PARAM((MIC_MAX_PARAMETER_SIZE/sizeof(void*)), MIC_MAX_PARAM_COUNT))
#define MIC_MAX_LOCAL_ARGS               (MIN_PARAM((MIC_MAX_PARAMETER_SIZE/sizeof(void*)), MIC_MAX_PARAM_COUNT))
#define MIC_MEM_BASE_ADDR_ALIGN          (MIC_DEV_MAXIMUM_ALIGN*8) // In bits
#define MIC_MAX_WORK_ITEM_DIMENSIONS     MAX_WORK_DIM
#define MIC_MAX_WORK_GROUP_SIZE          1024            // Must be power of 2, No API to get max number of fibers
#define MIC_DEFAULT_WG_SIZE              32
#define MIC_MIN_ACTUAL_PARAM_SIZE        sizeof(size_t)
#define MIC_MIN_ACTUAL_PARAM_PTR         size_t*
#define MIC_MIN_VECTOR_SIZE              16                // Minimum vector size, XMM == 16bytes

// min 1MB
#define MIC_PRINTF_BUFFER_SIZE           (1024*1024)

// Minimum memory size allocate for single WI instance
#define MIC_DEV_MIN_WI_PRIVATE_SIZE      (1024*sizeof(size_t))
// Maximum memory size that could be allocated for WG execution
#define MIC_DEV_MAX_WG_PRIVATE_SIZE      (MIC_DEV_MIN_WI_PRIVATE_SIZE*MIC_MAX_WORK_GROUP_SIZE)

//The muximum single buffer memory size (in bytes)
#define MIC_MAX_BUFFER_ALLOC_SIZE(deviceId) (MAX(128*1024*1024, MICSysInfo::getInstance().TotalPhysicalMemSize(deviceId)/4) & ~4095)
#define MIC_MAX_GLOBAL_MEM_SIZE(deviceId) (((MICSysInfo::getInstance().TotalPhysicalMemSize(deviceId)*3)/4) & ~4095)
#define MIC_AVAILABLE_PROCESS_MEMORY(deviceId) (MIN(MIC_MAX_GLOBAL_MEM_SIZE(deviceId) + 1024 * 1024 * 100, MICSysInfo::getInstance().TotalPhysicalMemSize(deviceId)) & ~4095)

// redirect all I/O requests from device to host, includion files and std handles
#define MIC_DEV_IO_PROXY_TO_HOST            true

#define NOTIFICATION_PORT_MAX_BARRIERS      256
