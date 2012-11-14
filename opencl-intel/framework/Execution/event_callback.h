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

		// IEventObserver
        cl_err_code ObservedEventStateChanged(const SharedPtr<OclEvent>& pEvent, cl_int returnCode);
		cl_int GetExpectedExecState() const { return m_eventCallbackExecState; }

	private:
		eventCallbackFn  m_callback;
		void*            m_pUserData;
		const cl_int     m_eventCallbackExecState;
	};
}}}    // Intel::OpenCL::Framework
