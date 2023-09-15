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

#pragma once
namespace Intel {
namespace OpenCL {
namespace Framework {

/* This union is defined to do type case between __m128i and cl_int4 / cl_uint4.
 */
typedef union sat_data {
  __m128i m128;
  cl_int4 cli4;
  cl_uint4 clui4;
} sat_data;

/**
 * Convert RGBA floating point color to image format requested.
 *
 * @param color RGBA float format
 * @param channelOrder target channel order
 * @param channelType target channel type
 * @param trgtColor allocated space for target color
 * @param trgtLength size (bytes) of target color
 *
 * @return
 * - CL_SUCCESS if OK
 * - CL_IMAGE_FORMAT_NOT_SUPPORTED if cannot convert to the specified format.
 * - CL_INVALID_ARG_SIZE if trgtLength is inappropriate for channel type.
 *   @
 */
cl_int norm_float_to_image(const cl_float4 *color,
                           const cl_channel_order channelOrder,
                           const cl_channel_type channelType, void *trgtColor,
                           const size_t trgtLength);

/**
 * Convert non normalized signed int point color to image format requested.
 *
 * @param color non normalized signed int.
 * @param channelOrder target channel order
 * @param channelType target channel type
 * @param trgtColor allocated space for target color
 * @param trgtLength size (bytes) of target color
 *
 * @return
 * - CL_SUCCESS if OK
 * - CL_IMAGE_FORMAT_NOT_SUPPORTED if cannot convert to the specified format.
 * - CL_INVALID_ARG_SIZE if trgtLength is inappropriate for channel type.
 *   @
 */
cl_int non_norm_signed_to_image(const cl_int4 *color,
                                const cl_channel_order channelOrder,
                                const cl_channel_type channelType,
                                void *trgtColor, const size_t trgtLength);

/**
 * Convert non normalized unsigned int point color to image format requested.
 *
 * @param color non normalized unsigned int.
 * @param channelOrder target channel order
 * @param channelType target channel type
 * @param trgtColor allocated space for target color
 * @param trgtLength size (bytes) of target color
 *
 * @return
 * - CL_SUCCESS if OK
 * - CL_IMAGE_FORMAT_NOT_SUPPORTED if cannot convert to the specified format.
 * - CL_INVALID_ARG_SIZE if trgtLength is inappropriate for channel type.
 *   @
 */
cl_int non_norm_unsigned_to_image(const cl_uint4 *color,
                                  const cl_channel_order channelOrder,
                                  const cl_channel_type channelType,
                                  void *trgtColor, const size_t trgtLength);
} // namespace Framework
} // namespace OpenCL
} // namespace Intel
