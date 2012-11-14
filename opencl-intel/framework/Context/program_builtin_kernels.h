#pragma once
#include "program.h"

#include <string>

namespace Intel { namespace OpenCL { namespace Framework {

	class ProgramWithBuiltInKernels : public Program
	{
	public:
		
        PREPARE_SHARED_PTR(ProgramWithBuiltInKernels);

        static SharedPtr<ProgramWithBuiltInKernels> Allocate(SharedPtr<Context> pContext, cl_uint uiNumDevices, SharedPtr<FissionableDevice>* pDevices, const char* szKernelNames,
            cl_int *piRet)
        {
            return SharedPtr<ProgramWithBuiltInKernels>(new ProgramWithBuiltInKernels(pContext, uiNumDevices, pDevices, szKernelNames, piRet));
        }

	protected:

        ProgramWithBuiltInKernels(SharedPtr<Context> pContext, cl_uint uiNumDevices, SharedPtr<FissionableDevice>* pDevices, const char* szKernelNames, cl_int *piRet);

		virtual ~ProgramWithBuiltInKernels();

		std::string	m_szKernelNames;
	};
}}}
