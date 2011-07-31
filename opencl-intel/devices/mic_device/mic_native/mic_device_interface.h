
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
//
// defines internal structures for params passing between MIC host and device
//
///////////////////////////////////////////////////////////
#pragma once

#include <stdint.h>

namespace Intel { namespace OpenCL { namespace Utils {

//
// NOTE: please be carefull for alignments!
//

//
// copy_program_to_device
//   Buffers:
//       buffer1 - normal buffer with serialized program [IN]
//       buffer2 - normal buffer with COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT [OUT]
//   MiscData
//       input - COPY_PROGRAM_TO_DEVICE_INPUT_STRUCT
//       output - none
//
struct COPY_PROGRAM_TO_DEVICE_INPUT_STRUCT
{
    uint64_t required_executable_size;
    uint64_t number_of_kernels;
};

struct COPY_PROGRAM_TO_DEVICE_KERNEL_INFO
{
    uint64_t    kernel_id;
    uint64_t    device_info_ptr;
};

struct COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT
{
    uint64_t    filled_kernels;
    // array of pointers to device kernel structs with size == number_of_kernels
    // in COPY_PROGRAM_TO_DEVICE_INPUT_STRUCT
    COPY_PROGRAM_TO_DEVICE_KERNEL_INFO device_kernel_info_pts[1];
};

#define COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT_SIZE( number_of_kernels ) \
    ( sizeof(COPY_PROGRAM_TO_DEVICE_OUTPUT_STRUCT) + sizeof(COPY_PROGRAM_TO_DEVICE_KERNEL_INFO)*((number_of_kernels) - 1))

}}}

