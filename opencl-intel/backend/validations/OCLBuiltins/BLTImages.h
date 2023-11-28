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

#ifndef BLT_IMAGES_H
#define BLT_IMAGES_H

#include "Helpers.h"
#include "ImagesALU.h"
#include "RefALU.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/Support/Debug.h"

// !!!! HACK
// Do not move #include "CL/cl.h" before including <math.h> since on VS2008 it
// generates Removing annoying 'ceil': attributes not present on previous
// declaration warning C4985

#include "CL/cl.h"
#include "cl_types.h"

#ifdef DEBUG_TYPE
#undef DEBUG_TYPE
#define DEBUG_TYPE "OpenCLReferenceRunner"
#endif

struct _cl_mem_obj_descriptor;
typedef struct _cl_mem_obj_descriptor cl_mem_obj_descriptor;

namespace Validation {
namespace OCLBuiltins {

Conformance::image_descriptor
CreateConfImageDesc(const cl_mem_obj_descriptor &in_Desc,
                    cl_image_format &out_ImageFmt);
Conformance::image_sampler_data CreateSamplerData(const uint32_t &in_sampler);
cl_channel_type
ConvertChannelDataTypeFromIntelOCLToCL(const cl_channel_type &val);

template <typename T1, typename T2>
void getCoordsByImageType(const cl_mem_object_type objType,
                          const llvm::GenericValue &CoordGV, T2 &u, T2 &v,
                          T2 &w) {
  u = T2(0);
  v = T2(0);
  w = T2(0);

  switch (objType) {
  case CL_MEM_OBJECT_IMAGE1D:
  case CL_MEM_OBJECT_IMAGE1D_BUFFER:
    u = T2(getVal<T1, 1>(CoordGV, 0));
    break;
  case CL_MEM_OBJECT_IMAGE1D_ARRAY:
  case CL_MEM_OBJECT_IMAGE2D:
    u = T2(getVal<T1, 2>(CoordGV, 0));
    v = T2(getVal<T1, 2>(CoordGV, 1));
    break;
  case CL_MEM_OBJECT_IMAGE2D_ARRAY:
  case CL_MEM_OBJECT_IMAGE3D:
    u = T2(getVal<T1, 4>(CoordGV, 0));
    v = T2(getVal<T1, 4>(CoordGV, 1));
    w = T2(getVal<T1, 4>(CoordGV, 2));
    break;
  default:
    break;
  }
}

template <typename T>
llvm::GenericValue local_read_image(const cl_mem_obj_descriptor *memobj,
                                    const llvm::GenericValue &CoordGV,
                                    const uint32_t sampler,
                                    const llvm::Type::TypeID CoordTy) {
  llvm::GenericValue gv;

  const cl_mem_object_type objType = memobj->memObjType;
  // coordinates
  float u, v, w;

  if (llvm::Type::FloatTyID == CoordTy)
    getCoordsByImageType<float, float>(objType, CoordGV, u, v, w);
  else
    getCoordsByImageType<int32_t, float>(objType, CoordGV, u, v, w);

  cl_image_format im_fmt;
  Conformance::image_descriptor desc = CreateConfImageDesc(*memobj, im_fmt);
  Conformance::image_sampler_data imageSampler = CreateSamplerData(sampler);
  T Pixel[4];
  Conformance::sample_image_pixel<T>(
      memobj->pData, // void *imageData,
      &desc,         // image_descriptor *imageInfo,
      u, v, w,
      &imageSampler, // image_sampler_data *imageSampler,
      Pixel);

  if (memobj->format.image_channel_order == CLK_DEPTH)
    getRef<T>(gv) = derefPointer<T>(Pixel + 0);
  else {
    gv.AggregateVal.resize(4);
    getRef<T, 4>(gv, 0) = derefPointer<T>(Pixel + 0);
    getRef<T, 4>(gv, 1) = derefPointer<T>(Pixel + 1);
    getRef<T, 4>(gv, 2) = derefPointer<T>(Pixel + 2);
    getRef<T, 4>(gv, 3) = derefPointer<T>(Pixel + 3);
  }
  return gv;
}

template <typename T>
llvm::GenericValue lle_X_read_image(llvm::FunctionType *FT,
                                    llvm::ArrayRef<llvm::GenericValue> Args) {
  const cl_mem_obj_descriptor *memobj =
      (cl_mem_obj_descriptor *)Args[0].PointerVal;

  const llvm::GenericValue &CoordGV = Args[2];

  const uint32_t sampler = uint32_t(Args[1].IntVal.getZExtValue());

  // datatype of coordinates == float or int
  llvm::Type::TypeID CoordTy;
  // for 1d image parameter number 2 is scalar, for other image
  // formats it is a vector
  if (FT->getParamType(2)->getNumContainedTypes() > 0)
    CoordTy = FT->getParamType(2)->getContainedType(0)->getTypeID();
  else
    CoordTy = FT->getParamType(2)->getTypeID();

  return local_read_image<T>(memobj, CoordGV, sampler, CoordTy);
}

template <typename T>
llvm::GenericValue
lle_X_read_image_samplerless(llvm::FunctionType *FT,
                             llvm::ArrayRef<llvm::GenericValue> Args) {
  const cl_mem_obj_descriptor *memobj =
      (cl_mem_obj_descriptor *)Args[0].PointerVal;

  const llvm::GenericValue &CoordGV = Args[1];

  const uint32_t sampler = uint32_t(CLK_ADDRESS_NONE | CLK_FILTER_NEAREST |
                                    CLK_NORMALIZED_COORDS_FALSE);

  // datatype of coordinates == float or int
  llvm::Type::TypeID CoordTy;
  // for 1d image parameter number 1 is scalar, for other image
  // formats it is a vector
  if (FT->getParamType(1)->getNumContainedTypes() > 0)
    CoordTy = FT->getParamType(1)->getContainedType(0)->getTypeID();
  else
    CoordTy = FT->getParamType(1)->getTypeID();

  return local_read_image<T>(memobj, CoordGV, sampler, CoordTy);
}

template <typename T>
void getColorForWriteImage(const llvm::GenericValue &PixelGV, T val[4]) {
  llvm::report_fatal_error("getColor: unsupported data type.");
}

template <typename T>
llvm::GenericValue lle_X_write_image(llvm::FunctionType *FT,
                                     llvm::ArrayRef<llvm::GenericValue> Args) {
  llvm::GenericValue gv;
  cl_mem_obj_descriptor *memobj = (cl_mem_obj_descriptor *)Args[0].PointerVal;

  cl_image_format im_fmt;
  Conformance::image_descriptor desc = CreateConfImageDesc(*memobj, im_fmt);

  const cl_mem_object_type objType = memobj->memObjType;

  // coordinates
  const llvm::GenericValue &CoordGV = Args[1];

  int32_t u, v, w;

  getCoordsByImageType<int32_t, int32_t>(objType, CoordGV, u, v, w);

  // datatype of pixel
  const llvm::GenericValue &PixelGV = Args[2];

  T val[4];

  if (desc.format->image_channel_order == CLK_DEPTH)
    val[0] = getVal<T>(PixelGV);
  else
    getColorForWriteImage<T>(PixelGV, val);
  Conformance::write_image_pixel<T>(memobj->pData, &desc, u, v, w, val);

  return gv;
}

llvm::GenericValue
lle_X_get_image_dim2(llvm::FunctionType *FT,
                     llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_get_image_width(llvm::FunctionType *FT,
                      llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_get_image_height(llvm::FunctionType *FT,
                       llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_get_image_depth(llvm::FunctionType *FT,
                      llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_get_image_dim3(llvm::FunctionType *FT,
                     llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_get_image_channel_data_type(llvm::FunctionType *FT,
                                  llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_get_image_channel_order(llvm::FunctionType *FT,
                              llvm::ArrayRef<llvm::GenericValue> Args);

llvm::GenericValue
lle_X_get_image_array_size(llvm::FunctionType *FT,
                           llvm::ArrayRef<llvm::GenericValue> Args);

/// @brief convert from Intel OpenCL enums with CLK_ prefix to CL_ prefix
cl_channel_order
ConvertChannelOrderFromIntelOCLToCL(const cl_channel_order &val);

/// @brief convert from Intel OpenCL enums with CLK_ prefix to CL_ prefix
cl_channel_type
ConvertChannelDataTypeFromIntelOCLToCL(const cl_channel_type &val);

/// @brief Create image_descriptor and cl_image_format for calling
/// Conformance::ImagesALU functions
Conformance::image_descriptor
CreateConfImageDesc(const cl_mem_obj_descriptor &in_Desc,
                    cl_image_format &out_ImageFmt);

/// @brief Create sampler data struct for calling Conformance::ImagesALU
/// functions
Conformance::image_sampler_data CreateSamplerData(const uint32_t &in_sampler);

} // namespace OCLBuiltins
} // namespace Validation

#endif // BLT_IMAGES_H
