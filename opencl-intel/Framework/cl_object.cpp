///////////////////////////////////////////////////////////
//  OCLObject.cpp
//  Implementation of the Class OCLObject
//  Created on:      10-Dec-2008 4:45:30 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////

#include "cl_object.h"
using namespace Intel::OpenCL::Framework;

OCLObject::OCLObject()
{
	m_uiRefCount = 0;
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

