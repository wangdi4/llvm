#pragma once
#include "program.h"

namespace Intel { namespace OpenCL { namespace Framework {

    // Created internally on call to clLinkProgram

	class ProgramForLink : public Program
	{

        PREPARE_SHARED_PTR(ProgramForLink);

	public:		

        static SharedPtr<ProgramForLink> Allocate(SharedPtr<Context> pContext, cl_uint uiNumDevices, SharedPtr<FissionableDevice>* pDevices, cl_int *piRet)
        {
            return SharedPtr<ProgramForLink>(new ProgramForLink(pContext, uiNumDevices, pDevices, piRet));
        }

	protected:

        ProgramForLink(SharedPtr<Context> pContext, cl_uint uiNumDevices, SharedPtr<FissionableDevice>* pDevices, cl_int *piRet);

		virtual ~ProgramForLink();
	};
}}}
