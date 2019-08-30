// CQ#366562/CQ#415958
// RUN: %clang_cc1 -fintel-compatibility -verify %s -DZERO_OK
// RUN: %clang_cc1 -fintel-compatibility -fintel-compatibility-disable=UnrollZero -verify %s
// RUN: %clang_cc1 -fintel-compatibility -fintel-compatibility-enable=UnrollZero -DZERO_OK -emit-llvm %s -o - | FileCheck --check-prefix=CHECK-ZERO_OK %s

#include <stdint.h>

void simple_test() {
  // CHECK: define void @_Z11simple_testv()
  int s = 0;
#ifdef ZERO_OK
#pragma unroll(0) // expected-no-diagnostics
#else
#pragma unroll(0) // expected-error{{invalid value '0'; must be positive}}
#endif
  for (int i = 0; i < 10; ++i) {
    s = s + i;
  }
  // CHECK-ZERO_OK-DAG: br label %{{[a-z.0-9]*}}, !llvm.loop ![[NORM_ZERO:[0-9]]]
}
template <int64_t L>
void size_test() {
  int s = 0;
#ifdef ZERO_OK
#pragma unroll(0) // expected-no-diagnostics
#else
#pragma unroll(0) // expected-error{{invalid value '0'; must be positive}}
#endif
  for (int i = 0; i < 10; ++i) {
    s = s + i;
  }
}

int main() {
  simple_test();

  size_test<0>();
  // CHECK-ZERO_OK-DAG: br label %{{[a-z.0-9]+}}, !llvm.loop ![[TEMPL_ZERO:[0-9]]]
}

// CHECK-DAG: ![[DISABLE_KEY:[0-9]]] = !{!"llvm.loop.unroll.disable"}
// CHECK-DAG: ![[NORM_ZERO]] = distinct !{![[NORM_ZERO]], ![[DISABLE_KEY]]}
// CHECK-DAG: ![[TEMPL_ZERO]] = distinct !{![[TEMPL_ZERO]], ![[DISABLE_KEY]]}
