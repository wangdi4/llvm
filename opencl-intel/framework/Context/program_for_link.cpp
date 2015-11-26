#include "Context.h"
#include "program_for_link.h"
#include "cl_logger.h"
#include "cl_sys_defines.h"
#include "kernel.h"
#include "sampler.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

ProgramForLink::ProgramForLink(SharedPtr<Context>            pContext,
                               cl_uint                       uiNumDevices,
                               SharedPtr<FissionableDevice>* pDevices,
                               cl_int*                       piRet)
: Program(pContext)
{
	cl_int ret = CL_SUCCESS;
	m_szNumAssociatedDevices = uiNumDevices;

    try
    {
        m_ppDevicePrograms.resize(m_szNumAssociatedDevices);

        for (size_t i = 0; i < m_szNumAssociatedDevices; ++i)
        {
            unique_ptr<DeviceProgram>& pDevProgram = m_ppDevicePrograms[i];
            pDevProgram.reset(new DeviceProgram());

            pDevProgram->SetDevice(pDevices[i]);
            pDevProgram->SetHandle(GetHandle());
            pDevProgram->SetContext(pContext->GetHandle());
        }
    }
    catch(std::bad_alloc& e)
    {
        ret = CL_OUT_OF_HOST_MEMORY;
    }

    if (piRet)
    {
        *piRet = ret;
    }
}

ProgramForLink::~ProgramForLink()
{}
