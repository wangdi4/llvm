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
// EXEMPLARY, OR CONSEQUENTIAL DAES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#pragma once

#include "cl_types2.h"

// Define function generic prototypes
// ///////////////////////////////
#define DEF_FLOAT_PROTO_vX_vY(FUNC)\
	float __attribute__((overloadable)) FUNC(float x)\
	{\
		float16 tempX;\
		tempX.lo.lo.lo.lo = x;\
		float16 res = mask_ ## FUNC(0x01, tempX);\
		return res.lo.lo.lo.lo;\
	}\
	float2 __attribute__((overloadable)) FUNC(float2 x)\
	{\
		float16 tempX;\
		tempX.lo.lo.lo = x;\
		float16 res = mask_ ## FUNC(0x03, tempX);\
		return res.lo.lo.lo;\
	}\
	float3 __attribute__((overloadable)) FUNC(float3 x)\
	{\
		float16 tempX;\
		tempX.s012 = x;\
		float16 res = mask_ ## FUNC(0x07, tempX);\
		return res.s012;\
	}\
	float4 __attribute__((overloadable)) FUNC(float4 x)\
	{\
		float16 tempX;\
		tempX.lo.lo = x;\
		float16 res = mask_ ## FUNC(0x0f, tempX);\
		return res.lo.lo;\
	}\
	float8 __attribute__((overloadable)) FUNC(float8 x)\
	{\
		float16 tempX;\
		tempX.lo = x;\
		float16 res = mask_ ## FUNC(0xff, tempX);\
		return res.lo;\
	}\
	double __attribute__((overloadable)) FUNC(double x)\
	{\
		double8 tempX;\
		tempX.lo.lo.lo = x;\
		double8 res = mask_ ## FUNC(0x01, tempX);\
		return res.lo.lo.lo;\
	}\
	double2 __attribute__((overloadable)) FUNC(double2 x)\
	{\
		double8 tempX;\
		tempX.lo.lo = x;\
		double8 res = mask_ ## FUNC(0x03, tempX);\
		return res.lo.lo;\
	}\
	double3 __attribute__((overloadable)) FUNC(double3 x)\
	{\
		double8 tempX;\
		tempX.s012 = x;\
		double8 res = mask_ ## FUNC(0x07, tempX);\
		return res.s012;\
	}\
	double4 __attribute__((overloadable)) FUNC(double4 x)\
	{\
		double8 tempX;\
		tempX.lo = x;\
		double8 res = mask_ ## FUNC(0x0f, tempX);\
		return res.lo;\
	}\
	double16 __attribute__((overloadable)) FUNC(double16 x)\
	{\
		double16 res;\
		res.lo = FUNC(x.lo);\
		res.hi = FUNC(x.hi);\
		return res;\
	}

#define DEF_FLOAT_PROTO_vX_vX_vY(FUNC)\
	float2 __attribute__((overloadable)) FUNC(float2 x, float2 y)\
	{\
		float16 tempX;\
		float16 tempY;\
		tempX.lo.lo.lo = x;\
		tempY.lo.lo.lo = y;\
		float16 res = FUNC(tempX, tempY);\
		return res.lo.lo.lo;\
	}\
	float3 __attribute__((overloadable)) FUNC(float3 x, float3 y)\
	{\
		float16 tempX;\
		float16 tempY;\
		tempX.s012 = x;\
		tempY.s012 = y;\
		float16 res = FUNC(tempX, tempY);\
		return res.s012;\
	}\
	float4 __attribute__((overloadable)) FUNC(float4 x, float4 y)\
	{\
		float16 tempX;\
		float16 tempY;\
		tempX.lo.lo = x;\
		tempY.lo.lo = y;\
		float16 res = FUNC(tempX, tempY);\
		return res.lo.lo;\
	}\
	float8 __attribute__((overloadable)) FUNC(float8 x, float8 y)\
	{\
		float16 tempX;\
		float16 tempY;\
		tempX.lo = x;\
		tempY.lo = y;\
		float16 res = FUNC(tempX, tempY);\
		return res.lo;\
	}\
	double3 __attribute__((overloadable)) FUNC(double3 x, double3 y)\
	{\
		double8 tempX;\
		double8 tempY;\
		tempX.s012 = x;\
		tempY.s012 = y;\
		double8 res = FUNC(tempX, tempY);\
		return res.s012;\
	}\
	double4 __attribute__((overloadable)) FUNC(double4 x, double4 y)\
	{\
		double8 tempX;\
		double8 tempY;\
		tempX.lo = x;\
		tempY.lo = y;\
		double8 res = FUNC(tempX, tempY);\
		return res.lo;\
	}\
	double16 __attribute__((overloadable)) FUNC(double16 x, double16 y)\
	{\
		double16 res;\
		res.lo =  FUNC(x.lo, y.lo);\
		res.hi =  FUNC(x.hi, y.hi);\
		return res;\
	}

#define DEF_FLOAT_PROTO_vX_vX_vX_vY(FUNC)\
	float __attribute__((overloadable)) FUNC(float x, float y, float z)\
	{\
		float16 tempX;\
		float16 tempY;\
		float16 tempZ;\
		tempX.lo.lo.lo.lo = x;\
		tempY.lo.lo.lo.lo = y;\
		tempZ.lo.lo.lo.lo = z;\
		float16 res = mask_ ## FUNC(0x01, tempX, tempY, tempZ);\
		return res.lo.lo.lo.lo;\
	}\
	float2 __attribute__((overloadable)) FUNC(float2 x, float2 y, float2 z)\
	{\
		float16 tempX;\
		float16 tempY;\
		float16 tempZ;\
		tempX.lo.lo.lo = x;\
		tempY.lo.lo.lo = y;\
		tempZ.lo.lo.lo = z;\
		float16 res = mask_ ## FUNC(0x03, tempX, tempY, tempZ);\
		return res.lo.lo.lo;\
	}\
	float3 __attribute__((overloadable)) FUNC(float3 x, float3 y, float3 z)\
	{\
		float16 tempX;\
		float16 tempY;\
		float16 tempZ;\
		tempX.s012 = x;\
		tempY.s012 = y;\
		tempZ.s012 = z;\
		float16 res = mask_ ## FUNC(0x07, tempX, tempY, tempZ);\
		return res.s012;\
	}\
	float4 __attribute__((overloadable)) FUNC(float4 x, float4 y, float4 z)\
	{\
		float16 tempX;\
		float16 tempY;\
		float16 tempZ;\
		tempX.lo.lo = x;\
		tempY.lo.lo = y;\
		tempZ.lo.lo = z;\
		float16 res = mask_ ## FUNC(0x0f, tempX, tempY, tempZ);\
		return res.lo.lo;\
	}\
	float8 __attribute__((overloadable)) FUNC(float8 x, float8 y, float8 z)\
	{\
		float16 tempX;\
		float16 tempY;\
		float16 tempZ;\
		tempX.lo = x;\
		tempY.lo = y;\
		tempZ.lo = z;\
		float16 res = mask_ ## FUNC(0xff, tempX, tempY, tempZ);\
		return res.lo;\
	}\
	double __attribute__((overloadable)) FUNC(double x, double y, double z)\
	{\
		double8 tempX;\
		double8 tempY;\
		double8 tempZ;\
		tempX.lo.lo.lo = x;\
		tempY.lo.lo.lo = y;\
		tempZ.lo.lo.lo = z;\
		double8 res = mask_ ## FUNC(0x01, tempX, tempY, tempZ);\
		return res.lo.lo.lo;\
	}\
	double2 __attribute__((overloadable)) FUNC(double2 x, double2 y, double2 z)\
	{\
		double8 tempX;\
		double8 tempY;\
		double8 tempZ;\
		tempX.lo.lo = x;\
		tempY.lo.lo = y;\
		tempZ.lo.lo = z;\
		double8 res = mask_ ## FUNC(0x03, tempX, tempY, tempZ);\
		return res.lo.lo;\
	}\
	double3 __attribute__((overloadable)) FUNC(double3 x, double3 y, double3 z)\
	{\
		double8 tempX;\
		double8 tempY;\
		double8 tempZ;\
		tempX.s012 = x;\
		tempY.s012 = y;\
		tempZ.s012 = z;\
		double8 res = mask_ ## FUNC(0x07, tempX, tempY, tempZ);\
		return res.s012;\
	}\
	double4 __attribute__((overloadable)) FUNC(double4 x, double4 y, double4 z)\
	{\
		double8 tempX;\
		double8 tempY;\
		double8 tempZ;\
		tempX.lo = x;\
		tempY.lo = y;\
		tempZ.lo = z;\
		double8 res = mask_ ## FUNC(0x0f, tempX, tempY, tempZ);\
		return res.lo;\
	}\
	double16 __attribute__((overloadable)) FUNC(double16 x, double16 y, double16 z)\
	{\
		double16 res;\
		res.lo = FUNC(x.lo, y.lo, z.lo);\
		res.hi = FUNC(x.hi, y.hi, z.hi);\
		return res;\
	}

#define DEF_FLOAT_PROTO_vX_X_X_vY(FUNC)\
	float2 __attribute__((overloadable)) FUNC(float2 x, float y, float z)\
	{\
		float2 yV = (float2)y;\
		float2 zV = (float2)z;\
		return FUNC(x, yV, zV);\
	}\
	float3 __attribute__((overloadable)) FUNC(float3 x, float y, float z)\
	{\
		float3 yV = (float3)y;\
		float3 zV = (float3)z;\
		return FUNC(x, yV, zV);\
	}\
	float4 __attribute__((overloadable)) FUNC(float4 x, float y, float z)\
	{\
		float4 yV = (float4)y;\
		float4 zV = (float4)z;\
		return FUNC(x, yV, zV);\
	}\
	float8 __attribute__((overloadable)) FUNC(float8 x, float y, float z)\
	{\
		float8 yV = (float8)y;\
		float8 zV = (float8)z;\
		return FUNC(x, yV, zV);\
	}\
	float16 __attribute__((overloadable)) FUNC(float16 x, float y, float z)\
	{\
		float16 yV = (float16)y;\
		float16 zV = (float16)z;\
		return FUNC(x, yV, zV);\
	}\
	double2 __attribute__((overloadable)) FUNC(double2 x, double y, double z)\
	{\
		double2 yV = (double2)y;\
		double2 zV = (double2)z;\
		return FUNC(x, yV, zV);\
	}\
	double3 __attribute__((overloadable)) FUNC(double3 x, double y, double z)\
	{\
		double3 yV = (double3)y;\
		double3 zV = (double3)z;\
		return FUNC(x, yV, zV);\
	}\
	double4 __attribute__((overloadable)) FUNC(double4 x, double y, double z)\
	{\
		double4 yV = (double4)y;\
		double4 zV = (double4)z;\
		return FUNC(x, yV, zV);\
	}\
	double8 __attribute__((overloadable)) FUNC(double8 x, double y, double z)\
	{\
		double8 yV = (double8)y;\
		double8 zV = (double8)z;\
		return FUNC(x, yV, zV);\
	}\
	double16 __attribute__((overloadable)) FUNC(double16 x, double y, double z)\
	{\
		double16 yV = (double16)y;\
		double16 zV = (double16)z;\
		return FUNC(x, yV, zV);\
	}

#define DEF_FLOAT_PROTO_vX_X_vY(FUNC)\
	float2 __attribute__((overloadable)) FUNC(float2 x, float y)\
	{\
		float2 yV = (float2)y;\
		return FUNC(x, yV);\
	}\
	float3 __attribute__((overloadable)) FUNC(float3 x, float y)\
	{\
		float3 yV = (float3)y;\
		return FUNC(x, yV);\
	}\
	float4 __attribute__((overloadable)) FUNC(float4 x, float y)\
	{\
		float4 yV = (float4)y;\
		return FUNC(x, yV);\
	}\
	float8 __attribute__((overloadable)) FUNC(float8 x, float y)\
	{\
		float8 yV = (float8)y;\
		return FUNC(x, yV);\
	}\
	float16 __attribute__((overloadable)) FUNC(float16 x, float y)\
	{\
		float16 yV = (float16)y;\
		return FUNC(x, yV);\
	}\
	double2 __attribute__((overloadable)) FUNC(double2 x, double y)\
	{\
		double2 yV = (double2)y;\
		return FUNC(x, yV);\
	}\
	double3 __attribute__((overloadable)) FUNC(double3 x, double y)\
	{\
		double3 yV = (double3)y;\
		return FUNC(x, yV);\
	}\
	double4 __attribute__((overloadable)) FUNC(double4 x, double y)\
	{\
		double4 yV = (double4)y;\
		return FUNC(x, yV);\
	}\
	double8 __attribute__((overloadable)) FUNC(double8 x, double y)\
	{\
		double8 yV = (double8)y;\
		return FUNC(x, yV);\
	}\
	double16 __attribute__((overloadable)) FUNC(double16 x, double y)\
	{\
		double16 yV = (double16)y;\
		return FUNC(x, yV);\
	}

#define DEF_FLOAT_PROTO_vX_vX_X_vY(FUNC)\
	float2 __attribute__((overloadable)) FUNC(float2 x, float2 y, float z)\
	{\
		float2 zV = (float2)z;\
		return FUNC(x, y, zV);\
	}\
	float3 __attribute__((overloadable)) FUNC(float3 x, float3 y, float z)\
	{\
		float3 zV = (float3)z;\
		return FUNC(x, y, zV);\
	}\
	float4 __attribute__((overloadable)) FUNC(float4 x, float4 y, float z)\
	{\
		float4 zV = (float4)z;\
		return FUNC(x, y, zV);\
	}\
	float8 __attribute__((overloadable)) FUNC(float8 x, float8 y, float z)\
	{\
		float8 zV = (float8)z;\
		return FUNC(x, y, zV);\
	}\
	float16 __attribute__((overloadable)) FUNC(float16 x, float16 y, float z)\
	{\
		float16 zV = (float16)z;\
		return FUNC(x, y, zV);\
	}\
	double2 __attribute__((overloadable)) FUNC(double2 x, double2 y, double z)\
	{\
		double2 zV = (double2)z;\
		return FUNC(x, y, zV);\
	}\
	double3 __attribute__((overloadable)) FUNC(double3 x, double3 y, double z)\
	{\
		double3 zV = (double3)z;\
		return FUNC(x, y, zV);\
	}\
	double4 __attribute__((overloadable)) FUNC(double4 x, double4 y, double z)\
	{\
		double4 zV = (double4)z;\
		return FUNC(x, y, zV);\
	}\
	double8 __attribute__((overloadable)) FUNC(double8 x, double8 y, double z)\
	{\
		double8 zV = (double8)z;\
		return FUNC(x, y, zV);\
	}\
	double16 __attribute__((overloadable)) FUNC(double16 x, double16 y, double z)\
	{\
		double16 zV = (double16)z;\
		return FUNC(x, y, zV);\
	}

#define DEF_FLOAT_PROTO_X_vX_vY(FUNC)\
	float2 __attribute__((overloadable)) FUNC(float x, float2 y)\
	{\
		float2 xV = (float2)x;\
		return FUNC(xV, y);\
	}\
	float3 __attribute__((overloadable)) FUNC(float x, float3 y)\
	{\
		float3 xV = (float3)x;\
		return FUNC(xV, y);\
	}\
	float4 __attribute__((overloadable)) FUNC(float x, float4 y)\
	{\
		float4 xV = (float4)x;\
		return FUNC(xV, y);\
	}\
	float8 __attribute__((overloadable)) FUNC(float x, float8 y)\
	{\
		float8 xV = (float8)x;\
		return FUNC(xV, y);\
	}\
	float16 __attribute__((overloadable)) FUNC(float x, float16 y)\
	{\
		float16 xV = (float16)x;\
		return FUNC(xV, y);\
	}\
	double2 __attribute__((overloadable)) FUNC(double x, double2 y)\
	{\
		double2 xV = (double2)x;\
		return FUNC(xV, y);\
	}\
	double3 __attribute__((overloadable)) FUNC(double x, double3 y)\
	{\
		double3 xV = (double3)x;\
		return FUNC(xV, y);\
	}\
	double4 __attribute__((overloadable)) FUNC(double x, double4 y)\
	{\
		double4 xV = (double4)x;\
		return FUNC(xV, y);\
	}\
	double8 __attribute__((overloadable)) FUNC(double x, double8 y)\
	{\
		double8 xV = (double8)x;\
		return FUNC(xV, y);\
	}\
	double16 __attribute__((overloadable)) FUNC(double x, double16 y)\
	{\
		double16 xV = (double16)x;\
		return FUNC(xV, y);\
	}

#define DEF_FLOAT_PROTO_X_X_vX_vY(FUNC)\
	float2 __attribute__((overloadable)) FUNC(float x, float y, float2 z)\
	{\
		float2 xV = (float2)x;\
		float2 yV = (float2)y;\
		return FUNC(xV, yV, z);\
	}\
	float3 __attribute__((overloadable)) FUNC(float x, float y, float3 z)\
	{\
		float3 xV = (float3)x;\
		float3 yV = (float3)y;\
		return FUNC(xV, yV, z);\
	}\
	float4 __attribute__((overloadable)) FUNC(float x, float y, float4 z)\
	{\
		float4 xV = (float4)x;\
		float4 yV = (float4)y;\
		return FUNC(xV, yV, z);\
	}\
	float8 __attribute__((overloadable)) FUNC(float x, float y, float8 z)\
	{\
		float8 xV = (float8)x;\
		float8 yV = (float8)y;\
		return FUNC(xV, yV, z);\
	}\
	float16 __attribute__((overloadable)) FUNC(float x, float y, float16 z)\
	{\
		float16 xV = (float16)x;\
		float16 yV = (float16)y;\
		return FUNC(xV, yV, z);\
	}\
	double2 __attribute__((overloadable)) FUNC(double x, double y, double2 z)\
	{\
		double2 xV = (double2)x;\
		double2 yV = (double2)y;\
		return FUNC(xV, yV, z);\
	}\
	double3 __attribute__((overloadable)) FUNC(double x, double y, double3 z)\
	{\
		double3 xV = (double3)x;\
		double3 yV = (double3)y;\
		return FUNC(xV, yV, z);\
	}\
	double4 __attribute__((overloadable)) FUNC(double x, double y, double4 z)\
	{\
		double4 xV = (double4)x;\
		double4 yV = (double4)y;\
		return FUNC(xV, yV, z);\
	}\
	double8 __attribute__((overloadable)) FUNC(double x, double y, double8 z)\
	{\
		double8 xV = (double8)x;\
		double8 yV = (double8)y;\
		return FUNC(xV, yV, z);\
	}\
	double16 __attribute__((overloadable)) FUNC(double x, double y, double16 z)\
	{\
		double16 xV = (double16)x;\
		double16 yV = (double16)y;\
		return FUNC(xV, yV, z);\
	}


DEF_FLOAT_PROTO_vX_vX_vX_vY(clamp)

DEF_FLOAT_PROTO_vX_X_X_vY(clamp)

DEF_FLOAT_PROTO_vX_vY(degrees)

DEF_FLOAT_PROTO_vX_vY(radians)

DEF_FLOAT_PROTO_vX_vX_vY(max)

DEF_FLOAT_PROTO_vX_X_vY(max)

DEF_FLOAT_PROTO_vX_vX_vY(min)

DEF_FLOAT_PROTO_vX_X_vY(min)

DEF_FLOAT_PROTO_vX_vX_vX_vY(mix)

DEF_FLOAT_PROTO_vX_vX_X_vY(mix)

DEF_FLOAT_PROTO_vX_vX_vY(step)

DEF_FLOAT_PROTO_X_vX_vY(step)

DEF_FLOAT_PROTO_vX_vX_vX_vY(smoothstep)

DEF_FLOAT_PROTO_X_X_vX_vY(smoothstep)

DEF_FLOAT_PROTO_vX_vY(sign)
