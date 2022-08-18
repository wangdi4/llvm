// INTEL_FEATURE_SW_DTRANS
// REQUIRES: intel_feature_sw_dtrans

// RUN: %clang_cc1 -flto -flto-unit -fwhole-program-vtables \
// RUN:  -fintel-compatibility -triple x86_64-unknown-linux \
// RUN:  -emit-llvm -opaque-pointers -o - %s \
// RUN:  | FileCheck --check-prefix=CHECK-LINUX --check-prefix=CHECK-BOTH %s

// RUN: %clang_cc1 -flto -flto-unit -fwhole-program-vtables \
// RUN:  -fintel-compatibility -triple x86_64-pc-windows-msvc \
// RUN:  -emit-llvm -opaque-pointers -o - %s \
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
  // CHECK-BOTH: %[[A_ADDR:.+]] = alloca ptr
  // CHECK-BOTH: %[[A_ADDR_LD:.+]] = load ptr, ptr %[[A_ADDR]]
  // CHECK-BOTH: %[[VTBL:.+]] = load ptr, ptr %[[A_ADDR_LD]]
  // CHECK-BOTH: [[N:%[0-9]+]] = call {{.*}}llvm.intel.wholeprogramsafe
  // CHECK-BOTH: br i1 [[N]], label
  // CHECK-BOTH: call {{.*}}llvm.{{.*}}type.test
  // CHECK-BOTH: call {{.*}}llvm.assume
  // CHECK-BOTH: br label
  a->f();
  // CHECK-BOTH: %[[VFN:.+]] = getelementptr inbounds ptr, ptr %[[VTBL]], i64 0
  // CHECK-BOTH: %[[VFN_LOAD:.+]] = load ptr, ptr %[[VFN]]
  // CHECK-BOTH: call {{.*}}void %[[VFN_LOAD]]
  // CHECK-BOTH: ret void
}

// end INTEL_FEATURE_SW_DTRANS
