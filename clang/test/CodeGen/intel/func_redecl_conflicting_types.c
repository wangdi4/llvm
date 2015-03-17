// CQ#366612
// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -DTEST1 %s -o - | FileCheck -check-prefix CHECK1 %s
// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -DTEST2 %s -o - | FileCheck -check-prefix CHECK2 %s
// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -DTEST3 %s -o - | FileCheck -check-prefix CHECK3 %s
// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -DTEST4 %s -o - | FileCheck -check-prefix CHECK4 %s
// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -DTEST5 %s -o - | FileCheck -check-prefix CHECK5 %s
// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -DTEST6 %s -o - | FileCheck -check-prefix CHECK6 %s
// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -DTEST7 %s -o - | FileCheck -check-prefix CHECK7 %s
// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -DTEST8 %s -o - | FileCheck -check-prefix CHECK8 %s

#ifdef TEST1

// CHECK1: call i32 @foo()
// CHECK1-NOT: declare void @foo()
// CHECK1: declare i32 @foo()

// Declarations with different return types.
void foo(void);
int foo(void);

int main() {
  foo();
  return 0;
}

#elif TEST2

// CHECK2: call void @foo(i32 {{.+}}, i32 {{.+}})
// CHECK2-NOT: declare void @foo(i32)
// CHECK2: declare void @foo(i32, i32)

// Declarations with different number of arguments.
void foo(int a);
void foo(int a, int b);

int main() {
  foo(0, 0);
  return 0;
}

#elif TEST3

// CHECK3: call i32 @foo(i32* {{.+}})
// CHECK3-NOT: declare void @foo(i32*)
// CHECK3: declare i32 @foo(i32*)

// Declarations with incompatible types.
void foo(int *a);
int foo(const int *a);

int main() {
  foo(0);
  return 0;
}

#elif TEST4

// CHECK4: %struct.S1 = type { i32, i8 }
// CHECK4: %struct.S2 = type { i64, i8* }
// CHECK4: call float @foo(%struct.S1* {{.+}}, %struct.S2* {{.+}})
// CHECK4-NOT: declare i32 @foo(i16 {{.+}}, double)
// CHECK4: declare float @foo(%struct.S1*, %struct.S2*)

// Declarations with incompatible types, user-defined types.
struct S1 {
  int i;
  char c;
};
struct S2 {
  long long ll;
  const char *cc;
};
int foo(short a, double b);
float foo(struct S1 *a, struct S2 *b);

int main() {
  struct S1 *s1p = 0;
  struct S2 s2;
  foo(s1p, &s2);
  return 0;
}

#elif TEST5

// CHECK5-NOT: call i32 @foo(i32)
// CHECK5: call void (i8*, ...)* @foo(i8* {{.+}}, {{.+}}, {{.+}}, {{.+}})
// CHECK5-NOT: declare i32 @foo(i32)
// CHECK5: declare void @foo(i8*, ...)

// Declarations with incompatible types, varargs 1.
int foo(int a);
void foo(const char *a, ...);

int main() {
  foo(0, 0, 0, 0);
  return 0;
}

#elif TEST6

// CHECK6-NOT: call i8* (i32*, float, ...)* @foo(i32* null, float 0, {{.+}} 0, {{.+}} 0, {{.+}} 0)
// CHECK6: call i64 (double, i64, i8*, ...)* @foo(double 0.000000e+00, i64 0, i8* null, {{.+}} 0, {{.+}} 0)
// CHECK6-NOT: declare i8* @foo(i32*, float, ...)
// CHECK6: declare i64 @foo(double, i64, i8*, ...)

// Declarations with incompatible types, varargs 2.
const char *foo(int *a, float b, ...);
long long foo(double a, long long b, const char *c, ...);

int main() {
  foo(0, 0, 0, 0, 0);
  return 0;
}

#elif TEST7

// CHECK7-NOT: define i32 @foo()
// CHECK7: define void @foo()
// CHECK7: call void @foo()
// CHECK7-NOT: declare i32 @foo()

// Definition and declaration.
void foo(void) {}
int foo(void);

int main() {
  foo();
  return 0;
}

#elif TEST8

// CHECK8-NOT: define i32 @foo()
// CHECK8: define void @foo()
// CHECK8: call void @foo()
// CHECK8-NOT: declare i32 @foo()

// Declaration and definition.
int foo(void);
void foo(void) {}

int main() {
  foo();
  return 0;
}

#else

#error Unknown test mode

#endif

