// INTEL_COLLAB
//
// RUN: %clang_cc1 -fopenmp -fopenmp-late-outline \
// RUN: -fopenmp-version=52 %s -verify=expected
//

typedef enum omp_allocator_handle_t {
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
} omp_allocator_handle_t;

int main () {
  int foo;
  int bar;
  int BadAlloc;
  omp_allocator_handle_t MyAlloc = omp_large_cap_mem_alloc;

  // expected-error@+1 {{expected ':'}}
  #pragma omp scope allocate(align(8)foo) private(foo)

  // expected-error@+1 {{expected ':'}}
  #pragma omp scope allocate(allocator(MyAlloc), align(2)foo,bar) private(foo,bar)

  // expected-error@+1 {{expected ':'}}
  #pragma omp scope allocate(align(4), allocator(MyAlloc)foo,bar) private(foo,bar)

  // expected-error@+1 {{'allocate' clause cannot contain more than one 'align' modifier}}
  #pragma omp scope allocate(allocator(MyAlloc), align(2), align(4):foo,bar) private(foo,bar)

  // expected-error@+1 {{'allocate' clause cannot contain more than one 'allocator' modifier}}
  #pragma omp scope allocate(allocator(MyAlloc), allocator(omp_pteam_mem_alloc), align(2):foo,bar) private(foo,bar)

  // expected-error@+2 {{'allocate' clause cannot contain more than one 'allocator' modifier}}
  // expected-error@+1 {{'allocate' clause cannot contain more than one 'align' modifier}}
  #pragma omp scope allocate(allocator(MyAlloc), allocator(omp_pteam_mem_alloc), align(2), align(4):foo,bar) private(foo,bar)

  // expected-error@+2 {{expected ')'}}
  // expected-note@+1 {{to match this '('}}
  #pragma omp scope allocate(allocator(MyAlloc, align(7:foo,bar) private(foo,bar)

  // expected-error@+2 {{expected ')'}}
  // expected-note@+1 {{to match this '('}}
  #pragma omp scope allocate(allocator(MyAlloc), align(7:foo,bar) private(foo,bar)

  // expected-error@+1 {{expected ':'}}
  #pragma omp scope allocate(allocator(MyAlloc), align(2)foo,bar) private(foo,bar)

  // expected-error@+1 {{expected ':'}}
  #pragma omp scope allocate(align(4), allocator(MyAlloc)foo,bar) private(foo,bar)

  // expected-error@+1 {{'allocate' clause cannot contain more than one 'align' modifier}}
  #pragma omp scope allocate(allocator(MyAlloc), align(2), align(4):foo,bar) private(foo,bar)

  // expected-error@+1 {{'allocate' clause cannot contain more than one 'allocator' modifier}}
  #pragma omp scope allocate(allocator(MyAlloc), allocator(omp_pteam_mem_alloc), align(2):foo,bar) private(foo,bar)

  // expected-error@+2 {{'allocate' clause cannot contain more than one 'allocator' modifier}}
  // expected-error@+1 {{'allocate' clause cannot contain more than one 'align' modifier}}
  #pragma omp scope allocate(allocator(MyAlloc), allocator(omp_pteam_mem_alloc), align(2), align(4):foo,bar) private(foo,bar)

  // expected-error@+2 {{expected ')'}}
  // expected-note@+1 {{to match this '('}}
  #pragma omp scope allocate(allocator(MyAlloc, align(7:foo,bar) private(foo,bar)

  // expected-error@+2 {{expected ')'}}
  // expected-note@+1 {{to match this '('}}
  #pragma omp scope allocate(allocator(MyAlloc), align(7:foo,bar) private(foo,bar)

  // expected-error@+1 {{expected '(' after 'allocator'}}
  #pragma omp scope allocate(allocator MyAlloc), align(7:foo,bar) private(foo,bar)

  // expected-error@+1 {{expected '(' after 'align'}}
  #pragma omp scope allocate(allocator(MyAlloc), align 7:foo,bar) private(foo,bar)

  // expected-error@+1 {{expected expression}}
  #pragma omp scope allocate(allocator(), align(7):foo,bar) private(foo,bar)

  // expected-error@+2 {{expected ')'}}
  // expected-note@+1 {{to match this '('}}
  #pragma omp scope allocate(allocator(MyAlloc, align(7):foo,bar) private(foo,bar)

  // expected-error@+1 {{'allocate' clause cannot contain more than one 'align' modifier}}
  #pragma omp scope allocate(allocator(MyAlloc), align(2), align(4):foo,bar) private(foo,bar)

  // expected-error@+3 {{use of undeclared identifier 'allocatorMyAlloc'}}
  // expected-error@+2 {{expected expression}}
  // expected-warning@+1 {{extra tokens at the end of '#pragma omp scope' are ignored}}
  #pragma omp scope allocate(allocatorMyAlloc):foo,bar) private(foo,bar)

  // expected-error@+2 {{expected ')'}}
  // expected-note@+1 {{to match this '('}}
  #pragma omp scope allocate(allocator(MyAlloc:foo,bar) private(foo,bar)

  // expected-error@+1 {{initializing 'const omp_allocator_handle_t' with an expression of incompatible type 'int'}}
  #pragma omp scope allocate(allocator(foo+1):foo) private(foo)

  // expected-error@+1 {{expected ':'}}
  #pragma omp scope allocate(allocator(MyAlloc)foo,bar) private(foo,bar)

  // expected-error@+1 {{initializing 'const omp_allocator_handle_t' with an expression of incompatible type 'int'}}
  #pragma omp scope allocate(allocator(BadAlloc):foo) private(foo)

  // expected-error@+1 {{expected ':'}}
  #pragma omp scope allocate(allocator(MyAlloc), align(4))

  // expected-error@+1 {{expected expression}}
  #pragma omp scope allocate(allocator(MyAlloc), align())

  // expected-error@+1 {{requested alignment is not a power of 2}}
  #pragma omp scope allocate(align(0): foo) private(foo)

  // expected-error@+1 {{requested alignment is not a power of 2}}
  #pragma omp scope allocate(allocator(MyAlloc), align(7):foo) private(foo)

  // expected-error@+2 {{expected ',' or ')' in 'allocate' clause}}
  // expected-error@+1 {{expected expression}}
  #pragma omp scope allocate(allocator(MyAlloc):foo;bar) private(foo,bar)

  // expected-error@+1 {{expected expression}}
  #pragma omp scope allocate(allocator(MyAlloc):,foo,bar) private(foo,bar)
  {}

  return 0;
}
// end INTEL_COLLAB



