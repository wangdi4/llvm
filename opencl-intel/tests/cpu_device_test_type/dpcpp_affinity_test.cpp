// Copyright (c) 2019 Intel Corporation
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

#include "cl_device_api.h"
#include "cl_sys_info.h"
#include "gtest_wrapper.h"

#ifndef _WIN32
extern IOCLDeviceAgent *dev_entry;
extern std::string SYCLAffinity;
extern std::string SYCLPlace;
extern unsigned int NumProcessors;
extern bool UseHalfProcessors;

class SYCLAffinityTest : public ::testing::Test {};

#if 0 // temporarily disable this case.
TEST_F(SYCLAffinityTest, affinity) {
  const unsigned int *computeUnitMap;
  size_t count;
  dev_entry->clDevGetComputeUnitMap(&computeUnitMap, &count);
  ASSERT_EQ(NumProcessors, (unsigned int)count);

  unsigned int numSockets = Intel::OpenCL::Utils::GetNumberOfCpuSockets();
  unsigned int numCoresPerSocket = NumProcessors / numSockets;
  unsigned int numCoresHalf = NumProcessors / 2;
  unsigned int numUsedProcessors =
      UseHalfProcessors ? numCoresHalf : NumProcessors;

  std::string errMsg =
      "Combination of SYCL_CPU_PLACES=" + SYCLPlace +
      ", SYCL_CPU_CU_AFFINITY=" + SYCLAffinity +
      " and SYCL_CPU_NUM_CUS=" + std::to_string(numUsedProcessors) + "/" +
      std::to_string(NumProcessors) + " failed at index ";
  for (unsigned int i = 0; i < numUsedProcessors; ++i) {
    unsigned int cpuId = i;
    if ("close" == SYCLAffinity) {
      if (UseHalfProcessors)
        cpuId = i * 2;
      else if ("cores" == SYCLPlace)
        cpuId = (i % numCoresHalf) * 2 + (i / numCoresHalf);
    } else if ("spread" == SYCLAffinity) {
      cpuId = (i % numSockets) * numCoresPerSocket;
      if (UseHalfProcessors)
        cpuId += (i / numSockets) * 2;
      else {
        if (Intel::OpenCL::Utils::IsHyperThreadingEnabled() &&
            "cores" == SYCLPlace) {
          cpuId += ((i - (i % numSockets)) % numCoresHalf) / numSockets * 2 +
                   (i / numCoresHalf);
        } else
          cpuId += i / numSockets;
      }
    }
    ASSERT_EQ(computeUnitMap[i], cpuId) << (errMsg + std::to_string(i));
  }
}
#endif
#endif
