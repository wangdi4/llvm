// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -fintel-compatibility -emit-llvm %s -o - | FileCheck %s
// expected-no-diagnostics

class A {
public:
 A();
 A(const A&);
 A& operator=(const A&);
 A& operator() (A a, A b) {return *this;}
};

// CHECK-LABEL foo
int foo(A,int i) 
{
   A x[100], obj, m; 
   obj = __sec_reduce(A(), x[:], m); 
   return 1;
}
// CHECK: %obj = alloca
// CHECK: %cean.acc. = alloca
// CHECK: call {{.+}}%cean.acc.
// CHECK: call {{.+}}%obj{{.+}}%cean.acc.
// CHECK: ret
