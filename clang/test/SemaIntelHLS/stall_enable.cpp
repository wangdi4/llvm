// RUN: %clang_cc1 -fhls -fsyntax-only -ast-dump -verify -pedantic %s | FileCheck %s

// expected-warning@+2 {{attribute 'stall_enable' is deprecated}}
// expected-note@+1 {{did you mean to use 'use_stall_enable_clusters' instead?}}
void __attribute__((stall_enable)) foo1() {}
// CHECK: FunctionDecl{{.*}}foo1
// CHECK: SYCLIntelUseStallEnableClustersAttr

void __attribute__((use_stall_enable_clusters)) foo2() {}
// CHECK: FunctionDecl{{.*}}foo2
// CHECK: SYCLIntelUseStallEnableClustersAttr

void foo3() {
  auto lambda = []() __attribute__((use_stall_enable_clusters)){};
  lambda();
  // CHECK: FunctionDecl{{.*}}foo3
  // CHECK: LambdaExpr
  // CHECK: SYCLIntelUseStallEnableClustersAttr
}

// expected-warning@+3 {{attribute 'stall_enable' is deprecated}}
// expected-note@+2 {{did you mean to use 'use_stall_enable_clusters' instead?}}
void foo4() {
  auto lambda = []() __attribute__((stall_enable)){};
  lambda();
  // CHECK: FunctionDecl{{.*}}foo4
  // CHECK: LambdaExpr
  // CHECK: SYCLIntelUseStallEnableClustersAttr
}

//expected-error@+1{{'use_stall_enable_clusters' attribute takes no arguments}}
void __attribute__((use_stall_enable_clusters(1))) bar1() {}

[[clang::use_stall_enable_clusters]] void bar2() {}

// expected-warning@+2 {{attribute 'clang::stall_enable' is deprecated}}
// expected-note@+1 {{did you mean to use 'clang::use_stall_enable_clusters' instead?}}
[[clang::stall_enable]] void bar3() {}
