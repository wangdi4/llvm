// Copyright (c) 2006-2013 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#include <iostream>
#include <cstring>
#include <cstdlib>
#include "CL/cl_platform.h"
#include "CL/cl.h"
#include "test_utils.h"

#define BUF_SIZE	1024

using namespace std;

extern cl_device_type gDeviceType;

static const char* sProg =
	"struct Node\n"
	"{\n"
	"	int m_num;\n"
	"	global struct Node* m_pNext;\n"
	"};\n"
	"\n"
	"kernel void SumIntsInLinkedList(global struct Node* pHead, global int* pResult)\n"
	"{\n"
	"	*pResult = 0;\n"
	"	while (pHead != (global void*)0)\n"
	"	{\n"
	"		*pResult += pHead->m_num;\n"
	"		pHead = pHead->m_pNext;\n"
	"	}\n"
	"}\n"
	"\n"
	"kernel void SumInts(global int* iArr, int szArrSize, global int* pSum)\n"
	"{\n"
	"	*pSum = 0;\n"
	"	for (size_t i = 0; i < szArrSize; i++)\n"
	"	{\n"
	"		*pSum += iArr[i];\n"
	"	}\n"
	"}\n";

struct Node
{
	cl_int m_num;
	Node* m_pNext;
};

static void TestSetKernelExecInfo(cl_context context, cl_device_id device, cl_command_queue queue, cl_program prog)
{
	const cl_int szListSize = 1000;
	std::vector<Node*> nodes;
	for (cl_int i = 0; i < szListSize; i++)
	{
		Node* const pNode = (Node*)clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER, sizeof(Node), 0);
		CheckException(L"clSVMAlloc", pNode != NULL, true);
		pNode->m_num = i;
		if (i == szListSize - 1)
		{
			pNode->m_pNext = NULL;
		}		
		if (i > 0)
		{
			nodes[i - 1]->m_pNext = pNode;
		}
		nodes.push_back(pNode);
	}
	cl_int* const piResult = (cl_int*)clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER, sizeof(cl_int), 0);
	CheckException(L"clSVMAlloc", piResult != NULL, true);
	
	cl_int iRet;
	cl_kernel kernel = clCreateKernel(prog, "SumIntsInLinkedList", &iRet);
	CheckException(L"clCreateKernel", CL_SUCCESS, iRet);
	iRet = clSetKernelArgSVMPointer(kernel, 0, nodes[0]);
	CheckException(L"clSetKernelArgSVMPointer", CL_SUCCESS, iRet);
	iRet = clSetKernelArgSVMPointer(kernel, 1, piResult);
	CheckException(L"clSetKernelArgSVMPointer", CL_SUCCESS, iRet);
	const cl_bool bFineGrainSystem = CL_FALSE;
	iRet = clSetKernelExecInfo(kernel, CL_KERNEL_EXEC_INFO_SVM_FINE_GRAIN_SYSTEM, sizeof(bFineGrainSystem), &bFineGrainSystem);
	CheckException(L"clSetKernelExecInfo", CL_SUCCESS, iRet);
	iRet = clSetKernelExecInfo(kernel, CL_KERNEL_EXEC_INFO_SVM_PTRS, (nodes.size() - 1) * sizeof(void*), &nodes[1]);
	CheckException(L"clSetKernelExecInfo", CL_SUCCESS, iRet);

	const size_t szGlobalWorkOffset = 0, szWorkSize = 1;
	iRet = clEnqueueNDRangeKernel(queue, kernel, 1, &szGlobalWorkOffset, &szWorkSize, NULL, 0, NULL, NULL);
	CheckException(L"clEnqueueNDRangeKernel", CL_SUCCESS, iRet);
	iRet = clFinish(queue);
	CheckException(L"clFinish",  CL_SUCCESS, iRet);

	const cl_int iExpected = szListSize * (2 * nodes[0]->m_num + szListSize - 1) / 2;	// sum of arithmetic progression	
	if (iExpected != *piResult)
	{
		throw exception();
	}

	iRet = clReleaseKernel(kernel);
	CheckException(L"clReleaseKernel", CL_SUCCESS, iRet);	
	for (std::vector<Node*>::iterator iter = nodes.begin(); iter != nodes.end(); iter++)
	{
		clSVMFree(context, *iter);
	}
	clSVMFree(context, piResult);
}

static void TestSetKernelArgSVMPointer(cl_context context, cl_device_id device, cl_command_queue queue, cl_program prog, bool bSysPtrs)
{
	cl_int* const piArr = bSysPtrs ? (cl_int*)malloc(BUF_SIZE) : (cl_int*)clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER, BUF_SIZE, 0);
	CheckException(bSysPtrs ? L"malloc" : L"clSVMAlloc", piArr != NULL, true);
    cl_int* const piResult = bSysPtrs ? (cl_int*)malloc(BUF_SIZE) : (cl_int*)clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER, sizeof(cl_int), 0);
	CheckException(bSysPtrs ? L"malloc" : L"clSVMAlloc", piArr != NULL, true);

	// initialize piArr
	for (size_t i = 0; i < BUF_SIZE / sizeof(cl_int); i++)
	{
		piArr[i] = i;
	}
		
	const cl_int iStartIndex = BUF_SIZE / sizeof(cl_int) / 2, iNumElems = BUF_SIZE / sizeof(cl_int) / 2;
	const cl_int iExpected = iNumElems * (2 * piArr[iStartIndex] + iNumElems - 1) / 2;	// sum of arithmetic progression	
	cl_int iRet;
	cl_kernel kernel = clCreateKernel(prog, "SumInts", &iRet);
	CheckException(L"clCreateKernel", CL_SUCCESS, iRet);
	iRet = clSetKernelArgSVMPointer(kernel, 0, &piArr[iStartIndex]);
	CheckException(L"clSetKernelArgSVMPointer", CL_SUCCESS, iRet);
	iRet = clSetKernelArg(kernel, 1, sizeof(cl_int), &iNumElems);
	CheckException(L"clSetKernelArg", CL_SUCCESS, iRet);
	iRet = clSetKernelArgSVMPointer(kernel, 2, piResult);
	CheckException(L"clSetKernelArgSVMPointer", CL_SUCCESS, iRet);

	const size_t szGlobalWorkOffset = 0, szWorkSize = 1;
	iRet = clEnqueueNDRangeKernel(queue, kernel, 1, &szGlobalWorkOffset, &szWorkSize, NULL, 0, NULL, NULL);
	CheckException(L"clEnqueueNDRangeKernel", CL_SUCCESS, iRet);
	iRet = clFinish(queue);
	CheckException(L"clFinish",  CL_SUCCESS, iRet);

	if (iExpected != *piResult)
	{
		throw exception();
	}    

	iRet = clReleaseKernel(kernel);
	CheckException(L"clReleaseKernel", CL_SUCCESS, iRet);
    if (bSysPtrs)
    {
        free(piArr);
        free(piResult);
    }
    else
    {
	    clSVMFree(context, piArr);
	    clSVMFree(context, piResult);
    }    
}

typedef void (CL_CALLBACK *pfnFreeFunc)(cl_command_queue queue, cl_uint uiNumSvmPtrs, void* pSvmPtrs[], void* pUserData);

static void CL_CALLBACK SVMFreeCallback(cl_command_queue queue, cl_uint uiNumSvmPtrs, void* pSvmPtrs[], void* pUserData)
{
	cout << "Executing SVMFreeCallback" << endl;
	for (cl_uint i = 0; i < uiNumSvmPtrs; i++)
	{
		clSVMFree(*(cl_context*)pUserData, pSvmPtrs[i]);
	}
}

enum InitMethod
{
	MEMCPY,
	MEM_FILL,
	MAP_UNMAP
};

static const char pattern[] = "hello world SVM";

static void InitBuffers(InitMethod method, cl_command_queue queue, void* pSrcBuf, void* pSvmPtr)
{
	cl_int iRet = clEnqueueSVMMemFill(queue, pSrcBuf, pattern, sizeof(pattern), BUF_SIZE, 0, NULL, NULL);
	CheckException(L"clEnqueueSVMMemFill", CL_SUCCESS, iRet);

	switch (method)
	{
	case MEMCPY:
		iRet = clEnqueueSVMMemcpy(queue, CL_FALSE, pSvmPtr, pSrcBuf, BUF_SIZE, 0, NULL, NULL);
		CheckException(L"clEnqueueSVMMemcpy", CL_SUCCESS, iRet);
		break;
	case MEM_FILL:
		iRet = clEnqueueSVMMemFill(queue, pSvmPtr, pattern, sizeof(pattern), BUF_SIZE, 0, NULL, NULL);
		CheckException(L"clEnqueueSVMMemFill", CL_SUCCESS, iRet);
		break;
	case MAP_UNMAP:
		iRet = clEnqueueSVMMap(queue, CL_TRUE, CL_MAP_WRITE_INVALIDATE_REGION, pSvmPtr, BUF_SIZE, 0, NULL, NULL);
		CheckException(L"clEnqueueSVMMap", CL_SUCCESS, iRet);
		MEMCPY_S(pSvmPtr, BUF_SIZE, pSrcBuf, BUF_SIZE);
		iRet = clEnqueueSVMUnmap(queue, pSvmPtr, 0, NULL, NULL);
		CheckException(L"clEnqueueSVMUnmap", CL_SUCCESS, iRet);
		break;
	}
}

static void TestEnqueueSVMCommands(cl_context context, cl_command_queue queue, InitMethod initMethod, bool bEnqueueFree = false, pfnFreeFunc freeFunc = NULL, void* pUserData = NULL)
{
	void *const pSvmPtr1 = clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER, BUF_SIZE, 0),
		 *const pSvmPtr2 = clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER, BUF_SIZE, 0),
		 *const pSvmPtr3 = clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER, BUF_SIZE, 0),
		 *const pSvmPtr4 = clSVMAlloc(context, CL_MEM_SVM_FINE_GRAIN_BUFFER, BUF_SIZE, 0);
	CheckException(L"clSVMAlloc", pSvmPtr1 != NULL, true);
	CheckException(L"clSVMAlloc", pSvmPtr2 != NULL, true);
	CheckException(L"clSVMAlloc", pSvmPtr3 != NULL, true);
	CheckException(L"clSVMAlloc", pSvmPtr4 != NULL, true);

	cl_int iRet;
	// create a buffer from the whole of pSvmPtr3
	cl_mem buf3 = clCreateBuffer(context, CL_MEM_USE_HOST_PTR, BUF_SIZE, pSvmPtr3, &iRet);
	CheckException(L"clCreateBuffer", CL_SUCCESS, iRet);
	// create 2 buffers from the 2 halves of pSvmPtr4
	cl_mem buf4a = clCreateBuffer(context, CL_MEM_USE_HOST_PTR, BUF_SIZE / 2, pSvmPtr4, &iRet);
	CheckException(L"clCreateBuffer", CL_SUCCESS, iRet);
	cl_mem buf4b = clCreateBuffer(context, CL_MEM_USE_HOST_PTR, BUF_SIZE / 2, (char*)pSvmPtr4 + BUF_SIZE / 2, &iRet);
	CheckException(L"clCreateBuffer", CL_SUCCESS, iRet);

	// create 2 sub-buffers from the 2 halves of buf4a
	cl_buffer_region regionA, regionB;
	regionA.origin = 0;
	regionB.origin = BUF_SIZE / 4;
	regionA.size = regionB.size = BUF_SIZE / 4;
	cl_mem subBuf4aa = clCreateSubBuffer(buf4a, 0, CL_BUFFER_CREATE_TYPE_REGION, &regionA, &iRet);
	CheckException(L"clCreateSubBuffer", CL_SUCCESS, iRet);
	cl_mem subBuf4ab = clCreateSubBuffer(buf4a, 0, CL_BUFFER_CREATE_TYPE_REGION, &regionB, &iRet);
	CheckException(L"clCreateSubBuffer", CL_SUCCESS, iRet);

	char* srcBuf 
#ifdef WIN32
		= (char*)_aligned_malloc(BUF_SIZE, sizeof(pattern));	// srcBuf is initialized using clEnqueueSVMMemFill
#elif defined (__ANDROID__)
        = (char*)memalign(sizeof(pattern), BUF_SIZE);
#else
		;
		int res = posix_memalign((void**)&srcBuf, sizeof(pattern), BUF_SIZE);
		assert(0 == res);
#endif
	char middleBuf[BUF_SIZE], dstBuf[BUF_SIZE];
	// pass the data from srcBuf to dstBuf through the SVM buffers, cl_mem buffers and sub-buffers
	InitBuffers(initMethod, queue, srcBuf, pSvmPtr1);
	cl_event memcpyEvent;
	iRet = clEnqueueSVMMemcpy(queue, CL_FALSE, pSvmPtr2, pSvmPtr1, BUF_SIZE, 0, NULL, &memcpyEvent);	// check a path in the code with event dependency
	CheckException(L"clEnqueueSVMMemcpy", CL_SUCCESS, iRet);
	iRet = clEnqueueSVMMemcpy(queue, CL_FALSE, middleBuf, pSvmPtr2, BUF_SIZE, 1, &memcpyEvent, NULL);
	CheckException(L"clEnqueueSVMMemcpy", CL_SUCCESS, iRet);
	iRet = clEnqueueWriteBuffer(queue, buf3, CL_FALSE, 0, BUF_SIZE, middleBuf, 0, NULL, NULL);
	CheckException(L"clEnqueueWriteBuffer", CL_SUCCESS, iRet);
	iRet = clEnqueueCopyBuffer(queue, buf3, buf4a, 0, 0, BUF_SIZE / 2, 0, NULL, NULL);
	CheckException(L"clEnqueueCopyBuffer", CL_SUCCESS, iRet); 
	iRet = clEnqueueCopyBuffer(queue, buf3, buf4b, BUF_SIZE / 2, 0, BUF_SIZE / 2, 0, NULL, NULL);
	CheckException(L"clEnqueueCopyBuffer", CL_SUCCESS, iRet);
	iRet = clEnqueueReadBuffer(queue, subBuf4aa, CL_FALSE, 0, BUF_SIZE / 4, dstBuf, 0, NULL, NULL);
	CheckException(L"clEnqueueReadBuffer", CL_SUCCESS, iRet);
	iRet = clEnqueueReadBuffer(queue, subBuf4ab, CL_FALSE, 0, BUF_SIZE / 4, &dstBuf[BUF_SIZE / 4], 0, NULL, NULL);
	CheckException(L"clEnqueueReadBuffer", CL_SUCCESS, iRet);
	iRet = clEnqueueReadBuffer(queue, buf4b, CL_TRUE, 0, BUF_SIZE / 2, &dstBuf[BUF_SIZE / 2], 0, NULL, NULL);
	CheckException(L"clEnqueueReadBuffer", CL_SUCCESS, iRet);

	bool bDiff = false;
	for (size_t i = 0; i < BUF_SIZE; i++)
	{
		if (srcBuf[i] != dstBuf[i])
		{
			cout << "srcBuf differs from dstBuf at index " << i << endl;
			bDiff = true;
			break;
		}
	}

	iRet = clReleaseMemObject(subBuf4aa);
	CheckException(L"clReleaseMemObject", CL_SUCCESS, iRet);
	iRet = clReleaseMemObject(subBuf4ab);
	CheckException(L"clReleaseMemObject", CL_SUCCESS, iRet);
	iRet = clReleaseMemObject(buf3);
	CheckException(L"clReleaseMemObject", CL_SUCCESS, iRet);
	iRet = clReleaseMemObject(buf4a);
	CheckException(L"clReleaseMemObject", CL_SUCCESS, iRet);
	iRet = clReleaseMemObject(buf4b);
	CheckException(L"clReleaseMemObject", CL_SUCCESS, iRet);

	if (!bEnqueueFree)
	{
		clSVMFree(context, NULL);	// check that we handle NULL pointer correctly and don't crash
		clSVMFree(context, pSvmPtr1);
		clSVMFree(context, pSvmPtr2);
		clSVMFree(context, pSvmPtr3);
		clSVMFree(context, pSvmPtr4);
	}
	else
	{
		void* pSvmPtrs[] = { pSvmPtr1, pSvmPtr2, pSvmPtr3, pSvmPtr4 };
		iRet = clEnqueueSVMFree(queue, sizeof(pSvmPtrs) / sizeof(pSvmPtrs[0]), pSvmPtrs, freeFunc, pUserData, 0, NULL, NULL);
		CheckException(L"clEnqueueSVMFree", CL_SUCCESS, iRet);
		iRet = clFinish(queue);
		CheckException(L"clFinish", CL_SUCCESS, iRet); 
	}
#ifdef _WIN32
	_aligned_free(srcBuf);
#else
	free(srcBuf);
#endif
	if (bDiff)
	{
		throw exception();
	}
}

bool clSvmTest()
{
	cl_int iRet = CL_SUCCESS;
    cl_platform_id platform = 0;
    bool bResult = true;
    cl_device_id device = NULL;
    cl_context context = NULL;
    cl_command_queue queue = NULL;
	cl_program prog = NULL;
    cl_device_svm_capabilities svm_caps;
    size_t temp;

    std::cout << "=============================================================" << std::endl;
    std::cout << "clTestEnqueueSVMCommands" << std::endl;
    std::cout << "=============================================================" << std::endl;

    try
    {
        iRet = clGetPlatformIDs(1, &platform, NULL);
        CheckException(L"clGetPlatformIDs", CL_SUCCESS, iRet);        
        iRet = clGetDeviceIDs(platform, gDeviceType, 1, &device, NULL);
        CheckException(L"clGetDeviceIDs", CL_SUCCESS, iRet);
        iRet = clGetDeviceInfo(device, CL_DEVICE_SVM_CAPABILITIES, sizeof(svm_caps), &svm_caps, &temp );
        CheckException(L"clGetDeviceInfo(CL_DEVICE_SVM_CAPABILITIES)", CL_SUCCESS, iRet);

        const cl_context_properties prop[3] = { CL_CONTEXT_PLATFORM, (cl_context_properties)platform, 0 };    
        context = clCreateContextFromType(prop, gDeviceType, NULL, NULL, &iRet);
		CheckException(L"clCreateContextFromType", CL_SUCCESS, iRet);

#if _WIN32
#pragma warning( suppress : 4996 )
    queue = clCreateCommandQueue(context, device, 0, &iRet);
#else
    queue = clCreateCommandQueue(context, device, 0, &iRet);
#endif		
		CheckException(L"clCreateCommandQueue", CL_SUCCESS, iRet);

		TestEnqueueSVMCommands(context, queue, MEMCPY);
		TestEnqueueSVMCommands(context, queue, MEM_FILL);
		TestEnqueueSVMCommands(context, queue, MAP_UNMAP);
		TestEnqueueSVMCommands(context, queue, MEMCPY, true);
		TestEnqueueSVMCommands(context, queue, MEMCPY, true, SVMFreeCallback, &context);

		const size_t szLengths = { strlen(sProg) };
		cl_program prog = clCreateProgramWithSource(context, 1, &sProg, &szLengths, &iRet);
		CheckException(L"clCreateProgramWithSource", CL_SUCCESS, iRet);
		iRet = clBuildProgram(prog, 1, &device, NULL, NULL, NULL);
		CheckException(L"clBuildProgram", CL_SUCCESS, iRet);
		TestSetKernelArgSVMPointer(context, device, queue, prog, false);
        TestSetKernelArgSVMPointer(context, device, queue, prog, true);
		TestSetKernelExecInfo(context, device, queue, prog);        

        cl_device_svm_capabilities svmCaps;
        iRet = clGetDeviceInfo(device, CL_DEVICE_SVM_CAPABILITIES, sizeof(svmCaps), &svmCaps, NULL);
        CheckException(L"clGetDeviceInfo", CL_SUCCESS, iRet);
        CheckException(L"clGetDeviceInfo", (cl_device_svm_capabilities)(CL_DEVICE_SVM_COARSE_GRAIN_BUFFER | CL_DEVICE_SVM_FINE_GRAIN_BUFFER | CL_DEVICE_SVM_FINE_GRAIN_SYSTEM | CL_DEVICE_SVM_ATOMICS), svmCaps);
	}
	catch (const std::exception&)
    {
        bResult = false;
    }
    if (context)
    {
        clReleaseContext(context);
    }
    if (queue)
    {
        clReleaseCommandQueue(queue);
    }
	if (prog)
	{
		clReleaseProgram(prog);
	}
    return bResult;
}
