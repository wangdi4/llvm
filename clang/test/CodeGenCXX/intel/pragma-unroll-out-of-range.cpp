// CQ#366562/CQ#415958
// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-pc-linux-gnu -emit-llvm -o - -o - %s | FileCheck %s
#include <stdint.h>


void simple_test() {
  // CHECK: define void @_Z11simple_testv()
  int s = 0;
#pragma unroll(0)
  for (int i = 0; i < 10; ++i){s = s + i;} 
  // CHECK: br label %{{[a-z.0-9]*}}, !llvm.loop ![[NORM_ZERO:[0-9]]]
#pragma unroll(-3)
  for (int i = 0; i < 10; ++i){s = s + i;} 
  // CHECK: br label %{{[a-z.0-9]+}}, !llvm.loop ![[NORM_NEG:[0-9]]]
#pragma unroll(1844674407370955148)
  for (int i = 0; i < 10; ++i){s = s + i;} 
  // CHECK: br label %{{[a-z.0-9]+}}, !llvm.loop ![[NORM_BIG:[0-9]]]

}
template<int64_t L>
void size_test() {
  int s = 0;
#pragma unroll(L)
  for (int i = 0; i < 10; ++i){s = s + i;} 
}

int main() {
  simple_test();

  size_test<0>();
  // CHECK: define linkonce_odr void @_Z9size_testILl0EEvv()
  // CHECK: br label %{{[a-z.0-9]+}}, !llvm.loop ![[TEMPL_ZERO:[0-9]]]
  size_test<-2>();
  // CHECK: define linkonce_odr void @_Z9size_testILln2EEvv()
  // CHECK: br label %{{[a-z.0-9]+}}, !llvm.loop ![[TEMPL_NEG:[0-9]]]
  size_test<1844674407370955161>();
  // CHECK: define linkonce_odr void @_Z9size_testILl1844674407370955161EEvv()
  // CHECK: br label %{{[a-z.0-9]+}}, !llvm.loop ![[TEMPL_BIG:[0-9]]]
}

// CHECK-DAG: ![[DISABLE_KEY:[0-9]]] = !{!"llvm.loop.unroll.disable"}
// CHECK-DAG: ![[ENABLE_KEY:[0-9]]] = !{!"llvm.loop.unroll.enable"}
// CHECK-DAG: ![[NORM_ZERO]] = distinct !{![[NORM_ZERO]], ![[DISABLE_KEY]]}
// CHECK-DAG: ![[NORM_NEG]] = distinct !{![[NORM_NEG]], ![[ENABLE_KEY]]}
// CHECK-DAG: ![[NORM_BIG]] = distinct !{![[NORM_BIG]], ![[ENABLE_KEY]]}
// CHECK-DAG: ![[TEMPL_ZERO]] = distinct !{![[TEMPL_ZERO]], ![[DISABLE_KEY]]}
// CHECK-DAG: ![[TEMPL_NEG]] = distinct !{![[TEMPL_NEG]], ![[ENABLE_KEY]]}
// CHECK-DAG: ![[TEMPL_BIG]] = distinct !{![[TEMPL_BIG]], ![[ENABLE_KEY]]}
