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
	float2 __attribute__((overloadable)) FUNC(float2 x)\
	{\
		float4 tempX;\
		tempX.lo = x;\
		float4 res = FUNC(tempX);\
		return res.lo;\
	}\
	float3 __attribute__((overloadable)) FUNC(float3 x)\
	{\
		float4 tempX;\
		tempX.s012 = x;\
		float4 res = FUNC(tempX);\
		return res.s012;\
	}\
	float16 __attribute__((overloadable)) FUNC(float16 x)\
	{\
		float16 res;\
		res.lo =  FUNC(x.lo);\
		res.hi =  FUNC(x.hi);\
		return res;\
	}\
	double3 __attribute__((overloadable)) FUNC(double3 x)\
	{\
		double3 res;\
		res.s01 = FUNC(x.s01);\
		res.s2 = FUNC(x.s2);\
		return res;\
	}\
	double8 __attribute__((overloadable)) FUNC(double8 x)\
	{\
		double8 res;\
		res.lo = FUNC(x.lo);\
		res.hi = FUNC(x.hi);\
		return res;\
	}\
	double16 __attribute__((overloadable)) FUNC(double16 x)\
	{\
		double16 res;\
		res.lo.lo = FUNC(x.lo.lo);\
		res.lo.hi = FUNC(x.lo.hi);\
		res.hi.lo = FUNC(x.hi.lo);\
		res.hi.hi = FUNC(x.hi.hi);\
		return res;\
	}

#define DEF_FLOAT_PROTO_vX_vX_vY(FUNC)\
	float2 __attribute__((overloadable)) FUNC(float2 x, float2 y)\
	{\
		float4 tempX;\
		float4 tempY;\
		tempX.lo = x;\
		tempY.lo = y;\
		float4 res = FUNC(tempX, tempY);\
		return res.lo;\
	}\
	float3 __attribute__((overloadable)) FUNC(float3 x, float3 y)\
	{\
		float4 tempX;\
		float4 tempY;\
		tempX.s012 = x;\
		tempY.s012 = y;\
		float4 res = FUNC(tempX, tempY);\
		return res.s012;\
	}\
	float16 __attribute__((overloadable)) FUNC(float16 x, float16 y)\
	{\
		float16 res;\
		res.lo =  FUNC(x.lo, y.lo);\
		res.hi =  FUNC(x.hi, y.hi);\
		return res;\
	}\
	double3 __attribute__((overloadable)) FUNC(double3 x, double3 y)\
	{\
		double3 res;\
		res.s01 = FUNC(x.s01, y.s01);\
		res.s2 = FUNC(x.s2, y.s2);\
		return res;\
	}\
	double8 __attribute__((overloadable)) FUNC(double8 x, double8 y)\
	{\
		double8 res;\
		res.lo =  FUNC(x.lo, y.lo);\
		res.hi =  FUNC(x.hi, y.hi);\
		return res;\
	}\
	double16 __attribute__((overloadable)) FUNC(double16 x, double16 y)\
	{\
		double16 res;\
		res.lo.lo =  FUNC(x.lo.lo, y.lo.lo);\
		res.lo.hi =  FUNC(x.lo.hi, y.lo.hi);\
		res.hi.lo =  FUNC(x.hi.lo, y.hi.lo);\
		res.hi.hi =  FUNC(x.hi.hi, y.hi.hi);\
		return res;\
	}

#define DEF_FLOAT_PROTO_vX_vX_vX_vY(FUNC)\
	float2 __attribute__((overloadable)) FUNC(float2 x, float2 y, float2 z)\
	{\
		float4 tempX;\
		float4 tempY;\
		float4 tempZ;\
		tempX.lo = x;\
		tempY.lo = y;\
		tempZ.lo = z;\
		float4 res = FUNC(tempX, tempY, tempZ);\
		return res.lo;\
	}\
	float3 __attribute__((overloadable)) FUNC(float3 x, float3 y, float3 z)\
	{\
		float4 tempX;\
		float4 tempY;\
		float4 tempZ;\
		tempX.s012 = x;\
		tempY.s012 = y;\
		tempZ.s012 = z;\
		float4 res = FUNC(tempX, tempY, tempZ);\
		return res.s012;\
	}\
	float16 __attribute__((overloadable)) FUNC(float16 x, float16 y, float16 z)\
	{\
		float16 res;\
		res.lo =  FUNC(x.lo, y.lo, z.lo);\
		res.hi =  FUNC(x.hi, y.hi, z.hi);\
		return res;\
	}\
	double3 __attribute__((overloadable)) FUNC(double3 x, double3 y, double3 z)\
	{\
		double3 res;\
		res.s01 = FUNC(x.s01, y.s01, z.s01);\
		res.s2 = FUNC(x.s2, y.s2, z.s2);\
		return res;\
	}\
	double8 __attribute__((overloadable)) FUNC(double8 x, double8 y, double8 z)\
	{\
		double8 res;\
		res.lo = FUNC(x.lo, y.lo, z.lo);\
		res.hi = FUNC(x.hi, y.hi, z.hi);\
		return res;\
	}\
	double16 __attribute__((overloadable)) FUNC(double16 x, double16 y, double16 z)\
	{\
		double16 res;\
		res.lo.lo = FUNC(x.lo.lo, y.lo.lo, z.lo.lo);\
		res.lo.hi = FUNC(x.lo.hi, y.lo.hi, z.lo.hi);\
		res.hi.lo = FUNC(x.hi.lo, y.hi.lo, z.hi.lo);\
		res.hi.hi = FUNC(x.hi.hi, y.hi.hi, z.hi.hi);\
		return res;\
	}

#define DEF_FLOAT_PROTO_vX_X_X_vY(FUNC)\
	float2 __attribute__((overloadable)) FUNC(float2 x, float y, float z)\
	{\
		float4 tempX;\
		tempX.lo = x;\
		float4 yV = (float4)y;\
		float4 zV = (float4)z;\
		float4 res = FUNC(tempX, yV, zV);\
		return res.lo;\
	}\
	float3 __attribute__((overloadable)) FUNC(float3 x, float y, float z)\
	{\
		float4 tempX;\
		tempX.s012 = x;\
		float4 yV = (float4)y;\
		float4 zV = (float4)z;\
		float4 res = FUNC(tempX, yV, zV);\
		return res.s012;\
	}\
	float4 __attribute__((overloadable)) FUNC(float4 x, float y, float z)\
	{\
		float4 yV = (float4)y;\
		float4 zV = (float4)z;\
		return FUNC(x, yV, zV);\
	}\
	float8 __attribute__((overloadable)) FUNC(float8 x, float y, float z)\
	{\
		float8 yV = (float8)(y,y,y,y,y,y,y,y);\
		float8 zV = (float8)(z,z,z,z,z,z,z,z);\
		return FUNC(x, yV, zV);\
	}\
	float16 __attribute__((overloadable)) FUNC(float16 x, float y, float z)\
	{\
		float16 res;\
		float8 yV = (float8)(y,y,y,y,y,y,y,y);\
		float8 zV = (float8)(z,z,z,z,z,z,z,z);\
		res.lo =  FUNC(x.lo, yV, zV);\
		res.hi =  FUNC(x.hi, yV, zV);\
		return res;\
	}\
	double2 __attribute__((overloadable)) FUNC(double2 x, double y, double z)\
	{\
		double2 yV = (double2)y;\
		double2 zV = (double2)z;\
		return FUNC(x, yV, zV);\
	}\
	double3 __attribute__((overloadable)) FUNC(double3 x, double y, double z)\
	{\
		double3 res;\
		double2 yV = (double2)y;\
		double2 zV = (double2)z;\
		res.s01 = FUNC(x.s01, yV, zV);\
		res.s2 = FUNC(x.s2, y, z);\
		return res;\
	}\
	double4 __attribute__((overloadable)) FUNC(double4 x, double y, double z)\
	{\
		double4 yV = (double4)y;\
		double4 zV = (double4)z;\
		return FUNC(x, yV, zV);\
	}\
	double8 __attribute__((overloadable)) FUNC(double8 x, double y, double z)\
	{\
		double8 res;\
		double4 yV = (double4)y;\
		double4 zV = (double4)z;\
		res.lo =  FUNC(x.lo, yV, zV);\
		res.hi =  FUNC(x.hi, yV, zV);\
		return res;\
	}\
	double16 __attribute__((overloadable)) FUNC(double16 x, double y, double z)\
	{\
		double16 res;\
		double4 yV = (double4)y;\
		double4 zV = (double4)z;\
		res.lo.lo =  FUNC(x.lo.lo, yV, zV);\
		res.lo.hi =  FUNC(x.lo.hi, yV, zV);\
		res.hi.lo =  FUNC(x.hi.lo, yV, zV);\
		res.hi.hi =  FUNC(x.hi.hi, yV, zV);\
		return res;\
	}

#define DEF_FLOAT_PROTO_vX_X_vY(FUNC)\
	float2 __attribute__((overloadable)) FUNC(float2 x, float y)\
	{\
		float4 tempX;\
		tempX.lo = x;\
		float4 yV = (float4)y;\
		float4 res = FUNC(tempX, yV);\
		return res.lo;\
	}\
	float3 __attribute__((overloadable)) FUNC(float3 x, float y)\
	{\
		float4 tempX;\
		tempX.s012 = x;\
		float4 yV = (float4)y;\
		float4 res = FUNC(tempX, yV);\
		return res.s012;\
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
		float16 res;\
		float8 yV = (float8)y;\
		res.lo =  FUNC(x.lo, yV);\
		res.hi =  FUNC(x.hi, yV);\
		return res;\
	}\
	double2 __attribute__((overloadable)) FUNC(double2 x, double y)\
	{\
		double2 yV = (double2)y;\
		return FUNC(x, yV);\
	}\
	double3 __attribute__((overloadable)) FUNC(double3 x, double y)\
	{\
		double3 res;\
		double2 yV = (double2)y;\
		res.s01 = FUNC(x.s01, yV);\
		res.s2 = FUNC(x.s2, y);\
		return res;\
	}\
	double4 __attribute__((overloadable)) FUNC(double4 x, double y)\
	{\
		double4 yV = (double4)y;\
		return FUNC(x, yV);\
	}\
	double8 __attribute__((overloadable)) FUNC(double8 x, double y)\
	{\
		double8 res;\
		double4 yV = (double4)y;\
		res.lo =  FUNC(x.lo, yV);\
		res.hi =  FUNC(x.hi, yV);\
		return res;\
	}\
	double16 __attribute__((overloadable)) FUNC(double16 x, double y)\
	{\
		double16 res;\
		double4 yV = (double4)y;\
		res.lo.lo =  FUNC(x.lo.lo, yV);\
		res.lo.hi =  FUNC(x.lo.hi, yV);\
		res.hi.lo =  FUNC(x.hi.lo, yV);\
		res.hi.hi =  FUNC(x.hi.hi, yV);\
		return res;\
	}

#define DEF_FLOAT_PROTO_vX_vX_X_vY(FUNC)\
	float2 __attribute__((overloadable)) FUNC(float2 x, float2 y, float z)\
	{\
		float4 tempX;\
		float4 tempY;\
		tempX.lo = x;\
		tempY.lo = y;\
		float4 zV = (float4)z;\
		float4 res = FUNC(tempX, tempY, zV);\
		return res.lo;\
	}\
	float3 __attribute__((overloadable)) FUNC(float3 x, float3 y, float z)\
	{\
		float4 tempX;\
		float4 tempY;\
		tempX.s012 = x;\
		tempY.s012 = y;\
		float4 zV = (float4)z;\
		float4 res = FUNC(tempX, tempY, zV);\
		return res.s012;\
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
		float16 res;\
		float8 zV = (float8)z;\
		res.lo =  FUNC(x.lo, y.lo, zV);\
		res.hi =  FUNC(x.hi, y.hi, zV);\
		return res;\
	}\
	double2 __attribute__((overloadable)) FUNC(double2 x, double2 y, double z)\
	{\
		double2 zV = (double2)z;\
		return FUNC(x, y, zV);\
	}\
	double3 __attribute__((overloadable)) FUNC(double3 x, double3 y, double z)\
	{\
		double3 res;\
		double2 zV = (double2)z;\
		res.s01 = FUNC(x.s01, y.s01, zV);\
		res.s2 = FUNC(x.s2, y.s2, z);\
		return res;\
	}\
	double4 __attribute__((overloadable)) FUNC(double4 x, double4 y, double z)\
	{\
		double4 zV = (double4)z;\
		return FUNC(x, y, zV);\
	}\
	double8 __attribute__((overloadable)) FUNC(double8 x, double8 y, double z)\
	{\
		double8 res;\
		double4 zV = (double4)z;\
		res.lo =  FUNC(x.lo, y.lo, zV);\
		res.hi =  FUNC(x.hi, y.hi, zV);\
		return res;\
	}\
	double16 __attribute__((overloadable)) FUNC(double16 x, double16 y, double z)\
	{\
		double16 res;\
		double4 zV = (double4)z;\
		res.lo.lo = FUNC(x.lo.lo, y.lo.lo, zV);\
		res.lo.hi = FUNC(x.lo.hi, y.lo.hi, zV);\
		res.hi.lo = FUNC(x.hi.lo, y.hi.lo, zV);\
		res.hi.hi = FUNC(x.hi.hi, y.hi.hi, zV);\
		return res;\
	}

#define DEF_FLOAT_PROTO_X_vX_vY(FUNC)\
	float2 __attribute__((overloadable)) FUNC(float x, float2 y)\
	{\
		float4 tempY;\
		tempY.lo = y;\
		float4 xV = (float4)x;\
		float4 res = FUNC(xV, tempY);\
		return res.lo;\
	}\
	float3 __attribute__((overloadable)) FUNC(float x, float3 y)\
	{\
		float4 tempY;\
		tempY.s012 = y;\
		float4 xV = (float4)x;\
		float4 res = FUNC(xV, tempY);\
		return res.s012;\
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
		float16 res;\
		float8 xV = (float8)x;\
		res.lo =  FUNC(xV, y.lo);\
		res.hi =  FUNC(xV, y.hi);\
		return res;\
	}\
	double2 __attribute__((overloadable)) FUNC(double x, double2 y)\
	{\
		double2 xV = (double2)x;\
		return FUNC(xV, y);\
	}\
	double3 __attribute__((overloadable)) FUNC(double x, double3 y)\
	{\
		double3 res;\
		double2 xV = (double2)x;\
		res.s01 = FUNC(xV, y.s01);\
		res.s2 = FUNC(x, y.s2);\
		return res;\
	}\
	double4 __attribute__((overloadable)) FUNC(double x, double4 y)\
	{\
		double4 res;\
		double4 xV = (double4)x;\
		return FUNC(xV, y);\
	}\
	double8 __attribute__((overloadable)) FUNC(double x, double8 y)\
	{\
		double8 res;\
		double4 xV = (double4)x;\
		res.lo = FUNC(xV, y.lo);\
		res.hi = FUNC(xV, y.hi);\
		return res;\
	}\
	double16 __attribute__((overloadable)) FUNC(double x, double16 y)\
	{\
		double16 res;\
		double4 xV = (double4)x;\
		res.lo.lo = FUNC(xV, y.lo.lo);\
		res.lo.hi = FUNC(xV, y.lo.hi);\
		res.hi.lo = FUNC(xV, y.hi.lo);\
		res.hi.hi = FUNC(xV, y.hi.hi);\
		return res;\
	}

#define DEF_FLOAT_PROTO_X_X_vX_vY(FUNC)\
	float2 __attribute__((overloadable)) FUNC(float x, float y, float2 z)\
	{\
		float4 tempZ;\
		tempZ.lo = z;\
		float4 xV = (float4)x;\
		float4 yV = (float4)y;\
		float4 res = FUNC(xV, yV, tempZ);\
		return res.lo;\
	}\
	float3 __attribute__((overloadable)) FUNC(float x, float y, float3 z)\
	{\
		float4 tempZ;\
		tempZ.s012 = z;\
		float4 xV = (float4)x;\
		float4 yV = (float4)y;\
		float4 res = FUNC(xV, yV, tempZ);\
		return res.s012;\
	}\
	float4 __attribute__((overloadable)) FUNC(float x, float y, float4 z)\
	{\
		float4 xV = (float4)x;\
		float4 yV = (float4)y;\
		return FUNC(xV, yV, z);\
	}\
	float8 __attribute__((overloadable)) FUNC(float x, float y, float8 z)\
	{\
		float8 yV = (float8)(y,y,y,y,y,y,y,y);\
		float8 xV = (float8)(x,x,x,x,x,x,x,x);\
		return FUNC(xV, yV, z);\
	}\
	float16 __attribute__((overloadable)) FUNC(float x, float y, float16 z)\
	{\
		float16 res;\
		float8 yV = (float8)(y,y,y,y,y,y,y,y);\
		float8 xV = (float8)(x,x,x,x,x,x,x,x);\
		res.lo =  FUNC(xV, yV, z.lo);\
		res.hi =  FUNC(xV, yV, z.hi);\
		return res;\
	}\
	double2 __attribute__((overloadable)) FUNC(double x, double y, double2 z)\
	{\
		double2 xV = (double2)x;\
		double2 yV = (double2)y;\
		return FUNC(xV, yV, z);\
	}\
	double3 __attribute__((overloadable)) FUNC(double x, double y, double3 z)\
	{\
		double3 res;\
		double2 xV = (double2)x;\
		double2 yV = (double2)y;\
		res.s01 = FUNC(xV, yV, z.s01);\
		res.s2 = FUNC(x, y, z.s2);\
		return res;\
	}\
	double4 __attribute__((overloadable)) FUNC(double x, double y, double4 z)\
	{\
		double4 xV = (double4)x;\
		double4 yV = (double4)y;\
		return FUNC(xV, yV, z);\
	}\
	double8 __attribute__((overloadable)) FUNC(double x, double y, double8 z)\
	{\
		double8 res;\
		double4 xV = (double4)x;\
		double4 yV = (double4)y;\
		res.lo =  FUNC(xV, yV, z.lo);\
		res.hi =  FUNC(xV, yV, z.hi);\
		return res;\
	}\
	double16 __attribute__((overloadable)) FUNC(double x, double y, double16 z)\
	{\
		double16 res;\
		double4 xV = (double4)x;\
		double4 yV = (double4)y;\
		res.lo.lo = FUNC(xV, yV, z.lo.lo);\
		res.lo.hi = FUNC(xV, yV, z.lo.hi);\
		res.hi.lo = FUNC(xV, yV, z.hi.lo);\
		res.hi.hi = FUNC(xV, yV, z.hi.hi);\
		return res;\
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
