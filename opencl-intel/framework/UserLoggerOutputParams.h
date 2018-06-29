// INTEL CONFIDENTIAL
//
// Copyright 2006-2018 Intel Corporation.
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

#include <string>
#include "ocl_config.h"

namespace Intel { namespace OpenCL { namespace Framework {

/**
 * This class is responsible for providing the values of output paramters to the FrameworkUserLogger after the function returns
 */
class OutputParamsValueProvider
{
public:

    /**
     * This class is responsible for special printing of output parameters (i.e., parsing a list of object)
     */
    class SpecialOutputParamPrinter
    {
    public:

        /**
         * @return the string to print
         */
        virtual std::string GetStringToPrint() const = 0;

    };

    /**
     * Constructor
     * @param apiLogger the ApiLogger that collects log data in the current API call function
     * @param specialPrinter an optinal SpecialOutputParamPrinter to use
     */
    OutputParamsValueProvider(Intel::OpenCL::Utils::ApiLogger& apiLogger, const SpecialOutputParamPrinter* specialPrinter = nullptr) :
        m_apiLogger(apiLogger), m_specialPrinter(specialPrinter) { }

    /**
     * Destructor
     */
    ~OutputParamsValueProvider();

    /**
     * Add an output parameter
     * @param T             the type of the value pointed to by the output parameter
     * @param name          the name of the parameter
     * @param param         the address of the output parameter
     * @param bIsPtr2Ptr    whether the parameter's type is a pointer to pointer
     * @param bIsUnsigned   whether the parameter is an unsigned integer type (only relevant when bIsPtr2Ptr is false)
     */
    template<typename T>
    void AddParam(const std::string& name, const T* param, bool bIsPtr2Ptr, bool bIsUnsigned = false)
    {
        m_outputParamsVec.push_back(ParamInfo(name, param, sizeof(T), bIsPtr2Ptr, bIsUnsigned));
    }    

private:

    struct ParamInfo {

        ParamInfo(const std::string& name, const void* addr, size_t size, bool bIsPtr2Ptr, bool bIsUnsigned) : 
            m_name(name), m_addr(addr), m_size(size), m_bIsPtr2Ptr(bIsPtr2Ptr), m_bIsUnsigned(bIsUnsigned) { }

        std::string m_name;
        const void* m_addr;
        size_t m_size;
        bool m_bIsPtr2Ptr;
        bool m_bIsUnsigned;
    };

    void Print2Logger();

    Intel::OpenCL::Utils::ApiLogger& m_apiLogger;
    std::vector<ParamInfo> m_outputParamsVec;
    const SpecialOutputParamPrinter* const m_specialPrinter;

};

inline OutputParamsValueProvider::~OutputParamsValueProvider()
{
    if (Intel::OpenCL::Utils::g_pUserLogger->IsApiLoggingEnabled())
    {
        Print2Logger();        
    }
}

}}}
