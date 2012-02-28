#pragma once
#include "program.h"

namespace Intel { namespace OpenCL { namespace Framework {

    // Created internally on call to clLinkProgram

	class ProgramForLink : public Program
	{
	public:
		ProgramForLink(Context* pContext, cl_uint uiNumDevices, FissionableDevice** pDevices, cl_int *piRet, ocl_entry_points * pOclEntryPoints);

	protected:
		virtual ~ProgramForLink();
	};
}}}
