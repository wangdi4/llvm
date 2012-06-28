#pragma once
#include "program.h"

#include <string>

namespace Intel { namespace OpenCL { namespace Framework {

	class ProgramWithBuiltInKernels : public Program
	{
	public:
		ProgramWithBuiltInKernels(Context* pContext, cl_uint uiNumDevices, FissionableDevice** pDevices, const char* szKernelNames, cl_int *piRet);

	protected:
		virtual ~ProgramWithBuiltInKernels();

		std::string	m_szKernelNames;
	};
}}}
