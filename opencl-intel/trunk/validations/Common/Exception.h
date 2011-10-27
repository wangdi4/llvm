/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  Exception.h

\*****************************************************************************/
#ifndef __EXCEPTION_H__
#define __EXCEPTION_H__

#include <string>
#include <stdexcept>

namespace Validation { namespace Exception {


    /// base class for validation exceptions
    class ValidationExceptionBase :
        public std::runtime_error
    {
    public:
        ValidationExceptionBase(const std::string& str)
            : std::runtime_error(str)
        {}

        virtual ~ValidationExceptionBase() throw() {}
    };

/// macro for convenient definition of validation exceptions derived from
/// the base class ValidationExceptionBase
#define DEFINE_VALIDATION_EXCEPTION(__name)\
    class __name : public ValidationExceptionBase{\
    public:\
    __name(const std::string& str) : ValidationExceptionBase(std::string(#__name)+' '+str){}\
    };

    /// Exception for reporting file input/output errors
    DEFINE_VALIDATION_EXCEPTION(IOError)

    /// Exception for reporting function call invalid arguments
    DEFINE_VALIDATION_EXCEPTION(InvalidArgument)

    /// Exception for reporting unsupported by NEAT situations.
    DEFINE_VALIDATION_EXCEPTION(NEATTrackFailure)

    /// Exception for illegal function call exceptions
    DEFINE_VALIDATION_EXCEPTION(IllegalFunctionCall)

    /// Exception for out of range access
    DEFINE_VALIDATION_EXCEPTION(OutOfRange)

    /// Exception for Not implemented functions
    DEFINE_VALIDATION_EXCEPTION(NotImplemented)

    /// Exception for illegal command line parameters
    DEFINE_VALIDATION_EXCEPTION(CmdLineException)

}}

#endif // __EXCEPTION_H__
