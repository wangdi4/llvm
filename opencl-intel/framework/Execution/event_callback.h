#pragma once

#include <cl_types.h>
#include "event_done_observer.h"

namespace Intel { namespace OpenCL { namespace Framework {

	typedef void (CL_CALLBACK *eventCallbackFn)(cl_event, cl_int, void *);

	class OclEvent;

	class EventCallback : public IEventDoneObserver
	{
	public:
		EventCallback(eventCallbackFn callback, void* pUserData) : m_callback(callback), m_pUserData(pUserData) {}
		virtual ~EventCallback() {}

		virtual cl_err_code NotifyEventDone(OclEvent* pEvent, cl_int returnCode = CL_SUCCESS);

	private:
		eventCallbackFn  m_callback;
		void*            m_pUserData;
	};
}}}    // Intel::OpenCL::Framework
