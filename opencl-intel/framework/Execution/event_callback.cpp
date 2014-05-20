#include "event_callback.h"
#include "ocl_event.h"
#include "cl_user_logger.h"
#include "ocl_config.h"

using namespace Intel::OpenCL::Framework;

EventCallback::EventCallback(eventCallbackFn callback, void* pUserData, const cl_int expectedExecState)
	: m_callback(callback), m_pUserData(pUserData), m_eventCallbackExecState(expectedExecState)
{
}

cl_err_code EventCallback::ObservedEventStateChanged(const SharedPtr<OclEvent>& pEvent, cl_int returnCode)
{
    assert (pEvent);
    cl_int retCode = returnCode;
    if (CL_COMPLETE != pEvent->GetEventExecState())
    {
        retCode = GetExpectedExecState();
    }
    if (GetUserLoggerInstance().IsApiLoggingEnabled())
    {
        std::stringstream stream;
        stream << "EventCallback(" << pEvent->GetHandle() << ", " << m_pUserData << ")" << std::endl;
        GetUserLoggerInstance().PrintString(stream.str());
    }
    m_callback(pEvent->GetHandle(), retCode, m_pUserData);
    return CL_SUCCESS;
}
