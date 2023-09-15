// INTEL CONFIDENTIAL
//
// Copyright 2013 Intel Corporation.
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

#include "ImageType.h"
#include "llvm/Support/DataTypes.h"

using namespace Validation;

ImageTypeValWrapper::ImageTypeValMetadataMap
    ImageTypeValWrapper::m_metaData(initStaticMap());

ImageTypeValWrapper::ImageTypeValMetadataMap
ImageTypeValWrapper::initStaticMap() {
  ImageTypeValMetadataMap metaData;
  metaData[OpenCL_MEM_OBJECT_IMAGE1D] =
      ImageTypeValMetadata(1, false, "CL_MEM_OBJECT_IMAGE1D");
  metaData[OpenCL_MEM_OBJECT_IMAGE1D_BUFFER] =
      ImageTypeValMetadata(1, false, "CL_MEM_OBJECT_IMAGE1D_BUFFER");
  metaData[OpenCL_MEM_OBJECT_IMAGE1D_ARRAY] =
      ImageTypeValMetadata(2, true, "CL_MEM_OBJECT_IMAGE1D_ARRAY");
  metaData[OpenCL_MEM_OBJECT_IMAGE2D] =
      ImageTypeValMetadata(2, false, "CL_MEM_OBJECT_IMAGE2D");
  metaData[OpenCL_MEM_OBJECT_IMAGE2D_ARRAY] =
      ImageTypeValMetadata(3, true, "CL_MEM_OBJECT_IMAGE2D_ARRAY");
  metaData[OpenCL_MEM_OBJECT_IMAGE3D] =
      ImageTypeValMetadata(3, false, "CL_MEM_OBJECT_IMAGE3D");

  metaData[UNSPECIFIED_MEM_OBJECT_IMAGE] =
      ImageTypeValMetadata(0, false, "UNSPECIFIED_MEM_OBJECT_IMAGE");
  metaData[INVALID_MEM_OBJECT_IMAGE] =
      ImageTypeValMetadata(0, false, "INVALID_MEM_OBJECT_IMAGE");
  return metaData;
}
