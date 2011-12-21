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

File Name:  IDynamicFunctionsResolver.h

\*****************************************************************************/

#pragma once
#include <string>

namespace Intel { namespace OpenCL { namespace DeviceBackend {

/**
 * This inetrface represent the dynamically loaded builtin functions address
 * resolver unit mainly the SVML
 */
class IDynamicFunctionsResolver
{
public:
    /**
     * @returns the function address of the required function; 0 in case function
     *  not known
     */
    virtual unsigned long long int GetFunctionAddress(const std::string& functionName) 
        const = 0;
};

}}} // namespace
