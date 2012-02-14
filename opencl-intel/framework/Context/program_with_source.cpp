#include "program_with_source.h"
#include "Device.h"
#include "Context.h"
#include "cl_sys_defines.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

ProgramWithSource::ProgramWithSource(Context* pContext, cl_uint uiNumStrings, const char** pSources, const size_t* pszLengths, cl_int* piRet, ocl_entry_points * pOclEntryPoints) : Program(pContext, pOclEntryPoints), m_szSourceString(NULL)
{
    if ((0 == uiNumStrings) || (NULL == pSources))
    {
        if (piRet)
	    {
		    *piRet = CL_INVALID_VALUE;
	    }
        return;
    }

    for ( unsigned int i = 0; i < uiNumStrings; ++i)
    {
        if (NULL == pSources[i])
        {
            if (piRet)
	        {
		        *piRet = CL_INVALID_VALUE;
	        }
            return;
        }
    }

	FissionableDevice** pDevices = pContext->GetDevices(&m_szNumAssociatedDevices);
	m_ppDevicePrograms  = new DeviceProgram* [m_szNumAssociatedDevices];

	if (!m_ppDevicePrograms)
	{
        if (piRet)
	    {
		    *piRet = CL_OUT_OF_HOST_MEMORY;
	    }
        return;
	}

	if (!CopySourceStrings(uiNumStrings, pSources, pszLengths))
	{
		delete[] m_ppDevicePrograms;
		m_ppDevicePrograms = NULL;

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
        m_ppDevicePrograms[i]->SetStateInternal(DEVICE_PROGRAM_SOURCE);
	}


	if (piRet)
	{
        *piRet = CL_SUCCESS;
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

	if (m_szSourceString)
	{
		delete[] m_szSourceString;
		m_szSourceString   = NULL;
	}
}

cl_err_code ProgramWithSource::GetInfo(cl_int param_name, size_t param_value_size, void * param_value, size_t * param_value_size_ret) const
{
	LOG_DEBUG(TEXT("ProgramWithSource::GetInfo enter. param_name=%d, param_value_size=%d, param_value=%d, param_value_size_ret=%d"), 
		param_name, param_value_size, param_value, param_value_size_ret);

	if (NULL == param_value && NULL == param_value_size_ret)
	{
		return CL_INVALID_VALUE;
	}
	size_t szParamValueSize = 0;

	switch ( (cl_program_info)param_name )
	{
	case CL_PROGRAM_SOURCE:
		{
            // Note: according to spec section 5.4.5, the length returned should include the null terminator
			szParamValueSize = strlen(m_szSourceString) + 1;

			if (NULL != param_value)
			{
				if (param_value_size < szParamValueSize)
				{
					return CL_INVALID_VALUE;
				}

				MEMCPY_S(param_value, szParamValueSize, m_szSourceString, szParamValueSize);
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
    size_t uiTotalLength = 1;
    size_t* puiStringLengths = new size_t[uiNumStrings];
    if (!puiStringLengths)
    {
        return false;
    }

	for(cl_uint ui = 0; ui < uiNumStrings; ++ui)
	{
		if ( (NULL == pszLengths) || (0 == pszLengths[ui]) )
		{
            puiStringLengths[ui] = strlen(pSources[ui]);
		}
		else
		{
            puiStringLengths[ui] = pszLengths[ui];
		}

        uiTotalLength += puiStringLengths[ui];
	}

    m_szSourceString = new char[uiTotalLength];
    if (!m_szSourceString)
    {
        delete[] puiStringLengths;
        return false;
    }

    char* szSourceString = m_szSourceString;
    MEMCPY_S(szSourceString, puiStringLengths[0], pSources[0], puiStringLengths[0]);

	for (cl_uint ui = 1; ui < uiNumStrings; ++ui)
	{
        szSourceString += puiStringLengths[ui - 1];
		MEMCPY_S(szSourceString, puiStringLengths[ui], pSources[ui], puiStringLengths[ui]);
	}

	m_szSourceString[uiTotalLength - 1] = '\0'; //uiTotalLength includes the NULL terminator

    delete[] puiStringLengths;
	return true;
}
