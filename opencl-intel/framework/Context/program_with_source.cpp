#include "program_with_source.h"
#include "Device.h"
#include "Context.h"
#include "cl_sys_defines.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

ProgramWithSource::ProgramWithSource(SharedPtr<Context> pContext,
                                     cl_uint            uiNumStrings,
                                     const char**       pSources,
                                     const size_t*      pszLengths,
                                     cl_int*            piRet) :
	Program(pContext)
{
    if ((0 == uiNumStrings) || (nullptr == pSources))
    {
        if (piRet)
	    {
		    *piRet = CL_INVALID_VALUE;
	    }
        return;
    }

    for ( unsigned int i = 0; i < uiNumStrings; ++i)
    {
        if (nullptr == pSources[i])
        {
            if (piRet)
	        {
		        *piRet = CL_INVALID_VALUE;
	        }
            return;
        }
    }



    cl_int ret = CL_SUCCESS;

	SharedPtr<FissionableDevice>* pDevices = pContext->GetDevices(&m_szNumAssociatedDevices);
    try
    {
        m_ppDevicePrograms.resize(m_szNumAssociatedDevices);
        CopySourceStrings(uiNumStrings, pSources, pszLengths);

        for(size_t i = 0; i < m_szNumAssociatedDevices; ++i)
        {
            unique_ptr<DeviceProgram>& pDevProgram = m_ppDevicePrograms[i];
            pDevProgram.reset(new DeviceProgram());

            pDevProgram->SetDevice(pDevices[i]);
            pDevProgram->SetHandle(GetHandle());
            pDevProgram->SetContext(pContext->GetHandle());
            pDevProgram->SetStateInternal(DEVICE_PROGRAM_SOURCE);
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

ProgramWithSource::~ProgramWithSource()
{}

cl_err_code ProgramWithSource::GetInfo(cl_int  param_name,
                                       size_t  param_value_size,
                                       void*   param_value,
                                       size_t* param_value_size_ret) const
{
	LOG_DEBUG(TEXT("ProgramWithSource::GetInfo enter. param_name=%d, param_value_size=%d, param_value=%d, param_value_size_ret=%d"),
		param_name, param_value_size, param_value, param_value_size_ret);

	size_t szParamValueSize = 0;

	switch(param_name)
	{
	case CL_PROGRAM_SOURCE:
		{
			szParamValueSize = m_SourceString.size();

			if (nullptr != param_value)
			{
				if (param_value_size < szParamValueSize)
				{
					return CL_INVALID_VALUE;
				}

				MEMCPY_S(param_value, szParamValueSize, m_SourceString.data(), szParamValueSize);
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

bool ProgramWithSource::CopySourceStrings(cl_uint       uiNumStrings,
                                          const char**  pSources,
                                          const size_t* pszLengths)
{
    size_t uiTotalLength = 1;
    std::vector<size_t> puiStringLengths(uiNumStrings);

	for(cl_uint ui = 0; ui < uiNumStrings; ++ui)
	{
		if ( (nullptr == pszLengths) || (0 == pszLengths[ui]) )
		{
            puiStringLengths[ui] = strlen(pSources[ui]);
		}
		else
		{
            puiStringLengths[ui] = pszLengths[ui];
		}

        uiTotalLength += puiStringLengths[ui];
	}

    m_SourceString.resize(uiTotalLength);

    char* szSourceString = &m_SourceString[0];
    MEMCPY_S(szSourceString, puiStringLengths[0], pSources[0], puiStringLengths[0]);

	for (cl_uint ui = 1; ui < uiNumStrings; ++ui)
	{
        szSourceString += puiStringLengths[ui - 1];
		MEMCPY_S(szSourceString, puiStringLengths[ui], pSources[ui], puiStringLengths[ui]);
	}

	m_SourceString[uiTotalLength - 1] = '\0'; //uiTotalLength includes the NULL terminator

	return true;
}
