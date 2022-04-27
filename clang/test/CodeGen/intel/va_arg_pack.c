// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -triple x86_64-pc-windows -opaque-pointers %s -o - | FileCheck %s --check-prefix=WIN
// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -triple x86_64-pc-linux -opaque-pointers %s -o - | FileCheck %s --check-prefix=LIN
extern int printf(const char* format, ...);

// match the "%d %d\n" constant string.
// WIN: [[FMTStr:@[^ ]+]] = linkonce_odr dso_local unnamed_addr constant [7 x i8] c"%d %d\0
// LIN: [[FMTStr:@[^ ]+]] = private unnamed_addr constant [7 x i8] c"%d %d\0
// match the "myprintf" constant string.
// WIN: [[MyPrintFStr:@[^ ]+]] = linkonce_odr dso_local unnamed_addr constant [10 x i8] c"myprintf:\00"
// LIN: [[MyPrintFStr:@[^ ]+]] = private unnamed_addr constant [10 x i8] c"myprintf:\00"
static __forceinline int myprintf(const char* format, ...) {
  // Shouldn't define the function myprintf.
  // WIN-NOT: define {{.*}} @myprintf
  // WIN-NOT: define {{.*}} @myprintf
  // everything inside main.
  // WIN: define dso_local i32 @main()
  // LIN: define dso_local i32 @main()
  // IDs for the foo/foo2 variable.
  // WIN: [[FOO2:%[0-9]]] = load i32, ptr %foo2
  // WIN: [[FOO:%[0-9]]] = load i32, ptr %foo
  // LIN: [[FOO:%[0-9]]] = load i32, ptr %foo
  // LIN: [[FOO2:%[0-9]]] = load i32, ptr %foo2
  int r = printf("myprintf:");
  // Ensure called with no additional args.
  // WIN: call i32 (ptr, ...) @printf(ptr noundef [[MyPrintFStr]])
  // LIN: call i32 (ptr, ...) @printf(ptr noundef [[MyPrintFStr]])
  if (r < 0) return r;
  int s = printf(format, __builtin_va_arg_pack());
  // printf should be called with the values of foo and foo2.
  // WIN: call i32 (ptr, ...) @printf(ptr noundef %{{[0-9]}}, i32 [[FOO]], i32 [[FOO2]])
  // LIN: call i32 (ptr, ...) @printf(ptr noundef %{{[0-9]}}, i32 [[FOO]], i32 [[FOO2]])
  if (s < 0) return s;
  return r + s + __builtin_va_arg_pack_len();
  // Should add '2' for the len.
  // WIN: = add nsw i32 %{{[^,]+}}, 2
  // LIN: = add nsw i32 %{{[^,]+}}, 2
}

int main() {
  int foo = 10;
  int foo2 = 11;
  return myprintf("%d %d\n", foo, foo2);
}


