//
// Copyright (C) 2023 Intel Corporation
//
// This software and the related documents are Intel copyrighted materials, and
// your use of them is governed by the express license under which they were
// provided to you ("License"). Unless the License provides otherwise, you may
// not use, modify, copy, publish, distribute, disclose or transmit this
// software or the related documents without Intel's prior written permission.
//
// This software and the related documents are provided as is, with no express
// or implied warranties, other than those that are expressly stated in the
// License.
//

#include "CL/cl.h"
#include "simple_fixture.h"
#include <numeric>
#include <string>
#include <vector>

class TestChannelMultiDimArray : public OCLFPGASimpleFixture {};

TEST_F(TestChannelMultiDimArray, Basic) {
  const std::string program_source = R"(
    #pragma OPENCL EXTENSION cl_intel_channels : enable
    channel int b[5][4][3][2];

    __attribute__((max_global_work_dim(0)))
    kernel void host_reader(
        global int *data, int n) {
      for (int i = 0; i < n; ++i) {
        while (!write_channel_nb_intel(b[4][3][2][1], data[i]))
          ;
      }
    }

    __attribute__((max_global_work_dim(0)))
    kernel void host_writer(
        global int *data, int n) {
      bool valid = false;
      for (int i = 0; i < n; ++i) {
        do {
          data[i] = read_channel_nb_intel(b[4][3][2][1], &valid);
        } while (!valid);
      }
    }
  )";

  ASSERT_TRUE(createAndBuildProgram(program_source))
      << "createAndBuildProgram failed";

  int n = 1000;
  std::vector<int> data(n);
  std::iota(data.begin(), data.end(), 0);

  cl_mem input_buffer = createBuffer<int>(
      n, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, &data.front());
  ASSERT_NE(nullptr, input_buffer) << "createBuffer failed";

  cl_mem output_buffer = createBuffer<int>(n, CL_MEM_WRITE_ONLY);
  ASSERT_NE(nullptr, output_buffer) << "createBuffer failed";

  ASSERT_TRUE(enqueueTask("host_reader", input_buffer, n))
      << "enqueueTask failed";
  ASSERT_TRUE(enqueueTask("host_writer", output_buffer, n))
      << "enqueueTask failed";

  std::vector<int> result(n);
  ASSERT_TRUE(readBuffer<int>("host_writer", output_buffer, n, &result.front()))
      << "readBuffer failed";

  for (int i = 0; i < n; ++i)
    ASSERT_EQ(data[i], result[i]) << " Data is differ at index " << i;
}
