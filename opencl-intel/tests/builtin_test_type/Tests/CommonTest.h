#ifndef _COMMON_TEST_H
#define _COMMON_TEST_H

#include <cppunit/extensions/HelperMacros.h>

class Common_Test : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( Common_Test );
	CPPUNIT_TEST( __clamp_test );
	CPPUNIT_TEST( __degrees_test );
	CPPUNIT_TEST( __max_test );
	CPPUNIT_TEST( __min_test );
	CPPUNIT_TEST( __step_test );
	CPPUNIT_TEST( __smoothstep_test );
	CPPUNIT_TEST( __sign_test );
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void __clamp_test();	
	void __degrees_test();
	void __max_test();
	void __min_test();
	void __step_test();
	void __smoothstep_test();
	void __sign_test();
private:
};

#endif  // _COMMON_TEST_H
