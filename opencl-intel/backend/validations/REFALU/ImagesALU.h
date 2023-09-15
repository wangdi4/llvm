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

#ifndef _IMAGESALU_H_
#define _IMAGESALU_H_

#include "Conformance/reference_math.h"
#include "Conformance/test_common/errorHelpers.h"
#include "Exception.h"
#include "FloatOperations.h"
#include "cl_types.h"
#include "dxfloat.h"

namespace Conformance {
typedef struct {
  size_t width;
  size_t height;
  size_t depth;
  size_t rowPitch;
  size_t slicePitch;
  size_t arraySize;
  cl_image_format *format;
  cl_mem buffer;
  cl_mem_object_type type;
} image_descriptor;

// Definition for our own sampler type, to mirror the cl_sampler internals
typedef struct {
  cl_addressing_mode addressing_mode;
  cl_filter_mode filter_mode;
  bool normalized_coords;
} image_sampler_data;

typedef struct {
  float p[4];
} FloatPixel;

extern int32_t has_alpha(cl_image_format *format);
size_t get_pixel_size(cl_image_format *format);

// get number of channels
size_t get_format_channel_count(const cl_image_format *format);

template <class T>
void read_image_pixel(void *imageData, image_descriptor *imageInfo, int x,
                      int y, int z, T *outData) {
  if (x < 0 || y < 0 || z < 0 || x >= (int)imageInfo->width ||
      (imageInfo->height != 0 && y >= (int)imageInfo->height) ||
      (imageInfo->depth != 0 && z >= (int)imageInfo->depth) ||
      (imageInfo->arraySize != 0 && z >= (int)imageInfo->arraySize)) {
    // Border color
    outData[0] = outData[1] = outData[2] = outData[3] = 0;
    if (!has_alpha(imageInfo->format))
      outData[3] = 1;
    return;
  }

  cl_image_format *format = imageInfo->format;

  unsigned int i;
  T tempData[4];

  // Advance to the right spot
  char *ptr = (char *)imageData;
  size_t pixelSize = get_pixel_size(format);

  ptr += z * imageInfo->slicePitch + y * imageInfo->rowPitch + x * pixelSize;

  // OpenCL only supports reading floats from certain formats
  switch (format->image_channel_data_type) {
  case CLK_SNORM_INT8: {
    cl_char *dPtr = (cl_char *)ptr;
    for (i = 0; i < get_format_channel_count(format); i++)
      tempData[i] = (T)dPtr[i];
    break;
  }

  case CLK_UNORM_INT8: {
    cl_uchar *dPtr = (cl_uchar *)ptr;
    for (i = 0; i < get_format_channel_count(format); i++)
      tempData[i] = (T)dPtr[i];
    break;
  }

  case CLK_SIGNED_INT8: {
    cl_char *dPtr = (cl_char *)ptr;
    for (i = 0; i < get_format_channel_count(format); i++)
      tempData[i] = (T)dPtr[i];
    break;
  }

  case CLK_UNSIGNED_INT8: {
    cl_uchar *dPtr = (cl_uchar *)ptr;
    for (i = 0; i < get_format_channel_count(format); i++)
      tempData[i] = (T)dPtr[i];
    break;
  }

  case CLK_SNORM_INT16: {
    cl_short *dPtr = (cl_short *)ptr;
    for (i = 0; i < get_format_channel_count(format); i++)
      tempData[i] = (T)dPtr[i];
    break;
  }

  case CLK_UNORM_INT16: {
    cl_ushort *dPtr = (cl_ushort *)ptr;
    for (i = 0; i < get_format_channel_count(format); i++)
      tempData[i] = (T)dPtr[i];
    break;
  }

  case CLK_SIGNED_INT16: {
    cl_short *dPtr = (cl_short *)ptr;
    for (i = 0; i < get_format_channel_count(format); i++)
      tempData[i] = (T)dPtr[i];
    break;
  }

  case CLK_UNSIGNED_INT16: {
    cl_ushort *dPtr = (cl_ushort *)ptr;
    for (i = 0; i < get_format_channel_count(format); i++)
      tempData[i] = (T)dPtr[i];
    break;
  }

  case CLK_HALF_FLOAT:
    // AAK!
    ::log_error("AAK!\n");
    break;

  case CLK_SIGNED_INT32: {
    cl_int *dPtr = (cl_int *)ptr;
    for (i = 0; i < get_format_channel_count(format); i++)
      tempData[i] = (T)dPtr[i];
    break;
  }

  case CLK_UNSIGNED_INT32: {
    cl_uint *dPtr = (cl_uint *)ptr;
    for (i = 0; i < get_format_channel_count(format); i++)
      tempData[i] = (T)dPtr[i];
    break;
  }

  case CLK_UNORM_SHORT_565: {
    cl_ushort *dPtr = (cl_ushort *)ptr;
    tempData[0] = (T)(dPtr[0] >> 11);
    tempData[1] = (T)((dPtr[0] >> 5) & 63);
    tempData[2] = (T)(dPtr[0] & 31);
    break;
  }

#ifdef OBSOLETE_FORMAT
  case CLK_UNORM_SHORT_565_REV: {
    unsigned short *dPtr = (unsigned short *)ptr;
    tempData[2] = (T)(dPtr[0] >> 11);
    tempData[1] = (T)((dPtr[0] >> 5) & 63);
    tempData[0] = (T)(dPtr[0] & 31);
    break;
  }

  case CLK_UNORM_SHORT_555_REV: {
    unsigned short *dPtr = (unsigned short *)ptr;
    tempData[2] = (T)((dPtr[0] >> 10) & 31);
    tempData[1] = (T)((dPtr[0] >> 5) & 31);
    tempData[0] = (T)(dPtr[0] & 31);
    break;
  }

  case CLK_UNORM_INT_8888: {
    unsigned int *dPtr = (unsigned int *)ptr;
    tempData[3] = (T)(dPtr[0] >> 24);
    tempData[2] = (T)((dPtr[0] >> 16) & 0xff);
    tempData[1] = (T)((dPtr[0] >> 8) & 0xff);
    tempData[0] = (T)(dPtr[0] & 0xff);
    break;
  }
  case CLK_UNORM_INT_8888_REV: {
    unsigned int *dPtr = (unsigned int *)ptr;
    tempData[0] = (T)(dPtr[0] >> 24);
    tempData[1] = (T)((dPtr[0] >> 16) & 0xff);
    tempData[2] = (T)((dPtr[0] >> 8) & 0xff);
    tempData[3] = (T)(dPtr[0] & 0xff);
    break;
  }

  case CLK_UNORM_INT_101010_REV: {
    unsigned int *dPtr = (unsigned int *)ptr;
    tempData[2] = (T)((dPtr[0] >> 20) & 0x3ff);
    tempData[1] = (T)((dPtr[0] >> 10) & 0x3ff);
    tempData[0] = (T)(dPtr[0] & 0x3ff);
    break;
  }
#endif
  case CLK_UNORM_SHORT_555: {
    cl_ushort *dPtr = (cl_ushort *)ptr;
    tempData[0] = (T)((dPtr[0] >> 10) & 31);
    tempData[1] = (T)((dPtr[0] >> 5) & 31);
    tempData[2] = (T)(dPtr[0] & 31);
    break;
  }

  case CLK_UNORM_INT_101010: {
    cl_uint *dPtr = (cl_uint *)ptr;
    tempData[0] = (T)((dPtr[0] >> 20) & 0x3ff);
    tempData[1] = (T)((dPtr[0] >> 10) & 0x3ff);
    tempData[2] = (T)(dPtr[0] & 0x3ff);
    break;
  }

  case CLK_FLOAT: {
    cl_float *dPtr = (cl_float *)ptr;
    for (i = 0; i < get_format_channel_count(format); i++)
      tempData[i] = (T)dPtr[i];
    break;
  }
  }

  outData[0] = outData[1] = outData[2] = 0;
  outData[3] = 1;

  if (format->image_channel_order == CLK_A) {
    outData[3] = tempData[0];
  } else if (format->image_channel_order == CLK_R) {
    outData[0] = tempData[0];
  } else if (format->image_channel_order == CLK_RA) {
    outData[0] = tempData[0];
    outData[3] = tempData[1];
  } else if (format->image_channel_order == CLK_RG) {
    outData[0] = tempData[0];
    outData[1] = tempData[1];
  } else if (format->image_channel_order == CLK_RGB) {
    outData[0] = tempData[0];
    outData[1] = tempData[1];
    outData[2] = tempData[2];
  } else if (format->image_channel_order == CLK_RGBA) {
    outData[0] = tempData[0];
    outData[1] = tempData[1];
    outData[2] = tempData[2];
    outData[3] = tempData[3];
  } else if (format->image_channel_order == CLK_ARGB) {
    outData[0] = tempData[1];
    outData[1] = tempData[2];
    outData[2] = tempData[3];
    outData[3] = tempData[0];
  } else if (format->image_channel_order == CLK_BGRA) {
    outData[0] = tempData[2];
    outData[1] = tempData[1];
    outData[2] = tempData[0];
    outData[3] = tempData[3];
  } else if (format->image_channel_order == CLK_INTENSITY) {
    outData[1] = tempData[0];
    outData[2] = tempData[0];
    outData[3] = tempData[0];
  } else if (format->image_channel_order == CLK_LUMINANCE) {
    outData[1] = tempData[0];
    outData[2] = tempData[0];
  } else if (format->image_channel_order == CLK_DEPTH) {
    outData[0] = tempData[0];
  } else if (format->image_channel_order == CLK_sRGBA) {
    outData[0] = (tempData[0] <= 0.04045)
                     ? tempData[0] / 12.92
                     : pow((tempData[0] + 0.055) / 1.055, 2.4);
    outData[1] = (tempData[1] <= 0.04045)
                     ? tempData[1] / 12.92
                     : pow((tempData[1] + 0.055) / 1.055, 2.4);
    outData[2] = (tempData[2] <= 0.04045)
                     ? tempData[2] / 12.92
                     : pow((tempData[2] + 0.055) / 1.055, 2.4);
    outData[3] = tempData[3];
  } else if (format->image_channel_order == CLK_sBGRA) {
    outData[0] = (tempData[2] <= 0.04045)
                     ? tempData[2] / 12.92
                     : pow((tempData[2] + 0.055) / 1.055, 2.4);
    outData[1] = (tempData[1] <= 0.04045)
                     ? tempData[1] / 12.92
                     : pow((tempData[1] + 0.055) / 1.055, 2.4);
    outData[2] = (tempData[0] <= 0.04045)
                     ? tempData[0] / 12.92
                     : pow((tempData[0] + 0.055) / 1.055, 2.4);
    outData[3] = tempData[3];
  }
#ifdef CL_1RGB_APPLE
  else if (format->image_channel_order == CLK_1RGB_APPLE) {
    outData[0] = tempData[1];
    outData[1] = tempData[2];
    outData[2] = tempData[3];
    outData[3] = 0xff;
  }
#endif
#ifdef CL_BGR1_APPLE
  else if (format->image_channel_order == CLK_BGR1_APPLE) {
    outData[0] = tempData[2];
    outData[1] = tempData[1];
    outData[2] = tempData[0];
    outData[3] = 0xff;
  }
#endif
  else {
    ::log_error("Invalid format:");
  }
}

// Stupid template rules
bool get_integer_coords_offset(float x, float y, float z, float xAddressOffset,
                               float yAddressOffset, float zAddressOffset,
                               size_t width, size_t height, size_t depth,
                               image_sampler_data *imageSampler,
                               image_descriptor *imageInfo, int &outX,
                               int &outY, int &outZ);

template <class T>
void sample_image_pixel_offset(void *imageData, image_descriptor *imageInfo,
                               float x, float y, float z, float xAddressOffset,
                               float yAddressOffset, float zAddressOffset,
                               image_sampler_data *imageSampler, T *outData) {
  int iX, iY, iZ;

  float max_w = imageInfo->width;
  float max_h;
  float max_d;

  switch (imageInfo->type) {
  case CL_MEM_OBJECT_IMAGE1D_ARRAY:
    max_h = imageInfo->arraySize;
    max_d = 0;
    break;
  case CL_MEM_OBJECT_IMAGE2D_ARRAY:
    max_h = imageInfo->height;
    max_d = imageInfo->arraySize;
    break;
  default:
    max_h = imageInfo->height;
    max_d = imageInfo->depth;
    break;
  }

  get_integer_coords_offset(x, y, z, xAddressOffset, yAddressOffset,
                            zAddressOffset, max_w, max_h, max_d, imageSampler,
                            imageInfo, iX, iY, iZ);

  read_image_pixel<T>(imageData, imageInfo, iX, iY, iZ, outData);
}

void pack_image_pixel(unsigned int *srcVector,
                      const cl_image_format *imageFormat, void *outData);
void pack_image_pixel(int *srcVector, const cl_image_format *imageFormat,
                      void *outData);
void pack_image_pixel(float *srcVector, const cl_image_format *imageFormat,
                      void *outData);

template <typename T>
void write_image_pixel(void *imageData, image_descriptor *imageInfo,
                       const int x, const int y, const int z, T *inData) {

  if (x < 0 || y < 0 || z < 0 || x >= (int)imageInfo->width ||
      (imageInfo->height != 0 && y >= (int)imageInfo->height) ||
      (imageInfo->depth != 0 && z >= (int)imageInfo->depth) ||
      (imageInfo->arraySize != 0 && z >= (int)imageInfo->arraySize)) {
    throw Validation::Exception::InvalidArgument(
        "write_image_pixel:: Coordinates out of boundaries");
  }

  cl_image_format *format = imageInfo->format;
  // Advance to the right spot
  char *ptr = (char *)imageData;
  size_t pixelSize = get_pixel_size(format);

  ptr += z * imageInfo->slicePitch + y * imageInfo->rowPitch + x * pixelSize;

  pack_image_pixel(inData, format, ptr);
}

template <class T>
void sample_image_pixel(void *imageData, image_descriptor *imageInfo, float x,
                        float y, float z, image_sampler_data *imageSampler,
                        T *outData) {
  return sample_image_pixel_offset<T>(imageData, imageInfo, x, y, z, 0.0f, 0.0f,
                                      0.0f, imageSampler, outData);
}

template <>
void sample_image_pixel<float>(void *imageData, image_descriptor *imageInfo,
                               float x, float y, float z,
                               image_sampler_data *imageSampler,
                               float *outData);

FloatPixel
sample_image_pixel_float(void *imageData, image_descriptor *imageInfo, float x,
                         float y, float z, image_sampler_data *imageSampler,
                         float *outData, int verbose, int *containsDenorms);

// get maximum relative error for pixel
float get_max_relative_error(cl_image_format *format,
                             image_sampler_data *sampler, int is3D,
                             int isLinearFilter);
// get maximum absolute error for pixel
float get_max_absolute_error(cl_image_format *format,
                             image_sampler_data *sampler);

FloatPixel sample_image_pixel_float_offset(
    void *imageData, image_descriptor *imageInfo, float x, float y, float z,
    float xAddressOffset, float yAddressOffset, float zAddressOffset,
    image_sampler_data *imageSampler, float *outData, int verbose,
    int *containsDenorms);
} // namespace Conformance

#endif
