// INTEL CONFIDENTIAL
//
// Copyright 2012 Intel Corporation.
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

#include "Generators.h"
#include "Buffer.h"
#include "BufferDesc.h"
#include "FloatOperations.h"
#include "Image.h"

namespace Validation {

static ImageSizeDesc ProcessImageSizeDesc(const ImageSizeDesc desc,
                                          const ImageTypeVal type) {
  ImageSizeDesc ret = desc;

  if (ret.width == 0)
    throw Exception::InvalidArgument(
        "[::ProcessImageSizeDesc]Size corrupted.: sezo width");

  if (type == OpenCL_MEM_OBJECT_IMAGE1D_ARRAY)
    ret.height = ret.array_size;
  else if (type == OpenCL_MEM_OBJECT_IMAGE2D_ARRAY)
    ret.depth = ret.array_size;

  if (ret.height == 0)
    ret.height = 1; // at least one column
  if (ret.depth == 0)
    ret.depth = 1; // at least one slice

  return ret;
}

void AbstractBufferGenerator::Generate(const IMemoryObject *ptr) {
  // check IMemoryObject *ptr is Buffer
  if (ptr->GetName() != Buffer::GetBufferName()) {
    throw Exception::InvalidArgument(
        "[AbstractBufferGenerator::Generate]Buffer expected");
  }
  BufferDesc buffDesc = GetBufferDescription(ptr->GetMemoryObjectDesc());
  uint64_t n_elemsTotal =
      buffDesc.NumOfElements(); // number of elements to generate

  if (0 == n_elemsTotal) // dont know how to generate null-size argument
    throw Exception::InvalidArgument(
        "[AbstractBufferGenerator::Generate]null-size argument");

  TypeDesc elemDesc = buffDesc.GetElementDescription();

  // order of ifs is important for dereferencing
  // possible variants - pointer, pointer to vector
  // vector, pointer
  //__kernel void example(__global float* in)
  if (elemDesc.GetType() == TPOINTER) {
    n_elemsTotal *= elemDesc.GetNumberOfElements();
    elemDesc = elemDesc.GetSubTypeDesc(0);
  }
  // pointer to array or simple array
  if (elemDesc.GetType() == TARRAY) {
    throw Exception::GeneratorBadTypeException(
        "[AbstractBufferGenerator::Generate]\
                                                       arrays at kernel argument are not supported");
  }
  // pointer to vectors or simple vector
  //__kernel void example(__global float4* in)
  //__kernel void example(float4 in)
  if (elemDesc.GetType() == TVECTOR) {
    n_elemsTotal *= elemDesc.GetNumberOfElements();
    elemDesc = elemDesc.GetSubTypeDesc(0);
  }

  void *p = ptr->GetDataPtr();

  m_headDesc = elemDesc;
  GenerateBuffer(p, n_elemsTotal, elemDesc.GetSizeInBytes());
}

void AbstractImageGenerator::Generate(const IMemoryObject *ptr) {
  // check IMemoryObject *ptr is Buffer
  if (ptr->GetName() != Image::GetImageName()) {
    throw Exception::InvalidArgument(
        "[AbstractImageGenerator::Generate]Image expected");
  }
  ImageDesc imdesc = GetImageDescription(ptr->GetMemoryObjectDesc());
  ImageSizeDesc size =
      ProcessImageSizeDesc(imdesc.GetSizesDesc(), imdesc.GetImageType());

  // position in MEMORY_OBJECT
  uint8_t *p = (uint8_t *)ptr->GetDataPtr();
  for (uint64_t d = 0; d < size.depth; ++d) {
    // position in current slice
    uint8_t *p_slice = p;
    for (uint64_t h = 0; h < size.height; ++h) {
      // generate one single scan line
      GenerateImage(p_slice, size.width, imdesc);
      p_slice += size.row;
    }
    p += size.slice;
  }
}

void ImageRandomGenerator::GenerateImage(const void *p,
                                         const uint64_t pixels_in_row,
                                         const ImageDesc &imdesc) {
  uint8_t *p_row = reinterpret_cast<uint8_t *>(const_cast<void *>(p));
  for (uint64_t i = 0; i < pixels_in_row; ++i) {
    GenerateAndPackPixel(p_row, imdesc);
    p_row += ImageDesc::CalcPixelSizeInBytes(imdesc.GetImageChannelDataType(),
                                             imdesc.GetImageChannelOrder());
  }
}

template <typename T>
static void PackPixel(const void *p, const void *pixel,
                      const ImageChannelOrderVal order) {
  uint32_t channelCount = GetChannelCount(order);

  for (uint32_t i = 0; i < channelCount; ++i) {
    (reinterpret_cast<T *>(const_cast<void *>(p)))[i] =
        (reinterpret_cast<T *>(const_cast<void *>(pixel)))[i];
  }
}

void ImageRandomGenerator::GenerateAndPackPixel(void *p,
                                                const ImageDesc &imdesc) {
  ImageChannelDataTypeValWrapper DataTypeWrapper(
      imdesc.GetImageChannelDataType());
  void *pixel = new uint8_t[DataTypeWrapper.GetSize() * 4];

#define GENERATE_AND_PACK_PIX_CASE(DATA_TYPE)                                  \
  case DATA_TYPE:                                                              \
    for (uint32_t i = 0; i < 4; ++i) {                                         \
      ((ImageChannelDataTypeValToCType<DATA_TYPE>::type *)pixel)[i] =          \
          RandomGeneratorInterfaceProvider<ImageChannelDataTypeValToCType<     \
              DATA_TYPE>::type>::sample(GetRandomUniformProvider());           \
    }                                                                          \
    PackPixel<ImageChannelDataTypeValToCType<DATA_TYPE>::type>(                \
        p, pixel, imdesc.GetImageChannelOrder());                              \
    break;

  switch (imdesc.GetImageChannelDataType()) {
    // filter all specific channledatat types here
    GENERATE_AND_PACK_PIX_CASE(OpenCL_SNORM_INT8)
    GENERATE_AND_PACK_PIX_CASE(OpenCL_SNORM_INT16)
    GENERATE_AND_PACK_PIX_CASE(OpenCL_UNORM_INT8)
    GENERATE_AND_PACK_PIX_CASE(OpenCL_UNORM_INT16)
    GENERATE_AND_PACK_PIX_CASE(OpenCL_SIGNED_INT8)
    GENERATE_AND_PACK_PIX_CASE(OpenCL_SIGNED_INT16)
    GENERATE_AND_PACK_PIX_CASE(OpenCL_SIGNED_INT32)
    GENERATE_AND_PACK_PIX_CASE(OpenCL_UNSIGNED_INT8)
    GENERATE_AND_PACK_PIX_CASE(OpenCL_UNSIGNED_INT16)
    GENERATE_AND_PACK_PIX_CASE(OpenCL_UNSIGNED_INT32)
    GENERATE_AND_PACK_PIX_CASE(OpenCL_HALF_FLOAT)
    GENERATE_AND_PACK_PIX_CASE(OpenCL_FLOAT)
    // special cases
  case OpenCL_UNORM_SHORT_565: {
    // for CL_RGB and CL_RGBx images
    uint16_t packed_pixel = 0, color, *u16Pixel = (uint16_t *)pixel;
    for (uint32_t i = 0; i < 4; ++i) {
      u16Pixel[i] = RandomGeneratorInterfaceProvider<uint16_t>::sample(
          GetRandomUniformProvider());
    }
    color = u16Pixel[0];
    packed_pixel |= (color & 0x1F) << 11;

    color = u16Pixel[1];
    packed_pixel |= (color & 0x3F) << 6;

    color = u16Pixel[2];
    packed_pixel |= (color & 0x1F);

    *((uint16_t *)p) = packed_pixel;
    break;
  }
  case OpenCL_UNORM_SHORT_555: {
    // for CL_RGB and CL_RGBx images
    uint16_t packed_pixel = 0, color, *u16Pixel = (uint16_t *)pixel;
    for (uint32_t i = 0; i < 4; ++i) {
      u16Pixel[i] = RandomGeneratorInterfaceProvider<uint16_t>::sample(
          GetRandomUniformProvider());
    }
    color = u16Pixel[0];
    packed_pixel |= (color & 0x1F) << 11;

    color = u16Pixel[1];
    packed_pixel |= (color & 0x1F) << 6;

    color = u16Pixel[2];
    packed_pixel |= (color & 0x1F) << 1;

    color = u16Pixel[3];
    packed_pixel |= (color & 0x1); // ignored x-bit in RGBx

    *((uint16_t *)p) = packed_pixel;
    break;
  }
  case OpenCL_UNORM_INT_101010: {
    // for CL_RGB and CL_RGBx images
    uint32_t packed_pixel = 0, color, *u32Pixel = (uint32_t *)pixel;
    for (uint32_t i = 0; i < 4; ++i) {
      u32Pixel[i] = RandomGeneratorInterfaceProvider<uint32_t>::sample(
          GetRandomUniformProvider());
    }
    color = u32Pixel[0];
    packed_pixel |= (color & 0x3FF) << 22;

    color = u32Pixel[1];
    packed_pixel |= (color & 0x3FF) << 12;

    color = u32Pixel[2];
    packed_pixel |= (color & 0x3FF) << 2;

    color = u32Pixel[3];
    packed_pixel |= (color & 0x3); // ignored x-bit in RGBx

    *((uint32_t *)p) = packed_pixel;
    break;
  }
  default:
    delete[] (uint8_t *)pixel;
    throw Exception::InvalidArgument(
        "[ImageRandomGenerator::GenerateAndPackPixel]\
                                             Attept to generate image with unsupported image channel data type");
  }
  delete[] (uint8_t *)pixel;
}

template <typename T>
void BufferConstGenerator<T>::GenerateBuffer(void *p, uint64_t n_elems,
                                             uint64_t stride) {
  uint8_t *ptr = (uint8_t *)p;
  for (uint64_t i = 0; i < n_elems; ++i) {
    ptr = (uint8_t *)p + i * stride;
    *((T *)ptr) = m_fillVal;
  }
}

template <typename T>
void BufferRandomGenerator<T>::GenerateBuffer(void *p, uint64_t n_elems,
                                              uint64_t stride) {
  IsScalarType<T> _notUsed;
  UNUSED_ARGUMENT(_notUsed);
  uint8_t *ptr = (uint8_t *)p;
  for (uint64_t i = 0; i < n_elems; ++i) {
    ptr = (uint8_t *)p + i * stride;
    *((T *)ptr) =
        RandomGeneratorInterfaceProvider<T>::sample(GetRandomUniformProvider());
  }
}

void BufferStructureGenerator::GenerateBuffer(void *p, uint64_t n_elems,
                                              uint64_t stride) {
  uint8_t *ptr = (uint8_t *)p;
  uint8_t *local_ptr;

  TypeDesc subElem;
  if (GetElementDesc().GetType() != TSTRUCT)
    throw Exception::InvalidArgument(
        "[BufferStructureGenerator::GenerateBuffer] incorrect type descriptor "
        "in structure generator");

  for (uint64_t i = 0; i < getSubGenerators().size(); ++i) {
    uint64_t numDereferencedElems = 1;
    subElem = m_headDesc.GetSubTypeDesc(i);

    if (subElem.GetType() == TPOINTER) {
      throw Exception::GeneratorBadTypeException(
          "[BufferStructureGenerator::GenerateBuffer]\
                                                           pointers within structs are not supported");
    }
    // order of ifs is important
    // simple array
    if (subElem.GetType() == TARRAY) {
      numDereferencedElems *= subElem.GetNumberOfElements();
      subElem = subElem.GetSubTypeDesc(0);
    }
    // array of vectors or simple vector
    if (subElem.GetType() == TVECTOR) {
      numDereferencedElems *= subElem.GetNumberOfElements();
      subElem = subElem.GetSubTypeDesc(0);
    }

    getSubGenerators()[i]->SetElementDesc(GetElementDesc().GetSubTypeDesc(i));
    local_ptr = ptr;
    for (uint64_t j = 0; j < numDereferencedElems; ++j) {
      getSubGenerators()[i]->GenerateBuffer(local_ptr, n_elems,
                                            GetElementDesc().GetSizeInBytes());
      local_ptr += subElem.GetSizeInBytes();
    }
    ptr += GetElementDesc()
               .GetSubTypeDesc(i)
               .GetSizeInBytes(); // goto next structure member
  }
}

#define BUFFERCONSTGENERATOR_FACTORY(Ty)                                       \
  else if (name == BufferConstGeneratorConfig<Ty>::getStaticName()) res =      \
      new BufferConstGenerator<Ty>(                                            \
          rng, static_cast<const BufferConstGeneratorConfig<Ty> *>(cfg));
#define BUFFERRANDOMGENERATOR_FACTORY(Ty)                                      \
  else if (name == BufferRandomGeneratorConfig<Ty>::getStaticName()) res =     \
      new BufferRandomGenerator<Ty>(rng);

AbstractGenerator *GeneratorFactory::create(const AbstractGeneratorConfig *cfg,
                                            const RandomUniformProvider &rng) {
  std::string name = cfg->getName();
  AbstractGenerator *res = 0;
  if (name == BufferConstGeneratorConfig<float>::getStaticName()) {
    res = new BufferConstGenerator<float>(
        rng, static_cast<const BufferConstGeneratorConfig<float> *>(cfg));
  }
  BUFFERCONSTGENERATOR_FACTORY(double)
  BUFFERCONSTGENERATOR_FACTORY(uint8_t)
  BUFFERCONSTGENERATOR_FACTORY(int8_t)
  BUFFERCONSTGENERATOR_FACTORY(uint16_t)
  BUFFERCONSTGENERATOR_FACTORY(int16_t)
  BUFFERCONSTGENERATOR_FACTORY(uint32_t)
  BUFFERCONSTGENERATOR_FACTORY(int32_t)
  BUFFERCONSTGENERATOR_FACTORY(uint64_t)
  BUFFERCONSTGENERATOR_FACTORY(int64_t)

  BUFFERRANDOMGENERATOR_FACTORY(float)
  BUFFERRANDOMGENERATOR_FACTORY(double)
  BUFFERRANDOMGENERATOR_FACTORY(uint8_t)
  BUFFERRANDOMGENERATOR_FACTORY(int8_t)
  BUFFERRANDOMGENERATOR_FACTORY(uint16_t)
  BUFFERRANDOMGENERATOR_FACTORY(int16_t)
  BUFFERRANDOMGENERATOR_FACTORY(uint32_t)
  BUFFERRANDOMGENERATOR_FACTORY(int32_t)
  BUFFERRANDOMGENERATOR_FACTORY(uint64_t)
  BUFFERRANDOMGENERATOR_FACTORY(int64_t)

  else if (name == BufferStructureGeneratorConfig::getStaticName()) {
    AbstractGeneratorConfigVector::const_iterator it;
    AbstractGeneratorConfig *acfg = const_cast<AbstractGeneratorConfig *>(cfg);

    BufferStructureGenerator *g = new BufferStructureGenerator(rng);

    for (it = static_cast<BufferStructureGeneratorConfig *>(acfg)
                  ->getConfigVector()
                  .begin();
         it != static_cast<BufferStructureGeneratorConfig *>(acfg)
                   ->getConfigVector()
                   .end();
         ++it) {
      g->getSubGenerators().push_back(
          static_cast<AbstractBufferGenerator *>(create(*it, rng)));
    }

    res = g;
  }
  else if (name == ImageRandomGeneratorConfig::getStaticName()) {
    res = new ImageRandomGenerator(rng);
  }
  else throw Exception::GeneratorBadTypeException(
      "[GeneratorFACTORY::create]wrong generator name");

  return res;
}
#undef BUFFERCONSTGENERATOR_FACTORY
#undef BUFFERRANDOMGENERATOR_FACTORY
} // namespace Validation
