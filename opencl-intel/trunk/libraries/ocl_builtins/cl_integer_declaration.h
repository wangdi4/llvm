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
//  cl_integer_declaration.h
///////////////////////////////////////////////////////////
#pragma once

#include "cl_types2.h"

// Defined constants used by MSFT
#define LLONG_MAX	LONG_MAX
#define LLONG_MIN	LONG_MIN
#define ULLONG_MAX	ULONG_MAX


#define DEF_INT_PROTO8_X_Y(FUNC, TI, TO)\
	_2##TO##8 __attribute__((overloadable)) FUNC(_2##TI##8 x)\
	{\
		_2##TO##8 res;\
		res.s0 = FUNC(x.s0);\
		res.s1 = FUNC(x.s1);\
		return res;\
	}\
	_3##TO##8 __attribute__((overloadable)) FUNC(_3##TI##8 x)\
	{\
		_16##TI##8 tmp;\
		tmp.s012 = x;\
		_16##TO##8 res = FUNC(tmp);\
		return res.s012;\
	}\
	_4##TO##8 __attribute__((overloadable)) FUNC(_4##TI##8 x)\
	{\
		_16##TI##8 tmp;\
		tmp.s0123 = x;\
		_16##TO##8 res = FUNC(tmp);\
		return res.s0123;\
	}\
	_8##TO##8 __attribute__((overloadable)) FUNC(_8##TI##8 x)\
	{\
		_16##TI##8 tmp;\
		tmp.lo = x;\
		_16##TO##8 res = FUNC(tmp);\
		return res.lo;\
	}

#define DEF_INT_PROTO16_X_Y(FUNC, TI, TO)\
	_2##TO##16 __attribute__((overloadable)) FUNC(_2##TI##16 x)\
	{\
		_8##TI##16 tmp;\
		tmp.s01 = x;\
		_8##TO##16 res = FUNC(tmp);\
		return res.s01;\
	}\
	_3##TO##16 __attribute__((overloadable)) FUNC(_3##TI##16 x)\
	{\
		_8##TI##16 tmp;\
		tmp.s012 = x;\
		_8##TO##16 res = FUNC(tmp);\
		return res.s012;\
	}\
	_4##TO##16 __attribute__((overloadable)) FUNC(_4##TI##16 x)\
	{\
		_8##TI##16 tmp;\
		tmp.lo = x;\
		_8##TO##16 res = FUNC(tmp);\
		return res.lo;\
	}\
	_16##TO##16 __attribute__((overloadable)) FUNC(_16##TI##16 x)\
	{\
		_16##TO##16 res;\
		res.lo = FUNC(x.lo);\
		res.hi = FUNC(x.hi);\
		return res;\
	}

#define DEF_INT_PROTO32_X_Y(FUNC, TI, TO)\
	_2##TO##32 __attribute__((overloadable)) FUNC(_2##TI##32 x)\
	{\
		_4##TI##32 tmpX;\
		tmpX.lo = x;\
		_4##TO##32 res = FUNC(tmpX);\
		return res.lo;\
	}\
	_3##TO##32 __attribute__((overloadable)) FUNC(_3##TI##32 x)\
	{\
		_4##TI##32 tmpX;\
		tmpX.s012 = x;\
		_4##TO##32 res = FUNC(tmpX);\
		return res.s012;\
	}\
	_8##TO##32 __attribute__((overloadable)) FUNC(_8##TI##32 x)\
	{\
		_8##TO##32 res;\
		res.lo = FUNC(x.lo);\
		res.hi = FUNC(x.hi);\
		return res;\
	}\
	_16##TO##32 __attribute__((overloadable)) FUNC(_16##TI##32 x)\
	{\
		_16##TO##32 res;\
		res.lo.lo = FUNC(x.lo.lo);\
		res.lo.hi = FUNC(x.lo.hi);\
		res.hi.lo = FUNC(x.hi.lo);\
		res.hi.hi = FUNC(x.hi.hi);\
		return res;\
	}

#define DEF_INT_PROTO64_X_Y(FUNC, TI, TO)\
	_3##TO##64 __attribute__((overloadable)) FUNC(_3##TI##64 x)\
	{\
		_3##TO##64 res;\
		res.s01 = FUNC((_2##TI##64)x.s01);\
		res.s2 = FUNC((_1##TI##64)x.s2);\
		return res.s012;\
	}\
	_4##TO##64 __attribute__((overloadable)) FUNC(_4##TI##64 x)\
	{\
		_4##TO##64 res;\
		res.lo = FUNC((_2##TI##64)x.lo);\
		res.hi = FUNC((_2##TI##64)x.hi);\
		return res;\
	}\
	_8##TO##64 __attribute__((overloadable)) FUNC(_8##TI##64 x)\
	{\
		_8##TO##64 res;\
		res.lo.lo = FUNC((_2##TI##64)x.lo.lo);\
		res.lo.hi = FUNC((_2##TI##64)x.lo.hi);\
		res.hi.lo = FUNC((_2##TI##64)x.hi.lo);\
		res.hi.hi = FUNC((_2##TI##64)x.hi.hi);\
		return res;\
	}\
	_16##TO##64 __attribute__((overloadable)) FUNC(_16##TI##64 x)\
	{\
		_16##TO##64 res;\
		res.lo.lo.lo = FUNC(x.lo.lo.lo);\
		res.lo.lo.hi = FUNC(x.lo.lo.hi);\
		res.lo.hi.lo = FUNC(x.lo.hi.lo);\
		res.lo.hi.hi = FUNC(x.lo.hi.hi);\
		res.hi.lo.lo = FUNC(x.hi.lo.lo);\
		res.hi.lo.hi = FUNC(x.hi.lo.hi);\
		res.hi.hi.lo = FUNC(x.hi.hi.lo);\
		res.hi.hi.hi = FUNC(x.hi.hi.hi);\
		return res;\
	}

#define DEF_INT_PROTO8_X_X_Y(FUNC, TI0, TI1, TO)\
	_2##TO##8 __attribute__((overloadable)) FUNC(_2##TI0##8 x, _2##TI1##8 y)\
	{\
		_2##TO##8 res;\
		res.s0 = FUNC(x.s0, y.s0);\
		res.s1 = FUNC(x.s1, y.s1);\
		return res;\
	}\
	_3##TO##8 __attribute__((overloadable)) FUNC(_3##TI0##8 x, _3##TI1##8 y)\
	{\
		_16##TI0##8 tmpX;\
		_16##TI1##8 tmpY;\
		tmpX.s012 = x;\
		tmpY.s012 = y;\
		_16##TO##8 res = FUNC(tmpX, tmpY);\
		return res.s012;\
	}\
	_4##TO##8 __attribute__((overloadable)) FUNC(_4##TI0##8 x, _4##TI1##8 y)\
	{\
		_16##TI0##8 tmpX;\
		_16##TI1##8 tmpY;\
		tmpX.s0123 = x;\
		tmpY.s0123 = y;\
		_16##TO##8 res = FUNC(tmpX, tmpY);\
		return res.s0123;\
	}\
	_8##TO##8 __attribute__((overloadable)) FUNC(_8##TI0##8 x, _8##TI1##8 y)\
	{\
		_16##TI0##8 tmpX;\
		_16##TI1##8 tmpY;\
		tmpX.lo = x;\
		tmpY.lo = y;\
		_16##TO##8 res = FUNC(tmpX, tmpY);\
		return res.lo;\
	}

#define DEF_INT_PROTO16_X_X_Y(FUNC, TI0, TI1, TO)\
	_2##TO##16 __attribute__((overloadable)) FUNC(_2##TI0##16 x, _2##TI1##16 y)\
	{\
		_8##TI0##16 tmpX;\
		_8##TI1##16 tmpY;\
		tmpX.s01 = x;\
		tmpY.s01 = y;\
		_8##TO##16 res = FUNC(tmpX, tmpY);\
		return res.s01;\
	}\
	_3##TO##16 __attribute__((overloadable)) FUNC(_3##TI0##16 x, _3##TI1##16 y)\
	{\
		_8##TI0##16 tmpX;\
		_8##TI1##16 tmpY;\
		tmpX.s012 = x;\
		tmpY.s012 = y;\
		_8##TO##16 res = FUNC(tmpX, tmpY);\
		return res.s012;\
	}\
	_4##TO##16 __attribute__((overloadable)) FUNC(_4##TI0##16 x, _4##TI1##16 y)\
	{\
		_8##TI0##16 tmpX;\
		_8##TI1##16 tmpY;\
		tmpX.lo = x;\
		tmpY.lo = y;\
		_8##TO##16 res = FUNC(tmpX, tmpY);\
		return res.lo;\
	}\
	_16##TO##16 __attribute__((overloadable)) FUNC(_16##TI0##16 x, _16##TI1##16 y)\
	{\
		_16##TO##16 res;\
		res.lo = FUNC(x.lo, y.lo);\
		res.hi = FUNC(x.hi, y.hi);\
		return res;\
	}

#define DEF_INT_PROTO32_X_X_Y(FUNC, TI0, TI1, TO)\
	_2##TO##32 __attribute__((overloadable)) FUNC(_2##TI0##32 x, _2##TI1##32 y)\
	{\
		_4##TI0##32 tmpX;\
		_4##TI1##32 tmpY;\
		tmpX.lo = x;\
		tmpY.lo = y;\
		_4##TO##32 res = FUNC(tmpX, tmpY);\
		return res.lo;\
	}\
	_3##TO##32 __attribute__((overloadable)) FUNC(_3##TI0##32 x, _3##TI1##32 y)\
	{\
		_4##TI0##32 tmpX;\
		_4##TI1##32 tmpY;\
		tmpX.s012 = x;\
		tmpY.s012 = y;\
		_4##TO##32 res = FUNC(tmpX, tmpY);\
		return res.s012;\
	}\
	_8##TO##32 __attribute__((overloadable)) FUNC(_8##TI0##32 x, _8##TI1##32 y)\
	{\
		_8##TO##32 res;\
		res.lo = FUNC(x.lo, y.lo);\
		res.hi = FUNC(x.hi, y.hi);\
		return res;\
	}\
	_16##TO##32 __attribute__((overloadable)) FUNC(_16##TI0##32 x, _16##TI1##32 y)\
	{\
		_16##TO##32 res;\
		res.lo.lo = FUNC(x.lo.lo, y.lo.lo);\
		res.lo.hi = FUNC(x.lo.hi, y.lo.hi);\
		res.hi.lo = FUNC(x.hi.lo, y.hi.lo);\
		res.hi.hi = FUNC(x.hi.hi, y.hi.hi);\
		return res;\
	}

#define DEF_INT_PROTO64_X_X_Y(FUNC, TI0, TI1, TO)\
	_3##TO##64 __attribute__((overloadable)) FUNC(_3##TI0##64 x, _3##TI1##64 y)\
	{\
		_3##TO##64 res;\
		res.s01 = FUNC(x.s01, y.s01);\
		res.s2 = FUNC(x.s2, y.s2);\
		return res;\
	}\
	_4##TO##64 __attribute__((overloadable)) FUNC(_4##TI0##64 x, _4##TI1##64 y)\
	{\
		_4##TO##64 res;\
		res.lo = FUNC(x.lo, y.lo);\
		res.hi = FUNC(x.hi, y.hi);\
		return res;\
	}\
	_8##TO##64 __attribute__((overloadable)) FUNC(_8##TI0##64 x, _8##TI1##64 y)\
	{\
		_8##TO##64 res;\
		res.lo.lo = FUNC(x.lo.lo, y.lo.lo);\
		res.lo.hi = FUNC(x.lo.hi, y.lo.hi);\
		res.hi.lo = FUNC(x.hi.lo, y.hi.lo);\
		res.hi.hi = FUNC(x.hi.hi, y.hi.hi);\
		return res;\
	}\
	_16##TO##64 __attribute__((overloadable)) FUNC(_16##TI0##64 x, _16##TI1##64 y)\
	{\
		_16##TO##64 res;\
		res.lo.lo.lo = FUNC(x.lo.lo.lo, y.lo.lo.lo);\
		res.lo.lo.hi = FUNC(x.lo.lo.hi, y.lo.lo.hi);\
		res.lo.hi.lo = FUNC(x.lo.hi.lo, y.lo.hi.lo);\
		res.lo.hi.hi = FUNC(x.lo.hi.hi, y.lo.hi.hi);\
		res.hi.lo.lo = FUNC(x.hi.lo.lo, y.hi.lo.lo);\
		res.hi.lo.hi = FUNC(x.hi.lo.hi, y.hi.lo.hi);\
		res.hi.hi.lo = FUNC(x.hi.hi.lo, y.hi.hi.lo);\
		res.hi.hi.hi = FUNC(x.hi.hi.hi, y.hi.hi.hi);\
		return res;\
	}


#define DEF_INT_PROTO8_vX_sX_Y(FUNC, TI0, TI1, TO)\
	_2##TO##8 __attribute__((overloadable)) FUNC(_2##TI0##8 x, _1##TI1##8 y)\
	{\
		_2##TO##8 res;\
		res.s0 = FUNC(x.s0, y);\
		res.s1 = FUNC(x.s1, y);\
		return res;\
	}\
	_3##TO##8 __attribute__((overloadable)) FUNC(_3##TI0##8 x, _1##TI1##8 y)\
	{\
		_16##TI0##8 tmpX;\
		_16##TI1##8 tmpY;\
		tmpX.s012 = x;\
		tmpY.s012 = (_3##TI0##8)y;\
		_16##TO##8 res = FUNC(tmpX, tmpY);\
		return res.s012;\
	}\
	_4##TO##8 __attribute__((overloadable)) FUNC(_4##TI0##8 x, _1##TI1##8 y)\
	{\
		_16##TI0##8 tmpX;\
		_16##TI1##8 tmpY;\
		tmpX.s0123 = x;\
		tmpY.s0123 = (_4##TI0##8)y;\
		_16##TO##8 res = FUNC(tmpX, tmpY);\
		return res.s0123;\
	}\
	_8##TO##8 __attribute__((overloadable)) FUNC(_8##TI0##8 x, _1##TI1##8 y)\
	{\
		_16##TI0##8 tmpX;\
		_16##TI1##8 tmpY;\
		tmpX.lo = x;\
		tmpY.lo = (_8##TI0##8)y;\
		_16##TO##8 res = FUNC(tmpX, tmpY);\
		return res.lo;\
	}\
	_16##TO##8 __attribute__((overloadable)) FUNC(_16##TI0##8 x, _1##TI1##8 y)\
	{\
		_16##TI1##8 tmpY;\
		tmpY = (_16##TI0##8)y;\
		return FUNC(x, tmpY);\
	}

#define DEF_INT_PROTO16_vX_sX_Y(FUNC, TI0, TI1, TO)\
	_2##TO##16 __attribute__((overloadable)) FUNC(_2##TI0##16 x, _1##TI1##16 y)\
	{\
		_8##TI0##16 tmpX;\
		_8##TI1##16 tmpY;\
		tmpX.s01 = x;\
		tmpY.s01 = (_2##TI0##16)y;\
		_8##TO##16 res = FUNC(tmpX, tmpY);\
		return res.s01;\
	}\
	_3##TO##16 __attribute__((overloadable)) FUNC(_3##TI0##16 x, _1##TI1##16 y)\
	{\
		_8##TI0##16 tmpX;\
		_8##TI1##16 tmpY;\
		tmpX.s012 = x;\
		tmpY.s012 = (_3##TI0##16)y;\
		_8##TO##16 res = FUNC(tmpX, tmpY);\
		return res.s012;\
	}\
	_4##TO##16 __attribute__((overloadable)) FUNC(_4##TI0##16 x, _1##TI1##16 y)\
	{\
		_8##TI0##16 tmpX;\
		_8##TI1##16 tmpY;\
		tmpX.lo = x;\
		tmpY.lo = (_4##TI0##16)y;\
		_8##TO##16 res = FUNC(tmpX, tmpY);\
		return res.lo;\
	}\
	_8##TO##16 __attribute__((overloadable)) FUNC(_8##TI0##16 x, _1##TI1##16 y)\
	{\
		_8##TO##16 tmpY;\
		tmpY = (_8##TO##16)y;\
		return FUNC(x, tmpY);\
	}\
	_16##TO##16 __attribute__((overloadable)) FUNC(_16##TI0##16 x, _1##TI1##16 y)\
	{\
		_16##TO##16 res;\
		_16##TO##16 tmpY;\
		tmpY = (_16##TO##16)y;\
		res.lo = FUNC(x.lo, tmpY.lo);\
		res.hi = FUNC(x.hi, tmpY.hi);\
		return res;\
	}

#define DEF_INT_PROTO32_vX_sX_Y(FUNC, TI0, TI1, TO)\
	_2##TO##32 __attribute__((overloadable)) FUNC(_2##TI0##32 x, _1##TI1##32 y)\
	{\
		_4##TI0##32 tmpX;\
		_4##TI1##32 tmpY;\
		tmpX.lo = x;\
		tmpY.lo = (_2##TO##32)y;\
		_4##TO##32 res = FUNC(tmpX, tmpY);\
		return res.lo;\
	}\
	_3##TO##32 __attribute__((overloadable)) FUNC(_3##TI0##32 x, _1##TI1##32 y)\
	{\
		_4##TI0##32 tmpX;\
		_4##TI1##32 tmpY;\
		tmpX.s012 = x;\
		tmpY.s012 = (_3##TO##32)y;\
		_4##TO##32 res = FUNC(tmpX, tmpY);\
		return res.s012;\
	}\
	_4##TO##32 __attribute__((overloadable)) FUNC(_4##TI0##32 x, _1##TI1##32 y)\
	{\
		_4##TO##32 tmpY;\
		tmpY = (_4##TO##32)y;\
		return FUNC(x, tmpY);\
	}\
	_8##TO##32 __attribute__((overloadable)) FUNC(_8##TI0##32 x, _1##TI1##32 y)\
	{\
		_8##TO##32 res;\
		_4##TO##32 tmpY;\
		tmpY = (_4##TO##32)y;\
		res.lo = FUNC(x.lo, tmpY);\
		res.hi = FUNC(x.hi, tmpY);\
		return res;\
	}\
	_16##TO##32 __attribute__((overloadable)) FUNC(_16##TI0##32 x, _1##TI1##32 y)\
	{\
		_16##TO##32 res;\
		_4##TO##32 tmpY;\
		tmpY = (_4##TO##32)y;\
		res.lo.lo = FUNC(x.lo.lo, tmpY);\
		res.lo.hi = FUNC(x.lo.hi, tmpY);\
		res.hi.lo = FUNC(x.hi.lo, tmpY);\
		res.hi.hi = FUNC(x.hi.hi, tmpY);\
		return res;\
	}

#define DEF_INT_PROTO64_vX_sX_Y(FUNC, TI0, TI1, TO)\
	_2##TO##64 __attribute__((overloadable)) FUNC(_2##TI0##64 x, _1##TI1##64 y)\
	{\
		return FUNC(x,(_2##TO##64)y);\
	}\
	_3##TO##64 __attribute__((overloadable)) FUNC(_3##TI0##64 x, _1##TI1##64 y)\
	{\
		_3##TO##64 res;\
		res.s01 = FUNC(x.s01, (_2##TO##64)y);\
		res.s2 = FUNC(x.s2, y);\
		return res;\
	}\
	_4##TO##64 __attribute__((overloadable)) FUNC(_4##TI0##64 x, _1##TI1##64 y)\
	{\
		_4##TO##64 res;\
		res.lo = FUNC(x.lo, (_2##TO##64)y);\
		res.hi = FUNC(x.hi, (_2##TO##64)y);\
		return res;\
	}\
	_8##TO##64 __attribute__((overloadable)) FUNC(_8##TI0##64 x, _1##TI1##64 y)\
	{\
		_8##TO##64 res;\
		_2##TO##64 tmpY = (_2##TO##64)y;\
		res.lo.lo = FUNC(x.lo.lo, tmpY);\
		res.lo.hi = FUNC(x.lo.hi, tmpY);\
		res.hi.lo = FUNC(x.hi.lo, tmpY);\
		res.hi.hi = FUNC(x.hi.hi, tmpY);\
		return res;\
	}\
	_16##TO##64 __attribute__((overloadable)) FUNC(_16##TI0##64 x, _1##TI1##64 y)\
	{\
		_16##TO##64 res;\
		_2##TO##64 tmpY = (_2##TO##64)y;\
		res.lo.lo.lo = FUNC(x.lo.lo.lo, tmpY);\
		res.lo.lo.hi = FUNC(x.lo.lo.hi, tmpY);\
		res.lo.hi.lo = FUNC(x.lo.hi.lo, tmpY);\
		res.lo.hi.hi = FUNC(x.lo.hi.hi, tmpY);\
		res.hi.lo.lo = FUNC(x.hi.lo.lo, tmpY);\
		res.hi.lo.hi = FUNC(x.hi.lo.hi, tmpY);\
		res.hi.hi.lo = FUNC(x.hi.hi.lo, tmpY);\
		res.hi.hi.hi = FUNC(x.hi.hi.hi, tmpY);\
		return res;\
	}



#define DEF_INT_PROTO8_X_X_X_Y(FUNC, TI0, TI1, TI2, TO)\
	_2##TO##8 __attribute__((overloadable)) FUNC(_2##TI0##8 x, _2##TI1##8 y,  _2##TI2##8 z)\
	{\
		_2##TO##8 res;\
		res.s0 = FUNC(x.s0, y.s0, z.s0);\
		res.s1 = FUNC(x.s1, y.s1, z.s1);\
		return res;\
	}\
	_3##TO##8 __attribute__((overloadable)) FUNC(_3##TI0##8 x, _3##TI1##8 y, _3##TI2##8 z)\
	{\
		_16##TI0##8 temp1, temp2, temp3;\
		temp1.s012 = x;\
		temp2.s012 = y;\
		temp3.s012 = z;\
		_16##TO##8 res;\
		res = FUNC(temp1, temp2, temp3);\
		return res.s012;\
	}\
	_4##TO##8 __attribute__((overloadable)) FUNC(_4##TI0##8 x, _4##TI1##8 y, _4##TI2##8 z)\
	{\
		_16##TI0##8 temp1, temp2, temp3;\
		temp1.s0123 = x;\
		temp2.s0123 = y;\
		temp3.s0123 = z;\
		_16##TO##8 res;\
		res = FUNC(temp1, temp2, temp3);\
		return res.s0123;\
	}\
	_8##TO##8 __attribute__((overloadable)) FUNC(_8##TI0##8 x, _8##TI1##8 y, _8##TI2##8 z)\
	{\
		_16##TI0##8 temp1, temp2, temp3;\
		temp1.lo = x;\
		temp2.lo = y;\
		temp3.lo = z;\
		_16##TO##8 res;\
		res = FUNC(temp1, temp2, temp3);\
		return res.lo;\
	}\

#define DEF_INT_PROTO16_X_X_X_Y(FUNC, TI0, TI1, TI2, TO)\
	_2##TO##16 __attribute__((overloadable)) FUNC(_2##TI0##16 x, _2##TI1##16 y, _2##TI2##16 z)\
	{\
		_8##TI0##16 temp1, temp2, temp3;\
		temp1.s01 = x;\
		temp2.s01 = y;\
		temp3.s01 = z;\
		_8##TO##16 res;\
		res = FUNC(temp1, temp2, temp3);\
		return res.s01;\
	}\
	_3##TO##16 __attribute__((overloadable)) FUNC(_3##TI0##16 x, _3##TI1##16 y, _3##TI2##16 z)\
		{\
		_8##TI0##16 temp1, temp2, temp3;\
		temp1.s012 = x;\
		temp2.s012 = y;\
		temp3.s012 = z;\
		_8##TO##16 res;\
		res = FUNC(temp1, temp2, temp3);\
		return res.s012;\
	}\
	_4##TO##16 __attribute__((overloadable)) FUNC(_4##TI0##16 x, _4##TI1##16 y, _4##TI2##16 z)\
		{\
		_8##TI0##16 temp1, temp2, temp3;\
		temp1.lo = x;\
		temp2.lo = y;\
		temp3.lo = z;\
		_8##TO##16 res;\
		res = FUNC(temp1, temp2, temp3);\
		return res.lo;\
	}\
	_16##TO##16 __attribute__((overloadable)) FUNC(_16##TI0##16 x, _16##TI1##16 y, _16##TI2##16 z)\
	{\
		_16##TO##16 res;\
		res.lo = FUNC(x.lo, y.lo, z.lo);\
		res.hi = FUNC(x.hi, y.hi, z.hi);\
		return res;\
	}

#define DEF_INT_PROTO32_X_X_X_Y(FUNC, TI0, TI1, TI2, TO)\
	_2##TO##32 __attribute__((overloadable)) FUNC(_2##TI0##32 x, _2##TI1##32 y, _2##TI2##32 z)\
	{\
		_4##TO##32 tmp1, tmp2, tmp3;\
		tmp1.lo = x;\
		tmp2.lo = y;\
		tmp3.lo = z;\
		_4##TO##32 res = FUNC(tmp1, tmp2, tmp3);\
		return res.lo;\
	}\
	_3##TO##32 __attribute__((overloadable)) FUNC(_3##TI0##32 x, _3##TI1##32 y, _3##TI2##32 z)\
	{\
		_4##TO##32 tmp1, tmp2, tmp3;\
		tmp1.s012 = x;\
		tmp2.s012 = y;\
		tmp3.s012 = z;\
		_4##TO##32 res = FUNC(tmp1, tmp2, tmp3);\
		return res.s012;\
	}\
	_8##TO##32 __attribute__((overloadable)) FUNC(_8##TI0##32 x, _8##TI1##32 y, _8##TI2##32 z)\
	{\
		_8##TO##32 res;\
		res.lo = FUNC(x.lo, y.lo, z.lo);\
		res.hi = FUNC(x.hi, y.hi, z.hi);\
		return res;\
	}\
	_16##TO##32 __attribute__((overloadable)) FUNC(_16##TI0##32 x, _16##TI1##32 y, _16##TI2##32 z)\
	{\
		_16##TO##32 res;\
		res.lo.lo = FUNC(x.lo.lo, y.lo.lo, z.lo.lo);\
		res.lo.hi = FUNC(x.lo.hi, y.lo.hi, z.lo.hi);\
		res.hi.lo = FUNC(x.hi.lo, y.hi.lo, z.hi.lo);\
		res.hi.hi = FUNC(x.hi.hi, y.hi.hi, z.hi.hi);\
		return res;\
	}

#define DEF_INT_PROTO64_X_X_X_Y(FUNC, TI0, TI1, TI2, TO)\
	_3##TO##64 __attribute__((overloadable)) FUNC(_3##TI0##64 x, _3##TI1##64 y, _3##TI2##64 z)\
	{\
		_3##TO##64 res;\
		res.s01 = FUNC(x.s01, y.s01, z.s01);\
		res.s2 = FUNC(x.s2, y.s2, z.s2);\
		return res;\
	}\
	_4##TO##64 __attribute__((overloadable)) FUNC(_4##TI0##64 x, _4##TI1##64 y, _4##TI2##64 z)\
	{\
		_4##TO##64 res;\
		res.lo = FUNC(x.lo, y.lo, z.lo);\
		res.hi = FUNC(x.hi, y.hi, z.hi);\
		return res;\
	}\
	_8##TO##64 __attribute__((overloadable)) FUNC(_8##TI0##64 x, _8##TI1##64 y, _8##TI2##64 z)\
	{\
		_8##TO##64 res;\
		res.lo.lo = FUNC(x.lo.lo, y.lo.lo, z.lo.lo);\
		res.lo.hi = FUNC(x.lo.hi, y.lo.hi, z.lo.hi);\
		res.hi.lo = FUNC(x.hi.lo, y.hi.lo, z.hi.lo);\
		res.hi.hi = FUNC(x.hi.hi, y.hi.hi, z.hi.hi);\
		return res;\
	}\
	_16##TO##64 __attribute__((overloadable)) FUNC(_16##TI0##64 x, _16##TI1##64 y, _16##TI2##64 z)\
	{\
		_16##TO##64 res;\
		res.lo.lo.lo = FUNC(x.lo.lo.lo, y.lo.lo.lo, z.lo.lo.lo);\
		res.lo.lo.hi = FUNC(x.lo.lo.hi, y.lo.lo.hi, z.lo.lo.hi);\
		res.lo.hi.lo = FUNC(x.lo.hi.lo, y.lo.hi.lo, z.lo.hi.lo);\
		res.lo.hi.hi = FUNC(x.lo.hi.hi, y.lo.hi.hi, z.lo.hi.hi);\
		res.hi.lo.lo = FUNC(x.hi.lo.lo, y.hi.lo.lo, z.hi.lo.lo);\
		res.hi.lo.hi = FUNC(x.hi.lo.hi, y.hi.lo.hi, z.hi.lo.hi);\
		res.hi.hi.lo = FUNC(x.hi.hi.lo, y.hi.hi.lo, z.hi.hi.lo);\
		res.hi.hi.hi = FUNC(x.hi.hi.hi, y.hi.hi.hi, z.hi.hi.hi);\
		return res;\
	}

#define DEF_INT_PROTO8_X_sX_sX_Y(FUNC, TI0, TI1, TI2, TO)\
	_2##TO##8 __attribute__((overloadable)) FUNC(_2##TI0##8 x, _1##TI1##8 y,  _1##TI2##8 z)\
	{\
		_2##TO##8 res;\
		res.s0 = FUNC(x.s0, y, z);\
		res.s1 = FUNC(x.s1, y, z);\
		return res;\
	}\
	_3##TO##8 __attribute__((overloadable)) FUNC(_3##TI0##8 x, _1##TI1##8 y, _1##TI2##8 z)\
	{\
		_16##TI0##8 temp1, temp2, temp3;\
		temp1.s012 = x;\
		temp2 = y;\
		temp3 = z;\
		_16##TO##8 res;\
		res = FUNC(temp1, temp2, temp3);\
		return res.s012;\
	}\
	_4##TO##8 __attribute__((overloadable)) FUNC(_4##TI0##8 x, _1##TI1##8 y, _1##TI2##8 z)\
	{\
		_16##TI0##8 temp1, temp2, temp3;\
		temp1.s0123 = x;\
		temp2 = y;\
		temp3 = z;\
		_16##TO##8 res;\
		res = FUNC(temp1, temp2, temp3);\
		return res.s0123;\
	}\
	_8##TO##8 __attribute__((overloadable)) FUNC(_8##TI0##8 x, _1##TI1##8 y, _1##TI2##8 z)\
	{\
		_16##TI0##8 temp1, temp2, temp3;\
		temp1.lo = x;\
		temp2 = y;\
		temp3 = z;\
		_16##TO##8 res;\
		res = FUNC(temp1, temp2, temp3);\
		return res.lo;\
	}\
	_16##TO##8 __attribute__((overloadable)) FUNC(_16##TI0##8 x, _1##TI1##8 y, _1##TI2##8 z)\
	{\
		_16##TI0##8 temp2, temp3;\
		temp2 = y;\
		temp3 = z;\
		_16##TO##8 res;\
		res = FUNC(x, temp2, temp3);\
		return res;\
	}

#define DEF_INT_PROTO16_X_sX_sX_Y(FUNC, TI0, TI1, TI2, TO)\
	_2##TO##16 __attribute__((overloadable)) FUNC(_2##TI0##16 x, _1##TI1##16 y, _1##TI2##16 z)\
	{\
		_8##TI0##16 temp1, temp2, temp3;\
		temp1.s01 = x;\
		temp2 = y;\
		temp3 = z;\
		_8##TO##16 res;\
		res = FUNC(temp1, temp2, temp3);\
		return res.s01;\
	}\
	_3##TO##16 __attribute__((overloadable)) FUNC(_3##TI0##16 x, _1##TI1##16 y, _1##TI2##16 z)\
		{\
		_8##TI0##16 temp1, temp2, temp3;\
		temp1.s012 = x;\
		temp2 = y;\
		temp3 = z;\
		_8##TO##16 res;\
		res = FUNC(temp1, temp2, temp3);\
		return res.s012;\
	}\
	_4##TO##16 __attribute__((overloadable)) FUNC(_4##TI0##16 x, _1##TI1##16 y, _1##TI2##16 z)\
		{\
		_8##TI0##16 temp1, temp2, temp3;\
		temp1.lo = x;\
		temp2 = y;\
		temp3 = z;\
		_8##TO##16 res;\
		res = FUNC(temp1, temp2, temp3);\
		return res.lo;\
	}\
	_8##TO##16 __attribute__((overloadable)) FUNC(_8##TI0##16 x, _1##TI1##16 y, _1##TI2##16 z)\
	{\
		_8##TI0##16 temp1, temp2;\
		temp1 = y;\
		temp2 = z;\
		return FUNC(x, temp1, temp2);\
	}\
	_16##TO##16 __attribute__((overloadable)) FUNC(_16##TI0##16 x, _1##TI1##16 y, _1##TI2##16 z)\
	{\
		_8##TI0##16 temp1, temp2;\
		_16##TO##16 res;\
		temp1 = y;\
		temp2 = z;\
		res.lo = FUNC(x.lo, temp1, temp2);\
		res.hi = FUNC(x.hi, temp1, temp2);\
		return res;\
	}

#define DEF_INT_PROTO32_X_sX_sX_Y(FUNC, TI0, TI1, TI2, TO)\
	_2##TO##32 __attribute__((overloadable)) FUNC(_2##TI0##32 x, _1##TI1##32 y, _1##TI2##32 z)\
	{\
		_4##TO##32 tmp1, tmp2, tmp3;\
		tmp1.lo = x;\
		tmp2 = y;\
		tmp3 = z;\
		_4##TO##32 res = FUNC(tmp1, tmp2, tmp3);\
		return res.lo;\
	}\
	_3##TO##32 __attribute__((overloadable)) FUNC(_3##TI0##32 x, _1##TI1##32 y, _1##TI2##32 z)\
	{\
		_4##TO##32 tmp1, tmp2, tmp3;\
		tmp1.s012 = x;\
		tmp2 = y;\
		tmp3 = z;\
		_4##TO##32 res = FUNC(tmp1, tmp2, tmp3);\
		return res.s012;\
	}\
	_4##TO##32 __attribute__((overloadable)) FUNC(_4##TI0##32 x, _1##TI1##32 y, _1##TI2##32 z)\
	{\
		_4##TO##32 tmp1, tmp2;\
		tmp1 = y;\
		tmp2 = z;\
		_4##TO##32 res = FUNC(x, tmp1, tmp2);\
		return res;\
	}\
	_8##TO##32 __attribute__((overloadable)) FUNC(_8##TI0##32 x, _1##TI1##32 y, _1##TI2##32 z)\
	{\
		_8##TO##32 res;\
		_4##TO##32 tmp1, tmp2;\
		tmp1 = y;\
		tmp2 = z;\
		res.lo =FUNC(x.lo, tmp1, tmp2);\
		res.hi =FUNC(x.hi, tmp1, tmp2);\
		return res;\
	}\
	_16##TO##32 __attribute__((overloadable)) FUNC(_16##TI0##32 x, _1##TI1##32 y, _1##TI2##32 z)\
	{\
		_16##TO##32 res;\
		_4##TO##32 tmp1, tmp2;\
		tmp1 = y;\
		tmp2 = z;\
		res.lo.lo = FUNC(x.lo.lo, tmp1, tmp2);\
		res.lo.hi = FUNC(x.lo.hi, tmp1, tmp2);\
		res.hi.lo = FUNC(x.hi.lo, tmp1, tmp2);\
		res.hi.hi = FUNC(x.hi.hi, tmp1, tmp2);\
		return res;\
	}

#define DEF_INT_PROTO64_X_sX_sX_Y(FUNC, TI0, TI1, TI2, TO)\
	_2##TO##64 __attribute__((overloadable)) FUNC(_2##TI0##64 x, _1##TI1##64 y, _1##TI2##64 z)\
	{\
		_2##TI1##64 tmpY;\
		_2##TI2##64 tmpZ;\
		tmpY = y;\
		tmpZ = z;\
		return FUNC(x, tmpY, tmpZ);\
	}\
	_3##TO##64 __attribute__((overloadable)) FUNC(_3##TI0##64 x, _1##TI1##64 y, _1##TI2##64 z)\
	{\
		_3##TO##64 res;\
		_2##TI1##64 tmpY;\
		_2##TI2##64 tmpZ;\
		tmpY = y;\
		tmpZ = z;\
		res.s01 = FUNC(x.s01, tmpY, tmpZ);\
		res.s2 = FUNC(x.s2, y, z);\
		return res;\
	}\
	_4##TO##64 __attribute__((overloadable)) FUNC(_4##TI0##64 x, _1##TI1##64 y, _1##TI2##64 z)\
	{\
		_4##TO##64 res;\
		_2##TI1##64 tmpY;\
		_2##TI2##64 tmpZ;\
		tmpY = y;\
		tmpZ = z;\
		res.lo = FUNC(x.lo, tmpY, tmpZ);\
		res.hi = FUNC(x.hi, tmpY, tmpZ);\
		return res;\
	}\
	_8##TO##64 __attribute__((overloadable)) FUNC(_8##TI0##64 x, _1##TI1##64 y, _1##TI2##64 z)\
	{\
		_8##TO##64 res;\
		_2##TI1##64 tmpY;\
		_2##TI2##64 tmpZ;\
		tmpY = y;\
		tmpZ = z;\
		res.lo.lo = FUNC(x.lo.lo, tmpY, tmpZ);\
		res.lo.hi = FUNC(x.lo.hi, tmpY, tmpZ);\
		res.hi.lo = FUNC(x.hi.lo, tmpY, tmpZ);\
		res.hi.hi = FUNC(x.hi.hi, tmpY, tmpZ);\
		return res;\
	}\
	_16##TO##64 __attribute__((overloadable)) FUNC(_16##TI0##64 x, _1##TI1##64 y, _1##TI2##64 z)\
	{\
		_16##TO##64 res;\
		_2##TI1##64 tmpY;\
		_2##TI2##64 tmpZ;\
		tmpY = y;\
		tmpZ = z;\
		res.lo.lo.lo = FUNC(x.lo.lo.lo, tmpY, tmpZ);\
		res.lo.lo.hi = FUNC(x.lo.lo.hi, tmpY, tmpZ);\
		res.lo.hi.lo = FUNC(x.lo.hi.lo, tmpY, tmpZ);\
		res.lo.hi.hi = FUNC(x.lo.hi.hi, tmpY, tmpZ);\
		res.hi.lo.lo = FUNC(x.hi.lo.lo, tmpY, tmpZ);\
		res.hi.lo.hi = FUNC(x.hi.lo.hi, tmpY, tmpZ);\
		res.hi.hi.lo = FUNC(x.hi.hi.lo, tmpY, tmpZ);\
		res.hi.hi.hi = FUNC(x.hi.hi.hi, tmpY, tmpZ);\
		return res;\
	}

#define DEF_UPSMPL_PROTO8_X_U_Y(FUNC, TI)\
	_2##TI##16 __attribute__((overloadable)) FUNC(_2##TI##8 x, _2u8 y)\
	{\
		_2##TI##16 res;\
		res.s0 = FUNC(x.s0, y.s0);\
		res.s1 = FUNC(x.s1, y.s1);\
		return res;\
	}\
	_3##TI##16 __attribute__((overloadable)) FUNC(_3##TI##8 x, _3u8 y)\
	{\
		_3##TI##16 res;\
		res.s0 = FUNC(x.s0, y.s0);\
		res.s1 = FUNC(x.s1, y.s1);\
		res.s2 = FUNC(x.s2, y.s2);\
		return res;\
	}\
	_4##TI##16 __attribute__((overloadable)) FUNC(_4##TI##8 x, _4u8 y)\
	{\
		_4##TI##16 res;\
		res.s0 = FUNC(x.s0, y.s0);\
		res.s1 = FUNC(x.s1, y.s1);\
		res.s2 = FUNC(x.s2, y.s2);\
		res.s3 = FUNC(x.s3, y.s3);\
		return res;\
	}\
	_8##TI##16 __attribute__((overloadable)) FUNC(_8##TI##8 x, _8u8 y)\
		{\
		_16##TI##8 tmp1;\
		_16u8 tmp2;\
		_16##TI##16 res;\
		tmp1.lo = x;\
		tmp2.lo = y;\
		res = FUNC(tmp1, tmp2);\
		return res.lo;\
	}

#define DEF_UPSMPL_PROTO16_X_U_Y(FUNC, TI)\
	_2##TI##32 __attribute__((overloadable)) FUNC(_2##TI##16 x, _2u16 y)\
	{\
		_2##TI##32 res;\
		res.s0 = FUNC(x.s0, y.s0);\
		res.s1 = FUNC(x.s1, y.s1);\
		return res;\
	}\
	_3##TI##32 __attribute__((overloadable)) FUNC(_3##TI##16 x, _3u16 y)\
	{\
		_8##TI##16 tmp1;\
		_8u16 tmp2;\
		_8##TI##32 res;\
		tmp1.s012 = x;\
		tmp2.s012 = y;\
		res = FUNC(tmp1, tmp2);\
		return res.s012;\
	}\
	_4##TI##32 __attribute__((overloadable)) FUNC(_4##TI##16 x, _4u16 y)\
	{\
		_8##TI##16 tmp1;\
		_8u16 tmp2;\
		_8##TI##32 res;\
		tmp1.s0123 = x;\
		tmp2.s0123 = y;\
		res = FUNC(tmp1, tmp2);\
		return res.lo;\
	}\
	_16##TI##32 __attribute__((overloadable)) FUNC(_16##TI##16 x, _16u16 y)\
	{\
		_16##TI##32 res;\
		res.lo = FUNC(x.lo, y.lo);\
		res.hi = FUNC(x.hi, y.hi);\
		return res;\
	}

#define DEF_UPSMPL_PROTO32_X_U_Y(FUNC, TI)\
	_2##TI##64 __attribute__((overloadable)) FUNC(_2##TI##32 x, _2u32 y)\
	{\
		_4##TI##32 tmp1;\
		_4u32 tmp2;\
		_4##TI##64 res;\
		tmp1.s01 = x;\
		tmp2.s01 = y;\
		res = FUNC(tmp1, tmp2);\
		return res.lo;\
	}\
	_3##TI##64 __attribute__((overloadable)) FUNC(_3##TI##32 x, _3u32 y)\
	{\
		_4##TI##32 tmp1;\
		_4u32 tmp2;\
		_4##TI##64 res;\
		tmp1.s012 = x;\
		tmp2.s012 = y;\
		res = FUNC(tmp1, tmp2);\
		return res.s012;\
	}\
	_8##TI##64 __attribute__((overloadable)) FUNC(_8##TI##32 x, _8u32 y)\
	{\
		_8##TI##64 res;\
		res.lo = FUNC(x.lo, y.lo);\
		res.hi = FUNC(x.hi, y.hi);\
		return res;\
	}\
	_16##TI##64 __attribute__((overloadable)) FUNC(_16##TI##32 x, _16u32 y)\
	{\
		_16##TI##64 res;\
		res.lo.lo = FUNC(x.lo.lo, y.lo.lo);\
		res.lo.hi = FUNC(x.lo.hi, y.lo.hi);\
		res.hi.lo = FUNC(x.hi.lo, y.hi.lo);\
		res.hi.hi = FUNC(x.hi.hi, y.hi.hi);\
		return res;\
	}

#define DEF_BUILT_IN_PROTO_ALL_VECTORS_X_Y(FUNC, TI, TO)\
	_2##TO __attribute__((overloadable)) FUNC(_2##TI x)\
	{\
		_2##TO res;\
		res.s0 = FUNC(x.s0);\
		res.s1 = FUNC(x.s1);\
		return res;\
	}\
	_3##TO __attribute__((overloadable)) FUNC(_3##TI x)\
	{\
		_3##TO res;\
		res.s0 = FUNC(x.s0);\
		res.s1 = FUNC(x.s1);\
		res.s2 = FUNC(x.s2);\
		return res;\
	}\
	_4##TO __attribute__((overloadable)) FUNC(_4##TI x)\
	{\
		_4##TO res;\
		res.s0 = FUNC(x.s0);\
		res.s1 = FUNC(x.s1);\
		res.s2 = FUNC(x.s2);\
		res.s3 = FUNC(x.s3);\
		return res;\
	}\
	_8##TO __attribute__((overloadable)) FUNC(_8##TI x)\
	{\
		_8##TO res;\
		res.s0 = FUNC(x.s0);\
		res.s1 = FUNC(x.s1);\
		res.s2 = FUNC(x.s2);\
		res.s3 = FUNC(x.s3);\
		res.s4 = FUNC(x.s4);\
		res.s5 = FUNC(x.s5);\
		res.s6 = FUNC(x.s6);\
		res.s7 = FUNC(x.s7);\
		return res;\
	}\
	_16##TO __attribute__((overloadable)) FUNC(_16##TI x)\
	{\
		_16##TO res;\
		res.s0 = FUNC(x.s0);\
		res.s1 = FUNC(x.s1);\
		res.s2 = FUNC(x.s2);\
		res.s3 = FUNC(x.s3);\
		res.s4 = FUNC(x.s4);\
		res.s5 = FUNC(x.s5);\
		res.s6 = FUNC(x.s6);\
		res.s7 = FUNC(x.s7);\
		res.s8 = FUNC(x.s8);\
		res.s9 = FUNC(x.s9);\
		res.sA = FUNC(x.sA);\
		res.sB = FUNC(x.sB);\
		res.sC = FUNC(x.sC);\
		res.sD = FUNC(x.sD);\
		res.sE = FUNC(x.sE);\
		res.sF = FUNC(x.sF);\
		return res;\
	}


// Full function definition (all integer types)
#define DEF_INT_PROTO_X_Y(FUNC, TI, TO)\
    DEF_INT_PROTO8_X_Y(FUNC, TI, TO)\
    DEF_INT_PROTO16_X_Y(FUNC, TI, TO)\
    DEF_INT_PROTO32_X_Y(FUNC, TI, TO)\
    DEF_INT_PROTO64_X_Y(FUNC, TI, TO)

#define DEF_INT_PROTO_X_X_Y(FUNC, TI0, TI1, TO)\
    DEF_INT_PROTO8_X_X_Y(FUNC, TI0, TI1, TO)\
    DEF_INT_PROTO16_X_X_Y(FUNC, TI0, TI1, TO)\
    DEF_INT_PROTO32_X_X_Y(FUNC, TI0, TI1, TO)\
    DEF_INT_PROTO64_X_X_Y(FUNC, TI0, TI1, TO)

#define DEF_INT_PROTO_vX_sX_Y(FUNC, TI0, TI1, TO)\
    DEF_INT_PROTO8_vX_sX_Y(FUNC, TI0, TI1, TO)\
    DEF_INT_PROTO16_vX_sX_Y(FUNC, TI0, TI1, TO)\
    DEF_INT_PROTO32_vX_sX_Y(FUNC, TI0, TI1, TO)\
    DEF_INT_PROTO64_vX_sX_Y(FUNC, TI0, TI1, TO)


#define DEF_MUL24_PROTO_X_X_Y(FUNC, TI0, TI1, TO)\
    DEF_INT_PROTO32_X_X_Y(FUNC, TI0, TI1, TO)

#define DEF_MAD24_PROTO_X_X_Y(FUNC, TI0, TI1, TI2, TO)\
    DEF_INT_PROTO32_X_X_X_Y(FUNC, TI0, TI1, TI2, TO)

#define DEF_UPSAMPL_PROTO_X_X_Y(FUNC, TI)\
    DEF_UPSMPL_PROTO8_X_U_Y(FUNC, TI)\
    DEF_UPSMPL_PROTO16_X_U_Y(FUNC, TI)\
    DEF_UPSMPL_PROTO32_X_U_Y(FUNC, TI)

#define DEF_INT_PROTO_X_X_X_Y(FUNC, TI0, TI1, TI2, TO)\
    DEF_INT_PROTO8_X_X_X_Y(FUNC, TI0, TI1, TI2, TO)\
    DEF_INT_PROTO16_X_X_X_Y(FUNC, TI0, TI1, TI2, TO)\
    DEF_INT_PROTO32_X_X_X_Y(FUNC, TI0, TI1, TI2, TO)\
    DEF_INT_PROTO64_X_X_X_Y(FUNC, TI0, TI1, TI2, TO)

#define DEF_INT_PROTO_X_sX_sX_Y(FUNC, TI0, TI1, TI2, TO)\
	DEF_INT_PROTO8_X_sX_sX_Y(FUNC, TI0, TI1, TI2, TO)\
	DEF_INT_PROTO16_X_sX_sX_Y(FUNC, TI0, TI1, TI2, TO)\
	DEF_INT_PROTO32_X_sX_sX_Y(FUNC, TI0, TI1, TI2, TO)\
	DEF_INT_PROTO64_X_sX_sX_Y(FUNC, TI0, TI1, TI2, TO)

// Full function definition for built in functions (all vector types)
#define DEF_LLVM_BUILT_IN_PROTO_X_Y(FUNC, TI, TO)\
	DEF_BUILT_IN_PROTO_ALL_VECTORS_X_Y(FUNC, TI##8, TO##8)\
    DEF_BUILT_IN_PROTO_ALL_VECTORS_X_Y(FUNC, TI##16, TO##16)\
    DEF_BUILT_IN_PROTO_ALL_VECTORS_X_Y(FUNC, TI##32, TO##32)\
    DEF_BUILT_IN_PROTO_ALL_VECTORS_X_Y(FUNC, TI##64, TO##64)

DEF_INT_PROTO_X_Y(abs, i, u)
DEF_INT_PROTO_X_Y(abs, u, u)
DEF_INT_PROTO_X_X_Y(abs_diff, i, i, u)
DEF_INT_PROTO_X_X_Y(abs_diff, u, u, u)
DEF_INT_PROTO_X_X_Y(add_sat, i, i, i)
DEF_INT_PROTO_X_X_Y(add_sat, u, u, u)
DEF_INT_PROTO_X_X_Y(hadd, i, i, i)
DEF_INT_PROTO_X_X_Y(hadd, u, u, u)
DEF_INT_PROTO_X_X_Y(rhadd, i, i, i)
DEF_INT_PROTO_X_X_Y(rhadd, u, u, u)
DEF_LLVM_BUILT_IN_PROTO_X_Y(clz, i, i)
DEF_LLVM_BUILT_IN_PROTO_X_Y(clz, u, u)
DEF_INT_PROTO_X_X_X_Y(clamp, u, u, u, u)
DEF_INT_PROTO_X_X_X_Y(clamp, i, i, i, i)
DEF_INT_PROTO_X_sX_sX_Y(clamp, u, u, u, u)
DEF_INT_PROTO_X_sX_sX_Y(clamp, i, i, i, i)
DEF_INT_PROTO_X_X_X_Y(mad_hi, u, u, u, u)
DEF_INT_PROTO_X_X_X_Y(mad_hi, i, i, i, i)
DEF_INT_PROTO_X_X_X_Y(mad_sat, u, u, u, u)
DEF_INT_PROTO_X_X_X_Y(mad_sat, i, i, i, i)
DEF_INT_PROTO_X_X_Y(max, i, i, i)
DEF_INT_PROTO_X_X_Y(max, u, u, u)
DEF_INT_PROTO_X_X_Y(min, i, i, i)
DEF_INT_PROTO_X_X_Y(min, u, u, u)

DEF_INT_PROTO_vX_sX_Y(max, i, i, i)
DEF_INT_PROTO_vX_sX_Y(max, u, u, u)
DEF_INT_PROTO_vX_sX_Y(min, i, i, i)
DEF_INT_PROTO_vX_sX_Y(min, u, u, u)

DEF_INT_PROTO_X_X_Y(mul_hi, i, i, i)
DEF_INT_PROTO_X_X_Y(mul_hi, u, u, u)
DEF_INT_PROTO_X_X_Y(rotate, i, i, i)
DEF_INT_PROTO_X_X_Y(rotate, u, u, u)
DEF_INT_PROTO_X_X_Y(sub_sat, i, i, i)
DEF_INT_PROTO_X_X_Y(sub_sat, u, u, u)
DEF_MUL24_PROTO_X_X_Y(mul24, i, i, i)
DEF_MUL24_PROTO_X_X_Y(mul24, u, u, u)
DEF_MAD24_PROTO_X_X_Y(mad24, i, i, i, i)
DEF_MAD24_PROTO_X_X_Y(mad24, u, u, u, u)
DEF_UPSAMPL_PROTO_X_X_Y(upsample, i)
DEF_UPSAMPL_PROTO_X_X_Y(upsample, u)

