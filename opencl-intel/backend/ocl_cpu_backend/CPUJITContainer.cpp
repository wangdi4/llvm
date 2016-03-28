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

File Name:  CPUJITContainer.cpp

\*****************************************************************************/
#include "CPUJITContainer.h"
#include "Kernel.h"
#include "KernelProperties.h"
#include "CPUProgram.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"

namespace Intel { namespace OpenCL { namespace DeviceBackend {

CPUJITContainer::CPUJITContainer():
    m_pFuncCode(nullptr),
    m_pFunction(nullptr),
    m_pModule(nullptr),
    m_pProps(nullptr)
{}

CPUJITContainer::CPUJITContainer(const void* pFuncCode,
                                 llvm::Function* pFunction,
                                 llvm::Module* pModule,
                                 KernelJITProperties* pProps):
    m_pFuncCode(pFuncCode),
    m_pFunction(pFunction),
    m_pModule(pModule),
    m_pProps(pProps) // get ownership of the pProps pointer
{}

CPUJITContainer::~CPUJITContainer()
{
    delete m_pProps;
}

void CPUJITContainer::Serialize(IOutputStream& ost, SerializationStatus* stats) const
{
    Serializer::SerialPointerHint((const void**)&m_pFuncCode, ost);
    Serializer::SerialPointerHint((const void**)&m_pFunction, ost);
    if(m_pFunction)
    {
        std::string name = m_pFunction->getName();
        Serializer::SerialString(name, ost);
    }
    Serializer::SerialPointerHint((const void**)&m_pModule, ost);
    Serializer::SerialPointerHint((const void**)&m_pProps, ost);
    if(m_pProps)
    {
        m_pProps->Serialize(ost, stats);
    }
}

void CPUJITContainer::Deserialize(IInputStream& ist, SerializationStatus* stats)
{
    std::string name;
    Serializer::DeserialPointerHint((void**)&m_pFuncCode, ist);
    Serializer::DeserialPointerHint((void**)&m_pFunction, ist);
    if(m_pFunction)
    {
        Serializer::DeserialString(name, ist);
    }
    Serializer::DeserialPointerHint((void**)&m_pModule, ist);
    Serializer::DeserialPointerHint((void**)&m_pProps, ist);
    if(m_pProps)
    {
        m_pProps = stats->GetBackendFactory()->CreateKernelJITProperties();
        m_pProps->Deserialize(ist, stats);
    }
    
    if(m_pModule)
    {
        m_pModule = (llvm::Module*)stats->GetPointerMark("pModule");
        if(m_pModule && m_pFunction)
        {
            m_pFunction = m_pModule->getFunction(name.c_str());
        }
    }
    
    CPUProgram* pProgram = (CPUProgram*)stats->GetPointerMark("pProgram");
    if(pProgram && m_pFuncCode && m_pFunction)
    {
        m_pFuncCode = reinterpret_cast<const void*>(
            pProgram->GetExecutionEngine()->getFunctionAddress(m_pFunction->getName().str()));
    }
}

}}} // namespace
