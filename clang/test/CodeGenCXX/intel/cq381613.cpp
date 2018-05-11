// RUN: %clang_cc1 -fintel-compatibility -O0 -triple x86_64-unknown-linux-gnu %s -emit-llvm -o - | FileCheck %s --check-prefixes CHECK,LIN
// RUN: %clang_cc1 -fintel-compatibility -O0 -triple x86_64-windows-msvc -emit-llvm -o - %s | FileCheck %s --check-prefixes CHECK,WIN

extern "C" int printf (const char *__restrict __format, ...);

struct C3 {
  int m0;
  C3():m0(7) {};
  ~C3() {
    m0 = (int)3;
    printf(" ~C3()\n");
  }
};

struct C2 {
  C3 m0;
  C2() { printf("C2()"); }
};

int main() {
// LIN: define i32 @main()
// WIN: define dso_local i32 @main()
  C2 o2;
// CHECK: [[o2:%.+]] = alloca %struct.C2
// CHECK-NEXT: [[tmp:%.+]] = alloca %struct.C3
  printf("o2.m0 = %x\n", o2.m0);
// CHECK: call {{.*}} [[ctor:@.+]](%struct.C2* [[o2]])
// CHECK-NEXT: [[m0:%.+]] = getelementptr inbounds %struct.C2, %struct.C2* [[o2]], i32 0, i32 0
// CHECK-NEXT: [[t0:%.+]] = bitcast %struct.C3* %agg.tmp to i8*
// CHECK-NEXT: [[t1:%.+]] = bitcast %struct.C3* %m0 to i8*
// CHECK-NEXT: call void @llvm.memcpy.p0i8.p0i8.i64(i8* align 4 [[t0]], i8* align 4 [[t1]]
// WIN-NEXT: [[coer:%.+]] = getelementptr inbounds %struct.C3, %struct.C3* [[tmp]], i32 0, i32 0
// WIN-NEXT: [[t2:%.+]] = load i32, i32* [[coer]]
// WIN-NEXT: @printf(i8* getelementptr inbounds ({{.+}}), i32 [[t2]])
// LIN-NEXT: @printf({{.+}}, %struct.C3* [[tmp]])
// CHECK: call void [[dtor:@.+]](%struct.C2* [[o2]])
// CHECK-NEXT: ret
}
