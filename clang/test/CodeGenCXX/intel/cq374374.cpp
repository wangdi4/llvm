// RUN: %clang_cc1 -triple x86_64-unknown-linux-gnu -std=c++11 -x c++ -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

#define A "a"
int main()
{
  char c[2] = ""A;
  // CHECK: @_ZZ4mainE1c = {{.*}} [2 x i8] c"a\00"
  return 0;
}
