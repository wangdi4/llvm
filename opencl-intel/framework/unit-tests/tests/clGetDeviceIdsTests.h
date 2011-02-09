#pragma once

#include <framework_proxy.h>
#include <cppunit/extensions/HelperMacros.h>

using namespace Intel::OpenCL::Framework;

class clGetDeviceIdsTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( clGetDeviceIdsTest );
	///////////////////////////////////////////////////////////////////////
	CPPUNIT_TEST( CPU_invalid_args );
	CPPUNIT_TEST( CPU_size_only );
	CPPUNIT_TEST( CPU );
	CPPUNIT_TEST( GPU );
	CPPUNIT_TEST( ACCELERATOR );
	CPPUNIT_TEST( DEFAULT );
	CPPUNIT_TEST( ALL );
	CPPUNIT_TEST( CPU_GPU );
	CPPUNIT_TEST( CPU_ACCELERATOR );
	CPPUNIT_TEST( CPU_DEFAULT );
	CPPUNIT_TEST( GPU_ACCELERATOR );
	CPPUNIT_TEST( GPU_DEFAULT );
	CPPUNIT_TEST( ACCELERATOR_DEFAULT );
	CPPUNIT_TEST( CPU_GPU_ACCELERATOR );
	CPPUNIT_TEST( CPU_GPU_DEFAULT );
	CPPUNIT_TEST( CPU_ACCELERATOR_DEFAULT );
	CPPUNIT_TEST( GPU_ACCELERATOR_DEFAULT );
	///////////////////////////////////////////////////////////////////////
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void CPU_invalid_args();
	void CPU_size_only();
	void CPU();
	void GPU();
	void ACCELERATOR();
	void DEFAULT();
	void ALL();
	void CPU_GPU();
	void CPU_ACCELERATOR();
	void CPU_DEFAULT();
	void GPU_ACCELERATOR();
	void GPU_DEFAULT();
	void ACCELERATOR_DEFAULT();
	void CPU_GPU_ACCELERATOR();
	void CPU_GPU_DEFAULT();
	void CPU_ACCELERATOR_DEFAULT();
	void GPU_ACCELERATOR_DEFAULT();

private:

};

