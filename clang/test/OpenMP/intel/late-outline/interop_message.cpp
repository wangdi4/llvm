// INTEL_COLLAB
// RUN: %clang_cc1 -fopenmp -fopenmp-late-outline -fopenmp-version=51 %s -verify

typedef void *omp_interop_t;
void foo(omp_interop_t obj0, omp_interop_t obj1, omp_interop_t obj2);

void foo1() {
  omp_interop_t obj0 = 0;
  omp_interop_t obj1 = 0;
  omp_interop_t obj2 = 0;

  #pragma omp interop init(targetsync:obj0)
  #pragma omp interop init(targetsync:obj1)
  #pragma omp interop init(targetsync:obj2)

  #pragma omp interop init(prefer_type("level_zero"),targetsync: obj0)
  #pragma omp interop init(prefer_type("sycl"),targetsync: obj1)
  // expected-warning@+1 {{Expected 'cuda', 'cuda_driver', 'opencl', 'sycl', 'hip', 'level_zero' or 'level_one' expression}}
  #pragma omp interop init(prefer_type("abcdefghi"),targetsync: obj2)

   foo(obj0, obj1, obj2);
}
// end INTEL_COLLAB
