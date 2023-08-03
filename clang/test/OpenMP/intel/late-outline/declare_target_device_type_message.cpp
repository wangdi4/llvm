// INTEL_COLLAB
// RUN: %clang_cc1 -verify -fopenmp -fopenmp-late-outline -fopenmp-version=50 \
// RUN:  -fopenmp-declare-target-device-type-error-enable %s -verify

// RUN: %clang_cc1 -verify -DNOWARNING -fopenmp-version=50 \
// RUN:   -fopenmp -fopenmp-late-outline %s -verify

#ifdef NOWARNING
// expected-no-diagnostics

#pragma omp declare target device_type(nohost)
static const char *Str = "test";
#pragma omp end declare target
#pragma omp declare target to(Str) device_type(any)

#pragma omp begin declare target device_type(nohost)
void zoo() {}
void x();
#pragma omp end declare target
#pragma omp declare target to(x) device_type(any)
void x() {
}
#else  // NOWARNING
#pragma omp declare target device_type(nohost)
static const char *Str = "test";
#pragma omp end declare target
// expected-error@+1 {{unexpected device_type clause with extended-list}}
#pragma omp declare target to(Str) device_type(any)

// expected-error@+1 {{expected at least one 'to' or 'link' clause}}
#pragma omp declare target device_type(any)
static const char *S = "test";
const char *getS() {
  return S;
}
// expected-error@+1 {{unexpected OpenMP directive '#pragma omp end declare target'}}
#pragma omp end declare target
#pragma omp begin declare target device_type(nohost)
void zoo() {}
void x();
#pragma omp end declare target

// expected-error@+1 {{unexpected device_type clause with extended-list}}
#pragma omp declare target to(x) device_type(any)
void x() {
}

#endif // NOWARNING
// end  INTEL_COLLAB
