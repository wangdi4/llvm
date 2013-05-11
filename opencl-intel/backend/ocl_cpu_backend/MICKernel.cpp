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

File Name:  MICKernel.cpp

\*****************************************************************************/

#include "MICKernel.h"
#include "MICJITContainer.h"
#include "IAbstractBackendFactory.h"
#include "MICSerializationService.h"
#include <stdio.h>
#ifdef KNC_CARD
#include <jitprofiling.h>
#endif

namespace Intel { namespace OpenCL { namespace DeviceBackend {

MICKernel::~MICKernel()
{
#ifdef KNC_CARD
  // Unregister with VTune
  for (std::vector<IKernelJITContainer*>::iterator it = m_JITs.begin(); it != m_JITs.end(); it++) {
    MICKernelJITProperties* props = static_cast<MICKernelJITProperties*>(((MICJITContainer*)*it)->GetProps());
    if (props->GetUseVTune()) {
      iJIT_Method_Id JitData;
      JitData.method_id = ((MICJITContainer*)*it)->GetFuncID(); 
      (void) ::iJIT_NotifyEvent(iJVM_EVENT_TYPE_METHOD_UNLOAD_START, &JitData);
    }
  }
#endif
}

void MICKernel::SetKernelID(unsigned long long int kernelID)
{
    m_kernelID = kernelID;
}

unsigned long long int MICKernel::GetKernelID() const
{
    return m_kernelID;
}

void MICKernel::Serialize(IOutputStream& ost, SerializationStatus* stats)
{
    Serializer::SerialPrimitive<unsigned long long int>(&m_kernelID, ost);
    Serializer::SerialString(m_name, ost);

    // Serial the kernel arguments (one by one)
    unsigned int vectorSize = m_args.size();
    Serializer::SerialPrimitive<unsigned int>(&vectorSize, ost);
    for(std::vector<cl_kernel_argument>::const_iterator it = m_args.begin(); it != m_args.end(); ++it)
    {
        cl_kernel_argument currentArgument = *it;
        Serializer::SerialPrimitive<cl_kernel_arg_type>(&currentArgument.type, ost);
        Serializer::SerialPrimitive<unsigned int>(&currentArgument.size_in_bytes, ost);
    }

    Serializer::SerialPointerHint((const void**)&m_pProps, ost); 
    if(NULL != m_pProps)
    {
        static_cast<MICKernelProperties*>(m_pProps)->Serialize(ost, stats);
    }

    // Serial the kernel JIT's (one by one)
    vectorSize = m_JITs.size();
    Serializer::SerialPrimitive<unsigned int>(&vectorSize, ost);
    for(std::vector<IKernelJITContainer*>::const_iterator it = m_JITs.begin(); it != m_JITs.end(); ++it)
    {
        MICJITContainer* currentArgument = (MICJITContainer*)(*it);
        Serializer::SerialPointerHint((const void**)&currentArgument, ost); 
        if(NULL != currentArgument)
        {
            currentArgument->Serialize(ost, stats);
        }
    }
}

void MICKernel::Deserialize(IInputStream& ist, SerializationStatus* stats)
{
    Serializer::DeserialPrimitive<unsigned long long int>(&m_kernelID, ist);
    Serializer::DeserialString(m_name, ist);

    // Deserial the kernel arguments (one by one)
    unsigned int vectorSize = 0;
    Serializer::DeserialPrimitive<unsigned int>(&vectorSize, ist);
    for(unsigned int i = 0; i < vectorSize; ++i)
    {
        cl_kernel_argument currentArgument;
        Serializer::DeserialPrimitive<cl_kernel_arg_type>(&currentArgument.type, ist);
        Serializer::DeserialPrimitive<unsigned int>(&currentArgument.size_in_bytes, ist);
        m_args.push_back(currentArgument);
    }

    Serializer::DeserialPointerHint((void**)&m_pProps, ist);
    if(NULL != m_pProps)
    {
        m_pProps = static_cast<MICKernelProperties*>(stats->GetBackendFactory()->CreateKernelProperties()); 
        static_cast<MICKernelProperties*>(m_pProps)->Deserialize(ist, stats);
    }

    Serializer::DeserialPrimitive<unsigned int>(&vectorSize, ist);
    for(unsigned int i = 0; i < vectorSize; ++i)
    {
        MICJITContainer* currentArgument = NULL;
        Serializer::DeserialPointerHint((void**)&currentArgument, ist); 
        if(NULL != currentArgument)
        {
            currentArgument = new MICJITContainer();
            currentArgument->Deserialize(ist, stats);
#ifdef KNC_CARD 
            // Register with VTune
            MICKernelJITProperties* props = static_cast<MICKernelJITProperties*>(currentArgument->GetProps());
            if (props->GetUseVTune()) {
              iJIT_Method_Load JitData;
              JitData.method_id = currentArgument->GetFuncID();
              JitData.method_name = (char *) GetKernelName();
              JitData.method_load_address = (void *) currentArgument->GetJITCode();
              JitData.method_size = currentArgument->GetJITCodeSize();
              JitData.line_number_size = 0;
              JitData.line_number_table = NULL;
              JitData.class_id = 0;
              JitData.class_file_name = NULL;
              JitData.source_file_name = NULL;
              JitData.user_data = NULL;
              JitData.user_data_size = 0;
              (void) ::iJIT_NotifyEvent(iJVM_EVENT_TYPE_METHOD_LOAD_FINISHED, &JitData);
            }
#endif
        }
        m_JITs.push_back(currentArgument);
    }
}


}}} // namespace
