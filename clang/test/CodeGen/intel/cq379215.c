// RUN: %clang_cc1 -triple i386-pc-elfiamcu -fintel-compatibility -emit-llvm %s -o - | FileCheck %s

// Structure that is more than 8 byte.
struct Big {
  double a1, a2, a3, a4, a5, a6, a7, a8, a9, a10;
};

// Empty union with zero size must be returned via memory.
union U1 {
} u1;

// Too large union (80 bytes) must be returned via memory.
union U2 {
  struct Big b;
} u2;

// Aggregate union, must be returned in register.
union U3 {
  int x;
} u3;

// Empty struct with zero size, must be returned via memory.
struct S1 {
} s1;

// Aggregate struct, must be returend in register.
struct S2 {
  int x;
} s2;

// CHECK: [[UNION1_TYPE:%.+]] = type {}
// CHECK: [[UNION2_TYPE:%.+]] = type { [[STRUCT_TYPE:%.+]] }
// CHECK: [[STRUCT_TYPE]] = type { double, double, double, double, double, double, double, double, double, double }
// CHECK: [[UNION3_TYPE:%.+]] = type { i32 }
// CHECK: [[STRUCT1_TYPE:%.+]] = type {}
// CHECK: [[STRUCT2_TYPE:%.+]] = type { i32 }

union U1 f1() { return u1; }
union U2 f2() { return u2; }
union U3 f3() { return u3; }
struct S1 g1() { return s1; }
struct S2 g2() { return s2; }
// CHECK: define void @f1([[UNION1_TYPE]]* noalias sret %{{.+}})
// CHECK: define void @f2([[UNION2_TYPE]]* noalias sret %{{.+}})
// CHECK: define i32 @f3()
// CHECK: define void @g1([[STRUCT1_TYPE]]* noalias sret %{{.+}})
// CHECK: define i32 @g2()

void run() {
  union U1 x1 = f1();
  union U2 x2 = f2();
  union U3 x3 = f3();
  struct S1 y1 = g1();
  struct S2 y2 = g2();

  // CHECK: [[X1:%.+]] = alloca [[UNION1_TYPE]]
  // CHECK: [[X2:%.+]] = alloca [[UNION2_TYPE]]
  // CHECK: [[X3:%.+]] = alloca [[UNION3_TYPE]]
  // CHECK: [[Y1:%.+]] = alloca [[STRUCT1_TYPE]]
  // CHECK: [[Y2:%.+]] = alloca [[STRUCT2_TYPE]]
  // CHECK: call void @f1([[UNION1_TYPE]]* sret [[X1]])
  // CHECK: call void @f2([[UNION2_TYPE]]* sret [[X2]])
  // CHECK: call void @g1([[STRUCT1_TYPE]]* sret [[Y1]])
}
