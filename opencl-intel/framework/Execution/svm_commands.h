// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

#pragma once

#include <vector>
#include "enqueue_commands.h"

namespace Intel { namespace OpenCL { namespace Framework {

/**
 * This class is responsible for freeing the shared virtual memory allocated using clSVMAlloc or a shared system memory pointer
 */
class SVMFreeCommand : public RuntimeCommand
{
public:

	/**
	 * Type of the user's callback function to be called to free the SVM pointers
	 */
	typedef void (CL_CALLBACK *pfnFreeFunc)(cl_command_queue queue, cl_uint uiNumSvmPtrs, void* pSvmPtrs[], void* pUserData);

	/**
	 * Constructor
	 * @param uiNumSvmPtrs			number of elements in pSvmPtrs
	 * @param pSvmPtrs				array of SVM pointers to be freed
	 * @param freeFunc				callback function to be called to free the SVM pointers, it may be NULL
	 * @param pUserData				optional user data to be passed to freeFunc
	 * @param cmdQueue				a pointer to the IOclCommandQueueBase on which this command is enqueued
	 * @param bIsDependentOnEvents	whether this command is dependent on some events
	 */
	SVMFreeCommand(cl_uint uiNumSvmPtrs, void* pSvmPtrs[], pfnFreeFunc freeFunc, void* pUserData, const SharedPtr<IOclCommandQueueBase>& cmdQueue, bool bIsDependentOnEvents) :
	   RuntimeCommand(cmdQueue, bIsDependentOnEvents), m_svmPtrs(pSvmPtrs, pSvmPtrs + uiNumSvmPtrs), m_freeFunc(freeFunc), m_pUserData(pUserData) { }

	// overriden methods:

	cl_err_code Execute();

	cl_command_type GetCommandType() const { return CL_COMMAND_SVM_FREE; }

	const char* GetCommandName() const { return "CL_COMMAND_SVM_FREE"; }

private:

	std::vector<void* > m_svmPtrs;
	const pfnFreeFunc m_freeFunc;
	void* const m_pUserData;

};

/**
 * This class is responsible for performing the command enqueued by clEnqueueSVMMemcpy in case both destination and source pointers are system pointers
 */
class RuntimeSVMMemcpyCommand : public RuntimeCommand
{
public:

	/**
	 * Constructor
	 * @param pDstPtr				a pointer to the destination
	 * @param pSrcPtr				a pointer to the source
	 * @param size					the size of the memory region to copy
	 * @param cmdQueue				a pointer to the IOclCommandQueueBase on which this command is enqueued
	 * @param bIsDependentOnEvents	whether this command is dependent on some events
	 */
	RuntimeSVMMemcpyCommand(void* pDstPtr, const void* pSrcPtr, size_t size, const SharedPtr<IOclCommandQueueBase>& cmdQueue, bool bIsDependentOnEvents) :
	  RuntimeCommand(cmdQueue, bIsDependentOnEvents), m_pDstPtr(pDstPtr), m_pSrcPtr(pSrcPtr), m_size(size) { }

	// overriden methods:

	cl_err_code Execute();

	cl_command_type GetCommandType() const { return CL_COMMAND_SVM_MEMCPY; }

	const char* GetCommandName() const { return "CL_COMMAND_SVM_MEMCPY"; }

private:

	void* const m_pDstPtr;
	const void* const m_pSrcPtr;
	const size_t m_size;

};

/**
 * This class is responsible for performing the command enqueued by clEnqueueSVMMemFill in case the SVM pointer is a system pointer 
 */
class RuntimeSVMMemFillCommand : public RuntimeCommand
{
public:

	/**
	 * Constructor
	 * @param pSvmPtr		a pointer to a memory region that will be filled with the pattern
	 * @param pPattern		a pointer to the data pattern
	 * @param szPatternSize size in bytes of the pattern
	 * @param size			size in bytes of the region being filled
	 * @param cmdQueue				a pointer to the IOclCommandQueueBase on which this command is enqueued
	 * @param bIsDependentOnEvents	whether this command is dependent on some events
	 */
	RuntimeSVMMemFillCommand(void* pSvmPtr, const void* pPattern, size_t szPatternSize, size_t size, const SharedPtr<IOclCommandQueueBase>& cmdQueue, bool bIsDependentOnEvents) :
	  RuntimeCommand(cmdQueue, bIsDependentOnEvents), m_pSvmPtr(pSvmPtr), m_pPattern(pPattern), m_szPatternSize(szPatternSize), m_size(size) { }

	// overriden methods:

	cl_err_code Execute();

	cl_command_type GetCommandType() const { return CL_COMMAND_SVM_MEMFILL; }

	const char* GetCommandName() const { return "CL_COMMAND_SVM_MEMFILL"; }

private:

	void* const m_pSvmPtr;
	const void* const m_pPattern;
	const size_t m_szPatternSize;
	const size_t m_size;
};

}}}
