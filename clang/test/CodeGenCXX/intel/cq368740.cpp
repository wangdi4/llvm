// RUN: %clang_cc1 -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

void foo1(int i);
void foo2(int i) {
  i = i - 1;
  return;
}

extern "C" void foo3() { ; }
extern "C" void foo4() { ; }

#pragma weak foo1 = foo2
#pragma weak foo3 = foo4

int main() {
  foo1(5);
  foo3();
  return 0;
}

// CHECK: {{.*}}foo1{{.*}} = weak alias {{.*}}foo2{{.*}}
// CHECK-NOT: {{.*}}foo3{{.*}} = weak alias
