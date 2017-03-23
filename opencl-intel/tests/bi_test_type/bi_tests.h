#ifndef __BI_TESTS_H__
#define __BI_TESTS_H__

#include "gtest/gtest.h"
#include "CL/cl.h"

#include <vector>


class BITest : public ::testing::Test {
protected:
  virtual void SetUp();
  virtual void TearDown();

  cl_command_queue createCommandQueue();
  cl_mem createBuffer(size_t size,
                      cl_mem_flags flags = CL_MEM_READ_WRITE,
                      void *host_ptr = nullptr);

  cl_platform_id                platform;
  cl_device_id                  device;
  cl_context                    context;
  std::vector<cl_command_queue> queues;
  std::vector<cl_mem>           buffers;
};

#endif // __BI_TESTS_H__
