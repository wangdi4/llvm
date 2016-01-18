// RUN: %clang_cc1 -triple x86_64-linux-gnu -emit-llvm -o - -verify %s | FileCheck %s
// expected-no-diagnostics

// CQ#377481. Check for '__builtin_clrsb' and '__builtin_clrsbll' builtins.
int test_cq377481(int val, long l, long long x) {
  int res = __builtin_clrsbll(x);
  // CHECK: %{{.*}} = call i64 @llvm.clrsb.i64(i64 %{{.*}})
  res += __builtin_clrsbl(l);
  // CHECK: %{{.*}} = call i64 @llvm.clrsb.i64(i64 %{{.*}})
  res += __builtin_clrsb(0xf0000000);
  // CHECK: %{{.*}} = add nsw i32 %{{.*}}, 3
  res += __builtin_clrsb(0);
  // CHECK: %{{.*}} = add nsw i32 %{{.*}}, 31
  return __builtin_clrsb(val) + res;
  // CHECK: %{{.*}} = call i32 @llvm.clrsb.i32(i32 %{{.*}})
}
