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

// Naming style:
// <transpose type>_<original trype in user kernel> x <packet width>
// Example: In the original kernel the user used char4, the packet width is 8
//          and the operation is load and transpose, the function name will be:
// load_transpose_char_4x8(...)


// ****************************************************************************
//                                 char_4x4
// ****************************************************************************

void INLINE_ATTRIBUTE  __ocl_load_transpose_char_4x4(__private char4* pLoadAdd, __private char4* xOut, __private char4* yOut, __private char4* zOut, __private char4* wOut);
void INLINE_ATTRIBUTE  __ocl_transpose_store_char_4x4(__private char4* pStoreAdd, char4 xIn, char4 yIn, char4 zIn, char4 wIn);

void INLINE_ATTRIBUTE  __ocl_gather_transpose_char_4x4(__private char4* pLoadAdd0, __private char4* pLoadAdd1, __private char4* pLoadAdd2, __private char4* pLoadAdd3,
                              __private char4* xOut, __private char4* yOut, __private char4* zOut, __private char4* wOut);
void INLINE_ATTRIBUTE  __ocl_transpose_scatter_char_4x4(__private char4* pStoreAdd0, __private char4* pStoreAdd1, __private char4* pStoreAdd2, __private char4* pStoreAdd3,
                               char4 xIn, char4 yIn, char4 zIn, char4 wIn);


void INLINE_ATTRIBUTE  __ocl_masked_load_transpose_char_4x4(__private char4* pLoadAdd, __private char4* xOut, __private char4* yOut, __private char4* zOut, __private char4* wOut, int4 mask);
void INLINE_ATTRIBUTE  __ocl_masked_transpose_store_char_4x4(__private char4* pStoreAdd, char4 xIn, char4 yIn, char4 zIn, char4 wIn, int4 mask);
void INLINE_ATTRIBUTE  __ocl_masked_gather_transpose_char_4x4(__private char4* pLoadAdd0, __private char4* pLoadAdd1, __private char4* pLoadAdd2, __private char4* pLoadAdd3,
                              __private char4* xOut, __private char4* yOut, __private char4* zOut, __private char4* wOut, int4 mask);
void INLINE_ATTRIBUTE  __ocl_masked_transpose_scatter_char_4x4(__private char4* pStoreAdd0, __private char4* pStoreAdd1, __private char4* pStoreAdd2, __private char4* pStoreAdd3,
                               char4 xIn, char4 yIn, char4 zIn, char4 wIn, int4 mask);



// ****************************************************************************
//                                 char_4x8
// ****************************************************************************

void INLINE_ATTRIBUTE  __ocl_load_transpose_char_4x8(__private char4* pLoadAdd, __private char8* xOut, __private char8* yOut, __private char8* zOut, __private char8* wOut);
void INLINE_ATTRIBUTE  __ocl_transpose_store_char_4x8(__private char4* pStoreAdd, char8 xIn, char8 yIn, char8 zIn, char8 wIn);


void INLINE_ATTRIBUTE  __ocl_gather_transpose_char_4x8(__private char4* pLoadAdd0, __private char4* pLoadAdd1, __private char4* pLoadAdd2, __private char4* pLoadAdd3,
                              __private char4* pLoadAdd4, __private char4* pLoadAdd5, __private char4* pLoadAdd6, __private char4* pLoadAdd7,
                              __private char8* xOut, __private char8* yOut, __private char8* zOut, __private char8* wOut);
void INLINE_ATTRIBUTE  __ocl_transpose_scatter_char_4x8(__private char4* pStoreAdd0, __private char4* pStoreAdd1, __private char4* pStoreAdd2, __private char4* pStoreAdd3,
                               __private char4* pStoreAdd4, __private char4* pStoreAdd5, __private char4* pStoreAdd6, __private char4* pStoreAdd7,
                               char8 xIn, char8 yIn, char8 zIn, char8 wIn);


void INLINE_ATTRIBUTE  __ocl_masked_load_transpose_char_4x8(__private char4* pLoadAdd, __private char8* xOut, __private char8* yOut, __private char8* zOut, __private char8* wOut, int8 mask);
void INLINE_ATTRIBUTE  __ocl_masked_transpose_store_char_4x8(__private char4* pStoreAdd, char8 xIn, char8 yIn, char8 zIn, char8 wIn, int8 mask);

void INLINE_ATTRIBUTE  __ocl_masked_gather_transpose_char_4x8(__private char4* pLoadAdd0, __private char4* pLoadAdd1, __private char4* pLoadAdd2, __private char4* pLoadAdd3,
                              __private char4* pLoadAdd4, __private char4* pLoadAdd5, __private char4* pLoadAdd6, __private char4* pLoadAdd7,
                              __private char8* xOut, __private char8* yOut, __private char8* zOut, __private char8* wOut, int8 mask);
void INLINE_ATTRIBUTE  __ocl_masked_transpose_scatter_char_4x8(__private char4* pStoreAdd0, __private char4* pStoreAdd1, __private char4* pStoreAdd2, __private char4* pStoreAdd3,
                               __private char4* pStoreAdd4, __private char4* pStoreAdd5, __private char4* pStoreAdd6, __private char4* pStoreAdd7,
                               char8 xIn, char8 yIn, char8 zIn, char8 wIn, int8 mask);


// ****************************************************************************
//                                 short_4x8
// ****************************************************************************

void INLINE_ATTRIBUTE  __ocl_load_transpose_short_4x8(__private short4* pLoadAdd, __private short8* xOut, __private short8* yOut, __private short8* zOut, __private short8* wOut);
void INLINE_ATTRIBUTE  __ocl_transpose_store_short_4x8(__private short4* pStoreAdd, short8 xIn, short8 yIn, short8 zIn, short8 wIn);

void INLINE_ATTRIBUTE  __ocl_gather_transpose_short_4x8(__private short4* pLoadAdd0, __private short4* pLoadAdd1, __private short4* pLoadAdd2, __private short4* pLoadAdd3,
                                __private short4* pLoadAdd4, __private short4* pLoadAdd5, __private short4* pLoadAdd6, __private short4* pLoadAdd7,
                                __private short8* xOut, __private short8* yOut, __private short8* zOut, __private short8* wOut);
void INLINE_ATTRIBUTE  __ocl_transpose_scatter_short_4x8(__private short4* pStoreAdd0, __private short4* pStoreAdd1, __private short4* pStoreAdd2, __private short4* pStoreAdd3,
                                __private short4* pStoreAdd4, __private short4* pStoreAdd5, __private short4* pStoreAdd6, __private short4* pStoreAdd7,
                                short8 xIn, short8 yIn, short8 zIn, short8 wIn);

void INLINE_ATTRIBUTE  __ocl_masked_load_transpose_short_4x8(__private short4* pLoadAdd, __private short8* xOut, __private short8* yOut, __private short8* zOut, __private short8* wOut, int8 mask);
void INLINE_ATTRIBUTE  __ocl_masked_transpose_store_short_4x8(__private short4* pStoreAdd, short8 xIn, short8 yIn, short8 zIn, short8 wIn, int8 mask);

void INLINE_ATTRIBUTE  __ocl_masked_gather_transpose_short_4x8(__private short4* pLoadAdd0, __private short4* pLoadAdd1, __private short4* pLoadAdd2, __private short4* pLoadAdd3,
                                __private short4* pLoadAdd4, __private short4* pLoadAdd5, __private short4* pLoadAdd6, __private short4* pLoadAdd7,
                                __private short8* xOut, __private short8* yOut, __private short8* zOut, __private short8* wOut, int8 mask);
void INLINE_ATTRIBUTE  __ocl_masked_transpose_scatter_short_4x8(__private short4* pStoreAdd0, __private short4* pStoreAdd1, __private short4* pStoreAdd2, __private short4* pStoreAdd3,
                                __private short4* pStoreAdd4, __private short4* pStoreAdd5, __private short4* pStoreAdd6, __private short4* pStoreAdd7,
                                short8 xIn, short8 yIn, short8 zIn, short8 wIn, int8 mask);


// ****************************************************************************
//                                 int_4x4
// ****************************************************************************

void INLINE_ATTRIBUTE  __ocl_load_transpose_int_4x4(__private int4* pLoadAdd, __private int4* xOut, __private int4* yOut, __private int4* zOut, __private int4* wOut);
void INLINE_ATTRIBUTE  __ocl_transpose_store_int_4x4(__private int4* pStoreAdd, int4 xIn, int4 yIn, int4 zIn, int4 wIn);

void INLINE_ATTRIBUTE  __ocl_gather_transpose_int_4x4(__private int4* pLoadAdd0, __private int4* pLoadAdd1, __private int4* pLoadAdd2, __private int4* pLoadAdd3,
                             __private int4* xOut, __private int4* yOut, __private int4* zOut, __private int4* wOut);
void INLINE_ATTRIBUTE  __ocl_transpose_scatter_int_4x4(__private int4* pStoreAdd0, __private int4* pStoreAdd1, __private int4* pStoreAdd2, __private int4* pStoreAdd3,
                              int4 xIn, int4 yIn, int4 zIn, int4 wIn);


void INLINE_ATTRIBUTE  __ocl_masked_load_transpose_int_4x4(__private int4* pLoadAdd, __private int4* xOut, __private int4* yOut, __private int4* zOut, __private int4* wOut, int4 mask);
void INLINE_ATTRIBUTE  __ocl_masked_transpose_store_int_4x4(__private int4* pStoreAdd, int4 xIn, int4 yIn, int4 zIn, int4 wIn, int4 mask);

void INLINE_ATTRIBUTE  __ocl_masked_gather_transpose_int_4x4(__private int4* pLoadAdd0, __private int4* pLoadAdd1, __private int4* pLoadAdd2, __private int4* pLoadAdd3,
                             __private int4* xOut, __private int4* yOut, __private int4* zOut, __private int4* wOut, int4 mask);
void INLINE_ATTRIBUTE  __ocl_masked_transpose_scatter_int_4x4(__private int4* pStoreAdd0, __private int4* pStoreAdd1, __private int4* pStoreAdd2, __private int4* pStoreAdd3,
                              int4 xIn, int4 yIn, int4 zIn, int4 wIn, int4 mask);


// ****************************************************************************
//                                 int_4x8
// ****************************************************************************

void INLINE_ATTRIBUTE  __ocl_load_transpose_int_4x8(__private int4* pLoadAdd, __private int8* xOut, __private int8* yOut, __private int8* zOut, __private int8* wOut);
void INLINE_ATTRIBUTE  __ocl_transpose_store_int_4x8(__private int4* pStoreAdd, int8 xIn, int8 yIn, int8 zIn, int8 wIn);

void INLINE_ATTRIBUTE  __ocl_masked_load_transpose_int_4x8(__private int4* pLoadAdd, __private int8* xOut, __private int8* yOut, __private int8* zOut, __private int8* wOut, int8 mask);
void INLINE_ATTRIBUTE  __ocl_masked_transpose_store_int_4x8(__private int4* pStoreAdd, int8 xIn, int8 yIn, int8 zIn, int8 wIn, int8 mask);

void INLINE_ATTRIBUTE  __ocl_gather_transpose_int_4x8(__private int4* pLoadAdd0, __private int4* pLoadAdd1, __private int4* pLoadAdd2, __private int4* pLoadAdd3,
                             __private int4* pLoadAdd4, __private int4* pLoadAdd5, __private int4* pLoadAdd6, __private int4* pLoadAdd7,
                             __private int8* xOut, __private int8* yOut, __private int8* zOut, __private int8* wOut);
void INLINE_ATTRIBUTE  __ocl_transpose_scatter_int_4x8(__private int4* pStoreAdd0, __private int4* pStoreAdd1, __private int4* pStoreAdd2, __private int4* pStoreAdd3,
                              __private int4* pStoreAdd4, __private int4* pStoreAdd5, __private int4* pStoreAdd6, __private int4* pStoreAdd7,
                              int8 xIn, int8 yIn, int8 zIn, int8 wIn);

void INLINE_ATTRIBUTE  __ocl_masked_gather_transpose_int_4x8(__private int4* pLoadAdd0, __private int4* pLoadAdd1, __private int4* pLoadAdd2, __private int4* pLoadAdd3,
                             __private int4* pLoadAdd4, __private int4* pLoadAdd5, __private int4* pLoadAdd6, __private int4* pLoadAdd7,
                             __private int8* xOut, __private int8* yOut, __private int8* zOut, __private int8* wOut, int8 mask);
void INLINE_ATTRIBUTE  __ocl_masked_transpose_scatter_int_4x8(__private int4* pStoreAdd0, __private int4* pStoreAdd1, __private int4* pStoreAdd2, __private int4* pStoreAdd3,
                              __private int4* pStoreAdd4, __private int4* pStoreAdd5, __private int4* pStoreAdd6, __private int4* pStoreAdd7,
                              int8 xIn, int8 yIn, int8 zIn, int8 wIn, int8 mask);


// ****************************************************************************
//                                 float_4x4
// ****************************************************************************

void INLINE_ATTRIBUTE  __ocl_load_transpose_float_4x4(__private float4* pLoadAdd, __private float4* xOut, __private float4* yOut, __private float4* zOut, __private float4* wOut);
void INLINE_ATTRIBUTE  __ocl_transpose_store_float_4x4(__private float4* pStoreAdd, float4 xIn, float4 yIn, float4 zIn, float4 wIn);

void INLINE_ATTRIBUTE  __ocl_gather_transpose_float_4x4(__private float4* pLoadAdd0, __private float4* pLoadAdd1, __private float4* pLoadAdd2, __private float4* pLoadAdd3,
                               __private float4* xOut, __private float4* yOut, __private float4* zOut, __private float4* wOut);
void INLINE_ATTRIBUTE  __ocl_transpose_scatter_float_4x4(__private float4* pStoreAdd0, __private float4* pStoreAdd1, __private float4* pStoreAdd2, __private float4* pStoreAdd3,
                                float4 xIn, float4 yIn, float4 zIn, float4 wIn);

void INLINE_ATTRIBUTE  __ocl_masked_load_transpose_float_4x4(__private float4* pLoadAdd, __private float4* xOut, __private float4* yOut, __private float4* zOut, __private float4* wOut, int4 mask);
void INLINE_ATTRIBUTE  __ocl_masked_transpose_store_float_4x4(__private float4* pStoreAdd, float4 xIn, float4 yIn, float4 zIn, float4 wIn, int4 mask);
void INLINE_ATTRIBUTE  __ocl_masked_gather_transpose_float_4x4(__private float4* pLoadAdd0, __private float4* pLoadAdd1, __private float4* pLoadAdd2, __private float4* pLoadAdd3,
                               __private float4* xOut, __private float4* yOut, __private float4* zOut, __private float4* wOut, int4 mask);
void INLINE_ATTRIBUTE  __ocl_masked_transpose_scatter_float_4x4(__private float4* pStoreAdd0, __private float4* pStoreAdd1, __private float4* pStoreAdd2, __private float4* pStoreAdd3,
                                float4 xIn, float4 yIn, float4 zIn, float4 wIn, int4 mask);


// ****************************************************************************
//                                 float_4x8
// ****************************************************************************

void INLINE_ATTRIBUTE  __ocl_load_transpose_float_4x8(__private float4* pLoadAdd, __private float8* xOut, __private float8* yOut, __private float8* zOut, __private float8* wOut);
void INLINE_ATTRIBUTE  __ocl_transpose_store_float_4x8(__private float4* pStoreAdd, float8 xIn, float8 yIn, float8 zIn, float8 wIn);

void INLINE_ATTRIBUTE  __ocl_masked_load_transpose_float_4x8(__private float4* pLoadAdd, __private float8* xOut, __private float8* yOut, __private float8* zOut, __private float8* wOut, int8 mask);
void INLINE_ATTRIBUTE  __ocl_masked_transpose_store_float_4x8(__private float4* pStoreAdd, float8 xIn, float8 yIn, float8 zIn, float8 wIn, int8 mask);

void INLINE_ATTRIBUTE  __ocl_gather_transpose_float_4x8(__private float4* pLoadAdd0, __private float4* pLoadAdd1, __private float4* pLoadAdd2, __private float4* pLoadAdd3,
                               __private float4* pLoadAdd4, __private float4* pLoadAdd5, __private float4* pLoadAdd6, __private float4* pLoadAdd7,
                               __private float8* xOut, __private float8* yOut, __private float8* zOut, __private float8* wOut);
void INLINE_ATTRIBUTE  __ocl_transpose_scatter_float_4x8(__private float4* pStoreAdd0, __private float4* pStoreAdd1, __private float4* pStoreAdd2, __private float4* pStoreAdd3,
                                __private float4* pStoreAdd4, __private float4* pStoreAdd5, __private float4* pStoreAdd6, __private float4* pStoreAdd7,
                                float8 xIn, float8 yIn, float8 zIn, float8 wIn);

void INLINE_ATTRIBUTE  __ocl_masked_gather_transpose_float_4x8(__private float4* pLoadAdd0, __private float4* pLoadAdd1, __private float4* pLoadAdd2, __private float4* pLoadAdd3,
                               __private float4* pLoadAdd4, __private float4* pLoadAdd5, __private float4* pLoadAdd6, __private float4* pLoadAdd7,
                               __private float8* xOut, __private float8* yOut, __private float8* zOut, __private float8* wOut, int8 mask);
void INLINE_ATTRIBUTE  __ocl_masked_transpose_scatter_float_4x8(__private float4* pStoreAdd0, __private float4* pStoreAdd1, __private float4* pStoreAdd2, __private float4* pStoreAdd3,
                                __private float4* pStoreAdd4, __private float4* pStoreAdd5, __private float4* pStoreAdd6, __private float4* pStoreAdd7,
                                float8 xIn, float8 yIn, float8 zIn, float8 wIn, int8 mask);
