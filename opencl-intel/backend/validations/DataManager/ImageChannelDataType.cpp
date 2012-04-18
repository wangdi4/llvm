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

File Name:  ImageChannelDataType.cpp

\*****************************************************************************/
#include "llvm/Support/DataTypes.h"

#include "ImageChannelDataType.h"
#include "dxfloat.h"        // for CFloat16
using namespace Validation;

ImageChannelDataTypeValWrapper::ImageChannelDataTypeMetadataMap 
ImageChannelDataTypeValWrapper::m_metaData(initStaticMap());

ImageChannelDataTypeValWrapper::ImageChannelDataTypeMetadataMap 
ImageChannelDataTypeValWrapper::initStaticMap()
{
    ImageChannelDataTypeMetadataMap metaData;
    metaData[OpenCL_SNORM_INT8            ] = ImageChannelDataTypeMetadata(sizeof(int8_t),    false,  "CL_SNORM_INT8");
    metaData[OpenCL_SNORM_INT16           ] = ImageChannelDataTypeMetadata(sizeof(int16_t),   false,  "CL_SNORM_INT16");
    metaData[OpenCL_UNORM_INT8            ] = ImageChannelDataTypeMetadata(sizeof(int8_t),    false,  "CL_UNORM_INT8");
    metaData[OpenCL_UNORM_INT16           ] = ImageChannelDataTypeMetadata(sizeof(int16_t),   false,  "CL_UNORM_INT16");
    metaData[OpenCL_UNORM_SHORT_565       ] = ImageChannelDataTypeMetadata(sizeof(int16_t),   false,  "CL_UNORM_SHORT_565");
    metaData[OpenCL_UNORM_SHORT_555       ] = ImageChannelDataTypeMetadata(sizeof(int16_t),   false,  "CL_UNORM_SHORT_555");
    metaData[OpenCL_UNORM_INT_101010      ] = ImageChannelDataTypeMetadata(sizeof(int32_t),   false,  "CL_UNORM_INT_101010");
    metaData[OpenCL_SIGNED_INT8           ] = ImageChannelDataTypeMetadata(sizeof(int8_t),    false,  "CL_SIGNED_INT8");
    metaData[OpenCL_SIGNED_INT16          ] = ImageChannelDataTypeMetadata(sizeof(int16_t),   false,  "CL_SIGNED_INT16");
    metaData[OpenCL_SIGNED_INT32          ] = ImageChannelDataTypeMetadata(sizeof(int32_t),   false,  "CL_SIGNED_INT32"); 
    metaData[OpenCL_UNSIGNED_INT8         ] = ImageChannelDataTypeMetadata(sizeof(uint8_t),   false,  "CL_UNSIGNED_INT8");
    metaData[OpenCL_UNSIGNED_INT16        ] = ImageChannelDataTypeMetadata(sizeof(uint16_t),  false,  "CL_UNSIGNED_INT16");
    metaData[OpenCL_UNSIGNED_INT32        ] = ImageChannelDataTypeMetadata(sizeof(uint32_t),  false,  "CL_UNSIGNED_INT32");
    metaData[OpenCL_HALF_FLOAT            ] = ImageChannelDataTypeMetadata(sizeof(CFloat16),  true,   "CL_HALF_FLOAT");
    metaData[OpenCL_FLOAT                 ] = ImageChannelDataTypeMetadata(sizeof(float),     true,   "CL_FLOAT");
    metaData[UNSPECIFIED_IMAGE_DATA_TYPE  ] = ImageChannelDataTypeMetadata(sizeof(int),       false,  "UNSPECIFIED_DATA_TYPE");
    metaData[INVALID_IMAGE_DATA_TYPE      ] = ImageChannelDataTypeMetadata(sizeof(int),       false,  "INVALID_IMAGE_DATA_TYPE");
    return metaData;
}
