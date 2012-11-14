///////////////////////////////////////////////////////////
//  OCLObject.hpp
//  Implementation of the Class OCLObject
//  Created on:      10-Dec-2008 4:45:30 PM
//  Original author: ulevy
///////////////////////////////////////////////////////////

template <class HandleType, class ParentHandleType>
OCLObject<HandleType,ParentHandleType>::OCLObject(ParentHandleType* parent, const std::string& typeName) : 
	OCLObjectBase(typeName), m_iId(0), m_uiRefCount(1), m_pParentHandle(parent),
	m_bTerminate(false), m_pLoggerClient(NULL), m_uiPendency(0)
{
	m_handle.object = this;
	if ( NULL != parent )
	{
		MEMCPY_S(&m_handle, sizeof(ocl_entry_points), &(parent->dispatch), sizeof(ocl_entry_points));
	}
#ifdef _DEBUG
	else
	{
		memset(&m_handle, 0, sizeof(ocl_entry_points));
	}
#endif
}

template <class HandleType, class ParentHandleType>
long OCLObject<HandleType, ParentHandleType>::Release()
{
	long newVal = --m_uiRefCount;
	if (newVal < 0)
	{
		++m_uiRefCount;
		return -1;
	}
	else if (0 == newVal)
	{
        NotifyInvisible();
	}
	return newVal;
}

template <class HandleType, class ParentHandleType>
cl_err_code OCLObject<HandleType,ParentHandleType>::Retain()
{
	m_uiRefCount++;
	return CL_SUCCESS;
}

