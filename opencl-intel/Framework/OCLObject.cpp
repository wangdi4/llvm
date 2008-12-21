///////////////////////////////////////////////////////////
//  OCLObject.cpp
//  Implementation of the Class OCLObject
//  Created on:      10-Dec-2008 4:45:30 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////

#include "OCLObject.h"
using namespace Intel::OpenCL::Framework;

OCLObject::OCLObject()
{}

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

cl_int OCLObject::GetId()
{
	return m_iId;
}

cl_err_code OCLObject::SetId(cl_int obj_id)
{
	m_iId = obj_id;
	return CL_SUCCESS;
}

