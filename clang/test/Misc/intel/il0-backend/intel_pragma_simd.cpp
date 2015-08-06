// RUN: %clang_cc1 -fintel-compatibility -fcilkplus -emit-llvm -verify -o - %s | FileCheck %s
// expected-no-diagnostics

// CHECK-LABEL: define {{.*float}} @{{.*}}simple{{.*}}(float* {{.+}}, float* {{.+}}, float* {{.+}}, float* {{.+}})
float simple(float *a, float *b, float *c, float *d) {
  float red1 = 0.0;
  int x = 3;
  #pragma simd assert reduction(+:red1)
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"SIMD_LOOP", metadata !"ASSERT", metadata !"REDUCTION", metadata !"+", metadata !"LVALUE", metadata !"SIMPLE", metadata float* {{%.+}}, metadata !"LINEAR", metadata i32* {{%.+}}, metadata i32 1)
  for (int i = 3; i < 32; i += 5) {
    a[i] = b[i] * c[i] * d[i];
    red1 += a[x];
    x += 5;
  }
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"SIMD_END_LOOP")
  return red1;
}

struct S {
  int val;
  S &operator*=(const S& that) { val += that.val; return *this; }
};

// CHECK-LABEL: define{{.+}}with_struct_1
int with_struct_1(void) {
  S red2;
  #pragma simd reduction(* : red2) assert
// call void (metadata, ...) @llvm.intel.pragma(metadata !"SIMD_LOOP", metadata !"REDUCTION", metadata !"*", metadata !"LVALUE", metadata !"SIMPLE", metadata %struct.S* {{%.+}}, metadata !"ASSERT", metadata !"LINEAR", metadata i32* {{%.+}}, metadata i32 1)
  for (int i = 0; i < 100; ++i) {
    S cur;
    cur.val = i;
    red2 *= cur;
  }
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"SIMD_END_LOOP")
  return red2.val;
}

