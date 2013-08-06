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
#include "llvm/Support/DataTypes.h"

#include "ImageChannelOrder.h"
using namespace Validation;

ImageChannelOrderValWrapper::ImageChannelOrderMetadataMap 
ImageChannelOrderValWrapper::m_metaData(initStaticMap());


ImageChannelOrderValWrapper::ImageChannelOrderMetadataMap 
ImageChannelOrderValWrapper::initStaticMap()
{
    ImageChannelOrderMetadataMap metaData;
    metaData[OpenCL_R]                    = ImageChannelOrderMetadata(1, "CL_R");
    metaData[OpenCL_Rx]                   = ImageChannelOrderMetadata(1, "CL_Rx");
    metaData[OpenCL_A]                    = ImageChannelOrderMetadata(1, "CL_A");
    metaData[OpenCL_INTENSITY]            = ImageChannelOrderMetadata(1, "CL_INTENSITY");
    metaData[OpenCL_LUMINANCE]            = ImageChannelOrderMetadata(1, "CL_LUMINANCE");
    metaData[OpenCL_RG]                   = ImageChannelOrderMetadata(2, "CL_RG");
    metaData[OpenCL_RGx]                  = ImageChannelOrderMetadata(2, "CL_RGx");
    metaData[OpenCL_RA]                   = ImageChannelOrderMetadata(2, "CL_RA");
    metaData[OpenCL_RGB]                  = ImageChannelOrderMetadata(3, "CL_RGB");
    metaData[OpenCL_RGBx]                 = ImageChannelOrderMetadata(3, "CL_RGBx"); 
    metaData[OpenCL_RGBA]                 = ImageChannelOrderMetadata(4, "CL_RGBA");
    metaData[OpenCL_ARGB]                 = ImageChannelOrderMetadata(4, "CL_ARGB");
    metaData[OpenCL_BGRA]                 = ImageChannelOrderMetadata(4, "CL_BGRA");
    metaData[OpenCL_DEPTH]                = ImageChannelOrderMetadata(1, "CL_DEPTH");
    metaData[OpenCL_sRGBA]                = ImageChannelOrderMetadata(4, "CL_sRGBA");
    metaData[OpenCL_sBGRA]                = ImageChannelOrderMetadata(4, "CL_sBGRA");
    metaData[UNSPECIFIED_CHANNEL_ORDER]   = ImageChannelOrderMetadata(1, "UNSPECIFIED_CHANNEL_ORDER");
    metaData[INVALID_CHANNEL_ORDER]       = ImageChannelOrderMetadata(1, "INVALID_CHANNEL_ORDER");
    return metaData;
}
