// Test that the is*RemarkEnabled overrides are working properly.  This remark
// requiring extra analysis is only conditionally enabled.

// INTEL add '-mllvm -loopopt=0' to disable additional GVN passes
// INTEL RUN: %clang_cc1 %s -Rpass-missed=gvn -O2 -emit-llvm-only -verify -mllvm -loopopt=0

int foo(int *x, int *y) {
  int a = *x;
  *y = 2;
  // expected-remark@+1 {{load of type i32 not eliminated}}
  return a + *x;
}
