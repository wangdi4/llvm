// RUN: %clang_cc1 -fintel-compatibility -emit-llvm -verify -o - %s | FileCheck %s
//***INTEL: pragma parallel test

// CHECK: target datalayout{{.*}}
// CHECK-NOT: call void (metadata, ...) @llvm.intel.pragma{{.*}}
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
// CHECK: call void (metadata, ...) @llvm.intel.pragma(metadata !"PARALLEL")
#pragma parallel
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"PARALLEL", metadata !"QUAL_ALWAYS")
#pragma parallel always
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"PARALLEL", metadata !"QUAL_ALWAYS", metadata !"QUAL_ASSERT")
#pragma parallel always assert
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"PARALLEL", metadata !"QUAL_PRIVATE", metadata !"LVALUE", metadata !"SIMPLE", metadata i32* @gVar, metadata !"QUAL_PRIVATE", metadata !"LVALUE", metadata !"SIMPLE", metadata i32* %lVar, metadata !"QUAL_PRIVATE", metadata !"LVALUE", metadata !"SIMPLE", metadata %struct.St1* @a, metadata !"QUAL_PRIVATE", metadata !"LVALUE", metadata !"SIMPLE", metadata [10 x i32]* %lArr, metadata !"QUAL_PRIVATE", metadata !"LVALUE", metadata !"SIMPLE", metadata [10 x i32]* @gArr)
#pragma parallel private(gVar, lVar), private (a, lArr, gArr[argc])
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"PARALLEL", metadata !"QUAL_PRIVATE", metadata !"LVALUE", metadata !"SIMPLE", metadata [10 x i32]* @gArr)
#pragma parallel private(gArr[2])
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"PARALLEL", metadata !"QUAL_LASTPRIVATE", metadata !"LVALUE", metadata !"SIMPLE", metadata i32* @gVar, metadata !"QUAL_LASTPRIVATE", metadata !"LVALUE", metadata !"SIMPLE", metadata i32* %lVar, metadata !"QUAL_FIRSTPRIVATE", metadata !"LVALUE", metadata !"SIMPLE", metadata %struct.St1* @a, metadata !"QUAL_FIRSTPRIVATE", metadata !"LVALUE", metadata !"SIMPLE", metadata [10 x i32]* %lArr, metadata !"QUAL_FIRSTPRIVATE", metadata !"LVALUE", metadata !"SIMPLE", metadata [10 x [40 x double]]* %lDblArr)
#pragma parallel lastprivate(gVar, lVar), firstprivate (a, lArr, lDblArr[20])
// CHECK-NEXT: call void (metadata, ...) @llvm.intel.pragma(metadata !"PARALLEL", metadata !"QUAL_LASTPRIVATE", metadata !"LVALUE", metadata !"SIMPLE", metadata [10 x %struct.St1]* %lSt1, metadata !"QUAL_LASTPRIVATE", metadata !"LVALUE", metadata !"SIMPLE", metadata [10 x [30 x %struct.St1]]* %lStSt1, metadata !"QUAL_FIRSTPRIVATE", metadata !"LVALUE", metadata !"SIMPLE", metadata [10 x %struct.St1]* @gSt1, metadata !"QUAL_FIRSTPRIVATE", metadata !"LVALUE", metadata !"SIMPLE", metadata [10 x [30 x %struct.St1]]* @gStSt1)
#pragma parallel lastprivate(lSt1[3], lStSt1[40]), firstprivate (gSt1, gStSt1)
  return (test(argc));
}

void asasas(char **argv) {
  ;
}

