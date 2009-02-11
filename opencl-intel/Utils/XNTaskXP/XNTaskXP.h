#pragma once

// LRB SDK XNTask library
#include <XN0Task_common.h>
#include <XN0Sys_common.h>

#include "cl_thread.h"

namespace Intel { namespace OpenCL { namespace Utils {

class XNTaskXPNotifyThread : public OclThread
{
public:
	XNTaskXPNotifyThread(XNSYNCOBJECT in_SyncObject, XN_SYNC_OBJECT_CALLBACK in_Callback, void* in_Arg) :
	  m_SyncObject(in_SyncObject), m_Callback(in_Callback), m_Arg(in_Arg) {};
protected:
	int	Run()
	{
		m_Callback(m_SyncObject, m_Arg);
		return 0;
	}

	XNSYNCOBJECT			m_SyncObject;
	XN_SYNC_OBJECT_CALLBACK m_Callback;
	void*					m_Arg;
};

}}}