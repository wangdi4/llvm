//RUN: %clang_cc1 -fhls -triple x86_64-unknown-linux-gnu -emit-llvm -no-opaque-pointers -o - %s | FileCheck %s

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
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %[[IBC1]], i8* align 4 bitcast (%struct.st* @__const._Z3foov.i to i8*)
// CHECK: %[[ATBC:[0-9]+]] = bitcast %struct.st* %agg-temp to i8*
// CHECK: %[[IBC2:[0-9]+]] = bitcast %struct.st* %i to i8*
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %[[ATBC]], i8* align 4 %[[IBC2]], i64 8, i1 false)
// CHECK: call void @llvm.fpga.reg.struct.p0s_struct.sts(%struct.st* %ii, %struct.st* %agg-temp)
  struct st iii;
  iii = __builtin_fpga_reg(ii);
// CHECK: %[[AT3BC:[0-9]+]] = bitcast %struct.st* %agg-temp2 to i8*
// CHECK: %[[IIBC:[0-9]+]] = bitcast %struct.st* %ii to i8*
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %[[AT3BC]], i8* align 4 %[[IIBC]], i64 8, i1 false)
// CHECK: call void @llvm.fpga.reg.struct.p0s_struct.sts(%struct.st* %ref.tmp, %struct.st* %agg-temp2)
// CHECK: %[[IIIBC:[0-9]+]] = bitcast %struct.st* %iii to i8*
// CHECK: %[[TMP2BC:[0-9]+]] = bitcast %struct.st* %ref.tmp to i8*
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %[[IIIBC]], i8* align 4 %[[TMP2BC]], i64 8, i1 false)
  struct st *iiii = __builtin_fpga_reg(&iii);
// CHECK: %[[IIIIRES:[0-9]+]] = call %struct.st* @llvm.fpga.reg.p0s_struct.sts(%struct.st* %iii)
// CHECK: store %struct.st* %[[IIIIRES]], %struct.st** %iiii, align 8
  union un u1 = {1};
// CHECK: %[[U1INIT:[0-9]+]] = bitcast %union.un* %u1 to i8*
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %[[U1INIT]], i8* align 4 bitcast (%union.un* @__const._Z3foov.u1 to i8*)
  union un u2, *u3;
  u2 = __builtin_fpga_reg(u1);
// CHECK: %[[U1TMP:[0-9]+]] = bitcast %union.un* %agg-temp4 to i8*
// CHECK: %[[U1BC:[0-9]+]] = bitcast %union.un* %u1 to i8*
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %[[U1TMP]], i8* align 4 %[[U1BC]], i64 4, i1 false)
// CHECK: call void @llvm.fpga.reg.struct.p0s_union.uns(%union.un* %ref.tmp3, %union.un* %agg-temp4)
// CHECK: %[[U2BC:[0-9]+]] = bitcast %union.un* %u2 to i8*
// CHECK: %[[RETBC:[0-9]+]] = bitcast %union.un* %ref.tmp3 to i8*
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %[[U2BC]], i8* align 4 %[[RETBC]], i64 4, i1 false)
  u3 = __builtin_fpga_reg(&u2);
// CHECK: %[[U3INIT:[0-9]+]] = call %union.un* @llvm.fpga.reg.p0s_union.uns(%union.un* %u2)
// CHECK: store %union.un* %[[U3INIT]], %union.un** %u3, align 8
  int *ap = &a;
// CHECK: store i32* %a, i32** %ap, align 8
// CHECK: %[[APLOAD:[0-9]+]] = load i32*, i32** %ap, align 8
  int *bp = __builtin_fpga_reg(ap);
// CHECK: %{{[0-9]+}} = call i32* @llvm.fpga.reg.p0i32(i32* %[[APLOAD]])

  struct stnonpod s1(123);
  struct stnonpod s2 = __builtin_fpga_reg(s1);
// CHECK: call void @_ZN8stnonpodC1Ei(%struct.stnonpod* {{[^,]*}}%s1, i32 {{[^,]*}}123)
// CHECK: %[[S1BC1:[0-9]+]] = bitcast %struct.stnonpod* %agg-temp5 to i8*
// CHECK: %[[S1BC2:[0-9]+]] = bitcast %struct.stnonpod* %s1 to i8*
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %[[S1BC1]], i8* align 4 %[[S1BC2]], i64 4, i1 false)
// CHECK: call void @llvm.fpga.reg.struct.p0s_struct.stnonpods(%struct.stnonpod* %s2, %struct.stnonpod* %agg-temp5)

  cl c1(123);
  cl c2 = __builtin_fpga_reg(c1);
// CHECK: call void @_ZN2clC1Ei(%class.cl* {{[^,]*}}%c1, i32 {{[^,]*}}123)
// CHECK: %[[C1BC1:[0-9]+]] = bitcast %class.cl* %agg-temp6 to i8*
// CHECK: %[[C1BC2:[0-9]+]] = bitcast %class.cl* %c1 to i8*
// CHECK: call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 %[[C1BC1]], i8* align 4 %[[C1BC2]], i64 4, i1 false)
// CHECK: call void @llvm.fpga.reg.struct.p0s_class.cls(%class.cl* %c2, %class.cl* %agg-temp6)
}

// CHECK: declare i32 @llvm.fpga.reg.i32(i32)
// CHECK: declare float @llvm.fpga.reg.f32(float)
// CHECK: declare void @llvm.fpga.reg.struct.p0s_struct.sts(%struct.st*, %struct.st*)
// CHECK: declare i32* @llvm.fpga.reg.p0i32(i32*)
