/*****************************************************************************\

Copyright (c) Intel Corporation (2010).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppels or otherwise,
    to any intellectual property rights is granted herein.

File Name:  exceptions.h

\*****************************************************************************/

#pragma once
#include <stdexcept>
#include "cl_device_api.h"


#ifndef LLVM_BACKEND_UNUSED
	#if defined(_WIN32)
		#define LLVM_BACKEND_UNUSED
	#else
		#define LLVM_BACKEND_UNUSED __attribute__ ((unused))
	#endif
#endif
#pragma warning (disable : 4985 ) /* disable ceil warnings */ 


namespace Intel { namespace OpenCL { namespace DeviceBackend { namespace Exceptions {

    /// base class for validation exceptions
    class DeviceBackendExceptionBase :
        public std::runtime_error
    {
    public:
        DeviceBackendExceptionBase(const std::string& str, cl_dev_err_code errCode = CL_DEV_ERROR_FAIL)
            : std::runtime_error(str), m_errCode(errCode)
        {}

        virtual ~DeviceBackendExceptionBase() throw() {}

        cl_dev_err_code GetErrorCode() const { return m_errCode; }

    private:
        cl_dev_err_code m_errCode;
    };



/// macro for convenient definition of device backend exceptions derived from
/// the base class DeviceBackendExceptionBase
#define DEFINE_EXCEPTION(__name)\
    namespace Exceptions{\
        class __name : public Exceptions::DeviceBackendExceptionBase{\
           public:\
            __name(const std::string& str, cl_dev_err_code errCode = CL_DEV_ERROR_FAIL) : DeviceBackendExceptionBase(std::string(#__name)+' '+str, errCode){}\
        };\
    }







}}}}
