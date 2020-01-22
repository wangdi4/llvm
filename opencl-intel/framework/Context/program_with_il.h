// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

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

    virtual const char* GetSourceInternal() { return m_pIL.data(); }

    virtual unsigned int GetSize()          { return m_pIL.size(); }

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
