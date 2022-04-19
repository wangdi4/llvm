// Support for Intel customization LangOptions::FArgumentNoalias
// RUN: %clang_cc1 -fintel-compatibility -fargument-noalias -triple x86_64-unknown-linux-gnu -emit-llvm -no-opaque-pointers %s -o - | FileCheck %s

// CHECK: define{{.*}}void @{{.+}}(i32* noalias noundef %{{.+}}, i32* noalias noundef %{{.+}}) #0 {
void test1(int *a, int *b) {
  a = b;
}

// CHECK: define{{.*}}double @{{.+}}(double noundef %{{.+}}, double noundef %{{.+}}) #0 {
double test2(double a, double b) {
  return a + b;
}

// CHECK: define{{.*}}i32 @{{.+}}(i32 noundef %{{.+}}, i32* noalias noundef %{{.+}}) #0 {
int test3(int a, int *b) {
  return (a == *b);
}

typedef struct A {
  int n;
  float ff;
} Struct;

// CHECK: define{{.*}}i32 @{{.+}}(%struct.A* noalias noundef %{{.+}}, %struct.A* noalias noundef %{{.+}}) #0 {
int test4(Struct *a, Struct *b) {
  return (a->n == b->n) && (a->ff == b->ff);
}

