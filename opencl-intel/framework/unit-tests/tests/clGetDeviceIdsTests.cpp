#include "stdafx.h"
#include "clGetDeviceIdsTests.h"
using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

extern char clFRAMEWORK_CFG_PATH[];

#define clGetDeviceIDs FrameworkProxy::Instance()->GetPlatformModule()->GetDeviceIDs

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( clGetDeviceIdsTest );


void clGetDeviceIdsTest::setUp()
{
	ConfigFile *pCfgFile = new ConfigFile();
	pCfgFile->Add<string>(CL_CONFIG_LOG_FILE, "C:\\cl.log");
	pCfgFile->Add<string>(CL_CONFIG_DEFAULT_DEVICE, "cpu_device.dll");
	pCfgFile->Add<string>(CL_CONFIG_DEVICES, "cpu_device.dll");
	pCfgFile->Add<string>(CL_CONFIG_FE_COMPILERS, "clang_compiler.dll");
	pCfgFile->Add<string>(CL_CONFIG_DEFAULT_FE_COMPILER, "clang_compiler.dll");
	pCfgFile->Add<bool>(CL_CONFIG_USE_LOGGER, true);
	ConfigFile::WriteFile("C:\\cl.cfg", *pCfgFile);

	strcpy_s(clFRAMEWORK_CFG_PATH, MAX_PATH-1, "C:\\cl.cfg");
	delete pCfgFile;
}


void clGetDeviceIdsTest::tearDown()
{
}


void clGetDeviceIdsTest::CPU_invalid_args()
{
	cl_int iRes = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU, 0, NULL, NULL);
	CPPUNIT_ASSERT_EQUAL(iRes, CL_INVALID_VALUE );
}

void clGetDeviceIdsTest::CPU_size_only()
{
	size_t size_ret;
	cl_int iRes = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU, 0, NULL, &size_ret);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check return value", iRes, CL_SUCCESS );
	CPPUNIT_ASSERT_EQUAL_MESSAGE("Check number of devices", (int)size_ret, 1 );
}

void clGetDeviceIdsTest::CPU()
{
	size_t size_ret;
	cl_device_id devices[10] = {0};
	cl_int iRes = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU, 10, devices, &size_ret);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check return value", iRes, CL_SUCCESS );
	CPPUNIT_ASSERT_MESSAGE("check device id", devices[0] != 0 );
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check returned size value", (int)size_ret, 1 );
}
void clGetDeviceIdsTest::GPU()
{
	size_t size_ret;
	cl_device_id devices[10] = {0};
	cl_int iRes = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU, 10, devices, &size_ret);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check return value", iRes, CL_DEVICE_NOT_FOUND );
}
void clGetDeviceIdsTest::ACCELERATOR()
{
	size_t size_ret;
	cl_device_id devices[10] = {0};
	cl_int iRes = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_ACCELERATOR, 10, devices, &size_ret);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check return value", iRes, CL_DEVICE_NOT_FOUND );
}
void clGetDeviceIdsTest::DEFAULT()
{
	size_t size_ret;
	cl_device_id devices[10] = {0};
	cl_int iRes = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_DEFAULT, 10, devices, &size_ret);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check return value", iRes, CL_SUCCESS );
	CPPUNIT_ASSERT_MESSAGE("check device id", devices[0] != 0 );
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check returned size value", (int)size_ret, 1 );
}
void clGetDeviceIdsTest::ALL()
{
	size_t size_ret;
	cl_device_id devices[10] = {0};
	cl_int iRes = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_ALL, 10, devices, &size_ret);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check return value", iRes, CL_SUCCESS );
	CPPUNIT_ASSERT_MESSAGE("check device id", devices[0] != 0 );
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check returned size value", (int)size_ret, 1 );
}
void clGetDeviceIdsTest::CPU_GPU()
{
	size_t size_ret;
	cl_device_id devices[10] = {0};
	cl_int iRes = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU, 10, devices, &size_ret);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check return value", iRes, CL_SUCCESS );
	CPPUNIT_ASSERT_MESSAGE("check device id", devices[0] != 0 );
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check returned size value", (int)size_ret, 1 );
}
void clGetDeviceIdsTest::CPU_ACCELERATOR()
{
	size_t size_ret;
	cl_device_id devices[10] = {0};
	cl_int iRes = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_ACCELERATOR, 10, devices, &size_ret);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check return value", iRes, CL_SUCCESS );
	CPPUNIT_ASSERT_MESSAGE("check device id", devices[0] != 0 );
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check returned size value", (int)size_ret, 1 );
}
void clGetDeviceIdsTest::CPU_DEFAULT()
{
	size_t size_ret;
	cl_device_id devices[10] = {0};
	cl_int iRes = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_DEFAULT, 10, devices, &size_ret);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check return value", iRes, CL_SUCCESS );
	CPPUNIT_ASSERT_MESSAGE("check device id", devices[0] != 0 );
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check returned size value", (int)size_ret, 1 );
}
void clGetDeviceIdsTest::GPU_ACCELERATOR()
{
	size_t size_ret;
	cl_device_id devices[10] = {0};
	cl_int iRes = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR, 10, devices, &size_ret);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check return value", iRes, CL_DEVICE_NOT_FOUND );
}
void clGetDeviceIdsTest::GPU_DEFAULT()
{
	size_t size_ret;
	cl_device_id devices[10] = {0};
	cl_int iRes = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_DEFAULT, 10, devices, &size_ret);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check return value", iRes, CL_SUCCESS );
	CPPUNIT_ASSERT_MESSAGE("check device id", devices[0] != 0 );
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check returned size value", (int)size_ret, 1 );
}
void clGetDeviceIdsTest::ACCELERATOR_DEFAULT()
{
	size_t size_ret;
	cl_device_id devices[10] = {0};
	cl_int iRes = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_ACCELERATOR | CL_DEVICE_TYPE_DEFAULT, 10, devices, &size_ret);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check return value", iRes, CL_SUCCESS );
	CPPUNIT_ASSERT_MESSAGE("check device id", devices[0] != 0 );
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check returned size value", (int)size_ret, 1 );
}
void clGetDeviceIdsTest::CPU_GPU_ACCELERATOR()
{
	size_t size_ret;
	cl_device_id devices[10] = {0};
	cl_int iRes = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR, 10, devices, &size_ret);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check return value", iRes, CL_SUCCESS );
	CPPUNIT_ASSERT_MESSAGE("check device id", devices[0] != 0 );
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check returned size value", (int)size_ret, 1 );
}
void clGetDeviceIdsTest::CPU_GPU_DEFAULT()
{
	size_t size_ret;
	cl_device_id devices[10] = {0};
	cl_int iRes = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_DEFAULT, 10, devices, &size_ret);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check return value", iRes, CL_SUCCESS );
	CPPUNIT_ASSERT_MESSAGE("check device id", devices[0] != 0 );
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check returned size value", (int)size_ret, 1 );
}
void clGetDeviceIdsTest::CPU_ACCELERATOR_DEFAULT()
{
	size_t size_ret;
	cl_device_id devices[10] = {0};
	cl_int iRes = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU | CL_DEVICE_TYPE_ACCELERATOR | CL_DEVICE_TYPE_DEFAULT, 10, devices, &size_ret);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check return value", iRes, CL_SUCCESS );
	CPPUNIT_ASSERT_MESSAGE("check device id", devices[0] != 0 );
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check returned size value", (int)size_ret, 1 );
}
void clGetDeviceIdsTest::GPU_ACCELERATOR_DEFAULT()
{
	size_t size_ret;
	cl_device_id devices[10] = {0};
	cl_int iRes = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_GPU | CL_DEVICE_TYPE_ACCELERATOR | CL_DEVICE_TYPE_DEFAULT, 10, devices, &size_ret);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check return value", iRes, CL_SUCCESS );
	CPPUNIT_ASSERT_MESSAGE("check device id", devices[0] != 0 );
	CPPUNIT_ASSERT_EQUAL_MESSAGE("check returned size value", (int)size_ret, 1 );
}