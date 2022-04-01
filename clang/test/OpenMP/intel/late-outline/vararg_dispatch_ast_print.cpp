// INTEL_COLLAB
// RUN: %clang_cc1 -triple=x86_64-pc-win32 -fopenmp -fopenmp-version=51     \
// RUN:   -fopenmp-late-outline -fsyntax-only -verify %s

// RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51 \
// RUN:   -fopenmp-late-outline -fsyntax-only -verify %s

// expected-no-diagnostics

// RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51 \
// RUN:   -fopenmp-late-outline -ast-print %s | FileCheck %s --check-prefix=PRINT

// RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51 \
// RUN:   -fopenmp-late-outline -emit-pch -o %t %s

// RUN: %clang_cc1 -triple x86_64-pc-linux-gnu -fopenmp -fopenmp-version=51 \
// RUN:   -fopenmp-late-outline -include-pch %t -ast-print %s | FileCheck %s --check-prefix=PRINT

#ifndef HEADER
#define HEADER

typedef void *omp_interop_t;

void vararg_foo(const char *fmt, omp_interop_t it, ...);
//PRINT: #pragma omp declare variant(vararg_foo) match(construct={dispatch}) append_args(interop(target))
#pragma omp declare variant(vararg_foo) match(construct={dispatch}) \
                                        append_args(interop(target))
void vararg_bar(const char *fmt, ...) { return; }

void test_four()
{
  //PRINT: #pragma omp dispatch
  //PRINT: vararg_bar("string", 2, 'a', "xyz");
  #pragma omp dispatch
  vararg_bar("string", 2, 'a', "xyz");
}
#endif // HEADER
// end INTEL_COLLAB
