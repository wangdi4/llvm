// RUN: %clang_cc1 %s -O0 -triple spir-unknown-unknown-intelfpga -emit-llvm -no-opaque-pointers -o - | FileCheck %s

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
// CHECK: store i32 123, i32* %a, align 4
// CHECK: store i32 321, i32* %myA, align 4
// CHECK: %[[LOADA1:[0-9]+]] = load i32, i32* %a, align 4
  int b = __builtin_fpga_reg(a);
// CHECK: %[[B:[0-9]+]] = call i32 @llvm.fpga.reg.i32(i32 %[[LOADA1]])
// CHECK: store i32 %[[B]], i32* %b, align 4
  int myB = __builtin_fpga_reg(myA);
// CHECK: %[[LOADMYA:[0-9]+]] = load i32, i32* %myA, align 4
// CHECK: %[[myB:[0-9]+]] = call i32 @llvm.fpga.reg.i32(i32 %[[LOADMYA]])
// CHECK: store i32 %[[myB]], i32* %myB, align 4
  int c = __builtin_fpga_reg(2.0f);
// CHECK: %{{[0-9]+}} = call float @llvm.fpga.reg.f32(float 2.000000e+00)
  int d = __builtin_fpga_reg( __builtin_fpga_reg( b+12 ));
// CHECK: %[[LOADB:[0-9]+]] = load i32, i32* %b, align 4
// CHECK: %add = add nsw i32 %[[LOADB]], 12
// CHECK: %[[D1:[0-9]+]] = call i32 @llvm.fpga.reg.i32(i32 %add)
// CHECK: %{{[0-9]+}} = call i32 @llvm.fpga.reg.i32(i32 %[[D1]])
  int e = __builtin_fpga_reg( __builtin_fpga_reg( a+b ));
// CHECK: %[[LOADA2:[0-9]+]] = load i32, i32* %a, align 4
// CHECK: %[[LOADB2:[0-9]+]] = load i32, i32* %b, align 4
// CHECK: %add1 = add nsw i32 %[[LOADA2]], %[[LOADB2]]
// CHECK: %[[E1:[0-9]+]] = call i32 @llvm.fpga.reg.i32(i32 %add1)
// CHECK: %{{[0-9]+}} = call i32 @llvm.fpga.reg.i32(i32 %[[E1]])
  int f;
  f = __builtin_fpga_reg(a);
// CHECK: %[[LOADA3:[0-9]+]] = load i32, i32* %a, align 4
// CHECK: %{{[0-9]+}} = call i32 @llvm.fpga.reg.i32(i32 %[[LOADA3]])
  struct st i = {1, 5.0f};
  struct st ii = __builtin_fpga_reg(i);
// CHECK: %[[IBC1:[0-9]+]] = bitcast %struct.st* %i to i8*
// CHECK: call void @llvm.memcpy.p0i8.p2i8.i32(i8* align 4 %[[IBC1]], i8 addrspace(2)* align 4 bitcast (%struct.st addrspace(2)* @__const.foo.i to i8 addrspace(2)*)
// CHECK: %[[ATBC:[0-9]+]] = bitcast %struct.st* %agg-temp to i8*
// CHECK: %[[IBC2:[0-9]+]] = bitcast %struct.st* %i to i8*
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i32(i8* align 4 %[[ATBC]], i8* align 4 %[[IBC2]], i32 8, i1 false)
// CHECK: call void @llvm.fpga.reg.struct.p0s_struct.sts(%struct.st* %ii, %struct.st* %agg-temp)
  struct st iii;
  iii = __builtin_fpga_reg(ii);
// CHECK: %[[AT3BC:[0-9]+]] = bitcast %struct.st* %[[AT3:agg-temp[0-9]*]] to i8*
// CHECK: %[[IIBC:[0-9]+]] = bitcast %struct.st* %ii to i8*
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i32(i8* align 4 %[[AT3BC]], i8* align 4 %[[IIBC]], i32 8, i1 false)
// CHECK: call void @llvm.fpga.reg.struct.p0s_struct.sts(%struct.st* %[[TMP2:tmp[0-9]*]], %struct.st* %[[AT3]])
// CHECK: %[[IIIBC:[0-9]+]] = bitcast %struct.st* %iii to i8*
// CHECK: %[[TMP2BC:[0-9]+]] = bitcast %struct.st* %[[TMP2]] to i8*
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i32(i8* align 4 %[[IIIBC]], i8* align 4 %[[TMP2BC]], i32 8, i1 false)
  struct st *iiii = __builtin_fpga_reg(&iii);
// CHECK: %[[IIIIRES:[0-9]+]] = call %struct.st* @llvm.fpga.reg.p0s_struct.sts(%struct.st* %iii)
// CHECK: store %struct.st* %[[IIIIRES]], %struct.st** %iiii, align 4
  union un u1 = {1};
// CHECK: %[[U1INIT:[0-9]+]] = bitcast %union.un* %u1 to i8*
// CHECK: call void @llvm.memcpy.p0i8.p2i8.i32(i8* align 4 %[[U1INIT]], i8 addrspace(2)* align 4 bitcast (%union.un addrspace(2)* @__const.foo.u1 to i8 addrspace(2)*)
  union un u2, *u3;
  u2 = __builtin_fpga_reg(u1);
// CHECK: %[[U1TMP:[0-9]+]] = bitcast %union.un* %[[ATMP5:agg-temp[0-9]*]] to i8*
// CHECK: %[[U1BC:[0-9]+]] = bitcast %union.un* %u1 to i8*
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i32(i8* align 4 %[[U1TMP]], i8* align 4 %[[U1BC]], i32 4, i1 false)
// CHECK: call void @llvm.fpga.reg.struct.p0s_union.uns(%union.un* %[[TMP4:tmp[0-9]*]], %union.un* %[[ATMP5]])
// CHECK: %[[U2BC:[0-9]+]] = bitcast %union.un* %u2 to i8*
// CHECK: %[[RETBC:[0-9]+]] = bitcast %union.un* %[[TMP4]] to i8*
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i32(i8* align 4 %[[U2BC]], i8* align 4 %[[RETBC]], i32 4, i1 false)
  u3 = __builtin_fpga_reg(&u2);
// CHECK: %[[U3INIT:[0-9]+]] = call %union.un* @llvm.fpga.reg.p0s_union.uns(%union.un* %u2)
// CHECK: store %union.un* %[[U3INIT]], %union.un** %u3, align 4
  int *ap = &a;
// CHECK: store i32* %a, i32** %ap, align 4
// CHECK: %[[APLOAD:[0-9]+]] = load i32*, i32** %ap, align 4
  int *bp = __builtin_fpga_reg(ap);
// CHECK: %{{[0-9]+}} = call i32* @llvm.fpga.reg.p0i32(i32* %[[APLOAD]])
}

// CHECK: declare i32 @llvm.fpga.reg.i32(i32)
// CHECK: declare float @llvm.fpga.reg.f32(float)
// CHECK: declare void @llvm.fpga.reg.struct.p0s_struct.sts(%struct.st*, %struct.st*)
// CHECK: declare i32* @llvm.fpga.reg.p0i32(i32*)

