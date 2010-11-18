#ifndef _BUFFER_TEST_H
#define _BUFFER_TEST_H

#include <context.h>
#include <cppunit/extensions/HelperMacros.h>

class BufferTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE( BufferTest );
	CPPUNIT_TEST( testConstructor );
	CPPUNIT_TEST( testCreateDeviceResource );
	CPPUNIT_TEST_SUITE_END();

public:
	void setUp();
	void tearDown();

	void testConstructor();
	void testCreateDeviceResource();

private:
	Intel::OpenCL::Framework::Context * m_pContext;
	Intel::OpenCL::Framework::Device * m_pDevice;
};

#endif  // _BUFFER_TEST_H
