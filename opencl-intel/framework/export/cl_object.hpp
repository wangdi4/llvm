// INTEL CONFIDENTIAL
//
// Copyright 2008-2018 Intel Corporation.
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

template <class HandleType, class ParentHandleType>
OCLObject<HandleType,ParentHandleType>::OCLObject(ParentHandleType* parent, const std::string& typeName) : 
    m_iId(0), m_uiRefCount(1), m_pParentHandle(parent), 
    m_bTerminate(false), m_bPreserveHandleOnDelete(false),
    m_typename(typeName), m_pLoggerClient(nullptr)
{
	m_handle.object = this;
	if ( nullptr != parent )
	{
		m_handle.dispatch    = parent->dispatch;
		m_handle.crtDispatch = parent->crtDispatch;

		// By default use logger of the parent object
		SET_LOGGER_CLIENT( ((OCLObject<ParentHandleType>*)parent->object)->GetLoggerClient());
	}
#ifdef _DEBUG
    else
    {
        memset(&m_handle, 0, sizeof(ocl_entry_points));
    }
#endif
}

template <class HandleType, class ParentHandleType>
void OCLObject<HandleType, ParentHandleType>::operator delete (void * mem)
{
    OCLObject* me = (OCLObject*)mem;
    
    // HACK!!! do not delete CLObject if m_bPreserveHandleOnDelete was requested
    if ((nullptr != mem) && (!me->m_bPreserveHandleOnDelete))
    {
        Intel::OpenCL::Utils::ReferenceCountedObject::operator delete( me );
    }
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

