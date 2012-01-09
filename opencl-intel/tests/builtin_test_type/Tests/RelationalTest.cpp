#include "RelationalTest.h"
#include <mathimf.h>
#include <math.h>
#include <ia32intrin.h>
#include "cl_relational_declaration.h"


#define MAX( _a, _b )   ( (_a) > (_b) ? (_a) : (_b) )
#define MIN( _a, _b )   ( (_a) < (_b) ? (_a) : (_b) )


// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( Relational_Test );


void Relational_Test::setUp()
{
}


void Relational_Test::tearDown()
{
}


void Relational_Test::__isequal_test()
{
//extern "C" R_FUNC_DECL int __isequal (float x, float y)
	float x = 10;
	float y = 5;
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__isequal1",0 , __isequalf (x,y));
//extern "C" __m64 __isequalf2 (__m64 x, __m64 y)
	__m64 x64 = _mm_set_pi32(13, 15);
	__m64 y64 = _mm_set_pi32(13, 15);
	int result[2];

	x64 = __isequalf2(x64, y64);
	memcpy(result, &x64, sizeof(x64));
	
	for(int i = 0; i< 2; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__isequalf2", -1, result[i]);
	}
//extern "C" R_FUNC_DECL __m128i __isequalf4 (__m128 x, __m128 y)
	__m128 xf = _mm_set1_ps(12.3);
	__m128 yf = _mm_set1_ps(12.3);
	__m128i res;
	int result1[4];

	res = __isequalf4(xf, yf);
	memcpy(result1, &res, sizeof(res));
	
	for(int i = 0; i< 4; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__isequalf4", -1, result1[i]);
	}

}

void Relational_Test::__isnotequal_test()
{
//extern "C" R_FUNC_DECL int __isnotequal (float x, float y)
	float x = 10;
	float y = 5;
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__isnotequalf1",1 , __isnotequalf (x,y));
//extern "C" __m64 __isnotequalf2 (__m64 x, __m64 y)
	__m64 x64 = _mm_set_pi32(13, 15);
	__m64 y64 = _mm_set_pi32(13, 15);
	int result[2];

	x64 = __isnotequalf2(x64, y64);
	memcpy(result, &x64, sizeof(x64));
	
	for(int i = 0; i< 2; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__isnotequalf2", 0, result[i]);
	}
//extern "C" R_FUNC_DECL __m128i __isequalf4 (__m128 x, __m128 y)
	__m128 xf = _mm_set1_ps(12.3);
	__m128 yf = _mm_set1_ps(12.7);
	__m128i res;
	int result1[4];

	res = __isnotequalf4(xf, yf);
	memcpy(result1, &res, sizeof(res));
	
	for(int i = 0; i< 4; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__isnotequalf4", -1, result1[i]);
	}

}

void Relational_Test::__isgreater_test()
{
//extern "C" R_FUNC_DECL int __isgreaterf1 (float x, float y)
	float x = 10.0;
	float y = 5.0;
	int tmp = __isgreaterf1(x,y);

	CPPUNIT_ASSERT_EQUAL_MESSAGE("__isgreaterf1",1 , tmp);
//extern "C" __m64 __isnotequalf2 (__m64 x, __m64 y)
	__m64 x64;
	__m64 y64;
	int result[2];
	float x1[2] = {20.0, 40.0};
	float y1[2] = {10.0, 15.0};

	memcpy((void*)&x64, &x1, sizeof(x64));
	memcpy((void*)&y64, &y1, sizeof(x64));

	x64 = __isgreaterf2(x64, y64);
	memcpy(result, &x64, sizeof(x64));
	
	for(int i = 0; i< 2; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__isgreaterf2", -1, result[i]);
	}
//extern "C" R_FUNC_DECL __m128i __isgreaterf4 (__m128 x, __m128 y)
	__m128 xf = _mm_set1_ps(10.1);
	__m128 yf = _mm_set1_ps(10.7);
	__m128i res;
	int result1[4];

	res = __isgreaterf4(xf, yf);
	memcpy(result1, &res, sizeof(res));
	
	for(int i = 0; i< 4; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__isgreaterf4", 0, result1[i]);
	}

}
void Relational_Test::__isgreaterequal_test()
{
//extern "C" R_FUNC_DECL int __isgreaterequalf1 (float x, float y)
	float x = 10;
	float y = 10;
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__isgreaterf1",1 , __isgreaterequalf1(x,y));
//extern "C" __m64 __isgreaterequalf2 (__m64 x, __m64 y)
	__m64 x64 = _mm_set_pi32(20, 40);
	__m64 y64 = _mm_set_pi32(10, 15);
	int result[2];

	x64 = __isgreaterequalf2(x64, y64);
	memcpy(result, &x64, sizeof(x64));
	
	for(int i = 0; i< 2; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__isgreaterf2", -1, result[i]);
	}
//extern "C" R_FUNC_DECL __m128i __isgreaterequalf4 (__m128 x, __m128 y)
	__m128 xf = _mm_set1_ps(10.1);
	__m128 yf = _mm_set1_ps(10.1);
	__m128i res;
	int result1[4];

	res = __isgreaterequalf4(xf, yf);
	memcpy(result1, &res, sizeof(res));
	
	for(int i = 0; i< 4; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__isgreaterequalf4", -1, result1[i]);
	}

}
void Relational_Test::__isless_test()
{
//extern "C" R_FUNC_DECL int __islessf1 (float x, float y)
	float x = 5;
	float y = 10;
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__isgreaterf1",1 , __islessf1(x,y));
//extern "C" __m64 __islessf2 (__m64 x, __m64 y)
	__m64 x64 = _mm_set_pi32(20, 40);
	__m64 y64 = _mm_set_pi32(10, 15);
	int result[2];

	x64 = __islessf2(x64, y64);
	memcpy(result, &x64, sizeof(x64));
	
	for(int i = 0; i< 2; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__islessf2", 0, result[i]);
	}
//extern "C" R_FUNC_DECL __m128i __islessf4 (__m128 x, __m128 y)
	__m128 xf = _mm_set1_ps(10.1);
	__m128 yf = _mm_set1_ps(10.7);
	__m128i res;
	int result1[4];

	res = __islessf4(xf, yf);
	memcpy(result1, &res, sizeof(res));
	
	for(int i = 0; i< 4; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__islessf4", -1, result1[i]);
	}
}
void Relational_Test::__islessequal_test()
{
//extern "C" R_FUNC_DECL int __islessequalf1 (float x, float y)
	float x = 5;
	float y = 5;
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__isgreaterf1",1 , __islessequalf1(x,y));
//extern "C" __m64 __islessequalf2 (__m64 x, __m64 y)
	__m64 x64 = _mm_set_pi32(10, 15);
	__m64 y64 = _mm_set_pi32(20, 15);
	int result[2];

	x64 = __islessequalf2(x64, y64);
	memcpy(result, &x64, sizeof(x64));
	
	for(int i = 0; i< 2; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__islessequalf2", -1, result[i]);
	}
//extern "C" R_FUNC_DECL __m128i __islessequalf4 (__m128 x, __m128 y)
	__m128 xf = _mm_set1_ps(10.7);
	__m128 yf = _mm_set1_ps(10.7);
	__m128i res;
	int result1[4];

	res = __islessequalf4(xf, yf);
	memcpy(result1, &res, sizeof(res));
	
	for(int i = 0; i< 4; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__islessequalf4", -1, result1[i]);
	}


}
void Relational_Test::__isfinite_test()
{
//extern "C" R_FUNC_DECL int __isfinitef1 (float x)
	float x = 5.0;
	int tmp = __isfinitef1 (x);

	CPPUNIT_ASSERT_EQUAL_MESSAGE("__isfinitef1",1 , tmp);
//extern "C" __m64 __isfinitef2 (__m64 x)
	
	__m64 x64;
	float xf[2] = {INFINITY, INFINITY};
	
	memcpy((void*)&x64, &xf, sizeof(x64));
	

	int result[2];

	x64 = __isfinitef2(x64);
	memcpy(result, &x64, sizeof(x64));
	
	for(int i = 0; i< 2; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__isfinitef2", 0, result[i]);
	}
//extern "C" R_FUNC_DECL __m128i __isfinitef4 (__m128 x, __m128 y)
		
	__m128i res;
	int result1[4];
	__m128 xf128 = _mm_set1_ps(INFINITY);

	res = __isfinitef4(xf128);
	memcpy(result1, &res, sizeof(res));
	
	for(int i = 0; i< 4; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__isfinitef4", 0, result1[i]);
	}


}
void Relational_Test::__isinf_test()
{
//extern "C" R_FUNC_DECL int isinff1 (float x)
	float x = 5;
	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("isfinitef1",0 , __isinff1 (x));
//extern "C" __m64 __isinff2 (__m64 x)
	__m128 xf = _mm_set1_ps(INFINITY);
	__m64 x64 = _mm_movepi64_pi64(_mm_castps_si128(xf));
	
	int result[2];

	x64 = __isinff2(x64);
	memcpy(result, &x64, sizeof(x64));
	
	for(int i = 0; i< 2; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__isinff2", -1, result[i]);
	}
//extern "C" R_FUNC_DECL __m128i __isfinitef4 (__m128 x, __m128 y)
	
	
	__m128i res;
	int result1[4];

	res = __isinff4(xf);
	memcpy(result1, &res, sizeof(res));
	
	for(int i = 0; i< 4; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__isinff4", -1, result1[i]);
	}
}
void Relational_Test::__isnormal_test()
{
//extern "C" R_FUNC_DECL int __isnormalf1 (float x)
	float x = 5;
	int resi = __isnormalf1 (x);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__isnormalf1",1 , resi);
//extern "C" __m64 __isnormalf2 (__m64 x)
	__m64 x64 = _mm_set_pi32(INFINITY, INFINITY);
	
	int result[2];

	x64 = __isnormalf2(x64);
	memcpy(result, &x64, sizeof(x64));
	
	for(int i = 0; i< 2; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__isnormalf2", 0, result[i]);
	}
//extern "C" R_FUNC_DECL __m128i __isnormalf4 (__m128 x, __m128 y)
	__m128 xf = _mm_set1_ps(INFINITY);
	
	__m128i res;
	int result1[4];

	res = __isnormalf4(xf);
	memcpy(result1, &res, sizeof(res));
	
	for(int i = 0; i< 4; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__isnormalf4", 0, result1[i]);
	}
}
void Relational_Test::__signbit_test()
{
//extern "C" R_FUNC_DECL int signbitf1 (float x)
	float x = -5;
	int resi = __signbitf1 (x);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__signbitf1",1 , resi);
//extern "C" __m64 __signbitf2 (__m64 x)
	__m128 xft = _mm_set1_ps(-5);
	__m64 x64 = _mm_movepi64_pi64(_mm_castps_si128(xft));

	
	int result[2];

	x64 = __signbitf2(x64);
	memcpy(result, &x64, sizeof(x64));
	
	for(int i = 0; i< 2; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__signbitf2", -1, result[i]);
	}
//extern "C" R_FUNC_DECL __m128i __signbitf4 (__m128 x)
	int in[4] = {-4, 3, -2, 1};
	__m128 xf = _mm_set_ps(in[3], in[2], in[1], in[0]);
	
	__m128i res;
	int result1[4];

	res = __signbitf4(xf);
	memcpy(result1, &res, sizeof(res));
	
	for(int i = 0; i< 4; i++)
	{
		int expected = 0;
		if(in[i] < 0)
		{
			expected = -1;
		}
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__signbitf2", expected, result1[i]);
	}
}

void Relational_Test::__any_test()
{
//TEST: extern "C" __declspec(dllexport) __m128i __any_char8(__m128i x);
	__m128i x = _mm_set_epi8(0x80, 0x80, 0, 0, 0 , 0, 0, 0, 0, 6, 5, 4, 3, 2, 1, 0);
	int res;	
	res = __any_16i8(x);
	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__any_16i8", 1, res);

//TEST: extern "C" __declspec(dllexport) __m128i ___anyf128f16(__m128i x);
	x = _mm_set_epi16(0x8000, 0, 0, 0, 0 , 0, 0, 0);
	
	res = __any_8i16(x);
	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__any_8i16", 1, res);

//TEST: extern "C" __declspec(dllexport) __m128i ___anyf128_32(__m128i x);
	x = _mm_set_epi32(0x8000, 0, 0, 0);
	
	res = __any_4i32(x);
	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__any_4i32", 0, res);
//TEST: extern "C" __declspec(dllexport) __m128i ___anyf128_64(__m128i x);
	__m64 zero = _mm_setr_pi32(0, 0);
	__m64 one = _mm_setr_pi32(-1, -1);
	x = _mm_set_epi64(one, zero);
	
	res = __any_2i64(x);
	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__any_2i64", 1, res);

}
void Relational_Test::__all_test()
{
	__m128i x = _mm_set_epi8(0x80, 0x80, 0, 0, 0 , 0, 0, 0, 0, 6, 5, 4, 3, 2, 1, 0);
	int res;	
	res = __all_16i8(x);
	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__all_16i8", 0, res);

//TEST: extern "C" __declspec(dllexport) __m128i _allf128f16(__m128i x);
	x = _mm_set_epi16(0x8000, 0x8000,0x8000, 0x8000,0x8000 , 0x8000, 0x8000, 0x8000);
	
	res = __all_8i16(x);
	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__all_8i16", 1, res);


}
//Each bit of the result is the corresponding bit of a if the
//corresponding bit of c is 0. Otherwise it is the
//corresponding bit of b.
void Relational_Test::__bitselect_test()
{
//extern "C" R_FUNC_DECL float4 bitselect_f4 (float4 a, float4 b, float4 c);
	__m128 a = _mm_set1_ps(21845.0);
	__m128 b = _mm_set1_ps(100.0);
	__m128 c = _mm_set1_ps(0);
	__m128 res;
	float result[4];

	res = __bitselect_f4(a, b, c);
	memcpy(result, &res, sizeof(float4));
	
	for(int i = 0; i< 4; i++)
	{
			int expected = __bitselect_f(0x5555, 0x4444, 0);
			CPPUNIT_ASSERT_EQUAL_MESSAGE("bitselect_f4", expected, (int)result[i]);
	}

}
//For each component of a vector type,
//result[i] = if MSB of c[i] is set ? b[i] : a[i].
//For a scalar type, result = c ? b : a.
void Relational_Test::__select_test()
{
//extern "C" R_FUNC_DECL float4 select_f4 (float4 a, float4 b, float4 c);
	__m128 a = _mm_set1_ps(119.0);
	__m128 b = _mm_set1_ps(267.0);
	__m128i c = _mm_set1_epi32(0);
	__m128 res;
	float result[4];
	
	res = __select_ffi4(a, b, c);
	memcpy(result, &res, sizeof(float4));
	

	for(int i = 0; i< 4; i++)
	{
		CPPUNIT_ASSERT_EQUAL_MESSAGE("select_f4", (float)119.0, result[i]);
	}
//extern "C"  R_FUNC_DECL __m128i __select_char16 (__m128i a, __m128i b, __m128i c);
	__m128i a1 = _mm_set1_epi8(0x12);
	__m128i b1 = _mm_set1_epi8(0x13);
	__m128i c1 = _mm_set1_epi8(0);
	__m128i res1;
	char result1[16];
	
	res1 = __select_16i8(a1, b1, c1);
	memcpy(result1, &res1, sizeof(res1));
	
	for(int i = 0; i< 16; i++)
	{
		CPPUNIT_ASSERT_EQUAL_MESSAGE("__select_16i8", (int)0x12, (int)result1[i]);
	}

//extern "C"  R_FUNC_DECL __m128i __select_short8 (__m128i a, __m128i b, __m128i c);
	 a1 = _mm_set1_epi16(0x12);
	 b1 = _mm_set1_epi16(0x13);
	 c1 = _mm_set1_epi16(0);
	
	short result2[8];
	
	res1 = __select_8i16(a1, b1, c1);
	memcpy(result2, &res1, sizeof(res1));
	
	for(int i = 0; i< 8; i++)
	{
		CPPUNIT_ASSERT_EQUAL_MESSAGE("__select_8i16", (short)0x12, (short)result2[i]);
	}

	float a2 = 20.0;
	float b2 = 30.0;
	int  c2 = 0;
	
	float res2 = 0;
	res2 = __select_ffi(a2, b2, c2);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("select_f", (float)20.0, res2);
}
