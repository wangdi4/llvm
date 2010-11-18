#include "program_with_source.h"
#include "device.h"
#include "Context.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

ProgramWithSource::ProgramWithSource(Context* pContext, cl_uint uiNumStrings, const char** pSources, const size_t* pszLengths, cl_int* piRet, ocl_entry_points * pOclEntryPoints) : Program(pContext, pOclEntryPoints)
{
	cl_int err = CL_SUCCESS;

	Device** pDevices = pContext->GetDevices(&m_szNumAssociatedDevices);
	m_pDevicePrograms  = new DeviceProgram[m_szNumAssociatedDevices];
	if (!m_pDevicePrograms)
	{
		err = CL_OUT_OF_HOST_MEMORY;
	}
	else
	{
		if (!CopySourceStrings(uiNumStrings, pSources, pszLengths))
		{
			err = CL_OUT_OF_HOST_MEMORY;
			delete[] m_pDevicePrograms;
			m_pDevicePrograms = NULL;
		}
		else
		{
			for (size_t i = 0; i < m_szNumAssociatedDevices; ++i)
			{
				m_pDevicePrograms[i].SetDevice(pDevices[i]);
				m_pDevicePrograms[i].SetHandle(GetHandle());
				m_pDevicePrograms[i].SetContext(pContext->GetHandle());
				m_pDevicePrograms[i].SetSource(m_uiNumStrings, m_pszStringLengths, const_cast<const char**>(m_pSourceStrings));
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
	if ((m_szNumAssociatedDevices > 0) && (NULL != m_pDevicePrograms))
	{
		delete[] m_pDevicePrograms;
		m_pDevicePrograms = NULL;
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
	LOG_DEBUG(L"ProgramWithSource::GetInfo enter. param_name=%d, param_value_size=%d, param_value=%d, param_value_size_ret=%d", 
		param_name, param_value_size, param_value, param_value_size_ret);

	cl_err_code clErrRet = CL_SUCCESS;
	if (NULL == param_value && NULL == param_value_size_ret)
	{
		return CL_INVALID_VALUE;
	}
	size_t szParamValueSize = 0;
	void * pValue = NULL;

	cl_uint uiParam = 0;
	switch ( (cl_program_info)param_name )
	{
	case CL_PROGRAM_BINARIES:
		{
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
					clErrRet = m_pDevicePrograms[i].GetBinary(0, NULL, &uiParam);
					if (CL_FAILED(clErrRet))
					{
						return clErrRet;
					}
					clErrRet = m_pDevicePrograms[i].GetBinary(uiParam, pParamValue[i], &uiParam);
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

	case CL_PROGRAM_BINARY_SIZES:
		{
			szParamValueSize = sizeof(size_t) * m_szNumAssociatedDevices;
			if (NULL != param_value)
			{
				if (param_value_size < szParamValueSize)
				{
					return CL_INVALID_VALUE;
				}
				size_t * pParamValue = static_cast<size_t *>(param_value);
				for (size_t i = 0; i < m_szNumAssociatedDevices; ++i)
				{
					clErrRet = m_pDevicePrograms[i].GetBinary(0, NULL, pParamValue + i);
					if (CL_FAILED(clErrRet))
					{
						return clErrRet;
					}
				}
			}
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
				memcpy_s(pSourceCode, m_pszStringLengths[0], m_pSourceStrings[0], m_pszStringLengths[0]);
				for (cl_uint ui = 1; ui < m_uiNumStrings; ++ui)
				{
					memcpy_s(pSourceCode + m_pszStringLengths[ui-1], m_pszStringLengths[ui], m_pSourceStrings[ui], m_pszStringLengths[ui]);
				}
				// Note: according to spec section 5.4.5, the length returned should include the null terminator
				pSourceCode[szParamValueSize - 1] = NULL;
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
			memcpy_s(m_pSourceStrings[ui], m_pszStringLengths[ui], pSources[ui], m_pszStringLengths[ui]);
		}
		//NULL terminate regardless of length
		m_pSourceStrings[ui][m_pszStringLengths[ui]] = 0; 
	}
	return true;
}
