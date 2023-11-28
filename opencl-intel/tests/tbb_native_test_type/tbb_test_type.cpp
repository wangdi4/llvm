// INTEL CONFIDENTIAL
//
// Copyright 2013 Intel Corporation.
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

#include "tbb_test_type.h"
#include "task_group_with_reference.h"
#include "tbb/task_arena.h"
#include <atomic>
#include <memory>

#ifndef _WIN32
#define SLEEP(x) sleep(x)
#else
#define SLEEP(x) Sleep(x)
#endif

void taskArenaTest() {
  int nthreads = tbb::global_control::active_value(
      tbb::global_control::max_allowed_parallelism);
  std::atomic<long> counter(nthreads);
  tbb::task_arena taskArena(nthreads, 1);

  parallel_functor fparallel(counter);
  arena_functor farena(fparallel, nthreads);
  taskArena.execute(farena);
}

void taskGroupDestructionTest() {
  auto taskArena = std::make_unique<tbb::task_arena>();
  auto taskGroup = std::make_unique<task_group_with_reference>();

  // Run a task and sleep 3 seconds
  taskArena->execute([&] { taskGroup->run([&] { SLEEP(3); }); });
  // Call destructor before task finished
  taskGroup.reset();
}

void taskGroupReferenceCountTest() {
  const unsigned int COUNT = 1000;
  task_group_with_reference taskGroup;

  // Increase reference count
  for (unsigned i = 0; i < COUNT; ++i)
    taskGroup.reserve_wait();

  EXPECT_EQ(taskGroup.ref_count(), COUNT);

  // Decrease reference count
  for (unsigned i = 0; i < COUNT; ++i)
    taskGroup.release_wait();

  EXPECT_EQ(taskGroup.ref_count(), 0U);
}

TEST(TBBNativeTest, Test_taskArena) { taskArenaTest(); }

TEST(TBBNativeTest, Test_taskGroupDestruction) { taskGroupDestructionTest(); }

TEST(TBBNativeTest, Test_taskGroupReferenceCount) {
  taskGroupReferenceCountTest();
}

int main(int argc, char *argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
