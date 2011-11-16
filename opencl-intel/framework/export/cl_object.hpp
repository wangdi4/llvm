///////////////////////////////////////////////////////////
//  OCLObject.hpp
//  Implementation of the Class OCLObject
//  Created on:      10-Dec-2008 4:45:30 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////

template <class HandleType>
OCLObject<HandleType>::OCLObject() :  m_iId(0), m_uiRefCount(1), m_uiPendency(1) , m_pLoggerClient(NULL)
{
	memset(&m_handle, 0, sizeof(HandleType));
}

template <class HandleType>
OCLObject<HandleType>::~OCLObject()
{}

template <class HandleType>
long OCLObject<HandleType>::Release()
{
	long newVal = --m_uiRefCount;
	if (newVal < 0)
	{
		++m_uiRefCount;
		return -1;
	}
	else if (0 == newVal)
	{
		//This may have the side effect of deleting the object
		RemovePendency();
	}
	return newVal;
}

template <class HandleType>
cl_err_code OCLObject<HandleType>::Retain()
{
	m_uiRefCount++;
	return CL_SUCCESS;
}

template <class HandleType>
long OCLObject<HandleType>::AddPendency()
{
	return ++m_uiPendency;
}

template <class HandleType>
long OCLObject<HandleType>::RemovePendency()
{
	long newVal = --m_uiPendency;
	if (0 == newVal)
	{
		Cleanup();
		delete this;
	}
	return newVal;
}
