// RUN: %clang_cc1 -triple -x86_64-linux-gnu %s -emit-llvm -opaque-pointers -o - | FileCheck %s -check-prefix=LINUX
// RUN: %clang_cc1 -triple -x86_64-windows-msvc -fintel-compatibility %s -emit-llvm -opaque-pointers -o - | FileCheck %s -check-prefix=WINDOWS

void foo(__float128){}
// LINUX: define dso_local void @_Z3foog(fp128 noundef %0)
// Demangles to: void __cdecl foo(struct __clang::__float128)
// WINDOWS: define dso_local void @"?foo@@YAXU__float128@__clang@@@Z"(ptr noundef %0)
