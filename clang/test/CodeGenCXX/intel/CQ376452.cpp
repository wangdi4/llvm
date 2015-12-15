// RUN: %clang_cc1 -fintel-compatibility -verify %s -emit-llvm -o - | FileCheck %s
// expected-no-diagnostics

// CHECK: [[I:@.+]] = {{.+}}constant i32,

struct S
{
    static const int i;
    friend int foobar(int = i);
};

int foobar(int);

// CHECK-LABEL: define i32 @main()
int main() {
  // CHECK: [[I_VAL:%.+]] = load i32, i32* [[I]],
  // CHECK: call i32 @{{.*}}foobar{{.*}}(i32 [[I_VAL]])
  return foobar();
}

