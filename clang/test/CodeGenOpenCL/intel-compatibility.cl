// Check that -fintel-compatibility doesn't change mangling of overloaded functions
// RUN: %clang_cc1 -cl-std=CL2.0 -emit-llvm -o - -triple spir-unknown-unknown %s | FileCheck %s
// RUN: %clang_cc1 -cl-std=CL2.0 -emit-llvm -o - -triple spir-unknown-unknown -fintel-compatibility %s | FileCheck %s

typedef short short4 __attribute__((ext_vector_type(4)));

// CHECK-DAG: declare spir_func <4 x i16> @_Z5clampDv4_sS_S_(<4 x i16> noundef, <4 x i16> noundef, <4 x i16> noundef)
short4 __attribute__ ((overloadable)) clamp(short4 x, short4 minval, short4 maxval);

// CHECK-DAG: declare spir_func <4 x i16> @_Z5clampDv4_sss(<4 x i16> noundef, i16 noundef signext, i16 noundef signext)
short4 __attribute__ ((overloadable)) clamp(short4 x, short minval, short maxval);

void kernel test() {
  short4 e0=0;

  // CHECK-DAG: call spir_func <4 x i16> @_Z5clampDv4_sss(<4 x i16> noundef zeroinitializer, i16 noundef signext 0, i16 noundef signext 255)
  clamp(e0, 0, 255);
  // CHECK-DAG: call spir_func <4 x i16> @_Z5clampDv4_sS_S_(<4 x i16> noundef zeroinitializer, <4 x i16> noundef zeroinitializer, <4 x i16> noundef zeroinitializer)
  clamp(e0, e0, e0);
}
