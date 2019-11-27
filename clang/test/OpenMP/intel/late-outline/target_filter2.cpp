// INTEL_COLLAB
// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=x86_64-pc-linux-gnu -emit-llvm-bc %s -o %t-host.bc
//
// RUN: %clang_cc1 -verify -triple x86_64-pc-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=x86_64-pc-linux-gnu -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc %s -emit-llvm -o - \
// RUN:  | FileCheck %s
//
// expected-no-diagnostics

void non_target_func() { }
void other_non_target_func() { }
void inside_outside_func() {}

//CHECK-NOT: define {{.*}}non_target
//CHECK: define {{.*}}func_with_target{{.*}} #[[CONTAINS_TARGET:[0-9]+]]
//CHECK-NOT: define {{.*}}non_target
void func_with_target() {
  #pragma omp target
  {
  }
  non_target_func();
}

//CHECK: define {{.*}}another_func_with_target{{.*}} #[[CONTAINS_TARGET]]
//CHECK-NOT: define {{.*}}non_target
void another_func_with_target()
{
  inside_outside_func();
  #pragma omp target
  {
    inside_outside_func();
  }
}
//CHECK: define {{.*}}inside_outside_func{{.*}} #[[DECLARE_TARGET:[0-9]+]]
//CHECK-NOT: define {{.*}}non_target

//CHECK: define {{.*}}another_function_with_lambda_with_target{{.*}}
//CHECK-NOT: define {{.*}}non_target
//Isn't really declare-target or contains-target at the IR level
void another_function_with_lambda_with_target()
{
    // One lambda should not be emitted.
    []() {
       non_target_func();
    }();
    // One lambda that must be emitted.
    []() {
       #pragma omp target
       { }
    }();
}
//CHECK: declare {{.*}}another_function_with_lambda_with_target{{.*}}
//CHECK: define {{.*}}another_function_with_lambda_with_target{{.*}} #[[CONTAINS_TARGET:[0-9]+]]

//CHECK: define {{.*}}member_with_target{{.*}} #[[CONTAINS_TARGET]]
//CHECK-NOT: define {{.*}}non_target
struct A {
  void member_with_target() {
    #pragma omp target
    {
    }
    non_target_member();
  }
  void non_target_member() {
    other_non_target_func();
  }
};

int main()
{
  A a;
  a.member_with_target();
  func_with_target();
  another_func_with_target();
  another_function_with_lambda_with_target();
  return 0;
}

//CHECK: attributes #[[CONTAINS_TARGET]] = {{.*}}"contains-openmp-target"="true"
//CHECK: attributes #[[DECLARE_TARGET]] = {{.*}}"openmp-target-declare"="true"

// end INTEL_COLLAB
