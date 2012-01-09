#ifndef _SVML_TEST_H
#define _SVML_TEST_H

#include <cppunit/extensions/HelperMacros.h>

class SVML_Test : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( SVML_Test );
	CPPUNIT_TEST( __exp10_test );
	//CPPUNIT_TEST( __atanpi_test );
	//CPPUNIT_TEST( __acospi_test );
//	CPPUNIT_TEST( __sincos_test );
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void __exp10_test();
	//void __atanpi_test();
	//void __acospi_test();
//	void __sincos_test();

private:
};

#endif  // _SVML_TEST_H
