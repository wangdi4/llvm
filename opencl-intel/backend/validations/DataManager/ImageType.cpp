/*****************************************************************************\

Copyright (c) Intel Corporation (2013).

INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
including liability for infringement of any proprietary rights, relating to
use of the code. No license, express or implied, by estoppels or otherwise,
to any intellectual property rights is granted herein.

File Name:  ImageType.cpp

\*****************************************************************************/
#include "llvm/Support/DataTypes.h"

#include "ImageType.h"
using namespace Validation;

ImageTypeValWrapper::ImageTypeValMetadataMap 
ImageTypeValWrapper::m_metaData(initStaticMap());

ImageTypeValWrapper::ImageTypeValMetadataMap 
ImageTypeValWrapper::initStaticMap()
{
    ImageTypeValMetadataMap metaData;
    metaData[OpenCL_MEM_OBJECT_IMAGE1D              ] = ImageTypeValMetadata(1,     false,    "CL_MEM_OBJECT_IMAGE1D");
    metaData[OpenCL_MEM_OBJECT_IMAGE1D_BUFFER       ] = ImageTypeValMetadata(1,     false,    "CL_MEM_OBJECT_IMAGE1D_BUFFER");
    metaData[OpenCL_MEM_OBJECT_IMAGE1D_ARRAY        ] = ImageTypeValMetadata(2,     true,     "CL_MEM_OBJECT_IMAGE1D_ARRAY");
    metaData[OpenCL_MEM_OBJECT_IMAGE2D              ] = ImageTypeValMetadata(2,     false,    "CL_MEM_OBJECT_IMAGE2D");
    metaData[OpenCL_MEM_OBJECT_IMAGE2D_ARRAY        ] = ImageTypeValMetadata(3,     true,     "CL_MEM_OBJECT_IMAGE2D_ARRAY");
    metaData[OpenCL_MEM_OBJECT_IMAGE3D              ] = ImageTypeValMetadata(3,     false,    "CL_MEM_OBJECT_IMAGE3D");

    metaData[UNSPECIFIED_MEM_OBJECT_IMAGE           ] = ImageTypeValMetadata(0,     false,    "UNSPECIFIED_MEM_OBJECT_IMAGE");
    metaData[INVALID_MEM_OBJECT_IMAGE               ] = ImageTypeValMetadata(0,     false,    "INVALID_MEM_OBJECT_IMAGE");
    return metaData;
}
