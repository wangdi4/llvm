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

#define OCL_COMMON_SUPPORTED_EXTENSIONS "cl_khr_global_int32_base_atomics "\
    "cl_khr_global_int32_extended_atomics cl_khr_local_int32_base_atomics "\
    "cl_khr_local_int32_extended_atomics cl_khr_byte_addressable_store "\
    "cl_intel_printf cl_ext_device_fission"
#define OCL_SUPPORTED_EXTENSIONS_WIN "cl_khr_gl_sharing cl_intel_dx9_media_sharing"
#define OCL_DOUBLE "cl_khr_fp64"

static const char OCL_SUPPORTED_EXTENSIONS[] = 
#if defined __DOUBLE_ENABLED__
#if (_WIN32)                                                
    OCL_DOUBLE " " OCL_COMMON_SUPPORTED_EXTENSIONS " " OCL_SUPPORTED_EXTENSIONS_WIN;
#else
    OCL_DOUBLE " " OCL_COMMON_SUPPORTED_EXTENSIONS;
#endif
#else
#if (_WIN32)                                                
    OCL_COMMON_SUPPORTED_EXTENSIONS " " OCL_SUPPORTED_EXTENSIONS_WIN;
#else
    OCL_COMMON_SUPPORTED_EXTENSIONS;
#endif
#endif
