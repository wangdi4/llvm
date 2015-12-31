// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-msvc-win32 %s -std=c++11 -emit-llvm -o - -fexceptions -fcxx-exceptions | FileCheck %s
// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu %s -std=c++11 -emit-llvm -o - -fexceptions -fcxx-exceptions | FileCheck %s

struct S{
  int a;
  S() {}
  ~S() {}
};

// CHECK-LABEL: @main
int main() {
  // CHECK: [[S_ADDR:%.+]] = alloca %struct.S,
  // CHECK: [[A_ADDR:%.+]] = alloca i32,
  // CHECK: [[B_ADDR:%.+]] = alloca i32*,
  // CHECK: [[S1_ADDR:%.+]] = alloca %struct.S*,
  // CHECK: [[TMP_ADDR:%.+]] = alloca %struct.S,
  S s;
  const volatile int a = 37;
  // CHECK: load volatile i32, i32* [[A_ADDR]]
  // CHECK: icmp ne i32 %{{.+}}, 0
  // CHECK: br i1
  // CHECK: invoke
  // CHECK: store i32* %{{.+}}, i32** [[B_ADDR]]
  int &&b = a ? a : throw;
  // CHECK: load volatile i32, i32* [[A_ADDR]]
  // CHECK: icmp ne i32 %{{.+}}, 0
  // CHECK: br i1
  // CHECK: call void @llvm.memcpy
  // CHECK: invoke
  // CHECK: store %struct.S* %{{.+}}, %struct.S** [[S1_ADDR]],
  // CHECK: call void
  // CHECK: call void
  S &&s1 = a ? s : throw;
  return b + s1.a;
}
