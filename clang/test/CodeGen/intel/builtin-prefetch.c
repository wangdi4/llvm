// RUN: %clang-cc1 -triple x86_64-unknown-unknown -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

int main(void) {
  int *p;
  // Default behavior with 1 argument.
  // CHECK: @llvm.prefetch(i8* {{.+}}, i32 0, i32 3, i32 1)
  __builtin_prefetch(p);
  // Default behavior with 3 arguments, which are in their allowed range.
  // CHECK: @llvm.prefetch(i8* {{.+}}, i32 1, i32 2, i32 1)
  __builtin_prefetch(p, 1, 2);
  // Assume 0 if the second argument is out of 0..1 range.
  // CHECK: @llvm.prefetch(i8* {{.+}}, i32 0, i32 1, i32 1)
  __builtin_prefetch(p, -1, 1);
  // Assume 0 if the third argument is out of 0..3 range.
  // CHECK: @llvm.prefetch(i8* {{.+}}, i32 1, i32 0, i32 1)
  __builtin_prefetch(p, 1, 4);
  // When both arguments are out of their range, assume 0 and 0.
  // CHECK: @llvm.prefetch(i8* {{.+}}, i32 0, i32 0, i32 1)
  __builtin_prefetch(p, 2, -1);
}
