// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -opaque-pointers -verify -triple spir64 -fopenmp \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s
//
// expected-no-diagnostics
// CHECK: [[TVD:@_ZTV7Derived]] = linkonce_odr target_declare unnamed_addr addrspace(1) constant { [4 x ptr addrspace(4)] }
// CHECK-SAME: { [4 x ptr addrspace(4)]
// CHECK-SAME:  [ptr addrspace(4) null,
// CHECK-SAME:   ptr addrspace(4) addrspacecast (ptr @_ZTI7Derived to ptr addrspace(4)),
// CHECK-SAME:   ptr addrspace(4) addrspacecast (ptr @_ZN7DerivedD1Ev to ptr addrspace(4)),
// CHECK-SAME:   ptr addrspace(4) addrspacecast (ptr @_ZN7DerivedD0Ev to ptr addrspace(4))]
// CHECK-SAME: }

// CHECK: [[TV:@_ZTVN10__cxxabiv120__si_class_type_infoE]] = external addrspace(1) global ptr addrspace(4)
// CHECK: [[TV1:@_ZTVN10__cxxabiv117__class_type_infoE]] = external addrspace(1) global ptr addrspace(4)

// CHECK: [[TIB:@_ZTI4Base]] = linkonce_odr constant { ptr addrspace(4), ptr addrspace(4) } {
// CHECK-SAME: ptr addrspace(4) getelementptr inbounds (ptr addrspace(4), ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTVN10__cxxabiv117__class_type_infoE to ptr addrspace(4)), i64 2),
// CHECK-SAME: ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTS4Base to ptr addrspace(4))
// CHECK-SAME: }

// CHECK: [[TID:@_ZTI7Derived]] = linkonce_odr constant { ptr addrspace(4), ptr addrspace(4), ptr addrspace(4) } {
// CHECK-SAME: ptr addrspace(4) getelementptr inbounds (ptr addrspace(4), ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTVN10__cxxabiv120__si_class_type_infoE to ptr addrspace(4)), i64 2),
// CHECK-SAME: ptr addrspace(4) addrspacecast (ptr addrspace(1) @_ZTS7Derived to ptr addrspace(4)),
// CHECK-SAME: ptr addrspace(4) addrspacecast (ptr @_ZTI4Base to ptr addrspace(4))
// CHECK-SAME: }

// CHECK: [[VTB:@_ZTV4Base]] = linkonce_odr target_declare unnamed_addr addrspace(1) constant { [4 x ptr addrspace(4)] } {
// CHECK-SAME: [4 x ptr addrspace(4)]
// CHECK-SAME: [ptr addrspace(4) null,
// CHECK-SAME:  ptr addrspace(4) addrspacecast (ptr [[TIB]] to ptr addrspace(4)),
// CHECK-SAME:  ptr addrspace(4) addrspacecast (ptr @_ZN4BaseD1Ev to ptr addrspace(4)),
// CHECK-SAME:  ptr addrspace(4) addrspacecast (ptr @_ZN4BaseD0Ev to ptr addrspace(4))] }

struct Base {
  virtual ~Base() = default;
};

struct Derived : public Base {
  #pragma omp declare target
  Derived();
  #pragma omp end declare target
};

Derived::Derived() { }

int main(void) {
  #pragma omp target
  {
  }
  return 0;
}

// end INTEL_COLLAB
