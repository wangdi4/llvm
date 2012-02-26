#pragma once

#include <cl_types.h>
#include "event_observer.h"

namespace Intel { namespace OpenCL { namespace Framework {

	typedef void (CL_CALLBACK *eventCallbackFn)(cl_event, cl_int, void *);

	class OclEvent;

	class EventCallback : public IEventObserver
	{
	public:
		EventCallback(eventCallbackFn callback, void* pUserData, const cl_int expectedExecState);

		virtual ~EventCallback() {}

		virtual cl_err_code ObservedEventStateChanged(OclEvent* pEvent, cl_int returnCode = CL_SUCCESS);

		cl_int GetExpectedExecState() const { return m_eventCallbackExecState; }

	private:
		eventCallbackFn  m_callback;
		void*            m_pUserData;
		const cl_int     m_eventCallbackExecState;
	};
}}}    // Intel::OpenCL::Framework
