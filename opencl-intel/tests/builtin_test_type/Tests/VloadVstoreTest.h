#ifndef _VloadVstore_TEST_H
#define _VloadVstore_TEST_H

#include <cppunit/extensions/HelperMacros.h>

class VloadVstore_Test : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( VloadVstore_Test );
	CPPUNIT_TEST( __2i8_test );
	CPPUNIT_TEST( __4i8_test );
	CPPUNIT_TEST( __8i8_test );
	CPPUNIT_TEST( __16i16_test );
	CPPUNIT_TEST( __16i32_test );
	//CPPUNIT_TEST( __16i64_test );
	CPPUNIT_TEST( __2f_test );	
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void __2i8_test();	
	void __4i8_test();	
	void __8i8_test();	
	void __16i16_test();	
	void __16i32_test();	
	//void __16i64_test();	
	void __2f_test();	

private:
};

#endif  // _VloadVstore_TEST_H
