// RUN: %clang_cc1 -fintel-compatibility -fopenmp %s -emit-llvm -o - | FileCheck %s

// CHECK-LABEL: @main
int main(int argc, char **argv) {
  int res = 0, i;
// CHECK: call void {{[^@]+}}@__kmpc_fork_call(
// CHECK: call void @__kmpc_for_static_init_4(
#pragma omp parallel for
#pragma simd vectorlength(8)
  for (i = 0; i < argc; i++)
    res += i;
// CHECK: call void @__kmpc_for_static_fini(
  return res;
}

