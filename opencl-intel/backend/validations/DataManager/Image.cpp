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

#include "Image.h"
#include "IContainerVisitor.h"
#include "llvm/Support/DataTypes.h"

namespace Validation {

///////////////////////////////////
/// Image implementation
///////////////////////////////////////
void Image::AllocateMemoryForData() {
  // Compute size of memory to allocate
  std::size_t buffSize = m_desc.GetSizeInBytes();
  // Allocate memory
  m_data = new uint8_t[buffSize];
}

Image::Image(const ImageDesc &desc) : m_desc(desc) {
  // Allocate memory
  AllocateMemoryForData();
}

Image::~Image() {
  if (0 != m_data) {
    delete[] m_data;
  }
}

void Image::Accept(IContainerVisitor &visitor) const {
  visitor.visitImage(this);
}

ImageDesc GetImageDescription(const IMemoryObjectDesc *iDesc) {
  const ImageDesc *desc = static_cast<const ImageDesc *>(iDesc);
  if (NULL != desc)
    return *desc;
  else
    throw Exception::InvalidArgument("ImageDesc is expected");
}

ImageTypeVal GetImageTypeFromDimCount(uint32_t dim_count, bool isArray) {
  ImageTypeVal imageType = UNSPECIFIED_MEM_OBJECT_IMAGE;
  if (dim_count == 1) {
    imageType = OpenCL_MEM_OBJECT_IMAGE1D;
  } else if (dim_count == 2) {
    imageType =
        isArray ? OpenCL_MEM_OBJECT_IMAGE1D_ARRAY : OpenCL_MEM_OBJECT_IMAGE2D;
  } else if (dim_count == 3) {
    imageType =
        isArray ? OpenCL_MEM_OBJECT_IMAGE2D_ARRAY : OpenCL_MEM_OBJECT_IMAGE3D;
  }
  return imageType;
}

} // namespace Validation
