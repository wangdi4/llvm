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

///////////////////////////////////////////////////////////
//  cl_relational_declaration.h
///////////////////////////////////////////////////////////
#pragma once

#include "cl_types2.h"

#define DEF_REL_FUNC_vI_vF_vF(FUNC)\
	_2i32 __attribute__((overloadable)) FUNC(float2 x, float2 y)\
	{\
	float4 tmp1, tmp2;\
	_4i32 res;\
	tmp1.lo = x;\
	tmp2.lo = y;\
	res = FUNC(tmp1, tmp2);\
	return res.lo;\
	}\
	_3i32 __attribute__((overloadable)) FUNC(float3 x, float3 y)\
	{\
	float4 tmp1, tmp2;\
	_4i32 res;\
	tmp1.s012 = x;\
	tmp2.s012 = y;\
	res = FUNC(tmp1, tmp2);\
	return res.s012;\
	}\
	_16i32 __attribute__((overloadable)) FUNC(float16 x, float16 y)\
	{\
	_16i32 res;\
	res.lo = FUNC(x.lo, y.lo);\
	res.hi = FUNC(x.hi, y.hi);\
	return res;\
	}

#define DEF_REL_FUNC_vI_vD_vD(FUNC)\
	_3i64 __attribute__((overloadable)) FUNC(double3 x, double3 y)\
	{\
		_4i64 res;\
		res.xy = FUNC(x.xy, y.xy);\
		res.zw = FUNC(x.zz, y.zz);\
		return res.xyz;\
	}\
	_8i64 __attribute__((overloadable)) FUNC(double8 x, double8 y)\
	{\
		_8i64 res;\
		res.lo = FUNC(x.lo, y.lo);\
		res.hi = FUNC(x.hi, y.hi);\
		return res;\
	}\
	_16i64 __attribute__((overloadable)) FUNC(double16 x, double16 y)\
	{\
		_16i64 res;\
		res.lo.lo = FUNC(x.lo.lo, y.lo.lo);\
		res.lo.hi = FUNC(x.lo.hi, y.lo.hi);\
		res.hi.lo = FUNC(x.hi.lo, y.hi.lo);\
		res.hi.hi = FUNC(x.hi.hi, y.hi.hi);\
		return res;\
	}

#define DEF_REL_FUNC_vI_vF(FUNC)\
	_2i32 __attribute__((overloadable)) FUNC(float2 x)\
	{\
		float4 tmp;\
		_4i32 res;\
		tmp.lo = x;\
		res = FUNC(tmp);\
		return res.lo;\
	}\
	_3i32 __attribute__((overloadable)) FUNC(float3 x)\
	{\
		float4 tmp;\
		_4i32 res;\
		tmp.s012 = x;\
		res = FUNC(tmp);\
		return res.s012;\
	}\
	_16i32 __attribute__((overloadable)) FUNC(float16 x)\
	{\
		_16i32 res;\
		res.lo = FUNC(x.lo);\
		res.hi = FUNC(x.hi);\
		return res;\
	}

#define DEF_REL_FUNC_vI_vD(FUNC)\
	_3i64 __attribute__((overloadable)) FUNC(double3 x)\
	{\
		_4i64 res;\
		res = FUNC(x.s0122);\
		return res.s012;\
	}\
	_8i64 __attribute__((overloadable)) FUNC(double8 x)\
	{\
		_8i64 res;\
		res.lo = FUNC(x.lo);\
		res.hi = FUNC(x.hi);\
		return res;\
	}\
	_16i64 __attribute__((overloadable)) FUNC(double16 x)\
	{\
		_16i64 res;\
		res.lo.lo = FUNC(x.lo.lo);\
		res.lo.hi = FUNC(x.lo.hi);\
		res.hi.lo = FUNC(x.hi.lo);\
		res.hi.hi = FUNC(x.hi.hi);\
		return res;\
	}

#define DEF_INT1_PROTO8_X_Y(FUNC, OP)\
	_1i32 __attribute__((overloadable)) FUNC(_2i8 x)\
	{\
	_1i32 res1, res2;\
	res1 = FUNC(x.lo);\
	res2 = FUNC(x.hi);\
	return res1 OP res2;\
	}\
	_1i32 __attribute__((overloadable)) FUNC(_3i8 x)\
	{\
	_1i32 res1, res2, res3;\
	res1 = FUNC(x.s0);\
	res2 = FUNC(x.s1);\
	res3 = FUNC(x.s2);\
	return res1 OP res2 OP res3;\
	}\
	_1i32 __attribute__((overloadable)) FUNC(_4i8 x)\
	{\
	_16i8 tmp;\
	tmp.lo.lo = x;\
    tmp.lo.hi = x;\
	tmp.hi.lo = x;\
	tmp.hi.hi = x;\
	return FUNC(tmp);\
	}\
	_1i32 __attribute__((overloadable)) FUNC(_8i8 x)\
	{\
	_16i8 tmp;\
	tmp.lo = x;\
	tmp.hi = x;\
	return FUNC(tmp);\
	}

#define DEF_INT1_PROTO16_X_Y(FUNC, OP)\
	_1i32 __attribute__((overloadable)) FUNC(_2i16 x)\
	{\
	_1i32 res1, res2;\
	res1 =  FUNC(x.lo);\
	res2 =  FUNC(x.hi);\
	return res1 OP res2;\
	}\
	_1i32 __attribute__((overloadable)) FUNC(_3i16 x)\
	{\
	_1i32 res1, res2, res3;\
	res1 =  FUNC(x.s0);\
	res2 =  FUNC(x.s1);\
	res3 =  FUNC(x.s2);\
	return res1 OP res2 OP res3;\
	}\
	_1i32 __attribute__((overloadable)) FUNC(_4i16 x)\
	{\
	_8i16 tmp;\
    tmp.lo = x;\
    tmp.hi = x;\
	return FUNC(tmp);\
	}\
	_1i32 __attribute__((overloadable)) FUNC(_16i16 x)\
	{\
	_1i32 res1, res2;\
	res1 = FUNC(x.lo);\
	res2 = FUNC(x.hi);\
	return res1 OP res2;\
	}

#define DEF_INT1_PROTO32_X_Y(FUNC, OP)\
	_1i32 __attribute__((overloadable)) FUNC(_2i32 x)\
	{\
	_4i32 tmp;\
	tmp.lo = x;\
	tmp.hi = x;\
	return FUNC(tmp);\
	}\
	_1i32 __attribute__((overloadable)) FUNC(_3i32 x)\
	{\
	_4i32 tmp;\
	tmp.s012 = x;\
	tmp.s3 = x.s0;\
	return FUNC(tmp);\
	}\
	_1i32 __attribute__((overloadable)) FUNC(_16i32 x)\
	{\
	_1i32 res1, res2;\
	res1 = FUNC(x.lo);\
	res2 = FUNC(x.hi);\
	return res1 OP res2;\
	}

#define DEF_INT1_PROTO64_X_Y(FUNC, OP)\
	_1i32 __attribute__((overloadable)) FUNC(_3i64 x)\
	{\
	_1i32 res1, res2;\
	res1 = FUNC(x.s01);\
	res2 = FUNC(x.s2);\
	return res1 OP res2;\
	}\
	_1i32 __attribute__((overloadable)) FUNC(_8i64 x)\
	{\
	_1i32 res1, res2;\
	res1 = FUNC(x.lo);\
	res2 = FUNC(x.hi);\
	return res1 OP res2;\
	}\
	_1i32 __attribute__((overloadable)) FUNC(_16i64 x)\
	{\
	_1i32 res1, res2, res3, res4;\
	res1 = FUNC(x.lo.lo);\
	res2 = FUNC(x.lo.hi);\
	res3 = FUNC(x.hi.lo);\
	res4 = FUNC(x.hi.hi);\
	return res1 OP res2 OP res3 OP res4;\
	}

#define DEF_R_PROTO8_X_X_X_Y(FUNC, TI0, TI1, FIN)\
	_2##TI0##8 __attribute__((overloadable)) FUNC(_2##TI0##8 x, _2##TI0##8 y,  _2##TI1##8 z)\
	{\
	_16##TI0##8 tmpX, tmpY;\
	_16##TI1##8 tmpZ;\
	_16##TI0##8 res;\
	tmpX.s01 = x;\
	tmpY.s01 = y;\
	tmpZ.s01 = z;\
	res = FUNC(tmpX, tmpY, tmpZ);\
	return res.s01;\
	}\
	_3##TI0##8 __attribute__((overloadable)) FUNC(_3##TI0##8 x, _3##TI0##8 y,  _3##TI1##8 z)\
	{\
	_16##TI0##8 tmpX, tmpY;\
	_16##TI1##8 tmpZ;\
	_16##TI0##8 res;\
	tmpX.s012 = x;\
	tmpY.s012 = y;\
	tmpZ.s012 = z;\
	res = FUNC(tmpX, tmpY, tmpZ);\
	return res.s012;\
	}\
	_4##TI0##8 __attribute__((overloadable)) FUNC(_4##TI0##8 x, _4##TI0##8 y, _4##TI1##8 z)\
	{\
	_16##TI0##8 tmpX, tmpY;\
	_16##TI1##8 tmpZ;\
	_16##TI0##8 res;\
	tmpX.s0123 = x;\
	tmpY.s0123 = y;\
	tmpZ.s0123 = z;\
	res = FUNC(tmpX, tmpY, tmpZ);\
	return res.s0123;\
	}\
	_8##TI0##8 __attribute__((overloadable)) FUNC(_8##TI0##8 x, _8##TI0##8 y, _8##TI1##8 z)\
	{\
	_16##TI0##8 tmpX, tmpY;\
	_16##TI1##8 tmpZ;\
	_16##TI0##8 res;\
	tmpX.lo = x;\
	tmpY.lo = y;\
	tmpZ.lo = z;\
	res = FUNC(tmpX, tmpY, tmpZ);\
	return res.lo;\
	}

#define DEF_R_PROTO16_X_X_X_Y(FUNC, TI0, TI1, FIN)\
	_2##TI0##16 __attribute__((overloadable)) FUNC(_2##TI0##16 x, _2##TI0##16 y, _2##TI1##16 z)\
	{\
	_8##TI0##16 tmpX, tmpY;\
	_8##TI1##16 tmpZ;\
	tmpX.s01 = x;\
	tmpY.s01 = y;\
	tmpZ.s01 = z;\
	_8##TI0##16 res = FUNC(tmpX, tmpY, tmpZ);\
	return res.s01;\
	}\
	_3##TI0##16 __attribute__((overloadable)) FUNC(_3##TI0##16 x, _3##TI0##16 y, _3##TI1##16 z)\
	{\
	_8##TI0##16 tmpX, tmpY;\
	_8##TI1##16 tmpZ;\
	tmpX.s012 = x;\
	tmpY.s012 = y;\
	tmpZ.s012 = z;\
	_8##TI0##16 res = FUNC(tmpX, tmpY, tmpZ);\
	return res.s012;\
	}\
	_4##TI0##16 __attribute__((overloadable)) FUNC(_4##TI0##16 x, _4##TI0##16 y, _4##TI1##16 z)\
	{\
	_8##TI0##16 tmpX, tmpY;\
	_8##TI1##16 tmpZ;\
	tmpX.lo = x;\
	tmpY.lo = y;\
	tmpZ.lo = z;\
	_8##TI0##16 res = FUNC(tmpX, tmpY, tmpZ);\
	return res.lo;\
	}\
	_16##TI0##16 __attribute__((overloadable)) FUNC(_16##TI0##16 x, _16##TI0##16 y, _16##TI1##16 z)\
	{\
	_16##TI0##16 res;\
	res.lo = FUNC(x.lo, y.lo, z.lo);\
	res.hi = FUNC(x.hi, y.hi, z.hi);\
	return res;\
	}

#define DEF_R_BITSELECT_PROTO16_X_X_X_Y(FUNC, TI0, TI1, FIN)\
	_2##TI0##16 __attribute__((overloadable)) FUNC(_2##TI0##16 x, _2##TI0##16 y, _2##TI1##16 z)\
	{\
	_8##TI0##16 tmpX, tmpY;\
	_8##TI1##16 tmpZ;\
	tmpX.s01 = x;\
	tmpY.s01 = y;\
	tmpZ.s01 = z;\
	_8##TI0##16 res = FUNC(tmpX, tmpY, tmpZ);\
	return res.s01;\
	}\
	_3##TI0##16 __attribute__((overloadable)) FUNC(_3##TI0##16 x, _3##TI0##16 y, _3##TI1##16 z)\
	{\
	_8##TI0##16 tmpX, tmpY;\
	_8##TI1##16 tmpZ;\
	tmpX.s012 = x;\
	tmpY.s012 = y;\
	tmpZ.s012 = z;\
	_8##TI0##16 res = FUNC(tmpX, tmpY, tmpZ);\
	return res.s012;\
	}\
	_4##TI0##16 __attribute__((overloadable)) FUNC(_4##TI0##16 x, _4##TI0##16 y, _4##TI1##16 z)\
	{\
	_8##TI0##16 tmpX, tmpY;\
	_8##TI1##16 tmpZ;\
	tmpX.lo = x;\
	tmpY.lo = y;\
	tmpZ.lo = z;\
	_8##TI0##16 res = FUNC(tmpX, tmpY, tmpZ);\
	return res.lo;\
	}

#define DEF_R_PROTO32_X_X_X_Y(FUNC, TI0, TI1, FIN)\
	_2##TI0##32 __attribute__((overloadable)) FUNC(_2##TI0##32 x, _2##TI0##32 y, _2##TI1##32 z)\
	{\
	_4##TI0##32 tmpX, tmpY, res;\
	_4##TI1##32 tmpZ;\
	tmpX.lo = x;\
	tmpY.lo = y;\
	tmpZ.lo = z;\
	res = FUNC(tmpX, tmpY, tmpZ);\
	return res.lo;\
	}\
	_3##TI0##32 __attribute__((overloadable)) FUNC(_3##TI0##32 x, _3##TI0##32 y, _3##TI1##32 z)\
	{\
	_4##TI0##32 tmpX, tmpY, res;\
	_4##TI1##32 tmpZ;\
	tmpX.s012 = x;\
	tmpY.s012 = y;\
	tmpZ.s012 = z;\
	res = FUNC(tmpX, tmpY, tmpZ);\
	return res.s012;\
	}\
	_16##TI0##32 __attribute__((overloadable)) FUNC(_16##TI0##32 x, _16##TI0##32 y, _16##TI1##32 z)\
	{\
	_16##TI0##32 res;\
	res.lo = FUNC(x.lo, y.lo, z.lo);\
	res.hi = FUNC(x.hi, y.hi, z.hi);\
	return res;\
	}

#define DEF_R_PROTO64_X_X_X_Y(FUNC, TI0, TI1, FIN)\
	_3##TI0##64 __attribute__((overloadable)) FUNC(_3##TI0##64 x, _3##TI0##64 y, _3##TI1##64 z)\
	{\
	_4##TI0##64 tmpX, tmpY, res;\
	_4##TI1##64 tmpZ;\
	tmpX.s012 = x;\
	tmpY.s012 = y;\
	tmpZ.s012 = z;\
	res = FUNC(tmpX, tmpY, tmpZ);\
	return res.s012;\
	}\
	_8##TI0##64 __attribute__((overloadable)) FUNC(_8##TI0##64 x, _8##TI0##64 y, _8##TI1##64 z)\
	{\
	_8##TI0##64 res;\
	res.lo = FUNC(x.lo, y.lo, z.lo);\
	res.hi = FUNC(x.hi, y.hi, z.hi);\
	return res;\
	}\
	_16##TI0##64 __attribute__((overloadable)) FUNC(_16##TI0##64 x, _16##TI0##64 y, _16##TI1##64 z)\
	{\
	_16##TI0##64 res;\
	res.lo.lo = FUNC(x.lo.lo, y.lo.lo, z.lo.lo);\
	res.lo.hi = FUNC(x.lo.hi, y.lo.hi, z.lo.hi);\
	res.hi.lo = FUNC(x.hi.lo, y.hi.lo, z.hi.lo);\
	res.hi.hi = FUNC(x.hi.hi, y.hi.hi, z.hi.hi);\
	return res;\
	}

#define DEF_R_PROTO_F_F_F_Y(FUNC, TI)\
	float2 __attribute__((overloadable)) FUNC (float2 x, float2 y, _2##TI##32 z)\
	{\
	float4 tmpX, tmpY, res;\
	_4##TI##32 tmpZ;\
	tmpX.lo = x;\
	tmpY.lo = y;\
	tmpZ.lo = z;\
	res = FUNC (tmpX, tmpY, tmpZ);\
	return res.lo;\
	}\
	float3 __attribute__((overloadable)) FUNC (float3 x, float3 y, _3##TI##32 z)\
	{\
	float4 tmpX, tmpY, res;\
	_4##TI##32 tmpZ;\
	tmpX.s012 = x;\
	tmpY.s012 = y;\
	tmpZ.s012 = z;\
	res = FUNC (tmpX, tmpY, tmpZ);\
	return res.s012;\
	}\
	float16 __attribute__((overloadable)) FUNC (float16 x, float16 y, _16##TI##32 z)\
	{\
	float16 res;\
	res.lo = FUNC (x.lo, y.lo, z.lo);\
	res.hi = FUNC (x.hi, y.hi, z.hi);\
	return res;\
	}

#define DEF_R_PROTO_D_D_D_Y(FUNC, TI)\
	double3 __attribute__((overloadable)) FUNC(double3 x, double3 y, _3##TI##64 z)\
	{\
		double4 res;\
		res = FUNC(x.s0122, y.s0122, z.s0122);\
		return res.s012;\
	}\
	double8 __attribute__((overloadable)) FUNC(double8 x, double8 y, _8##TI##64 z)\
	{\
		double8 res;\
		res.lo = FUNC(x.lo, y.lo, z.lo);\
		res.hi = FUNC(x.hi, y.hi, z.hi);\
		return res;\
	}\
	double16 __attribute__((overloadable)) FUNC(double16 x, double16 y, _16##TI##64 z)\
	{\
		double16 res;\
		res.lo.lo = FUNC(x.lo.lo, y.lo.lo, z.lo.lo);\
		res.lo.hi = FUNC(x.lo.hi, y.lo.hi, z.lo.hi);\
		res.hi.lo = FUNC(x.hi.lo, y.hi.lo, z.hi.lo);\
		res.hi.hi = FUNC(x.hi.hi, y.hi.hi, z.hi.hi);\
		return res;\
	}

#define DEF_R_PROTO_F_F_F_F(FUNC)\
	float2 __attribute__((overloadable)) FUNC (float2 a, float2 b, float2 c)\
	{\
		float4	tmpX, tmpY, tmpZ;\
		tmpX.lo = a;\
		tmpY.lo = b;\
		tmpZ.lo = c;\
		float4  res = FUNC(tmpX, tmpY, tmpZ);\
		return res.lo;\
	}\
	float3 __attribute__((overloadable)) FUNC (float3 a, float3 b, float3 c)\
	{\
		float4	tmpX, tmpY, tmpZ;\
		tmpX.s012 = a;\
		tmpY.s012 = b;\
		tmpZ.s012 = c;\
		float4  res = FUNC(tmpX, tmpY, tmpZ);\
		return res.s012;\
	}\
	float16 __attribute__((overloadable)) FUNC (float16 a, float16 b, float16 c)\
	{\
		float16 res;\
		res.lo = FUNC(a.lo, b.lo, c.lo);\
		res.hi = FUNC(a.hi, b.hi, c.hi);\
		return res;\
	}

#define DEF_R_PROTO_D_D_D_D(FUNC)\
	double3 __attribute__((overloadable)) FUNC(double3 a, double3 b, double3 c)\
	{\
		double3 res;\
		res.s01 = FUNC(a.s01, b.s01, c.s01);\
		res.s2 = FUNC(a.s2, b.s2, c.s2);\
		return res;\
	}\
	double8 __attribute__((overloadable)) FUNC(double8 a, double8 b, double8 c)\
	{\
		double8 res;\
		res.lo = FUNC(a.lo, b.lo, c.lo);\
		res.hi = FUNC(a.hi, b.hi, c.hi);\
		return res;\
	}\
	double16 __attribute__((overloadable)) FUNC(double16 a, double16 b, double16 c)\
	{\
		double16 res;\
		res.lo.lo = FUNC(a.lo.lo, b.lo.lo, c.lo.lo);\
		res.lo.hi = FUNC(a.lo.hi, b.lo.hi, c.lo.hi);\
		res.hi.lo = FUNC(a.hi.lo, b.hi.lo, c.hi.lo);\
		res.hi.hi = FUNC(a.hi.hi, b.hi.hi, c.hi.hi);\
		return res;\
	}

// Full function definition (all integer types)
#define DEF_INT1_PROTO_X_Y(FUNC, OP)\
	DEF_INT1_PROTO8_X_Y(FUNC, OP)\
	DEF_INT1_PROTO16_X_Y(FUNC, OP)\
	DEF_INT1_PROTO32_X_Y(FUNC, OP)\
	DEF_INT1_PROTO64_X_Y(FUNC, OP)

#define DEF_R_PROTO_SELECT(FUNC, TI0, TI1, FIN)\
	DEF_R_PROTO8_X_X_X_Y(FUNC, TI0, TI1, FIN)\
	DEF_R_PROTO16_X_X_X_Y(FUNC, TI0, TI1, FIN)\
	DEF_R_PROTO32_X_X_X_Y(FUNC, TI0, TI1, FIN)\
	DEF_R_PROTO64_X_X_X_Y(FUNC, TI0, TI1, FIN)

#define DEF_R_PROTO_BITSELECT(FUNC, TI0, TI1, FIN)\
	DEF_R_PROTO8_X_X_X_Y(FUNC, TI0, TI1, FIN)\
	DEF_R_BITSELECT_PROTO16_X_X_X_Y(FUNC, TI0, TI1, FIN)\
	DEF_R_PROTO32_X_X_X_Y(FUNC, TI0, TI1, FIN)\
	DEF_R_PROTO64_X_X_X_Y(FUNC, TI0, TI1, FIN)

DEF_REL_FUNC_vI_vF_vF(islessgreater)
DEF_REL_FUNC_vI_vD_vD(islessgreater)

DEF_REL_FUNC_vI_vF(isfinite)
DEF_REL_FUNC_vI_vD(isfinite)
DEF_REL_FUNC_vI_vF(isinf)
DEF_REL_FUNC_vI_vD(isinf)
DEF_REL_FUNC_vI_vF(isnan)
DEF_REL_FUNC_vI_vD(isnan)
DEF_REL_FUNC_vI_vF(isnormal)
DEF_REL_FUNC_vI_vD(isnormal)

DEF_REL_FUNC_vI_vF_vF(isordered)
DEF_REL_FUNC_vI_vD_vD(isordered)
DEF_REL_FUNC_vI_vF_vF(isunordered)
DEF_REL_FUNC_vI_vD_vD(isunordered)
DEF_REL_FUNC_vI_vF(signbit)
DEF_REL_FUNC_vI_vD(signbit)

DEF_INT1_PROTO_X_Y(all, &&)
DEF_INT1_PROTO_X_Y(any, ||)

DEF_R_PROTO_BITSELECT(bitselect, i, i, )
DEF_R_PROTO_BITSELECT(bitselect, u, u, )

DEF_R_PROTO_F_F_F_F(bitselect)
DEF_R_PROTO_D_D_D_D(bitselect)

DEF_R_PROTO_SELECT(select, i, i, )
DEF_R_PROTO_SELECT(select, i, u, u)
DEF_R_PROTO_SELECT(select, u, i, )
DEF_R_PROTO_SELECT(select, u, u, u)
DEF_R_PROTO_F_F_F_Y(select, u)
DEF_R_PROTO_F_F_F_Y(select, i)
DEF_R_PROTO_D_D_D_Y(select, u)
DEF_R_PROTO_D_D_D_Y(select, i)
