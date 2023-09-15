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

#include "ImageCallbackLibrary.h"
#include "BitCodeContainer.h"
#include "Compiler.h"
#include "LibraryProgramManager.h"
#include "Program.h"
#include "ServiceFactory.h"
#include "SystemInfo.h"
#include "cl_sys_info.h"
#include "exceptions.h"

#include "llvm/ADT/StringRef.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/Memory.h"

#include <string>

#if defined(_WIN32)
#include <windows.h>
#else
#include <linux/limits.h>
#define MAX_PATH PATH_MAX
#endif

#define TRAP_NAME "trap_function"

namespace Intel {
namespace OpenCL {
namespace DeviceBackend {

const std::string channelOrderToPrefix(cl_channel_order _co) {
  std::string toReturn = channelOrderToString(_co);
  // Must match order in cl_api/cl.h and cl_api/cl_2_0.h
  // channel_order starts with CL_ that we need to cut
  toReturn = toReturn.substr(strlen("CL_"), toReturn.size());
  return toReturn;
}

const std::string samplerToAddrModePrefix(SamplerType _sampler) {
  int addressMode = _sampler & __ADDRESS_MASK;
  switch (addressMode) {
  case CLK_ADDRESS_MIRRORED_REPEAT:
    return "MIRRORED_REPEAT";
    break;
  case CLK_ADDRESS_REPEAT:
    return "REPEAT";
  case CLK_ADDRESS_CLAMP:
    return "CLAMP_TO_EDGE";
  case CLK_ADDRESS_CLAMP_TO_EDGE:
    return "CLAMP_TO_EDGE";
  case CLK_ADDRESS_NONE:
    return "NONE";
  default:
    assert(0 && "Unkown addressing mode in sampler");
    return "Unknown";
  }
}

const std::string imgTypeToDimPrefix(cl_mem_object_type _type) {
  switch (_type) {
  case CL_MEM_OBJECT_IMAGE1D:
  case CL_MEM_OBJECT_IMAGE1D_ARRAY:
  case CL_MEM_OBJECT_IMAGE1D_BUFFER:
    return "1D";
  case CL_MEM_OBJECT_IMAGE2D:
  case CL_MEM_OBJECT_IMAGE2D_ARRAY:
    return "2D";
  case CL_MEM_OBJECT_IMAGE3D:
    return "3D";
  default:
    throw Exceptions::DeviceBackendExceptionBase(
        "Invalid type of image object");
  }
}

const std::string channelDataTypeToPrefix(cl_channel_type _ct) {
  std::string toReturn = channelTypeToString(_ct);
  // channel_type starts with CL_ that we need to cut
  toReturn = toReturn.substr(strlen("CL_"), toReturn.size());
  return toReturn;
}

const std::string VecSizeToPrefix(VecSize _size) {
  switch (_size) {
  case SCALAR:
    return "";
  case SOA4:
    return "soa4_";
  case SOA8:
    return "soa8_";
  case SOA16:
    return "soa16_";
  }

  throw Exceptions::DeviceBackendExceptionBase(
      std::string("Internal error. Unsupported vector size"));
}

const std::string FilterToPrefix(cl_filter_mode _filterMode) {
  switch (_filterMode) {
  case CL_FILTER_LINEAR:
    return "LINEAR";
  case CL_FILTER_NEAREST:
    return "NEAREST";
  default:
    throw Exceptions::DeviceBackendExceptionBase(
        std::string("Internal error. Unsupported filter mode"));
  }
}

std::string ImageCallbackLibrary::getLibraryBasename() {
  char szModuleName[MAX_PATH];

  Intel::OpenCL::Utils::GetModuleDirectory(szModuleName, MAX_PATH);

  // Klocwork warning - false alarm the Id is always in correct bounds
  const char *pCPUPrefix = m_CpuId->GetCPUPrefix();
  if (pCPUPrefix == nullptr)
    throw Exceptions::DeviceBackendExceptionBase(
        std::string("Internal error. NULL CPU prefix"));

  std::string ret = std::string(szModuleName) + OCL_LIBRARY_TARGET_NAME +
                    std::string(pCPUPrefix);

  assert(ret.length() <= MAX_PATH);
  return ret;
}

std::string ImageCallbackLibrary::getLibraryObjectName() {
  return getLibraryBasename() + OCL_PRECOMPILED_OUTPUT_EXTENSION;
}

UndefCbkDesc::UndefCbkDesc(UndefCbkType _type, VecSize _vecSize)
    : Type(_type), Size(_vecSize) {}

std::string UndefCbkDesc::GetName() const {
  std::string vecStr = VecSizeToPrefix(Size);

  switch (Type) {
  case READ_CBK_UNDEF_INT:
    return vecStr + "read_sample_UNDEFINED_QUAD_INT";
  case READ_CBK_UNDEF_FLOAT:
    return vecStr + "read_sample_UNDEFINED_QUAD_FLOAT";
  case TRANS_CBK_UNDEF_FLOAT:
    return vecStr + "trans_coord_float_UNDEFINED";
  case TRANS_CBK_UNDEF_FLOAT_FLOAT:
    return vecStr + "trans_coord_float_float_UNDEFINED";
  }

  throw Exceptions::DeviceBackendExceptionBase(
      "Type of undefined callback is invalid!");
}

TransCbkDesc::TransCbkDesc(bool _isInt, SamplerType _sampler,
                           VecSize _vectorSize)
    : IsIntFormat(_isInt), Sampler(_sampler), VectorSize(_vectorSize) {}

std::string TransCbkDesc::GetName() const {
  std::stringstream ss;
  ss << VecSizeToPrefix(VectorSize);
  ss << "trans_coord_float_";
  if (!IsIntFormat)
    ss << "float_";

  ss << samplerToAddrModePrefix(Sampler) << "_";
  std::string isNormalizedStr =
      (Sampler & CLK_NORMALIZED_COORDS_TRUE) ? "TRUE" : "FALSE";
  ss << isNormalizedStr << "_";

  if (Sampler & CLK_FILTER_LINEAR)
    ss << "LINEAR";
  else
    ss << "NEAREST";

  return ss.str();
}

ReadCbkDesc::ReadCbkDesc(bool _isClamp, cl_channel_order _ch_order,
                         cl_channel_type _ch_type, cl_filter_mode _filter,
                         cl_mem_object_type _imageType, VecSize _vectorSize)
    : IsClamp(_isClamp), Filter(_filter), ImageType(_imageType),
      VectorSize(_vectorSize) {
  Format.image_channel_order = _ch_order;
  Format.image_channel_data_type = _ch_type;
}

std::string ReadCbkDesc::GetName() const {
  std::stringstream ss;
  ss << VecSizeToPrefix(VectorSize);
  ss << "read_sample_";
  if (Filter == CL_FILTER_NEAREST)
    ss << "NEAREST"
       << "_";
  else
    ss << "LINEAR" << imgTypeToDimPrefix(ImageType) << "_";
  std::string clampStr = IsClamp ? "CLAMP" : "NO_CLAMP";
  ss << clampStr << "_";
  ss << channelOrderToPrefix(Format.image_channel_order) << "_";
  ss << channelDataTypeToPrefix(Format.image_channel_data_type);
  return ss.str();
}

WriteCbkDesc::WriteCbkDesc(cl_channel_order _ch_order, cl_channel_type _ch_type,
                           VecSize _vectorSize) {
  Format.image_channel_data_type = _ch_type;
  Format.image_channel_order = _ch_order;
  VectorSize = _vectorSize;
}

std::string WriteCbkDesc::GetName() const {
  std::stringstream ss;
  ss << VecSizeToPrefix(VectorSize);
  ss << "write_sample_";
  ss << channelOrderToPrefix(Format.image_channel_order) << "_";
  ss << channelDataTypeToPrefix(Format.image_channel_data_type);
  return ss.str();
}

void ImageCallbackLibrary::Build() {
  m_ImageFunctions = new ImageCallbackFunctions();
}

// For CPU this should be left empty
bool ImageCallbackLibrary::LoadExecutable() { return true; }

ImageCallbackFunctions::ImageCallbackFunctions() {
  // Reuse LLJIT from library program because precompiled image callback
  // objects and library objects are in the same .o file.
  Program *P = LibraryProgramManager::getInstance()->getProgram();
  assert(P && "Invalid library program");
  m_LLJIT = P->GetLLJIT();
  assert(m_LLJIT && "Invalid LLJIT from library program");
}

void *ImageCallbackFunctions::GetCbkPtr(const CbkDesc &_desc) {
  std::string name = _desc.GetName();

  void *addr = nullptr;
  auto sym = m_LLJIT->lookup(name);
  if (llvm::Error err = sym.takeError()) {
    llvm::logAllUnhandledErrors(std::move(err), llvm::errs());
    addr = nullptr;
  } else
    addr = reinterpret_cast<void *>(static_cast<uintptr_t>(sym->getValue()));

  if (addr == nullptr) {
    std::stringstream ss;
    ss << "Internal error. Failed to retreive pointer to function "
       << _desc.GetName();
    throw Exceptions::DeviceBackendExceptionBase(ss.str());
  }
  return addr;
}

struct TrapDescriptor : public CbkDesc {
  std::string GetName() const override { return TRAP_NAME; }
};

void *ImageCallbackFunctions::GetTrapCbk() {
  TrapDescriptor Desc;
  return GetCbkPtr(Desc);
}

ImageCallbackLibrary::~ImageCallbackLibrary() {
  if (m_ImageFunctions)
    delete m_ImageFunctions;

  delete m_Compiler;
}

} // namespace DeviceBackend
} // namespace OpenCL
} // namespace Intel
