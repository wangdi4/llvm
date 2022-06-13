<<<<<<< HEAD
// RUN: %clang_cc1 -opaque-pointers -triple x86_64-windows -emit-llvm -o - %s | FileCheck %s
=======
// INTEL RUN: %clang_cc1 -opaque-pointers -triple x86_64-windows -emit-llvm -o - %s | FileCheck %s
>>>>>>> ded2b513e5a4b4ccd73c27dff577d6f1c3a5e664

extern "C" {
  const char a __attribute__((used)){};
}

// CHECK: @a = internal constant i8 0
// CHECK: @llvm.used = appending global [1 x ptr] [ptr @a]
