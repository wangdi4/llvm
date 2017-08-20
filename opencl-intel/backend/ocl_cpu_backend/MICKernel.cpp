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

static void populateLineNumberInfo(const LineNumberTable *table,
                                   std::vector<LineNumberInfo> &result,
                                   int from, size_t size) {
  result.clear();
  if (table == nullptr)
    return;

  unsigned numEntries = table->size();
  for (unsigned offsetIndex = 0; offsetIndex < numEntries; offsetIndex++) {
    int offset = (*table)[offsetIndex].offset - from;
    if (offset < 0)
      continue;
    if (offset > (int)size)
      break;

    LineNumberInfo info = {(unsigned)offset,
                           (unsigned)((*table)[offsetIndex].line) };
    result.push_back(info);
  }
}

static void registerWithVTune(const ModuleJITHolder *MJH, KernelID kernelId,
                              const char *kernelName, size_t codeSize,
                              const void *JITCode) {
  // Set up line number table
  const LineNumberTable *lineNumberTable =
      MJH->GetKernelLineNumberTable(kernelId);
  std::vector<LineNumberInfo> lineNumberInfo;
  lineNumberInfo.reserve(lineNumberTable->size());

  populateLineNumberInfo(lineNumberTable, lineNumberInfo, 0, codeSize);

  // Create function data to send to vtune.
  // See documentation at jitprofiling.h for information on how it should be
  // structured.
  iJIT_Method_Load JitData;
  // Vtune requires IDs > 999
  JitData.method_id = 1000 + MJH->GetKernelVtuneFunctionId(kernelId);
  JitData.method_name = const_cast<char *>(kernelName);
  JitData.method_load_address = const_cast<void *>(JITCode);
  JitData.method_size = codeSize;
  JitData.line_number_size = lineNumberInfo.size();
  JitData.line_number_table = lineNumberInfo.data();
  JitData.class_id = 0;
  JitData.class_file_name = nullptr;
  JitData.source_file_name =
      const_cast<char *>(MJH->GetKernelFilename(kernelId));
  JitData.user_data = nullptr;
  JitData.user_data_size = 0;
  iJIT_NotifyEvent(iJVM_EVENT_TYPE_METHOD_LOAD_FINISHED, &JitData);

  const InlinedFunctions *inlinedFunctions =
      MJH->GetKernelInlinedFunctions(kernelId);
  if (inlinedFunctions == nullptr || inlinedFunctions->size() == 0)
    return;

  for (InlinedFunctions::const_iterator iter = inlinedFunctions->begin(),
                                        end = inlinedFunctions->end();
       iter != end; iter++) {
    const InlinedFunction &inlinedFunc = *iter;

    populateLineNumberInfo(lineNumberTable, lineNumberInfo, inlinedFunc.from,
                           inlinedFunc.size);

    // Create inline function data to send to vtune.
    // See documentation at jitprofiling.h for information on how it should be
    // structured.
    iJIT_Method_Inline_Load InlineJitData;
    void *address = (void *)(((unsigned long)JITCode) + inlinedFunc.from);
    // Vtune requires IDs > 999
    InlineJitData.method_id = 1000 + inlinedFunc.id;
    InlineJitData.parent_method_id = 1000 + inlinedFunc.parentId;
    InlineJitData.method_name =
        const_cast<char *>(inlinedFunc.funcname.c_str());
    InlineJitData.method_load_address = address;
    InlineJitData.method_size = inlinedFunc.size;
    InlineJitData.source_file_name =
        const_cast<char *>(inlinedFunc.filename.c_str());
    InlineJitData.line_number_size = lineNumberInfo.size();
    InlineJitData.line_number_table = lineNumberInfo.data();
    InlineJitData.class_file_name = nullptr;
    iJIT_NotifyEvent(iJVM_EVENT_TYPE_METHOD_INLINE_LOAD_FINISHED,
                     &InlineJitData);
  }
}

#endif

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

MICKernel::~MICKernel() {}

void MICKernel::SetKernelID(unsigned long long int kernelID) {
  m_kernelID = kernelID;
}

unsigned long long int MICKernel::GetKernelID() const { return m_kernelID; }

// In both of these functions, handle is used as an index in the JIT list
const void *MICKernel::CreateEntryPointHandle(const void *JitEP) const {
  if (GetKernelJIT(0)->GetJITCode() == JitEP)
    return 0;
  return (void *)1;
}
const void *MICKernel::ResolveEntryPointHandle(const void *Handle) const {
  // FIXME [OpenCL 2.0]: looks not OK with non-uniform work-group size.
  if (0 == Handle)
    return GetKernelJIT(0)->GetJITCode();
  assert(GetKernelJITCount() > 1 && "requesting un-available JIT!");
  return GetKernelJIT(1)->GetJITCode();
}

void MICKernel::Serialize(IOutputStream &ost, SerializationStatus *stats) const {
  Serializer::SerialPrimitive<unsigned long long int>(&m_kernelID, ost);
  Kernel::Serialize(ost, stats);
}

void MICKernel::Deserialize(IInputStream &ist, SerializationStatus *stats) {
  Serializer::DeserialPrimitive<unsigned long long int>(&m_kernelID, ist);
  Kernel::Deserialize(ist, stats);

#ifdef KNC_CARD
  for (unsigned int i = 0; i < m_JITs.size(); ++i) {
    MICJITContainer *currentArgument = static_cast<MICJITContainer*>(m_JITs[i]);
    if (nullptr != currentArgument) {
      // Register with VTune
      KernelJITProperties *props =
          static_cast<KernelJITProperties *>(currentArgument->GetProps());
      if (props->GetUseVTune()) {
        ModuleJITHolder *MJH =
            (ModuleJITHolder *)stats->GetPointerMark("pModuleJITHolder");
        registerWithVTune(MJH, currentArgument->GetFuncID(), GetKernelName(),
                          currentArgument->GetJITCodeSize(),
                          currentArgument->GetJITCode());
      }     
    }
  }
#endif 
}

}
}
} // namespace
