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

#include "ImageChannelDataType.h"
#include "dxfloat.h" // for CFloat16
#include "llvm/Support/DataTypes.h"

using namespace Validation;

ImageChannelDataTypeValWrapper::ImageChannelDataTypeMetadataMap
    ImageChannelDataTypeValWrapper::m_metaData(initStaticMap());

ImageChannelDataTypeValWrapper::ImageChannelDataTypeMetadataMap
ImageChannelDataTypeValWrapper::initStaticMap() {
  ImageChannelDataTypeMetadataMap metaData;
  metaData[OpenCL_SNORM_INT8] =
      ImageChannelDataTypeMetadata(sizeof(int8_t), false, "CL_SNORM_INT8");
  metaData[OpenCL_SNORM_INT16] =
      ImageChannelDataTypeMetadata(sizeof(int16_t), false, "CL_SNORM_INT16");
  metaData[OpenCL_UNORM_INT8] =
      ImageChannelDataTypeMetadata(sizeof(int8_t), false, "CL_UNORM_INT8");
  metaData[OpenCL_UNORM_INT16] =
      ImageChannelDataTypeMetadata(sizeof(int16_t), false, "CL_UNORM_INT16");
  metaData[OpenCL_UNORM_SHORT_565] = ImageChannelDataTypeMetadata(
      sizeof(int16_t), false, "CL_UNORM_SHORT_565");
  metaData[OpenCL_UNORM_SHORT_555] = ImageChannelDataTypeMetadata(
      sizeof(int16_t), false, "CL_UNORM_SHORT_555");
  metaData[OpenCL_UNORM_INT_101010] = ImageChannelDataTypeMetadata(
      sizeof(int32_t), false, "CL_UNORM_INT_101010");
  metaData[OpenCL_SIGNED_INT8] =
      ImageChannelDataTypeMetadata(sizeof(int8_t), false, "CL_SIGNED_INT8");
  metaData[OpenCL_SIGNED_INT16] =
      ImageChannelDataTypeMetadata(sizeof(int16_t), false, "CL_SIGNED_INT16");
  metaData[OpenCL_SIGNED_INT32] =
      ImageChannelDataTypeMetadata(sizeof(int32_t), false, "CL_SIGNED_INT32");
  metaData[OpenCL_UNSIGNED_INT8] =
      ImageChannelDataTypeMetadata(sizeof(uint8_t), false, "CL_UNSIGNED_INT8");
  metaData[OpenCL_UNSIGNED_INT16] = ImageChannelDataTypeMetadata(
      sizeof(uint16_t), false, "CL_UNSIGNED_INT16");
  metaData[OpenCL_UNSIGNED_INT32] = ImageChannelDataTypeMetadata(
      sizeof(uint32_t), false, "CL_UNSIGNED_INT32");
  metaData[OpenCL_HALF_FLOAT] =
      ImageChannelDataTypeMetadata(sizeof(CFloat16), true, "CL_HALF_FLOAT");
  metaData[OpenCL_FLOAT] =
      ImageChannelDataTypeMetadata(sizeof(float), true, "CL_FLOAT");
  metaData[UNSPECIFIED_IMAGE_DATA_TYPE] =
      ImageChannelDataTypeMetadata(sizeof(int), false, "UNSPECIFIED_DATA_TYPE");
  metaData[INVALID_IMAGE_DATA_TYPE] = ImageChannelDataTypeMetadata(
      sizeof(int), false, "INVALID_IMAGE_DATA_TYPE");
  return metaData;
}
