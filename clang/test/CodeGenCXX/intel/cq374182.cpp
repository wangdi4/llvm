// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu %s -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-windows-msvc -emit-llvm -o - %s | FileCheck %s

typedef int t;
t __restrict__ aaaaa;

// CHECK: @{{.*aaaaa.*}} = global 



