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

#include "VectorWidth.h"

using namespace Validation;

VectorWidthWrapper::VectorWidthMetadataMap
    VectorWidthWrapper::m_metaData(initStaticMap());

VectorWidthWrapper::VectorWidthMetadataMap VectorWidthWrapper::initStaticMap() {
  VectorWidthMetadataMap metaData;
  metaData[V1] = VectorWidthWrapper::VectorWidthMetadata(1, "v1");
  metaData[V2] = VectorWidthWrapper::VectorWidthMetadata(2, "v2");
  metaData[V3] = VectorWidthWrapper::VectorWidthMetadata(3, "v3");
  metaData[V4] = VectorWidthWrapper::VectorWidthMetadata(4, "v4");
  metaData[V8] = VectorWidthWrapper::VectorWidthMetadata(8, "v8");
  metaData[V16] = VectorWidthWrapper::VectorWidthMetadata(16, "v16");
  return metaData;
}
