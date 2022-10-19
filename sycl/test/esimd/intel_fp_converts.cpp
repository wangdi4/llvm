// INTEL_FEATURE_ESIMD_EMBARGO
// The contents of this test checks for the following PVC+ features:
//  fp32<->bf8 and fp32->tf32 convertion
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
  simd<float, 16> F_srnd12 = 0;
  simd<uint32_t, 16> TF32 = 0;
  simd<uint8_t, 16> out1 = convert_to_bf8(F_srnd12);
  // CHECK: call <16 x i8> @llvm.genx.qf.cvt.v16i8.v16f32(<16 x float> {{[^)]+}})
  simd<float, 16> out2 = convert_from_bf8<float>(out1);
  // CHECK: call <16 x float> @llvm.genx.qf.cvt.v16f32.v16i8(<16 x i8> {{[^)]+}})
  simd<uint32_t, 16> out3 = convert_to_tf32(F_srnd12);
  // CHECK: call <16 x i32> @llvm.genx.tf32.cvt.v16i32.v16f32(<16 x float> {{[^)]+}})
  simd<float, 16> out4 = convert_from_tf32<float>(TF32);
  // Note: tf32->float conversion is essentially a bitcast
  // CHECK: bitcast {{[^ ]+}} addrspace(4)* {{[^ ]+}} to <16 x float> addrspace(4)*
  return out4;
}
// end INTEL_FEATURE_ESIMD_EMBARGO
