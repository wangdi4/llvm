#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include "Arrangement.h"
#include "built_in_func.h"


Arrangement::Arrangement()
{
	m_program = NULL;
	m_kernel = NULL;
	m_context = NULL;
	m_queue = NULL;
	m_param = NULL;
}

Arrangement::~Arrangement()
{
	if (m_program)
		clReleaseProgram(m_program);
	if (m_kernel)
		clReleaseKernel(m_kernel);
	if (m_context)
		clReleaseContext(m_context);
	if (m_queue)
		clReleaseCommandQueue(m_queue);
	if (m_param)
		clReleaseMemObject(m_param);
}

cl_int Arrangement::Init(const char* test, const char* func, const char* tail)
{
	cl_int			rc;
	// prepare OCL
	cl_device_id	dev;
	rc = clGetDeviceIDs(NULL, CL_DEVICE_TYPE_DEFAULT, 1, &dev, NULL);
	if (CL_SUCCESS != rc)
		return false;
	rc = clGetDeviceInfo(dev, CL_DEVICE_MAX_WORK_GROUP_SIZE,  sizeof(m_items), &m_items, NULL);
	if (CL_SUCCESS != rc)
		return false;
	m_context = clCreateContextFromType(0, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, &rc);
	if (NULL == m_context)
		return false;
	m_queue = clCreateCommandQueue(m_context, dev, NULL, &rc);
	if (CL_SUCCESS != rc)
		return false;


	// prepare kernel
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(sa);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = true;

	HANDLE			File = CreateFile
		(KERNELS, GENERIC_READ, FILE_SHARE_READ, &sa, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (NULL == File || (HANDLE)-1 == File)
	{
		printf("Cannot open kernels file\n");
		return false;
	}
	DWORD			FSize = GetFileSize(File, NULL);
	char*			Source = new char[FSize + 1];
	DWORD			ReadSize;
	ReadFile(File, Source, FSize, &ReadSize, NULL);
	Source[ReadSize] = 0;
	CloseHandle(File);
	if (FSize != ReadSize)
	{
		printf("Cannot read '%s'\n", KERNELS);
		return false;
	}

	// compile the kernel's file
	m_program = clCreateProgramWithSource(m_context, 1, (const char **)&Source, NULL, NULL);
	if (NULL == m_program)
	{
		printf("Cannot create program from '%s'\n", KERNELS);
		return false;
	}
	delete Source;
	rc = clBuildProgram(m_program, 1, &dev, NULL, NULL, NULL);
	if (CL_SUCCESS != rc)
	{
		size_t			size;
		rc = clGetProgramBuildInfo(m_program, dev, CL_PROGRAM_BUILD_LOG, 0, NULL, &size);
		char*			info;
		info = new char[size];
		rc = clGetProgramBuildInfo(m_program, dev, CL_PROGRAM_BUILD_LOG, size, info, &size);
		printf("Cannot create program from '%s'\n%s\n", KERNELS, info);
		delete info;
		return false;
	}

	// get the kernel itself
	cl_char		KernelName[128];
	sprintf_s((char *)KernelName, sizeof(KernelName), "%s_%s", func, tail);
	m_kernel = clCreateKernel(m_program, (const char *)KernelName, &rc);
	if (m_kernel)
		return PROCESSED_OK;
	else
		return PROCESSED_FAIL;
}

int Arrangement::InvokeKernel()
{
	cl_event	ev;
	cl_int		rc;
	size_t		global_work_items = m_items;
	size_t		local_work_items = 1;
	rc = clEnqueueNDRangeKernel(m_queue, m_kernel, 1, NULL, &global_work_items, &local_work_items, 0, NULL, &ev);
	if (CL_SUCCESS != rc)
		return PROCESSED_FAIL;
	rc = clWaitForEvents(1, &ev);
	clReleaseEvent(ev);
	if (CL_SUCCESS != rc)
		return PROCESSED_FAIL;
	return PROCESSED_OK;
}

int Arrangement::SetParameter(void* data, size_t size)
{
	cl_int			rc;
	m_param = clCreateBuffer(m_context, CL_MEM_COPY_HOST_PTR, size, data, &rc);
	if (CL_SUCCESS != rc)
		return PROCESSED_FAIL;
	rc = clEnqueueWriteBuffer(m_queue, m_param, CL_TRUE, 0, size, data, 0, NULL, NULL);
	if (CL_SUCCESS != rc)
		return PROCESSED_FAIL;
	rc = clSetKernelArg(m_kernel, 0, sizeof(cl_mem), &m_param);
	if (CL_SUCCESS != rc)			// use result of clSetKernelArg
		return PROCESSED_FAIL;
	return PROCESSED_OK;
}

int Arrangement::GetResults(void* data, size_t size)
{
	cl_int			rc;
	rc = clEnqueueReadBuffer(m_queue, m_param, CL_TRUE, 0, size, data, 0, NULL, NULL);
	if (CL_SUCCESS != rc)
		return PROCESSED_FAIL;
	return PROCESSED_OK;
}

size_t Arrangement::GetItems()
{
	return m_items;
}