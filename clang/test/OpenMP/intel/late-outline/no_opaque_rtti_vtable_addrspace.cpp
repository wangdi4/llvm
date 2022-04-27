// INTEL_COLLAB
// RUN: %clang_cc1 -no-opaque-pointers -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -no-opaque-pointers -verify -triple spir64 -fopenmp \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s
//
// expected-no-diagnostics
// CHECK: [[STD:%struct.Derived]] = type { %struct.Base }
// CHECK: [[STB:%struct.Base]] = type { i32 (...)* addrspace(4)* }
// CHECK: [[TVD:@_ZTV7Derived]] = linkonce_odr target_declare unnamed_addr addrspace(1) constant { [4 x i8 addrspace(4)*] } { [4 x i8 addrspace(4)*] [i8 addrspace(4)* null, i8 addrspace(4)* addrspacecast (i8* bitcast ({ i8 addrspace(4)*, i8 addrspace(4)*, i8 addrspace(4)* }* @_ZTI7Derived to i8*) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8* bitcast (void (%struct.Derived addrspace(4)*)* @_ZN7DerivedD1Ev to i8*) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8* bitcast (void (%struct.Derived addrspace(4)*)* @_ZN7DerivedD0Ev to i8*) to i8 addrspace(4)*)] }
// CHECK: [[TV:@_ZTVN10__cxxabiv120__si_class_type_infoE]] = external addrspace(1) global i8 addrspace(4)*
// CHECK: [[TV1:@_ZTVN10__cxxabiv117__class_type_infoE]] = external addrspace(1) global i8 addrspace(4)*
// CHECK: [[TIB:@_ZTI4Base]] = linkonce_odr constant { i8 addrspace(4)*, i8 addrspace(4)* } { i8 addrspace(4)* bitcast (i8 addrspace(4)* addrspace(4)* getelementptr inbounds (i8 addrspace(4)*, i8 addrspace(4)* addrspace(4)* addrspacecast (i8 addrspace(4)* addrspace(1)* @_ZTVN10__cxxabiv117__class_type_infoE to i8 addrspace(4)* addrspace(4)*), i64 2) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8 addrspace(1)* getelementptr inbounds ([6 x i8], [6 x i8] addrspace(1)* @_ZTS4Base, i32 0, i32 0) to i8 addrspace(4)*) }
// CHECK: [[TID:@_ZTI7Derived]] = linkonce_odr constant { i8 addrspace(4)*, i8 addrspace(4)*, i8 addrspace(4)* } { i8 addrspace(4)* bitcast (i8 addrspace(4)* addrspace(4)* getelementptr inbounds (i8 addrspace(4)*, i8 addrspace(4)* addrspace(4)* addrspacecast (i8 addrspace(4)* addrspace(1)* @_ZTVN10__cxxabiv120__si_class_type_infoE to i8 addrspace(4)* addrspace(4)*), i64 2) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8 addrspace(1)* getelementptr inbounds ([9 x i8], [9 x i8] addrspace(1)* @_ZTS7Derived, i32 0, i32 0) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8* bitcast ({ i8 addrspace(4)*, i8 addrspace(4)* }* @_ZTI4Base to i8*) to i8 addrspace(4)*) }
// CHECK: [[VTB:@_ZTV4Base]] = linkonce_odr target_declare unnamed_addr addrspace(1) constant { [4 x i8 addrspace(4)*] } { [4 x i8 addrspace(4)*] [i8 addrspace(4)* null, i8 addrspace(4)* addrspacecast (i8* bitcast ({ i8 addrspace(4)*, i8 addrspace(4)* }* [[TIB]] to i8*) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8* bitcast (void (%struct.Base addrspace(4)*)* @_ZN4BaseD1Ev to i8*) to i8 addrspace(4)*), i8 addrspace(4)* addrspacecast (i8* bitcast (void ([[STB]] addrspace(4)*)* @_ZN4BaseD0Ev to i8*) to i8 addrspace(4)*)] }
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
