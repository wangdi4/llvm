// RUN: %clang_cc1 -triple x86_64-pc-windows-msvc -fintel-compatibility -fms-compatibility -fintel-ms-compatibility -O0 -emit-llvm %s -o - | FileCheck %s

__declspec(dllimport) int __cdecl f1();
static __inline int f1() { return 1; }

int f2();
static __inline int f2() { return 2; }

__declspec(dllimport) int f3();
static int f3() { return 3; }

int f4();
static int f4();
int f4() { return 4; }

__declspec(dllimport) int f5();
__inline int f5();
static int f5() { return 5; }

__declspec(dllimport) __inline int f6();
static int f6() { return 6; }

__declspec(dllexport) __inline int f7();
static int f7() { return 7; }

__declspec(dllexport) int f8();
static int f8() { return 8; }

__declspec(dllexport) int f9();
static int f9();
__inline int f9() { return 9; }

int g() {
  return f1() + f2() + f3() + f4() + f5() + f6() + f7() + f8() + f9();
}

// CHECK-DAG: declare dllimport i32 @f1
// CHECK-DAG: define internal i32 @f2
// CHECK-DAG: define {{.*}}dllexport i32 @f3
// CHECK-DAG: define internal i32 @f4
// CHECK-DAG: declare dllimport i32 @f5
// CHECK-DAG: declare dllimport i32 @f6
// CHECK-DAG: define {{.*}}dllexport i32 @f7
// CHECK-DAG: define {{.*}}dllexport i32 @f8
// CHECK-DAG: define {{.*}}dllexport i32 @f9
