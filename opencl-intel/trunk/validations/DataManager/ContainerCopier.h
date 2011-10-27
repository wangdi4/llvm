/*****************************************************************************\

Copyright (c) Intel Corporation (2011).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  ContainerCopier.h

\*****************************************************************************/

#include "IContainerVisitor.h"
#include "IMemoryObject.h"

namespace Validation
{
    /// Creates BufferContainer copy.
    /// For given BufferContainerList and index of buffer container it creates
    /// new buffer container list with single buffer container in it
    /// Then it copies all buffers from source buffer container to new
    /// Then it returns pointer to new BufferContainerList
    /// It doesn't free memory so calling code should handle it
    class ContainerCopier : IContainerVisitor
    {
    public:
        ContainerCopier()
        :m_pList(NULL)
        {
        }

        /// This method wouldn't be responsible for memory freeing.
        /// It is responsibility of calling code.
        BufferContainerList* CloneBufferContainer(const IBufferContainerList* pList, uint32_t idx)
        {
            BufferContainer* pBc = static_cast<BufferContainer*>(pList->GetBufferContainer(idx));
            m_pList = new BufferContainerList();
            m_pList->CreateBufferContainer();
            pBc->Accept(*this);
            return m_pList;
        }

        virtual void visitImage( const IMemoryObject* in_pImage )
        {
            assert(m_pList->GetBufferContainerCount() == 1);
            IBufferContainer* pBc = m_pList->GetBufferContainer(0);
            const IMemoryObjectDesc* pDesc = in_pImage->GetMemoryObjectDesc();
            std::auto_ptr<ImageDesc> pNewDesc(static_cast<ImageDesc *>(pDesc->Clone()));
            IMemoryObject* pNewBuf = pBc->CreateImage(*pNewDesc);
            void *pNewData = pNewBuf->GetDataPtr();
            void *pOldData = in_pImage->GetDataPtr();
            const ImageDesc *pImgDesc = 
                static_cast<const ImageDesc *>(in_pImage->GetMemoryObjectDesc());
            memcpy(pNewData, pOldData, pImgDesc->GetImageSizeInBytes());
        }

        virtual void visitBuffer( const IMemoryObject* in_pBuffer )
        {
            assert(m_pList->GetBufferContainerCount() == 1);
            IBufferContainer* pBc = m_pList->GetBufferContainer(0);
            const IMemoryObjectDesc* pDesc = in_pBuffer->GetMemoryObjectDesc();
            std::auto_ptr<BufferDesc> pNewDesc(static_cast<BufferDesc *>(pDesc->Clone()));
            IMemoryObject* pNewBuf = pBc->CreateBuffer(*pNewDesc);
            void *pNewData = pNewBuf->GetDataPtr();
            void *pOldData = in_pBuffer->GetDataPtr();
            const BufferDesc *pBufDesc = 
                static_cast<const BufferDesc *>(in_pBuffer->GetMemoryObjectDesc());
            memcpy(pNewData, pOldData, pBufDesc->GetBufferSizeInBytes());
        }

        virtual void visitBufferContainer( const IBufferContainer* pBufferContainer)
        {
            // Do nothing
        }
        virtual void visitBufferContainerList( const IBufferContainerList* pBufferContainerList )
        {
            assert(false);
        }

    private:
        BufferContainerList* m_pList;
    };
}
