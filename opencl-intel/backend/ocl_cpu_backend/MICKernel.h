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

File Name:  MICKernel.h

\*****************************************************************************/
#pragma once

#include "Kernel.h"
#include "Serializer.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class SerializationStatus;
class MICKernelProperties;

class MICKernel : public Kernel
{
public:
    MICKernel():
      Kernel(), 
      m_kernelID(0) 
      { };

    MICKernel(const std::string& name, const std::vector<cl_kernel_argument>& args,
           KernelProperties* pProps):
        Kernel(name, args, pProps),
        m_kernelID(0)
        { };

    /**
     * Sets the kernel ID
     */
    void SetKernelID(unsigned long long int kernelID);

    /**
     * @returns an unsigned long which represents the kernel id - this id is unique
     *  per kernel - ; in case of failure 0 will be returned
     */
    virtual unsigned long long int GetKernelID() const;

    /**
     * Serialization methods for the class (used by the serialization service)
     */
    void Serialize(IOutputStream& ost, SerializationStatus* stats);
    void Deserialize(IInputStream& ist, SerializationStatus* stats);

protected:
    unsigned long long int m_kernelID;
};

}}} // namespace
