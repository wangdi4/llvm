// Test with fast math
// if INTEL_CUSTOMIZATION
// RUN: %clang_cc1 -triple x86_64-linux-gnu -emit-llvm -DFAST \
// RUN: -mreassociate -opaque-pointers \
// RUN: -o - %s | FileCheck --check-prefixes CHECK,CHECKFASTINTL,CHECKNPP %s
// endif // INTEL_CUSTOMIZATION
//
// RUN: %clang_cc1 -triple i386-pc-linux-gnu -emit-llvm -DFAST \
// RUN: -mreassociate \
// RUN: -o - %s | FileCheck --check-prefixes CHECK,CHECKFAST,CHECKNP %s
//
// if INTEL_CUSTOMIZATION
// RUN: %clang_cc1 -triple i386-pc-linux-gnu -emit-llvm -DFAST \
// RUN: -mreassociate -opaque-pointers \
// RUN: -o - %s | FileCheck --check-prefixes CHECK,CHECKSTINT-32 %s
// endif // INTEL_CUSTOMIZATION

// Test with fast math and fprotect-parens
// RUN: %clang_cc1 -triple i386-pc-linux-gnu -emit-llvm -DFAST \
// RUN: -mreassociate -fprotect-parens -ffp-contract=on\
// RUN: -o - %s | FileCheck --check-prefixes CHECK,CHECKFAST,CHECKPP %s
//
// Test without fast math: llvm intrinsic not created
// RUN: %clang_cc1 -triple i386-pc-linux-gnu -emit-llvm -fprotect-parens\
// RUN: -o - %s | FileCheck --implicit-check-not="llvm.arithmetic.fence" %s
//
// if INTEL_CUSTOMIZATION
// RUN: %clang_cc1 -triple x86_64-linux-gnu -emit-llvm -fprotect-parens \
// RUN: -o - %s -opaque-pointers \
// RUN: | FileCheck --implicit-check-not="llvm.arithmetic.fence" %s
// endif // INTEL_CUSTOMIZATION
//
// Test with fast math on spir target
// RUN: %clang_cc1 -triple spir64  -emit-llvm -DFAST \
// RUN: -mreassociate -o - %s \
// RUN: | FileCheck --check-prefixes CHECK,CHECKFAST,CHECKNP %s
//

int v;
int addit(float a, float b) {
  // CHECK: define {{.*}}@addit(float noundef %a, float noundef %b) #0 {
  // if INTEL_CUSTOMIZATION
  // CHECKSTINT-32: [[P_32:%.*]] = alloca ptr, align 4
  // CHECKSTINT-32-NEXT: [[I_32:%.*]] = alloca i32, align 4
  // CHECKSTINT-32: [[K_32:%.*]] = alloca ptr, align 4
  // endif // INTEL_CUSTOMIZATION

  _Complex double cd, cd1;
  cd = __arithmetic_fence(cd1);
  // CHECKFAST: call{{.*}} double @llvm.arithmetic.fence.f64({{.*}}real)
  // CHECKFAST: call{{.*}} double @llvm.arithmetic.fence.f64({{.*}}imag)

  // if INTEL_CUSTOMIZATION
  // CHECKFASTINTL: [[A:%.*]] = alloca float, align 4
  // CHECKFASTINTL-NEXT: [[B:%.*]] = alloca float, align 4
  // CHECKFASTINTL-NEXT: [[CD:%.*]] = alloca { double, double }, align 8
  // CHECKFASTINTL-NEXT: [[CD1:%.*]] = alloca { double, double }, align 8
  // CHECKFASTINTL-NEXT: [[VEC1:%.*]] = alloca <2 x float>, align 8
  // CHECKFASTINTL-NEXT: [[VEC2:%.*]] = alloca <2 x float>, align 8
  // CHECKFASTINTL-NEXT: [[P:%.*]] = alloca ptr, align 8
  // CHECKFASTINTL-NEXT: [[I:%.*]] = alloca i32, align 4
  // CHECKFASTINTL-NEXT: [[K:%.*]] = alloca ptr, align 8
  // CHECKFASTINTL-NEXT: store float {{.*}}, ptr [[A]], align 4
  // CHECKFASTINTL-NEXT: store float {{.*}}, ptr [[B]], align 4
  // CHECKFASTINTL-NEXT: [[GEP_CD1_R:%.*]] = getelementptr inbounds { double, double }, ptr [[CD1]], i32 0, i32 0
  // CHECKFASTINTL-NEXT: [[CD1_REAL:%.*]] = load double, ptr [[GEP_CD1_R]], align 8
  // CHECKFASTINTL-NEXT: [[GEP_CD1_I:%.*]] = getelementptr inbounds { double, double }, ptr [[CD1]], i32 0, i32 1
  // CHECKFASTINTL-NEXT: [[CD1_IMAG:%.*]] = load double, ptr [[GEP_CD1_I]], align 8
  // CHECKFASTINTL-NEXT: [[TMP0:%.*]] = call{{.*}} double @llvm.arithmetic.fence.f64(double [[CD1_REAL]])
  // CHECKFASTINTL-NEXT: [[TMP1:%.*]] = call{{.*}} double @llvm.arithmetic.fence.f64(double [[CD1_IMAG]])
  // endif // INTEL_CUSTOMIZATION
  // Vector should be supported.
  typedef float __v2f32 __attribute__((__vector_size__(8)));
  __v2f32 vec1, vec2;
  vec1 = __arithmetic_fence(vec2);
  // if INTEL_CUSTOMIZATION
  // CHECKFASTINTL-NEXT: [[CD_REAL:%.*]] = getelementptr inbounds { double, double }, ptr [[CD]], i32 0, i32 0
  // CHECKFASTINTL-NEXT: [[CD_IMAG:%.*]] = getelementptr inbounds { double, double }, ptr [[CD]], i32 0, i32 1

  // CHECKFASTINTL-NEXT: store double [[TMP0]], ptr [[CD_REAL]], align 8
  // CHECKFASTINTL-NEXT: store double [[TMP1]], ptr [[CD_IMAG]], align 8

  // CHECKFASTINTL-NEXT: [[L2:%.*]] = load <2 x float>, ptr [[VEC2]], align 8
  // CHECKFASTINTL-NEXT: [[TMP3:%.*]] = call{{.*}} <2 x float> @llvm.arithmetic.fence.v2f32(<2 x float> [[L2]]
  // CHECKFASTINTL-NEXT: store <2 x float> [[TMP3]], ptr [[VEC1]], align 8
  // endif // INTEL_CUSTOMIZATION

  // CHECKFAST: call{{.*}} <2 x float> @llvm.arithmetic.fence.v2f32
  vec2 = (vec2 + vec1);
  // CHECKPP: call{{.*}} <2 x float> @llvm.arithmetic.fence.v2f32

  // if INTEL_CUSTOMIZATION
  // CHECKFASTINTL: [[TMP4:%.*]] = load <2 x float>, ptr [[VEC2]], align 8
  // CHECKFASTINTL-NEXT: [[TMP5:%.*]] = load <2 x float>, ptr [[VEC1]], align 8
  // CHECKFASTINTL: [[TMP6:%.*]] = fadd reassoc <2 x float> [[TMP4]], [[TMP5]]

  // CHECKNPP: store <2 x float> [[TMP6]], ptr [[VEC2]], align 8
  // endif // INTEL_CUSTOMIZATION

  v = __arithmetic_fence(a + b);
  // CHECKFAST: call{{.*}} float @llvm.arithmetic.fence.f32(float %add{{.*}})

  v = (a + b);
  // CHECKPP: call{{.*}} float @llvm.arithmetic.fence.f32(float %add{{.*}})
  v = a + (b*b);
  // CHECKPP: fmul reassoc
  // CHECKPP-NEXT: call{{.*}} float @llvm.arithmetic.fence.f32(float %mul)
  // CHECKNP: fmul
  // CHECKNP: fadd
  v = b + a*a;
  // CHECKPP: call{{.*}} float @llvm.fmuladd.f32
  // CHECKNP: fmul
  // CHECKNP: fadd
  v = b + __arithmetic_fence(a*a); // Fence blocks recognition of FMA
  // CHECKPP: fmul
  // CHECKNP: fmul

  b = (a);
  (a) = b;
  // CHECK-NEXT fptosi
  // CHECK-NEXT store i32
  // CHECK-NEXT load float
  // CHECK-NEXT store float
  // CHECK-NEXT load float
  // CHECK-NEXT store float

  // if INTEL_CUSTMIZATION
  int *p;
  int i = __fence(1 + 2);
  // CHECKFASTINTL: store i32 3, ptr [[I]], align 4
  // CHECK-NOT: call{{.*}} float @llvm.arithmetic.fence.int(float {{.*}})

  int *k = __fence(i + p);
  // CHECKFASTINTL-NEXT: [[TMP26:%.*]] = load i32, ptr [[I]], align 4
  // CHECKFASTINTL-NEXT: [[TMP27:%.*]] =  load ptr, ptr [[P]], align 8
  // CHECKFASTINTL-NEXT: [[IDX:%.*]] = sext i32 [[TMP26]] to i64
  // CHECKFASTINTL-NEXT: [[GEP11:%.*]] = getelementptr inbounds i32, ptr [[TMP27]], i64 [[IDX]]
  // CHECKFASTINTL: store ptr [[GEP11]], ptr [[K]], align 8

  // CHECKSTINT-32: [[TMP24:%.*]] = load i32, ptr [[I_32]], align 4
  // CHECKSTINT-32-NEXT: [[TMP25:%.*]] = load ptr, ptr [[P_32]], align 4
  // CHECKSTINT-32-NEXT: [[GEP10:%.*]] = getelementptr inbounds i32, ptr [[TMP25]], i32 [[TMP24]]
  // CHECKSTINT-32-NEXT: store ptr [[GEP10]], ptr [[K_32]], align 4
  // CHECK-NOT: call{{.*}} float @llvm.arithmetic.fence.int(float {{.*}})
  // endif // INTEL_CUSTOMIZATION

  return 0;
  // CHECK-NEXT ret i32 0
}
int addit1(int a, int b) {
  // CHECK: define {{.*}}@addit1(i32 noundef %a, i32 noundef %b{{.*}}
  v = (a + b);
  // CHECK-NOT: call{{.*}} float @llvm.arithmetic.fence.int(float noundef %add)
  return 0;
}
#ifdef FAST
#pragma float_control(precise, on)
int subit(float a, float b, float *fp) {
  // CHECKFAST: define {{.*}}@subit(float noundef %a, float noundef %b{{.*}}
  *fp = __arithmetic_fence(a - b);
  *fp = (a + b);
  // CHECK-NOT: call{{.*}} float @llvm.arithmetic.fence.f32(float noundef %add)
  return 0;
}
#endif
