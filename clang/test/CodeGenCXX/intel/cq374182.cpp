// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu %s -emit-llvm -o - | FileCheck %s --check-prefix=LIN
// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-windows-msvc -emit-llvm -o - %s | FileCheck %s --check-prefix=WIN

typedef int t;
t __restrict__ aaaaa;

// LIN: @{{.*aaaaa.*}} = global 
// WIN: @{{.*aaaaa.*}} = dso_local global 



