// RUN: %clang_cc1 -triple i386-unknown-unknown -emit-llvm -o - %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -emit-llvm -o - %s | FileCheck %s

// INTEL_CUSTOMIZATION
typedef float __m256 __attribute__ ((__vector_size__ (32)));

__m256 __attribute__((intel_ocl_bicc)) __svml_f1(void);

void f2(void) {
  __svml_f1();
// CHECK: call svml_unified_cc_256 <8 x float> @__svml_f1()
}

// CHECK: declare svml_unified_cc_256 <8 x float> @__svml_f1()
// end INTEL_CUSTOMIZATION
