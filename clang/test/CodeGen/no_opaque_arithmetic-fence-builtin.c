// Test with fast math
// RUN: %clang_cc1 -triple x86_64-linux-gnu -emit-llvm -no-opaque-pointers -DFAST \
// RUN: -mreassociate \
// RUN: -o - %s | FileCheck --check-prefixes CHECK,CHECKFAST,CHECKNPP %s
//
// RUN: %clang_cc1 -triple i386-pc-linux-gnu -emit-llvm -no-opaque-pointers -DFAST \
// RUN: -mreassociate  \
// RUN: -o - %s | FileCheck --check-prefixes CHECK,CHECKSTINT-32 %s

// Test with fast math and fprotect-parens
// RUN: %clang_cc1 -triple x86_64-linux-gnu -emit-llvm -no-opaque-pointers -DFAST \
// RUN: -mreassociate -fprotect-parens -ffp-contract=on\
// RUN:  -o - %s \
// RUN: | FileCheck --check-prefixes CHECK,CHECKFAST,CHECKPP %s

// Test without fast math: llvm intrinsic not created
// RUN: %clang_cc1 -triple x86_64-linux-gnu -emit-llvm -no-opaque-pointers -fprotect-parens \
// RUN: -o - %s \
// RUN: | FileCheck --implicit-check-not="llvm.arithmetic.fence" %s

int v;

int addit(float a, float b) {
  // CHECK: define {{.*}}@addit(float {{.*}}, float {{.*}}) #0 {

  // CHECKSTINT-32: [[P_32:%.*]] = alloca i32*, align 4
  // CHECKSTINT-32-NEXT: [[I_32:%.*]] = alloca i32, align 4
  // CHECKSTINT-32: [[K_32:%.*]] = alloca i32*, align 4

  _Complex double cd, cd1;
  cd = __arithmetic_fence(cd1);

  // CHECKFAST: [[A:%.*]] = alloca float, align 4
  // CHECKFAST-NEXT: [[B:%.*]] = alloca float, align 4
  // CHECKFAST-NEXT: [[CD:%.*]] = alloca { double, double }, align 8
  // CHECKFAST-NEXT: [[CD1:%.*]] = alloca { double, double }, align 8
  // CHECKFAST-NEXT: [[VEC1:%.*]] = alloca <2 x float>, align 8
  // CHECKFAST-NEXT: [[VEC2:%.*]] = alloca <2 x float>, align 8
  // CHECKFAST-NEXT: [[P:%.*]] = alloca i32*, align 8
  // CHECKFAST-NEXT: [[I:%.*]] = alloca i32, align 4
  // CHECKFAST-NEXT: [[K:%.*]] = alloca i32*, align 8
  // CHECKFAST-NEXT: store float {{.*}}, float* [[A]], align 4
  // CHECKFAST-NEXT: store float {{.*}}, float* [[B]], align 4
  // CHECKFAST-NEXT: [[GEP_CD1_R:%.*]] = getelementptr inbounds { double, double }, { double, double }* [[CD1]], i32 0, i32 0
  // CHECKFAST-NEXT: [[CD1_REAL:%.*]] = load double, double* [[GEP_CD1_R]], align 8
  // CHECKFAST-NEXT: [[GEP_CD1_I:%.*]] = getelementptr inbounds { double, double }, { double, double }* [[CD1]], i32 0, i32 1
  // CHECKFAST-NEXT: [[CD1_IMAG:%.*]] = load double, double* [[GEP_CD1_I]], align 8
  // CHECKFAST-NEXT: [[TMP0:%.*]] = call{{.*}} double @llvm.arithmetic.fence.f64(double [[CD1_REAL]])
  // CHECKFAST-NEXT: [[TMP1:%.*]] = call{{.*}} double @llvm.arithmetic.fence.f64(double [[CD1_IMAG]])

  typedef float __v2f32 __attribute__((__vector_size__(8)));
  __v2f32 vec1, vec2;
  vec1 = __arithmetic_fence(vec2);
  // CHECKFAST-NEXT: [[CD_REAL:%.*]] = getelementptr inbounds { double, double }, { double, double }* [[CD]], i32 0, i32 0
  // CHECKFAST-NEXT: [[CD_IMAG:%.*]] = getelementptr inbounds { double, double }, { double, double }* [[CD]], i32 0, i32 1

  // CHECKFAST-NEXT: store double [[TMP0]], double* [[CD_REAL]], align 8
  // CHECKFAST-NEXT: store double [[TMP1]], double* [[CD_IMAG]], align 8

  // CHECKFAST-NEXT: [[L2:%.*]] = load <2 x float>, <2 x float>* [[VEC2]], align 8
  // CHECKFAST-NEXT: [[TMP3:%.*]] = call{{.*}} <2 x float> @llvm.arithmetic.fence.v2f32(<2 x float> [[L2]]
  // CHECKFAST-NEXT: store <2 x float> [[TMP3]], <2 x float>* [[VEC1]], align 8

  vec2 = (vec2 + vec1);
  // CHECKFAST-NEXT: [[TMP4:%.*]] = load <2 x float>, <2 x float>* [[VEC2]], align 8
  // CHECKFAST-NEXT: [[TMP5:%.*]] = load <2 x float>, <2 x float>* [[VEC1]], align 8
  // CHECKFAST: [[TMP6:%.*]] = fadd reassoc <2 x float> [[TMP4]], [[TMP5]]

  // CHECKPP: [[TMP7:%.*]] = call reassoc <2 x float> @llvm.arithmetic.fence.v2f32(<2 x float> [[TMP6]])
  // CHECKPP: store <2 x float> [[TMP7]], <2 x float>* [[VEC2]], align 8

  // CHECKNPP: store <2 x float> [[TMP6]], <2 x float>* [[VEC2]], align 8

  v = __arithmetic_fence(a + b);
  // CHECKFAST: [[TMP9:%.*]] = load float, float* [[A]], align 4
  // CHECKFAST-NEXT: [[TMP10:%.*]] = load float, float* [[B]], align 4
  // CHECKFAST-NEXT: [[ADD1:%.*]] = fadd reassoc float [[TMP9]], [[TMP10]]
  // CHECKFAST-NEXT: [[CALL8:%.*]] = call{{.*}} float @llvm.arithmetic.fence.f32(float [[ADD1]])
  // CHECKFAST-NEXT: [[CONV:%.*]] = fptosi float [[CALL8]] to i32
  // CHECKFAST: store i32 [[CONV]], i32* @v, align 4

  v = (a + b);
  // CHECKFAST-NEXT: [[TMP11:%.*]] = load float, float* [[A]], align 4
  // CHECKFAST-NEXT: [[TMP12:%.*]] = load float, float* [[B]], align 4
  // CHECKFAST: [[TMP13:%.*]] = fadd reassoc float [[TMP11]], [[TMP12]]

  // CHECKPP: [[TMP14:%.*]] = call reassoc float @llvm.arithmetic.fence.f32(float [[TMP13]])
  // CHECKPP-NEXT: [[CONV1:%.*]] = fptosi float [[TMP14]] to i32
  // CHECKPP: store i32 [[CONV1]], i32* @v, align 4

  // CHECKNPP: [[CONV2:%.*]] = fptosi float [[TMP13]] to i32
  // CHECKNPP: store i32 [[CONV2]], i32* @v, align 4

  v = a + (b * b);
  // CHECKFAST-NEXT: [[TMP15:%.*]] = load float, float* [[A]], align 4
  // CHECKFAST-NEXT: [[TMP16:%.*]] = load float, float* [[B]], align 4
  // CHECKFAST-NEXT: [[TMP17:%.*]] = load float, float* [[B]], align 4
  // CHECKFAST: [[TMP18:%.*]] = fmul reassoc float [[TMP16]], [[TMP17]]

  // CHECKPP: [[TMP19:%.*]] = call reassoc float @llvm.arithmetic.fence.f32(float [[TMP18]])
  // CHECKPP-NEXT: [[TMP20:%.*]] = fadd reassoc float [[TMP15]], [[TMP19]]
  // CHECKPP-NEXT: [[CONV4:%.*]] = fptosi float [[TMP20]] to i32
  // CHECKPP: store i32 [[CONV4]], i32* @v, align 4

  b = (a);
  // CHECKFAST: [[TMP22:%.*]] = load float, float* [[A]], align 4
  // CHECKFAST-NEXT: store float [[TMP22]], float* [[B]], align 4

  (a) = b;
  // CHECKFAST-NEXT: [[TMP23:%.*]] = load float, float* [[B]], align 4
  // CHECKFAST-NEXT: store float [[TMP23]], float* [[A]], align 4

  int *p;
  int i = __fence(1 + 2);
  // CHECKFAST: store i32 3, i32* [[I]], align 4
  // CHECK-NOT: call{{.*}} float @llvm.arithmetic.fence.int(float {{.*}})

  int *k = __fence(i + p);
  // CHECKFAST-NEXT: [[TMP26:%.*]] = load i32, i32* [[I]], align 4
  // CHECKFAST-NEXT: [[TMP27:%.*]] =  load i32*, i32** [[P]], align 8
  // CHECKFAST-NEXT: [[IDX:%.*]] = sext i32 [[TMP26]] to i64
  // CHECKFAST-NEXT: [[GEP11:%.*]] = getelementptr inbounds i32, i32* [[TMP27]], i64 [[IDX]]
  // CHECKFAST: store i32* [[GEP11]], i32** [[K]], align 8

  // CHECKSTINT-32: [[TMP24:%.*]] = load i32, i32* [[I_32]], align 4
  // CHECKSTINT-32-NEXT: [[TMP25:%.*]] = load i32*, i32** [[P_32]], align 4
  // CHECKSTINT-32-NEXT: [[GEP10:%.*]] = getelementptr inbounds i32, i32* [[TMP25]], i32 [[TMP24]]
  // CHECKSTINT-32-NEXT: store i32* [[GEP10]], i32** [[K_32]], align 4

  // CHECK-NOT: call{{.*}} float @llvm.arithmetic.fence.int(float {{.*}})

  return 0;
  // CHECK-NEXT ret i32 0
}

int addit1(int a, int b) {
  // CHECK: define {{.*}}@addit1(i32 {{.*}}, i32 {{.*}}
  v = (a + b);
  // CHECK-NOT: call{{.*}} float @llvm.arithmetic.fence.int(float {{.*}})
  return 0;
}
#ifdef FAST
#pragma float_control(precise, on)
int subit(float a, float b, float *fp) {
  // CHECKFAST: define {{.*}}@subit(float {{.*}}, float {{.*}}
  *fp = __arithmetic_fence(a - b);
  *fp = (a + b);
  // CHECK-NOT: call{{.*}} float @llvm.arithmetic.fence.f32(float {{.*}})
  return 0;
}
#endif
