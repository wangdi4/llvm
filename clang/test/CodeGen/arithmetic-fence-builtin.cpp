// Test with fast math
// RUN: %clang_cc1 -triple x86_64-linux-gnu -emit-llvm \
// RUN: -mreassociate  -o - %s -opaque-pointers \
// RUN: | FileCheck --check-prefixes CHECK,CHECKFAST %s

// Test 32bit version
// RUN: %clang_cc1 -triple i386-pc-linux-gnu -emit-llvm \
// RUN: -mreassociate  -o - %s -opaque-pointers \
// RUN: | FileCheck --check-prefixes CHECK,CHECKSTINT-32 %s
//
// Test with fast math and fprotect-parens
// RUN: %clang_cc1 -triple x86_64-linux-gnu -emit-llvm \
// RUN: -mreassociate -fprotect-parens -ffp-contract=on \
// RUN:  -o - %s -opaque-pointers\
// RUN: | FileCheck --check-prefixes CHECK,CHECKFAST %s

int v;

template <typename T> T addF(T a, T b) {
  return __fence(a + b);
}

template <typename T> T addAF(T a, T b) {
  return __arithmetic_fence(a + b);
}

int addit(float a, float b) {
  // CHECK: define {{.*}} @{{.*}}additff(float {{.*}}, float {{.*}}) #0 {
  int *p;
  float f = addF(a, b);
  int i = addF(1, 2);
  int *k = __fence(3 + p);
  float af = addAF(a,b);

  // CHECK: [[ADDR_A:%.*]] = alloca float, align 4
  // CHECK: [[ADDR_B:%.*]] = alloca float, align 4

  // CHECKFAST: [[P_64:%.*]] = alloca ptr, align 8
  // CHECKSTINT-32: [[P_32:%.*]] = alloca ptr, align 4

  // CHECK: [[F:%.*]] = alloca float, align 4
  // CHECK: [[I:%.*]] = alloca i32, align 4

  // CHECKFAST: [[K_64:%.*]] = alloca ptr, align 8
  // CHECKSTINT-32: [[K_32:%.*]] = alloca ptr, align 4

  // CHECK-NEXT: [[AF:%.*]] = alloca float, align 4
  // CHECK-NEXT: store float {{.*}}, ptr [[ADDR_A]], align 4
  // CHECK-NEXT: store float {{.*}}, ptr [[ADDR_B]], align 4

  // CHECK-NEXT: [[TEMP_A:%.*]] = load float, ptr [[ADDR_A]], align 4
  // CHECK-NEXT: [[TEMP_B:%.*]] = load float, ptr [[ADDR_B]], align 4

  // CHECK:  [[CALL:%.*]] = call reassoc noundef float @_Z4addFIfET_S0_S0_(float noundef [[TEMP_A]], float noundef [[TEMP_B]])
  // CHECK-NEXT:  store float [[CALL]], ptr [[F]], align 4
  // CHECK-NEXT: [[CALL1:%.*]] = call noundef i32 @_Z4addFIiET_S0_S0_(i32 noundef 1, i32 noundef 2)
  // CHECK:  store i32 [[CALL1]], ptr [[I]], align 4

  // CHECKSTINT-32-NEXT: [[TEMP1:%.*]] = load ptr, ptr [[P_32]], align 4
  // CHECKSTINT-32-NEXT: [[GEP1:%.*]] = getelementptr inbounds i32, ptr [[TEMP1]], i32 3
  // CHECKSTINT-32-NEXT: store ptr [[GEP1]], ptr [[K_32]], align 4
  // CHECKSTINT-32-NEXT: load float, ptr [[ADDR_A]], align 4
  // CHECKSTINT-32: load float, ptr [[ADDR_B]], align 4

  // CHECKFAST-NEXT: [[TEMP2:%.*]] = load ptr, ptr [[P_64]], align 8
  // CHECKFAST-NEXT: [[GEP2:%.*]] = getelementptr inbounds i32, ptr [[TEMP2]], i64 3
  // CHECKFAST-NEXT: store ptr [[GEP2]], ptr [[K_64]], align 8
  // CHECKFAST-NEXT: [[TEMP3:%.*]] = load float, ptr [[ADDR_A]], align 4
  // CHECKFAST-NEXT: [[TEMP4:%.*]] = load float, ptr [[ADDR_B]], align 4
  // CHECKFAST-NEXT: [[CALL2:%.*]] = call reassoc noundef float @_Z5addAFIfET_S0_S0_(float noundef [[TEMP3]], float noundef [[TEMP4]])

  // CHECKFAST:  store float [[CALL2]], ptr [[AF]], align 4

  return 0;
  // CHECK-NEXT ret i32 0
}

  // CHECK: define linkonce_odr noundef float @_Z5addAFIfET_S0_S0_(float noundef {{.*}}, float noundef {{.*}})
  // CHECK: [[A:%.*]] = alloca float, align 4
  // CHECK-NEXT: [[B:%.*]] = alloca float, align 4
  // CHECK-NEXT: store float {{.*}}, ptr [[A]], align 4
  // CHECK-NEXT: store float {{.*}}, ptr [[B]], align 4
  // CHECK-NEXT: [[Z1:%.*]] = load float, ptr [[A]], align 4
  // CHECK-NEXT: [[Z2:%.*]] = load float, ptr [[B]], align 4
  // CHECK-NEXT: [[ADD:%.*]] = fadd reassoc float [[Z1]], [[Z2]]
  // CHECK-NEXT: [[RES:%.*]] = call reassoc float @llvm.arithmetic.fence.f32(float [[ADD]])
  // CHECK: ret float [[RES]]
