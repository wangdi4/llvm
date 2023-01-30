<<<<<<< HEAD
// RUN: %clang_cc1 -verify -fopenmp -fopenmp-version=52 -x c -triple x86_64-apple-darwin10 %s
=======
// RUN: %clang_cc1 -fopenmp -fopenmp-version=52 -x c -triple x86_64-apple-darwin10 %s -o - 2>&1 | FileCheck %s --check-prefix=CHECK
>>>>>>> 2ebda47619d7e8b72d460a955de4f398f637489f

extern int omp_get_initial_device();
extern void *omp_get_mapped_ptr(void *, int);

void t() {
<<<<<<< HEAD
  omp_get_mapped_ptr(&x, omp_get_initial_device()); //expected-error {{use of undeclared identifier 'x'}}
}

=======
  omp_get_mapped_ptr(&x, omp_get_initial_device());
}

// CHECK: error: use of undeclared identifier 'x'
// CHECK-NOT: crash
>>>>>>> 2ebda47619d7e8b72d460a955de4f398f637489f
