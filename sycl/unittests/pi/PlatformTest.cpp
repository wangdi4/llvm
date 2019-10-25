//==---- PlatformTest.cpp --- PI unit tests --------------------------------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <CL/sycl/detail/pi.hpp>
#include <gtest/gtest.h>
#include <vector>

namespace {

using namespace cl::sycl;

class PlatformTest : public ::testing::Test {
protected:
  std::vector<pi_platform> _platforms;

  PlatformTest() : _platforms{} { detail::pi::initialize(); };

  ~PlatformTest() override = default;

  void SetUp() {
    ASSERT_NO_FATAL_FAILURE(Test::SetUp());

    const static char *platform_count_key = "PiPlatformCount";

    pi_uint32 platform_count = 0u;

<<<<<<< HEAD
#if INTEL_CUSTOMIZATION
  if (platformCount < 1u)
    return;
#else // INTEL_CUSTOMIZATION
  ASSERT_GT(platformCount, 0u) << "piPlatformsGet found 0 platforms.\n";
#endif // INTEL_CUSTOMIZATION
=======
    // Initialize the logged number of platforms before the following assertion.
    RecordProperty(platform_count_key, platform_count);
>>>>>>> 637a178b7a0fe9ef59665cd330ebfa2e8a97de09

    ASSERT_EQ(RT::piPlatformsGet(0, 0, &platform_count), PI_SUCCESS);

    // Overwrite previous log value with queried number of platforms.
    RecordProperty(platform_count_key, platform_count);

    if (platform_count == 0u) {
      std::cout
          << "WARNING: piPlatformsGet does not find any PI platforms.\n";

      // Do not call into OpenCL below as a platform count of 0 might fail with
      // OpenCL implementations if the platforms pointer is not `nullptr`.
      return;
    }

    _platforms.resize(platform_count, nullptr);

    ASSERT_EQ(RT::piPlatformsGet(_platforms.size(), _platforms.data(), nullptr),
              PI_SUCCESS);
  }
};

TEST_F(PlatformTest, piPlatformsGet) {
  // The PlatformTest::SetUp method is called to prepare for this test case
  // implicitly tests the calls to `piPlatformsGet`.
}

TEST_F(PlatformTest, piPlatformGetInfo) {
  auto get_info_test = [](pi_platform platform, _pi_platform_info info) {
    size_t reported_string_length = 0;
    EXPECT_EQ(RT::piPlatformGetInfo(platform, info, 0u, nullptr,
                                    &reported_string_length),
              PI_SUCCESS);

    // Create a larger result string to catch overwrites.
    std::vector<char> param_value(reported_string_length * 2u, '\0');
    EXPECT_EQ(RT::piPlatformGetInfo(platform, info, param_value.size(),
                                    param_value.data(), 0u),
              PI_SUCCESS)
        << "piPlatformGetInfo for " << RT::platformInfoToString(info)
        << " failed.\n";

    const auto returned_string_length = strlen(param_value.data()) + 1;

    EXPECT_EQ(returned_string_length, reported_string_length)
        << "Returned string length " << returned_string_length
        << " does not equal reported string length " << reported_string_length
        << ".\n";
  };

<<<<<<< HEAD
  pi_uint32 platformCount = 0;
  PI_CALL(RT::piPlatformsGet(0, 0, &platformCount));
#if INTEL_CUSTOMIZATION
  if (platformCount < 1u)
    return;
#endif // INTEL_CUSTOMIZATION
  std::vector<pi_platform> platforms(platformCount);
  PI_CALL(RT::piPlatformsGet(platformCount, platforms.data(), nullptr));

  auto out_string_buffer = std::unique_ptr<char[]>(new char[out_string_size]);
  auto out_string = out_string_buffer.get();

  for (auto i = 0u; i < platformCount; ++i) {
    const auto &platform = platforms[i];
    get_info_test(out_string, platform, PI_PLATFORM_INFO_NAME);
    get_info_test(out_string, platform, PI_PLATFORM_INFO_VENDOR);
    get_info_test(out_string, platform, PI_PLATFORM_INFO_PROFILE);
    get_info_test(out_string, platform, PI_PLATFORM_INFO_VERSION);
    get_info_test(out_string, platform, PI_PLATFORM_INFO_EXTENSIONS);
=======
  for (const auto &platform : _platforms) {
    get_info_test(platform, PI_PLATFORM_INFO_NAME);
    get_info_test(platform, PI_PLATFORM_INFO_VENDOR);
    get_info_test(platform, PI_PLATFORM_INFO_PROFILE);
    get_info_test(platform, PI_PLATFORM_INFO_VERSION);
    get_info_test(platform, PI_PLATFORM_INFO_EXTENSIONS);
>>>>>>> 637a178b7a0fe9ef59665cd330ebfa2e8a97de09
  }
}
} // namespace
