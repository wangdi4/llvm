#include "Context.h"
#include "program_for_link.h"
#include "cl_logger.h"
#include "cl_sys_defines.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

ProgramForLink::ProgramForLink(Context* pContext, cl_uint uiNumDevices, FissionableDevice** pDevices, cl_int *piRet, ocl_entry_points * pOclEntryPoints)
: Program(pContext, pOclEntryPoints)
{
	m_szNumAssociatedDevices = uiNumDevices;
    m_ppDevicePrograms  = new DeviceProgram* [m_szNumAssociatedDevices];
    if (!m_ppDevicePrograms)
	{
        if (piRet)
	    {
		    *piRet = CL_OUT_OF_HOST_MEMORY;
	    }
        return;
	}


	for (size_t i = 0; i < m_szNumAssociatedDevices; ++i)
	{
        m_ppDevicePrograms[i] = new DeviceProgram();
        if (NULL == m_ppDevicePrograms[i])
        {
            for (size_t j = 0; j < i; ++j)
            {
                delete m_ppDevicePrograms[j];
            }
            delete[] m_ppDevicePrograms;
            m_ppDevicePrograms = NULL;

            if (piRet)
	        {
		        *piRet = CL_OUT_OF_HOST_MEMORY;
	        }
            return;
        }

        m_ppDevicePrograms[i]->SetDevice(pDevices[i]);
        m_ppDevicePrograms[i]->SetHandle(GetHandle());
        m_ppDevicePrograms[i]->SetContext(pContext->GetHandle());
	}

	if (piRet)
	{
		*piRet = CL_SUCCESS;
	}
}

ProgramForLink::~ProgramForLink()
{
    if ((m_szNumAssociatedDevices > 0) && (NULL != m_ppDevicePrograms))
    {
        for (size_t i = 0; i < m_szNumAssociatedDevices; ++i)
        {
            delete m_ppDevicePrograms[i];
        }
        delete[] m_ppDevicePrograms;
        m_ppDevicePrograms = NULL;
	}
}
