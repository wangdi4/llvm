// INTEL_FEATURE_ESIMD_EMBARGO
//==------- intel_prefetch_2d.cpp.cpp - DPC++ ESIMD on-device test ---------==//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===/
// REQUIRES: intel_feature_esimd_embargo

// See ../intel_README.md for instruction explaining how to use
// Intel Falcon Shores GPU simulator.

// RUN: %{build} -fsycl-device-code-split=per_kernel -DESIMD_EMBARGO -o %t.out
// The test requires FCS (Falcon Shores) emulator or FSC GPU!
// RUNx: env IGC_VCApiOptions=-ftranslate-legacy-memory-intrinsics %{run} %t.ou

// The tests verifies new supported width for lsc_prefetch_2d in FS1.
// The simulation support will only arrive on ww46'23, so the test cannot be ru
// just yet.
// The max width for 2d load/store is 64B, and FS1 prefetch allows <=64,
// or 256B, so read 64B-wide block arrays with load/store and prefetch 4 of
// them at once.

#include <sycl/ext/intel/esimd.hpp>
#include <sycl/sycl.hpp>

#include <iostream>

using namespace sycl;
using namespace sycl::ext::intel::esimd;
using namespace sycl::ext::intel::esimd::detail;
using namespace sycl::ext::intel::experimental::esimd;
using namespace sycl::ext::intel::experimental::esimd::detail;

template <typename T> T get_rand() {
  T v = rand();
  if constexpr (sizeof(T) > 4)
    v = (v << 32) | rand();
  return v;
}

template <int case_num, typename T, uint32_t Groups, uint32_t Threads,
          int BlockWidth, int BlockHeight = 1, int NBlocks = 1,
          cache_hint L1H = cache_hint::none, cache_hint L3H = cache_hint::none>
bool test(unsigned SurfaceWidth, unsigned SurfaceHeight, unsigned SurfacePitch,
          int X, int Y) {

  constexpr int N =
      get_lsc_block_2d_data_size<T, NBlocks, BlockHeight, BlockWidth,
                                 false /*Transposed*/, false /*Transformed*/>();
  /* Due to store_2d a is subject to stricter restrictions:
   *   NBlocks always 1, no Transposed, no Transformed, max BlockHeight 8.
   * Series of 2d stores with height 1 are used to write loaded data to output
   * buffer. Also Transformed load_2d extends BlockWidth to the next power of 2
   * and rounds up BlockHeight.
   */
  constexpr int SH = BlockHeight;
  constexpr int SW = BlockWidth;
  constexpr int SN = get_lsc_block_2d_data_size<T, 1u, 1u, SW, false, false>();
  constexpr int ArrayWidth = SW * NBlocks;
  constexpr int NArrays = 4;

  static_assert(ArrayWidth * sizeof(T) * NArrays == 256,
                "Expect to prefetch 256B wide array of blocks");

  std::cout << "N  = " << N << std::endl;
  std::cout << "SN = " << SN << std::endl;
  std::cout << "W  = " << BlockWidth << " SW = " << SW << std::endl;
  std::cout << "H  = " << BlockHeight << " SH = " << SH << std::endl;
  std::cout << "ArrayWidth = " << ArrayWidth << std::endl;

  T old_val = get_rand<T>();

  auto q = queue{gpu_selector_v};
  auto dev = q.get_device();
  std::cout << "Running case #" << case_num << " on "
            << dev.get_info<sycl::info::device::name>() << "\n";
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

        lsc_prefetch_2d<T, BlockWidth, BlockHeight, NBlocks * NArrays, L1H,
                        L3H>(in + off, width, height, pitch, X, Y);

        for (int i = 0; i < NArrays; i++) {
          simd<T, N> vals;
          vals = lsc_load_2d<T, BlockWidth, BlockHeight, NBlocks, false, false,
                             L1H, L3H>(in + off, width, height, pitch,
                                       X + i * ArrayWidth, Y);

          for (int j = 0; j < NBlocks; j++) {
            for (int k = 0; k < SH; k++) {
              simd<T, SN> v = vals.template select<SN, 1>(j * SN * SH + k * SW);
              lsc_store_2d<T, SW>(out + off, SurfaceWidth * sizeof(T) - 1,
                                  SurfaceHeight - 1,
                                  SurfacePitch * sizeof(T) - 1,
                                  X + i * ArrayWidth + j * SW, Y + k, v);
            }
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

  for (int gid = 0; gid < Groups * Threads; gid++) {
    for (int j = 0; j < SurfaceHeight; j++) {
      for (int i = 0; i < SurfacePitch; i++) {
        T e = old_val;
        // index in linear buffer
        int idx = i + j * SurfacePitch + gid * SurfaceSize;

        // check if inside block arrays.
        if ((i >= X) && (i < X + BlockWidth * NBlocks * NArrays) &&
            (i < SurfaceWidth) && (j >= Y) && (j < Y + BlockHeight))
          e = in[idx];

        if (out[idx] != e) {
          passed = false;
          std::cout << "out[" << idx << "] = 0x" << std::hex
                    << (uint64_t)out[idx] << " vs etalon = 0x" << (uint64_t)e
                    << std::dec << std::endl;
        }
      }
    }
  }

  if (!passed)
    std::cout << "Case #" << case_num << " FAILED" << std::endl;

  sycl::free(out, ctx);
  sycl::free(in, ctx);

  return passed;
}

constexpr uint32_t seed = 322;

constexpr cache_hint L1H = cache_hint::cached;
constexpr cache_hint L3H = cache_hint::uncached;

int main(void) {
  srand(seed);
  bool passed = true;

  // Select the parameters to hit 256B width requirement for prefetch.
  // The test routine will prefetch 4 of such blocks and then will read them.
  // sizeof(uint8_t) * 64 block width * 4 array size = 256B.
  passed &= test<0, uint8_t, 1, 1, 64, 8, 1, L1H, L3H>(256, 64, 256, 4, 21);
  // sizeof(uint16_t) * 32 block width * 4 array size = 256B.
  passed &= test<1, uint16_t, 1, 1, 32, 4, 1, L1H, L3H>(128, 16, 128, 2, 1);

  std::cout << (passed ? "Passed\n" : "FAILED\n");
  return passed ? 0 : 1;
}
// end INTEL_FEATURE_ESIMD_EMBARGO
