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
#include "common_utils.h"
#include "ocl_supported_extensions.h"
#include "llvm/ADT/StringMap.h"

using namespace llvm;
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

// Reference CPU extensions. Update this list if supported extension names or
// version changes.
static const StringMap<cl_version> extRefCPU = {
    {OCL_EXT_KHR_SPIRV_LINKONCE_ODR, CL_MAKE_VERSION(1, 0, 0)},
    {OCL_EXT_KHR_FP64, CL_MAKE_VERSION(1, 0, 0)},
    {OCL_EXT_KHR_FP16, CL_MAKE_VERSION(1, 0, 0)},

    {OCL_EXT_INTEL_SUBGROUPS, CL_MAKE_VERSION(1, 0, 0)},
    {OCL_EXT_INTEL_SUBGROUPS_CHAR, CL_MAKE_VERSION(1, 0, 0)},
    {OCL_EXT_INTEL_SUBGROUPS_SHORT, CL_MAKE_VERSION(1, 0, 0)},
    {OCL_EXT_INTEL_SUBGROUPS_LONG, CL_MAKE_VERSION(1, 0, 0)},
    {OCL_EXT_INTEL_SUBGROUPS_REQD_SIZE, CL_MAKE_VERSION(1, 0, 0)},
    {OCL_EXT_KHR_SUBGROUP_SHUFFLE, CL_MAKE_VERSION(1, 0, 0)},
    {OCL_EXT_KHR_SUBGROUP_SHUFFLE_RELATIVE, CL_MAKE_VERSION(1, 0, 0)},
    {OCL_EXT_KHR_SUBGROUP_EXTENDED_TYPES, CL_MAKE_VERSION(1, 0, 0)},
    {OCL_EXT_KHR_SUBGROUP_NON_UNIFORM_ARITHMETIC, CL_MAKE_VERSION(1, 0, 0)},
    {OCL_EXT_INTEL_SPIRV_SUBGROUPS, CL_MAKE_VERSION(1, 0, 0)},

    {OCL_EXT_KHR_GLOBAL_BASE_ATOMICS, CL_MAKE_VERSION(1, 0, 0)},
    {OCL_EXT_KHR_GLOBAL_EXTENDED_ATOMICS, CL_MAKE_VERSION(1, 0, 0)},
    {OCL_EXT_KHR_LOCAL_BASE_ATOMICS, CL_MAKE_VERSION(1, 0, 0)},
    {OCL_EXT_KHR_LOCAL_EXTENDED_ATOMICS, CL_MAKE_VERSION(1, 0, 0)},
    {OCL_EXT_KHR_INT64_BASE_ATOMICS, CL_MAKE_VERSION(1, 0, 0)},
    {OCL_EXT_KHR_INT64_EXTENDED_ATOMICS, CL_MAKE_VERSION(1, 0, 0)},

    {OCL_EXT_KHR_3D_IMAGE_WRITES, CL_MAKE_VERSION(1, 0, 0)},
    {OCL_EXT_KHR_BYTE_ADDRESSABLE_STORE, CL_MAKE_VERSION(1, 0, 0)},
    {OCL_EXT_KHR_DEPTH_IMAGES, CL_MAKE_VERSION(1, 0, 0)},
    {OCL_EXT_KHR_ICD, CL_MAKE_VERSION(1, 0, 0)},
    {OCL_EXT_KHR_IL_PROGRAM, CL_MAKE_VERSION(1, 0, 0)},

    {OCL_EXT_INTEL_UNIFIED_SHARED_MEMORY, CL_MAKE_VERSION(1, 0, 0)},
    {OCL_EXT_INTEL_DEVICE_ATTRIBUTE_QUERY, CL_MAKE_VERSION(1, 0, 0)},

    {OCL_EXT_INTEL_EXEC_BY_LOCAL_THREAD, CL_MAKE_VERSION(1, 0, 0)},
    {OCL_EXT_INTEL_VEC_LEN_HINT, CL_MAKE_VERSION(1, 0, 0)},
#ifndef _WIN32
    {OCL_EXT_INTEL_DEVICE_PARTITION_BY_NAMES, CL_MAKE_VERSION(1, 0, 0)},
#endif
    {OCL_EXT_KHR_SPIR, CL_MAKE_VERSION(1, 0, 0)},
    {OCL_EXT_KHR_IMAGE2D_FROM_BUFFER, CL_MAKE_VERSION(1, 0, 0)},
    {OCL_EXT_INTEL_DEVICELIB_ASSERT, CL_MAKE_VERSION(1, 0, 0)},
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
  StringRef exts{extensions};
  SmallVector<StringRef, 64> extensionVec;
  SplitString(exts.substr(0, exts.find_first_of('\0')), extensionVec);
  ASSERT_EQ(extensionVec.size(), extRefCPU.size());

  for (const auto &ext : extensionVec)
    ASSERT_TRUE(extRefCPU.count(ext))
        << ext.str() << " is not in reference extensions";
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

TEST_F(CheckExtensions, WithVersion) {
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
  ASSERT_EQ(extsWithVer.size(), extRefCPU.size());

  for (const auto &extVer : extsWithVer) {
    auto it = extRefCPU.find(extVer.name);
    ASSERT_NE(it, extRefCPU.end())
        << extVer.name << " is not in reference extensions";
    ASSERT_EQ(extVer.version, it->second)
        << extVer.name << "(" << makeVersonString(extVer.version) << ")"
        << " is not in reference extensions";
  }
}
