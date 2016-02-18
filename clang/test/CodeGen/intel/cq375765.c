// RUN: %clang_cc1 -triple i386-pc-linux -fintel-compatibility -O0 %s -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 -fintel-compatibility -fsyntax-only -verify %s

typedef int (*pf)(int, int);
typedef int __attribute__((regparm(1))) (*pf2)(int, int);

int f1(int a, int b) { return a + b; }
// CHECK: define i32 @f1(i32 %{{.+}}, i32 %{{.+}})
int __attribute__((regparm(1))) f2(int a, int b) { return a - b; }
// CHECK: define i32 @f2(i32 inreg %{{.+}}, i32 %{{.+}})

int main(int argc, char **argv) {
  int x1 = (argc ? f1 : f2)(1, 2); // expected-warning{{function type mismatch: different regparm attributes}}

  // CHECK: [[COND1:%.+]] = select {{.*}} i32 (i32, i32)* @f1, i32 (i32, i32)* @f2
  // CHECK: [[CALL1:%.+]] = call i32 [[COND1]](i32 1, i32 2)
  // CHECK: store i32 [[CALL1]], i32* %x1

  pf func1 = argc ? f1 : f2; // expected-warning{{function type mismatch: different regparm attributes}}

  // CHECK: [[COND2:%.+]] = select {{.*}} i32 (i32, i32)* @f1, i32 (i32, i32)* @f2
  // CHECK: store i32 (i32, i32)* [[COND2]], i32 (i32, i32)** [[FUNC1:%.+]]

  pf2 func2 = argc ? f1 : f2; // expected-warning{{function type mismatch: different regparm attributes}}
                              // expected-warning@-1{{incompatible pointer types initializing 'pf2'}}
  // CHECK: [[COND3:%.+]] = select {{.*}} i32 (i32, i32)* @f1, i32 (i32, i32)* @f2
  // CHECK: store i32 (i32, i32)* [[COND3]], i32 (i32, i32)** [[FUNC2:%.+]]

  int x2 = func1(3, 4);
  // CHECK: [[FF2:%.+]] = load i32 (i32, i32)*, i32 (i32, i32)** [[FUNC1]]
  // CHECK: [[CALL2:%.+]] = call i32 [[FF2]](i32 3, i32 4)
  // CHECK: store i32 [[CALL2]], i32* %x2

  int x3 = func2(5, 6);
  // CHECK: [[FF3:%.+]] = load i32 (i32, i32)*, i32 (i32, i32)** [[FUNC2]]
  // CHECK: [[CALL3:%.+]] = call i32 [[FF3]](i32 inreg 5, i32 6)
  // CHECK: store i32 [[CALL3]], i32* %x3

  return x1 + x2 + x3;
}
