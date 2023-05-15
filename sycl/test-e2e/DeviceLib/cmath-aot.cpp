// REQUIRES: opencl-aot, cpu
// UNSUPPORTED: windows

<<<<<<< HEAD
// DEFINE: %{mathflags} = %if cl_options %{/clang:-fno-fast-math%} %else %{-fno-fast-math%}

// RUN: %clangxx -fsycl -fsycl-targets=spir64_x86_64 %{mathflags} %S/cmath_test.cpp -o %t.cmath.out
// RUN: %{run} %t.cmath.out

// RUN: %clangxx -fsycl -fsycl-targets=spir64_x86_64 %{mathflags} %S/cmath_fp64_test.cpp -o %t.cmath.fp64.out
// RUN: %{run} %t.cmath.fp64.out

// RUN: %clangxx -fsycl -fsycl-targets=spir64_x86_64 %{mathflags} %S/std_complex_math_test.cpp -o %t.complex.out
// RUN: %{run} %t.complex.out

// RUN: %clangxx -fsycl -fsycl-targets=spir64_x86_64 %{mathflags} %S/std_complex_math_fp64_test.cpp -o %t.complex.fp64.out
// RUN: %{run} %t.complex.fp64.out
=======
// DEFINE:  = %if cl_options %{/clang:-fno-fast-math%} %else %{-fno-fast-math%}

// RUN: %clangxx -fsycl -fsycl-targets=spir64_x86_64 %{mathflags} %S/cmath_test.cpp -o %t.cmath.out
// RUN: %CPU_RUN_PLACEHOLDER %t.cmath.out

// RUN: %clangxx -fsycl -fsycl-targets=spir64_x86_64 %{mathflags} %S/cmath_fp64_test.cpp -o %t.cmath.fp64.out
// RUN: %CPU_RUN_PLACEHOLDER %t.cmath.fp64.out

// RUN: %clangxx -fsycl -fsycl-targets=spir64_x86_64 %{mathflags} %S/std_complex_math_test.cpp -o %t.complex.out
// RUN: %CPU_RUN_PLACEHOLDER %t.complex.out

// RUN: %clangxx -fsycl -fsycl-targets=spir64_x86_64 %{mathflags} %S/std_complex_math_fp64_test.cpp -o %t.complex.fp64.out
// RUN: %CPU_RUN_PLACEHOLDER %t.complex.fp64.out
>>>>>>> 591d1c8c0e6f2d96cd959da38696b15894af05be
