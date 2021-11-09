// RUN: %clang_cc1 -triple x86_64-windows-pc -fintel-compatibility %s -emit-llvm -fintel-long-double-size=64 -o - | FileCheck %s --check-prefix=SIZE64
// RUN: %clang_cc1 -triple i386-windows-pc -fintel-compatibility %s -emit-llvm -fintel-long-double-size=80 -o - | FileCheck %s --check-prefix=SIZE80_x86
// RUN: %clang_cc1 -triple x86_64-windows-pc -fintel-compatibility %s -emit-llvm -fintel-long-double-size=80 -o - | FileCheck %s --check-prefix=SIZE80
// RUN: %clang_cc1 -triple x86_64-windows-pc -fintel-compatibility %s -emit-llvm -fintel-long-double-size=128 -o - | FileCheck %s --check-prefix=SIZE128
// RUN: %clang_cc1 -triple x86_64-windows-pc -fintel-compatibility %s -emit-llvm -o - | FileCheck %s --check-prefix=SIZE64

long double global;
// SIZE64: @global = dso_local global double 0
// SIZE80: @global = dso_local global x86_fp80 0xK{{0+}}, align 16
// SIZE80_x86: @global = dso_local global x86_fp80 0xK{{0+}}, align 16
// SIZE128: @global = dso_local global fp128 0

long double func(long double param) {
  // SIZE64: define dso_local double @func(double %param)
  // SIZE80: define dso_local x86_fp80 @func(x86_fp80 %param)
  // SIZE80_x86: define dso_local x86_fp80 @func(x86_fp80 %param)
  // SIZE128: define dso_local fp128  @func(fp128 %param)
  long double local = param;
  // SIZE64: alloca double
  // SIZE80: alloca x86_fp80, align 16
  // SIZE80_x86: alloca x86_fp80, align 16
  // SIZE128: alloca fp128
  local = param;
  return local + param;
}
