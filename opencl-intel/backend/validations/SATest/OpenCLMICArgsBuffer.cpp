/*****************************************************************************\

Copyright (c) Intel Corporation (2010, 2011).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  OpenCLMICArgsBuffer.cpp

\*****************************************************************************/
#include "OpenCLMICArgsBuffer.h"
#include "Buffer.h"
#include "Image.h"
#include "SATestException.h"
#include "Exception.h"

#include "mem_utils.h"
#include "MICNative/mic_dev_limits.h"

#define DEBUG_TYPE "OpenCLMICArgsBuffer"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"

#define CHECK_COI_RESULT(_COIFUNC)                                                  \
    {                                                                               \
    COIRESULT result = _COIFUNC;                                                    \
    if (COI_SUCCESS != result)                                                      \
    {                                                                               \
        std::cerr << #_COIFUNC" retruned " << COIResultGetName(result) << std::endl;\
    }                                                                               \
}

using namespace Validation;

OpenCLMICArgsBuffer::OpenCLMICArgsBuffer(const cl_kernel_argument * pKernelArgs, 
                                   cl_uint kernelNumArgs, 
                                   IBufferContainerList * input,
                                   DispatcherData* dispatcher,
                                   COIBuffersWrapper& coiBuffers,
                                   const COIPROCESS& deviceProcess):
m_pKernelArgs(pKernelArgs), m_kernelNumArgs(kernelNumArgs), m_dispatcher(dispatcher)
{
    m_argsBufferSize = CalcArgsBufferSize();

    m_dispatcher->kernelArgSize = m_argsBufferSize;
    m_pArgsBuffer = new uint8_t[m_argsBufferSize];
    FillArgsBuffer(input, coiBuffers, deviceProcess);
}

OpenCLMICArgsBuffer::~OpenCLMICArgsBuffer(void)
{
    delete [] m_pArgsBuffer;
}

uint8_t* OpenCLMICArgsBuffer::GetArgsBuffer()
{
    return m_pArgsBuffer;
}

size_t OpenCLMICArgsBuffer::GetArgsBufferSize()
{
    return m_argsBufferSize;
}

// TODO: Share one implementation between OpenCLMICArgsBuffer and OpenCLArgsBuffer.
void Validation::FillMemObjDescriptor( cl_mem_obj_descriptor& mem_desc, const BufferDesc& buffer_desc, void* pData)
{
    mem_desc.dim_count = 1;
    mem_desc.dimensions.dim[0] = buffer_desc.GetBufferSizeInBytes();
    mem_desc.pData = pData;
}

void Validation::FillMemObjDescriptor( cl_mem_obj_descriptor& mem_desc, const ImageDesc& image_desc, void* pData)
{
    mem_desc.dim_count = image_desc.GetNumOfDimensions();
    ImageSizes imSizes = image_desc.GetSizes();
    mem_desc.dimensions.dim[0] = imSizes.width;
    mem_desc.dimensions.dim[1] = imSizes.height;
    mem_desc.dimensions.dim[2] = imSizes.depth;
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
    default: throw Exception::InvalidArgument("FillMemObjDescriptor:: Unknown Image pixel data type");
    }
    mem_desc.uiElementSize = image_desc.GetElementSize();
    mem_desc.pData = pData;
}

void OpenCLMICArgsBuffer::FillArgsBuffer(IBufferContainerList * input,
                                         COIBuffersWrapper& coiBuffers,
                                         const COIPROCESS& deviceProcess)
{
    size_t stLocMemSize = 0;
    size_t offset = 0;
    for (unsigned int i = 0; i < m_kernelNumArgs; i++)
    {
        IMemoryObject* pMemObj = input->GetBufferContainer(0)->GetMemoryObject(i);
        void * pData = pMemObj->GetDataPtr();

        if (CL_KRNL_ARG_PTR_IMG_2D == m_pKernelArgs[i].type || CL_KRNL_ARG_PTR_IMG_3D == m_pKernelArgs[i].type)
        {
            DEBUG(llvm::dbgs()<< "Filling image data.\n");
            // TODO: This code is almost identical to the next branch. Rewrite it using common function.
            // Kernel argument is an image - need to pass a pointer in the arguments buffer
            ImageDesc imageDesc = GetImageDescription(pMemObj->GetMemoryObjectDesc());
            size_t imageSize = imageDesc.GetImageSizeInBytes();

            BufferDirective buffDirective;
            buffDirective.bufferIndex = coiBuffers.GetNumberOfBuffers();
            buffDirective.offset_in_blob = offset;

            FillMemObjDescriptor( buffDirective.mem_obj_desc, imageDesc, NULL);

            // Create buffer with dispatcher data for all kernels.

            coiBuffers.CreateBufferFromMemory(imageSize, deviceProcess, COI_SINK_WRITE, pData);
            m_directives.push_back(buffDirective);

            offset += sizeof(void *);
        }
        else if ( CL_KRNL_ARG_PTR_GLOBAL == m_pKernelArgs[i].type || CL_KRNL_ARG_PTR_CONST == m_pKernelArgs[i].type )
        {
            DEBUG(llvm::dbgs()<< "Filling pointer data.\n");
            // Kernel argument is a buffer - need to pass a pointer in the arguments buffer
            BufferDesc bufferDesc = GetBufferDescription(pMemObj->GetMemoryObjectDesc());
            size_t bufferSize = bufferDesc.GetBufferSizeInBytes();

            BufferDirective buffDirective;
            buffDirective.bufferIndex = coiBuffers.GetNumberOfBuffers();
            buffDirective.offset_in_blob = offset;

            FillMemObjDescriptor( buffDirective.mem_obj_desc, bufferDesc, NULL);

            // Create buffer with dispatcher data for all kernels.
            if (CL_KRNL_ARG_PTR_GLOBAL == m_pKernelArgs[i].type)
            {
                // pointer to the global space - we can write
                coiBuffers.CreateBufferFromMemory(bufferSize, deviceProcess, COI_SINK_WRITE, pData);
            }
            else
            {
                // pointer to the constant space - we can only read
                coiBuffers.CreateBufferFromMemory(bufferSize, deviceProcess, COI_SINK_READ, pData);
            }

            m_directives.push_back(buffDirective);
            DEBUG(llvm::dbgs() << "Buffer size = " << bufferSize << "Buffer index = " << 
                m_directives[m_directives.size() - 1].bufferIndex << "\tOffset in blob = " << 
                m_directives[m_directives.size() - 1].offset_in_blob << "\n");

            offset += sizeof(void *);
        }
        else if (CL_KRNL_ARG_PTR_LOCAL == m_pKernelArgs[i].type)
        {
            // Kernel argument is a local buffer
            // Need to pass pointer to somewhere in local memory buffer

            BufferDesc bufferDesc = GetBufferDescription(pMemObj->GetMemoryObjectDesc());
            size_t origSize = bufferDesc.GetBufferSizeInBytes();
            size_t locSize = ADJUST_SIZE_TO_MAXIMUM_ALIGN(origSize); 
            *(size_t *)(m_pArgsBuffer + offset) = locSize;
            stLocMemSize += locSize;
            offset += sizeof(void *);

            // Values are assigned in CreateExecutableContext
        }
        else if (CL_KRNL_ARG_VECTOR == m_pKernelArgs[i].type)
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

void OpenCLMICArgsBuffer::CopyOutput(IRunResult * runResult, IBufferContainerList * input, const char* kernelName)
{
    size_t stLocMemSize = 0;
    size_t offset = 0;

    IBufferContainer * bufferContainer = runResult->GetOutput(kernelName).CreateBufferContainer();
    IBufferContainer * pOutBC = input->GetBufferContainer(0);
    for (unsigned int i = 0; i < m_kernelNumArgs; i++)
    {
        const IMemoryObjectDesc * pMemObjDesc = pOutBC->GetMemoryObject(i)->GetMemoryObjectDesc();
        if ( CL_KRNL_ARG_PTR_IMG_2D == m_pKernelArgs[i].type || CL_KRNL_ARG_PTR_IMG_3D == m_pKernelArgs[i].type )
        {
            // Kernel argument is a image
            // Need to pass a pointer in the arguments buffer
            ImageDesc imageDesc = GetImageDescription(pMemObjDesc);
            IMemoryObject * buffer = bufferContainer->CreateImage(imageDesc);
            void * pData = buffer->GetDataPtr();

            size_t imageSize = imageDesc.GetImageSizeInBytes();

            void* pImageArgData = pOutBC->GetMemoryObject(i)->GetDataPtr();

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

            size_t bufferSize = bufferDesc.GetBufferSizeInBytes();

            void* pBufferArgData = pOutBC->GetMemoryObject(i)->GetDataPtr();

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
        else if (CL_KRNL_ARG_VECTOR == m_pKernelArgs[i].type)
        {
            void* pBufferArgData = pOutBC->GetMemoryObject(i)->GetDataPtr();

            unsigned int uiSize = m_pKernelArgs[i].size_in_bytes;
            uiSize = (uiSize & 0xFFFF) * (uiSize >> 16);

            memcpy(pData, pBufferArgData, bufferDesc.GetBufferSizeInBytes());

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

size_t OpenCLMICArgsBuffer::CalcArgsBufferSize()
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
        else if (CL_KRNL_ARG_VECTOR == m_pKernelArgs[i].type)
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
