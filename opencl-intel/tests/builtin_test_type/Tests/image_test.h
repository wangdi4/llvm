#pragma once

#include <cppunit/extensions/HelperMacros.h>

class ImageTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( ImageTest );
	
	CPPUNIT_TEST( __read_2d_uii_1u8_test );
	CPPUNIT_TEST( __read_2d_ii_2u16_test );
	CPPUNIT_TEST( __read_2d_fi_3s32_test );
	CPPUNIT_TEST( __read_2d_fi_4f_test );
	CPPUNIT_TEST( __read_3d_fi_4f_test );
	CPPUNIT_TEST( __read_2d_ff_nearest_norm_4f_test );
	CPPUNIT_TEST( __read_2d_ff_linear_unorm_4f_test );
	CPPUNIT_TEST( __read_2d_if_linear_unnorm_2s16_test );
	CPPUNIT_TEST( __read_3d_ff_nearest_norm_4f_test );
	CPPUNIT_TEST( __read_3d_ff_linear_unorm_4f_test );
	CPPUNIT_TEST( __read_3d_if_linear_unnorm_2s16_test );

	CPPUNIT_TEST( __write_2d_uii_1u8_test );
	CPPUNIT_TEST( __write_2d_ii_2u16_test );
	CPPUNIT_TEST( __write_2d_fi_3s32_test );
	CPPUNIT_TEST( __write_2d_fi_4f_test );

	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

private:
	// Test single plane (L) unsigned 8
	void __read_2d_uii_1u8_test();
	void __read_2d_ii_2u16_test();
	void __read_2d_fi_3s32_test();
	void __read_2d_fi_4f_test();
	void __read_3d_fi_4f_test();
	void __read_2d_ff_nearest_norm_4f_test();
	void __read_2d_ff_linear_unorm_4f_test();
	void __read_2d_if_linear_unnorm_2s16_test();
	void __read_3d_ff_nearest_norm_4f_test();
	void __read_3d_ff_linear_unorm_4f_test();
	void __read_3d_if_linear_unnorm_2s16_test();

	void __write_2d_uii_1u8_test();
	void __write_2d_ii_2u16_test();
	void __write_2d_fi_3s32_test();
	void __write_2d_fi_4f_test();

};
