// INTEL CONFIDENTIAL
//
// Copyright 2012-2018 Intel Corporation.
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

#include "Utils.h"
#include <cassert>
#include <sstream>
#include <string>

namespace reflection {

// string represenration for the primitive types
static const char *PrimitiveNames[PRIMITIVE_NUM] = {"bool",
                                                    "uchar",
                                                    "char",
                                                    "ushort",
                                                    "short",
                                                    "uint",
                                                    "int",
                                                    "ulong",
                                                    "long",
                                                    "half",
                                                    "float",
                                                    "double",
                                                    "void",
                                                    "...",
                                                    "image1d_t",
                                                    "image1d_ro_t",
                                                    "image1d_wo_t",
                                                    "image1d_rw_t",
                                                    "image2d_t",
                                                    "image2d_ro_t",
                                                    "image2d_wo_t",
                                                    "image2d_rw_t",
                                                    "image2d_depth_t",
                                                    "image2d_depth_ro_t",
                                                    "image2d_depth_wo_t",
                                                    "image2d_depth_rw_t",
                                                    "image3d_t",
                                                    "image3d_ro_t",
                                                    "image3d_wo_t",
                                                    "image3d_rw_t",
                                                    "image1d_buffer_t",
                                                    "image1d_buffer_ro_t",
                                                    "image1d_buffer_wo_t",
                                                    "image1d_buffer_rw_t",
                                                    "image1d_array_t",
                                                    "image1d_array_ro_t",
                                                    "image1d_array_wo_t",
                                                    "image1d_array_rw_t",
                                                    "image2d_array_t",
                                                    "image2d_array_ro_t",
                                                    "image2d_array_wo_t",
                                                    "image2d_array_rw_t",
                                                    "image2d_array_depth_t",
                                                    "image2d_array_depth_ro_t",
                                                    "image2d_array_depth_wo_t",
                                                    "image2d_array_depth_rw_t",
                                                    "event_t",
                                                    "clk_event_t",
                                                    "queue_t",
                                                    "pipe_t",
                                                    "pipe_ro_t",
                                                    "pipe_wo_t",
                                                    "sampler_t"};

const char *mangledTypes[PRIMITIVE_NUM] = {
    "b",                            // BOOL
    "h",                            // UCHAR
    "c",                            // CHAR
    "t",                            // USHORT
    "s",                            // SHORT
    "j",                            // UINT
    "i",                            // INT
    "m",                            // ULONG
    "l",                            // LONG
    "Dh",                           // HALF
    "f",                            // FLOAT
    "d",                            // DOUBLE
    "v",                            // VOID
    "z",                            // VarArg
    "11ocl_image1d",                // PRIMITIVE_IMAGE_1D_T
    "14ocl_image1d_ro",             // PRIMITIVE_IMAGE_1D_T
    "14ocl_image1d_wo",             // PRIMITIVE_IMAGE_1D_T
    "14ocl_image1d_rw",             // PRIMITIVE_IMAGE_1D_T
    "11ocl_image2d",                // PRIMITIVE_IMAGE_2D_T
    "14ocl_image2d_ro",             // PRIMITIVE_IMAGE_2D_T
    "14ocl_image2d_wo",             // PRIMITIVE_IMAGE_2D_T
    "14ocl_image2d_rw",             // PRIMITIVE_IMAGE_2D_T
    "16ocl_image2ddepth",           // PRIMITIVE_IMAGE_2D_DEPTH_T
    "20ocl_image2d_depth_ro",       // PRIMITIVE_IMAGE_2D_DEPTH_T
    "20ocl_image2d_depth_wo",       // PRIMITIVE_IMAGE_2D_DEPTH_T
    "20ocl_image2d_depth_rw",       // PRIMITIVE_IMAGE_2D_DEPTH_T
    "11ocl_image3d",                // PRIMITIVE_IMAGE_3D_T
    "14ocl_image3d_ro",             // PRIMITIVE_IMAGE_3D_T
    "14ocl_image3d_wo",             // PRIMITIVE_IMAGE_3D_T
    "14ocl_image3d_rw",             // PRIMITIVE_IMAGE_3D_T
    "17ocl_image1dbuffer",          // PRIMITIVE_IMAGE_1D_BUFFER_T
    "21ocl_image1d_buffer_ro",      // PRIMITIVE_IMAGE_1D_BUFFER_T
    "21ocl_image1d_buffer_wo",      // PRIMITIVE_IMAGE_1D_BUFFER_T
    "21ocl_image1d_buffer_rw",      // PRIMITIVE_IMAGE_1D_BUFFER_T
    "16ocl_image1darray",           // PRIMITIVE_IMAGE_1D_ARRAY_T
    "20ocl_image1d_array_ro",       // PRIMITIVE_IMAGE_1D_ARRAY_T
    "20ocl_image1d_array_wo",       // PRIMITIVE_IMAGE_1D_ARRAY_T
    "20ocl_image1d_array_rw",       // PRIMITIVE_IMAGE_1D_ARRAY_T
    "16ocl_image2darray",           // PRIMITIVE_IMAGE_2D_ARRAY_T
    "20ocl_image2d_array_ro",       // PRIMITIVE_IMAGE_2D_ARRAY_T
    "20ocl_image2d_array_wo",       // PRIMITIVE_IMAGE_2D_ARRAY_T
    "20ocl_image2d_array_rw",       // PRIMITIVE_IMAGE_2D_ARRAY_T
    "21ocl_image2darraydepth",      // PRIMITIVE_IMAGE_2D_ARRAY_DEPTH_T
    "26ocl_image2d_array_depth_ro", // PRIMITIVE_IMAGE_2D_ARRAY_DEPTH_T
    "26ocl_image2d_array_depth_wo", // PRIMITIVE_IMAGE_2D_ARRAY_DEPTH_T
    "26ocl_image2d_array_depth_rw", // PRIMITIVE_IMAGE_2D_ARRAY_DEPTH_T
    "9ocl_event",                   // PRIMITIVE_EVENT_T
    "12ocl_clkevent",               // PRIMITIVE_CLK_EVENT_T
    "9ocl_queue",                   // PRIMITIVE_QUEUE_T
    "8ocl_pipe",                    // PRIMITIVE_PIPE_T
    "11ocl_pipe_ro",                // PRIMITIVE_PIPE_RO_T
    "11ocl_pipe_wo",                // PRIMITIVE_PIPE_WO_T
    "11ocl_sampler"                 // PRIMITIVE_SAMPLER_T
};

const char *readableAttribute[ATTR_NUM] = {
    "__private", "__global", "__constant", "__local",
    "__generic", "restrict", "volatile",   "const"};

const char *mangledAttribute[ATTR_NUM] = {
    "",
    "U3AS1",
    "U3AS2",
    "U3AS3",
    "U3AS4",
    "r",
    "V",
    "K"
};

const char *mangledPrimitiveString(TypePrimitiveEnum t) {
  return mangledTypes[t];
}

const char *readablePrimitiveString(TypePrimitiveEnum t) {
  return PrimitiveNames[t];
}

std::string llvmPrimitiveString(TypePrimitiveEnum t) {
  assert(t >= PRIMITIVE_STRUCT_FIRST && t <= PRIMITIVE_STRUCT_LAST &&
         "assuming struct primitive type only!");
  return std::string("opencl.") + std::string(PrimitiveNames[t]);
}

std::string getMangledAttribute(TypeAttributeEnum attribute) {
  return mangledAttribute[attribute];
}

std::string getReadableAttribute(TypeAttributeEnum attribute) {
  return readableAttribute[attribute];
}

std::string getDuplicateString(int index) {
  assert(index >= 0 && "illegal index");
  if (0 == index)
    return "S_";
  std::stringstream ss;
  ss << "S" << index - 1 << "_";
  return ss.str();
}

} // namespace reflection {
