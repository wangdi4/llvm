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
#ifndef INLINE_ATTRIBUTE
#define INLINE_ATTRIBUTE __attribute__((always_inline))
#endif

#ifndef INTERNAL_INLINE_ATTRIBUTE 
#define INTERNAL_INLINE_ATTRIBUTE inline INLINE_ATTRIBUTE
#endif

#if defined(__SSE4_2__)
// Naming style:
// <transpose type>_<original trype in user kernel> x <packet width>
// Example: In the original kernel the user used char4, the packet width is 8 
//          and the operation is load and transpose, the function name will be:
// load_transpose_char4x8(...)


// ****************************************************************************
//                                 char4x4
// ****************************************************************************

void INLINE_ATTRIBUTE __ocl_load_transpose_char4x4(char4* pLoadAdd, char4* xOut, char4* yOut, char4* zOut, char4* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_store_char4x4(char4* pStoreAdd, char4 xIn, char4 yIn, char4 zIn, char4 wIn);

void INLINE_ATTRIBUTE __ocl_gather_transpose_char4x4(char4* pLoadAdd0, char4* pLoadAdd1, char4* pLoadAdd2, char4* pLoadAdd3,
                              char4* xOut, char4* yOut, char4* zOut, char4* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_scatter_char4x4(char4* pStoreAdd0, char4* pStoreAdd1, char4* pStoreAdd2, char4* pStoreAdd3,
                               char4 xIn, char4 yIn, char4 zIn, char4 wIn);

#if defined(__AVX__)
void INLINE_ATTRIBUTE __ocl_masked_load_transpose_char4x4(char4* pLoadAdd, char4* xOut, char4* yOut, char4* zOut, char4* wOut, int4 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_store_char4x4(char4* pStoreAdd, char4 xIn, char4 yIn, char4 zIn, char4 wIn, int4 mask);
void INLINE_ATTRIBUTE __ocl_masked_gather_transpose_char4x4(char4* pLoadAdd0, char4* pLoadAdd1, char4* pLoadAdd2, char4* pLoadAdd3,
                              char4* xOut, char4* yOut, char4* zOut, char4* wOut, int4 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_scatter_char4x4(char4* pStoreAdd0, char4* pStoreAdd1, char4* pStoreAdd2, char4* pStoreAdd3,
                               char4 xIn, char4 yIn, char4 zIn, char4 wIn, int4 mask);
#endif // defined(__AVX__)


// ****************************************************************************
//                                 char4x8
// ****************************************************************************

void INLINE_ATTRIBUTE __ocl_load_transpose_char4x8(char4* pLoadAdd, char8* xOut, char8* yOut, char8* zOut, char8* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_store_char4x8(char4* pStoreAdd, char8 xIn, char8 yIn, char8 zIn, char8 wIn);


void INLINE_ATTRIBUTE __ocl_gather_transpose_char4x8(char4* pLoadAdd0, char4* pLoadAdd1, char4* pLoadAdd2, char4* pLoadAdd3,
                              char4* pLoadAdd4, char4* pLoadAdd5, char4* pLoadAdd6, char4* pLoadAdd7,
                              char8* xOut, char8* yOut, char8* zOut, char8* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_scatter_char4x8(char4* pStoreAdd0, char4* pStoreAdd1, char4* pStoreAdd2, char4* pStoreAdd3,
                               char4* pStoreAdd4, char4* pStoreAdd5, char4* pStoreAdd6, char4* pStoreAdd7,
                               char8 xIn, char8 yIn, char8 zIn, char8 wIn);


#if defined(__AVX__)
void INLINE_ATTRIBUTE __ocl_masked_load_transpose_char4x8(char4* pLoadAdd, char8* xOut, char8* yOut, char8* zOut, char8* wOut, int8 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_store_char4x8(char4* pStoreAdd, char8 xIn, char8 yIn, char8 zIn, char8 wIn, int8 mask);

void INLINE_ATTRIBUTE __ocl_masked_gather_transpose_char4x8(char4* pLoadAdd0, char4* pLoadAdd1, char4* pLoadAdd2, char4* pLoadAdd3,
                              char4* pLoadAdd4, char4* pLoadAdd5, char4* pLoadAdd6, char4* pLoadAdd7,
                              char8* xOut, char8* yOut, char8* zOut, char8* wOut, int8 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_scatter_char4x8(char4* pStoreAdd0, char4* pStoreAdd1, char4* pStoreAdd2, char4* pStoreAdd3,
                               char4* pStoreAdd4, char4* pStoreAdd5, char4* pStoreAdd6, char4* pStoreAdd7,
                               char8 xIn, char8 yIn, char8 zIn, char8 wIn, int8 mask);
#endif // defined(__AVX__)

// ****************************************************************************
//                                 int4x4
// ****************************************************************************

void INLINE_ATTRIBUTE __ocl_load_transpose_int4x4(int4* pLoadAdd, int4* xOut, int4* yOut, int4* zOut, int4* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_store_int4x4(int4* pStoreAdd, int4 xIn, int4 yIn, int4 zIn, int4 wIn);

void INLINE_ATTRIBUTE __ocl_gather_transpose_int4x4(int4* pLoadAdd0, int4* pLoadAdd1, int4* pLoadAdd2, int4* pLoadAdd3,
                             int4* xOut, int4* yOut, int4* zOut, int4* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_scatter_int4x4(int4* pStoreAdd0, int4* pStoreAdd1, int4* pStoreAdd2, int4* pStoreAdd3,
                              int4 xIn, int4 yIn, int4 zIn, int4 wIn);

#if defined(__AVX__)
void INLINE_ATTRIBUTE __ocl_masked_load_transpose_int4x4(int4* pLoadAdd, int4* xOut, int4* yOut, int4* zOut, int4* wOut, int4 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_store_int4x4(int4* pStoreAdd, int4 xIn, int4 yIn, int4 zIn, int4 wIn, int4 mask);

void INLINE_ATTRIBUTE __ocl_masked_gather_transpose_int4x4(int4* pLoadAdd0, int4* pLoadAdd1, int4* pLoadAdd2, int4* pLoadAdd3,
                             int4* xOut, int4* yOut, int4* zOut, int4* wOut, int4 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_scatter_int4x4(int4* pStoreAdd0, int4* pStoreAdd1, int4* pStoreAdd2, int4* pStoreAdd3,
                              int4 xIn, int4 yIn, int4 zIn, int4 wIn, int4 mask);
#endif // defined(__AVX__)

// ****************************************************************************
//                                 int4x8
// ****************************************************************************

#if defined(__AVX__)
void INLINE_ATTRIBUTE __ocl_load_transpose_int4x8(int4* pLoadAdd, int8* xOut, int8* yOut, int8* zOut, int8* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_store_int4x8(int4* pStoreAdd, int8 xIn, int8 yIn, int8 zIn, int8 wIn);

void INLINE_ATTRIBUTE __ocl_masked_load_transpose_int4x8(int4* pLoadAdd, int8* xOut, int8* yOut, int8* zOut, int8* wOut, int8 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_store_int4x8(int4* pStoreAdd, int8 xIn, int8 yIn, int8 zIn, int8 wIn, int8 mask);

void INLINE_ATTRIBUTE __ocl_gather_transpose_int4x8(int4* pLoadAdd0, int4* pLoadAdd1, int4* pLoadAdd2, int4* pLoadAdd3,
                             int4* pLoadAdd4, int4* pLoadAdd5, int4* pLoadAdd6, int4* pLoadAdd7,
                             int8* xOut, int8* yOut, int8* zOut, int8* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_scatter_int4x8(int4* pStoreAdd0, int4* pStoreAdd1, int4* pStoreAdd2, int4* pStoreAdd3,
                              int4* pStoreAdd4, int4* pStoreAdd5, int4* pStoreAdd6, int4* pStoreAdd7,
                              int8 xIn, int8 yIn, int8 zIn, int8 wIn);

void INLINE_ATTRIBUTE __ocl_masked_gather_transpose_int4x8(int4* pLoadAdd0, int4* pLoadAdd1, int4* pLoadAdd2, int4* pLoadAdd3,
                             int4* pLoadAdd4, int4* pLoadAdd5, int4* pLoadAdd6, int4* pLoadAdd7,
                             int8* xOut, int8* yOut, int8* zOut, int8* wOut, int8 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_scatter_int4x8(int4* pStoreAdd0, int4* pStoreAdd1, int4* pStoreAdd2, int4* pStoreAdd3,
                              int4* pStoreAdd4, int4* pStoreAdd5, int4* pStoreAdd6, int4* pStoreAdd7,
                              int8 xIn, int8 yIn, int8 zIn, int8 wIn, int8 mask);
#endif // __AVX__

// ****************************************************************************
//                                 float4x4
// ****************************************************************************

void INLINE_ATTRIBUTE __ocl_load_transpose_float4x4(float4* pLoadAdd, float4* xOut, float4* yOut, float4* zOut, float4* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_store_float4x4(float4* pStoreAdd, float4 xIn, float4 yIn, float4 zIn, float4 wIn);

void INLINE_ATTRIBUTE __ocl_gather_transpose_float4x4(float4* pLoadAdd0, float4* pLoadAdd1, float4* pLoadAdd2, float4* pLoadAdd3,
                               float4* xOut, float4* yOut, float4* zOut, float4* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_scatter_float4x4(float4* pStoreAdd0, float4* pStoreAdd1, float4* pStoreAdd2, float4* pStoreAdd3,
                                float4 xIn, float4 yIn, float4 zIn, float4 wIn);
#if defined(__AVX__)
void INLINE_ATTRIBUTE __ocl_masked_load_transpose_float4x4(float4* pLoadAdd, float4* xOut, float4* yOut, float4* zOut, float4* wOut, int4 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_store_float4x4(float4* pStoreAdd, float4 xIn, float4 yIn, float4 zIn, float4 wIn, int4 mask);
void INLINE_ATTRIBUTE __ocl_masked_gather_transpose_float4x4(float4* pLoadAdd0, float4* pLoadAdd1, float4* pLoadAdd2, float4* pLoadAdd3,
                               float4* xOut, float4* yOut, float4* zOut, float4* wOut, int4 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_scatter_float4x4(float4* pStoreAdd0, float4* pStoreAdd1, float4* pStoreAdd2, float4* pStoreAdd3,
                                float4 xIn, float4 yIn, float4 zIn, float4 wIn, int4 mask);
#endif // defined(__AVX__)

// ****************************************************************************
//                                 float4x8
// ****************************************************************************
#if defined(__AVX__)
void INLINE_ATTRIBUTE __ocl_load_transpose_float4x8(float4* pLoadAdd, float8* xOut, float8* yOut, float8* zOut, float8* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_store_float4x8(float4* pStoreAdd, float8 xIn, float8 yIn, float8 zIn, float8 wIn);

void INLINE_ATTRIBUTE __ocl_masked_load_transpose_float4x8(float4* pLoadAdd, float8* xOut, float8* yOut, float8* zOut, float8* wOut, int8 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_store_float4x8(float4* pStoreAdd, float8 xIn, float8 yIn, float8 zIn, float8 wIn, int8 mask);

void INLINE_ATTRIBUTE __ocl_gather_transpose_float4x8(float4* pLoadAdd0, float4* pLoadAdd1, float4* pLoadAdd2, float4* pLoadAdd3,
                               float4* pLoadAdd4, float4* pLoadAdd5, float4* pLoadAdd6, float4* pLoadAdd7,
                               float8* xOut, float8* yOut, float8* zOut, float8* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_scatter_float4x8(float4* pStoreAdd0, float4* pStoreAdd1, float4* pStoreAdd2, float4* pStoreAdd3,
                                float4* pStoreAdd4, float4* pStoreAdd5, float4* pStoreAdd6, float4* pStoreAdd7,
                                float8 xIn, float8 yIn, float8 zIn, float8 wIn);

void INLINE_ATTRIBUTE __ocl_masked_gather_transpose_float4x8(float4* pLoadAdd0, float4* pLoadAdd1, float4* pLoadAdd2, float4* pLoadAdd3,
                               float4* pLoadAdd4, float4* pLoadAdd5, float4* pLoadAdd6, float4* pLoadAdd7,
                               float8* xOut, float8* yOut, float8* zOut, float8* wOut, int8 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_scatter_float4x8(float4* pStoreAdd0, float4* pStoreAdd1, float4* pStoreAdd2, float4* pStoreAdd3,
                                float4* pStoreAdd4, float4* pStoreAdd5, float4* pStoreAdd6, float4* pStoreAdd7,
                                float8 xIn, float8 yIn, float8 zIn, float8 wIn, int8 mask);

#endif // defined(__AVX__)
#endif // defined(__SSE_4_2__)
