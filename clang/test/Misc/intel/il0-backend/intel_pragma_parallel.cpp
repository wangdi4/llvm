// RUN: %clang_cc1 -IntelCompat -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: pragma parallel test

// CHECK: target datalayout{{.*}}
// CHECK-NOT: call void @llvm.intel.pragma{{.*}}
#pragma parallel                // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel always         // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel always assert // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel fgsdswf c // expected-warning {{this pragma must immediately precede a statement}}
struct Class1 {
  int as;
#pragma parallel                // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel always         // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel always assert // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel fgsdswf c // expected-warning {{this pragma must immediately precede a statement}}
  char b;
};

#pragma parallel                // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel always         // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel always assert // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel fgsdswf c // expected-warning {{this pragma must immediately precede a statement}}
struct St1 {
  float rrr;
  double e;
} a;

#pragma parallel                // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel always         // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel always assert // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel fgsdswf c // expected-warning {{this pragma must immediately precede a statement}}
int test(int argc) {
  int wwww;
#pragma parallel                // expected-warning {{this pragma may not be used here}}
#pragma parallel always         // expected-warning {{this pragma may not be used here}}
#pragma parallel always assert // expected-warning {{this pragma may not be used here}}
  float fdf;
  return (argc);
}

#pragma parallel                // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel always         // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel always assert // expected-warning {{this pragma must immediately precede a statement}}
#pragma parallel fgsdswf c // expected-warning {{this pragma must immediately precede a statement}}
int gVar;
double gDbl;
int gArr[10];
double gDblArr[10][40];
St1 gSt1[10];
St1 gStSt1[10][30];
// CHECK: define i32 @main
int main(int argc, char **argv) {
  int lVar;
  double lDbl;
  int lArr[10];
  double lDblArr[10][40];
  St1 lSt1[10];
  St1 lStSt1[10][30];
// CHECK: call void @llvm.intel.pragma(metadata !1)
#pragma parallel
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !2)
#pragma parallel always
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !3)
#pragma parallel always assert
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !{metadata !"PARALLEL", metadata !"QUAL_PRIVATE", metadata !"LVALUE", metadata !"SIMPLE", i32* @gVar, metadata !"QUAL_PRIVATE", metadata !"LVALUE", metadata !"SIMPLE", i32* %lVar, metadata !"QUAL_PRIVATE", metadata !"LVALUE", metadata !"SIMPLE", %struct.St1* @a, metadata !"QUAL_PRIVATE", metadata !"LVALUE", metadata !"SIMPLE", [10 x i32]* %lArr, metadata !"QUAL_PRIVATE", metadata !"LVALUE", metadata !"SIMPLE", [10 x i32]* @gArr})
#pragma parallel private(gVar, lVar), private (a, lArr, gArr[argc])
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !4)
#pragma parallel private(gArr[2])
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !{metadata !"PARALLEL", metadata !"QUAL_LASTPRIVATE", metadata !"LVALUE", metadata !"SIMPLE", i32* @gVar, metadata !"QUAL_LASTPRIVATE", metadata !"LVALUE", metadata !"SIMPLE", i32* %lVar, metadata !"QUAL_FIRSTPRIVATE", metadata !"LVALUE", metadata !"SIMPLE", %struct.St1* @a, metadata !"QUAL_FIRSTPRIVATE", metadata !"LVALUE", metadata !"SIMPLE", [10 x i32]* %lArr, metadata !"QUAL_FIRSTPRIVATE", metadata !"LVALUE", metadata !"SIMPLE", [10 x [40 x double]]* %lDblArr})
#pragma parallel lastprivate(gVar, lVar), firstprivate (a, lArr, lDblArr[20])
// CHECK-NEXT: call void @llvm.intel.pragma(metadata !{metadata !"PARALLEL", metadata !"QUAL_LASTPRIVATE", metadata !"LVALUE", metadata !"SIMPLE", [10 x %struct.St1]* %lSt1, metadata !"QUAL_LASTPRIVATE", metadata !"LVALUE", metadata !"SIMPLE", [10 x [30 x %struct.St1]]* %lStSt1, metadata !"QUAL_FIRSTPRIVATE", metadata !"LVALUE", metadata !"SIMPLE", [10 x %struct.St1]* @gSt1, metadata !"QUAL_FIRSTPRIVATE", metadata !"LVALUE", metadata !"SIMPLE", [10 x [30 x %struct.St1]]* @gStSt1})
#pragma parallel lastprivate(lSt1[3], lStSt1[40]), firstprivate (gSt1, gStSt1)
  return (test(argc));
}

void asasas(char **argv) {
  ;
}

// CHECK: !1 = metadata !{metadata !"PARALLEL"}
// CHECK-NEXT: !2 = metadata !{metadata !"PARALLEL", metadata !"QUAL_ALWAYS"}
// CHECK-NEXT: !3 = metadata !{metadata !"PARALLEL", metadata !"QUAL_ALWAYS", metadata !"QUAL_ASSERT"}
// CHECK-NEXT: !4 = metadata !{metadata !"PARALLEL", metadata !"QUAL_PRIVATE", metadata !"LVALUE", metadata !"SIMPLE", [10 x i32]* @gArr}
