/*****************************************************************************\

Copyright (c) Intel Corporation (2009).

    INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
    LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
    ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
    PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
    DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
    PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
    including liability for infringement of any proprietary rights, relating to
    use of the code. No license, express or implied, by estoppel or otherwise,
    to any intellectual property rights is granted herein.

File Name:  exceptions.h

Abstract:

Notes:

\*****************************************************************************/

#pragma once
#include <stdexcept>

/// macro for convenient definition of validation exceptions derived from
/// the base class ValidationExceptionBase
#define DEFINE_FRONTEND_EXCEPTION(__name)\
class __name : public std::domain_error{\
public:\
__name(const std::string& str) : std::domain_error(str){}\
};

DEFINE_FRONTEND_EXCEPTION(invalid_input_param)
DEFINE_FRONTEND_EXCEPTION(common_clang_error)
DEFINE_FRONTEND_EXCEPTION(unknown_hw_platform)
DEFINE_FRONTEND_EXCEPTION(internal_error)
