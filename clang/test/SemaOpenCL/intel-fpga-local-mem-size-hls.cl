// RUN: %clang_cc1 -triple spir-unknown-unknown-intelfpga -fsyntax-only -ast-dump -pedantic %s | FileCheck %s
// RUN: %clang_cc1 -triple x86_64-unknown-unknown-intelfpga -fsyntax-only -ast-dump -pedantic %s | FileCheck %s

__kernel void foo(__local __attribute__((local_mem_size(32))) int *ip) {}
// CHECK: FunctionDecl{{.*}}foo
// CHECK-NEXT: ParmVarDecl{{.*}}ip
// CHECK-NEXT: OpenCLLocalMemSizeAttr

