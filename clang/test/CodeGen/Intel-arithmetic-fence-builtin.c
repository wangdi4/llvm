// Test with fast math
// RUN: %clang_cc1 -triple x86_64-linux-gnu -emit-llvm -mreassociate \
// RUN: -o - %s | FileCheck --check-prefixes CHECK,CHECK64 %s

// RUN: %clang_cc1 -triple i386-pc-linux-gnu -emit-llvm -mreassociate \
// RUN: -o - %s | FileCheck --check-prefixes CHECK,CHECK32 %s

int v;
int test() {
  // CHECK: define dso_local i32 @test()
  int *p;
  int i = __fence(1 + 2);
  // CHECK: [[P:%.*]] = alloca ptr
  // CHECK-NEXT: [[I:%.*]] = alloca i32
  // CHECK-NEXT: [[K:%.*]] = alloca ptr
  // CHECK-NEXT: [[L:%.*]] = alloca ptr
  // CHECK-NEXT: store i32 3, ptr [[I]]

  int *k = __fence(i + p);
  int **l = __fence(&p);
  // CHECK-NEXT: [[TMP0:%.*]] = load i32, ptr [[I]]
  // CHECK-NEXT: [[TMP1:%.*]] = load ptr, ptr [[P]]

  // CHECK64: [[IDX:%.*]] = sext i32 [[TMP0]] to i64
  // CHECK64-NEXT: [[ADD64:%.*]] = getelementptr inbounds i32, ptr [[TMP1]], i64 [[IDX]]
  // CHECK64-NEXT: store ptr [[ADD64]], ptr [[K]]

  // CHECK32: [[ADD32:%.*]] = getelementptr inbounds i32, ptr [[TMP1]], i32 [[TMP0]]
  // CHECK32-NEXT: store ptr [[ADD32]], ptr [[K]]

  // CHECK: store ptr [[P]], ptr [[L]]

  return 0;
  // CHECK-NEXT ret i32 0




}
