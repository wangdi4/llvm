// INTEL_FEATURE_ESIMD_EMBARGO
// The test verifies DPAS API with hf8, bf8, df types.
// REQUIRES: intel_feature_esimd_embargo

// RUN: %clangxx -fsycl -c -Xclang -emit-llvm %s -o %t
// RUN: %clangxx -fsycl -c -fsycl-device-only -Xclang -emit-llvm %s -o %t
// RUN: sycl-post-link -split-esimd -lower-esimd -S %t -o %t.table
// RUN: FileCheck %s -input-file=%t_esimd_0.ll

#include <sycl/ext/intel/esimd.hpp>
#include <sycl/sycl.hpp>

using namespace sycl::ext::intel::esimd;

namespace xmx = sycl::ext::intel::esimd::xmx;

using hfloat8 = int8_t;
using bfloat8 = int8_t;

constexpr auto bf8 = xmx::dpas_argument_type::bf8;
constexpr auto hf8 = xmx::dpas_argument_type::hf8;
constexpr auto df = xmx::dpas_argument_type::df;

SYCL_ESIMD_FUNCTION SYCL_EXTERNAL void xmx_func();
SYCL_ESIMD_FUNCTION SYCL_EXTERNAL void xmx_func_end();

class EsimdFunctor {
public:
  void operator()() __attribute__((sycl_explicit_simd)) { xmx_func(); }
};

template <typename name, typename Func>
__attribute__((sycl_kernel)) void kernel(Func kernelFunc) {
  kernelFunc();
}

void bar() {
  EsimdFunctor esimdf;
  kernel<class kernel_esimd>(esimdf);
}

template <typename... T> SYCL_ESIMD_FUNCTION SYCL_EXTERNAL void zoo(T... A);

SYCL_ESIMD_FUNCTION SYCL_EXTERNAL void xmx_func() {
  // DPAS: Result(M x N) = A(M x K) * B(K x N)
  // where:
  //   M = RepeatCount;
  //   K = SystolicDepth * OpsPerChannel;
  //   N = ExecutionSize, must be 16 on PVC and 8 on DG2.

  constexpr int M_four = 4;
  constexpr int K_double = 8 * 1;
  constexpr int N_double = 8; // execution size must be 8 for double on PVC

  // CHECK: define dso_local spir_func void @_Z8xmx_funcv()

  { // ======= DPAS DOUBLE ====================================================
    simd<double, M_four *N_double> R = 0;
    simd<double, M_four *N_double> C = 0;

    simd<double, K_double *N_double> B = 0;
    simd<double, M_four *K_double> A = 0;

    // ------------------- DOUBLE: WITH ACC OPERAND ---------------------------
    R = xmx::dpas<8, M_four, double>(C, B, A);
    zoo(R);
    // CHECK: call <32 x double> @llvm.genx.dpas2.v32f64.v32f64.v64f64.v32f64(<32 x double> {{[^,]+}}, <64 x double> {{[^,]+}}, <32 x double> {{[^,]+}}, i32 13, i32 13, i32 8, i32 4, i32 1, i32 1)

    // ------------------- DOUBLE: NO ACC OPERAND -----------------------------
    R = xmx::dpas<8, M_four, double>(B, A);
    zoo(R);
    // CHECK: call <32 x double> @llvm.genx.dpas.nosrc0.v32f64.v64f64.v32f64(<64 x double> {{[^,]+}}, <32 x double> {{[^,]+}}, i32 67636493)
  }

  xmx_func_end();
  // CHECK: call spir_func void @_Z12xmx_func_endv()
}
// end INTEL_FEATURE_ESIMD_EMBARGO
