// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:  -fopenmp-targets=spir64 -emit-llvm-bc %s -o %t-host.bc

// RUN: %clang_cc1 -opaque-pointers -verify -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline -fopenmp-typed-clauses \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s
//
// expected-no-diagnostics
namespace PR6251 {


template<typename T>
struct A { A(); };

struct B : virtual A<char> { };
struct C : virtual A<char> { };

struct D : B, C  {
#pragma omp declare target
// CHECK-LABEL: @_ZN6PR62511DC2Ev(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[THIS_ADDR:%.*]] = alloca ptr addrspace(4), align 8
// CHECK-NEXT: [[VTT_ADDR:%vtt.addr]] = alloca ptr addrspace(4)
// CHECK-NEXT:    [[THIS_ADDR_ASCAST:%.*]] = addrspacecast ptr [[THIS_ADDR]] to ptr addrspace(4)
// CHECK-NEXT: [[VTT_ADDR_ASCAST:%vtt.addr.ascast]] = addrspacecast ptr [[VTT_ADDR]] to ptr addrspace(4)
// CHECK: %vtt2 = load ptr addrspace(4), ptr addrspace(4) [[VTT_ADDR_ASCAST]], align 8
// CHECK:    ret void
  D();
};

//
D::D() { }

// CHECK-LABEL: @_ZN6PR62514testEv(
// CHECK-NEXT:  entry:
// CHECK-NEXT:    [[D:%.*]] = alloca %"struct.PR6251::D", align 8
// CHECK-NEXT:    [[D_ASCAST:%.*]] = addrspacecast ptr [[D]] to ptr addrspace(4)
// CHECK-NEXT:    [[TMP0:%.*]] = call token @llvm.directive.region.entry() [ "DIR.OMP.TARGET"(), "QUAL.OMP.OFFLOAD.ENTRY.IDX"(i32 0), "QUAL.OMP.PRIVATE:TYPED"(ptr addrspace(4) [[D_ASCAST]]
// CHECK-NEXT:    call spir_func void @_ZN6PR62511DC1Ev(ptr addrspace(4) {{.*}} [[D_ASCAST]]) #[[ATTR5:[0-9]+]]
// CHECK-NEXT:    call void @llvm.directive.region.exit(token [[TMP0]]) [ "DIR.OMP.END.TARGET"() ]
// CHECK-NEXT:    ret void
//
void test() {
  #pragma omp target
  D d;
}

}
// end INTEL_COLLAB
