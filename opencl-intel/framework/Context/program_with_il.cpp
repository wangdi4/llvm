#include "Context.h"
#include "program_with_il.h"
#include "cl_logger.h"
#include "cl_sys_defines.h"

using namespace Intel::OpenCL::Framework;
using namespace Intel::OpenCL::Utils;

ProgramWithIL::ProgramWithIL(SharedPtr<Context>   pContext,
                             const unsigned char* pIL,
                             size_t               length,
                             cl_int*              piRet)
    : Program(pContext)
{
    cl_int ret = CL_SUCCESS;

    SharedPtr<FissionableDevice>* pDevices = pContext->GetDevices(&m_szNumAssociatedDevices);
    try
    {
        m_pIL.assign(pIL, pIL + length);
        m_ppDevicePrograms.resize(m_szNumAssociatedDevices);

        for(size_t i = 0; i < m_szNumAssociatedDevices; ++i)
        {
            //std::unique_ptr<DeviceProgram>& pDevProgram = *I;
            unique_ptr<DeviceProgram>& pDevProgram = m_ppDevicePrograms[i];
            //pDevProgram.reset(new DeviceProgram());
            pDevProgram.reset(new DeviceProgram());

            pDevProgram->SetDevice(pDevices[i]);
            pDevProgram->SetHandle(GetHandle());
            pDevProgram->SetContext(pContext->GetHandle());

            cl_prog_binary_type uiBinaryType;
            if(!pDevProgram->CheckProgramBinary(length, pIL, &uiBinaryType)
                || uiBinaryType != CL_PROG_BIN_COMPILED_SPIRV)
            {
                ret = CL_INVALID_VALUE;
                break;
            }

            // TODO: delete next line once SPIRV consumer will be moved to FE.
            pDevProgram->SetBinaryInternal(length, pIL, CL_PROGRAM_BINARY_TYPE_SPIRV);

            pDevProgram->SetStateInternal(DEVICE_PROGRAM_SPIRV);
        }
    }
    catch(std::bad_alloc& e)
    {
        ret = CL_OUT_OF_HOST_MEMORY;
    }

    if(piRet)
    {
        *piRet = ret;
    }
}

cl_err_code ProgramWithIL::GetInfo(cl_int  param_name,
                                   size_t  param_value_size,
                                   void*   param_value,
                                   size_t* param_value_size_ret) const
{
	switch(param_name)
	{
	case CL_PROGRAM_IL:
		{
            size_t szParamValueSize = m_pIL.size();
			if (nullptr != param_value)
			{
				if (param_value_size < szParamValueSize)
				{
					return CL_INVALID_VALUE;
				}

				MEMCPY_S(param_value, szParamValueSize, m_pIL.data(), szParamValueSize);
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

ProgramWithIL::~ProgramWithIL()
{}
