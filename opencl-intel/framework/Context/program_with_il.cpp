#include "Context.h"
#include "program_with_il.h"
#include "cl_logger.h"
#include "cl_sys_defines.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

ProgramWithIL::ProgramWithIL(SharedPtr<Context> pContext, const unsigned char* pIL, size_t length, cl_int *piRet)
                            : Program(pContext)
{
    cl_int err = CL_SUCCESS;
    cl_int ret = CL_SUCCESS;

    SharedPtr<FissionableDevice>* pDevices = pContext->GetDevices(&m_szNumAssociatedDevices);
    m_ppDevicePrograms  = new DeviceProgram*[m_szNumAssociatedDevices];
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

        err = m_ppDevicePrograms[i]->SetBinary(length, pIL, NULL);
        if (CL_SUCCESS != err)
        {
            if (CL_INVALID_BINARY == err)
            {
                ret = CL_INVALID_VALUE;
                // Must continue loading binaries for the rest of the devices
            }
            else
            {
                for (size_t j = 0; j < i; ++j)
                {
                    delete m_ppDevicePrograms[j];
                }
                delete[] m_ppDevicePrograms;
                m_ppDevicePrograms = NULL;

                ret = err;
                break;
            }
        }

        if (!m_ppDevicePrograms[i]->GetBinaryTypeInternal() != CL_PROGRAM_BINARY_TYPE_SPIRV)
        {
            m_ppDevicePrograms[i]->SetStateInternal(DEVICE_PROGRAM_SPIRV);
        }
        else
        {
            ret = CL_INVALID_VALUE;
        }
    }

    if (piRet)
    {
        *piRet = ret;
    }
}

ProgramWithIL::~ProgramWithIL()
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
