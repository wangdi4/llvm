/////////////////////////////////////////////////////////////////////////
// llvm_program.cpp:
/////////////////////////////////////////////////////////////////////////
// INTEL CONFIDENTIAL
// Copyright 2007-2008 Intel Corporation All Rights Reserved.
//
// The source code contained or described herein and all documents related 
// to the source code ("Material") are owned by Intel Corporation or its 
// suppliers or licensors. Title to the Material remains with Intel Corporation
// or its suppliers and licensors. The Material may contain trade secrets and 
// proprietary and confidential information of Intel Corporation and its 
// suppliers and licensors, and is protected by worldwide copyright and trade 
// secret laws and treaty provisions. No part of the Material may be used, copied, 
// reproduced, modified, published, uploaded, posted, transmitted, distributed, 
// or disclosed in any way without Intel’s prior express written permission. 
//
// No license under any patent, copyright, trade secret or other intellectual
// property right is granted to or conferred upon you by disclosure or delivery 
// of the Materials, either expressly, by implication, inducement, estoppel or 
// otherwise. Any license under such intellectual property rights must be express
// and approved by Intel in writing.
//
// Unless otherwise agreed by Intel in writing, you may not remove or alter this notice 
// or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors 
// in any way.
/////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "llvm_program.h"
#include "ocl_rt.h"

#include <math.h>
#include <emmintrin.h>

#pragma comment(lib, "svml_dispmt.lib")

#ifdef __USE_INTRIN_FASTCALL__
#define OCL_INTRIN_CALL					__fastcall
#else
#define OCL_INTRIN_CALL
#endif

// List of the intrinsics to be substitued by internal function
char*	szWIIntrinNames[] =
{
#ifdef __USE_INTRIN_FASTCALL
	"gid",		"@gid@4",
	"ndims",	"@ndims@0",
	"gdim",		"@gdim@4",
	"ldim",		"@ldim@4",
	"lid",		"@lid@4",
	"tgdim",	"@tgdim@4",
	"tgid",		"@tgid@4",
#else
	"gid",		"gid",
	"ndims",	"ndims",
	"gdim",		"gdim",
	"ldim",		"ldim",
	"lid",		"lid",
	"tgdim",	"tgdim",
	"tgid",		"tgid",
#endif
	"__cosf4",	"__svml_cosf4",
	"__acosf4",	"__svml_acosf4",
};

unsigned int	uiWIIntrinCount = sizeof(szWIIntrinNames)/sizeof(char*);

////////////////////////////////////////////////////////////////////////////////////////////
// OCL LLVM inline functions
extern "C" __declspec(dllexport) int OCL_INTRIN_CALL gid(int i)
{
	return (int)get_global_id(i);
}

extern "C" __declspec(dllexport) int OCL_INTRIN_CALL lid(int i)
{
	return (int)get_local_id(i);
}

extern "C" __declspec(dllexport) int OCL_INTRIN_CALL ndims()
{
	return (int)get_work_dim();
}

extern "C" __declspec(dllexport) int OCL_INTRIN_CALL gdim(int i)
{
	return (int)get_global_size(i);
}

extern "C" __declspec(dllexport) int OCL_INTRIN_CALL ldim(int i)
{
	return (int)get_local_size(i);
}

extern "C" __declspec(dllexport) int OCL_INTRIN_CALL tgdim(int i)
{
	return (int)get_num_groups(i);
}

extern "C" __declspec(dllexport) int OCL_INTRIN_CALL tgid(int i)
{
	return (int)get_group_id(i);
}


extern "C" __declspec(dllexport) float __dotf4(__m128 a, __m128 b)
{
	a = _mm_mul_ps(a, b);
	a = _mm_hadd_ps(a, a);
	a = _mm_hadd_ps(a, a);
	return  _mm_cvtss_f32(a);
}

extern "C" __m128 __svml_cosf4(__m128 a);
#pragma comment(linker, "/INCLUDE:___svml_cosf4")
#pragma comment(linker, "/EXPORT:___svml_cosf4")

extern "C" __declspec(dllexport) float __cosf(float f)
{
	return (float)cos((double)f);
}

extern "C" __declspec(dllexport) __m64 __cosf2(__m64 a)
{
	__m128i	temp = _mm_movpi64_epi64(a);
	temp = _mm_castps_si128(__svml_cosf4(_mm_castsi128_ps(temp)));
	return _mm_movepi64_pi64(temp);
}

struct __m256
{
	__m128	a, b;
};

extern "C" __declspec(dllexport) __m256 __cosf8(__m128 a, __m128 b)
{
	__m256 res;

	res.a = __svml_cosf4(a);
	res.b = __svml_cosf4(b);

	return res;
}

extern "C" __declspec(dllexport) __m128 __cosf16(__m128 a, __m128 b)
{
	return a;
}

extern "C" __m128 __svml_acosf4(__m128 a);
#pragma comment(linker, "/INCLUDE:___svml_acosf4")
#pragma comment(linker, "/EXPORT:___svml_acosf4")

extern "C" __declspec(dllexport) float __acosf(float f)
{
	return (float)acos((double)f);
}

extern "C" __declspec(dllexport) __m64 __acosf2(__m64 a)
{
	__m128i	temp = _mm_movpi64_epi64(a);
	temp = _mm_castps_si128(__svml_acosf4(_mm_castsi128_ps(temp)));
	return _mm_movepi64_pi64(temp);
}


extern "C" __declspec(dllexport) __m256 __acosf8(__m128 a, __m128 b)
{
	__m256 res;

	res.a = __svml_cosf4(a);
	res.b = __svml_cosf4(b);

	return res;
}

extern "C" __declspec(dllexport) __m128 __acosf16(__m128 a, __m128 b)
{
	return a;
}
