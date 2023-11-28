// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#include "OCLKernelBufferContainerListAllocator.h"
#include "Buffer.h"
#include "BufferContainerList.h"
#include "Image.h"

namespace Validation {
void OCLKernelBufferContainerListAllocator::Read(IContainer *p) {
  IBufferContainerList *pBCL = static_cast<IBufferContainerList *>(p);
  // should be one BufferContainer
  IBufferContainer *pBC = pBCL->CreateBufferContainer();
  uint32_t i = 0;

  // loop over arguments
  for (OCLKernelArgumentsList::const_iterator ip = m_list.begin();
       ip != m_list.end(); ++ip, ++i) {
    IMemoryObjectDescPtr pDesc = *ip;
    if (pDesc->GetName() == BufferDesc::GetBufferDescName()) {
      // buffer
      BufferDesc *bdesc = static_cast<BufferDesc *>(pDesc.get());
      // dereference the pointers
      TypeDesc Ty = bdesc->GetElementDescription();
      if (Ty.GetType() == TPOINTER) {
        // recalculate number of elements in buffer
        bdesc->SetNumOfElements(bdesc->NumOfElements() *
                                Ty.GetNumberOfElements());
        Ty = Ty.GetSubTypeDesc(0); // dereference the pointer
        bdesc->SetElementDecs(Ty); // set up new descriptor
      }
      pBC->CreateBuffer(*bdesc);
    } else if (pDesc->GetName() == ImageDesc::GetImageDescName()) {
      // image
      ImageDesc *idesc = static_cast<ImageDesc *>(pDesc.get());
      pBC->CreateImage(*idesc);
    } else
      // unknown IMemoryObjectDesc
      throw Exception::GeneratorBadTypeException("unknown IMemoryObjectDesc");
  }
}
} // namespace Validation
