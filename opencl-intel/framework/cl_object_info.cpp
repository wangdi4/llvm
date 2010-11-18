///////////////////////////////////////////////////////////
//  OCLObjectInfo.cpp
//  Implementation of the Class OCLObjectInfo
//  Created on:      10-Dec-2008 4:45:30 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////

#include "cl_object_info.h"
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

cl_err_code OCLObjectInfo::SetString(cl_int param_name, const size_t length, const char str[])
{
	// check input parameters
	if (NULL == str || length < (-1) )
	{
		return CL_INVALID_VALUE;
	}
	// create new object info param
	OCLObjectInfoParam * pParam = new OCLObjectInfoParam(param_name, length, (void*)str);
	//set param in the information object
	return SetParam(param_name, pParam);
}

/*****
 *
 ****/
OCLObjectInfoParam::OCLObjectInfoParam(cl_int param_name, size_t param_value_size, void * param_value)
{
	// assign parameter's name
	m_iParamName = param_name;
	// assign parameter's value size
	m_szParamSize = param_value_size;
	// allocate new chars array for data
	m_pParamValue = new char[m_szParamSize];
	if (NULL == m_pParamValue)
	{
		// memory allocation failed
		m_iParamName = -1;
		m_szParamSize = 0;
	}
	// copy data from user to parameter
	errno_t err = memcpy_s(m_pParamValue, m_szParamSize, param_value, param_value_size);
	if (0 != err)
	{
		// copy failed
		m_iParamName = -1;
		m_szParamSize = 0;
		m_pParamValue = NULL;
	}
}

OCLObjectInfoParam::OCLObjectInfoParam()
{

}

OCLObjectInfoParam::~OCLObjectInfoParam()
{
	if (m_szParamSize > 0)
	{
		delete[] m_pParamValue;
		m_pParamValue = NULL;
		m_szParamSize = 0;
	}
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