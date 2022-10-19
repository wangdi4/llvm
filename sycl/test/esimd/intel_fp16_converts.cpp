// INTEL_FEATURE_ESIMD_EMBARGO
// The contents of this test checks for the following PVC+ features:
// bfloat16 <-> float conversions
// REQUIRES: intel_feature_esimd_embargo
// UNSUPPORTED: linux

// Checks host+device compilation
// RUN: %clangxx -fsycl -fsyntax-only %s

// Checks that lowerESIMD pass builds proper vc-intrinsics
// RUN: %clangxx -O0 -fsycl -c -fsycl-device-only -Xclang -emit-llvm %s -o %t
// RUN: sycl-post-link -split-esimd -lower-esimd -O0 -S %t -o %t.table
// RUN: FileCheck %s -input-file=%t_esimd_0.ll

#include <CL/sycl.hpp>
#include <sycl/ext/intel/experimental/esimd.hpp>

using namespace sycl::ext::intel::experimental::esimd;

SYCL_ESIMD_FUNCTION SYCL_EXTERNAL void bf16_vector();
SYCL_ESIMD_FUNCTION SYCL_EXTERNAL void bf16_scalar();

class EsimdFunctor {
public:
  void operator()() __attribute__((sycl_explicit_simd)) {
    bf16_vector();
    bf16_scalar();
  }
};

template <typename name, typename Func>
__attribute__((sycl_kernel)) void kernel(Func kernelFunc) {
  kernelFunc();
}

void bar() {
  EsimdFunctor esimdf;
  kernel<class kernel_esimd>(esimdf);
}

SYCL_ESIMD_FUNCTION SYCL_EXTERNAL void bf16_vector() {
  simd<float, 16> F32 = 0;
  simd<uint16_t, 16> BF16 = convert_to_bf16(F32);
  // CHECK: call <16 x half> @llvm.genx.bf.cvt.v16f16.v16f32(<16 x float> {{[^)]+}})
  simd<float, 16> F32_conv = convert_from_bf16(BF16);
  // CHECK: call <16 x float> @llvm.genx.bf.cvt.v16f32.v16f16(<16 x half> {{[^)]+}})
}

SYCL_ESIMD_FUNCTION SYCL_EXTERNAL void bf16_scalar() {
  float F32_scalar = 0;
  uint16_t BF16_scalar = convert_to_bf16(F32_scalar);
  // CHECK: call <1 x f16> @llvm.genx.bf.cvt.v1f16.v1f32(<1 x float> {{[^)]+}})
  float F32_scalar_conv = convert_from_bf16(BF16_scalar);
  // CHECK: call <1 x float> @llvm.genx.bf.cvt.v1f32.v1f16(<1 x f16> {{[^)]+}})
}
// end INTEL_FEATURE_ESIMD_EMBARGO
