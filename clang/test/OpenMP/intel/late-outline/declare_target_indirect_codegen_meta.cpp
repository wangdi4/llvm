// INTEL_COLLAB
//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fopenmp-version=51 \
//RUN:  -emit-llvm-bc -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline \
//RUN:  -Werror -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -triple spir64 -fopenmp-version=51 \
//RUN:  -aux-triple x86_64-unknown-linux-gnu \
//RUN:  -emit-llvm -disable-llvm-passes \
//RUN:  -fopenmp -fopenmp-targets=spir64 \
//RUN:  -fopenmp-late-outline \
//RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_host.bc \
//RUN:  -Wsource-uses-openmp -o - %s | FileCheck %s -check-prefix TARG

// expected-no-diagnostics

#pragma omp begin declare target indirect
template <typename T>
T baz(void *) { return T(); }
#pragma omp end declare target

constexpr bool f() {return false;}
#pragma omp begin declare target indirect(f())
void zoo(int *) {}
void xoo() {}
#pragma omp end declare target
#pragma omp declare target to(zoo) indirect(true)

struct C {
#pragma omp declare target
  void virtual foo() {};
};

void test_5(C *p, void (C::*q)(void)) {
  #pragma omp target
  (p->*q)();
}
typedef  void (C::*PMFn)();
int main()
{
  void *x;
  baz<float>(x);
  baz<int>(x);
  C c;
  C *cp = &c;
  PMFn pf = &C::foo;
  test_5(cp, pf);
}
//
// HOST: !omp_offload.info = !{!0, !1, !2, !3, !4}
// TARG: !omp_offload.info = !{!0, !1, !2, !3, !4}
// HOST: !1 = !{i32 2, !"_Z3bazIfET_Pv", i32 2, float (i8*)* @_Z3bazIfET_Pv}
// TARG: !1 = !{i32 2, !"_Z3bazIfET_Pv", i32 2, float (i8 addrspace(4)*)* @_Z3bazIfET_Pv}
// HOST-NEXT: !2 = !{i32 2, !"_Z3bazIiET_Pv", i32 3, i32 (i8*)* @_Z3bazIiET_Pv}
// TARG-NEXT: !2 = !{i32 2, !"_Z3bazIiET_Pv", i32 3, i32 (i8 addrspace(4)*)* @_Z3bazIiET_Pv}
// HOST-NEXT: !3 = !{i32 2, !"_Z3zooPi", i32 0, void (i32*)* @_Z3zooPi}
// TARG-NEXT: !3 = !{i32 2, !"_Z3zooPi", i32 0, void (i32 addrspace(4)*)* @_Z3zooPi}
// HOST-NEXT: !4 = !{i32 2, !"_ZN1C3fooEv", i32 4, void (%struct.C*)* @_ZN1C3fooEv}
// TARG-NEXT: !4 = !{i32 2, !"_ZN1C3fooEv", i32 4, void (%struct.C addrspace(4)*)* @_ZN1C3fooEv}

// end INTEL_COLLAB
