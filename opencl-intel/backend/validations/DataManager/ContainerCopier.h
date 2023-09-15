// INTEL CONFIDENTIAL
//
// Copyright 2011 Intel Corporation.
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

#include "IContainerVisitor.h"
#include "IMemoryObject.h"

namespace Validation {
/// Creates BufferContainer copy.
/// For given BufferContainerList and index of buffer container it creates
/// new buffer container list with single buffer container in it
/// Then it copies all buffers from source buffer container to new
/// Then it returns pointer to new BufferContainerList
/// It doesn't free memory so calling code should handle it
class ContainerCopier : IContainerVisitor {
public:
  ContainerCopier() : m_pList(NULL) {}

  /// This method wouldn't be responsible for memory freeing.
  /// It is responsibility of calling code.
  BufferContainerList *CloneBufferContainer(const IBufferContainerList *pList,
                                            uint32_t idx) {
    BufferContainer *pBc =
        static_cast<BufferContainer *>(pList->GetBufferContainer(idx));
    m_pList = new BufferContainerList();
    m_pList->CreateBufferContainer();
    pBc->Accept(*this);
    return m_pList;
  }

  virtual void visitImage(const IMemoryObject *in_pImage) override {
    assert(m_pList->GetBufferContainerCount() == 1);
    IBufferContainer *pBc = m_pList->GetBufferContainer(0);
    const IMemoryObjectDesc *pDesc = in_pImage->GetMemoryObjectDesc();
    std::unique_ptr<ImageDesc> pNewDesc(
        static_cast<ImageDesc *>(pDesc->Clone()));
    IMemoryObject *pNewBuf = pBc->CreateImage(*pNewDesc);
    void *pNewData = pNewBuf->GetDataPtr();
    void *pOldData = in_pImage->GetDataPtr();
    const ImageDesc *pImgDesc =
        static_cast<const ImageDesc *>(in_pImage->GetMemoryObjectDesc());
    memcpy(pNewData, pOldData, pImgDesc->GetSizeInBytes());
  }

  virtual void visitBuffer(const IMemoryObject *in_pBuffer) override {
    assert(m_pList->GetBufferContainerCount() == 1);
    IBufferContainer *pBc = m_pList->GetBufferContainer(0);
    const IMemoryObjectDesc *pDesc = in_pBuffer->GetMemoryObjectDesc();
    std::unique_ptr<BufferDesc> pNewDesc(
        static_cast<BufferDesc *>(pDesc->Clone()));
    IMemoryObject *pNewBuf = pBc->CreateBuffer(*pNewDesc);
    void *pNewData = pNewBuf->GetDataPtr();
    void *pOldData = in_pBuffer->GetDataPtr();
    const BufferDesc *pBufDesc =
        static_cast<const BufferDesc *>(in_pBuffer->GetMemoryObjectDesc());
    memcpy(pNewData, pOldData, pBufDesc->GetSizeInBytes());
  }

  virtual void
  visitBufferContainer(const IBufferContainer *pBufferContainer) override {
    // Do nothing
  }
  virtual void visitBufferContainerList(
      const IBufferContainerList *pBufferContainerList) override {
    assert(false);
  }

private:
  BufferContainerList *m_pList;
};
} // namespace Validation
