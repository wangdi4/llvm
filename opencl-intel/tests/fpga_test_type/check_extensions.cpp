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
#include <algorithm>
#include <iterator>
#include <sstream>

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

  ASSERT_TRUE(extensionsPlatform == extensions)
      << "Expected that platform and device extensions are equal!";

  // Reference FPGA extensions. Update this list if supported extension names
  // changes.
  std::set<std::string> extRefFPGA = {"cl_khr_icd",
                                      "cl_khr_byte_addressable_store",
                                      "cl_intel_fpga_host_pipe",
                                      "cles_khr_int64",
                                      "cl_khr_il_program",
                                      "cl_khr_global_int32_base_atomics",
                                      "cl_khr_global_int32_extended_atomics",
                                      "cl_khr_local_int32_base_atomics",
                                      "cl_khr_local_int32_extended_atomics"};

  // Remove trailing '\0'
  extensions.erase(std::find(extensions.begin(), extensions.end(), '\0'),
                   extensions.end());
  // Check each queried extension is present in reference extensions
  std::istringstream extss(extensions);
  for (auto i = std::istream_iterator<std::string>(extss),
            e = std::istream_iterator<std::string>();
       i != e; ++i) {
    ASSERT_EQ(extRefFPGA.count(*i), 1U)
        << ("Expect " + (*i) + " exists once in reference extensions");
    extRefFPGA.erase(*i);
  }

  // Check extRefFPGA is empty
  if (!extRefFPGA.empty()) {
    std::ostringstream ss;
    std::copy(extRefFPGA.begin(), extRefFPGA.end(),
              std::ostream_iterator<std::string>(ss, ","));
    FAIL() << ("Reference extensions " + ss.str() +
               " are not in extensions queried from clGetDeviceInfo");
  }
}
