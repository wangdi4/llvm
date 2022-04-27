//RUN: %clang_cc1 -fhls -emit-llvm -no-opaque-pointers -triple x86_64-unknown-linux-gnu -o - %s | FileCheck %s

void __attribute__((cluster)) foo1() {}
// CHECK: @_Z4foo1v{{.*}}!cluster [[CFOO1:![0-9]+]]

void __attribute__((cluster("clustername"))) foo2() {}
// CHECK: @_Z4foo2v{{.*}}!cluster [[CFOO2:![0-9]+]]

void __attribute__((cluster(""))) foo3() {}
// CHECK: @_Z4foo3v{{.*}}!cluster [[CFOO3:![0-9]+]]

void foo4() {
  auto lambda = []() __attribute__((cluster("lambdaattr"))){};
  lambda();
  // CHECK: @"_ZZ4foo4vENK3$_0clEv"(%class.anon* {{[^,]*}} %this){{.*}}!cluster [[LAMBDA:![0-9]+]]
}

//CHECK: [[CFOO1]] = !{!"", i32 0}
//CHECK: [[CFOO2]] = !{!"clustername", i32 1}
//CHECK: [[CFOO3]] = !{!"", i32 1}
//CHECK: [[LAMBDA]] = !{!"lambdaattr", i32 1}
