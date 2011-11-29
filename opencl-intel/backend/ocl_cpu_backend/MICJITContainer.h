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

File Name:  MICJITContainer.h

\*****************************************************************************/
#pragma once

#include "cl_dev_backend_api.h"
#include "cl_device_api.h"
#include "Kernel.h"
#include "ModuleJITHolder.h"
#include "MICKernelProperties.h"
#include "Serializer.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class SerializationStatus;

/**
 * Used for MIC JIT, This class isn't owner of the JIT it's just point to the 
 * function entrypoint and owns the properties only
 */

class MICJITContainer: public IKernelJITContainer
{
public:
    MICJITContainer():
        m_pModuleJITHolder(NULL),
        m_pProps(NULL)
        { };
    MICJITContainer(const ModuleJITHolder* pModuleHolder,
                    unsigned long long int funcID,
                    MICKernelJITProperties* pProps);
    ~MICJITContainer();

    /*
     * ICLDevBackendJITContainer methods
     */
    virtual const void* GetJITCode() const;

    virtual size_t GetJITCodeSize() const { return 0; } // TODO: Check this later

    /*
     * IJITContainer methods
     */
    KernelJITProperties* GetProps() const  { return m_pProps; }

    /**
     * Serialization methods for the class (used by the serialization service)
     */
    void Serialize(IOutputStream& ost, SerializationStatus* stats);
    void Deserialize(IInputStream& ist, SerializationStatus* stats);

private:
    const ModuleJITHolder*  m_pModuleJITHolder; // not owned by the class
    unsigned long long int  m_funcID;

    MICKernelJITProperties*   m_pProps;

    // Klockwork Issue
    MICJITContainer ( const MICJITContainer& x );

    // Klockwork Issue
    MICJITContainer& operator= ( const MICJITContainer& x );
};

}}} // namespace

