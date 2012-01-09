#include "ConversionsTest.h"
#include <mathimf.h>
#include <math.h>
#include <ia32intrin.h>
#include "cl_conversions_declaration.h"



// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( Conversions_Test );


void Conversions_Test::setUp()
{
}


void Conversions_Test::tearDown()
{
}


void Conversions_Test::__uchar2_convert_test()
{
	_2u16 input_2u16;		
	__declspec(align(16)) unsigned char result1[16];
	__m128i tmp;

	//__convert_uchar2_ushort2 TEST
	input_2u16.a = 3;
	input_2u16.b = 4;
	tmp = __convert_uchar2_ushort2(input_2u16);
	_mm_store_si128((__m128i *)&result1, tmp);
	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__convert_uchar2_ushort2 ", (unsigned char)input_2u16.a, result1[0]);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__convert_uchar2_ushort2 ", (unsigned char)input_2u16.b, result1[1]);
	
	//__convert_uchar2_uint2
	_2u32 input_2u32 = _mm_setr_pi32 (2, 3);
	tmp = __convert_uchar2_uint2(input_2u32);
	_mm_store_si128((__m128i *)result1, tmp);

	CPPUNIT_ASSERT_EQUAL_MESSAGE("__convert_uchar2_uint2 ", (unsigned char)2, result1[0]);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__convert_uchar2_uint2 ", (unsigned char)3, result1[1]);

	//__convert_uchar2_ulong2
	_2u64 input_2u64 = _mm_setr_epi32 (8, 0, 9, 0);
	tmp = __convert_uchar2_ulong2(input_2u64);
	_mm_store_si128((__m128i *)result1, tmp);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__convert_uchar2_ulong2 ", (unsigned char)8, result1[0]);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__convert_uchar2_ulong2 ", (unsigned char)9, result1[1]);


}

