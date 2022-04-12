// RUN: %clang_cc1 -fhls -fsyntax-only -ast-dump -verify -pedantic %s | FileCheck %s

// expected-warning@+2 {{attribute 'stall_enable' is deprecated}}
// expected-note@+1 {{did you mean to use 'use_stall_enable_clusters' instead?}}
void __attribute__((stall_enable)) foo1(void) {}
// CHECK: FunctionDecl{{.*}}foo1
// CHECK: SYCLIntelUseStallEnableClustersAttr

void __attribute__((use_stall_enable_clusters)) foo2(void) {}
// CHECK: FunctionDecl{{.*}}foo2
// CHECK: SYCLIntelUseStallEnableClustersAttr

//expected-error@+1{{'use_stall_enable_clusters' attribute takes no arguments}}
void __attribute__((use_stall_enable_clusters(1))) bar1(void) {}
