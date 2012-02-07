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

File Name:  SATestException.h

\*****************************************************************************/
#ifndef SATest_EXCEPTION_H
#define SATest_EXCEPTION_H

#include "Exception.h"

namespace Validation { namespace Exception {
    /// Exception for reporting test comparison failure
    DEFINE_VALIDATION_EXCEPTION(TestFailException)

    /// Exception for reporting test run process failure
    DEFINE_VALIDATION_EXCEPTION(TestRunnerException)

    /// Exception for reporting reference run process failure
    DEFINE_VALIDATION_EXCEPTION(TestReferenceRunnerException)

    /// Exception for reporting general failures
    DEFINE_VALIDATION_EXCEPTION(GeneralException)

    /// Exception for reporting COI library failure
    DEFINE_VALIDATION_EXCEPTION(COIUsageException)

    /// Exception for reporting back-end failure
    DEFINE_VALIDATION_EXCEPTION(BackendException)
}}

#endif // SATest_EXCEPTION_H
