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

#include "base_fixture.h"
#include "ocl_supported_extensions.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/StringSet.h"

using namespace llvm;

class CheckExtensions : public OCLFPGABaseFixture {};

TEST_F(CheckExtensions, FPGADevice) {
  // Query list of extension names from clGetPlatformInfo/clGetDeviceInfo
  cl_platform_id platform_id = platform();
  cl_device_id device_id = device();

  size_t retSize;
  cl_int err = clGetPlatformInfo(platform_id, CL_PLATFORM_EXTENSIONS, 0,
                                 nullptr, &retSize);
  ASSERT_EQ(CL_SUCCESS, err) << "clGetPlatformInfo failed";

  std::string extensionsPlatform(retSize, '\0');
  err = clGetPlatformInfo(platform_id, CL_PLATFORM_EXTENSIONS,
                          extensionsPlatform.size(), &extensionsPlatform[0],
                          nullptr);
  ASSERT_EQ(CL_SUCCESS, err) << "clGetPlatformInfo failed";

  err = clGetDeviceInfo(device_id, CL_DEVICE_EXTENSIONS, 0, nullptr, &retSize);
  ASSERT_EQ(CL_SUCCESS, err) << "clGetDeviceInfo failed";

  std::string extensions(retSize, '\0');
  err = clGetDeviceInfo(device_id, CL_DEVICE_EXTENSIONS, extensions.size(),
                        &extensions[0], nullptr);
  ASSERT_EQ(CL_SUCCESS, err) << "clGetDeviceInfo failed";

  ASSERT_EQ(extensionsPlatform, extensions)
      << "Expected that platform and device extensions are equal!";

  // Reference FPGA extensions. Update this list if supported extension names
  // changes.
  StringSet<> extRefFPGA{OCL_EXT_ES_KHR_INT64,
                         OCL_EXT_INTEL_CHANNELS,
                         OCL_EXT_INTEL_FPGA_HOST_PIPE,
                         OCL_EXT_INTEL_PROGRAM_SCOPE_HOST_PIPE,
                         OCL_EXT_INTEL_UNIFIED_SHARED_MEMORY,
                         OCL_EXT_KHR_3D_IMAGE_WRITES,
                         OCL_EXT_KHR_BYTE_ADDRESSABLE_STORE,
                         OCL_EXT_KHR_DEPTH_IMAGES,
                         OCL_EXT_KHR_FP64,
                         OCL_EXT_KHR_GLOBAL_BASE_ATOMICS,
                         OCL_EXT_KHR_GLOBAL_EXTENDED_ATOMICS,
                         OCL_EXT_KHR_ICD,
                         OCL_EXT_KHR_IL_PROGRAM,
                         OCL_EXT_KHR_LOCAL_BASE_ATOMICS,
                         OCL_EXT_KHR_LOCAL_EXTENDED_ATOMICS,
                         OCL_EXT_KHR_SPIRV_LINKONCE_ODR};
  StringRef exts{extensions};
  SmallVector<StringRef, 64> extensionVec;
  SplitString(exts.substr(0, exts.find_first_of('\0')), extensionVec);
  ASSERT_EQ(extensionVec.size(), extRefFPGA.size());

  for (const auto &ext : extensionVec)
    ASSERT_TRUE(extRefFPGA.contains(ext))
        << ext.str() << " not found in reference extensions";
}
