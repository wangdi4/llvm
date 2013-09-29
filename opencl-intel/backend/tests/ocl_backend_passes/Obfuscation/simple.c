// RUN: clang -O0 -x cl -emit-llvm -include %S/../../../clang_headers/opencl_.h -S %s -o - | oclopt -module-obfuscation -S | FileCheck %s

kernel void tst(global float *I) {
// CHECK: @tst
// CHECK: %"0"
// CHECK-NOT: %I
  *I = 3.1415;
}
