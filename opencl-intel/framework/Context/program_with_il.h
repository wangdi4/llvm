#pragma once
#include "program.h"
#include "cl_shared_ptr.hpp"

namespace Intel { namespace OpenCL { namespace Framework {

class ProgramWithIL : public Program
{
    std::vector<char> m_pIL;
public:

    PREPARE_SHARED_PTR(ProgramWithIL)

    static SharedPtr<ProgramWithIL> Allocate(SharedPtr<Context>   pContext,
                                             const unsigned char* pIL,
                                             size_t               length,
                                             cl_int*              piRet)
    {
        return SharedPtr<ProgramWithIL>(new ProgramWithIL(pContext,
                                                          pIL,
                                                          length,
                                                          piRet));
    }

    virtual const char* GetSourceInternal() { return m_pIL.data(); };

    cl_err_code GetInfo(cl_int  param_name,
                        size_t  param_value_size,
                        void*   param_value,
                        size_t* param_value_size_ret) const;
protected:

    ProgramWithIL(SharedPtr<Context>   pContext,
                  const unsigned char* pIL,
                  size_t               length,
                  cl_int*              piRet);

    virtual ~ProgramWithIL();

};

}}}
