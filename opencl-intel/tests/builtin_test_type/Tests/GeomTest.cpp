//#include "stdafx.h"
#include "GeomTest.h"
#include <ia32intrin.h>
#include <mathimf.h>
#include "cl_geom_declaration.h"



// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( Geom_Test );


void Geom_Test::setUp()
{
}


void Geom_Test::tearDown()
{
}


//Returns fmin(fmax(x, minval), maxval) .
//Results are undefined if minval > maxval.
void Geom_Test::__cross_test()
{
	float4 x = _mm_set1_ps(40);
	float4 y = _mm_set1_ps(40);
	float result[4];
	float4 res = __crossf4(x, y);
	memcpy(result, &res, sizeof(float4));

	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ERROR cross wrong behavour", 0,result[0] ,0);
}
void Geom_Test::__length_test()
{
	float4 x = _mm_set1_ps(3);
	float res = __lengthf4(x);
	float expected  = sqrtf(4*3*3);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ERROR length wrong behavour",expected ,res ,0);
}
void Geom_Test::__distance_test()
{
	float4 x = _mm_set1_ps(6);
	float4 y = _mm_set1_ps(3);
	float res = __distancef4(x, y);
	float expected  = sqrtf(4*3*3);
	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ERROR distance wrong behavour",expected ,res ,0);
}
