// When we compile a PCH cl_khr_fp64 is enabled by default.
// And its support is imported alongside with the PCH.
// Check that we can disable features enabled in the PCH we import.

// No PCH
// RUN: %clang_cc1 -x cl -O0 -cl-std=CL1.2 %s -fsyntax-only -verify -DFP64
// RUN: %clang_cc1 -x cl -O0 -cl-std=CL1.2 %s -fsyntax-only -verify -cl-ext=-cl_khr_fp64

// With PCH
// RUN: %clang_cc1 -x cl -O0 -cl-std=CL1.2 -I %S -emit-pch -o %t.pch %S/intel-extensions-import.h
// RUN: %clang_cc1 -x cl -O0 -cl-std=CL1.2 -include-pch %t.pch %s -fsyntax-only -verify -DIMPORT -DFP64
// RUN: %clang_cc1 -x cl -O0 -cl-std=CL1.2 -include-pch %t.pch %s -fsyntax-only -verify -DIMPORT -cl-ext=-cl_khr_fp64

#ifdef FP64
// expected-no-diagnostics
#else
// expected-error@+4 {{use of type 'double' requires cl_khr_fp64 support}}
// expected-error@+4 {{use of type 'double' requires cl_khr_fp64 support}}
// expected-warning@+4 {{double precision constant requires cl_khr_fp64}}
#endif
void kernel f1(double da) {
  double d;
  (void) 1.0;
#ifdef IMPORT
  d = foo();
#endif
}
