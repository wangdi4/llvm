// Copyright (c) 2006-2007 Intel Corporation
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

// Define function generic prototypes
#define DEF_GEOM_PROTO_F_F(FUNC)

#define DEF_GEOM_PROTO_D_D(FUNC)

#define DEF_GEOM_PROTO_F2_F(FUNC) \
	float __##FUNC##f2as4(float4 x);\
	float __attribute__((overloadable)) FUNC(float2 x)\
	{\
        float4	tempX = {0};\
		tempX.lo = x;\
		return __##FUNC##f2as4(tempX);\
	}

#define DEF_GEOM_PROTO_D2_D(FUNC)

#define DEF_GEOM_PROTO_F3_F(FUNC) \
	float __attribute__((overloadable)) FUNC(float3 x)\
	{\
		float4	tempX = {0};\
		tempX.s012 = x;\
		return FUNC(tempX);\
	}

#define DEF_GEOM_PROTO_D3_D(FUNC) \
	double __attribute__((overloadable)) FUNC(double3 x)\
	{\
		double4	tempX = {0};\
		tempX.s012 = x;\
		return FUNC(tempX);\
	}

#define DEF_GEOM_PROTO_F4_F(FUNC)

#define DEF_GEOM_PROTO_D4_D(FUNC)

#define DEF_GEOM_PROTO_F_F_F(FUNC)

#define DEF_GEOM_PROTO_D_D_D(FUNC)

#define DEF_GEOM_PROTO_F2_F2_F(FUNC) \
	float __##FUNC##f2as4(float4 x, float4 y);\
	float __attribute__((overloadable)) FUNC(float2 x, float2 y)\
	{\
		float4	tempX = {0};\
		tempX.lo = x;\
		float4	tempY = {0};\
		tempY.lo = y;\
		return __##FUNC##f2as4(tempX, tempY);\
	}

#define DEF_GEOM_PROTO_D2_D2_D(FUNC)

#define DEF_GEOM_PROTO_F3_F3_F(FUNC) \
	float __attribute__((overloadable)) FUNC(float3 x, float3 y)\
	{\
		float4	tempX = {0};\
		float4	tempY = {0};\
		tempX.s012 = x;\
		tempY.s012 = y;\
		return FUNC(tempX, tempY);\
	}

#define DEF_GEOM_PROTO_D3_D3_D(FUNC) \
	double __attribute__((overloadable)) FUNC(double3 x, double3 y)\
	{\
		double4	tempX = {0};\
		double4	tempY = {0};\
		tempX.s012 = x;\
		tempY.s012 = y;\
		return FUNC(tempX, tempY);\
	}

#define DEF_GEOM_PROTO_F4_F4_F(FUNC)

#define DEF_GEOM_PROTO_D4_D4_D(FUNC)

#define DEF_GEOM_PROTO_F2_F2(FUNC) \
	float4 __##FUNC##f2as4(float4 x);\
	float2 __attribute__((overloadable)) FUNC(float2 x)\
	{\
		float4	tempX = {0};\
		tempX.lo = x;\
		tempX = __##FUNC##f2as4(tempX);\
		return tempX.lo;\
	}

#define DEF_GEOM_PROTO_D2_D2(FUNC)

#define DEF_GEOM_PROTO_F3_F3(FUNC) \
	float3 __attribute__((overloadable)) FUNC(float3 x)\
	{\
		float4	tempX = {0};\
		float4	res;\
		tempX.s012 = x;\
		res = FUNC(tempX);\
		return res.s012;\
	}

#define DEF_GEOM_PROTO_D3_D3(FUNC) \
	double3 __attribute__((overloadable)) FUNC(double3 x)\
	{\
		double4	tempX = {0};\
		double4	res;\
		tempX.s012 = x;\
		res = FUNC(tempX);\
		return res.s012;\
	}

#define DEF_GEOM_PROTO_F4_F4(FUNC)

#define DEF_GEOM_PROTO_D4_D4(FUNC)

#define DEF_GEOM_PROTO_F3_F3_F3(FUNC)\
	float3 __attribute__((overloadable)) FUNC(float3 x, float3 y)\
	{\
		float4	tempX = {0};\
		float4	tempY = {0};\
		float4	res;\
		tempX.s012 = x;\
		tempY.s012 = y;\
		res = FUNC(tempX, tempY);\
		return res.s012;\
	}

#define DEF_GEOM_PROTO_D3_D3_D3(FUNC)\
	double3 __attribute__((overloadable)) FUNC(double3 x, double3 y)\
	{\
		double4	tempX = {0};\
		double4	tempY = {0};\
		double4	res;\
		tempX.s012 = x;\
		tempY.s012 = y;\
		res = FUNC(tempX, tempY);\
		return res.s012;\
	}

#define DEF_GEOM_PROTO_F4_F4_F4(FUNC)

#define DEF_GEOM_PROTO_D4_D4_D4(FUNC)


DEF_GEOM_PROTO_F_F(normalize)
DEF_GEOM_PROTO_F2_F2(normalize)
DEF_GEOM_PROTO_F3_F3(normalize)
DEF_GEOM_PROTO_F4_F4(normalize)

DEF_GEOM_PROTO_D_D(normalize)
DEF_GEOM_PROTO_D2_D2(normalize)
DEF_GEOM_PROTO_D3_D3(normalize)
DEF_GEOM_PROTO_D4_D4(normalize)

DEF_GEOM_PROTO_F_F(fast_normalize)
DEF_GEOM_PROTO_F2_F2(fast_normalize)
DEF_GEOM_PROTO_F3_F3(fast_normalize)
DEF_GEOM_PROTO_F4_F4(fast_normalize)

DEF_GEOM_PROTO_F_F(length)
DEF_GEOM_PROTO_F2_F(length)
DEF_GEOM_PROTO_F3_F(length)
DEF_GEOM_PROTO_F4_F(length)

DEF_GEOM_PROTO_D_D(length)
DEF_GEOM_PROTO_D2_D(length)
DEF_GEOM_PROTO_D3_D(length)
DEF_GEOM_PROTO_D4_D(length)

DEF_GEOM_PROTO_F_F(fast_length)
DEF_GEOM_PROTO_F2_F(fast_length)
DEF_GEOM_PROTO_F3_F(fast_length)
DEF_GEOM_PROTO_F4_F(fast_length)

DEF_GEOM_PROTO_F_F_F(distance)
DEF_GEOM_PROTO_F2_F2_F(distance)
DEF_GEOM_PROTO_F3_F3_F(distance)
DEF_GEOM_PROTO_F4_F4_F(distance)

DEF_GEOM_PROTO_D_D_D(distance)
DEF_GEOM_PROTO_D2_D2_D(distance)
DEF_GEOM_PROTO_D3_D3_D(distance)
DEF_GEOM_PROTO_D4_D4_D(distance)

DEF_GEOM_PROTO_F_F_F(fast_distance)
DEF_GEOM_PROTO_F2_F2_F(fast_distance)
DEF_GEOM_PROTO_F3_F3_F(fast_distance)
DEF_GEOM_PROTO_F4_F4_F(fast_distance)

DEF_GEOM_PROTO_F_F_F(dot)
DEF_GEOM_PROTO_F2_F2_F(dot)
DEF_GEOM_PROTO_F3_F3_F(dot)
DEF_GEOM_PROTO_F4_F4_F(dot)

DEF_GEOM_PROTO_D_D_D(dot)
DEF_GEOM_PROTO_D2_D2_D(dot)
DEF_GEOM_PROTO_D3_D3_D(dot)
DEF_GEOM_PROTO_D4_D4_D(dot)

DEF_GEOM_PROTO_F3_F3_F3(cross)
DEF_GEOM_PROTO_F4_F4_F4(cross)

DEF_GEOM_PROTO_D3_D3_D3(cross)
DEF_GEOM_PROTO_D4_D4_D4(cross)
