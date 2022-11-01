// INTEL_FEATURE_ESIMD_EMBARGO
// The contents of this test checks for the following PVC+ features:
// bfloat16 <-> float conversions
// REQUIRES: intel_feature_esimd_embargo

// Checks host+device compilation
// RUN: %clangxx -fsycl -fsyntax-only %s

// Checks that lowerESIMD pass builds proper vc-intrinsics
// RUN: %clangxx -O2 -fsycl -c -fsycl-device-only -Xclang -emit-llvm %s -o %t
// RUN: sycl-post-link -split-esimd -lower-esimd -O0 -S %t -o %t.table
// RUN: FileCheck %s -input-file=%t_esimd_0.ll

#include <sycl/ext/intel/esimd.hpp>
#include <sycl/sycl.hpp>

using namespace sycl::ext::intel::esimd;
using namespace sycl::ext::intel::experimental::esimd;

SYCL_ESIMD_FUNCTION SYCL_EXTERNAL void bf16_vector();
SYCL_ESIMD_FUNCTION SYCL_EXTERNAL void bf16_scalar();

using bfloat16 = sycl::ext::oneapi::experimental::bfloat16;

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

SYCL_ESIMD_FUNCTION SYCL_EXTERNAL void bf16_vector() {
  simd<float, 16> F32 = 0;
  simd<bfloat16, 16> BF16 = convert_to_bf16(F32);
  // CHECK: call <16 x half> @llvm.genx.bf.cvt.v16f16.v16f32(<16 x float> {{[^)]+}})
  simd<float, 16> F32_conv = convert_from_bf16(BF16);
  // CHECK: call <16 x float> @llvm.genx.bf.cvt.v16f32.v16f16(<16 x half> {{[^)]+}})

  simd<float, 8> F32B = 0;
  simd<bfloat16, 8> BF16B = F32B;
  // CHECK: call <8 x half> @llvm.genx.bf.cvt.v8f16.v8f32(<8 x float> {{[^)]+}})
  simd<float, 8> F32B_conv = BF16B;
  // CHECK: call <8 x float> @llvm.genx.bf.cvt.v8f32.v8f16(<8 x half> {{[^)]+}})
}

SYCL_ESIMD_FUNCTION SYCL_EXTERNAL void bf16_scalar() {
  float F32_scalar = 0;
  bfloat16 BF16_scalar = convert_to_bf16(F32_scalar);
  // CHECK: call <1 x half> @llvm.genx.bf.cvt.v1f16.v1f32(<1 x float> {{[^)]+}})
  float F32_scalar_conv = convert_from_bf16(BF16_scalar);
  // CHECK: call <1 x float> @llvm.genx.bf.cvt.v1f32.v1f16(<1 x half> {{[^)]+}})

  // Note that this is the compilation test only. It checks that IR is correct.
  // The actual support in GPU RT is on the way though.
  float F32B_scalar = 1;
  bfloat16 BF16B_scalar = F32B_scalar;
  // CHECK: call spir_func noundef zeroext i16 @_Z27__spirv_ConvertFToBF16INTELf(float {{[^)]+}})
  float F32B_scalar_conv = BF16B_scalar;
  // CHECK: call spir_func noundef float @_Z27__spirv_ConvertBF16ToFINTELt(i16 {{[^)]+}})
}
// end INTEL_FEATURE_ESIMD_EMBARGO
