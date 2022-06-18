<<<<<<< HEAD
// INTEL RUN: %clang_cc1 -opaque-pointers -emit-llvm %s -o - -triple=wasm32-unknown-unknown | FileCheck %s
=======
// RUN: %clang_cc1 -opaque-pointers -emit-llvm %s -o - -triple=wasm32-unknown-unknown | FileCheck %s
>>>>>>> 15c096f39c0f12576716b099a0f82705955958a9

int main(int argc, char* argv[]) {
  return 0;
}
// CHECK-NOT: __main_void
// CHECK: define i32 @__main_argc_argv(i32 noundef %argc, ptr noundef %argv) #0 { 
