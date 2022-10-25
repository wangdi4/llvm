// INTEL_FEATURE_ESIMD_EMBARGO
// The contents of this test checks for the following PVC_XT+ features:
//     * stochastic rounding (srnd)
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

SYCL_ESIMD_FUNCTION SYCL_EXTERNAL simd<float, 16> foo();

// Note: lines #21-37 are needed to avoid sycl-post-link throwing away "foo"
// (SYCL_EXTERNAL) function.
class EsimdFunctor {
public:
  void operator()() __attribute__((sycl_explicit_simd)) {
    foo();
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

SYCL_ESIMD_FUNCTION SYCL_EXTERNAL simd<float, 16> foo() {
  simd<float, 16> F_srnd1 = 0;
  simd<float, 16> F_srnd2 = 0;
// for now, it seems that we support *half* type only for device target
#if defined(__SYCL_DEVICE_ONLY__)
  simd<_Float16, 16> HF_srnd1 = 0;
  simd<_Float16, 16> HF_srnd2 = 0;
  // fp32 (float) -> half
  // CHECK: call <16 x half> @llvm.genx.srnd.v16f16.v16f32.v16f32(<16 x float> {{[^,]+}}, <16 x float> {{[^)]+}})
  simd<_Float16, 16> F32_srnd_out1 = esimd_srnd<EsimdPrecisionType::FP16>(F_srnd1, F_srnd2);
  // half -> bf8
  // CHECK: call <16 x i8> @llvm.genx.srnd.v16i8.v16f16.v16f16(<16 x half> {{[^,]+}}, <16 x half> {{[^,]+}})
  simd<uint8_t, 16> BF8_srnd_out2 = esimd_srnd<EsimdPrecisionType::BF8>(HF_srnd1, HF_srnd2);
  // fp32 -> bf8 (emulated sequence, fp32 converted to half)
  // CHECK: call <16 x i8> @llvm.genx.srnd.v16i8.v16f16.v16f16(<16 x half> {{[^,]+}}, <16 x half> {{[^,]+}})
  simd<uint8_t, 16> BF8_srnd_out3 = esimd_srnd<EsimdPrecisionType::BF8>(F_srnd1, F_srnd2);
#endif
  return simd<float, 16>();
}

// end INTEL_FEATURE_ESIMD_EMBARGO
