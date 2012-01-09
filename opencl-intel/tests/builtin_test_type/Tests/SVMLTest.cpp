
//#include "stdafx.h"
#include "SVMLTest.h"
#include <math.h>
#include <ia32intrin.h>


#include "cl_math_declaration.h"




#define BUFF_SIZE 100

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( SVML_Test );


void SVML_Test::setUp()
{
}


void SVML_Test::tearDown()
{
}



//
//
#if SVML_NEW_RELEASE
void SVML_Test::__atanpi_test()
{
	float16 xxxx, yyyy, zzzz;
	float result[16], result1[16];
	//f16 validation
	yyyy.a = _mm_set1_ps(0.1);
	yyyy.b = _mm_set1_ps(0.2);
	yyyy.c = _mm_set1_ps(0.3);
	yyyy.d = _mm_set1_ps(0.4);

	xxxx = __atanpif16(yyyy);
	zzzz = __atanf16(xxxx);

	memcpy(result, &zzzz, sizeof(float16));
	memcpy(result1, &xxxx, sizeof(float16));
	
	
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ERROR atanpif16 wrong behavour", result[0],result1[0]*INRNL_PI ,0.2);
  
	//f8 validation
	float8 xx, yy, zz;
	
	yy.a = _mm_set1_ps(0.1);
	yy.b = _mm_set1_ps(0.2);

	xx = __atanpif8(yy);
	zz = __atanf8(xx);

	memcpy(result, &zz, sizeof(float8));
	memcpy(result1, &xx, sizeof(float8));
	
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ERROR atanpif8 wrong behavour", result[0],result1[0]*INRNL_PI ,0.2);
	
	//f4 validation
	float4 x, y, z;
	
	y = _mm_set1_ps(0.1);
	
	x = __atanpif4(y);
	z = __atanf4(x);

	memcpy(result, &z, sizeof(float4));
	memcpy(result1, &x, sizeof(float4));
	
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ERROR atanpif4 wrong behavour", result[0],result1[0]*INRNL_PI ,0.2);        

	//f2 validation
	float2 x2, y2, z2;
	
	y2 = _mm_set1_pi32(0.1);
	
	x2 = __atanpif2(y2);
	z2 = __atanf2(x2);

	memcpy(result, &z2, sizeof(float2));
	memcpy(result1, &x2, sizeof(float2));
	
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ERROR atanpif2 wrong behavour", result[0],result1[0]*INRNL_PI ,0.2);
}
#endif
void SVML_Test::__exp10_test()
{
	float2 x2;
	float4 x;
	float8 xx;
	float16 xxxx;
	float result[16], result1[16];

	
	//f16 validation
	xxxx.a = _mm_set1_ps(3);
	xxxx.b = _mm_set1_ps(3);
	xxxx.c = _mm_set1_ps(3);
	xxxx.d = _mm_set1_ps(3);

	xxxx = __exp10f16(xxxx);
	
	xx.a = _mm_set1_ps(3);
	xx.b = _mm_set1_ps(3);

	xx = __exp10f8(xx);

	x = _mm_set1_ps(3);
	
	x = __exp10f4(x);

	x2 = _mm_set1_pi32(3);
	
	__declspec(align(16)) float temp[4];

	_mm_store_ps(temp, x);

	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ERROR __exp10f16 error", 1000 , temp[0] ,0.1);

}
#ifdef SVML_NEW_RELEASE
void SVML_Test::__acospi_test()
{
	float a[BUFF_SIZE] = {0.5, 1.2, 0.5, 1.2, 0.5, 1.2, 0.5, 1.2, 0.5, 1.2};
	float r[BUFF_SIZE];
	bool ret = true;

	int N = 10;

	#pragma ivdep
	for (int i =0; i<N; i++)
	{
		r[i] = cos(a[i]);
	}

	float16 xxxx, yyyy, zzzz;
	//__acoshf16 validation

	yyyy.a = _mm_set1_ps(0.1);
	yyyy.b = _mm_set1_ps(0.2);
	yyyy.c = _mm_set1_ps(0.3);
	yyyy.d = _mm_set1_ps(0.4);

	xxxx = __cosf16(yyyy);
	yyyy = __acosf16(xxxx);
		
	float result[16], result1[16];

	memcpy(result, &yyyy, sizeof(float16));

	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ERROR acos after cos didnt get same value error", 0.1 ,result[0] ,0.1);
	

//	__acospif16 validation
	yyyy.a = _mm_set1_ps(0.1);
	yyyy.b = _mm_set1_ps(0.2);
	yyyy.c = _mm_set1_ps(0.3);
	yyyy.d = _mm_set1_ps(0.4);

	xxxx = __acospif16(yyyy);
	zzzz = __acosf16(yyyy);

	memcpy(result, &zzzz, sizeof(float16));
	memcpy(result1, &xxxx, sizeof(float16));
	
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ERROR __acoshf16 wrong behavour", result1[0]*INRNL_PI ,result[0] ,0.1);
}
#endif
/*
void SVML_Test::__sincos_test()
{
	__declspec(align(16)) float a[] = {0.5, 1.2, 0.5, 1.2};
	__declspec(align(16)) float rSin[4];
	__declspec(align(16)) float rCos[4];

	float4 x;

	x = _mm_load_ps(a);
	x = __sincosf4(x, (float4*)rCos);

	_mm_store_ps(rSin, x);
	for(int i=0; i<4; ++i)
	{
		CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ERROR __sincosf4()-sin wrong behavour", sin(a[i]), rSin[i], 0.001);
		CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ERROR __sincosf4()-cos wrong behavour", cos(a[i]), rCos[i], 0.001);
	}
}
*/
