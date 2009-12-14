#include "stdafx.h"
#include "BufferTest.h"
#include <cl_buffer.h>
#include <device.h>
using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( BufferTest );


void BufferTest::setUp()
{
	m_pDevice = NULL;
	m_pContext = NULL;

	m_pDevice = new Device();
	m_pDevice->InitDevice("cpu_device.dll");
	cl_err_code clErr = 0;
	m_pContext = new Context(0, 1, &m_pDevice, NULL, NULL, &clErr);
}


void BufferTest::tearDown()
{
	delete m_pContext;
	//delete m_pDevice;
}


void BufferTest::testConstructor()
{
	cl_err_code clErr = CL_SUCCESS;
	
	Buffer * pBuffer = NULL;
	
	pBuffer = new Buffer(m_pContext, 0, NULL, 0, &clErr);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("flags = 0", clErr, CL_SUCCESS);
	int iType = (int) pBuffer->GetFlags();
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check flags", iType, CL_MEM_READ_WRITE);
	delete pBuffer;

	pBuffer = new Buffer(m_pContext, CL_MEM_READ_ONLY, NULL, 0, &clErr);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("flags = CL_MEM_READ_ONLY", clErr, CL_SUCCESS );
	delete pBuffer;

	pBuffer = new Buffer(m_pContext, CL_MEM_WRITE_ONLY, NULL, 0, &clErr);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("flags = CL_MEM_WRITE_ONLY", clErr, CL_SUCCESS );
	delete pBuffer;
}

void BufferTest::testCreateDeviceResource()
{
	cl_err_code clErr = CL_SUCCESS;
	
	Buffer * pBuffer = new Buffer(m_pContext, CL_MEM_READ_ONLY, NULL, 0, &clErr);
	clErr = pBuffer->CreateDeviceResource((cl_device_id)m_pDevice->GetId());

	CPPUNIT_ASSERT_EQUAL( clErr, CL_SUCCESS );

	delete pBuffer;
}
