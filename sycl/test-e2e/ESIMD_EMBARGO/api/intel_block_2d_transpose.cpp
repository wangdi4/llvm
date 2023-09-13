// INTEL_FEATURE_ESIMD_EMBARGO
//==------- intel_block_2d_transpose.cpp  - DPC++ ESIMD on-device test -==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// REQUIRES: intel_feature_esimd_embargo

// See ../intel_README.md for instruction explaining how to use
// Intel Falcon Shores GPU simulator.

// RUN: %{build} -fsycl-device-code-split=per_kernel -DESIMD_EMBARGO -o %t.out
// The test requires FCS (Falcon Shores) emulator or FSC GPU!
// RUN: env IGC_VCApiOptions=-ftranslate-legacy-memory-intrinsics %{run} %t.out

// Tests new requirements for block_2d operations for Falcon Shores

#include <iostream>
#include <sycl/ext/intel/esimd.hpp>
#include <sycl/sycl.hpp>

using namespace sycl;
using namespace sycl::ext::intel::esimd;
using namespace sycl::ext::intel::experimental::esimd;

template <typename T> T get_rand() {
  T v = rand();
  if constexpr (sizeof(T) > 4)
    v = (v << 32) | rand();
  return v;
}

template <typename T, uint32_t Groups, uint32_t Threads, int BlockWidth,
          int BlockHeight = 1>
bool test(unsigned SurfaceWidth, unsigned SurfaceHeight, unsigned SurfacePitch,
          int X, int Y) {
  constexpr int NBlocks = 1;
  constexpr int N = NBlocks * BlockHeight * BlockWidth;
  /* Due to store_2d a is subject to stricter restrictions:
   *   NBlocks always 1, no Transposed, no Transformed, max BlockHeight 8.
   * Series of 2d stores with height 1 are used to write loaded data to output
   * buffer.
   */
  constexpr int SH = BlockHeight;
  constexpr int SW = BlockWidth;
  constexpr int SN = SW;

  std::cout << "N  = " << N << std::endl;
  std::cout << "SN = " << SN << std::endl;
  std::cout << "W  = " << BlockWidth << " SW = " << SW << std::endl;
  std::cout << "H  = " << BlockHeight << " SH = " << SH << std::endl;

  T old_val = get_rand<T>();

  queue q;
  auto dev = q.get_device();

  auto ctx = q.get_context();

  // workgroups
  sycl::range<1> GlobalRange{Groups};
  // threads in each group
  sycl::range<1> LocalRange{Threads};
  sycl::nd_range<1> Range{GlobalRange * LocalRange, LocalRange};

  unsigned SurfaceSize = SurfacePitch * SurfaceHeight * NBlocks;
  unsigned Size = SurfaceSize * Groups * Threads;

  T *out = static_cast<T *>(sycl::malloc_shared(Size * sizeof(T), dev, ctx));
  for (int i = 0; i < Size; i++)
    out[i] = old_val;

  T *in = static_cast<T *>(sycl::malloc_shared(Size * sizeof(T), dev, ctx));
  for (int i = 0; i < Size; i++)
    in[i] = get_rand<T>();

  try {
    auto e = q.submit([&](handler &cgh) {
      cgh.parallel_for(Range, [=](sycl::nd_item<1> ndi) SYCL_ESIMD_KERNEL {
        uint16_t globalID = ndi.get_global_id(0);
        uint32_t off = globalID * SurfaceSize;

        unsigned width = SurfaceWidth * sizeof(T) - 1;
        unsigned height = SurfaceHeight - 1;
        unsigned pitch = SurfacePitch * sizeof(T) - 1;

        simd<T, N> vals;

        vals = lsc_load_2d<T, BlockWidth, BlockHeight, NBlocks, true>(
            in + off, width, height, pitch, X, Y);

        for (int i = 0; i < NBlocks; i++) {
          for (int j = 0; j < SH; j++) {
            simd<T, SN> v = vals.template select<SN, 1>(i * SN * SH + j * SW);
            lsc_store_2d<T, SW>(out + off, SurfaceWidth * sizeof(T) - 1,
                                SurfaceHeight - 1, SurfacePitch * sizeof(T) - 1,
                                X + i * SW, Y + j, v);
          }
        }
      });
    });
    e.wait();
  } catch (sycl::exception const &e) {
    std::cout << "SYCL exception caught: " << e.what() << '\n';
    sycl::free(out, ctx);
    sycl::free(in, ctx);
    return false;
  }

  bool passed = true;
  int ErrorCount = 0;

  for (int gid = 0; gid < Groups * Threads; gid++) {
    int dx = 0, dy = 0;
    for (int j = 0; j < SurfaceHeight; j++) {
      for (int i = 0; i < SurfacePitch; i++) {
        T e = old_val;
        // index in linear buffer
        int idx = i + j * SurfacePitch + gid * SurfaceSize;

        // check if inside block
        if ((i >= X) && (i < X + BlockWidth) && (j >= Y) &&
            (j < Y + BlockHeight)) {
          if (i < SurfaceWidth) {
            if (X + dx < SurfaceWidth)
              e = in[(X + dx) + (Y + dy) * SurfacePitch + gid * SurfaceSize];
            else
              e = (T)0;
          }
          dy += 1;
          if (dy == BlockHeight) {
            dy = 0;
            dx += 1;
          }
        }

        if (out[idx] != e) {
          passed = false;
          if (ErrorCount++ < 10) {
            std::cout << "out[" << idx << "] = 0x" << std::hex << out[idx]
                      << " vs etalon = 0x" << e << std::dec << std::endl;
          }
        }
      }
    }
  }

  if (!passed)
    std::cout << " FAILED" << std::endl;

  sycl::free(out, ctx);
  sycl::free(in, ctx);

  return passed;
}

int main(void) {
  constexpr uint32_t seed = 322;
  srand(seed);
  bool passed = true;
  passed &= test<uint64_t, 1, 1, 4, 8>(16, 10, 16, 1, 2);
  passed &= test<uint32_t, 1, 1, 16, 8>(32, 10, 32, 1, 2);

  passed &= test<uint64_t, 2, 2, 4, 8>(16, 10, 16, 1, 2);
  passed &= test<uint32_t, 3, 2, 16, 8>(32, 10, 32, 1, 2);

  std::cout << (passed ? "Test PASSED\n" : "Test FAILED\n");
  return passed ? 0 : 1;
}
// end INTEL_FEATURE_ESIMD_EMBARGO
