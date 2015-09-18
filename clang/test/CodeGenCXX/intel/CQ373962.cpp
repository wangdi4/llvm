// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu -emit-llvm -o - -x c++ %s | FileCheck %s

class temp {
public:
  void add(int, int, int);
  void add(int, int) {}
};

void sub(int, int, int) {}
void sub(int, int);

// CHECK-LABEL: main
int main() {
  void (temp::*ptr)(int, int) = &temp::add;
  void (*ptr1)(int, int, int) = &sub;

  // CHECK: store void (i32, i32, i32)* @_Z3subiii, void (i32, i32, i32)** %{{.+}},
  // CHECK: icmp eq i64 %{{.+}}, ptrtoint (void (%class.temp*, i32, i32)* @_ZN4temp3addEii to i64)
  int j = (ptr == &temp::add);
  // CHECK: [[PTR1:%.+]] = load void (i32, i32, i32)*, void (i32, i32, i32)** %{{.+}},
  // CHECK: icmp eq void (i32, i32, i32)* [[PTR1]], @_Z3subiii
  int i = (ptr1 == &sub);
  // CHECK: icmp eq i64 ptrtoint (void (%class.temp*, i32, i32)* @_ZN4temp3addEii to i64), %{{.+}}
  int j1 = (&temp::add == ptr);
  // CHECK: [[PTR1:%.+]] = load void (i32, i32, i32)*, void (i32, i32, i32)** %{{.+}},
  // CHECK: icmp eq void (i32, i32, i32)* @_Z3subiii, [[PTR1]]
  int i1 = (&sub == ptr1);
  return 0;
}
