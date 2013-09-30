// RUN: clang -O0 -x cl -emit-llvm -include %S/../../../clang_headers/opencl_.h -S %s -o - | opt -module-obfuscation -S | FileCheck %s

kernel void tst(global float *I) {
// CHECK: @tst
// CHECK: %"0"
// CHECK-NOT: %I
// CHECK: ; <label>:{{[0-9]+}}
// CHECK: ; <label>:{{[0-9]+}}
  if (*I == 1.0f)
    *I = 3.1415;
}
