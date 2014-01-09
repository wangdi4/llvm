#include "Context.h"
#include "program_with_binary.h"
#include "cl_logger.h"
#include "cl_sys_defines.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

ProgramWithBinary::ProgramWithBinary(SharedPtr<Context> pContext, cl_uint uiNumDevices, SharedPtr<FissionableDevice>* pDevices, const size_t* pszLengths,
									 const unsigned char** pBinaries, cl_int* piBinaryStatus, cl_int *piRet)
	: Program(pContext)
{
	cl_int err = CL_SUCCESS;
	cl_int ret = CL_SUCCESS;
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
        cl_int* piBinStatus = (NULL == piBinaryStatus) ? NULL : piBinaryStatus + i;
        err = m_ppDevicePrograms[i]->SetBinary(pszLengths[i], pBinaries[i], piBinStatus);
        if (CL_SUCCESS != err)
        {
            if (CL_INVALID_BINARY == err)
            {
                ret = CL_INVALID_BINARY;
                // Must continue loading binaries for the rest of the devices
            }
            else
            {
                ret = err;
                break;
            }
        }

        // if device is custom then set binary to custom
        if (pDevices[i]->GetRootDevice()->GetDeviceType() == CL_DEVICE_TYPE_CUSTOM)
        {
            m_ppDevicePrograms[i]->SetStateInternal(DEVICE_PROGRAM_CUSTOM_BINARY);
        }
        else
        {
            m_ppDevicePrograms[i]->SetStateInternal(DEVICE_PROGRAM_LOADED_IR);
        }
	}

	if (piRet)
	{
		*piRet = ret;
	}
}

ProgramWithBinary::~ProgramWithBinary()
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
