#include "IntegerTest.h"
#include <math.h>
#include <ia32intrin.h>
#include "cl_integer_declaration.h"

#define UCHAR_MIN   0
#define USHRT_MIN   0
#define UINT_MIN    0

#define MAX( _a, _b )   ( (_a) > (_b) ? (_a) : (_b) )
#define MIN( _a, _b )   ( (_a) < (_b) ? (_a) : (_b) )

#define BUFF_SIZE 100

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( Integer_Test );


void Integer_Test::setUp()
{
}


void Integer_Test::tearDown()
{
}


void Integer_Test::__abs_test()
{
//TEST: extern "C" __declspec(dllexport) __m128i __abs_16i8(__m128i x);
	__m128i x = _mm_set_epi8(-15, -14, -13, -12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0);
		
	char result[16];
	x = __abs_16i8(x);
	memcpy(result, &x, sizeof(result));
	
	for(char i = 0; i< 16; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__abs_16i8", i, result[i]);
	}
//TEST: extern "C" __declspec(dllexport) __m128i __abs_8i16(__m128i x);
		
	__m128i x__8i16 = _mm_set_epi16( -7, -6, -5, -4, -3, -2, -1, 0);
	short result1[8];
	x__8i16 = __abs_8i16(x__8i16);
	memcpy(result1, &x__8i16, sizeof(_8i16));
	
	for(short i = 0; i< 8; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_abs_8i16", i, result1[i]);
	}
//TEST: extern "C" __declspec(dllexport) __m128i __abs_4i32(__m128i x);
	
	__m128i x_int4 = _mm_set_epi32( -3, -2, -1, 0);
	int result2[4];
	x_int4 = __abs_4i32(x_int4);
	memcpy(result2, &x_int4, sizeof(result2));
	
	for(int i = 0; i< 4; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_abs_4i32", i, result2[i]);
	}

//TEST: extern "C" __declspec(dllexport) __m128i __abs_2i64(__m128i x);\

	__m64 zero = _mm_setr_pi32(0, 0);
	__m64 one = _mm_setr_pi32(-1, -1);
	__m128i x_long2 = _mm_set_epi64(one, zero);
	__int64 result3[2];
	
	x_long2 = __abs_2i64(x_long2);
	memcpy(result3, &x_long2, sizeof(result3));
	
	for(__int64 i = 0; i< 2; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_abs_2i64", i, result3[i]);
	}

//TEST: extern "C" __declspec(dllexport) __m64 __abs_8i8(__m64 x);
	__m64 x_64 = _mm_set_pi8(-7, -6, -5, -4, -3, -2, -1, 0);
		
	char result4[8];
	__m128i res_8i8 = __abs_8i8(x_64);
	memcpy(result4, &res_8i8, sizeof(result4));
	
	for(char i = 0; i< 8; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_abs_8i8", i, result4[i]);
	}
//TEST: extern "C" __declspec(dllexport) __m64 __abs_4i16(__m64 x);
	__m64 x_4i16 = _mm_set_pi16(-3, -2, -1, 0);
		
	short result5[4];
	res_8i8 = __abs_4i16(x_4i16);
	memcpy(result5, &res_8i8, sizeof(result5));
	
	for(short i = 0; i< 4; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_abs_4i16", i, result5[i]);
	}

//TEST: extern "C" __declspec(dllexport) __m64 __abs_2i32(__m64 x);
	__m64 x_2i32 = _mm_set_pi32(-1, 0);
		
	int result6[2];
	x_2i32 = __abs_2i32(x_2i32);
	memcpy(result6, &x_2i32, sizeof(x_2i32));
	
	for(int i = 0; i< 2; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_abs_2i32", i, result6[i]);
	}
//TEST: extern "C" __declspec(dllexport) __m64 __abs_1i64(__m64 x);
	__int64 x_1i64 = (__int64)-1;
		
	__int64 result7;
	result7 = __abs_1i64(x_1i64);
	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("_abs_1i64", (__int64)1, result7);
}

void Integer_Test::__abs_diff_test()
{

//Test 8i8
	char inA[8] = {0x0, 0x0, 0x0, 0x0, 0xff, 0xff, 0xff, 0x8c};
	char inB[8] = {0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x19};
	__m64 x64 = _mm_set_pi8(inA[0], inA[1], inA[2], inA[3],inA[4], inA[5], inA[6], inA[7]);
	__m64 y64 = _mm_set_pi8(inB[0], inB[1], inB[2], inB[3],inB[4], inB[5], inB[6], inB[7]);
	unsigned char result64[8];
	__m128i res_8i8 = __abs_diff_8i8(x64, y64);
	memcpy(result64, &res_8i8, sizeof(result64));
	
	for(char i = 0; i< 8; i++)
	{
		unsigned char expected = inA[7-i]- inB[7-i];		 
        if( inB[7-i] > inA[7-i] )
		{
            expected = inB[7-i] - inA[7-i];	
		}
			//expected = (char)abs(expected);
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__abs_diff_8i8", expected, result64[i]);
	}

//TEST: extern "C" __declspec(dllexport) __m128i __abs_diff_16i8(__m128i x, __m128i y);
	__m128i x = _mm_set_epi8(15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0);
	__m128i y = _mm_set1_epi8(0);
	char result[16];
	x = __abs_diff_16i8(y, x);
	memcpy(result, &x, sizeof(result));
	
	for(char i = 0; i< 16; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_abs_diff_16i8", i, result[i]);
	}

//TEST: extern "C" __declspec(dllexport) __m128i __abs_diff_8i16(__m128i x, __m128i y);
   __m128i x__8i16 = _mm_set_epi16( 7, 6, 5, 4, 3, 2, 1, 0);
	__m128i y__8i16 = _mm_set1_epi16(0);
	
	short result1[8];
	x__8i16 = __abs_diff_8i16(y__8i16, x__8i16);
	memcpy(result1, &x__8i16, sizeof(x__8i16));
	
	for(short i = 0; i< 8; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_abs_diff_8i16", i, result1[i]);
	}
//TEST: extern "C" __declspec(dllexport)  __m128i __abs_diff_4i32(__m128i x, __m128i y);
	__m128i x_int4 = _mm_set_epi32( 3, 2, 1, 0);
	__m128i y_int4 = _mm_set1_epi32( 0);
	int result2[4];
	x_int4 = __abs_diff_4i32(y_int4, x_int4);
	memcpy(result2, &x_int4, sizeof(result2));
	
	for(int i = 0; i< 4; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_abs_diff_4i32", i, result2[i]);
	}

//TEST: extern "C" __declspec(dllexport)  __m128i __abs_diff_2i64(__m128i x, __m128i y);

	__m64 zero = _mm_setzero_si64();
	__m64 one = _mm_setr_pi32(1,0);
	__m128i x_long2 = _mm_set_epi64(one, zero);
	__m128i y_long2 = _mm_setzero_si128();
	__int64 result3[2];
	
	x_long2 = __abs_diff_2i64(y_long2, x_long2);
	memcpy(result3, &x_long2, sizeof(result3));
	
	for(__int64 i = 0; i< 2; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_abs_diff_2i64", i, result3[i]);
	}
	

//TEST: extern "C" __declspec(dllexport)  __m64 __abs_diff_8i8(__m64 x, __m64 y);
	__m64 x_64 = _mm_set_pi8(7, 6, 5, 4, 3, 2, 1, 0);
		
	char result4[8];
	res_8i8 = __abs_diff_8i8(zero, x_64);
	memcpy(result4, &res_8i8, sizeof(result4));
	
	for(char i = 0; i< 8; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_abs_diff_8i8", i, result4[i]);
	}

//TEST: extern "C" __declspec(dllexport)  __m64 __abs_diff_4i16(__m64 x, __m64 y);
	__m64 x_4i16 = _mm_set_pi16(3, 2, 1, 0);
		
	short result5[4];
	res_8i8 = __abs_diff_4i16(zero, x_4i16);
	memcpy(result5, &res_8i8, sizeof(result5));
	
	for(short i = 0; i< 4; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_abs_diff_4i16", i, result5[i]);
	}

//TEST: extern "C" __declspec(dllexport)  __m64 __abs_diff_2i32(__m64 x, __m64 y);
	__m64 x_2i32 = _mm_set_pi32(1, 0);
		
	int result6[2];
	x_2i32 = __abs_diff_2i32(zero, x_2i32);
	memcpy(result6, &x_2i32, sizeof(x_2i32));
	
	for(int i = 0; i< 2; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_abs_diff_2i32", i, result6[i]);
	}
//TEST: extern "C" __declspec(dllexport)  __m64 __abs_diff_1i64(__m64 x, __m64 y);}
	__int64 x_1i64 = (__int64)-1;
		
	__int64 result7;
	result7 = __abs_diff_1i64(x_1i64, 0);
	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("_abs_diff_1i64", (__int64)1, result7);

//TEST: extern "C" __declspec(dllexport)  _16u16 __abs_diff_16i16(_16i16 x, _16i16 y);}
	_16i16 x_16i16, y_16i16;

	x_16i16.a = _mm_set1_epi16(10);
	x_16i16.b = _mm_set1_epi16(12);
	y_16i16.a = _mm_set1_epi16(-10);
	y_16i16.b = _mm_set1_epi16(-12);

	_16u16 result8;
	result8 = __abs_diff_16i16(x_16i16, y_16i16);
	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__abs_diff_16i16", 20, _mm_extract_epi16(result8.a, 0));
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__abs_diff_16i16", 24, _mm_extract_epi16(result8.b, 0));

}
void Integer_Test::__add_sat_test()
{
//TEST: extern "C" INT_FUNC_DECL  __m128i __add_sat_16i8(__m128i x, __m128i y);
	int inA[16] = {-15, 0, -13, 12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0};
	int inB[16] = {CHAR_MIN, CHAR_MAX, -13, 12, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	__m128i x = _mm_setr_epi8(inA[0], inA[1], inA[2], inA[3], inA[4], inA[5], inA[6], inA[7], inA[8], inA[9], inA[10], inA[11], inA[12], inA[13], inA[14], inA[15]);
	__m128i y = _mm_setr_epi8(inB[0], inB[1], inB[2], inB[3], inB[4], inB[5], inB[6], inB[7], inB[8], inB[9], inB[10], inB[11], inB[12], inB[13], inB[14], inB[15]);
	char result[16];
	x = __add_sat_16i8(x, y);
	memcpy(result, &x, sizeof(result));
	
	for(char i = 0; i< 16; i++)
	{
		 int expected = (int) inA[i] + (int) inB[i];
	     expected = MAX( expected, CHAR_MIN );
         expected = MIN( expected, CHAR_MAX );
	
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_add_sat_16i8", (char)expected, result[i]);
	}
//TEST: extern "C" INT_FUNC_DECL  __m128i __add_sat_16u8(__m128i x, __m128i y);
	int inC[16] = {UCHAR_MIN, UCHAR_MAX, 13, 12, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int inD[16] = {UCHAR_MIN, UCHAR_MAX, UCHAR_MAX, UCHAR_MIN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	x = _mm_setr_epi8(inD[0], inD[1], inD[2], inD[3], inD[4], inD[5], inD[6], inD[7], inD[8], inD[9], inD[10], inD[11], inD[12], inD[13], inD[14], inD[15]);
	y = _mm_setr_epi8(inC[0], inC[1], inC[2], inC[3], inC[4], inC[5], inC[6], inC[7], inC[8], inC[9], inC[10], inC[11], inC[12], inC[13], inC[14], inC[15]);
	memcpy(result, &x, sizeof(result));
	memcpy(result, &y, sizeof(result));
	x = __add_sat_16u8(x, y);
	memcpy(result, &x, sizeof(result));
	
	for(char i = 0; i< 16; i++)
	{
		 int expected = (int) inD[i] + (int) inC[i];
	     expected = MAX( expected, UCHAR_MIN );
         expected = MIN( expected, UCHAR_MAX );
	
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_add_sat_16u8", (unsigned char)expected, (unsigned char)result[i]);
	}
//TEST: extern "C" INT_FUNC_DECL  __m128i __add_sat_8i16(__m128i x, __m128i y);
//TEST: extern "C" INT_FUNC_DECL  __m128i __add_sat_8u16(__m128i x, __m128i y);
//TEST: extern "C" INT_FUNC_DECL  __m128i __add_sat_4i32(__m128i x, __m128i y);
	int inE[4] = {INT_MIN, INT_MAX, 13, 0};
	int inF[4] = {INT_MIN, INT_MAX, INT_MAX, INT_MIN};
	int result1[4];
	
	x = _mm_setr_epi32(inE[0], inE[1], inE[2], inE[3]);
	y = _mm_setr_epi32(inF[0], inF[1], inF[2], inF[3]);
	memcpy(result1, &x, sizeof(result1));//for debug
	memcpy(result1, &y, sizeof(result1));//for debug
	x = __add_sat_4i32(x, y);
	memcpy(result1, &x, sizeof(result1));
	
	for(char i = 0; i< 4; i++)
	{
		int expected = (int) inE[i] + (int) inF[i];
		if( inF[i] > 0 )
        {
            if( expected < inE[i] )
                expected = INT_MAX;
        }
        else
        {
            if( expected > inE[i] )
                expected = INT_MIN;
        }
		     	
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_add_sat_4i32", (int)expected, result1[i]);
	}
//TEST: extern "C" INT_FUNC_DECL  __m128i __add_sat_4u32(__m128i x, __m128i y);
//TEST: extern "C" INT_FUNC_DECL  __m128i __add_sat_2i64(__m128i x, __m128i y);
//TEST: extern "C" INT_FUNC_DECL  __m128i __add_sat_2u64(__m128i x, __m128i y);

//TEST: extern "C" INT_FUNC_DECL  __m64 __add_sat_8i8(__m64 x, __m64 y);
//TEST: extern "C" INT_FUNC_DECL  __m64 __add_sat_8u8(__m64 x, __m64 y);
//TEST: extern "C" INT_FUNC_DECL  __m64 __add_sat_4i16(__m64 x, __m64 y);
//TEST: extern "C" INT_FUNC_DECL  __m64 __add_sat_4u16(__m64 x, __m64 
//TEST: extern "C" INT_FUNC_DECL  __m64 __add_sat_2i32(__m64 x, __m64 y);
//TEST: extern "C" INT_FUNC_DECL  __m64 __add_sat_2u32(__m64 x, __m64 y);
	int inG[2] = {3, INT_MAX};
	int inH[2] = {INT_MIN, INT_MAX};
	__m64 x64, y64;
	
	
	x64 = _mm_setr_pi32(inG[0], inG[1]);
	y64 = _mm_setr_pi32(inH[0], inH[1]);
	
	x64 = __add_sat_2u32(x64, y64);
	memcpy(result1, &x64, sizeof(result1));
	
	for(char i = 0; i< 2; i++)
	{
		unsigned int expected =  inG[i] + inH[i];
		

		if( expected <inG[i] )
        {
            expected = UINT_MAX;
        }
             	
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_add_sat_2u32", (int)expected, result1[i]);
	}
//TEST: extern "C" INT_FUNC_DECL  __m64 __add_sat_1i64(__m64 x, __m64 y);
//TEST: extern "C" INT_FUNC_DECL  __m64 __add_sat_1u64(__m64 x, __m64 y);

}


void Integer_Test::__hadd_test()
{
//extern "C" INT_FUNC_DECL  __m128i __hadd_16u8(__m128i x, __m128i y);
	char inA[16];
	char inB[16];
	for(unsigned int i = 0; i< 16; i++)
	{
		inA[i] = i;
		inB[i] = i;
	}
	__m128i x = _mm_setr_epi8(inA[0], inA[1], inA[2], inA[3], inA[4], inA[5], inA[6], inA[7], inA[8], inA[9], inA[10], inA[11], inA[12], inA[13], inA[14], inA[15]);
	__m128i y = _mm_setr_epi8(inB[0], inB[1], inB[2], inB[3], inB[4], inB[5], inB[6], inB[7], inB[8], inB[9], inB[10], inB[11], inB[12], inB[13], inB[14], inB[15]);
	char result[16];
	x = __hadd_16u8(x, y);
	memcpy(result, &x, sizeof(result));
	
	for(char i = 0; i< 16; i++)
	{
		 char expected = (inA[i] + inB[i]) >> 1;
	     
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_hadd_16u8", (char)expected, result[i]);
	}

//extern "C" INT_FUNC_DECL  __m128i __hadd_16i8(__m128i x, __m128i y);
	x = _mm_setr_epi8(inA[0], inA[1], inA[2], inA[3], inA[4], inA[5], inA[6], inA[7], inA[8], inA[9], inA[10], inA[11], inA[12], inA[13], inA[14], inA[15]);
	y = _mm_setr_epi8(inB[0], inB[1], inB[2], inB[3], inB[4], inB[5], inB[6], inB[7], inB[8], inB[9], inB[10], inB[11], inB[12], inB[13], inB[14], inB[15]);
	x = __hadd_16i8(x, y);
	memcpy(result, &x, sizeof(result));
	
	for(char i = 0; i< 16; i++)
	{
		 char expected = (inA[i] + inB[i]) >> 1;
	     
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_hadd_16i8", (char)expected, result[i]);
	}
//extern "C" INT_FUNC_DECL  __m128i __hadd_8u16(__m128i x, __m128i y);
	x = _mm_setr_epi16(inA[0], inA[1], inA[2], inA[3], inA[4], inA[5], inA[6], inA[7]);
	y = _mm_setr_epi16(inB[0], inB[1], inB[2], inB[3], inB[4], inB[5], inB[6], inB[7]);
	x = __hadd_8u16(x, y);
	short result1[8];
	memcpy(result1, &x, sizeof(result1));
	
	for(char i = 0; i< 8; i++)
	{
		 short expected = ((int)(inA[i]) + (int)(inB[i])) >> 1;
	     
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_hadd_8u16", expected, result1[i]);
	}
//extern "C" INT_FUNC_DECL  __m128i __hadd_8i16(__m128i x, __m128i y);
	x = _mm_setr_epi16(inA[0], inA[1], inA[2], inA[3], inA[4], inA[5], inA[6], inA[7]);
	y = _mm_setr_epi16(inB[0], inB[1], inB[2], inB[3], inB[4], inB[5], inB[6], inB[7]);
	x = __hadd_8i16(x, y);
	
	memcpy(result1, &x, sizeof(result1));
	
	for(char i = 0; i< 8; i++)
	{
		 short expected = (int)(inA[i]) + (int)(inB[i]) >> 1;
	     
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_hadd_8i16", expected, result1[i]);
	}

//extern "C" INT_FUNC_DECL  __m128i __hadd_4u32(__m128i x, __m128i y);
	x = _mm_setr_epi32(inA[0], inA[1], inA[2], inA[3]);
	y = _mm_setr_epi32(inB[0], inB[1], inB[2], inB[3]);
	x = __hadd_4u32(x, y);
	unsigned int result2[4];
	
	memcpy(result2, &x, sizeof(result2));
	
	for(char i = 0; i< 4; i++)
	{
		 unsigned int expected = (inA[i] + inB[i]) >> 1;
	     
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_hadd_4u32", expected, result2[i]);
	}
//extern "C" INT_FUNC_DECL  __m128i __hadd_4i32(__m128i x, __m128i y);
	x = _mm_setr_epi32(inA[0], inA[1], inA[2], inA[3]);
	y = _mm_setr_epi32(inB[0], inB[1], inB[2], inB[3]);
	x = __hadd_4i32(x, y);
	int result3[4];
	
	memcpy(result3, &x, sizeof(result3));
	
	for(char i = 0; i< 4; i++)
	{
		 int expected = (inA[i] + inB[i]) >> 1;
	     
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_hadd_4i32", expected, result3[i]);
	}

//extern "C" INT_FUNC_DECL  __m128i __hadd_2u64(__m128i x, __m128i y);
	__int64 input[2] = {30, 40};
	__m64 inC, inD;

	memcpy(&inC, &input[0], sizeof(inC));
	memcpy(&inD, &input[1], sizeof(inD));
	
	x = _mm_setr_epi64(inC, inC);
	y = _mm_setr_epi64(inD, inD);

	x = __hadd_2u64(x, y);
	__int64 result4[2];
	

	memcpy(result4, &x, sizeof(result4));

	
	 __int64 expected = (input[0] + input[1]) >> 1;
	     
	 CPPUNIT_ASSERT_EQUAL_MESSAGE("_hadd_2u64", expected, result4[0]);
	 CPPUNIT_ASSERT_EQUAL_MESSAGE("_hadd_2u64", expected, result4[1]);
	
//extern "C" INT_FUNC_DECL  __m128i __hadd_2i64(__m128i x, __m128i y);
	memcpy(&inC, &input[0], sizeof(inC));
	memcpy(&inD, &input[1], sizeof(inD));
	
	x = _mm_setr_epi64(inC, inC);
	y = _mm_setr_epi64(inD, inD);

	x = __hadd_2i64(x, y);
	

	memcpy(result4, &x, sizeof(result4));

	
	 expected = (input[0] + input[1]) >> 1;
	     
	 CPPUNIT_ASSERT_EQUAL_MESSAGE("_hadd_2i64", expected, result4[0]);
	 CPPUNIT_ASSERT_EQUAL_MESSAGE("_hadd_2i64", expected, result4[1]);


//extern "C" INT_FUNC_DECL  __m64 __hadd_8u8(__m64 x, __m64 y);
	__m64 x64, y64, z64;
	__declspec(align(16)) char result5[16];
	
	
	x64 = _mm_setr_pi8(inA[0], inA[1], inA[2], inA[3], inA[4], inA[5], inA[6], inA[7]);
	y64 = _mm_setr_pi8(inB[0], inB[1], inB[2], inB[3], inB[4], inB[5], inB[6], inB[7]);

	__m128i res_8i8 = __hadd_8u8(x64, y64);
	memcpy(result5, &res_8i8, sizeof(result5));
	
	for(char i = 0; i< 8; i++)
	{
		char expected = ( inA[i] +  inB[i]) >> 1;
		     	
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_hadd_8u8", expected, (char)result5[i]);
	}
//extern "C" INT_FUNC_DECL  __m64 __hadd_8i8(__m64 x, __m64 y);
	res_8i8 = __hadd_8i8(x64, y64);
	_mm_store_si128((__m128i *)&result5, res_8i8);
	
	
	for(char i = 0; i< 8; i++)
	{
		char expected = ( inA[i] +  inB[i]) >> 1;
		     	
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_hadd_8i8", expected, (char)result5[i]);
	}

//extern "C" INT_FUNC_DECL  __m64 __hadd_4u16(__m64 x, __m64 y);	
	__declspec(align(16)) short result6[8];
	
	x64 = _mm_setr_pi16(inA[0], inA[1], inA[2], inA[3]);
	y64 = _mm_setr_pi16(inB[0], inB[1], inB[2], inB[3]);

	res_8i8 = __hadd_4u16(x64, y64);
	_mm_store_si128((__m128i *)result6, res_8i8);
	
	
	for(char i = 0; i< 4; i++)
	{
		short expected = ( inA[i] +  inB[i]) >> 1;
		     	
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_hadd_4u16", expected, result6[i]);
	}

//extern "C" INT_FUNC_DECL  __m64 __hadd_4i16(__m64 x, __m64 y);
	res_8i8 = __hadd_4i16(x64, y64);
	_mm_store_si128((__m128i *)result6, res_8i8);
	
	for(char i = 0; i< 4; i++)
	{
		short expected = ( inA[i] +  inB[i]) >> 1;
		     	
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_hadd_4i16", expected, result6[i]);
	}
//extern "C" INT_FUNC_DECL  __m64 __hadd_2u32(__m64 x, __m64 y);
	int result7[2];
	
	x64 = _mm_setr_pi32(inA[0], inA[1]);
	y64 = _mm_setr_pi32(inB[0], inB[1]);

	z64 = __hadd_2u32(x64, y64);
	memcpy(result7, &z64, sizeof(z64));
	
	for(char i = 0; i< 2; i++)
	{
		int expected = ( inA[i] +  inB[i]) >> 1;
		     	
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_hadd_2u32", expected, result7[i]);
	}
//extern "C" INT_FUNC_DECL  __m64 __hadd_2i32(__m64 x, __m64 y);
	z64 = __hadd_2i32(x64, y64);
	memcpy(result7, &z64, sizeof(z64));
	
	for(char i = 0; i< 2; i++)
	{
		int expected = ( inA[i] +  inB[i]) >> 1;
		     	
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_hadd_2i32", expected, result7[i]);
	}
//extern "C" INT_FUNC_DECL  __m64 __hadd_1u64(__m64 x, __m64 y);
	__int64 result8;
	
	result8 = __hadd_1u64(input[0], input[1]);
	
	expected = (input[0] + input[1]) >> 1;
	     
	CPPUNIT_ASSERT_EQUAL_MESSAGE("_hadd_1u64", expected, result8);
	
//extern "C" INT_FUNC_DECL  __m64 __hadd_1i64(__m64 x, __m64 y);
		
	result8 = __hadd_1i64(input[0], input[1]);
	
	 expected = (input[0] + input[1]) >> 1;
	     
	CPPUNIT_ASSERT_EQUAL_MESSAGE("_hadd_2i64", expected, result8);
	
}

void Integer_Test::__rhadd_test()
{
//extern "C" INT_FUNC_DECL  __m128i __hadd_16u8(__m128i x, __m128i y);
	char inA[16];
	char inB[16];
	for(unsigned int i = 0; i< 16; i++)
	{
		inA[i] = i;
		inB[i] = i;
	}
	__m128i x = _mm_setr_epi8(inA[0], inA[1], inA[2], inA[3], inA[4], inA[5], inA[6], inA[7], inA[8], inA[9], inA[10], inA[11], inA[12], inA[13], inA[14], inA[15]);
	__m128i y = _mm_setr_epi8(inB[0], inB[1], inB[2], inB[3], inB[4], inB[5], inB[6], inB[7], inB[8], inB[9], inB[10], inB[11], inB[12], inB[13], inB[14], inB[15]);
	unsigned char result[16];
	x = __rhadd_16u8(x, y);
	memcpy(result, &x, sizeof(result));
	
	for(char i = 0; i< 16; i++)
	{
		 unsigned char expected = ((unsigned char)inA[i] + (unsigned char)inB[i] +1) >> 1;
	     
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_rhadd_16u8", expected, result[i]);
	}

//extern "C" INT_FUNC_DECL  __m128i __rhadd_16i8(__m128i x, __m128i y);
	x = _mm_setr_epi8(inA[0], inA[1], inA[2], inA[3], inA[4], inA[5], inA[6], inA[7], inA[8], inA[9], inA[10], inA[11], inA[12], inA[13], inA[14], inA[15]);
	y = _mm_setr_epi8(inB[0], inB[1], inB[2], inB[3], inB[4], inB[5], inB[6], inB[7], inB[8], inB[9], inB[10], inB[11], inB[12], inB[13], inB[14], inB[15]);
	x = __rhadd_16i8(x, y);
	memcpy(result, &x, sizeof(result));
	
	for(char i = 0; i< 16; i++)
	{
		 char expected = (inA[i] + inB[i]+1) >> 1;
	     
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_rhadd_16i8", (char)expected, (char)result[i]);
	}
//extern "C" INT_FUNC_DECL  __m128i __rhadd_8u16(__m128i x, __m128i y);
	x = _mm_setr_epi16(inA[0], inA[1], inA[2], inA[3], inA[4], inA[5], inA[6], inA[7]);
	y = _mm_setr_epi16(inB[0], inB[1], inB[2], inB[3], inB[4], inB[5], inB[6], inB[7]);
	x = __rhadd_8u16(x, y);
	unsigned short result1[8];
	memcpy(result1, &x, sizeof(result1));
	
	for(char i = 0; i< 8; i++)
	{
		 unsigned short expected = (unsigned int)(inA[i]) + (unsigned int)(inB[i]) +1 >> 1;
	     
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_rhadd_8u16", expected, result1[i]);
	}
//extern "C" INT_FUNC_DECL  __m128i __rhadd_8i16(__m128i x, __m128i y);
	x = _mm_setr_epi16(inA[0], inA[1], inA[2], inA[3], inA[4], inA[5], inA[6], inA[7]);
	y = _mm_setr_epi16(inB[0], inB[1], inB[2], inB[3], inB[4], inB[5], inB[6], inB[7]);
	x = __rhadd_8i16(x, y);
	
	memcpy(result1, &x, sizeof(result1));
	
	for(char i = 0; i< 8; i++)
	{
		 short expected = (int)(inA[i]) + (int)(inB[i]) +1 >> 1;
	     
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_rhadd_8i16", expected, (short)result1[i]);
	}

//extern "C" INT_FUNC_DECL  __m128i __rhadd_4u32(__m128i x, __m128i y);
	x = _mm_setr_epi32(inA[0], inA[1], inA[2], inA[3]);
	y = _mm_setr_epi32(inB[0], inB[1], inB[2], inB[3]);
	x = __rhadd_4u32(x, y);
	unsigned int result2[4];
	
	memcpy(result2, &x, sizeof(result2));
	
	for(char i = 0; i< 4; i++)
	{
		 unsigned int expected = (inA[i] + inB[i] +1) >> 1;
	     
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_rhadd_4u32", expected, result2[i]);
	}
//extern "C" INT_FUNC_DECL  __m128i __rhadd_4i32(__m128i x, __m128i y);
	x = _mm_setr_epi32(inA[0], inA[1], inA[2], inA[3]);
	y = _mm_setr_epi32(inB[0], inB[1], inB[2], inB[3]);
	x = __rhadd_4i32(x, y);
	int result3[4];
	
	memcpy(result3, &x, sizeof(result3));
	
	for(char i = 0; i< 4; i++)
	{
		 int expected = (inA[i] + inB[i] +1) >> 1;
	     
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_rhadd_4i32", expected, result3[i]);
	}

//extern "C" INT_FUNC_DECL  __m128i __rhadd_2u64(__m128i x, __m128i y);
	__int64 input[2] = {30, 40};
	__m64 inC, inD;

	memcpy(&inC, &input[0], sizeof(inC));
	memcpy(&inD, &input[1], sizeof(inD));
	
	x = _mm_setr_epi64(inC, inC);
	y = _mm_setr_epi64(inD, inD);

	x = __rhadd_2u64(x, y);
	__int64 result4[2];
	

	memcpy(result4, &x, sizeof(result4));

	
	 __int64 expected = (input[0] + input[1] +1) >> 1;
	     
	 CPPUNIT_ASSERT_EQUAL_MESSAGE("_rhadd_2u64", expected, result4[0]);
	 CPPUNIT_ASSERT_EQUAL_MESSAGE("_rhadd_2u64", expected, result4[1]);
	
//extern "C" INT_FUNC_DECL  __m128i __rhadd_2i64(__m128i x, __m128i y);
	memcpy(&inC, &input[0], sizeof(inC));
	memcpy(&inD, &input[1], sizeof(inD));
	
	x = _mm_setr_epi64(inC, inC);
	y = _mm_setr_epi64(inD, inD);

	x = __rhadd_2i64(x, y);
	

	memcpy(result4, &x, sizeof(result4));

	
	 expected = (input[0] + input[1] +1) >> 1;
	     
	 CPPUNIT_ASSERT_EQUAL_MESSAGE("_rhadd_2i64", expected, result4[0]);
	 CPPUNIT_ASSERT_EQUAL_MESSAGE("_rhadd_2i64", expected, result4[1]);


//extern "C" INT_FUNC_DECL  __m64 __rhadd_8u8(__m64 x, __m64 y);
	__m64 x64, y64, z64;
	__m128i z128i;	
	__declspec(align(16)) char result5[16];
	
	x64 = _mm_setr_pi8(inA[0], inA[1], inA[2], inA[3], inA[4], inA[5], inA[6], inA[7]);
	y64 = _mm_setr_pi8(inB[0], inB[1], inB[2], inB[3], inB[4], inB[5], inB[6], inB[7]);

	z128i = __rhadd_8u8(x64, y64);
	_mm_store_si128((__m128i *)result5, z128i);

	
	for(char i = 0; i< 8; i++)
	{
		char expected = ( inA[i] +  inB[i] +1) >> 1;
		     	
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_rhadd_8u8", expected, (char)result5[i]);
	}
//extern "C" INT_FUNC_DECL  __m64 __rhadd_8i8(__m64 x, __m64 y);
	z128i = __rhadd_8i8(x64, y64);	
	_mm_store_si128((__m128i *)result5, z128i);
	for(char i = 0; i< 8; i++)
	{
		char expected = ( inA[i] +  inB[i] +1) >> 1;
		     	
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_rhadd_8i8", expected, (char)result5[i]);
	}

//extern "C" INT_FUNC_DECL  __m64 __rhadd_4u16(__m64 x, __m64 y);	
	__declspec(align(16)) short result6[8];

	x64 = _mm_setr_pi16(inA[0], inA[1], inA[2], inA[3]);
	y64 = _mm_setr_pi16(inB[0], inB[1], inB[2], inB[3]);

	z128i = __rhadd_4u16(x64, y64);
	_mm_store_si128((__m128i *)result6, z128i);	
	
	for(char i = 0; i< 4; i++)
	{
		short expected =  (unsigned int)(inA[i]) +  (unsigned int)(inB[i]) +1 >> 1;
		     	
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_rhadd_4u16", expected, result6[i]);
	}

//extern "C" INT_FUNC_DECL  __m64 __rhadd_4i16(__m64 x, __m64 y);
	z128i = __rhadd_4i16(x64, y64);
	_mm_store_si128((__m128i *)result6, z128i);	
	
	for(char i = 0; i< 4; i++)
	{
		short expected = ( inA[i] +  inB[i] +1) >> 1;
		     	
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_rhadd_4i16", expected, result6[i]);
	}
//extern "C" INT_FUNC_DECL  __m64 __rhadd_2u32(__m64 x, __m64 y);
	int result7[2];
	
	x64 = _mm_setr_pi32(inA[0], inA[1]);
	y64 = _mm_setr_pi32(inB[0], inB[1]);

	z64 = __rhadd_2u32(x64, y64);
	memcpy(result7, &z64, sizeof(z64));
	
	for(char i = 0; i< 2; i++)
	{
		int expected = ( inA[i] +  inB[i] +1) >> 1;
		     	
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_rhadd_2u32", expected, result7[i]);
	}
//extern "C" INT_FUNC_DECL  __m64 __rhadd_2i32(__m64 x, __m64 y);
	z64 = __rhadd_2i32(x64, y64);
	memcpy(result7, &z64, sizeof(z64));
	
	for(char i = 0; i< 2; i++)
	{
		int expected = ( inA[i] +  inB[i] +1) >> 1;
		     	
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_rhadd_2i32", expected, result7[i]);
	}
//extern "C" INT_FUNC_DECL  __m64 __rhadd_1u64(__m64 x, __m64 y);
	__int64 result8;
	
	
	result8 = __rhadd_1u64(input[0], input[1]);
	
    expected = (input[0] + input[1] +1) >> 1;
	     
	CPPUNIT_ASSERT_EQUAL_MESSAGE("_rhadd_1u64", expected, result8);
	
//extern "C" INT_FUNC_DECL  __m64 __rhadd_1i64(__m64 x, __m64 y);
		
	result8 = __rhadd_1i64(input[0], input[1]);
		
	expected = (input[0] + input[1] +1) >> 1;
	     
	CPPUNIT_ASSERT_EQUAL_MESSAGE("_rhadd_2i64", expected, result8);
	
}


void Integer_Test::__max_test()
{
//TEST: extern "C" __declspec(dllexport) __m128i __max_16u8(__m128i x, __m128i y);

	__m128i x = _mm_set1_epi8(UCHAR_MAX);
	__m128i y = _mm_set1_epi8(UCHAR_MIN);
	__m128i z;
	unsigned char result[16];
	
	z = __max_16u8(x, y);
	memcpy(result, &z, sizeof(z));
	
	for(char i = 0; i< 16; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_max_16u8", (unsigned char)UCHAR_MAX, (unsigned char)result[i]);
	}

//extern "C" INT_FUNC_DECL  __m128i __max_16i8(__m128i x, __m128i y);
	x = _mm_set1_epi8(CHAR_MAX);
	y = _mm_set1_epi8(CHAR_MIN);

	//max function Crash 
	z = __max_16i8(y, x);
	memcpy(result, &z, sizeof(z));
	
	for(char i = 0; i< 16; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_max_16u8", (char)CHAR_MAX, ( char)result[i]);
	}

//extern "C" INT_FUNC_DECL  __m128i __max_8u16(__m128i x, __m128i y);
	x = _mm_set1_epi16(USHRT_MAX);
	y = _mm_set1_epi16(USHRT_MIN);

	z = __max_8u16(x, y);

	short result1[16];
	memcpy(result1, &z, sizeof(z));
	
	for(char i = 0; i< 8; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_max_8u16", (unsigned short)USHRT_MAX, ( unsigned short)result1[i]);
	}

//extern "C" INT_FUNC_DECL  __m128i __max_8i16(__m128i x, __m128i y);
	x = _mm_set1_epi16(SHRT_MAX);
	y = _mm_set1_epi16(SHRT_MIN);

	z = __max_8i16(x, y);


	memcpy(result1, &z, sizeof(z));
	
	for(char i = 0; i< 8; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_max_8i16", (short)SHRT_MAX, (short)result1[i]);
	}
//extern "C" INT_FUNC_DECL  __m128i __max_4u32(__m128i x, __m128i y);
	x = _mm_set1_epi32(UINT_MAX);
	y = _mm_set1_epi32(UINT_MIN);

	z = __max_4u32(x, y);

	int result2[4];
	memcpy(result2, &z, sizeof(z));
	
	for(char i = 0; i< 4; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_max_4u32", (unsigned int)UINT_MAX, (unsigned int)result2[i]);
	}

//extern "C" INT_FUNC_DECL  __m128i __max_4i32(__m128i x, __m128i y);
	x = _mm_set1_epi32(INT_MAX);
	y = _mm_set1_epi32(INT_MIN);

	z = __max_4i32(x, y);

	
	memcpy(result2, &z, sizeof(z));
	
	for(char i = 0; i< 4; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_max_4i32", (int)INT_MAX, (int)result2[i]);
	}

//extern "C" INT_FUNC_DECL  __m128i __max_2u64(__m128i x, __m128i y);
	__int64  m_max[2] = {ULONG_MAX, ULONG_MAX};
	__int64  m_min[2] = {0, 0};
	
	memcpy(&x, &m_max, sizeof(x));
	memcpy(&y, &m_min, sizeof(y));
	__int64 result3[2];
	
	z = __max_2u64(x,y);
	memcpy(result3, &z, sizeof(z));
	
	for(unsigned int i = 0; i< 2; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_max_2u64", (unsigned __int64)ULONG_MAX, (unsigned __int64)result3[i]);
	}

//extern "C" INT_FUNC_DECL  __m128i __max_2i64(__m128i x, __m128i y);
	m_max[0] = m_max[1] = LONG_MAX;
		
	memcpy(&x, &m_max, sizeof(x));
	memcpy(&y, &m_min, sizeof(y));

	z = __max_2i64(x,y);

	memcpy(result3, &z, sizeof(z));
	
	for(unsigned int i = 0; i< 2; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_max_2u64", (__int64)LONG_MAX, (__int64)result3[i]);
	}

//extern "C" INT_FUNC_DECL  __m64 __max_8u8(__m64 x, __m64 y);
	__m64 x64, y64;	
	__declspec(align(16)) unsigned char result5[16];

	x64 = _mm_set1_pi8(UCHAR_MAX);
	y64 = _mm_set1_pi8(UCHAR_MIN);
	

	__m128i z128i = __max_8u8(x64, y64);
	_mm_store_si128((__m128i *)result5, z128i);	
	
	for(char i = 0; i< 8; i++)
	{
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_max_8u8", (unsigned char)UCHAR_MAX, (unsigned char)result5[i]);
	}
//extern "C" INT_FUNC_DECL  __m64 __max_8i8(__m64 x, __m64 y);	
	__declspec(align(16)) char result6[16];

	x64 = _mm_set1_pi8(CHAR_MAX);
	y64 = _mm_set1_pi8(CHAR_MIN);
	

	z128i = __max_8i8(x64, y64);	
	_mm_store_si128((__m128i *)result6, z128i);	
	
	for(char i = 0; i< 8; i++)
	{
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_max_8i8", (char)CHAR_MAX, (char)result6[i]);
	}
//extern "C" INT_FUNC_DECL  __m64 __max_4u16(__m64 x, __m64 y);	
	__declspec(align(16)) unsigned short result7[8];

	x64 = _mm_set1_pi16(USHRT_MAX);
	y64 = _mm_set1_pi16(USHRT_MIN);
	

	z128i = __max_4u16(x64, y64);	
	_mm_store_si128((__m128i *)result7, z128i);	
	
	for(char i = 0; i< 4; i++)
	{
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_max_4u16", (unsigned short)USHRT_MAX, (unsigned short)result7[i]);
	}
//extern "C" INT_FUNC_DECL  __m64 __max_4i16(__m64 x, __m64 y);
	x64 = _mm_set1_pi16(SHRT_MAX);
	y64 = _mm_set1_pi16(SHRT_MIN);
	

	z128i = __max_4i16(x64, y64);
	_mm_store_si128((__m128i *)result7, z128i);		
	
	for(char i = 0; i< 4; i++)
	{
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_max_4i16", (short)SHRT_MAX, (short)result7[i]);
	}

//extern "C" INT_FUNC_DECL  __m64 __max_2u32(__m64 x, __m64 y);
	unsigned int result8[2];

	x64 = _mm_set1_pi32(UINT_MAX);
	y64 = _mm_set1_pi32(UINT_MIN);
	

	__m64 z64 = __max_2u32(x64, y64);
	memcpy(result8, &z64, sizeof(z64));
	
	for(char i = 0; i< 2; i++)
	{
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_max_2u32", (unsigned int)UINT_MAX, (unsigned int)result8[i]);
	}
//extern "C" INT_FUNC_DECL  __m64 __max_2i32(__m64 x, __m64 y);
    int result9[2];
	x64 = _mm_set1_pi32(INT_MAX);
	y64 = _mm_set1_pi32(INT_MIN);
	

	z64 = __max_2i32(x64, y64);
	memcpy(result9, &z64, sizeof(z64));
	
	for(char i = 0; i< 2; i++)
	{
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_max_2i32", ( int)INT_MAX, ( int)result9[i]);
	}

//extern "C" INT_FUNC_DECL  __m64 __max_1u64(__m64 x, __m64 y);

	__int64  m64_max = ULONG_MAX;
	__int64  m64_min = 0;
	__int64 result10;
	
	result10 = __max_1u64(m64_max,m64_min);
	
	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("_max_1u64", (unsigned __int64)ULONG_MAX, (unsigned __int64)result10);

//extern "C" INT_FUNC_DECL  __m64 __max_1i64(__m64 x, __m64 y);
	m64_max = LONG_MAX;
	
	result10 = __max_1i64(m64_max,m64_min);

	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("_max_1i64", (__int64)LONG_MAX, (__int64)result10);

}

void Integer_Test::__min_test()
{
//TEST: extern "C" __declspec(dllexport) __m128i __min_16u8(__m128i x, __m128i y);

	__m128i x = _mm_set1_epi8(UCHAR_MAX);
	__m128i y = _mm_set1_epi8(UCHAR_MIN);
	__m128i z;
	unsigned char result[16];
	
	z = __min_16u8(x, y);
	memcpy(result, &z, sizeof(z));
	
	for(char i = 0; i< 16; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_min_16u8", (unsigned char)UCHAR_MIN, (unsigned char)result[i]);
	}

//extern "C" INT_FUNC_DECL  __m128i __min_16i8(__m128i x, __m128i y);
	x = _mm_set1_epi8(CHAR_MAX);
	y = _mm_set1_epi8(CHAR_MIN);

	//min function Crash 
	z = __min_16i8(y, x);
	memcpy(result, &z, sizeof(z));
	
	for(char i = 0; i< 16; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_min_16u8", (char)CHAR_MIN, ( char)result[i]);
	}

//extern "C" INT_FUNC_DECL  __m128i __min_8u16(__m128i x, __m128i y);
	x = _mm_set1_epi16(USHRT_MAX);
	y = _mm_set1_epi16(USHRT_MIN);

	z = __min_8u16(x, y);

	short result1[16];
	memcpy(result1, &z, sizeof(z));
	
	for(char i = 0; i< 8; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_min_8u16", (unsigned short)USHRT_MIN, ( unsigned short)result1[i]);
	}

//extern "C" INT_FUNC_DECL  __m128i __min_8i16(__m128i x, __m128i y);
	x = _mm_set1_epi16(SHRT_MAX);
	y = _mm_set1_epi16(SHRT_MIN);

	z = __min_8i16(x, y);


	memcpy(result1, &z, sizeof(z));
	
	for(char i = 0; i< 8; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_min_8i16", (short)SHRT_MIN, (short)result1[i]);
	}
//extern "C" INT_FUNC_DECL  __m128i __min_4u32(__m128i x, __m128i y);
	x = _mm_set1_epi32(UINT_MAX);
	y = _mm_set1_epi32(UINT_MIN);

	z = __min_4u32(x, y);

	int result2[4];
	memcpy(result2, &z, sizeof(z));
	
	for(char i = 0; i< 4; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_min_4u32", (unsigned int)UINT_MIN, (unsigned int)result2[i]);
	}

//extern "C" INT_FUNC_DECL  __m128i __min_4i32(__m128i x, __m128i y);
	x = _mm_set1_epi32(INT_MAX);
	y = _mm_set1_epi32(INT_MIN);

	z = __min_4i32(x, y);

	
	memcpy(result2, &z, sizeof(z));
	
	for(char i = 0; i< 4; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_min_4i32", (int)INT_MIN, (int)result2[i]);
	}

//extern "C" INT_FUNC_DECL  __m128i __min_2u64(__m128i x, __m128i y);
	__int64  m_max[2] = {ULONG_MAX, ULONG_MAX};
	__int64  m_min[2] = {0, 0};
	
	memcpy(&x, &m_max, sizeof(x));
	memcpy(&y, &m_min, sizeof(y));
	unsigned __int64 result3[2];
	
	z = __min_2u64(x,y);
	memcpy(result3, &z, sizeof(z));
	
	for(unsigned int i = 0; i< 2; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_min_2u64", (unsigned __int64)m_min[0], (unsigned __int64)result3[i]);
	}

//extern "C" INT_FUNC_DECL  __m128i __min_2i64(__m128i x, __m128i y);
	m_max[0] = m_max[1] = LONG_MAX;
	m_min[0] = m_min[1] = LONG_MIN;
		
	memcpy(&x, &m_max, sizeof(x));
	memcpy(&y, &m_min, sizeof(y));
	__int64 result4[2];

	z = __min_2i64(x,y);

	memcpy(result4, &z, sizeof(z));
	
	for(unsigned int i = 0; i< 2; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_min_2i64", (__int64)LONG_MIN, (__int64)result4[i]);
	}

//extern "C" INT_FUNC_DECL  __m64 __min_8u8(__m64 x, __m64 y);
	__m64 x64, y64;
	__declspec(align(16)) unsigned char result5[16];

	x64 = _mm_set1_pi8(UCHAR_MAX);
	y64 = _mm_set1_pi8(UCHAR_MIN);
	

	__m128i z128i = __min_8u8(x64, y64);
	_mm_store_si128((__m128i *)result5, z128i);			
	
	for(char i = 0; i< 8; i++)
	{
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_min_8u8", (unsigned char)UCHAR_MIN, (unsigned char)result5[i]);
	}
//extern "C" INT_FUNC_DECL  __m64 __min_8i8(__m64 x, __m64 y);
	__declspec(align(16)) char result6[16];

	x64 = _mm_set1_pi8(CHAR_MAX);
	y64 = _mm_set1_pi8(CHAR_MIN);
	

	z128i = __min_8i8(x64, y64);	
	_mm_store_si128((__m128i *)result6, z128i);			

	for(char i = 0; i< 8; i++)
	{
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_min_8i8", (char)CHAR_MIN, (char)result6[i]);
	}
//extern "C" INT_FUNC_DECL  __m64 __min_4u16(__m64 x, __m64 y);
	__declspec(align(16)) unsigned short result7[8];

	x64 = _mm_set1_pi16(USHRT_MAX);
	y64 = _mm_set1_pi16(USHRT_MIN);
	

	z128i = __min_4u16(x64, y64);
	_mm_store_si128((__m128i *)result7, z128i);			
	
	for(char i = 0; i< 4; i++)
	{
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_min_4u16", (unsigned short)USHRT_MIN, (unsigned short)result7[i]);
	}
//extern "C" INT_FUNC_DECL  __m64 __min_4i16(__m64 x, __m64 y);
	x64 = _mm_set1_pi16(SHRT_MAX);
	y64 = _mm_set1_pi16(SHRT_MIN);
	

	z128i = __min_4i16(x64, y64);	
	_mm_store_si128((__m128i *)result7, z128i);			
	
	for(char i = 0; i< 4; i++)
	{
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_min_4i16", (short)SHRT_MIN, (short)result7[i]);
	}

//extern "C" INT_FUNC_DECL  __m64 __min_2u32(__m64 x, __m64 y);
	unsigned int result8[2];

	x64 = _mm_set1_pi32(UINT_MAX);
	y64 = _mm_set1_pi32(UINT_MIN);
	

	__m64 z64 = __min_2u32(x64, y64);
	memcpy(result8, &z64, sizeof(z64));
	
	for(char i = 0; i< 2; i++)
	{
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_min_2u32", (unsigned int)UINT_MIN, (unsigned int)result8[i]);
	}
//extern "C" INT_FUNC_DECL  __m64 __min_2i32(__m64 x, __m64 y);
    int result9[2];
	x64 = _mm_set1_pi32(INT_MAX);
	y64 = _mm_set1_pi32(INT_MIN);
	

	z64 = __min_2i32(x64, y64);
	memcpy(result9, &z64, sizeof(z64));
	
	for(char i = 0; i< 2; i++)
	{
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_min_2i32", ( int)INT_MIN, ( int)result9[i]);
	}

//extern "C" INT_FUNC_DECL  __m64 __min_1u64(__m64 x, __m64 y);

	__int64  m64_max = ULONG_MAX;
	__int64  m64_min = 0;
	
	__int64 result10;
	
	result10 = __min_1u64(m64_max,m64_min);

	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("_min_1u64", (unsigned __int64)m64_min, (unsigned __int64)result10);

//extern "C" INT_FUNC_DECL  __m64 __min_1i64(__m64 x, __m64 y);
	m64_max = LONG_MAX;
		
	result10 = __min_1i64(m64_max,m64_min);

	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("_min_1i64", (__int64)m64_min, (__int64)result10);

}

void Integer_Test::__mad_sat_test()
{
	//x*y+z and saturate the result
//TEST extern "C" INT_FUNC_DECL __m128i _mad_sat_16i8(__m128i x, __m128i y, __m128i z);
	__m128i x = _mm_set1_epi8(0);
	__m128i y = _mm_set1_epi8(UCHAR_MIN);
	__m128i z = _mm_set1_epi8(UCHAR_MAX);
	unsigned char result[16];
	
	z = __mad_sat_16i8(x, y, z);
	memcpy(result, &z, sizeof(z));
	
	for(char i = 0; i< 16; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_mad_sat_16i8", (unsigned char)UCHAR_MAX, (unsigned char)result[i]);
	}

//TEST extern "C" INT_FUNC_DECL __m128i _mad_sat_2u64(__m128i x, __m128i y, __m128i z);
	unsigned __int64  m_max[2] = {ULONG_MAX, ULONG_MAX};
	unsigned __int64  m_min[2] = {0, 0};
	
	memcpy(&x, &m_max, sizeof(x));
	memcpy(&y, &m_min, sizeof(y));
	memcpy(&z, &m_max, sizeof(z));
	unsigned __int64 result3[2];
	
	z = __mad_sat_2u64(x, y, z);
	memcpy(result3, &z, sizeof(z));
	
	for(int i = 0; i< 2; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("__mad_sat_16i8", (unsigned __int64)ULONG_MAX, (unsigned __int64)result3[i]);
	}

//extern "C" INT_FUNC_DECL  __m64 _mad_sat_8i8(__m64 x, __m64 y, __m64 z);
	__declspec(align(16)) char result6[16];

	__m64 x64 = _mm_set1_pi8(CHAR_MAX);
	__m64 y64 = _mm_set1_pi8(1);
	__m64 z64 = _mm_set1_pi8(0);
	

	__m128i z128i = __mad_sat_8i8(x64, y64, z64);
	_mm_store_si128((__m128i *)result6, z128i);				
	
	for(char i = 0; i< 8; i++)
	{
		CPPUNIT_ASSERT_EQUAL_MESSAGE("__mad_sat_8i8", (char)CHAR_MAX, (char)result6[i]);
	}


}

void Integer_Test::__mul_hi_test()
{
//TEST extern "C" INT_FUNC_DECL __m128i __mul_hi_16i8(__m128i x, __m128i y);
	__m128i x = _mm_set1_epi8(UCHAR_MAX);
	__m128i y = _mm_set1_epi8(1);
	unsigned char result[16];
	
	__m128i z = __mul_hi_16i8(x, y);
	memcpy(result, &z, sizeof(z));
	
	for(char i = 0; i< 16; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_mul_hi_16i8", (unsigned char)UCHAR_MAX, (unsigned char)result[i]);
	}

//TEST extern "C" INT_FUNC_DECL __m128i __mul_hi_2i64(__m128i x, __m128i y);
	__m64 max = _mm_setr_pi32(LONG_MAX, LONG_MAX);
	__m64 one = _mm_setr_pi32(1, 1);
	x = _mm_set_epi64(one, one);
	y = _mm_set_epi64(max,max);
	__int64 result4[2];

	z = __mul_hi_2i64(x,y);

	memcpy(result4, &z, sizeof(z));
	
	for(unsigned int i = 0; i< 2; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_mul_hi_2i64", (__int64)LONG_MAX, (__int64)result4[i]);
	}

//extern "C" INT_FUNC_DECL  __m64 __mul_hi_8i8(__m64 x, __m64 y);
	__declspec(align(16)) char result6[16];

	__m64 x64 = _mm_set1_pi8(UCHAR_MAX);
	__m64 y64 = _mm_set1_pi8(1);
	__m128i z128i;
	

	z128i = __mul_hi_8i8(x64, y64);
	_mm_store_si128((__m128i *)result6, z128i);				
	
	for(char i = 0; i< 8; i++)
	{
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_mul_hi_8i8", (char)UCHAR_MAX, (char)result6[i]);
	}


}

void Integer_Test::__sub_sat_test()
{
//TEST: extern "C" INT_FUNC_DECL  __m128i __sub_sat_16i8(__m128i x, __m128i y);
	int inA[16] = {-15, 0, -13, 12, -11, -10, -9, -8, -7, -6, -5, -4, -3, -2, -1, 0};
	int inB[16] = {CHAR_MIN, CHAR_MAX, -13, 12, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	__m128i x = _mm_setr_epi8(inA[0], inA[1], inA[2], inA[3], inA[4], inA[5], inA[6], inA[7], inA[8], inA[9], inA[10], inA[11], inA[12], inA[13], inA[14], inA[15]);
	__m128i y = _mm_setr_epi8(inB[0], inB[1], inB[2], inB[3], inB[4], inB[5], inB[6], inB[7], inB[8], inB[9], inB[10], inB[11], inB[12], inB[13], inB[14], inB[15]);
	char result[16];
	x = __sub_sat_16i8(x, y);
	memcpy(result, &x, sizeof(result));
	
	for(char i = 0; i< 16; i++)
	{
		 int expected = (int) inA[i] - (int) inB[i];
	     expected = MAX( expected, CHAR_MIN );
         expected = MIN( expected, CHAR_MAX );
	
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_sub_sat_16i8", (char)expected, result[i]);
	}
//TEST: extern "C" INT_FUNC_DECL  __m128i __sub_sat_16u8(__m128i x, __m128i y);
	int inC[16] = {UCHAR_MIN, UCHAR_MAX, 13, 12, 11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	int inD[16] = {UCHAR_MIN, UCHAR_MAX, UCHAR_MAX, UCHAR_MIN, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	x = _mm_setr_epi8(inD[0], inD[1], inD[2], inD[3], inD[4], inD[5], inD[6], inD[7], inD[8], inD[9], inD[10], inD[11], inD[12], inD[13], inD[14], inD[15]);
	y = _mm_setr_epi8(inC[0], inC[1], inC[2], inC[3], inC[4], inC[5], inC[6], inC[7], inC[8], inC[9], inC[10], inC[11], inC[12], inC[13], inC[14], inC[15]);
	memcpy(result, &x, sizeof(result));
	memcpy(result, &y, sizeof(result));
	x = __sub_sat_16u8(x, y);
	memcpy(result, &x, sizeof(result));
	
	for(char i = 0; i< 16; i++)
	{
		 int expected = (int) inD[i] - (int) inC[i];
	     expected = MAX( expected, UCHAR_MIN );
         expected = MIN( expected, UCHAR_MAX );
	
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_add_sat_16u8", (unsigned char)expected, (unsigned char)result[i]);
	}
//TEST: extern "C" INT_FUNC_DECL  __m128i __sub_sat_8i16(__m128i x, __m128i y);
//TEST: extern "C" INT_FUNC_DECL  __m128i __sub_sat_8u16(__m128i x, __m128i y);
//TEST: extern "C" INT_FUNC_DECL  __m128i __sub_sat_4i32(__m128i x, __m128i y);
	int inE[4] = {INT_MIN, INT_MAX, 13, 0};
	int inF[4] = {INT_MIN, INT_MAX, INT_MAX, INT_MIN};
	int result1[4];
	
	x = _mm_setr_epi32(inE[0], inE[1], inE[2], inE[3]);
	y = _mm_setr_epi32(inF[0], inF[1], inF[2], inF[3]);
	memcpy(result1, &x, sizeof(result1));//for debug
	memcpy(result1, &y, sizeof(result1));//for debug
	x = __sub_sat_4i32(x, y);
	memcpy(result1, &x, sizeof(result1));
	
	for(char i = 0; i< 4; i++)
	{
		int expected =  inE[i] - inF[i];
		if( inF[i] < 0 )
        {
            if( expected < inE[i] )
                expected = INT_MAX;
        }
        else
        {
            if( expected > inE[i] )
                expected = INT_MIN;
        }
		     	
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_sub_sat_4i32", (int)expected, result1[i]);
	}
//TEST: extern "C" INT_FUNC_DECL  __m128i __sub_sat_4u32(__m128i x, __m128i y);
//TEST: extern "C" INT_FUNC_DECL  __m128i __sub_sat_2i64(__m128i x, __m128i y);
//TEST: extern "C" INT_FUNC_DECL  __m128i __sub_sat_2u64(__m128i x, __m128i y);

//TEST: extern "C" INT_FUNC_DECL  __m64 __sub_sat_8i8(__m64 x, __m64 y);
//TEST: extern "C" INT_FUNC_DECL  __m64 __sub_sat_8u8(__m64 x, __m64 y);
//TEST: extern "C" INT_FUNC_DECL  __m64 __sub_sat_4i16(__m64 x, __m64 y);
//TEST: extern "C" INT_FUNC_DECL  __m64 __sub_sat_4u16(__m64 x, __m64 
//TEST: extern "C" INT_FUNC_DECL  __m64 __sub_sat_2i32(__m64 x, __m64 y);
//TEST: extern "C" INT_FUNC_DECL  __m64 __sub_sat_2u32(__m64 x, __m64 y);
	unsigned int inG[2] = {3, INT_MAX};
	unsigned int inH[2] = {INT_MIN, INT_MAX};
	__m64 x64, y64;
	
	
	x64 = _mm_setr_pi32(inG[0], inG[1]);
	y64 = _mm_setr_pi32(inH[0], inH[1]);
	
	x64 = __sub_sat_2u32(x64, y64);
	memcpy(result1, &x64, sizeof(result1));
	
	for(char i = 0; i< 2; i++)
	{
		unsigned int expected =  inG[i] - inH[i];
		
		if( inG[i] < inH[i] )
        {
            expected = 0;
        }
             	
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_sub_sat_2u32", (int)expected, result1[i]);
	}
//TEST: extern "C" INT_FUNC_DECL  __m64 __sub_sat_1i64(__m64 x, __m64 y);
//TEST: extern "C" INT_FUNC_DECL  __m64 __sub_sat_1u64(__m64 x, __m64 y);

}

void Integer_Test::__clz_test()
{
//TEST: extern "C" __declspec(dllexport) __m128i __clz_16u8(__m128i x);

	__m128i x = _mm_set1_epi8(0xFF);
	__m128i z;
	unsigned char result[16];
	
	z = __clz_16u8(x);
	memcpy(result, &z, sizeof(z));
	
	for(char i = 0; i< 16; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_clz_16u8", (int)0, (int)result[i]);
	}

//extern "C" INT_FUNC_DECL  __m128i __clz_16i8(__m128i x);
	x = _mm_set1_epi8(0x0F);

	z = __clz_16i8(x);
	memcpy(result, &z, sizeof(z));
	
	for(char i = 0; i< 16; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_clz_16i8", (int)(4), (int)result[i]);
	}

//extern "C" INT_FUNC_DECL  __m128i __clz_4i32(__m128i x);
	int result1[4];
	x = _mm_set1_epi32(0x0000FFFF);
	z = __clz_4i32(x);
	memcpy(result1, &z, sizeof(z));
	
	for(char i = 0; i< 4; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_clz_4i32", (int)(16), result1[i]);
	}
}

//Returns mul_hi(a, b) + c.
void Integer_Test::__mad_hi_test()
{
	//TEST extern "C" INT_FUNC_DECL __m128i __mad_hi_16i8(__m128i x, __m128i y, __m128i z);
	__m128i x = _mm_set1_epi8(0);
	__m128i y = _mm_set1_epi8(UCHAR_MIN);
	__m128i z = _mm_set1_epi8(UCHAR_MAX);
	unsigned char result[16];
	
	z = __mad_hi_16i8(x, y, z);
	memcpy(result, &z, sizeof(z));
	
	for(char i = 0; i< 16; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_mad_hi_16i8", (int)UCHAR_MAX, (int)result[i]);
	}

//TEST extern "C" INT_FUNC_DECL __m128i __mad_hi_2u64(__m128i x, __m128i y, __m128i z);
	unsigned __int64  m_max[2] = {ULONG_MAX, ULONG_MAX};
	unsigned __int64  m_min[2] = {0, 0};
	
	memcpy(&x, &m_max, sizeof(x));
	memcpy(&y, &m_min, sizeof(y));
	memcpy(&z, &m_max, sizeof(z));
	unsigned __int64 result3[2];
	
	z = __mad_hi_2u64(x, y, z);
	memcpy(result3, &z, sizeof(z));
	
	for(int i = 0; i< 2; i++)
	{
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_mad_hi_16i8", (unsigned __int64)ULONG_MAX, (unsigned __int64)result3[i]);
	}

//extern "C" INT_FUNC_DECL  __m64 __mad_hi_8i8(__m64 x, __m64 y, __m64 z);
	__declspec(align(16)) char result6[16];

	__m64 x64 = _mm_set1_pi8(UCHAR_MAX);
	__m64 y64 = _mm_set1_pi8(1);
	__m64 z64 = _mm_set1_pi8(0);
	

	__m128i z128i = __mad_hi_8i8(x64, y64, z64);
	_mm_store_si128((__m128i *)result6, z128i);		
	
	for(char i = 0; i< 8; i++)
	{
		CPPUNIT_ASSERT_EQUAL_MESSAGE("_mad_hi_8i8", (char)UCHAR_MAX, result6[i]);
	}


}

// rotate(x, count);
void Integer_Test::__rotate_test()
{
	//TEST: extern "C" __declspec(dllexport) __m128i __rotate_8i16(__m128i x, __m128i count);
	__m128i x = _mm_set_epi16(7, 6, 5, 4, 3, 2, 1, 0);
	__m128i count = _mm_set1_epi16(1);
	short result1[8];
	x = __rotate_8i16(x, count);
	memcpy(result1, &x, sizeof(x));
	
	for(short i = 0; i< 8; i++)
	{
			short expected =  ( i << 1 ) | ( i >> ( 16 - 1 ));
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_rotate_8i16", expected, result1[i]);
	}

	//TEST: extern "C" __declspec(dllexport) __m128i __rotate_2i64(__m128i x, __m128i count);
	unsigned __int64  m_value[2] = {ULONG_MAX, 3};
	
	memcpy(&x, &m_value, sizeof(x));
	count = _mm_set_epi32(0, 3, 0, 3);
	__int64 result2[8];
	
	x = __rotate_2i64(x, count);

	memcpy(result2, &x, sizeof(x));
	for(__int64 i = 0; i< 2; i++)
	{
			__int64 expected =  ( m_value[i] << 3 ) | ( m_value[i] >> ( 64 - 3 ));
			CPPUNIT_ASSERT_EQUAL_MESSAGE("_rotate_2i64", expected, result2[i]);
	}

}
