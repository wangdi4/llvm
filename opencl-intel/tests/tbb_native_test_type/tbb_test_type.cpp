// INTEL CONFIDENTIAL
//
// Copyright 2013-2020 Intel Corporation.
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

#ifndef WIN32
#include <unistd.h>
#endif

#include <tbb/tbb.h>

struct parallel_functor {
  parallel_functor(std::atomic<long> &counter) : my_counter(counter) {}

  void operator()(const tbb::blocked_range<long> &r) const {
    my_counter--;
    while (my_counter > 0) {
#ifdef WIN32
      _mm_pause();
#else
      usleep(0);
#endif
    }
  }

  std::atomic<long> &my_counter;
};

struct arena_functor {
  arena_functor(parallel_functor &parallel, long n)
      : my_parallel(parallel), my_n(n) {}
  void operator()() {
    tbb::auto_partitioner part;

    tbb::parallel_for(tbb::blocked_range<long>(0, my_n, 1), my_parallel, part);
  }

  parallel_functor &my_parallel;
  long my_n;
};

int main(int argc, char *argv[]) {
  int nThreads = tbb::global_control::active_value(
      tbb::global_control::max_allowed_parallelism);
  std::atomic<long> counter;
  counter = nThreads;
  tbb::task_arena arena(nThreads, 1);

  parallel_functor Fparallel(counter);
  arena_functor Farena(Fparallel, nThreads);

  arena.execute(Farena);

  return 0;
}
