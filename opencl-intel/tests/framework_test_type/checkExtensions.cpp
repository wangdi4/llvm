// Copyright (c) 2020-2021 Intel Corporation
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
#include "common_utils.h"
#include <algorithm>
#include <iterator>
#include <sstream>

extern cl_device_type gDeviceType;

class CheckExtensions : public ::testing::TestWithParam<bool> {
protected:
  virtual void SetUp() {
    if (GetParam() && !SETENV("CL_CONFIG_CPU_EXPERIMENTAL_FP16", "1"))
      FAIL() << "Failed to set CL_CONFIG_CPU_EXPERIMENTAL_FP16";
    cl_int err = clGetPlatformIDs(1, &m_platform, NULL);
    ASSERT_OCL_SUCCESS(err, "clGetPlatformIDs");

    err = clGetDeviceIDs(m_platform, gDeviceType, 1, &m_device, NULL);
    ASSERT_OCL_SUCCESS(err, "clGetDeviceIDs");
  }

  virtual void TearDown() {
    if (GetParam() && !UNSETENV("CL_CONFIG_CPU_EXPERIMENTAL_FP16"))
      FAIL() << "Failed to unset CL_CONFIG_CPU_EXPERIMENTAL_FP16";
  }

protected:
  cl_platform_id m_platform;
  cl_device_id m_device;
};

// Reference CPU extensions. Update this list if supported extension names or
// version changes.
const std::map<std::string, cl_version> extRefCPU = {
    {"cl_khr_icd", CL_MAKE_VERSION(1, 0, 0)},
    {"cl_khr_global_int32_base_atomics", CL_MAKE_VERSION(1, 0, 0)},
    {"cl_khr_global_int32_extended_atomics", CL_MAKE_VERSION(1, 0, 0)},
    {"cl_khr_local_int32_base_atomics", CL_MAKE_VERSION(1, 0, 0)},
    {"cl_khr_local_int32_extended_atomics", CL_MAKE_VERSION(1, 0, 0)},
    {"cl_khr_int64_base_atomics", CL_MAKE_VERSION(1, 0, 0)},
    {"cl_khr_int64_extended_atomics", CL_MAKE_VERSION(1, 0, 0)},
    {"cl_khr_byte_addressable_store", CL_MAKE_VERSION(1, 0, 0)},
    {"cl_khr_depth_images", CL_MAKE_VERSION(1, 0, 0)},
    {"cl_khr_3d_image_writes", CL_MAKE_VERSION(1, 0, 0)},
    {"cl_khr_il_program", CL_MAKE_VERSION(1, 0, 0)},
    {"cl_intel_unified_shared_memory_preview", CL_MAKE_VERSION(1, 0, 0)},
    {"cl_intel_exec_by_local_thread", CL_MAKE_VERSION(1, 0, 0)},
    {"cl_intel_vec_len_hint", CL_MAKE_VERSION(1, 0, 0)},
#ifndef _WIN32
    {"cl_intel_device_partition_by_names", CL_MAKE_VERSION(1, 0, 0)},
#endif
    {"cl_khr_spir", CL_MAKE_VERSION(1, 0, 0)},
    {"cl_khr_fp64", CL_MAKE_VERSION(1, 0, 0)},
    {"cl_khr_image2d_from_buffer", CL_MAKE_VERSION(1, 0, 0)},
    {"cl_intel_device_attribute_query", CL_MAKE_VERSION(1, 0, 0)},
    {"cl_intel_required_subgroup_size", CL_MAKE_VERSION(1, 0, 0)},
    {"cl_intel_spirv_subgroups", CL_MAKE_VERSION(1, 0, 0)},
    {"cl_intel_subgroups", CL_MAKE_VERSION(1, 0, 0)},
    {"cl_intel_subgroups_char", CL_MAKE_VERSION(1, 0, 0)},
    {"cl_intel_subgroups_long", CL_MAKE_VERSION(1, 0, 0)},
    {"cl_intel_subgroups_short", CL_MAKE_VERSION(1, 0, 0)},
    {"cl_khr_spirv_linkonce_odr", CL_MAKE_VERSION(1, 0, 0)},
    // FIXME: Re-claim cl_khr_subgroup_ballot support when we implement all
    // required builtins.
    // {"cl_khr_subgroup_ballot", CL_MAKE_VERSION(1, 0, 0)}
    };

TEST_P(CheckExtensions, CpuDevice) {
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
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo");

  std::string extensions(retSize, '\0');
  err = clGetDeviceInfo(m_device, CL_DEVICE_EXTENSIONS, extensions.size(),
                        &extensions[0], nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo");

  ASSERT_TRUE(extensionsPlatform == extensions)
      << " Expected that platform and device extensions are equal!";

  // Remove trailing '\0'
  extensions.erase(std::find(extensions.begin(), extensions.end(), '\0'),
                   extensions.end());
  // Check each queried extension is present in reference extensions
  std::map<std::string, cl_version> extRef(extRefCPU);
  if (GetParam())
    extRef.insert({"cl_khr_fp16", CL_MAKE_VERSION(1, 0, 0)});
  std::istringstream extss(extensions);
  for (auto i = std::istream_iterator<std::string>(extss),
            e = std::istream_iterator<std::string>();
       i != e; ++i) {
    ASSERT_EQ(extRef.count(*i), 1)
        << ("Expect " + (*i) + " exists once in reference extensions");
    extRef.erase(*i);
  }

  // Check extRef is empty.
  if (!extRef.empty()) {
    std::ostringstream ss;
    for (auto ext : extRef)
      ss << ext.first << ",";
    FAIL() << ("Reference extensions " + ss.str() +
               " are not in extensions queried from clGetDeviceInfo");
  }
}

static inline std::string makeVersonString(cl_version version) {
  std::ostringstream ss;
  ss << "v";
  ss << CL_VERSION_MAJOR(version);
  ss << ".";
  ss << CL_VERSION_MINOR(version);
  ss << ".";
  ss << CL_VERSION_PATCH(version);
  return ss.str();
}

static bool operator==(const cl_name_version &lhs, const cl_name_version &rhs) {
  return strcmp(lhs.name, rhs.name) == 0 && lhs.version == rhs.version;
}

TEST_P(CheckExtensions, WithVersion) {
  // Query list of extension names from clGetPlatformInfo/clGetDeviceInfo
  size_t retSize;
  size_t numOfExts;
  cl_int err = clGetPlatformInfo(
      m_platform, CL_PLATFORM_EXTENSIONS_WITH_VERSION, 0, nullptr, &retSize);
  ASSERT_OCL_SUCCESS(err, "clGetPlatformInfo");

  numOfExts = retSize / sizeof(cl_name_version);
  std::vector<cl_name_version> extsWithVerPlatform(numOfExts);
  err = clGetPlatformInfo(m_platform, CL_PLATFORM_EXTENSIONS_WITH_VERSION,
                          retSize, extsWithVerPlatform.data(), nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetPlatformInfo");

  err = clGetDeviceInfo(m_device, CL_DEVICE_EXTENSIONS_WITH_VERSION, 0, nullptr,
                        &retSize);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo");

  numOfExts = retSize / sizeof(cl_name_version);
  std::vector<cl_name_version> extsWithVer(numOfExts);
  err = clGetDeviceInfo(m_device, CL_DEVICE_EXTENSIONS_WITH_VERSION, retSize,
                        extsWithVer.data(), nullptr);
  ASSERT_OCL_SUCCESS(err, "clGetDeviceInfo");

  ASSERT_TRUE(extsWithVer == extsWithVerPlatform)
      << " Expected that platform and device extensions are equal!";

  // Check each queried extension is present in reference extensions
  std::map<std::string, cl_version> extRef(extRefCPU);
  if (GetParam())
    extRef.insert({"cl_khr_fp16", CL_MAKE_VERSION(1, 0, 0)});
  for (auto extVer : extsWithVer) {
    ASSERT_EQ(extVer.version, extRef[extVer.name])
        << ("Expect " + std::string(extVer.name) + "(" +
            makeVersonString(extVer.version) +
            ") exists once in reference extensions");
    if (extVer.version == extRef[extVer.name])
        extRef.erase(extVer.name);
  }

  // Check extRef is empty.
  if (!extRef.empty()) {
    std::ostringstream ss;
    for (auto ext : extRef)
      ss << ext.first << "(" << makeVersonString(ext.second) << "), ";

    FAIL() << ("Reference extensions " + ss.str() +
               "are not in extensions queried from clGetDeviceInfo");
  }
}

INSTANTIATE_TEST_SUITE_P(FrameworkTestType, CheckExtensions,
                         ::testing::Values(false, true));
