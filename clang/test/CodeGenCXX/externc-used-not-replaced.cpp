<<<<<<< HEAD
// INTEL RUN: %clang_cc1 -opaque-pointers -triple x86_64-windows -emit-llvm -o - %s | FileCheck %s
=======
// RUN: %clang_cc1 -opaque-pointers -triple x86_64-windows -emit-llvm -o - %s | FileCheck %s
>>>>>>> cac38efc0d31d42f74cee83478d0a35bdec895c6

extern "C" {
  const char a __attribute__((used)){};
}

// CHECK: @a = internal constant i8 0
// CHECK: @llvm.used = appending global [1 x ptr] [ptr @a]
