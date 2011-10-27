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

File Name:  MICKernelProperties.h

\*****************************************************************************/
#pragma once

#include "KernelProperties.h"
#include "Serializer.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

class SerializationStatus;

class MICKernelJITProperties : public KernelJITProperties
{
public:
    /**
     * Serialization methods for the class (used by the serialization service)
     */
    void Serialize(IOutputStream& ost, SerializationStatus* stats);
    void Deserialize(IInputStream& ist, SerializationStatus* stats);
};

class MICKernelProperties : public KernelProperties
{
public:
    MICKernelProperties() { };

    MICKernelProperties(KernelProperties* pKernelProps);

    /**
     * Serialization methods for the class (used by the serialization service)
     */
    void Serialize(IOutputStream& ost, SerializationStatus* stats);
    void Deserialize(IInputStream& ist, SerializationStatus* stats);
};

}}} // namespace
