// RUN: %clangxx -fsycl -fsycl-targets=%sycl_triple %s -o %t.out
// RUN: %t.out

//==--------------- group.cpp - SYCL group test ----------------------------==//
//
// Test for experimental global offset support.
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include <CL/sycl.hpp>

#include <cassert>
//#include <memory>

using namespace cl::sycl;

int main(int argc, char *argv[]) {
  // Item indexer with offset
  {
    vector_class<int3> data(10, int3{-1});
    const range<1> globalRange(6);
    const id<1> globalOffset(4);
    {
      buffer<int3, 1> b(data.data(), range<1>(10),
                        {property::buffer::use_host_ptr()});
      queue myQueue;
      myQueue.submit([&](handler &cgh) {
        auto B = b.get_access<access::mode::read_write>(cgh);
        cgh.parallel_for<class item1_offset>(
            globalRange, globalOffset, [=](item<1> index) {
              B[index.get_id()] = int3{index.get_id()[0], index.get_range()[0],
                                       index.get_offset()[0]};
            });
      });
    }
    for (int i = 0; i < data.size(); i++) {
      const int id = data[i].s0();
      const int range = data[i].s1();
      const int offset = data[i].s2();
      if (i < globalOffset[0]) {
        assert(id == -1);
        assert(range == -1);
        assert(offset == -1);
      } else {
        assert(id == i);
        assert(range == globalRange[0]);
        assert(offset == globalOffset[0]);
      }
    }
  }

  // ND_Item indexer
  {
    vector_class<int3> data(10, int3{-1});
    const range<1> globalRange(6);
    const range<1> localRange(3);
    const id<1> globalOffset(4);
    const nd_range<1> ndRange(globalRange, localRange, globalOffset);
    {
      buffer<int3, 1> b(data.data(), range<1>(10),
                        {property::buffer::use_host_ptr()});
      queue myQueue;
      myQueue.submit([&](handler &cgh) {
        auto B = b.get_access<access::mode::read_write>(cgh);
        cgh.parallel_for<class item1_nd_range>(ndRange, [=](nd_item<1> index) {
          B[index.get_global_id()] =
              int3{index.get_global_id()[0], index.get_global_range()[0],
                   index.get_offset()[0]};
        });
      });
    }
    for (int i = 0; i < data.size(); i++) {
      const int id = data[i].s0();
      const int range = data[i].s1();
      const int offset = data[i].s2();
      if (i < globalOffset[0]) {
        assert(id == -1);
        assert(range == -1);
        assert(offset == -1);
      } else {
        assert(id == i);
        assert(range == globalRange[0]);
        assert(offset == globalOffset[0]);
      }
    }
  }
  std::cout << "Test passed." << std::endl;
  return 0;
}
