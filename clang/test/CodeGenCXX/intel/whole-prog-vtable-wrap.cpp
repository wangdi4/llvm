// RUN: %clang_cc1 -flto -flto-unit -fwhole-program-vtables \
// RUN:  -fintel-compatibility -triple x86_64-unknown-linux \
// RUN:  -emit-llvm -o - %s \
// RUN:  | FileCheck --check-prefix=CHECK-LINUX --check-prefix=CHECK-BOTH %s

// RUN: %clang_cc1 -flto -flto-unit -fwhole-program-vtables \
// RUN:  -fintel-compatibility -triple x86_64-pc-windows-msvc \
// RUN:  -emit-llvm -o - %s \
// RUN:  | FileCheck --check-prefix=CHECK-MS --check-prefix=CHECK-BOTH %s

struct A { A(); virtual void f(); };
struct B : virtual A { B(); virtual void g(); virtual void h(); };
struct C : virtual A { C(); };

namespace {
  struct D : B, C { D(); virtual void f(); virtual void h(); };
}

A::A() {}
B::B() {}
C::C() {}
D::D() {}

void A::f() { }
void B::g() { }
void D::f() { }
void D::h() { }

// CHECK-LINUX: define {{.*}}_Z2afP1A
// CHECK-MS: define {{.*}}"?af@@YAXPEAUA@@@Z"
void af(A *a) {
  // CHECK-BOTH: [[N:%[0-9]+]] = call {{.*}}llvm.intel.wholeprogramsafe
  // CHECK-BOTH: br i1 [[N]], label
  // CHECK-BOTH: call {{.*}}llvm.type.test
  // CHECK-BOTH: call {{.*}}llvm.assume
  // CHECK-BOTH: br label
  a->f();
  // CHECK-BOTH: call {{.*}}void{{.*}}A*
  // CHECK-BOTH: ret void
}
