// INTEL_COLLAB
//RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -emit-llvm-bc \
//RUN:  -disable-llvm-passes -fopenmp -fopenmp-targets=spir64    \
//RUN:  -fopenmp-late-outline -fintel-compatibility   \
//RUN:  -Wsource-uses-openmp -o %t_host.bc %s

//RUN: %clang_cc1 -triple spir64 -emit-llvm -disable-llvm-passes   \
//RUN:  -fopenmp -fopenmp-targets=spir64 -fopenmp-late-outline     \
//RUN:  -fintel-compatibility -fopenmp-is-device -verify  -o - %s   \
//RUN:  -fopenmp-host-ir-file-path %t_host.bc -Wno-device-printf-literals

extern "C" int printf(const char*,...);
struct A { int o; } s;
void foo()
{
  #pragma omp target
  // expected-warning@+1 {{format specifies type 'int' but the argument has type 'struct A'}}
  printf("%d\n", s); // expected-error {{cannot compile this non-scalar arg to printf yet}}
  const unsigned char C[] = "Hello";
  #pragma omp target
  // no warning
  printf("%s\n", C);
}
