//RUN: %clang_cc1 -fintel-compatibility -verify %s
//RUN: %clang_cc1 -triple x86_64-unknown-unknown -fintel-compatibility %s -emit-llvm -o - | FileCheck %s

int main(int a, char **b, char **c, int d, int e) { // expected-warning{{too many parameters (5) for 'main': must be 0, 2, or 3}} 
  return 0;

  // CHECK: [[RET:%.+]] = alloca i32
  // CHECK: [[A:%.+]] = alloca i32
  // CHECK: [[B:%.+]] = alloca i8**
  // CHECK: [[C:%.+]] = alloca i8**
  // CHECK: [[D:%.+]] = alloca i32
  // CHECK: [[E:%.+]] = alloca i32

  // CHECK: store i32 0, i32* [[RET]]
  // CHECK: store i32 %{{.+}}, i32* [[A]]
  // CHECK: store i8** %{{.+}}, i8*** [[B]]
  // CHECK: store i8** %{{.+}}, i8*** [[C]]
  // CHECK: store i32 %{{.+}}, i32* [[D]]
  // CHECK: store i32 %{{.+}}, i32* [[E]]
}
