#ifndef _INTEGER_TEST_H
#define _INTEGER_TEST_H

#include <cppunit/extensions/HelperMacros.h>

class Integer_Test : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( Integer_Test );
	CPPUNIT_TEST( __abs_test );
	CPPUNIT_TEST( __abs_diff_test );
	CPPUNIT_TEST( __add_sat_test );
	CPPUNIT_TEST( __hadd_test );
	CPPUNIT_TEST( __rhadd_test );
	CPPUNIT_TEST( __max_test );
	CPPUNIT_TEST( __min_test );
	CPPUNIT_TEST( __mul_hi_test );
	CPPUNIT_TEST( __clz_test );
	CPPUNIT_TEST( __sub_sat_test );
	CPPUNIT_TEST( __rotate_test );	
	CPPUNIT_TEST( __mad_sat_test );
	CPPUNIT_TEST( __mad_hi_test );	
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void __abs_test();
	void __abs_diff_test();
	void __add_sat_test();	
	void __hadd_test();
	void __rhadd_test();
	void __max_test();
	void __min_test();
	void __mul_hi_test();
	void __clz_test();	
	void __sub_sat_test();	
	void __rotate_test();
	void __upsample_test();	
	void __mad_sat_test();
	void __mad_hi_test();	

private:
};

#endif  // _INTEGER_TEST_H
