// RUN: %clang_cc1 -fhls -fsyntax-only -ast-dump -verify -pedantic %s | FileCheck %s

void __attribute__((cluster)) foo1() {}
// CHECK: FunctionDecl{{.*}}foo1
// CHECK: ClusterAttr
// CHECK-NOT: HasName

void __attribute__((cluster("clustername"))) foo2() {}
// CHECK: FunctionDecl{{.*}}foo2
// CHECK: ClusterAttr
// CHECK-SAME: "clustername" HasName

void __attribute__((cluster(""))) foo3() {}
// CHECK: FunctionDecl{{.*}}foo3
// CHECK: ClusterAttr
// CHECK-SAME: "" HasName

void foo4() {
  auto lambda = []() __attribute__((cluster("lambdaattr"))){};
  lambda();
  // CHECK: FunctionDecl{{.*}}foo4
  // CHECK: LambdaExpr
  // CHECK: ClusterAttr
  // CHECK-SAME: "lambdaattr" HasName
}

//expected-error@+1{{'cluster' attribute takes no more than 1 argument}}
void __attribute__((cluster("clustername", 1))) bar1() {}

//expected-error@+1{{expected string literal as argument of 'cluster' attribute}}
void __attribute__((cluster(123))) bar2() {}

//expected-error@+1{{expected string literal as argument of 'cluster' attribute}}
void __attribute__((cluster(foo1))) bar3() {}

[[clang::cluster("cluster_name")]] void bar4() {}
