//#include "stdafx.h"
#include "CommonTest.h"
#include <math.h>
#include <ia32intrin.h>
#include "cl_common_declaration.h"



// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( Common_Test );


void Common_Test::setUp()
{
}


void Common_Test::tearDown()
{
}


//Returns fmin(fmax(x, minval), maxval) .
//Results are undefined if minval > maxval.
void Common_Test::__clamp_test()
{
	float4 x = _mm_set1_ps(40);
	float4 minval = _mm_set1_ps(80);
	float4 maxval = _mm_set1_ps(1000);
	float result[4];
	float4 res = __clampf4(x, minval, maxval);
	memcpy(result, &res, sizeof(float4));

	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ERROR clampf4 wrong behavour", result[0],80 ,0);
}
//Converts radians to degrees, i.e. (180 / pi) *
//radians
void Common_Test::__degrees_test()
{
	float fx = 40;
	float4 x = _mm_set1_ps(fx);
	
	float result[4];
	float4 res = __degreesf4(x);
	float fres = __degreesf(fx);
	memcpy(result, &res, sizeof(float4));

	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ERROR degreesf4 wrong behavour", result[0],fres ,0);
}
//Returns y if x < y, otherwise it returns x. If x and y
//are infinite or NaN, the return values are undefined
void Common_Test::__max_test()
{
	float fx = 40;
	float4 x = _mm_set1_ps(fx);
	float fy = 50;
	float4 y = _mm_set1_ps(fy);
	float result[4];

	float4 res = __maxf4(x, y);
	float fres = __maxf(fx, fy);
	memcpy(result, &res, sizeof(float4));

	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ERROR maxf4 wrong behavour", result[0],fres ,0);
}
//Returns x if x < y, otherwise it returns y. If x and y
//are infinite or NaN, the return values are undefined
void Common_Test::__min_test()
{
	float fx = 40;
	float4 x = _mm_set1_ps(fx);
	float fy = 50;
	float4 y = _mm_set1_ps(fy);
	float result[4];

	float4 res = __minf4(x, y);
	float fres = __minf(fx, fy);
	memcpy(result, &res, sizeof(float4));

	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ERROR minf4 wrong behavour", result[0],fres ,0);
}

//step - Returns 0.0 if x < edge, otherwise it returns 1.0
void Common_Test::__step_test()
{
	float fx = 5.0;
	float4 x = _mm_set1_ps(fx);
	float fedge = 4.0;
	float4 edge = _mm_set1_ps(fedge);
	float result[8];

	float4 res = __stepf4(edge, x);
	float fres = __stepf(fedge, fx);
	memcpy(result, &res, sizeof(float4));

	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ERROR stepf4 wrong behavour", result[0],fres ,0);
}
void Common_Test::__smoothstep_test()
{
	float fx = 5.0;
	float4 x = _mm_set1_ps(fx);
	float fedge0 = 4.0;
	float4 edge0 = _mm_set1_ps(fedge0);
	float result[8];
	float fedge1 = 6.0;
	float4 edge1 = _mm_set1_ps(fedge1);

	float4 res = __smoothstepf4(edge0, edge1, x);
	float fres = __smoothstepf(fedge0, fedge1, fx);
	memcpy(result, &res, sizeof(float4));

	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ERROR smoothstepf4 in limit wrong behavour", result[0],fres ,0);


	fx = 3.0;
	x = _mm_set1_ps(fx);
	
	res = __smoothstepf4(edge0, edge1, x);
	fres = __smoothstepf(fedge0, fedge1, fx);
	memcpy(result, &res, sizeof(float4));

	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ERROR smoothstepf4 wrong behavour x<edg0", result[0],fres ,0);

	fx = 7.0;
	x = _mm_set1_ps(fx);
	
	res = __smoothstepf4(edge0, edge1, x);
	fres = __smoothstepf(fedge0, fedge1, fx);
	memcpy(result, &res, sizeof(float4));

	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ERROR smoothstepf4 wrong behavour x>edg1", result[0],fres ,0);


}
void Common_Test::__sign_test()
{
	float fx = 5.0;
	float4 x = _mm_set1_ps(fx);
	float result[8];

	float4 res = __signf4(x);
	float fres = __signf(fx);
	memcpy(result, &res, sizeof(float4));

	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ERROR signf4 positive x", result[0],fres ,0);

	fx = -5.0;
	x = _mm_set1_ps(fx);
	res = __signf4(x);
	fres = __signf(fx);
	memcpy(result, &res, sizeof(float4));

	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ERROR signf4 negative x", result[0],fres ,0);

	fx = -0.0;
	x = _mm_set1_ps(fx);
	res = __signf4(x);
	fres = __signf(fx);
	memcpy(result, &res, sizeof(float4));

	CPPUNIT_ASSERT_DOUBLES_EQUAL_MESSAGE("ERROR signf4 -0.0", result[0],fres ,0);

}