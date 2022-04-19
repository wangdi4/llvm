// RUN: %clang_cc1 %s -O0 -triple spir-unknown-unknown-intelfpga -emit-llvm -opaque-pointers -o - | FileCheck %s

struct st {
  int a;
  float b;
};
// CHECK: %struct.st = type { i32, float }

union un {
  int a;
  char c[4];
};
// CHECK: %union.un = type { i32 }

typedef int myInt;

void foo() {
// CHECK: define{{.*}}spir_func void @foo()
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
// CHECK: call void @llvm.memcpy.p0.p2.i32(ptr align 4 %i, ptr addrspace(2) align 4 @__const.foo.i
// CHECK: call void @llvm.memcpy.p0.p0.i32(ptr align 4 %agg-temp, ptr align 4 %i, i32 8, i1 false)
// CHECK: call void @llvm.fpga.reg.struct.p0(ptr %ii, ptr %agg-temp)
  struct st iii;
  iii = __builtin_fpga_reg(ii);
// CHECK: call void @llvm.memcpy.p0.p0.i32(ptr align 4 %[[AT3:agg-temp[0-9]*]], ptr align 4 %ii, i32 8, i1 false)
// CHECK: call void @llvm.fpga.reg.struct.p0(ptr %[[TMP2:tmp[0-9]*]], ptr %[[AT3]])
// CHECK: call void @llvm.memcpy.p0.p0.i32(ptr align 4 %iii, ptr align 4 %[[TMP2]], i32 8, i1 false)
  struct st *iiii = __builtin_fpga_reg(&iii);
// CHECK: %[[IIIIRES:[0-9]+]] = call ptr @llvm.fpga.reg.p0(ptr %iii)
// CHECK: store ptr %[[IIIIRES]], ptr %iiii, align 4
  union un u1 = {1};
// CHECK: call void @llvm.memcpy.p0.p2.i32(ptr align 4 %u1, ptr addrspace(2) align 4 @__const.foo.u1
  union un u2, *u3;
  u2 = __builtin_fpga_reg(u1);
// CHECK: call void @llvm.memcpy.p0.p0.i32(ptr align 4 %[[ATMP5:agg-temp[0-9]*]], ptr align 4 %u1, i32 4, i1 false)
// CHECK: call void @llvm.fpga.reg.struct.p0(ptr %[[TMP4:tmp[0-9]*]], ptr %[[ATMP5]])
// CHECK: call void @llvm.memcpy.p0.p0.i32(ptr align 4 %u2, ptr align 4 %[[TMP4]], i32 4, i1 false)
  u3 = __builtin_fpga_reg(&u2);
// CHECK: %[[U3INIT:[0-9]+]] = call ptr @llvm.fpga.reg.p0(ptr %u2)
// CHECK: store ptr %[[U3INIT]], ptr %u3, align 4
  int *ap = &a;
// CHECK: store ptr %a, ptr %ap, align 4
// CHECK: %[[APLOAD:[0-9]+]] = load ptr, ptr %ap, align 4
  int *bp = __builtin_fpga_reg(ap);
// CHECK: %{{[0-9]+}} = call ptr @llvm.fpga.reg.p0(ptr %[[APLOAD]])
}

// CHECK: declare i32 @llvm.fpga.reg.i32(i32)
// CHECK: declare float @llvm.fpga.reg.f32(float)
// CHECK: declare void @llvm.fpga.reg.struct.p0(ptr, ptr)
// CHECK: declare ptr @llvm.fpga.reg.p0(ptr)

