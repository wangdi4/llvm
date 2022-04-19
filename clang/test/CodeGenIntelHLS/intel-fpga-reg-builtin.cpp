//RUN: %clang_cc1 -fhls -triple x86_64-unknown-linux-gnu -emit-llvm -opaque-pointers -o - %s | FileCheck %s

struct st {
  int a;
  float b;
};
// CHECK: %struct.st = type { i32, float }

struct stnonpod {
  int x;
  stnonpod(int a) {
    x = a;
  }
};

class cl {
public:
  int x;
  cl(int a) {
    x = a;
  }
};

union un {
  int a;
  char c[4];
};
// CHECK: %union.un = type { i32 }

typedef int myInt;

void foo() {
// CHECK: define{{.*}}void @_Z3foov()
  int a=123;
  myInt myA = 321;
// CHECK: store i32 123, ptr %a, align 4
// CHECK: store i32 321, ptr %myA, align 4
// CHECK: %[[LOADA1:[0-9]+]] = load i32, ptr %a, align 4
  int b = __builtin_fpga_reg(a);
// CHECK: %[[B:[0-9]+]] = call i32 @llvm.fpga.reg.i32(i32 %[[LOADA1]])
// CHECK: store i32 %[[B]], ptr %b, align 4
  int myB = __builtin_fpga_reg(myA);
// CHECK: %[[LOADMYA:[0-9]+]] = load i32, ptr %myA, align 4
// CHECK: %[[myB:[0-9]+]] = call i32 @llvm.fpga.reg.i32(i32 %[[LOADMYA]])
// CHECK: store i32 %[[myB]], ptr %myB, align 4
  int c = __builtin_fpga_reg(2.0f);
// CHECK: %{{[0-9]+}} = call float @llvm.fpga.reg.f32(float 2.000000e+00)
  int d = __builtin_fpga_reg( __builtin_fpga_reg( b+12 ));
// CHECK: %[[LOADB:[0-9]+]] = load i32, ptr %b, align 4
// CHECK: %add = add nsw i32 %[[LOADB]], 12
// CHECK: %[[D1:[0-9]+]] = call i32 @llvm.fpga.reg.i32(i32 %add)
// CHECK: %{{[0-9]+}} = call i32 @llvm.fpga.reg.i32(i32 %[[D1]])
  int e = __builtin_fpga_reg( __builtin_fpga_reg( a+b ));
// CHECK: %[[LOADA2:[0-9]+]] = load i32, ptr %a, align 4
// CHECK: %[[LOADB2:[0-9]+]] = load i32, ptr %b, align 4
// CHECK: %add1 = add nsw i32 %[[LOADA2]], %[[LOADB2]]
// CHECK: %[[E1:[0-9]+]] = call i32 @llvm.fpga.reg.i32(i32 %add1)
// CHECK: %{{[0-9]+}} = call i32 @llvm.fpga.reg.i32(i32 %[[E1]])
  int f;
  f = __builtin_fpga_reg(a);
// CHECK: %[[LOADA3:[0-9]+]] = load i32, ptr %a, align 4
// CHECK: %{{[0-9]+}} = call i32 @llvm.fpga.reg.i32(i32 %[[LOADA3]])
  struct st i = {1, 5.0f};
  struct st ii = __builtin_fpga_reg(i);
// CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 4 %i, ptr align 4 @__const._Z3foov.i, i64 8, i1 false)
// CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 4 %agg-temp, ptr align 4 %i, i64 8, i1 false)
// CHECK: call void @llvm.fpga.reg.struct.p0(ptr %ii, ptr %agg-temp)
  struct st iii;
  iii = __builtin_fpga_reg(ii);
// CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 4 %agg-temp2, ptr align 4 %ii, i64 8, i1 false)
// CHECK: call void @llvm.fpga.reg.struct.p0(ptr %ref.tmp, ptr %agg-temp2)
// CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 4 %iii, ptr align 4 %ref.tmp, i64 8, i1 false)
  struct st *iiii = __builtin_fpga_reg(&iii);
// CHECK: %[[IIIIRES:[0-9]+]] = call ptr @llvm.fpga.reg.p0(ptr %iii)
// CHECK: store ptr %[[IIIIRES]], ptr %iiii, align 8
  union un u1 = {1};
// CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 4 %u1, ptr align 4 @__const._Z3foov.u1
  union un u2, *u3;
  u2 = __builtin_fpga_reg(u1);
// CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 4 %agg-temp4, ptr align 4 %u1, i64 4, i1 false)
// CHECK: call void @llvm.fpga.reg.struct.p0(ptr %ref.tmp3, ptr %agg-temp4)
// CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 4 %u2, ptr align 4 %ref.tmp3, i64 4, i1 false)
  u3 = __builtin_fpga_reg(&u2);
// CHECK: %[[U3INIT:[0-9]+]] = call ptr @llvm.fpga.reg.p0(ptr %u2)
// CHECK: store ptr %[[U3INIT]], ptr %u3, align 8
  int *ap = &a;
// CHECK: store ptr %a, ptr %ap, align 8
// CHECK: %[[APLOAD:[0-9]+]] = load ptr, ptr %ap, align 8
  int *bp = __builtin_fpga_reg(ap);
// CHECK: %{{[0-9]+}} = call ptr @llvm.fpga.reg.p0(ptr %[[APLOAD]])

  struct stnonpod s1(123);
  struct stnonpod s2 = __builtin_fpga_reg(s1);
// CHECK: call void @_ZN8stnonpodC1Ei(ptr {{[^,]*}}%s1, i32 {{[^,]*}}123)
// CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 4 %agg-temp5, ptr align 4 %s1, i64 4, i1 false)
// CHECK: call void @llvm.fpga.reg.struct.p0(ptr %s2, ptr %agg-temp5)

  cl c1(123);
  cl c2 = __builtin_fpga_reg(c1);
// CHECK: call void @_ZN2clC1Ei(ptr {{[^,]*}}%c1, i32 {{[^,]*}}123)
// CHECK: call void @llvm.memcpy.p0.p0.i64(ptr align 4 %agg-temp6, ptr align 4 %c1, i64 4, i1 false)
// CHECK: call void @llvm.fpga.reg.struct.p0(ptr %c2, ptr %agg-temp6)
}

// CHECK: declare i32 @llvm.fpga.reg.i32(i32)
// CHECK: declare float @llvm.fpga.reg.f32(float)
// CHECK: declare void @llvm.fpga.reg.struct.p0(ptr, ptr)
// CHECK: declare ptr @llvm.fpga.reg.p0(ptr)
