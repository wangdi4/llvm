// INTEL CONFIDENTIAL
//
// Copyright 2010 Intel Corporation.
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

#include "BuiltinModules.h"
#include "CPUCompiler.h"
#include "StaticObjectLoader.h"
#include "cl_cpu_detect.h"
#include "cl_utils.h"
#include "llvm/ExecutionEngine/ObjectCache.h"
#include <memory>

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

// TODO: ensure it is defined from cl_image_declaration.h
// indexes of translation callbacks for each type of sampler
// !!!IMPORTANT!!! These defines should be the same as in cl_image_declaration.h
enum SamplerType {
  NONE_FALSE_NEAREST = 0x10,
  CLAMP_FALSE_NEAREST = 0x14,
  CLAMPTOEDGE_FALSE_NEAREST = 0x12,
  REPEAT_FALSE_NEAREST = 0x16,
  MIRRORED_FALSE_NEAREST = 0x18,

  NONE_TRUE_NEAREST = 0x11,
  CLAMP_TRUE_NEAREST = 0x15,
  CLAMPTOEDGE_TRUE_NEAREST = 0x13,
  REPEAT_TRUE_NEAREST = 0x17,
  MIRRORED_TRUE_NEAREST = 0x19,

  NONE_FALSE_LINEAR = 0x20,
  CLAMP_FALSE_LINEAR = 0x24,
  CLAMPTOEDGE_FALSE_LINEAR = 0x22,
  REPEAT_FALSE_LINEAR = 0x26,
  MIRRORED_FALSE_LINEAR = 0x28,

  NONE_TRUE_LINEAR = 0x21,
  CLAMP_TRUE_LINEAR = 0x25,
  CLAMPTOEDGE_TRUE_LINEAR = 0x23,
  REPEAT_TRUE_LINEAR = 0x27,
  MIRRORED_TRUE_LINEAR = 0x29,

  SAMPLER_UNDEFINED = 0xFF
};

// Describes callback type that is used if read_image is called
// with parameters that by spec produce undefined return value
enum UndefCbkType {
  // Undefined reading callback with integer coordinates
  READ_CBK_UNDEF_INT,
  // Undefined reading callback with floating point coordinates
  READ_CBK_UNDEF_FLOAT,
  // Undefined translation callback for floating point coordinates but integer
  // image
  TRANS_CBK_UNDEF_FLOAT,
  // Undefined translation callback for floating poitn coordinates and float
  // image
  TRANS_CBK_UNDEF_FLOAT_FLOAT
};

// callback vector size
enum VecSize { SCALAR = 1, SOA4 = 4, SOA8 = 8, SOA16 = 16 };

// Auxiliary functions for image callback names mangling
const std::string channelOrderToPrefix(cl_channel_order _co);
const std::string samplerToAddrModePrefix(SamplerType _sampler);
const std::string imgTypeToDimPrefix(cl_mem_object_type _type);
const std::string channelDataTypeToPrefix(cl_channel_type _ct);
const std::string VecSizeToPrefix(VecSize _size);
const std::string FilterToPrefix(cl_filter_mode _filterMode);

class CbkDesc {
public:
  virtual std::string GetName() const = 0;
  virtual ~CbkDesc() {}
};

class UndefCbkDesc : public CbkDesc {
public:
  UndefCbkDesc(UndefCbkType _type, VecSize _vecSize = SCALAR);
  virtual std::string GetName() const override;

private:
  UndefCbkType Type;
  VecSize Size;
};

class TransCbkDesc : public CbkDesc {
public:
  TransCbkDesc(bool _isInt, SamplerType _sampler, VecSize _vectorSize = SCALAR);
  virtual std::string GetName() const override;

private:
  bool IsIntFormat;
  SamplerType Sampler;
  VecSize VectorSize;
};

class ReadCbkDesc : public CbkDesc {
public:
  ReadCbkDesc(bool _isClamp, cl_channel_order _ch_order,
              cl_channel_type _ch_type, cl_filter_mode _filter,
              // For regular 1.1 images reading callbacks are common for 2D and
              // 3D so set 2D here for generality
              cl_mem_object_type _imageType = CL_MEM_OBJECT_IMAGE2D,
              VecSize _vectorSize = SCALAR);

  virtual std::string GetName() const override;

private:
  cl_image_format Format;
  bool IsClamp;
  cl_filter_mode Filter;
  cl_mem_object_type ImageType;
  VecSize VectorSize;
};

class WriteCbkDesc : public CbkDesc {
public:
  WriteCbkDesc(cl_channel_order _ch_order, cl_channel_type _ch_type,
               VecSize _vectorSize = SCALAR);
  // Returns llvm name of a function
  virtual std::string GetName() const override;

private:
  VecSize VectorSize;
  cl_image_format Format;
};

const bool FLT_CBK = false;
const bool INT_CBK = !FLT_CBK;

const bool CLAMP_CBK = true;
const bool NO_CLAMP_CBK = !CLAMP_CBK;

// Returns size of array
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a)                                                          \
  ((sizeof(a) / sizeof(*(a))) /                                                \
   static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))
#endif

/**
 *  Holds the entire callback functions set for a specific compiled library
 * (i.e., per architecture)
 */
class ImageCallbackFunctions {
public:
  ImageCallbackFunctions();

private:
  // Used for function lookup. Not owned by this class.
  llvm::orc::LLJIT *m_LLJIT;

  void *GetCbkPtr(const CbkDesc &_desc);

public:
  void *GetUndefinedCbk(UndefCbkType _type, VecSize _vecSize = SCALAR) {
    UndefCbkDesc desc(_type, _vecSize);
    return GetCbkPtr(desc);
  }

  void *GetTranslationCbk(bool _isInt, SamplerType _sampler) {
    TransCbkDesc desc(_isInt, _sampler);
    return GetCbkPtr(desc);
  }

  void *GetReadingCbk(bool _isClamp, cl_channel_order _ch_order,
                      cl_channel_type _ch_type, cl_filter_mode _filter,
                      cl_mem_object_type _imageType = CL_MEM_OBJECT_IMAGE2D,
                      VecSize _vecSize = SCALAR) {
    ReadCbkDesc desc(_isClamp, _ch_order, _ch_type, _filter, _imageType,
                     _vecSize);
    return GetCbkPtr(desc);
  }

  void *GetWritingCbk(cl_channel_order _ch_order, cl_channel_type _ch_type,
                      VecSize _vectorSize = SCALAR) {
    WriteCbkDesc desc(_ch_order, _ch_type, _vectorSize);
    return GetCbkPtr(desc);
  }

  void *GetTrapCbk();
};

/**
 *  Provides loading and building image library for specified cpu. Owns compiler
 * instance
 */
class ImageCallbackLibrary {
public:
  /**
   *  ctor
   */
  ImageCallbackLibrary(const Intel::OpenCL::Utils::CPUDetect *cpuId,
                       CPUCompiler *compiler)
      : m_CpuId(cpuId), m_ImageFunctions(NULL), m_Compiler(compiler) {}

  /**
   *  Populates m_ImageFunctions. Should only be called after Load call
   */
  void Build();
  /**
   *  MIC-specific: serialize library and load it to the device
   *  Then send address back to the host
   */
  bool LoadExecutable();
  /**
   *  Returns pointer to object with previously built image function pointers.
   *  library should already be loaded and built
   */
  ImageCallbackFunctions *getImageCallbackFunctions() {
    return m_ImageFunctions;
  }

  ~ImageCallbackLibrary();

  ImageCallbackLibrary(const ImageCallbackLibrary &) = delete;
  ImageCallbackLibrary &operator=(const ImageCallbackLibrary &) = delete;

private:
  /// Get the path to the builtin library
  std::string getLibraryBasename();
  std::string getLibraryObjectName();

  const Intel::OpenCL::Utils::CPUDetect *m_CpuId;
  // Instance with all function pointers. Owned by this class
  ImageCallbackFunctions *m_ImageFunctions;
  // Pointer to Compiler. Owned by this class.
  CPUCompiler *m_Compiler;
};

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
