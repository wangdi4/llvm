#include "Context.h"
#include "program_with_binary.h"
#include "cl_logger.h"
#include "cl_sys_defines.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

ProgramWithBinary::ProgramWithBinary(Context* pContext, cl_uint uiNumDevices, FissionableDevice** pDevices, const size_t* pszLengths, const unsigned char** pBinaries, cl_int* piBinaryStatus, cl_int *piRet, ocl_entry_points * pOclEntryPoints)
: Program(pContext, pOclEntryPoints)
{
	cl_int err = CL_SUCCESS;
	cl_int ret = CL_SUCCESS;
	m_szNumAssociatedDevices = uiNumDevices;
    m_ppDevicePrograms  = new DeviceProgram* [m_szNumAssociatedDevices];
    if (!m_ppDevicePrograms)
	{
		err = CL_OUT_OF_HOST_MEMORY;
	}
	else
	{
		for (size_t i = 0; i < m_szNumAssociatedDevices; ++i)
		{
            m_ppDevicePrograms[i] = new DeviceProgram();
            if (NULL == m_ppDevicePrograms[i])
            {
                err = CL_OUT_OF_HOST_MEMORY;
                for (size_t j = 0; j < i; ++j)
                {
                    delete m_ppDevicePrograms[j];
                }
                delete[] m_ppDevicePrograms;
                m_ppDevicePrograms = NULL;
                break;
            }
            else
            {
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
			}
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

cl_err_code ProgramWithBinary::GetInfo(cl_int param_name, size_t param_value_size, void *param_value, size_t *param_value_size_ret)
{
	LOG_DEBUG(TEXT("ProgramWithBinary::GetInfo enter. param_name=%d, param_value_size=%d, param_value=%d, param_value_size_ret=%d"), 
		param_name, param_value_size, param_value, param_value_size_ret);

	cl_err_code clErrRet = CL_SUCCESS;
	if (NULL == param_value && NULL == param_value_size_ret)
	{
		return CL_INVALID_VALUE;
	}
	size_t szParamValueSize = 0;

	size_t uiParam = 0;
	switch ( (cl_program_info)param_name )
	{
	case CL_PROGRAM_BINARIES:
		{
            OclAutoReader CS(&m_deviceProgramLock);

			szParamValueSize = sizeof(char *) * m_szNumAssociatedDevices;
			char ** pParamValue = static_cast<char **>(param_value);
			// get  data
			if (NULL != pParamValue)
			{
				if (param_value_size < szParamValueSize)
				{
					return CL_INVALID_VALUE;
				}
				for (size_t i = 0; i < m_szNumAssociatedDevices; ++i)
				{
					clErrRet = m_ppDevicePrograms[i]->GetBinary(0, NULL, &uiParam);
					if (CL_FAILED(clErrRet))
					{
						return clErrRet;
					}
					clErrRet = m_ppDevicePrograms[i]->GetBinary(uiParam, pParamValue[i], &uiParam);
					if (CL_FAILED(clErrRet))
					{
						return clErrRet;
					}
				}
			}
			// get  size
			if (NULL != param_value_size_ret)
			{
				*param_value_size_ret = szParamValueSize;
			}
			return CL_SUCCESS;
		}

	case CL_PROGRAM_SOURCE:
		//Todo: check with Ofer what we need to return here
		return CL_INVALID_VALUE;

	default:
		//No need for specialized implementation
		return Program::GetInfo(param_name, param_value_size, param_value, param_value_size_ret);
	}
}
