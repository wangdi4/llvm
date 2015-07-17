// CQ#373129
// REQUIRES: llvm-backend
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility %s -emit-llvm -o - | FileCheck %s

void check(int *a, short *b, char *c) {
// CHECK: [[PTRINT1:%.+]] = ptrtoint
// CHECK: [[MASKEDPTR1:%.+]] = and i64 [[PTRINT1]], 63
// CHECK: [[MASKCOND1:%.+]] = icmp eq i64 [[MASKEDPTR1]], 0
// CHECK: call void @llvm.assume(i1 [[MASKCOND1]])
  __assume_aligned(a, 64);
// CHECK: [[PTRINT2:%.+]] = ptrtoint
// CHECK: [[MASKEDPTR2:%.+]] = and i64 [[PTRINT2]], 31
// CHECK: [[MASKCOND2:%.+]] = icmp eq i64 [[MASKEDPTR2]], 0
// CHECK: call void @llvm.assume(i1 [[MASKCOND2]])
  __assume_aligned(b, 32);
// CHECK: [[PTRINT3:%.+]] = ptrtoint
// CHECK: [[MASKEDPTR3:%.+]] = and i64 [[PTRINT3]], 0
// CHECK: [[MASKCOND3:%.+]] = icmp eq i64 [[MASKEDPTR3]], 0
// CHECK: call void @llvm.assume(i1 [[MASKCOND3]])
  __assume_aligned(c, 1);
}
