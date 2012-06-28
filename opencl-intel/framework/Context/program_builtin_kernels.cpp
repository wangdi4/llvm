#include "Context.h"
#include "program_builtin_kernels.h"
#include "cl_logger.h"
#include "cl_sys_defines.h"
#include "Device.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

ProgramWithBuiltInKernels::ProgramWithBuiltInKernels(Context* pContext, cl_uint uiNumDevices, FissionableDevice** pDevices, const char* szKernelNames, cl_int *piRet)
	: Program(pContext), m_szKernelNames(szKernelNames)
{
	cl_int retError = CL_SUCCESS;
	m_szNumAssociatedDevices = uiNumDevices;
    m_ppDevicePrograms  = new DeviceProgram* [m_szNumAssociatedDevices];
    if (!m_ppDevicePrograms)
	{
        if (NULL != piRet)
	    {
		    *piRet = CL_OUT_OF_HOST_MEMORY;
	    }
        return;
	}

	size_t i;
	bool bDeviceProgramCreated = false;
	for (i = 0; (i<m_szNumAssociatedDevices) ; ++i)
	{
        m_ppDevicePrograms[i] = new DeviceProgram();
        if (NULL == m_ppDevicePrograms[i])
        {
			retError = CL_OUT_OF_HOST_MEMORY;
			break;
        }

		cl_dev_program pDevProg;
		cl_dev_err_code err = pDevices[i]->GetDeviceAgent()->clDevCreateBuiltInKernelProgram(szKernelNames, &pDevProg);
		if ( CL_DEV_FAILED(err) && (CL_DEV_INVALID_KERNEL_NAME != err) )
		{
			retError = CL_OUT_OF_RESOURCES;
			break;
		}

        m_ppDevicePrograms[i]->SetDevice(pDevices[i]);
        m_ppDevicePrograms[i]->SetHandle(GetHandle());
        m_ppDevicePrograms[i]->SetContext(pContext->GetHandle());

        m_ppDevicePrograms[i]->SetStateInternal(DEVICE_PROGRAM_BUILTIN_KERNELS);
		m_ppDevicePrograms[i]->SetDeviceHandleInternal(pDevProg);
		bDeviceProgramCreated = true;
	}

	if ( !bDeviceProgramCreated )
	{
		// No device program is created, probably wrong names are provided
		retError = CL_INVALID_VALUE;
	}

	if ( CL_FAILED(retError) )
	{
		for (size_t j = 0; j < i; ++j)
		{
			cl_dev_program pDevProg = m_ppDevicePrograms[j]->GetDeviceProgramHandle();
			if ( NULL != pDevProg )
			{
				pDevices[i]->GetDeviceAgent()->clDevReleaseProgram(pDevProg);
			}
			delete m_ppDevicePrograms[j];
		}

		delete[] m_ppDevicePrograms;
		m_ppDevicePrograms = NULL;
	}

    if (piRet)
	{
		*piRet = retError;
	}
}

ProgramWithBuiltInKernels::~ProgramWithBuiltInKernels()
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
