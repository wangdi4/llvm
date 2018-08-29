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

#include "OpenCLArgsBuffer.h"
#include "Buffer.h"
#include "Image.h"
#include "SATestException.h"
#include "Exception.h"

#include "mem_utils.h"

#include "cpu_dev_limits.h"

#include <algorithm>
#include <functional>

using namespace Validation;

// must be a multiple of 64 to avoid alignment issues.
const size_t  PaddingSize = 4096;
const uint8_t PaddingVal  = 0xcc;

OpenCLArgsBuffer::OpenCLArgsBuffer(const cl_kernel_argument * pKernelArgs, 
                                   cl_uint kernelNumArgs, 
                                   IBufferContainerList * input,
                                   const ICLDevBackendImageService* pImageService,
                                   bool isCheckOOBAccess) : 
m_pKernelArgs(pKernelArgs), m_kernelNumArgs(kernelNumArgs), m_pImageService(pImageService), m_isCheckOOBAccess(isCheckOOBAccess)
{
    m_argsBufferSize = CalcArgsBufferSize();

    m_pArgsBuffer = new uint8_t[m_argsBufferSize];
    FillArgsBuffer(input);
}

OpenCLArgsBuffer::~OpenCLArgsBuffer(void)
{
    DestroyArgsBuffer();
    delete [] m_pArgsBuffer;
}

uint8_t* OpenCLArgsBuffer::GetArgsBuffer()
{
    return m_pArgsBuffer;
}

size_t OpenCLArgsBuffer::GetArgsBufferSize()
{
    return m_argsBufferSize;
}

void Validation::FillMemObjDescriptor( cl_mem_obj_descriptor& mem_desc, const BufferDesc& buffer_desc, void* pData)
{
    mem_desc.dim_count = 1;
    mem_desc.memObjType = CL_MEM_OBJECT_BUFFER;
    mem_desc.dimensions.dim[0] = buffer_desc.GetSizeInBytes();
    mem_desc.pData = pData;
}

void Validation::FillMemObjDescriptor( cl_mem_obj_descriptor& mem_desc,
                                       const ImageDesc& image_desc,
                                       void* pData,
                                       const ICLDevBackendImageService* pImageService)
{
    // workaround for implementing OpenCL 1.2
    mem_desc.dim_count = image_desc.GetDimensionCount();
    switch(image_desc.GetImageType())
    {
    case OpenCL_MEM_OBJECT_IMAGE1D:        mem_desc.memObjType = CL_MEM_OBJECT_IMAGE1D; break;
    case OpenCL_MEM_OBJECT_IMAGE1D_ARRAY:  mem_desc.memObjType = CL_MEM_OBJECT_IMAGE1D_ARRAY; break;
    case OpenCL_MEM_OBJECT_IMAGE1D_BUFFER: mem_desc.memObjType = CL_MEM_OBJECT_IMAGE1D_BUFFER; break;
    case OpenCL_MEM_OBJECT_IMAGE2D:        mem_desc.memObjType = CL_MEM_OBJECT_IMAGE2D; break;
    case OpenCL_MEM_OBJECT_IMAGE2D_ARRAY:  mem_desc.memObjType = CL_MEM_OBJECT_IMAGE2D_ARRAY; break;
    case OpenCL_MEM_OBJECT_IMAGE3D:        mem_desc.memObjType = CL_MEM_OBJECT_IMAGE3D; break;
    default: throw Exception::InvalidArgument("FillMemObjDescriptor:: Unknown image type");
    }
    ImageSizeDesc imSizes = image_desc.GetSizesDesc();
    mem_desc.dimensions.dim[0] = imSizes.width;
    mem_desc.dimensions.dim[1] = imSizes.height;
    mem_desc.dimensions.dim[2] = imSizes.depth;

    if(image_desc.GetImageType() == OpenCL_MEM_OBJECT_IMAGE1D_ARRAY) {
        mem_desc.dimensions.dim[1] = imSizes.array_size;
    }
    if(image_desc.GetImageType() == OpenCL_MEM_OBJECT_IMAGE2D_ARRAY) {
        mem_desc.dimensions.dim[2] = imSizes.array_size;
    }

    mem_desc.pitch[0] = imSizes.row;
    mem_desc.pitch[1] = imSizes.slice;
    ImageChannelDataTypeVal dataType = image_desc.GetImageChannelDataType();
    switch (dataType)
    {
    case OpenCL_SNORM_INT8:         mem_desc.format.image_channel_data_type = CLK_SNORM_INT8; break;
    case OpenCL_SNORM_INT16:        mem_desc.format.image_channel_data_type = CLK_SNORM_INT16; break;
    case OpenCL_UNORM_INT8:         mem_desc.format.image_channel_data_type = CLK_UNORM_INT8; break;
    case OpenCL_UNORM_INT16:        mem_desc.format.image_channel_data_type = CLK_UNORM_INT16; break;
    case OpenCL_UNORM_SHORT_565:    mem_desc.format.image_channel_data_type = CLK_UNORM_SHORT_565; break;
    case OpenCL_UNORM_SHORT_555:    mem_desc.format.image_channel_data_type = CLK_UNORM_SHORT_555; break;
    case OpenCL_UNORM_INT_101010:   mem_desc.format.image_channel_data_type = CLK_UNORM_INT_101010; break;
    case OpenCL_SIGNED_INT8:        mem_desc.format.image_channel_data_type = CLK_SIGNED_INT8; break;
    case OpenCL_SIGNED_INT16:       mem_desc.format.image_channel_data_type = CLK_SIGNED_INT16; break;
    case OpenCL_SIGNED_INT32:       mem_desc.format.image_channel_data_type = CLK_SIGNED_INT32; break;
    case OpenCL_UNSIGNED_INT8:      mem_desc.format.image_channel_data_type = CLK_UNSIGNED_INT8; break;
    case OpenCL_UNSIGNED_INT16:     mem_desc.format.image_channel_data_type = CLK_UNSIGNED_INT16; break;
    case OpenCL_UNSIGNED_INT32:     mem_desc.format.image_channel_data_type = CLK_UNSIGNED_INT32; break;
    case OpenCL_HALF_FLOAT:         mem_desc.format.image_channel_data_type = CLK_HALF_FLOAT; break;
    case OpenCL_FLOAT:              mem_desc.format.image_channel_data_type = CLK_FLOAT; break;
    default: throw Exception::InvalidArgument("FillMemObjDescriptor:: Unknown Image channel data type");
    }
    ImageChannelOrderVal order = image_desc.GetImageChannelOrder();
    switch (order)
    {
    case OpenCL_R:          mem_desc.format.image_channel_order = CLK_R; break;
    case OpenCL_Rx:         mem_desc.format.image_channel_order = CL_Rx; throw; break;
    case OpenCL_A:          mem_desc.format.image_channel_order = CLK_A; break;
    case OpenCL_INTENSITY:  mem_desc.format.image_channel_order = CLK_INTENSITY; break;
    case OpenCL_LUMINANCE:  mem_desc.format.image_channel_order = CLK_LUMINANCE; break;
    case OpenCL_RG:         mem_desc.format.image_channel_order = CLK_RG; break;
    case OpenCL_RGx:        mem_desc.format.image_channel_order = CL_RGx; throw; break;
    case OpenCL_RA:         mem_desc.format.image_channel_order = CLK_RA; break;
    case OpenCL_RGB:        mem_desc.format.image_channel_order = CLK_RGB; break;
    case OpenCL_RGBx:       mem_desc.format.image_channel_order = CL_RGBx; throw; break;
    case OpenCL_RGBA:       mem_desc.format.image_channel_order = CLK_RGBA; break;
    case OpenCL_ARGB:       mem_desc.format.image_channel_order = CLK_ARGB; break;
    case OpenCL_BGRA:       mem_desc.format.image_channel_order = CLK_BGRA; break;
    case OpenCL_DEPTH:      mem_desc.format.image_channel_order = CLK_DEPTH; break;
    case OpenCL_sRGBA:      mem_desc.format.image_channel_order = CLK_sRGBA; break;
    case OpenCL_sBGRA:      mem_desc.format.image_channel_order = CLK_sBGRA; break;
    default: throw Exception::InvalidArgument("FillMemObjDescriptor:: Unknown Image pixel data type");
    }
    mem_desc.uiElementSize = image_desc.GetElementSize();
    mem_desc.pData = pData;

    if(pImageService != NULL)
    {
        /// Allocate data for auxiliary image structures
        _image_aux_data* auxObject = NULL;
        size_t auxObjectSize=pImageService->GetAuxilarySize();
        auxObject = (_image_aux_data*)Validation::align_malloc(auxObjectSize, CPU_DEV_MAXIMUM_ALIGN);
        /// Create auxiliary image structures
        pImageService->CreateImageObject(&mem_desc, auxObject);
    }
}

void OpenCLArgsBuffer::FillArgsBuffer(IBufferContainerList * input)
{
    size_t stLocMemSize = 0;
    size_t offset = 0;
    if (m_kernelNumArgs != input->GetBufferContainer(0)->GetMemoryObjectCount())
    {
        throw Exception::InvalidArgument(std::string("Number of buffers in input data file "
            "do not match to actual number of kernel arguments!"));
    }
    for (unsigned int i = 0; i < m_kernelNumArgs; i++)
    {
        IMemoryObject* pMemObj = input->GetBufferContainer(0)->GetMemoryObject(i);
        void * pData = pMemObj->GetDataPtr();

        if (CL_KRNL_ARG_PTR_IMG_1D == m_pKernelArgs[i].type ||
            CL_KRNL_ARG_PTR_IMG_1D_ARR == m_pKernelArgs[i].type ||
            CL_KRNL_ARG_PTR_IMG_1D_BUF == m_pKernelArgs[i].type ||
            CL_KRNL_ARG_PTR_IMG_2D_ARR == m_pKernelArgs[i].type ||
            CL_KRNL_ARG_PTR_IMG_2D == m_pKernelArgs[i].type ||
            CL_KRNL_ARG_PTR_IMG_2D_DEPTH == m_pKernelArgs[i].type ||
            CL_KRNL_ARG_PTR_IMG_2D_ARR_DEPTH == m_pKernelArgs[i].type ||
            CL_KRNL_ARG_PTR_IMG_3D == m_pKernelArgs[i].type)
        {
            // TODO: This code is almost identical to the next branch. Rewrite it using common function.
            // Kernel argument is an image - need to pass a pointer in the arguments buffer
            ImageDesc imageDesc = GetImageDescription(pMemObj->GetMemoryObjectDesc());
            size_t imageSize = imageDesc.GetSizeInBytes();

            // Kernel execution assumes all buffer arguments are aligned
            // If we do not align the buffer the execution crashes
            auto_ptr_aligned spNewBuffer((char*)align_malloc(imageSize, CPU_DEV_MAXIMUM_ALIGN));
            auto_ptr_aligned spMemDesc((char*)align_malloc(sizeof(cl_mem_obj_descriptor), CPU_DEV_MAXIMUM_ALIGN));

            memcpy(spNewBuffer.get(), (char*)pData, imageSize);
            assert(m_pImageService);
            FillMemObjDescriptor( *((cl_mem_obj_descriptor*)spMemDesc.get()), imageDesc, spNewBuffer.get(), m_pImageService);

            void ** pBufferArg = (void **)(m_pArgsBuffer + offset);
            *pBufferArg = spMemDesc.release();
            spNewBuffer.release();

            offset += sizeof(void *);
        }
        else if ( CL_KRNL_ARG_PTR_GLOBAL <= m_pKernelArgs[i].type )
        {
            // Kernel argument is a buffer - need to pass a pointer in the arguments buffer
            BufferDesc bufferDesc = GetBufferDescription(pMemObj->GetMemoryObjectDesc());
            size_t bufferSize = bufferDesc.GetSizeInBytes();
            
            // Add Padding to both sides of the buffer
            if (m_isCheckOOBAccess) 
            {
                bufferSize += (2*PaddingSize);
            }

            // Kernel execution assumes all buffer arguments are aligned
            // If we do not align the buffer the execution crashes
            auto_ptr_aligned spNewBuffer((char*)align_malloc(bufferSize, CPU_DEV_MAXIMUM_ALIGN));
            auto_ptr_aligned spMemDesc((char*)align_malloc(sizeof(cl_mem_obj_descriptor), CPU_DEV_MAXIMUM_ALIGN));

            if (m_isCheckOOBAccess) 
            {
                char* pPaddedData = spNewBuffer.get();
                std::fill(pPaddedData, pPaddedData + bufferSize, PaddingVal);
                memcpy(pPaddedData + PaddingSize, (char*)pData, bufferSize - 2*PaddingSize);
                FillMemObjDescriptor( *((cl_mem_obj_descriptor*)spMemDesc.get()), bufferDesc, pPaddedData + PaddingSize);
            }
            else
            {
                memcpy(spNewBuffer.get(), (char*)pData, bufferSize);
                FillMemObjDescriptor( *((cl_mem_obj_descriptor*)spMemDesc.get()), bufferDesc, spNewBuffer.get());
            }

            void ** pBufferArg = (void **)(m_pArgsBuffer + offset);
            *pBufferArg = spMemDesc.release();
            spNewBuffer.release();

            offset += sizeof(void *);
        }
        else if (CL_KRNL_ARG_PTR_LOCAL == m_pKernelArgs[i].type)
        {
            // Kernel argument is a local buffer
            // Need to pass pointer to somewhere in local memory buffer

            BufferDesc bufferDesc = GetBufferDescription(pMemObj->GetMemoryObjectDesc());
            size_t origSize = bufferDesc.GetSizeInBytes();
            size_t locSize = ADJUST_SIZE_TO_MAXIMUM_ALIGN(origSize); 
            *(size_t *)(m_pArgsBuffer + offset) = locSize;
            stLocMemSize += locSize;
            offset += sizeof(void *);

            // Values are assigned in CreateExecutableContext
        }
        else if (CL_KRNL_ARG_VECTOR == m_pKernelArgs[i].type || CL_KRNL_ARG_VECTOR_BY_REF == m_pKernelArgs[i].type)
        {
            // Upper part of uiSize is number of element in vector (int2/float4/...)
            // Lower part of uiSize is size of type in vector

            unsigned int uiSize = m_pKernelArgs[i].size_in_bytes;
            uiSize = (uiSize & 0xFFFF) * (uiSize >> 16);

            void* pBufferArg = (void *)(m_pArgsBuffer + offset);
            memcpy(pBufferArg, pData, uiSize);

            offset += uiSize;
        }
        else if (CL_KRNL_ARG_SAMPLER == m_pKernelArgs[i].type)
        {
            // Kernel argument is a sampler
            // Need to pass sampler flags

            cl_int* pBufferArg = (cl_int *)(m_pArgsBuffer + offset);
            *pBufferArg = ((cl_int *)pData)[0];

            offset += sizeof(cl_int);
        }
        else
        {
            // Kernel argument is a simple type (int/float/...)
            // Need to pass the value itself in the arguments buffer

            void* pBufferArg = (void *)(m_pArgsBuffer + offset);
            memcpy(pBufferArg, pData, m_pKernelArgs[i].size_in_bytes);
            offset += m_pKernelArgs[i].size_in_bytes;
        }
    }
}

void OpenCLArgsBuffer::DestroyArgsBuffer()
{
    size_t stLocMemSize = 0;
    size_t offset = 0;
    for (unsigned int i = 0; i < m_kernelNumArgs; i++)
    {
        if (CL_KRNL_ARG_PTR_IMG_2D == m_pKernelArgs[i].type ||
            CL_KRNL_ARG_PTR_IMG_3D == m_pKernelArgs[i].type ||
            CL_KRNL_ARG_PTR_IMG_1D == m_pKernelArgs[i].type ||
            CL_KRNL_ARG_PTR_IMG_1D_BUF == m_pKernelArgs[i].type ||
            CL_KRNL_ARG_PTR_IMG_1D_ARR == m_pKernelArgs[i].type ||
            CL_KRNL_ARG_PTR_IMG_2D_DEPTH == m_pKernelArgs[i].type ||
            CL_KRNL_ARG_PTR_IMG_2D_ARR_DEPTH == m_pKernelArgs[i].type ||
            CL_KRNL_ARG_PTR_IMG_2D_ARR == m_pKernelArgs[i].type)
        {   // images
            void ** pBufferArg = (void **)(m_pArgsBuffer + offset);
            cl_mem_obj_descriptor* pMemDesc = *(cl_mem_obj_descriptor**)pBufferArg;
            align_free(pMemDesc->pData);
            align_free(pMemDesc);

            offset += sizeof(void *);
        }

        else if ( CL_KRNL_ARG_PTR_GLOBAL <= m_pKernelArgs[i].type )
        {
            // Kernel argument is a buffer
            // Need to pass a pointer in the arguments buffer
            void ** pBufferArg = (void **)(m_pArgsBuffer + offset);

            cl_mem_obj_descriptor* pMemDesc = *(cl_mem_obj_descriptor**)pBufferArg;
           
            // Correct pointer in case the buffer is padded 
            void* pBuffer = m_isCheckOOBAccess ? (char*)(pMemDesc->pData) - PaddingSize : pMemDesc->pData;
            align_free(pBuffer);
            align_free(pMemDesc);

            offset += sizeof(void *);
        }
        else if (CL_KRNL_ARG_PTR_LOCAL == m_pKernelArgs[i].type)
        {
            // Kernel argument is a local buffer
            // Need to pass pointer to somewhere in local memory buffer

            size_t origSize = (size_t)*(((void**)(m_pArgsBuffer + offset)));
            size_t locSize = ADJUST_SIZE_TO_MAXIMUM_ALIGN(origSize); 
            stLocMemSize += locSize;
            offset += sizeof(void *);

            // TODO : do we need to delete this?
        }
        else if (CL_KRNL_ARG_VECTOR == m_pKernelArgs[i].type || CL_KRNL_ARG_VECTOR_BY_REF == m_pKernelArgs[i].type)
        {
            // Upper part of uiSize is number of element in vector (int2/float4/...)
            // Lower part of uiSize is size of type in vector

            unsigned int uiSize = m_pKernelArgs[i].size_in_bytes;
            uiSize = (uiSize & 0xFFFF) * (uiSize >> 16);

            offset += uiSize;
        }
        else if (CL_KRNL_ARG_SAMPLER == m_pKernelArgs[i].type)
        {
            // Kernel argument is a sampler
            // Need to pass sampler flags

            offset += sizeof(cl_int);
        }
        else
        {
            // Kernel argument is a simple type (int/float/...)
            // Need to pass the value itself in the arguments buffer
            offset += m_pKernelArgs[i].size_in_bytes;
        }
    }
}

void OpenCLArgsBuffer::CopyOutput(IBufferContainerList &output, const IBufferContainerList *input)
{
    size_t stLocMemSize = 0;
    size_t offset = 0;

    IBufferContainer *bufferContainer = output.CreateBufferContainer();
    IBufferContainer *pOutBC = input->GetBufferContainer(0);
    for (unsigned int i = 0; i < m_kernelNumArgs; i++)
    {
        const IMemoryObjectDesc * pMemObjDesc = pOutBC->GetMemoryObject(i)->GetMemoryObjectDesc();
        if ( CL_KRNL_ARG_PTR_IMG_1D == m_pKernelArgs[i].type ||
            CL_KRNL_ARG_PTR_IMG_1D_ARR == m_pKernelArgs[i].type ||
            CL_KRNL_ARG_PTR_IMG_1D_BUF == m_pKernelArgs[i].type ||
            CL_KRNL_ARG_PTR_IMG_2D_ARR == m_pKernelArgs[i].type ||
            CL_KRNL_ARG_PTR_IMG_2D == m_pKernelArgs[i].type ||
            CL_KRNL_ARG_PTR_IMG_2D_DEPTH == m_pKernelArgs[i].type ||
            CL_KRNL_ARG_PTR_IMG_2D_ARR_DEPTH == m_pKernelArgs[i].type ||
            CL_KRNL_ARG_PTR_IMG_3D == m_pKernelArgs[i].type)
        {
            // Kernel argument is a image
            // Need to pass a pointer in the arguments buffer
            ImageDesc imageDesc = GetImageDescription(pMemObjDesc);
            IMemoryObject * buffer = bufferContainer->CreateImage(imageDesc);
            void * pData = buffer->GetDataPtr();

            size_t imageSize = imageDesc.GetSizeInBytes();

            void ** pImageArg = (void **)(m_pArgsBuffer + offset);
            cl_mem_obj_descriptor* pMemDesc = *(cl_mem_obj_descriptor**)pImageArg;
            void* pImageArgData = pMemDesc->pData;

            memcpy(pData, pImageArgData, imageSize);

            offset += sizeof(void*);
            continue;
        }

        BufferDesc bufferDesc = GetBufferDescription(pMemObjDesc);
        IMemoryObject * buffer = bufferContainer->CreateBuffer(bufferDesc);
        void * pData = buffer->GetDataPtr();

        if ( CL_KRNL_ARG_PTR_GLOBAL <= m_pKernelArgs[i].type )
        {
            // Kernel argument is a buffer
            // Need to pass a pointer in the arguments buffer

            size_t bufferSize = bufferDesc.GetSizeInBytes();

            void ** pBufferArg = (void **)(m_pArgsBuffer + offset);
            cl_mem_obj_descriptor* pMemDesc = *(cl_mem_obj_descriptor**)pBufferArg;
            void* pBufferArgData = pMemDesc->pData;

            if (m_isCheckOOBAccess) 
            {
                TypeDesc bufferElemDesc = bufferDesc.GetElementDescription();
                // if vector has 3 components, it is actually includes 4 components, so we need to skip the checking of forth element
                size_t vec3align = 0;
                if(bufferElemDesc.GetType() == TVECTOR) {
                    if( bufferDesc.SizeOfVector() == 3) {
                        vec3align = bufferDesc.GetElementDescription().GetSubTypeDesc(0).GetSizeInBytes();
                    }
                }

                // Check for mutations
                if ( (std::find_if( (char*)pBufferArgData - PaddingSize, 
                                    (char*)pBufferArgData, 
                                    std::bind2nd(std::not_equal_to<char>(), PaddingVal)) != (char*)pBufferArgData) 
                        ||
                     (std::find_if( (char*)pBufferArgData + bufferSize + vec3align, 
                                    (char*)pBufferArgData + bufferSize + PaddingSize, 
                                    std::bind2nd(std::not_equal_to<char>(), PaddingVal)) != (char*)pBufferArgData + bufferSize + PaddingSize) )
                {
                    throw Exception::OutOfRange("Padding was mutated!");
                }
            }

            memcpy(pData, pBufferArgData, bufferSize);

            offset += sizeof(void*);
        }
        else if (CL_KRNL_ARG_PTR_LOCAL == m_pKernelArgs[i].type)
        {
            // Kernel argument is a local buffer
            // Need to pass pointer to somewhere in local memory buffer

            size_t origSize = (size_t)*(((void**)(m_pArgsBuffer + offset)));
            size_t locSize = ADJUST_SIZE_TO_MAXIMUM_ALIGN(origSize); 
            stLocMemSize += locSize;
            offset += sizeof(void*);
            // TODO : assign value
        }
        else if (CL_KRNL_ARG_VECTOR == m_pKernelArgs[i].type || CL_KRNL_ARG_VECTOR_BY_REF == m_pKernelArgs[i].type)
        {
            // Upper part of uiSize is number of element in vector (int2/float4/...)
            // Lower part of uiSize is size of type in vector

            unsigned int uiSize = m_pKernelArgs[i].size_in_bytes;
            uiSize = (uiSize & 0xFFFF) * (uiSize >> 16);

            void* pBufferArg = (void *)(m_pArgsBuffer + offset);
            memcpy(pData, pBufferArg, uiSize);

            offset += uiSize;
        }
        else if (CL_KRNL_ARG_SAMPLER == m_pKernelArgs[i].type)
        {
            // Kernel argument is a sampler
            // Need to pass sampler flags

            cl_int* pBufferArg = (cl_int *)(m_pArgsBuffer + offset);
            ((cl_int *)pData)[0] = *pBufferArg;

            offset += sizeof(cl_int);
        }
        else
        {
            // Kernel argument is a simple type (int/float/...)
            // Need to pass the value itself in the arguments buffer

            void* pBufferArg = (void *)(m_pArgsBuffer + offset);
            memcpy(pData, pBufferArg, m_pKernelArgs[i].size_in_bytes);
            offset += m_pKernelArgs[i].size_in_bytes;
        }
    }
}

size_t OpenCLArgsBuffer::CalcArgsBufferSize()
{
    size_t bufferSize = 0;

    for (unsigned int i =0; i < m_kernelNumArgs; i++)
    {
        if ( CL_KRNL_ARG_PTR_GLOBAL <= m_pKernelArgs[i].type )
        {
            // Kernel argument is a buffer
            // Need to pass a pointer in the arguments buffer
            bufferSize += sizeof(void *);
        }
        else if (CL_KRNL_ARG_PTR_LOCAL == m_pKernelArgs[i].type)
        {
            // Kernel argument is a local buffer
            // Need to pass pointer to somewhere in local memory buffer
            bufferSize += sizeof(void *);
        }
        else if (CL_KRNL_ARG_VECTOR == m_pKernelArgs[i].type || CL_KRNL_ARG_VECTOR_BY_REF == m_pKernelArgs[i].type)
        {
            // Kernel argument is a vector
            // Need to pass all the vector in the argument buffer

            unsigned int uiSize = m_pKernelArgs[i].size_in_bytes;
            // Upper part of uiSize is number of element in vector (int2/float4/...)
            // Lower part of uiSize is size of type in vector
            uiSize = (uiSize & 0xFFFF) * (uiSize >> 16);
            bufferSize += uiSize;
        }
        else if (CL_KRNL_ARG_SAMPLER == m_pKernelArgs[i].type)
        {
            // Kernel argument is a sampler
            // Need to pass sampler flags
            bufferSize += sizeof(cl_int);
        }
        else
        {
            // Kernel argument is a simple type (int/float/...)
            // Need to pass the value itself in the arguments buffer
            bufferSize += m_pKernelArgs[i].size_in_bytes;
        }
    }

    return bufferSize;
}
