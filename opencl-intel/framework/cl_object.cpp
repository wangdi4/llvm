///////////////////////////////////////////////////////////
//  OCLObject.cpp
//  Implementation of the Class OCLObject
//  Created on:      10-Dec-2008 4:45:30 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////

#include "cl_object.h"
#include <windows.h>

using namespace Intel::OpenCL::Framework;

OCLObject::OCLObject()
{
	m_uiRefCount = 0;
	m_uiPendency = 0;
	m_pHandle = 0;
	m_iId = 0;
	m_pLoggerClient = NULL;
	Retain();
}

OCLObject::~OCLObject()
{}

cl_err_code OCLObject::Release()
{
	if (m_uiRefCount <= 0)
	{
		return CL_INVALID_OPERATION;
	}
	--m_uiRefCount;
	return CL_SUCCESS;
}

cl_err_code OCLObject::Retain()
{
	++m_uiRefCount;
	return CL_SUCCESS;
}

void OCLObject::AddPendency()
{
	::InterlockedIncrement(&m_uiPendency);
}

void OCLObject::RemovePendency()
{
	::InterlockedDecrement(&m_uiPendency);
}

bool OCLObject::ReadyForDeletion()
{
	return ::InterlockedCompareExchange(&m_uiPendency, 0, 0) == 0;
}
