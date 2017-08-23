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

// define all supported extensions names
// each device may build it's supported extension list from these defines

#define OCL_EXT_KHR_ICD                     "cl_khr_icd"
#define OCL_INTEL_VEC_LEN_HINT              "cl_intel_vec_len_hint"
#define OCL_EXT_KHR_GLOBAL_BASE_ATOMICS     "cl_khr_global_int32_base_atomics"
#define OCL_EXT_KHR_GLOBAL_EXTENDED_ATOMICS "cl_khr_global_int32_extended_atomics"
#define OCL_EXT_KHR_LOCAL_BASE_ATOMICS      "cl_khr_local_int32_base_atomics"
#define OCL_EXT_KHR_LOCAL_EXTENDED_ATOMICS  "cl_khr_local_int32_extended_atomics"
#define OCL_EXT_KHR_BYTE_ADDRESSABLE_STORE  "cl_khr_byte_addressable_store"

#define OCL_EXT_KHR_FP64                    "cl_khr_fp64"

#define OCL_EXT_KHR_DX9_MEDIA_SHARING       "cl_khr_dx9_media_sharing"
#define OCL_EXT_INTEL_DX9_MEDIA_SHARING     "cl_intel_dx9_media_sharing"
#define OCL_EXT_KHR_D3D11_SHARING           "cl_khr_d3d11_sharing"
#define OCL_EXT_KHR_GL_SHARING              "cl_khr_gl_sharing"

#define OCL_EXT_KHR_DEPTH_IMAGES            "cl_khr_depth_images"
#define OCL_EXT_KHR_3D_IMAGE_WRITES         "cl_khr_3d_image_writes"
#define OCL_EXT_INTEL_EXEC_BY_LOCAL_THREAD  "cl_intel_exec_by_local_thread"

#define OCL_EXT_KHR_SPIR                    "cl_khr_spir"

#define OCL_EXT_KHR_IMAGE2D_FROM_BUFFER     "cl_khr_image2d_from_buffer"
