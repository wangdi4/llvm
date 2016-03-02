// RUN: %clang_cc1 -triple x86_64-apple-darwin10 -fintel-compatibility -std=gnu99 --extended_float_types %s -emit-llvm -o - | FileCheck %s


// Define __complex128 type corresponding to __float128 (as in GCC headers).
typedef _Complex float __attribute__((mode(TC))) __complex128;

void check() {
  // CHECK: alloca { fp128, fp128 }
  __complex128 tmp;
}

int check_sizeof_Quad() {
  // CHECK: {{.+}} = call i64 (i64, ...) @llvm.intel.sizeof.i64(i64 32, { fp128, fp128 }* getelementptr ({ fp128, fp128 }, { fp128, fp128 }* null, i32 1))
  return sizeof(__complex128);
}
