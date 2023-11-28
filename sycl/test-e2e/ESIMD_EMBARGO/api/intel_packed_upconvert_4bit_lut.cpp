// INTEL_FEATURE_ESIMD_EMBARGO
//==- intel_packed_upconvert_4bit_lut.cpp  - DPC++ ESIMD on-device test -==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===-------------------------------------------------------------------===//
// REQUIRES: intel_feature_esimd_embargo

// RUN: %{build} -fsycl-esimd-force-stateless-mem -o %t.out
// RUN: env IGC_VCApiOptions=-ftranslate-legacy-memory-intrinsics %{run} %t.out

#include "../../ESIMD/esimd_test_utils.hpp"

#include <iostream>
#include <sycl/ext/intel/esimd.hpp>
#include <sycl/sycl.hpp>

using namespace sycl;

using namespace sycl::ext::intel;
using namespace sycl::ext::intel::esimd;

int main() {
  queue q(esimd_test::ESIMDSelector, esimd_test::createExceptionHandler());

  auto dev = q.get_device();
  std::cout << "Running on " << dev.get_info<sycl::info::device::name>()
            << "\n";

  constexpr unsigned Size = 16;
  constexpr unsigned VL = 16;
  uint32_t *Out = new uint32_t[Size];
  for (unsigned i = 0; i < Size; ++i) {
    Out[i] = 0;
  }

  try {
    buffer<uint32_t, 1> bufout(Out, range<1>(Size));

    // We need that many workgroups
    range<1> GlobalRange{Size / VL};

    // We need that many threads in each group
    range<1> LocalRange{1};

    auto e = q.submit([&](handler &cgh) {
      auto POut = bufout.template get_access<access::mode::write>(cgh);
      cgh.parallel_for(
          GlobalRange * LocalRange, [=](id<1> i) SYCL_ESIMD_KERNEL {
            unsigned int offset = i * VL * sizeof(uint32_t);

            simd<uint32_t, 16> lut(
                {0x00000000, 0x3f803f80, 0x40004000, 0x40804080, 0x41004100,
                 0x41804180, 0x42004200, 0x42804280, 0x7fff7fff, 0xbf80bf80,
                 0xc000c000, 0xc080c080, 0xc100c100, 0xc180c180, 0xc200c200,
                 0xc280c280});

            simd<uint32_t, 16> src(0, i);

            simd<uint8_t, 16 * 4> src_byte = src.bit_cast_view<uint8_t>();

            simd<uint32_t, 16> res =
                experimental::esimd::packed_4bit_upconvert_lut<0>(lut,
                                                                  src_byte);

            res.copy_to(POut, offset);
          });
    });
    e.wait();
  } catch (sycl::exception const &e) {
    std::cout << "SYCL exception caught: " << e.what() << '\n';

    delete[] Out;
    return 1;
  }

  int err_cnt = 0;

  // TODO: Add error check once we can run the test

  delete[] Out;

  std::cout << (err_cnt > 0 ? "FAILED\n" : "Passed\n");
  return err_cnt;
}

// end INTEL_FEATURE_ESIMD_EMBARGO
