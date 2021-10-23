// INTEL_FEATURE_ESIMD_EMBARGO
// The contents of this test checks for PVC+ features
// REQUIRES: intel_feature_esimd_embargo
// RUN: %clangxx -DESIMD_GEN12_7 -O0 -fsycl -c -Xclang -emit-llvm %s -o %t
// RUN: %clangxx -DESIMD_GEN12_7 -O0 -fsycl -c -fsycl-device-only -Xclang -emit-llvm %s -o %t
// RUN: sycl-post-link -split-esimd -lower-esimd -O0 -S %t -o %t.table
// RUN: FileCheck %s -input-file=%t_esimd_0.ll

#include <CL/sycl.hpp>
#include <sycl/ext/intel/experimental/esimd.hpp>

using namespace sycl::ext::intel::experimental::esimd;

SYCL_ESIMD_FUNCTION SYCL_EXTERNAL simd<float, 16> foo();

class EsimdFunctor {
public:
  void operator()() __attribute__((sycl_explicit_simd)) { foo(); }
};

template <typename name, typename Func>
__attribute__((sycl_kernel)) void kernel(Func kernelFunc) {
  kernelFunc();
}

void bar() {
  EsimdFunctor esimdf;
  kernel<class kernel_esimd>(esimdf);
}

SYCL_ESIMD_FUNCTION SYCL_EXTERNAL simd<float, 16> foo() {
  simd<char, 512> B_SRC1 = 0;
  simd<char, 32> B_SRC2  = 0;
  simd<short, 16> B_ACC  = 7;
  simd<float, 16> B_DST  = 0;
  simd<int, 128> B_ISRC1 = 0;
  simd<int, 8> B_ISRC2   = 0;
  B_DST = esimd_dpas<EsimdPrecisionType::BF8, EsimdPrecisionType::BF8, float, 8, 1>
      (B_ACC, B_ISRC1, B_ISRC2);
  // @llvm.genx.dpas2.v16f32.v16i16.v128i32.v8i32(<16 x i16> %9, <128 x i32> %11, <8 x i32> %13, i32 11, i32 11, i32 8, i32 1, i32 1, i32 1)
  // CHECK: call <16 x float> @llvm.genx.dpas2.v16f32.v16i16.v128i32.v8i32(<16 x i16> {{[^,]+}}, <128 x i32> {{[^,]+}}, <8 x i32> {{[^,]+}}, i32 11, i32 11, i32 8, i32 1, i32 1, i32 1)
  return B_DST;
}
// end INTEL_FEATURE_ESIMD_EMBARGO
