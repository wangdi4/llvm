// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-linux-gnu %s -emit-llvm -o - | FileCheck %s
// RUN: %clang_cc1 -fintel-compatibility -triple x86_64-unknown-windows-msvc %s -emit-llvm -o - | FileCheck %s
// UNSUPPORTED: ms-sdk

char *s = "Intel(R) oneAPI DPC++/C++ Compiler";
char *VERSION = __VERSION__;
// CHECK: c"[[S:.+]]\00"
// CHECK: c"[[S]] {{[1-9][0-9][0-9][0-9][\.][0-9]+[\.][0-9]+}} ({{.+}})\00"
