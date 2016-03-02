// REQUIRES: llvm-backend
// The special IL0 backend version of the test is in the il0-backend subfolder.
// RUN: %clang_cc1 -fintel-compatibility -std=gnu99 --gnu_version=40400 %s -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 -fintel-compatibility -std=gnu89 --extended_float_types %s -emit-llvm -o - | FileCheck %s


// Define __complex128 type corresponding to __float128 (as in GCC headers).
typedef _Complex float __attribute__((mode(TC))) __complex128;

void check() {
  // CHECK: alloca { fp128, fp128 }
  __complex128 tmp;
}

int check_sizeof_Quad() {
  // CHECK: ret i32 32
  return sizeof(__complex128);
}
