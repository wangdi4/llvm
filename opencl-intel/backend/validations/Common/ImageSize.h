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

#ifndef __IMAGE_SIZE_H__
#define __IMAGE_SIZE_H__

#include "ImageType.h"

namespace Validation {
// this struct is used to support binary reading from 1.1 images
struct ImageSizeDesc_1_1 {
  // sizes
  uint64_t width;
  uint64_t height;
  uint64_t depth;
  // size of lines in bytes (AKA pitch, widthStep)
  uint64_t row;
  uint64_t slice;

  ImageSizeDesc_1_1() : width(0), height(0), depth(0), row(0), slice(0) {}
};

struct ImageSizeDesc : public ImageSizeDesc_1_1 {
  uint64_t array_size;

  ImageSizeDesc() : array_size(0) {}

  void Init(ImageTypeVal imageType, const uint64_t &in_width,
            const uint64_t &in_height, const uint64_t &in_depth,
            const uint64_t &in_row, const uint64_t &in_slice,
            const uint64_t &in_array_size) {
    switch (imageType) {
    case OpenCL_MEM_OBJECT_IMAGE1D:
    case OpenCL_MEM_OBJECT_IMAGE1D_BUFFER:
      width = in_width;
      row = in_row;
      break;
    case OpenCL_MEM_OBJECT_IMAGE1D_ARRAY:
      width = in_width;
      row = in_row;
      array_size = in_array_size;
      break;
    case OpenCL_MEM_OBJECT_IMAGE2D:
      width = in_width;
      height = in_height;
      row = in_row;
      break;
    case OpenCL_MEM_OBJECT_IMAGE2D_ARRAY:
      width = in_width;
      height = in_height;
      row = in_row;
      array_size = in_array_size;
      break;
    case OpenCL_MEM_OBJECT_IMAGE3D:
      width = in_width;
      height = in_height;
      depth = in_depth;
      row = in_row;
      slice = in_slice;
      break;
    default:
      throw Exception::OutOfRange("Incorrect image type.");
    }
  }

  /// comparison
  /// !!! Compares only sizes,  row and slice are ignored
  inline bool operator==(const ImageSizeDesc &a) const {
    bool res = true;
    res &= (a.width == width);
    res &= (a.height == height);
    res &= (a.depth == depth);
    res &= (a.array_size == array_size);
    // res &= (a.row == row);
    // res &= (a.slice == slice);
    return res;
  }

  inline bool operator!=(const ImageSizeDesc &a) const { return !(*this == a); }
};

} // namespace Validation

#endif // __IMAGE_SIZE_H__
