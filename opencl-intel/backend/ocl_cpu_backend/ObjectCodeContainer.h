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

File Name:  ObjectCodeContainer.h

\*****************************************************************************/
#pragma once

#include "cl_dev_backend_api.h"
#include "cl_types.h"


namespace Intel { namespace OpenCL { namespace DeviceBackend {

/**
 * Represents the container for Binary Object which contains serialized data
 * for the whole ocl program
 */
class ObjectCodeContainer : public ICLDevBackendCodeContainer
{
public:
    ObjectCodeContainer(const cl_prog_container_header* pObjectCodeContainer);
    ~ObjectCodeContainer();

    const void* GetCode() const;
    size_t GetCodeSize() const;

    const void* GetObject() const;
    size_t GetObjectSize() const;

private:
    cl_prog_container_header* m_pObjectCodeContainer; 
};

}}} // namespace

