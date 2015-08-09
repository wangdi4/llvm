// CQ#373129
// REQUIRES: llvm-backend
// RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility %s -emit-llvm -o - | FileCheck %s

// CHECK: type { i32*, i8* }

template<typename T1, typename T2, int N>
struct AssumeAlignedCorrect {
  T1 a;
  T2 b;

  void check() {
    // CHECK: [[PTRINT1:%.+]] = ptrtoint i32* %{{.+}} to i64
    // CHECK: [[MASKEDPTR1:%.+]] = and i64 [[PTRINT1]], 63
    // CHECK: [[MASKCOND1:%.+]] = icmp eq i64 [[MASKEDPTR1]], 0
    // CHECK: call void @llvm.assume(i1 [[MASKCOND1]])
    __assume_aligned(a, 64);
    // CHECK: [[PTRINT2:%.+]] = ptrtoint i32* %{{.+}} to i64
    // CHECK: [[MASKEDPTR2:%.+]] = and i64 [[PTRINT2]], 31
    // CHECK: [[MASKCOND2:%.+]] = icmp eq i64 [[MASKEDPTR2]], 0
    // CHECK: call void @llvm.assume(i1 [[MASKCOND2]])
    __assume_aligned(a, N);
    // CHECK: [[PTRINT3:%.+]] = ptrtoint i8* %{{.+}} to i64
    // CHECK: [[MASKEDPTR3:%.+]] = and i64 [[PTRINT3]], 0
    // CHECK: [[MASKCOND3:%.+]] = icmp eq i64 [[MASKEDPTR3]], 0
    // CHECK: call void @llvm.assume(i1 [[MASKCOND3]])
    __assume_aligned(b, 1);
  }
};

void test() {
  AssumeAlignedCorrect<int *, char *, 32> correct;
  correct.check();
}
