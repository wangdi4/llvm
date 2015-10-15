#pragma once
#include "program.h"
#include "cl_shared_ptr.hpp"

namespace Intel { namespace OpenCL { namespace Framework {

	class ProgramWithIL : public Program
	{
	public:

        PREPARE_SHARED_PTR(ProgramWithIL)

        static SharedPtr<ProgramWithIL> Allocate(SharedPtr<Context> pContext, const unsigned char* pIL,
                                                 size_t length, cl_int *piRet)
        {
            return SharedPtr<ProgramWithIL>(new ProgramWithIL(pContext, pIL, length, piRet));
        }

	protected:

        ProgramWithIL(SharedPtr<Context> pContext, const unsigned char* pIL, size_t length, cl_int *piRet);

        virtual ~ProgramWithIL();

	};
}}}
