/*****************************************************************************\

Copyright (c) Intel Corporation (2012).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name: OCLKernelBufferContainerListAllocator.cpp

\*****************************************************************************/

#include "OCLKernelBufferContainerListAllocator.h"
#include "Image.h"
#include "Buffer.h"
#include "BufferContainerList.h"


namespace Validation
{
    void OCLKernelBufferContainerListAllocator::Read(IContainer *p){
        IBufferContainerList* pBCL = static_cast<IBufferContainerList*>(p);
        // should be one BufferContainer
        IBufferContainer * pBC = pBCL->CreateBufferContainer();
        uint32_t i=0;

        // loop over arguments
        for (OCLKernelArgumentsList::const_iterator ip = m_list.begin(); ip != m_list.end(); ++ip, ++i)
        {
            IMemoryObjectDescPtr pDesc = *ip;
            if (pDesc->GetName() == BufferDesc::GetBufferDescName())
            {
                // buffer
                BufferDesc * bdesc = static_cast<BufferDesc *>(pDesc.get());
                //dereference the pointers
                TypeDesc Ty = bdesc->GetElementDescription();
                if(Ty.GetType() == TPOINTER)
                {
                    //recalculate number of elements in buffer
                    bdesc->SetNumOfElements(bdesc->NumOfElements()*Ty.GetNumberOfElements());
                    Ty = Ty.GetSubTypeDesc(0);//dereference the pointer
                    bdesc->SetElementDecs(Ty);//set up new descriptor
                }
                pBC->CreateBuffer(*bdesc);
            }
            else if(pDesc->GetName() == ImageDesc::GetImageDescName())
            {
                // image
                ImageDesc * idesc = static_cast<ImageDesc *>(pDesc.get());
                pBC->CreateImage(*idesc);
            }
            else
                // unknown IMemoryObjectDesc
                throw Exception::GeneratorBadTypeException("unknown IMemoryObjectDesc");
        }
    }
}
