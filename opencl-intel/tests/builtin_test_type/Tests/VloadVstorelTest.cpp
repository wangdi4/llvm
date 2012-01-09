#include "VloadVstoreTest.h"
#include <mathimf.h>
#include <math.h>
#include <ia32intrin.h>
#include "cl_vloadvstore_declaration.h"



// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( VloadVstore_Test );


void VloadVstore_Test::setUp()
{
}


void VloadVstore_Test::tearDown()
{
}


void VloadVstore_Test::__2i8_test()
{
	_2i8 data;
	char storePtr[4];	
	__declspec(align(16)) _2i8 loadPtr[8];
	__m128i tmp;
	data.a = 3;
	data.b = 4;
	__vstore2_i8(data, 1, (char*)&storePtr);
	tmp = __vload2_2i8(1, (char*)&storePtr);
	_mm_store_si128((__m128i *)loadPtr, tmp);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__vstore2_i8 __vload2_2i8 ", (char)storePtr[2], (char)(loadPtr[0].a));	
}

void VloadVstore_Test::__4i8_test()
{
	_4i8 data;
	data.a = 3;
	data.b = 4;
	data.c = 5;
	data.d = 6;
	char storePtr[8];
	__declspec(align(16)) _4i8 loadPtr[4];
	__m128i tmp;
	__declspec(align(16)) char res2[16];	

	__vstore4_i8(data, 1, (char*)&storePtr);
	tmp = __vload4_4i8(1, (char*)&storePtr);
	_mm_store_si128((__m128i *)loadPtr, tmp);
	// Store to temporary buffer		
	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__vstore4_i8 __vload4_4i8 ", (char)storePtr[4],(char) loadPtr[0].a);	
}

void VloadVstore_Test::__8i8_test()
{
	//In release mode due to ICC bug things not working properly
#ifdef DEBUG
	_8i8 data = _mm_set_pi32(3,4);
	_8i8 storePtr[8];
	__declspec(align(16)) char result[16];
	__declspec(align(16)) char in[8];

	__vstore8_i8(data, 7, (char*)&storePtr);
	__m128i tmp = __vload8_8i8(7, (char*)&storePtr);
	_mm_store_si128((__m128i *)result, tmp);

	// Store to temporary buffer
	memcpy(in, &storePtr[7], sizeof(_8i8));	
	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__vstore8_i8 __vload8_8i8 ", in[0], result[0]);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__vstore8_i8 __vload8_8i8 ", in[1], result[1]);
#endif
}
void VloadVstore_Test::__16i16_test()
{
	_16i16 data;
	data.a = _mm_set_epi32(3,4, 5, 6);
	data.b = _mm_set_epi32(11,12, 13, 14);
	_16i16 storePtr[10];
	_16i16 loadPtr;
	__declspec(align(16)) short res1[16], res2[16];	

	__vstore16_i16(data, 9, (short*)&storePtr);
	loadPtr = __vload16_16i16(9, (short*)&storePtr);

	// Store to temporary buffer
	_mm_store_si128((__m128i *)res1, storePtr[9].a);
	_mm_store_si128((__m128i *)res2, loadPtr.a);
	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__vstore16_i16 __vload16_16i16 ", res1[0], res2[0]);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__vstore16_i16 __vload16_16i16 ", res1[1], res2[1]);
}
void VloadVstore_Test::__16i32_test()
{
	_16i32 data;
	data.a = _mm_set_epi32(3,4, 5, 6);
	data.b = _mm_set_epi32(11,12, 13, 14);
	data.c = _mm_set_epi32(0,0, 0, 0);
	data.d = _mm_set_epi32(20,21, 22, 23);
	_16i32 storePtr[10];
	_16i32 loadPtr;
	__declspec(align(16)) int res1[16], res2[16];	

	__vstore16_i32(data, 9, (int*)&storePtr);
	loadPtr = __vload16_16i32(9, (int*)&storePtr);

	// Store to temporary buffer
	_mm_store_si128((__m128i *)res1, storePtr[9].a);
	_mm_store_si128((__m128i *)res2, loadPtr.a);
	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__vstore16_i32 __vload16_16i32 ", res1[0], res2[0]);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__vstore16_i32 __vload16_16i32 ", res1[1], res2[1]);
}

#if ICC_ABI_BUG
void VloadVstore_Test::__16i64_test()
{
	_16i64 data;
	data.a = _mm_set_epi32(0,1, 2, 3);
	data.b = _mm_set_epi32(4,5, 6, 7);
	data.c = _mm_set_epi32(8,9, 10, 11);
	data.d = _mm_set_epi32(12,13, 14, 15);
	data.e = _mm_set_epi32(16,17, 18, 19);
	data.f = _mm_set_epi32(20,21, 22, 23);
	data.g = _mm_set_epi32(24,25, 26, 27);
	data.h = _mm_set_epi32(28,29, 30, 31);

	_16i64 storePtr[4];
	_16i64 loadPtr;
	__declspec(align(16)) long res1[16], res2[16];	

	__vstore16_i64(data, 3, (long*)&storePtr);
	loadPtr = __vload16_16i64(3, (long*)&storePtr);

	// Store to temporary buffer
	_mm_store_si128((__m128i *)res1, storePtr[3].a);
	_mm_store_si128((__m128i *)res2, loadPtr.a);
	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__vstore16_i64 __vload16_16i64 ", res1[0], res2[0]);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__vstore16_i64 __vload16_16i64 ", res1[1], res2[1]);
}
#endif
void VloadVstore_Test::__2f_test()
{
#ifdef DEBUG
	float2 data = _mm_set_pi32(3.0,4.5);
	float2 storePtr[3];
	float2 loadPtr;
	__declspec(align(16)) float res1[4], res2[4];	

	__vstore2_f(data, 2, (float*)&storePtr);
	loadPtr = __vload2_f2(2, (float*)&storePtr);

	// Store to temporary buffer
	memcpy(res1, &storePtr[2], sizeof(float2));
	memcpy(res2, &loadPtr, sizeof(float2));
	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__vstore2_f __vload2_f2 ", res1[0], res2[0]);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("__vstore2_f __vload2_f2 ", res1[1], res2[1]);
#endif
}
