#include "program_with_source.h"
#include "Device.h"
#include "Context.h"
#include "cl_sys_defines.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

ProgramWithSource::ProgramWithSource(Context* pContext, cl_uint uiNumStrings, const char** pSources, const size_t* pszLengths, cl_int* piRet, ocl_entry_points * pOclEntryPoints) : Program(pContext, pOclEntryPoints)
{
	cl_int err = CL_SUCCESS;

	FissionableDevice** pDevices = pContext->GetDevices(&m_szNumAssociatedDevices);
	m_ppDevicePrograms  = new DeviceProgram* [m_szNumAssociatedDevices];
	m_pSourceStrings = NULL;
	m_pszStringLengths = NULL;
	if (!m_ppDevicePrograms)
	{
		err = CL_OUT_OF_HOST_MEMORY;
	}
	else
	{
		if (!CopySourceStrings(uiNumStrings, pSources, pszLengths))
		{
			err = CL_OUT_OF_HOST_MEMORY;
			delete[] m_ppDevicePrograms;
			m_ppDevicePrograms = NULL;
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
                    m_ppDevicePrograms[i]->SetSource(m_uiNumStrings, m_pszStringLengths, const_cast<const char**>(m_pSourceStrings));
                }
			}
		}
	}

	if (piRet)
	{
		*piRet = err;
	}

}

ProgramWithSource::~ProgramWithSource()
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

	if (m_pSourceStrings)
	{
		for (size_t i = 0; i < m_uiNumStrings; ++i)
		{
			delete[] m_pSourceStrings[i];
		}
		delete[] m_pSourceStrings;
		delete[] m_pszStringLengths;
		m_pSourceStrings   = NULL;
		m_pszStringLengths = NULL;
	}
}

cl_err_code ProgramWithSource::GetInfo(cl_int param_name, size_t param_value_size, void * param_value, size_t * param_value_size_ret)
{
	LOG_DEBUG(TEXT("ProgramWithSource::GetInfo enter. param_name=%d, param_value_size=%d, param_value=%d, param_value_size_ret=%d"), 
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
		{
			szParamValueSize = 1;
			for (cl_uint ui = 0; ui < m_uiNumStrings; ++ui)
			{
				szParamValueSize += m_pszStringLengths[ui];
			}

			if (NULL != param_value)
			{
				if (param_value_size < szParamValueSize)
				{
					return CL_INVALID_VALUE;
				}
				char* pSourceCode = static_cast<char *>(param_value);
				MEMCPY_S(pSourceCode, m_pszStringLengths[0], m_pSourceStrings[0], m_pszStringLengths[0]);
				for (cl_uint ui = 1; ui < m_uiNumStrings; ++ui)
				{
					MEMCPY_S(pSourceCode + m_pszStringLengths[ui-1], m_pszStringLengths[ui], m_pSourceStrings[ui], m_pszStringLengths[ui]);
				}
				// Note: according to spec section 5.4.5, the length returned should include the null terminator
				pSourceCode[szParamValueSize - 1] = 0;
			}
			if (param_value_size_ret)
			{
				*param_value_size_ret = szParamValueSize;
			}
			return CL_SUCCESS;
		}

	default:
		//No need for specialized implementation
		return Program::GetInfo(param_name, param_value_size, param_value, param_value_size_ret);
	}
}

bool ProgramWithSource::CopySourceStrings(cl_uint uiNumStrings, const char** pSources, const size_t* pszLengths)
{
	m_uiNumStrings     = uiNumStrings;
	m_pszStringLengths = new size_t[m_uiNumStrings];
	if (NULL == m_pszStringLengths)
	{
		return false;
	}

	for(cl_uint ui = 0; ui < m_uiNumStrings; ++ui)
	{
		if ( (NULL == pszLengths) || (0 == pszLengths[ui]) )
		{
			m_pszStringLengths[ui] = strlen(pSources[ui]);
		}
		else
		{
			m_pszStringLengths[ui] = pszLengths[ui];
		}
	}

	m_pSourceStrings = new char* [m_uiNumStrings];

	for (cl_uint ui = 0; ui < m_uiNumStrings; ++ui)
	{
		m_pSourceStrings[ui] = new char[m_pszStringLengths[ui] + 1];
		if (NULL == m_pSourceStrings[ui])
		{
			// free all previous strings
			for (cl_uint uj = 0; uj < ui; ++uj)
			{
				delete[] m_pSourceStrings[uj];
			}
			delete[] m_pSourceStrings;
			m_pSourceStrings = NULL;
			delete[] m_pszStringLengths;
			m_pszStringLengths = NULL;
			return false;
		}
		if (0 != m_pszStringLengths[ui])
		{
			MEMCPY_S(m_pSourceStrings[ui], m_pszStringLengths[ui], pSources[ui], m_pszStringLengths[ui]);
		}
		//NULL terminate regardless of length
		m_pSourceStrings[ui][m_pszStringLengths[ui]] = 0; 
	}
	return true;
}

cl_err_code ProgramWithSource::NotifyDeviceFissioned(FissionableDevice* parent, size_t count, FissionableDevice** children)
{
    // Prevent further read access to device program map as we're about to relocate it
    OclAutoWriter CS(&m_deviceProgramLock);

    // Prepare new device program map
    cl_uint szNewNumAssociatedDevices = m_szNumAssociatedDevices + (cl_uint)count;
    DeviceProgram** ppNewDevicesPrograms = new DeviceProgram*[szNewNumAssociatedDevices];
    if (NULL == ppNewDevicesPrograms)
    {
        return CL_OUT_OF_HOST_MEMORY;
    }
    MEMCPY_S(ppNewDevicesPrograms, sizeof(DeviceProgram*) * m_szNumAssociatedDevices, m_ppDevicePrograms, sizeof(DeviceProgram*) * m_szNumAssociatedDevices);

    // Get the device program to be cloned
    DeviceProgram* pDeviceProgram = InternalGetDeviceProgram(parent->GetHandle());
    assert(pDeviceProgram);

    for (size_t i = 0; i < count; ++i)
    {
        DeviceProgram* pNewDeviceProgram = new DeviceProgram(*pDeviceProgram);
        if (NULL == pNewDeviceProgram)
        {
            for (size_t j = 0; j < i; ++j)
            {
                delete ppNewDevicesPrograms[m_szNumAssociatedDevices + j];
            }
            delete[] ppNewDevicesPrograms;
            return CL_OUT_OF_HOST_MEMORY;
        }
        ppNewDevicesPrograms[m_szNumAssociatedDevices + i] = pNewDeviceProgram;
    }

    m_szNumAssociatedDevices = szNewNumAssociatedDevices;
    delete[] m_ppDevicePrograms;
    m_ppDevicePrograms = ppNewDevicesPrograms;

    return CL_SUCCESS;
}
