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

File Name:  MICProgram.h

\*****************************************************************************/
#pragma once

#include "Program.h"
#include "ModuleJITHolder.h"
#include "Serializer.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class SerializationStatus;

class MICProgram : public Program, public ICLDevBackendProgramJITCodeProperties
{
public:
    MICProgram();
    virtual ~MICProgram();

    /**
     * Store the given module JIT into the program
     * 
     * Note: will take ownership on passed module JIT
     */
    void SetModuleJITHolder( ModuleJITHolder* pModuleJITHolder); 

    /**
     * Gets the Program JIT Conatiner (debuging use only)
     */
    const ModuleJITHolder* GetModuleJITHolder() const;

    /**
     * Gets the program JIT Code Properties; 
     *
     * @returns JIT Code properties interface, NULL in case of failure
     */
    virtual const ICLDevBackendProgramJITCodeProperties* GetProgramJITCodeProperties() const;

    /**
     * @returns the size of the JIT code
     */
    virtual size_t GetCodeSize() const;

    /**
     * Serialization methods for the class (used by the serialization service)
     */
    void Serialize(IOutputStream& ost, SerializationStatus* stats) const;
    void Deserialize(IInputStream& ist, SerializationStatus* stats);

private:
    std::auto_ptr<ModuleJITHolder>        m_pModuleJITHolder;
};

}}} // namespace
