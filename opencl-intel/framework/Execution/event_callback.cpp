#include "event_callback.h"
#include "ocl_event.h"

using namespace Intel::OpenCL::Framework;

cl_err_code EventCallback::NotifyEventDone(OclEvent* pEvent, cl_int returnCode /* = CL_SUCCESS */)
{
	assert (pEvent);
	m_callback(pEvent->GetHandle(), returnCode, m_pUserData);
	delete this;
	return CL_SUCCESS;
}
