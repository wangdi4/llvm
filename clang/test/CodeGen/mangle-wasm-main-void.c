<<<<<<< HEAD
// INTEL RUN: %clang_cc1 -opaque-pointers -emit-llvm %s -o - -triple=wasm32-unknown-unknown | FileCheck %s
=======
// RUN: %clang_cc1 -opaque-pointers -emit-llvm %s -o - -triple=wasm32-unknown-unknown | FileCheck %s
>>>>>>> 15c096f39c0f12576716b099a0f82705955958a9

int main(void) {
  return 0;
}
// CHECK: @__main_void = hidden alias i32 (), ptr @main
// CHECK: define i32 @main() #0 {
