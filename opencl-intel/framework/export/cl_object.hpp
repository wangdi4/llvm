///////////////////////////////////////////////////////////
//  OCLObject.hpp
//  Implementation of the Class OCLObject
//  Created on:      10-Dec-2008 4:45:30 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////

template <class HandleType>
OCLObject<HandleType>::OCLObject() : m_uiRefCount(0), m_uiPendency(0), m_iId(0), m_pLoggerClient(NULL)
{
	memset(&m_handle, 0, sizeof(HandleType));
	Retain();
}

template <class HandleType>
OCLObject<HandleType>::~OCLObject()
{}

template <class HandleType>
cl_err_code OCLObject<HandleType>::Release()
{
	if (m_uiRefCount <= 0)
	{
		return CL_INVALID_OPERATION;
	}
	--m_uiRefCount;
	return CL_SUCCESS;
}

template <class HandleType>
cl_err_code OCLObject<HandleType>::Retain()
{
	++m_uiRefCount;
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
	return --m_uiPendency;
}

template <class HandleType>
bool OCLObject<HandleType>::ReadyForDeletion()
{
	return (0 == m_uiPendency) && (0 == m_uiRefCount);
}
