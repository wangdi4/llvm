// RUN: %clang -std=c++11 -fno-exceptions -Xclang -fintel-compatibility -Xclang -fcilkplus -emit-llvm -O0 -c -S %s -o %t
// RUN: FileCheck --input-file=%t %s
// REQUIRES: cilkplus

void f1(int &v);

void test1() {
  int v = 1;
  _Cilk_spawn f1(v);
  //CHECK: define void @{{.*}}test1
  //CHECK: call void @__cilk_spawn_helper
}
