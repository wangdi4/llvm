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

#include "ImageChannelOrder.h"
#include "llvm/Support/DataTypes.h"

using namespace Validation;

ImageChannelOrderValWrapper::ImageChannelOrderMetadataMap
    ImageChannelOrderValWrapper::m_metaData(initStaticMap());

ImageChannelOrderValWrapper::ImageChannelOrderMetadataMap
ImageChannelOrderValWrapper::initStaticMap() {
  ImageChannelOrderMetadataMap metaData;
  metaData[OpenCL_R] = ImageChannelOrderMetadata(1, "CL_R");
  metaData[OpenCL_Rx] = ImageChannelOrderMetadata(1, "CL_Rx");
  metaData[OpenCL_A] = ImageChannelOrderMetadata(1, "CL_A");
  metaData[OpenCL_INTENSITY] = ImageChannelOrderMetadata(1, "CL_INTENSITY");
  metaData[OpenCL_LUMINANCE] = ImageChannelOrderMetadata(1, "CL_LUMINANCE");
  metaData[OpenCL_RG] = ImageChannelOrderMetadata(2, "CL_RG");
  metaData[OpenCL_RGx] = ImageChannelOrderMetadata(2, "CL_RGx");
  metaData[OpenCL_RA] = ImageChannelOrderMetadata(2, "CL_RA");
  metaData[OpenCL_RGB] = ImageChannelOrderMetadata(3, "CL_RGB");
  metaData[OpenCL_RGBx] = ImageChannelOrderMetadata(3, "CL_RGBx");
  metaData[OpenCL_RGBA] = ImageChannelOrderMetadata(4, "CL_RGBA");
  metaData[OpenCL_ARGB] = ImageChannelOrderMetadata(4, "CL_ARGB");
  metaData[OpenCL_BGRA] = ImageChannelOrderMetadata(4, "CL_BGRA");
  metaData[OpenCL_DEPTH] = ImageChannelOrderMetadata(1, "CL_DEPTH");
  metaData[OpenCL_sRGBA] = ImageChannelOrderMetadata(4, "CL_sRGBA");
  metaData[OpenCL_sBGRA] = ImageChannelOrderMetadata(4, "CL_sBGRA");
  metaData[UNSPECIFIED_CHANNEL_ORDER] =
      ImageChannelOrderMetadata(1, "UNSPECIFIED_CHANNEL_ORDER");
  metaData[INVALID_CHANNEL_ORDER] =
      ImageChannelOrderMetadata(1, "INVALID_CHANNEL_ORDER");
  return metaData;
}
