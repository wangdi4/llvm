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

// VTune-related functionality is only enabled when this is actually
// running on the device.
#ifdef KNC_CARD

#include "ModuleJITHolder.h"
#include <jitprofiling.h>

static void populateLineNumberInfo(const LineNumberTable* table,
                                   std::vector<LineNumberInfo>& result,
                                   int from,
                                   size_t size) {
    result.clear();
    if (table == NULL) return;

    unsigned numEntries = table->size();
    for (unsigned offsetIndex = 0; offsetIndex < numEntries; offsetIndex++) {
        int offset = (*table)[offsetIndex].offset - from;
        if (offset < 0) continue;
        if (offset > (int) size) break;

        LineNumberInfo info = {
            (unsigned)offset,
            (unsigned)((*table)[offsetIndex].line)
        };
        result.push_back(info);
    }
}

static void registerWithVTune(const ModuleJITHolder* MJH,
                              KernelID kernelId,
                              const char* kernelName,
                              size_t codeSize,
                              const void* JITCode) {
    // Set up line number table
    const LineNumberTable* lineNumberTable = MJH->GetKernelLineNumberTable(kernelId);
    std::vector<LineNumberInfo> lineNumberInfo;
    lineNumberInfo.reserve(lineNumberTable->size());

    populateLineNumberInfo(lineNumberTable, lineNumberInfo, 0, codeSize);

    // Create function data to send to vtune.
    // See documentation at jitprofiling.h for information on how it should be
    // structured.
    iJIT_Method_Load JitData;
    // Vtune requires IDs > 999
    JitData.method_id = 1000 + MJH->GetKernelVtuneFunctionId(kernelId);
    JitData.method_name = const_cast<char*>(kernelName);
    JitData.method_load_address = const_cast<void*>(JITCode);
    JitData.method_size = codeSize;
    JitData.line_number_size = lineNumberInfo.size();
    JitData.line_number_table = lineNumberInfo.data();
    JitData.class_id = 0;
    JitData.class_file_name = NULL;
    JitData.source_file_name = const_cast<char*>(MJH->GetKernelFilename(kernelId));
    JitData.user_data = NULL;
    JitData.user_data_size = 0;
    iJIT_NotifyEvent(iJVM_EVENT_TYPE_METHOD_LOAD_FINISHED, &JitData);

    const InlinedFunctions* inlinedFunctions = MJH->GetKernelInlinedFunctions(kernelId);
    if (inlinedFunctions == NULL || inlinedFunctions->size() == 0) return;

    for (InlinedFunctions::const_iterator iter = inlinedFunctions->begin(),
        end = inlinedFunctions->end(); iter != end; iter++) {
        const InlinedFunction& inlinedFunc = *iter;

        populateLineNumberInfo(lineNumberTable, lineNumberInfo,
            inlinedFunc.from, inlinedFunc.size);

        // Create inline function data to send to vtune.
        // See documentation at jitprofiling.h for information on how it should be
        // structured.
        iJIT_Method_Inline_Load InlineJitData;
        void* address = (void*)(((unsigned long)JITCode) + inlinedFunc.from);
        // Vtune requires IDs > 999
        InlineJitData.method_id = 1000 + inlinedFunc.id;
        InlineJitData.parent_method_id = 1000 + inlinedFunc.parentId;
        InlineJitData.method_name = const_cast<char*>(inlinedFunc.funcname.c_str());
        InlineJitData.method_load_address = address;
        InlineJitData.method_size = inlinedFunc.size;
        InlineJitData.source_file_name = const_cast<char*>(inlinedFunc.filename.c_str());
        InlineJitData.line_number_size = lineNumberInfo.size();
        InlineJitData.line_number_table = lineNumberInfo.data();
        InlineJitData.class_file_name = NULL;
        iJIT_NotifyEvent(iJVM_EVENT_TYPE_METHOD_INLINE_LOAD_FINISHED, &InlineJitData);
    }
}

#endif

namespace Intel { namespace OpenCL { namespace DeviceBackend {

MICKernel::~MICKernel()
{
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
              ModuleJITHolder* MJH = (ModuleJITHolder*)stats->GetPointerMark("pModuleJITHolder");
              registerWithVTune(MJH, currentArgument->GetFuncID(), GetKernelName(),
                  currentArgument->GetJITCodeSize(), currentArgument->GetJITCode());
            }
#endif
        }
        m_JITs.push_back(currentArgument);
    }
}

}}} // namespace
