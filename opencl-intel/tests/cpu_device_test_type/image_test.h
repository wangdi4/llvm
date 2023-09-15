// Copyright (c) 2006 Intel Corporation
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
// image_test.h
///////////////////////////////////////////////////////////

#pragma once

#include "cl_device_api.h"

extern bool imageTest(bool profiling);
extern bool copyImage(bool profiling, IOCLDevMemoryObject *srcMemObj,
                      IOCLDevMemoryObject *dstMemObj, cl_uint src_dim_count,
                      cl_uint dst_dim_count, size_t src_origin[MAX_WORK_DIM],
                      size_t dst_origin[MAX_WORK_DIM],
                      size_t region[MAX_WORK_DIM]);
extern bool writeImage(bool profiling, IOCLDevMemoryObject *memObj,
                       void *pHostImage, unsigned int dim_count, size_t width,
                       size_t height, size_t depth, bool bIsBuffer);
extern bool readImage(bool profiling, IOCLDevMemoryObject *memObj,
                      void *pHostImage, unsigned int dim_count, size_t width,
                      size_t height, size_t depth, bool bIsBuffer);
