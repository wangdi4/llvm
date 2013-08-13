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

void INLINE_ATTRIBUTE __ocl_load_transpose_char4x4(private char4* pLoadAdd, private char4* xOut, private char4* yOut, private char4* zOut, private char4* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_store_char4x4(private char4* pStoreAdd, char4 xIn, char4 yIn, char4 zIn, char4 wIn);

void INLINE_ATTRIBUTE __ocl_gather_transpose_char4x4(private char4* pLoadAdd0, private char4* pLoadAdd1, private char4* pLoadAdd2, private char4* pLoadAdd3,
                              private char4* xOut, private char4* yOut, private char4* zOut, private char4* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_scatter_char4x4(private char4* pStoreAdd0, char4* pStoreAdd1, char4* pStoreAdd2, char4* pStoreAdd3,
                               char4 xIn, char4 yIn, char4 zIn, char4 wIn);

#if defined(__AVX__)
void INLINE_ATTRIBUTE __ocl_masked_load_transpose_char4x4(private char4* pLoadAdd, private char4* xOut, private char4* yOut, private char4* zOut, private char4* wOut, int4 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_store_char4x4(private char4* pStoreAdd, char4 xIn, char4 yIn, char4 zIn, char4 wIn, int4 mask);
void INLINE_ATTRIBUTE __ocl_masked_gather_transpose_char4x4(private char4* pLoadAdd0, private char4* pLoadAdd1, private char4* pLoadAdd2, private char4* pLoadAdd3,
                              private char4* xOut, private char4* yOut, private char4* zOut, private char4* wOut, int4 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_scatter_char4x4(private char4* pStoreAdd0, private char4* pStoreAdd1, private char4* pStoreAdd2, private char4* pStoreAdd3,
                               char4 xIn, char4 yIn, char4 zIn, char4 wIn, int4 mask);
#endif // defined(__AVX__)


// ****************************************************************************
//                                 char4x8
// ****************************************************************************

void INLINE_ATTRIBUTE __ocl_load_transpose_char4x8(private char4* pLoadAdd, private char8* xOut, private char8* yOut, private char8* zOut, private char8* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_store_char4x8(private char4* pStoreAdd, char8 xIn, char8 yIn, char8 zIn, char8 wIn);


void INLINE_ATTRIBUTE __ocl_gather_transpose_char4x8(private char4* pLoadAdd0, private char4* pLoadAdd1, private char4* pLoadAdd2, private char4* pLoadAdd3,
                              private char4* pLoadAdd4, private char4* pLoadAdd5, private char4* pLoadAdd6, private char4* pLoadAdd7,
                              private char8* xOut, private char8* yOut, private char8* zOut, private char8* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_scatter_char4x8(private char4* pStoreAdd0, private char4* pStoreAdd1, private char4* pStoreAdd2, private char4* pStoreAdd3,
                               private char4* pStoreAdd4, private char4* pStoreAdd5, private char4* pStoreAdd6, private char4* pStoreAdd7,
                               char8 xIn, char8 yIn, char8 zIn, char8 wIn);


#if defined(__AVX__)
void INLINE_ATTRIBUTE __ocl_masked_load_transpose_char4x8(private char4* pLoadAdd, private char8* xOut, private char8* yOut, private char8* zOut, private char8* wOut, int8 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_store_char4x8(private char4* pStoreAdd, char8 xIn, char8 yIn, char8 zIn, char8 wIn, int8 mask);

void INLINE_ATTRIBUTE __ocl_masked_gather_transpose_char4x8(private char4* pLoadAdd0, private char4* pLoadAdd1, private char4* pLoadAdd2, private char4* pLoadAdd3,
                              private char4* pLoadAdd4, private char4* pLoadAdd5, private char4* pLoadAdd6, private char4* pLoadAdd7,
                              private char8* xOut, private char8* yOut, private char8* zOut, private char8* wOut, int8 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_scatter_char4x8(private char4* pStoreAdd0, private char4* pStoreAdd1, private char4* pStoreAdd2, private char4* pStoreAdd3,
                               private char4* pStoreAdd4, private char4* pStoreAdd5, private char4* pStoreAdd6, private char4* pStoreAdd7,
                               char8 xIn, char8 yIn, char8 zIn, char8 wIn, int8 mask);
#endif // defined(__AVX__)

// ****************************************************************************
//                                 short4x8
// ****************************************************************************

#if defined(__AVX__)
void INLINE_ATTRIBUTE __ocl_load_transpose_short4x8(private short4* pLoadAdd, private short8* xOut, private short8* yOut, private short8* zOut, private short8* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_store_short4x8(short4* pStoreAdd, short8 xIn, short8 yIn, short8 zIn, short8 wIn);

void INLINE_ATTRIBUTE __ocl_gather_transpose_short4x8(private short4* pLoadAdd0, private short4* pLoadAdd1, private short4* pLoadAdd2, private short4* pLoadAdd3,
                                private short4* pLoadAdd4, private short4* pLoadAdd5, private short4* pLoadAdd6, private short4* pLoadAdd7,
                                private short8* xOut, private short8* yOut, private short8* zOut, private short8* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_scatter_short4x8(short4* pStoreAdd0, short4* pStoreAdd1, short4* pStoreAdd2, short4* pStoreAdd3,
                                private short4* pStoreAdd4, private short4* pStoreAdd5, private short4* pStoreAdd6, private short4* pStoreAdd7,
                                short8 xIn, short8 yIn, short8 zIn, short8 wIn);

void INLINE_ATTRIBUTE __ocl_masked_load_transpose_short4x8(private short4* pLoadAdd, private short8* xOut, private short8* yOut, private short8* zOut, private short8* wOut, int8 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_store_short4x8(private short4* pStoreAdd, short8 xIn, short8 yIn, short8 zIn, short8 wIn, int8 mask);

void INLINE_ATTRIBUTE __ocl_masked_gather_transpose_short4x8(private short4* pLoadAdd0, private short4* pLoadAdd1, private short4* pLoadAdd2, private short4* pLoadAdd3,
                                private short4* pLoadAdd4, private short4* pLoadAdd5, private short4* pLoadAdd6, private short4* pLoadAdd7,
                                private short8* xOut, private short8* yOut, private short8* zOut, private short8* wOut, int8 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_scatter_short4x8(private short4* pStoreAdd0, private short4* pStoreAdd1, private short4* pStoreAdd2, private short4* pStoreAdd3,
                                private short4* pStoreAdd4, private short4* pStoreAdd5, private short4* pStoreAdd6, private short4* pStoreAdd7,
                                short8 xIn, short8 yIn, short8 zIn, short8 wIn, int8 mask);
#endif // defined(__AVX__)

// ****************************************************************************
//                                 int4x4
// ****************************************************************************

void INLINE_ATTRIBUTE __ocl_load_transpose_int4x4(private int4* pLoadAdd, private int4* xOut, private int4* yOut, private int4* zOut, private int4* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_store_int4x4(int4* pStoreAdd, int4 xIn, int4 yIn, int4 zIn, int4 wIn);

void INLINE_ATTRIBUTE __ocl_gather_transpose_int4x4(private int4* pLoadAdd0, private int4* pLoadAdd1, private int4* pLoadAdd2, private int4* pLoadAdd3,
                             private int4* xOut, private int4* yOut, private int4* zOut, private int4* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_scatter_int4x4(private int4* pStoreAdd0, private int4* pStoreAdd1, private int4* pStoreAdd2, private int4* pStoreAdd3,
                              int4 xIn, int4 yIn, int4 zIn, int4 wIn);

#if defined(__AVX__)
void INLINE_ATTRIBUTE __ocl_masked_load_transpose_int4x4(private int4* pLoadAdd, private int4* xOut, private int4* yOut, private int4* zOut, private int4* wOut, int4 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_store_int4x4(private int4* pStoreAdd, int4 xIn, int4 yIn, int4 zIn, int4 wIn, int4 mask);

void INLINE_ATTRIBUTE __ocl_masked_gather_transpose_int4x4(private int4* pLoadAdd0, private int4* pLoadAdd1, private int4* pLoadAdd2, private int4* pLoadAdd3,
                             int4* xOut, int4* yOut, int4* zOut, int4* wOut, int4 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_scatter_int4x4(private int4* pStoreAdd0, private int4* pStoreAdd1, private int4* pStoreAdd2, private int4* pStoreAdd3,
                              int4 xIn, int4 yIn, int4 zIn, int4 wIn, int4 mask);
#endif // defined(__AVX__)

// ****************************************************************************
//                                 int4x8
// ****************************************************************************

#if defined(__AVX__)
void INLINE_ATTRIBUTE __ocl_load_transpose_int4x8(private int4* pLoadAdd, private int8* xOut, private int8* yOut, private int8* zOut, private int8* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_store_int4x8(int4* pStoreAdd, int8 xIn, int8 yIn, int8 zIn, int8 wIn);

void INLINE_ATTRIBUTE __ocl_masked_load_transpose_int4x8(private int4* pLoadAdd, private int8* xOut, private int8* yOut, private int8* zOut, private int8* wOut, int8 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_store_int4x8(private int4* pStoreAdd, int8 xIn, int8 yIn, int8 zIn, int8 wIn, int8 mask);

void INLINE_ATTRIBUTE __ocl_gather_transpose_int4x8(private int4* pLoadAdd0, private int4* pLoadAdd1, private int4* pLoadAdd2, private int4* pLoadAdd3,
                             private int4* pLoadAdd4, private int4* pLoadAdd5, private int4* pLoadAdd6, private int4* pLoadAdd7,
                             private int8* xOut, private int8* yOut, private int8* zOut, private int8* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_scatter_int4x8(private int4* pStoreAdd0, private int4* pStoreAdd1, private int4* pStoreAdd2, private int4* pStoreAdd3,
                              private int4* pStoreAdd4, private int4* pStoreAdd5, private int4* pStoreAdd6, private int4* pStoreAdd7,
                              int8 xIn, int8 yIn, int8 zIn, int8 wIn);

void INLINE_ATTRIBUTE __ocl_masked_gather_transpose_int4x8(private int4* pLoadAdd0, private int4* pLoadAdd1, private int4* pLoadAdd2, private int4* pLoadAdd3,
                             private int4* pLoadAdd4, private int4* pLoadAdd5, private int4* pLoadAdd6, private int4* pLoadAdd7,
                             private int8* xOut, private int8* yOut, private int8* zOut, private int8* wOut, int8 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_scatter_int4x8(private int4* pStoreAdd0, private int4* pStoreAdd1, private int4* pStoreAdd2, private int4* pStoreAdd3,
                              private int4* pStoreAdd4, private int4* pStoreAdd5, private int4* pStoreAdd6, private int4* pStoreAdd7,
                              int8 xIn, int8 yIn, int8 zIn, int8 wIn, int8 mask);
#endif // __AVX__

// ****************************************************************************
//                                 float4x4
// ****************************************************************************

void INLINE_ATTRIBUTE __ocl_load_transpose_float4x4(private float4* pLoadAdd, private float4* xOut, private float4* yOut, private float4* zOut, private float4* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_store_float4x4(private float4* pStoreAdd, float4 xIn, float4 yIn, float4 zIn, float4 wIn);

void INLINE_ATTRIBUTE __ocl_gather_transpose_float4x4(private float4* pLoadAdd0, private float4* pLoadAdd1, private float4* pLoadAdd2, private float4* pLoadAdd3,
                               private float4* xOut, private float4* yOut, private float4* zOut, private float4* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_scatter_float4x4(private float4* pStoreAdd0, private float4* pStoreAdd1, private float4* pStoreAdd2, private float4* pStoreAdd3,
                                float4 xIn, float4 yIn, float4 zIn, float4 wIn);
#if defined(__AVX__)
void INLINE_ATTRIBUTE __ocl_masked_load_transpose_float4x4(private float4* pLoadAdd, private float4* xOut, private float4* yOut, private float4* zOut, private float4* wOut, int4 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_store_float4x4(private float4* pStoreAdd, float4 xIn, float4 yIn, float4 zIn, float4 wIn, int4 mask);
void INLINE_ATTRIBUTE __ocl_masked_gather_transpose_float4x4(private float4* pLoadAdd0, private float4* pLoadAdd1, private float4* pLoadAdd2, private float4* pLoadAdd3,
                               private float4* xOut, private float4* yOut, private float4* zOut, private float4* wOut, int4 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_scatter_float4x4(private float4* pStoreAdd0, private float4* pStoreAdd1, private float4* pStoreAdd2, private float4* pStoreAdd3,
                                float4 xIn, float4 yIn, float4 zIn, float4 wIn, int4 mask);
#endif // defined(__AVX__)

// ****************************************************************************
//                                 float4x8
// ****************************************************************************
#if defined(__AVX__)
void INLINE_ATTRIBUTE __ocl_load_transpose_float4x8(private float4* pLoadAdd, private float8* xOut, private float8* yOut, private float8* zOut, private float8* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_store_float4x8(private float4* pStoreAdd, float8 xIn, float8 yIn, float8 zIn, float8 wIn);

void INLINE_ATTRIBUTE __ocl_masked_load_transpose_float4x8(private float4* pLoadAdd, private float8* xOut, private float8* yOut, private float8* zOut, private float8* wOut, int8 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_store_float4x8(private float4* pStoreAdd, float8 xIn, float8 yIn, float8 zIn, float8 wIn, int8 mask);

void INLINE_ATTRIBUTE __ocl_gather_transpose_float4x8(private float4* pLoadAdd0, private float4* pLoadAdd1, private float4* pLoadAdd2, private float4* pLoadAdd3,
                               private float4* pLoadAdd4, private float4* pLoadAdd5, private float4* pLoadAdd6, private float4* pLoadAdd7,
                               private float8* xOut, private float8* yOut, private float8* zOut, private float8* wOut);
void INLINE_ATTRIBUTE __ocl_transpose_scatter_float4x8(private float4* pStoreAdd0, private float4* pStoreAdd1, private float4* pStoreAdd2, private float4* pStoreAdd3,
                                private float4* pStoreAdd4, private float4* pStoreAdd5, private float4* pStoreAdd6, private float4* pStoreAdd7,
                                float8 xIn, float8 yIn, float8 zIn, float8 wIn);

void INLINE_ATTRIBUTE __ocl_masked_gather_transpose_float4x8(private float4* pLoadAdd0, private float4* pLoadAdd1, private float4* pLoadAdd2, private float4* pLoadAdd3,
                               private float4* pLoadAdd4, private float4* pLoadAdd5, private float4* pLoadAdd6, private float4* pLoadAdd7,
                               private float8* xOut, private float8* yOut, private float8* zOut, private float8* wOut, int8 mask);
void INLINE_ATTRIBUTE __ocl_masked_transpose_scatter_float4x8(private float4* pStoreAdd0, private float4* pStoreAdd1, private float4* pStoreAdd2, private float4* pStoreAdd3,
                                private float4* pStoreAdd4, private float4* pStoreAdd5, private float4* pStoreAdd6, private float4* pStoreAdd7,
                                float8 xIn, float8 yIn, float8 zIn, float8 wIn, int8 mask);

#endif // defined(__AVX__)
#endif // defined(__SSE4_2__)

#if !defined(__AVX__)
#if !defined(__SSE4_2__) 
void __ocl_transpose_char4x4(char4 xyzw0, char4 xyzw1, char4 xyzw2, char4 xyzw3,
                              private char4* xOut, private char4* yOut, private char4* zOut, private char4* wOut);

void __inline__ __attribute__((always_inline)) __ocl_transpose_char4x8(char4 xyzw0, char4 xyzw1, char4 xyzw2, char4 xyzw3,
                              char4 xyzw4, char4 xyzw5, char4 xyzw6, char4 xyzw7,
                              private char8* xOut, private char8* yOut, private char8* zOut, private char8* wOut);

void __inline__ __attribute__((always_inline)) __ocl_transpose_char8x4( char8 xIn, char8 yIn, char8 zIn, char8 wIn,
                              private char4* xyzw0, private char4* xyzw1, private char4* xyzw2, private char4* xyzw3,
                              private char4* xyzw4, private char4* xyzw5, private char4* xyzw6, private char4* xyzw7);

void __inline__ __attribute__((always_inline)) __ocl_gather_transpose_char4x4(private char4* pLoadAdd0, private char4* pLoadAdd1, private char4* pLoadAdd2, private char4* pLoadAdd3,
                              private char4* xOut, private char4* yOut, private char4* zOut, private char4* wOut);

void __inline__ __attribute__((always_inline)) __ocl_transpose_scatter_char4x4(private char4* pStoreAdd0, private char4* pStoreAdd1, private char4* pStoreAdd2, private char4* pStoreAdd3,
                               char4 xIn, char4 yIn, char4 zIn, char4 wIn);

void __inline__ __attribute__((always_inline)) __ocl_gather_transpose_char4x8(private char4* pLoadAdd0, private char4* pLoadAdd1, private char4* pLoadAdd2, private char4* pLoadAdd3,
                              private char4* pLoadAdd4, private char4* pLoadAdd5, private char4* pLoadAdd6, private char4* pLoadAdd7,
                              private char8* xOut, private char8* yOut, private char8* zOut, private char8* wOut);

void __ocl_transpose_scatter_char4x8(private char4* pStoreAdd0, private char4* pStoreAdd1, private char4* pStoreAdd2, private char4* pStoreAdd3,
                               private char4* pStoreAdd4, private char4* pStoreAdd5, private char4* pStoreAdd6, private char4* pStoreAdd7,
                               char8 xIn, char8 yIn, char8 zIn, char8 wIn);
#endif

void __ocl_masked_gather_transpose_char4x4(private char4* pLoadAdd0, private char4* pLoadAdd1, private char4* pLoadAdd2, private char4* pLoadAdd3,
                              private char4* xOut, private char4* yOut, private char4* zOut, private char4* wOut, int4 mask);

void __ocl_masked_gather_transpose_char4x8(private char4* pLoadAdd0, private char4* pLoadAdd1, private char4* pLoadAdd2, private char4* pLoadAdd3,
                              private char4* pLoadAdd4, private char4* pLoadAdd5, private char4* pLoadAdd6, private char4* pLoadAdd7,
                              private char8* xOut, private char8* yOut, private char8* zOut, private char8* wOut, int8 mask);
#endif // __AVX__
