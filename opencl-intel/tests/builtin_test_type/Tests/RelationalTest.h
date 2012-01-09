#ifndef _INTEGER_TEST_H
#define _INTEGER_TEST_H

#include <cppunit/extensions/HelperMacros.h>

class Relational_Test : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( Relational_Test );
	CPPUNIT_TEST( __isequal_test );
	CPPUNIT_TEST( __isnotequal_test );
	CPPUNIT_TEST( __isgreater_test );
	CPPUNIT_TEST( __isgreaterequal_test );
	CPPUNIT_TEST( __isless_test );
	CPPUNIT_TEST( __isfinite_test );
	CPPUNIT_TEST( __isinf_test );
	CPPUNIT_TEST( __isnormal_test );
	CPPUNIT_TEST( __signbit_test );
	CPPUNIT_TEST( __any_test );
	CPPUNIT_TEST( __all_test );
	CPPUNIT_TEST( __bitselect_test );
	CPPUNIT_TEST( __select_test );
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void __isequal_test();
	void __isnotequal_test();
	void __isgreater_test();
	void __isgreaterequal_test();
	void __isless_test();
	void __islessequal_test();
	void __isfinite_test();
	void __isinf_test();
	void __isnormal_test();
	void __signbit_test();
	void __any_test ();
	void __all_test ();
	void __bitselect_test ();
	void __select_test ();
private:
};

#endif  // _INTEGER_TEST_H
