#include "event_callback.h"
#include "ocl_event.h"

using namespace Intel::OpenCL::Framework;

EventCallback::EventCallback(eventCallbackFn callback, void* pUserData, const cl_int expectedExecState)
	: m_callback(callback), m_pUserData(pUserData), m_eventCallbackExecState(expectedExecState)
{
}


cl_err_code EventCallback::ObservedEventStateChanged(OclEvent* pEvent, cl_int returnCode /* = CL_SUCCESS */)
{
	assert (pEvent);
	cl_int retCode = returnCode;
	if (CL_COMPLETE != pEvent->GetEventExecState())
	{
		retCode = GetExpectedExecState();
	}
	m_callback(pEvent->GetHandle(), retCode, m_pUserData);
	delete this;
	return CL_SUCCESS;
}
