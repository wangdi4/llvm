// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -no-opaque-pointers -triple x86_64-pc-windows %s -o - | FileCheck %s --check-prefix=WIN
// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -no-opaque-pointers -triple x86_64-pc-linux %s -o - | FileCheck %s --check-prefix=LIN
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
  // WIN: [[FOO2:%[0-9]]] = load i32, i32* %foo2
  // WIN: [[FOO:%[0-9]]] = load i32, i32* %foo
  // LIN: [[FOO:%[0-9]]] = load i32, i32* %foo
  // LIN: [[FOO2:%[0-9]]] = load i32, i32* %foo2
  int r = printf("myprintf:");
  // Ensure called with no additional args.
  // WIN: call i32 (i8*, ...) @printf(i8* noundef getelementptr inbounds ([10 x i8], [10 x i8]* [[MyPrintFStr]], i64 0, i64 0))
  // LIN: call i32 (i8*, ...) @printf(i8* noundef getelementptr inbounds ([10 x i8], [10 x i8]* [[MyPrintFStr]], i64 0, i64 0))
  if (r < 0) return r;
  int s = printf(format, __builtin_va_arg_pack());
  // printf should be called with the values of foo and foo2.
  // WIN: call i32 (i8*, ...) @printf(i8* noundef %{{[0-9]}}, i32 [[FOO]], i32 [[FOO2]])
  // LIN: call i32 (i8*, ...) @printf(i8* noundef %{{[0-9]}}, i32 [[FOO]], i32 [[FOO2]])
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


