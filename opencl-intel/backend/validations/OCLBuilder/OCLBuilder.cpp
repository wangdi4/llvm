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

#include "OCLBuilder.h"
#include "cl_env.h"
#include "clang_device_info.h"
#include "ocl_string_exception.h"
#include "opencl_c_features.h"

using namespace llvm;
using namespace Intel::OpenCL::FECompilerAPI;
using namespace Intel::OpenCL;
using namespace Intel::OpenCL::Utils;

namespace Validation {
#define BE_FE_COMPILER_USE_EXTENSIONS                                          \
  " cl_khr_fp64 cl_khr_icd cl_khr_spirv_linkonce_odr cl_khr_il_program"        \
  " cl_khr_global_int32_base_atomics cl_khr_global_int32_extended_atomics"     \
  " cl_khr_local_int32_base_atomics cl_khr_local_int32_extended_atomics"       \
  " cl_khr_int64_base_atomics cl_khr_int64_extended_atomics"                   \
  " cl_khr_byte_addressable_store cl_intel_printf cl_ext_device_fission"       \
  " cl_intel_unified_shared_memory"

#define BE_FE_COMPILER_USE_EXTENSIONS_FPGA                                     \
  " cl_intel_channels cl_intel_fpga_host_pipe"                                 \
  " cl_intel_mem_channel_property"

#define BE_FE_COMPILER_USE_EXTENSIONS_CPU                                      \
  " cl_khr_int64_base_atomics cl_khr_int64_extended_atomics"                   \
  " cl_intel_exec_by_local_thread cl_intel_vec_len_hint"                       \
  " cl_intel_device_partition_by_names cl_khr_spir cl_khr_image2d_from_buffer" \
  " cl_khr_depth_images cl_khr_3d_image_writes"                                \
  " cl_intel_device_attribute_query"                                           \
  " cl_intel_subgroups cl_intel_subgroups_char cl_intel_subgroups_short"       \
  " cl_intel_subgroups_long cl_intel_required_subgroup_size"                   \
  " cl_intel_spirv_subgroups cl_khr_subgroup_shuffle"                          \
  " cl_khr_subgroup_shuffle_relative cl_khr_subgroup_extended_types"           \
  " cl_khr_subgroup_non_uniform_arithmetic"

OCLBuilder &OCLBuilder::Instance() {
  // Statically initialized instance of the builder
  static OCLBuilder instance;
  return instance;
}

OCLBuilder &OCLBuilder::createCompiler() {
  try {
    m_CommonBuilder.createCompiler();
    return *this;
  } catch (ocl_string_exception &Error) {
    throw Validation::Exception::OperationFailed(Error.what());
  }
}

// Sets the build options
OCLBuilder &OCLBuilder::withBuildOptions(const char *options) {
  try {
    m_CommonBuilder.withBuildOptions(options);
    return *this;
  } catch (ocl_string_exception &Error) {
    throw Validation::Exception::OperationFailed(Error.what());
  }
}

// sets the OCL source to be compiled
OCLBuilder &OCLBuilder::withSource(const char *src) {
  try {
    m_CommonBuilder.withSource(src);
    return *this;
  } catch (ocl_string_exception &Error) {
    throw Validation::Exception::OperationFailed(Error.what());
  }
}

OCLBuilder &OCLBuilder::withFP64Support(bool FP64) {
  try {
    m_CommonBuilder.withFP64Support(FP64);
    return *this;
  } catch (ocl_string_exception &Error) {
    throw Validation::Exception::OperationFailed(Error.what());
  }
}

OCLBuilder &OCLBuilder::withImageSupport(bool IS) {
  try {
    m_CommonBuilder.withImageSupport(IS);
    return *this;
  } catch (ocl_string_exception &Error) {
    throw Validation::Exception::OperationFailed(Error.what());
  }
}

OCLBuilder &OCLBuilder::withFpgaEmulator(bool IsFPGA) {
  try {
    m_CommonBuilder.withFpgaEmulator(IsFPGA);
    return *this;
  } catch (ocl_string_exception &Error) {
    throw Validation::Exception::OperationFailed(Error.what());
  }
}

OCLBuilder &OCLBuilder::withFP16Support(bool FP16) {
  try {
    m_CommonBuilder.withFP16Support(FP16);
    return *this;
  } catch (ocl_string_exception &Error) {
    throw Validation::Exception::OperationFailed(Error.what());
  }
}

OCLBuilder &OCLBuilder::withExtensions(bool IsFPGA) {
  std::string ext = std::string(BE_FE_COMPILER_USE_EXTENSIONS) +
                    std::string(IsFPGA ? BE_FE_COMPILER_USE_EXTENSIONS_FPGA
                                       : BE_FE_COMPILER_USE_EXTENSIONS_CPU);
  if (!IsFPGA)
    ext += " cl_khr_fp16";
  m_CommonBuilder.withExtensions(ext);
  return *this;
}

OCLBuilder &OCLBuilder::withOpenCLCFeatures() {
  m_CommonBuilder.withOpenCLCFeatures(
      (Twine(OPENCL_C_3D_IMAGE_WRITES) + Twine(" ") +
       Twine(OPENCL_C_ATOMIC_ORDER_ACQ_REL) + Twine(" ") +
       Twine(OPENCL_C_ATOMIC_ORDER_SEQ_CST) + Twine(" ") +
       Twine(OPENCL_C_ATOMIC_SCOPE_DEVICE) + Twine(" ") +
       Twine(OPENCL_C_ATOMIC_SCOPE_ALL_DEVICES) + Twine(" ") +
       Twine(OPENCL_C_DEVICE_ENQUEUE) + Twine(" ") +
       Twine(OPENCL_C_GENERIC_ADDRESS_SPACE) + Twine(" ") +
       Twine(OPENCL_C_FP64) + Twine(" ") + Twine(OPENCL_C_IMAGES) + Twine(" ") +
       Twine(OPENCL_C_INT64) + Twine(" ") + Twine(OPENCL_C_PIPES) + Twine(" ") +
       Twine(OPENCL_C_PROGRAM_SCOPE_GLOBAL_VARIABLES) + Twine(" ") +
       Twine(OPENCL_C_READ_WRITE_IMAGES) + Twine(" ") +
       Twine(OPENCL_C_SUBGROUPS) + Twine(" ") +
       Twine(OPENCL_C_WORK_GROUP_COLLECTIVE_FUNCTIONS) + Twine(" "))
          .str());
  return *this;
}

// cleanup function
void OCLBuilder::close() {
  try {
    m_CommonBuilder.close();
  } catch (ocl_string_exception &Error) {
    throw Validation::Exception::OperationFailed(Error.what());
  }
}

// Compiles the (previously given) source file with the compiler loaded from
//(the previously given) library.
// Returns value: IOCLFEBinaryResult, which contains the binary result in its
// bytecode form, with some metadeta on it (size in bytes etc.)
Intel::OpenCL::ClangFE::IOCLFEBinaryResult *OCLBuilder::build() {
  try {
    return m_CommonBuilder.build();
  } catch (ocl_string_exception &Error) {
    throw Validation::Exception::OperationFailed(Error.what());
  }
}

OCLBuilder::OCLBuilder()
    : m_CommonBuilder(Intel::OpenCL::Utils::CommonOCLBuilder::instance()) {}

} // namespace Validation
