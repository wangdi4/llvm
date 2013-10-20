// Copyright (c) 2006-2009 Intel Corporation
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

#pragma once

#define OCL_INTEL_DX9_MEDIA_SHARING_EXT "cl_intel_dx9_media_sharing"
#define OCL_KHR_DX9_MEDIA_SHARING_EXT   "cl_khr_dx9_media_sharing"
#define OCL_KHR_D3D11_SHARING_EXT       "cl_khr_d3d11_sharing"

#define OCL_COMMON_SUPPORTED_EXTENSIONS "cl_khr_icd cl_khr_global_int32_base_atomics "\
    "cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics "\
    "cl_khr_local_int32_extended_atomics cl_khr_byte_addressable_store "\
    "cl_khr_spir cl_intel_exec_by_local_thread "\
    "cl_khr_depth_images cl_khr_3d_image_writes cl_khr_image2d_from_buffer"
#define OCL_SUPPORTED_EXTENSIONS_WIN "cl_khr_gl_sharing "\
    OCL_INTEL_DX9_MEDIA_SHARING_EXT " "\
    OCL_KHR_DX9_MEDIA_SHARING_EXT " "\
    OCL_KHR_D3D11_SHARING_EXT
#define OCL_DOUBLE "cl_khr_fp64"

static const char OCL_SUPPORTED_EXTENSIONS[] =
    OCL_COMMON_SUPPORTED_EXTENSIONS " "
#if defined __DOUBLE_ENABLED__
    OCL_DOUBLE " "
#endif
#if (_WIN32)
    OCL_SUPPORTED_EXTENSIONS_WIN " "
#endif
    ;

static const char OCL_SUPPORTED_EXTENSIONS_ATOM[] =
    OCL_COMMON_SUPPORTED_EXTENSIONS " "
#if (_WIN32)
    OCL_SUPPORTED_EXTENSIONS_WIN " "
#endif
    ;
