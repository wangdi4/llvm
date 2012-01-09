#ifndef _CONVERSIONS_TEST_H
#define _CONVERSIONS_TEST_H

#include <cppunit/extensions/HelperMacros.h>

class Conversions_Test : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( Conversions_Test );
	CPPUNIT_TEST( __uchar2_convert_test );
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void __uchar2_convert_test();	
private:
};

#endif  // _CONVERSIONS_TEST_H
