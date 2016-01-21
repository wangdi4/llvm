// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -fintel-compatibility -O0 -emit-llvm %s -o - | FileCheck %s
// RUN: %clang_cc1 -triple=x86_64-windows -fintel-compatibility -O0 -emit-llvm %s -o - | FileCheck %s

// CHECK: [[IT:%.+]] = type
// CHECK: [[ST:%.+]] = type
// CHECK: [[C:@.+]] = private constant [[IT]]
//         @.ref.tmp = private constant %struct.I zeroinitializer, align 4

struct I {
  unsigned n;
  int c[3];
  int s;
};

struct S {
  I v;
  void init(const I&);
};

void foo(S &s) {
// CHECK: define void {{@.*}}([[ST]]* dereferenceable(20) [[S:%.+]])
  s.init({ 0, {}, 0 });  // expected-warning{{generalized initializer lists are a C++11 extension}}
// CHECK: [[SA:%.+]] = alloca [[ST]]*
// CHECK: store [[ST]]* [[S]], [[ST]]** [[SA]]
// CHECK: [[T0:%.+]] = load [[ST]]*, [[ST]]** [[SA]]
// CHECK: call void {{@.*init.*}}([[ST]]* [[T0]], [[IT]]* dereferenceable(20) [[C]]
// call void @_ZN1S4initERK1I(%struct.S* %0, %struct.I* dereferenceable(20) @.ref.tmp)
}
