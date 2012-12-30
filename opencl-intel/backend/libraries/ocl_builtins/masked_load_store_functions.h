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
#pragma OPENCL EXTENSION cl_khr_fp64 : enable

#if defined(__AVX__)


// ****************************************************************************
//                                 char4x4
// ****************************************************************************

void masked_load_char4x4(char4* pLoadAdd, char4* pLoadedValues, int4 mask);
void masked_store_char4x4(char4* pStoreAdd, char4* pValuesToStore, int4 mask);


// ****************************************************************************
//                                 char4x8
// ****************************************************************************

void masked_load_char4x8(char4* pLoadAdd, char4* pLoadedValues, int8 mask);
void masked_store_char4x8(char4* pStoreAdd, char4* pValuesToStore, int8 mask);

// ****************************************************************************
//                                 int4x4
// ****************************************************************************

void masked_load_int4x4(int4* pLoadAdd, int4* pLoadedValues, int4 mask);
void masked_store_int4x4(int4* pStoreAdd, int4* pValuesToStore, int4 mask);

// ****************************************************************************
//                                 int4x8
// ****************************************************************************

void masked_load_int4x8(int4* pLoadAdd, int4* pLoadedValues, int8 mask);
void masked_store_int4x8(int4* pStoreAdd, int4* pValuesToStore, int8 mask);

// ****************************************************************************
//                                 float4x4
// ****************************************************************************

void masked_load_float4x4(float4* pLoadAdd, float4* pLoadedValues, int4 mask);
void masked_store_float4x4(float4* pStoreAdd, float4* pValuesToStore, int4 mask);

// ****************************************************************************
//                                 float4x8
// ****************************************************************************

void masked_load_float4x8(float4* pLoadAdd, float4* pLoadedValues, int8 mask);
void masked_store_float4x8(float4* pStoreAdd, float4* pValuesToStore, int8 mask);

// ****************************************************************************
//                                 int4
// ****************************************************************************

int4 masked_load_int4(int4* pLoadAdd, int4 mask);
void masked_store_int4(int4* pStoreAdd, int4 data, int4 mask);

// ****************************************************************************
//                                 int8
// ****************************************************************************

int8 masked_load_int8(int8* pLoadAdd, int8 mask);
void masked_store_int8(int8* pStoreAdd, int8 data, int8 mask);

// ****************************************************************************
//                                 long4
// ****************************************************************************

long4 masked_load_long4(long4* pLoadAdd, int4 mask);
void masked_store_long4(long4* pStoreAdd, long4 data, int4 mask);

// ****************************************************************************
//                                 long8
// ****************************************************************************

long8 masked_load_long8(long8* pLoadAdd, int8 mask);
void masked_store_long8(long8* pStoreAdd, long8 data, int8 mask);

// ****************************************************************************
//                                 float4
// ****************************************************************************

float4 masked_load_float4(float4* pLoadAdd, int4 mask);
void masked_store_float4(float4* pStoreAdd, float4 data, int4 mask);

// ****************************************************************************
//                                 float8
// ****************************************************************************

float8 masked_load_float8(float8* pLoadAdd, int8 mask);
void masked_store_float8(float8* pStoreAdd, float8 data, int8 mask);

// ****************************************************************************
//                                 double4
// ****************************************************************************

double4 masked_load_double4(double4* pLoadAdd, int4 mask);
void masked_store_double4(double4* pStoreAdd, double4 data, int4 mask);

// ****************************************************************************
//                                 double8
// ****************************************************************************

double8 masked_load_double8(double8* pLoadAdd, int8 mask);
void masked_store_double8(double8* pStoreAdd, double8 data, int8 mask);

#endif // defined(__AVX__)
