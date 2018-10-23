// RUN: %clang_cc1 -fhls -fsyntax-only -ast-dump -verify -pedantic %s | FileCheck %s

void __attribute__((stall_enable)) foo1() {}
// CHECK: FunctionDecl{{.*}}foo1
// CHECK: StallEnableAttr

void foo2() {
  auto lambda = []() __attribute__((stall_enable)){};
  lambda();
  // CHECK: FunctionDecl{{.*}}foo2
  // CHECK: LambdaExpr
  // CHECK: StallEnableAttr
}

//expected-error@+1{{'stall_enable' attribute takes no arguments}}
void __attribute__((stall_enable(1))) bar1() {}

[[clang::stall_enable]] void bar2() {}
