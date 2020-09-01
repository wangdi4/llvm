// INTEL_COLLAB
// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux-gnu -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -emit-llvm-bc -o %t-host.bc %s
//
// RUN: %clang_cc1 -verify -triple spir64 -fopenmp \
// RUN:  -fintel-compatibility -fopenmp-late-outline \
// RUN:  -fopenmp-targets=spir64 -fopenmp-is-device \
// RUN:  -fopenmp-host-ir-file-path %t-host.bc -emit-llvm -o - %s \
// RUN:  | FileCheck %s
//
// expected-no-diagnostics

// Test handling of aliases in OpenMP device code.

//CHECK-DAG: @bar_A {{.*}}alias {{.*}}@bar
//CHECK-DAG: @baz_A {{.*}}alias {{.*}}@baz
//CHECK-DAG: @faz_A {{.*}}alias {{.*}}@faz
//CHECK-DAG: @bazz_A {{.*}}alias {{.*}}@bazz

//CHECK-DAG: define {{.*}}bar
//CHECK-DAG: define {{.*}}baz
//CHECK-DAG: define {{.*}}faz
//CHECK-DAG: define {{.*}}bazz

int printit(const char* c) { return (int)c[0]; }

int foo(void)
{
  return printit("this is foo\n");
}

// If 'foo' is not compiled on device, aliases to 'foo' should not be an error.
int foo_A(void) __attribute__ ((alias("foo")));

int bar(void)
{
  return printit("this is bar\n");
}

// If alias is declare target we can compile it. But if declared after 'bar'
// and only calls are through the alias it won't resolve.  If there is a
// direct call then it will be okay.
#pragma omp declare target
int bar_A(void) __attribute__ ((alias("bar")));
#pragma omp end declare target

// Both function and alias declare target should be fine.
#pragma omp declare target
int baz(void)
{
  return printit("this is baz\n");
}
int baz_A(void) __attribute__ ((alias("baz")));
#pragma omp end declare target

// In either order.
int bazz_A(void) __attribute__ ((alias("bazz")));
#pragma omp declare target
int bazz(void)
{
  return printit("this is bazz\n");
}
#pragma omp end declare target

// If the alias is not declare target, but the target is already defined
// that should be okay.
#pragma omp declare target
int faz(void)
{
  return printit("this is faz\n");
}
#pragma omp end declare target
int faz_A(void) __attribute__ ((alias("faz")));

int main()
{
  foo_A();
  #pragma omp target
  baz_A();
  #pragma omp target
  faz_A();
  #pragma omp target
  bar();
  #pragma omp target
  bar_A();
  #pragma omp target
  bazz_A();

  return 0;
}
// end INTEL_COLLAB
