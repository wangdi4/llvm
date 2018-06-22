// INTEL CONFIDENTIAL
//
// Copyright 2010-2018 Intel Corporation.
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you (License). Unless the License provides otherwise, you may not
// use, modify, copy, publish, distribute, disclose or transmit this software or
// the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.

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
