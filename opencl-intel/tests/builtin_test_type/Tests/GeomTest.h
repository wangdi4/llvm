#ifndef _Geom_TEST_H
#define _Geom_TEST_H

#include <cppunit/extensions/HelperMacros.h>

class Geom_Test : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( Geom_Test );
	CPPUNIT_TEST( __cross_test );
	CPPUNIT_TEST( __length_test );
	CPPUNIT_TEST( __distance_test );
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void __cross_test();	
	void __length_test();
	void __distance_test();
private:
};

#endif  // _Geom_TEST_H
