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

File Name:  ImageChannelOrder.cpp

\*****************************************************************************/
#include "llvm/System/DataTypes.h"

#include "ImageChannelOrder.h"
using namespace Validation;

std::map<ImageChannelOrderVal, ImageChannelOrderValWrapper::ImageChannelOrderMetadata> ImageChannelOrderValWrapper::m_metaData;

bool ImageChannelOrderValWrapper::m_isStaticInit = false;

void ImageChannelOrderValWrapper::initStatic()
{
    // TODO: not thread safe
    m_metaData.clear();

    m_metaData[OpenCL_R]                    = ImageChannelOrderMetadata(1, "CL_R");
    m_metaData[OpenCL_Rx]                   = ImageChannelOrderMetadata(1, "CL_Rx");
    m_metaData[OpenCL_A]                    = ImageChannelOrderMetadata(1, "CL_A");
    m_metaData[OpenCL_INTENSITY]            = ImageChannelOrderMetadata(1, "CL_INTENSITY");
    m_metaData[OpenCL_LUMINANCE]            = ImageChannelOrderMetadata(1, "CL_LUMINANCE");
    m_metaData[OpenCL_RG]                   = ImageChannelOrderMetadata(2, "CL_RG");
    m_metaData[OpenCL_RGx]                  = ImageChannelOrderMetadata(2, "CL_RGx");
    m_metaData[OpenCL_RA]                   = ImageChannelOrderMetadata(2, "CL_RA");
    m_metaData[OpenCL_RGB]                  = ImageChannelOrderMetadata(3, "CL_RGB");
    m_metaData[OpenCL_RGBx]                 = ImageChannelOrderMetadata(3, "CL_RGBx"); 
    m_metaData[OpenCL_RGBA]                 = ImageChannelOrderMetadata(4, "CL_RGBA");
    m_metaData[OpenCL_ARGB]                 = ImageChannelOrderMetadata(4, "CL_ARGB");
    m_metaData[OpenCL_BGRA]                 = ImageChannelOrderMetadata(4, "CL_BGRA");
    m_metaData[UNSPECIFIED_CHANNEL_ORDER]   = ImageChannelOrderMetadata(1, "UNSPECIFIED_CHANNEL_ORDER");
    m_metaData[INVALID_CHANNEL_ORDER]       = ImageChannelOrderMetadata(1, "INVALID_CHANNEL_ORDER");

    m_isStaticInit = true;

}
