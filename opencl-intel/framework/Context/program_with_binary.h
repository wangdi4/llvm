#pragma once
#include "program.h"
#include "cl_shared_ptr.hpp"

namespace Intel { namespace OpenCL { namespace Framework {

	class ProgramWithBinary : public Program
	{
	public:		

        PREPARE_SHARED_PTR(ProgramWithBinary);

        static SharedPtr<ProgramWithBinary> Allocate(SharedPtr<Context> pContext, cl_uint uiNumDevices, SharedPtr<FissionableDevice>* pDevices, const size_t* pszLengths,
            const unsigned char** pBinaries, cl_int* piBinaryStatus, cl_int *piRet)
        {
            return SharedPtr<ProgramWithBinary>(new ProgramWithBinary(pContext, uiNumDevices, pDevices, pszLengths, pBinaries, piBinaryStatus, piRet));
        }

	protected:

        ProgramWithBinary(SharedPtr<Context> pContext, cl_uint uiNumDevices, SharedPtr<FissionableDevice>* pDevices, const size_t* pszLengths, const unsigned char** pBinaries,
			cl_int* piBinaryStatus, cl_int *piRet);

		virtual ~ProgramWithBinary();

	};
}}}
