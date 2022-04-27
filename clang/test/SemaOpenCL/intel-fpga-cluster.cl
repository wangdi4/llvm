// RUN: %clang_cc1 -x cl -triple spir-unknown-unknown-intelfpga -fsyntax-only -ast-dump -verify -pedantic %s | FileCheck %s
// RUN: %clang_cc1 -x cl -triple x86_64-unknown-unknown-intelfpga -fsyntax-only -ast-dump -verify -pedantic %s | FileCheck %s

void __attribute__((cluster)) foo1(void) {}
// CHECK: FunctionDecl{{.*}}foo1
// CHECK: ClusterAttr
// CHECK-NOT: HasName

void __attribute__((cluster("clustername"))) foo2(void) {}
// CHECK: FunctionDecl{{.*}}foo2
// CHECK: ClusterAttr
// CHECK-SAME: "clustername" HasName

void __attribute__((cluster(""))) foo3(void) {}
// CHECK: FunctionDecl{{.*}}foo3
// CHECK: ClusterAttr
// CHECK-SAME: "" HasName

//expected-error@+1{{'cluster' attribute takes no more than 1 argument}}
void __attribute__((cluster("clustername", 1))) bar1(void) {}

//expected-error@+1{{'cluster' attribute requires a string}}
void __attribute__((cluster(123))) bar2(void) {}

constant int var = 1;

//expected-error@+1{{'cluster' attribute requires a string}}
void __attribute__((cluster(var))) bar3(void) {}
