// RUN: %clang_cc1 -triple -x86_64-linux-gnu %s -emit-llvm -o - | FileCheck %s -check-prefix=LINUX
// RUN: %clang_cc1 -triple -x86_64-windows-msvc -fintel-compatibility %s -emit-llvm -o - | FileCheck %s -check-prefix=WINDOWS

void foo(__float128){}
// LINUX: define void @_Z3foog(fp128 %0)

// Demangles to: void __cdecl foo(struct __clang::__float128)
// WINDOWS: define dso_local void @"?foo@@YAXU__float128@__clang@@@Z"(fp128* %0)
