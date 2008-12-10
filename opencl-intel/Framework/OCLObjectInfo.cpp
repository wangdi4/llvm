///////////////////////////////////////////////////////////
//  OCLObjectInfo.cpp
//  Implementation of the Class OCLObjectInfo
//  Created on:      10-Dec-2008 4:45:30 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////

#include "OCLObjectInfo.h"
using namespace Intel::OpenCL::Framework;

OCLObjectInfo::OCLObjectInfo()
{

}

OCLObjectInfo::~OCLObjectInfo()
{
	m_mapInfoParams.clear();
}

cl_err_code OCLObjectInfo::GetParam(cl_int param_name, OCLObjectInfoParam ** ppParam)
{
	if (NULL == ppParam)
	{
		return CL_INVALID_VALUE;
	}
	
	map<cl_int, OCLObjectInfoParam*>::iterator it = m_mapInfoParams.find(param_name);
	if (it == m_mapInfoParams.end())
	{
		return CL_INVALID_VALUE;
	}

	*ppParam = it->second;
	return  CL_SUCCESS;
}

cl_err_code OCLObjectInfo::SetParam(cl_int param_name, OCLObjectInfoParam * pParam)
{
	m_mapInfoParams[param_name] = pParam;
	return  CL_SUCCESS;
}


OCLObjectInfoParam::OCLObjectInfoParam(cl_int param_name, size_t param_value_size, void * param_value)
{
	m_iParamName = param_name;
	m_szParamSize = param_value_size;
	m_pParamValue = param_value;
}

OCLObjectInfoParam::OCLObjectInfoParam()
{

}

OCLObjectInfoParam::~OCLObjectInfoParam()
{
}


const cl_int OCLObjectInfoParam::GetName()
{
	return  m_iParamName;
}


const size_t OCLObjectInfoParam::GetSize()
{
	return  m_szParamSize;
}


const void* OCLObjectInfoParam::GetValue()
{
	return  m_pParamValue;
}