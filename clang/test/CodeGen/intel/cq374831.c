// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu %s -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-windows-msvc %s -emit-llvm -o - | FileCheck %s
// REQUIRES: non-ms-sdk
// REQUIRES: llvm-backend

char *s = "Intel(R) Clang Based C++";
char *VERSION = __VERSION__;
// CHECK: c"[[S:.+]]\00"
// CHECK: c"[[S]]{{.+}}\00"
