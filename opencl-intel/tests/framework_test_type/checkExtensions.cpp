// Copyright (c) 2020 Intel Corporation
// All rights reserved.
//
// WARRANTY DISCLAIMER
//
// THESE MATERIALS ARE PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL INTEL OR ITS
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THESE
// MATERIALS, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Intel Corporation is the author of the Materials, and requests that all
// problem reports or change requests be submitted to it directly

#include "TestsHelpClasses.h"
#include <algorithm>
#include <iterator>
#include <sstream>

extern cl_device_type gDeviceType;

class CheckExtensions : public ::testing::Test {
protected:
  virtual void SetUp() {
    cl_int err = clGetPlatformIDs(1, &m_platform, NULL);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, NULL);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");
  }

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
};

TEST_F(CheckExtensions, CpuDevice) {
  // Query list of extension names from clGetPlatformInfo/clGetDeviceInfo
  size_t retSize;
  cl_int err = clGetPlatformInfo(m_platform, CL_PLATFORM_EXTENSIONS, 0, nullptr,
                                 &retSize);
  ASSERT_OCL_SUCCESS(err, "clGetPlatformInfo");

  std::string extensionsPlatform(retSize, '\0');
  err = clGetPlatformInfo(m_platform, CL_PLATFORM_EXTENSIONS,
                          extensionsPlatform.size(), &extensionsPlatform[0],
                          nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetPlatformInfo");
  err = clGetDeviceInfo(m_device, CL_DEVICE_EXTENSIONS, 0, nullptr, &retSize);
  ASSERT_OCL_SUCCESS(err, "clGetPlatformInfo");

  std::string extensions(retSize, '\0');
  err = clGetDeviceInfo(m_device, CL_DEVICE_EXTENSIONS, extensions.size(),
                        &extensions[0], nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetPlatformInfo");

  ASSERT_TRUE(extensionsPlatform == extensions)
      << " Expected that platform and device extensions are equal!";

  // Reference CPU extensions. Update this list if supported extension names
  // changes.
  std::set<std::string> extRefCPU = {"cl_khr_icd",
                                     "cl_khr_global_int32_base_atomics",
                                     "cl_khr_global_int32_extended_atomics",
                                     "cl_khr_local_int32_base_atomics",
                                     "cl_khr_local_int32_extended_atomics",
                                     "cl_khr_int64_base_atomics",
                                     "cl_khr_int64_extended_atomics",
                                     "cl_khr_byte_addressable_store",
                                     "cl_khr_depth_images",
                                     "cl_khr_3d_image_writes",
                                     "cl_khr_il_program",
                                     "cl_intel_unified_shared_memory_preview",
                                     "cl_intel_exec_by_local_thread",
                                     "cl_intel_vec_len_hint",
#ifndef _WIN32
                                     "cl_intel_device_partition_by_names",
#endif
                                     "cl_khr_spir",
                                     "cl_khr_fp64",
                                     "cl_khr_image2d_from_buffer",
                                     "cl_intel_required_subgroup_size",
                                     "cl_intel_spirv_subgroups",
                                     "cl_intel_subgroups",
                                     "cl_intel_subgroups_char",
                                     "cl_intel_subgroups_long",
                                     "cl_intel_subgroups_short"};
  // Remove trailing '\0'
  extensions.erase(std::find(extensions.begin(), extensions.end(), '\0'),
                   extensions.end());
  // Check each queried extension is present in reference extensions
  std::istringstream extss(extensions);
  for (auto i = std::istream_iterator<std::string>(extss),
            e = std::istream_iterator<std::string>();
       i != e; ++i) {
    ASSERT_EQ(extRefCPU.count(*i), 1)
        << ("Expect " + (*i) + " exists once in reference extensions");
    extRefCPU.erase(*i);
  }

  // Check extRefCPU is empty.
  if (!extRefCPU.empty()) {
    std::ostringstream ss;
    std::copy(extRefCPU.begin(), extRefCPU.end(),
              std::ostream_iterator<std::string>(ss, ","));
    FAIL() << ("Reference extensions " + ss.str() +
               " are not in extensions queried from clGetDeviceInfo");
  }
}
