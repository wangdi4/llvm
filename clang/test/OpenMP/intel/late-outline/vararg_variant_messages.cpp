// INTEL_COLLAB
// RUN: %clang_cc1 -verify -triple x86_64-unknown-linux -fopenmp-late-outline \
// RUN:   -fopenmp -fopenmp-version=51 -std=c++11 -o - %s

typedef void *omp_interop_t;

void novararg_foo(const char *fmt, omp_interop_t it);

// expected-error@+1 {{variant in '#pragma omp declare variant' with type 'void (const char *, omp_interop_t)' (aka 'void (const char *, void *)') is incompatible with type 'void (const char *, ...)' with appended arguments}}
#pragma omp declare variant(novararg_foo) match(construct={dispatch}) \
                                          append_args(interop(target))
void novararg_bar2(const char *fmt, ...) { return; }
//end INTEL_COLLAB
