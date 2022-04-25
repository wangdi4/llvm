// INTEL_COLLAB
// RUN: %clang_cc1 -opaque-pointers -verify -fopenmp -fopenmp-late-outline %s
//
// RUN: %clang_cc1 -opaque-pointers -verify -Wopenmp-allocate -fopenmp -fopenmp-late-outline %s
//
// RUN: %clang_cc1 -opaque-pointers -verify -Wno-openmp-allocate -DNOWARNING -fopenmp \
// RUN: -fopenmp-late-outline %s
//
// RUN: %clang_cc1 -opaque-pointers -triple x86_64-unknown-linux-gnu \
// RUN:  -emit-llvm-bc -disable-llvm-passes \
// RUN:  -fopenmp -fopenmp-targets=spir64 \
// RUN:  -fopenmp-late-outline \
// RUN:  -o %t_spir.bc %s -DSPIR

// Check current default for -fopenmp-target-malloc
// RUN: %clang_cc1 -opaque-pointers -triple spir64 \
// RUN:  -aux-triple x86_64-unknown-linux-gnu \
// RUN:  -fopenmp -fopenmp-targets=spir64 \
// RUN:  -fopenmp-late-outline \
// RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_spir.bc \
// RUN:  %s -DSPIR -DDEVICE_ERROR -verify

// Check -fopenmp-target-malloc
// RUN: %clang_cc1 -opaque-pointers -triple spir64 \
// RUN:  -aux-triple x86_64-unknown-linux-gnu \
// RUN:  -fopenmp -fopenmp-targets=spir64 -fopenmp-late-outline \
// RUN:  -fopenmp-target-malloc \
// RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_spir.bc \
// RUN:  %s -DSPIR -verify

// Check -fno-openmp-target-malloc
// RUN: %clang_cc1 -opaque-pointers -triple spir64 \
// RUN:  -aux-triple x86_64-unknown-linux-gnu \
// RUN:  -fopenmp -fopenmp-targets=spir64 -fopenmp-late-outline \
// RUN:  -fno-openmp-target-malloc \
// RUN:  -fopenmp-is-device -fopenmp-host-ir-file-path %t_spir.bc \
// RUN:  %s -DSPIR -DDEVICE_ERROR -verify

enum omp_allocator_handle_t {
  omp_null_allocator = 0,
  omp_default_mem_alloc = 1,
  omp_large_cap_mem_alloc = 2,
  omp_const_mem_alloc = 3,
  omp_high_bw_mem_alloc = 4,
  omp_low_lat_mem_alloc = 5,
  omp_cgroup_mem_alloc = 6,
  omp_pteam_mem_alloc = 7,
  omp_thread_mem_alloc = 8,
  KMP_ALLOCATOR_MAX_HANDLE = __UINTPTR_MAX__
};

#ifdef SPIR
// Expect an error for device compilation without dynamic allocation
// enabled, either by default or when using -fno-openmp-target-malloc.
#ifndef DEVICE_ERROR
// expected-no-diagnostics
#endif
void foo() {
  #pragma omp parallel
  for(int t = 0; t < 32; ++t) {
    int bar;
#ifdef DEVICE_ERROR
  // expected-error@+2 {{allocate directive not supported for selected target}}
#endif
    #pragma omp allocate(bar) allocator(omp_pteam_mem_alloc)
  }
}
#else // SPIR
#ifdef NOWARNING
// expected-no-diagnostics
#endif

// omp_default_mem_alloc: verify no warning
int al1 = 0;
#pragma omp allocate(al1) allocator(omp_default_mem_alloc)
//
// thread local: no warning
_Thread_local unsigned int v6;
#pragma omp allocate(v6) allocator(omp_thread_mem_alloc)

int al2;
#ifndef NOWARNING
// expected-warning@+2 {{'omp_null_allocator' is equivalent to 'omp_default_mem_alloc’ for variables with static storage duration, consider using a local variable instead}}
#endif
#pragma omp allocate(al2)

extern int al3;
#ifndef NOWARNING
// expected-warning@+2 {{'omp_large_cap_mem_alloc' is equivalent to 'omp_default_mem_alloc’ for variables with static storage duration, consider using a local variable instead}}
#endif
#pragma omp allocate(al3) allocator(omp_large_cap_mem_alloc)

// should get two warnings, one for each variable
static int al4;
int al5;
#ifndef NOWARNING
// expected-warning@+3 {{'omp_high_bw_mem_alloc' is equivalent to 'omp_default_mem_alloc’ for variables with static storage duration, consider using a local variable instead}}
// expected-warning@+2 {{'omp_high_bw_mem_alloc' is equivalent to 'omp_default_mem_alloc’ for variables with static storage duration, consider using a local variable instead}}
#endif
#pragma omp allocate(al4, al5) allocator(omp_high_bw_mem_alloc)

int al6[10];
#ifndef NOWARNING
// expected-warning@+2 {{'omp_low_lat_mem_alloc' is equivalent to 'omp_default_mem_alloc’ for variables with static storage duration, consider using a local variable instead}}
#endif
#pragma omp allocate(al6) allocator(omp_low_lat_mem_alloc)

int main() {
  int al7;

  // No warnings: local
  #pragma omp allocate(al7) allocator(omp_pteam_mem_alloc)
  {
    int al8;
    // No warnings: local
    #pragma omp allocate(al8) allocator(omp_cgroup_mem_alloc)
  }

  static int al9;
#ifndef NOWARNING
  // expected-warning@+2 {{'omp_large_cap_mem_alloc' is equivalent to 'omp_default_mem_alloc’ for variables with static storage duration, consider using a local variable instead}}
#endif
  #pragma omp allocate(al9) allocator(omp_large_cap_mem_alloc)
}
#endif // SPIR
// end INTEL_COLLAB
