#pragma once
#include "program.h"

namespace Intel { namespace OpenCL { namespace Framework {

	class ProgramWithBinary : public Program
	{
	public:
		ProgramWithBinary(Context* pContext, cl_uint uiNumDevices, FissionableDevice** pDevices, const size_t* pszLengths, const unsigned char** pBinaries, cl_int* piBinaryStatus, cl_int *piRet, ocl_entry_points * pOclEntryPoints);

		cl_err_code	GetInfo(cl_int param_name, size_t param_value_size, void * param_value, size_t * param_value_size_ret) const;

	protected:
		virtual ~ProgramWithBinary();

	};
}}}
