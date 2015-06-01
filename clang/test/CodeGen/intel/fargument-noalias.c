// CQ#369692
// RUN: %clang_cc1 -fintel-compatibility -fargument-noalias -emit-llvm %s -o - | FileCheck %s

// CHECK: define void @{{.+}}(i32* noalias %{{.+}}, i32* noalias %{{.+}}) #0 {
void test1(int *a, int *b) {
  a = b;
}

// CHECK: define double @{{.+}}(double %{{.+}}, double %{{.+}}) #0 {
double test2(double a, double b) {
  return a + b;
}

// CHECK: define i32 @{{.+}}(i32 %{{.+}}, i32* noalias %{{.+}}) #0 {
int test3(int a, int *b) {
  return (a == *b);
}

typedef struct A {
  int n;
  float ff;
} Struct;

// CHECK: define i32 @{{.+}}(%struct.A* noalias %{{.+}}, %struct.A* noalias %{{.+}}) #0 {
int test4(Struct *a, Struct *b) {
  return (a->n == b->n) && (a->ff == b->ff);
}

