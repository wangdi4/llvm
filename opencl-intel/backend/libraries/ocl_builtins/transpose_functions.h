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

#include "cl_types2.h"


#if defined(__AVX__)


// ****************************************************************************
//                                 char4x4
// ****************************************************************************

void load_transpose_char4x4(char4* pLoadAdd, char4* xOut, char4* yOut, char4* zOut, char4* wOut);
void transpose_store_char4x4(char4* pStoreAdd, char4 xIn, char4 yIn, char4 zIn, char4 wIn);

// ****************************************************************************
//                                 char4x8
// ****************************************************************************

void load_transpose_char4x8(char4* pLoadAdd, char8* xOut, char8* yOut, char8* zOut, char8* wOut);
void transpose_store_char4x8(char4* pStoreAdd, char8 xIn, char8 yIn, char8 zIn, char8 wIn);

// ****************************************************************************
//                                 int4x4
// ****************************************************************************

void load_transpose_int4x4(int4* pLoadAdd, int4* xOut, int4* yOut, int4* zOut, int4* wOut);
void transpose_store_int4x4(int4* pStoreAdd, int4 xIn, int4 yIn, int4 zIn, int4 wIn);

// ****************************************************************************
//                                 int4x8
// ****************************************************************************

void load_transpose_int4x8(int4* pLoadAdd, int8* xOut, int8* yOut, int8* zOut, int8* wOut);
void transpose_store_int4x8(int4* pStoreAdd, int8 xIn, int8 yIn, int8 zIn, int8 wIn);

// ****************************************************************************
//                                 float4x4
// ****************************************************************************

void load_transpose_float4x4(float4* pLoadAdd, float4* xOut, float4* yOut, float4* zOut, float4* wOut);
void transpose_store_float4x4(float4* pStoreAdd, float4 xIn, float4 yIn, float4 zIn, float4 wIn);

// ****************************************************************************
//                                 float4x8
// ****************************************************************************

void load_transpose_float4x8(float4* pLoadAdd, float8* xOut, float8* yOut, float8* zOut, float8* wOut);
void transpose_store_float4x8(float4* pStoreAdd, float8 xIn, float8 yIn, float8 zIn, float8 wIn);


#endif // defined(__AVX__)
