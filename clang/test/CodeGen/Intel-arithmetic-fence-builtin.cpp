// RUN: %clang_cc1 -triple x86_64-linux-gnu -emit-llvm \
// RUN: -mreassociate  -o - %s \
// RUN: | FileCheck --check-prefixes CHECK,CHECKFAST %s

// RUN: %clang_cc1 -triple i386-pc-linux-gnu -emit-llvm \
// RUN: -mreassociate  -o - %s \
// RUN: | FileCheck --check-prefixes CHECK,CHECK32 %s

// RUN: %clang_cc1 -triple x86_64-linux-gnu -emit-llvm \
// RUN: -mreassociate -fprotect-parens -ffp-contract=on \
// RUN:  -o - %s \
// RUN: | FileCheck --check-prefixes CHECK,CHECKFAST %s

template <typename T> T addF(T a, T b) {
  return __fence(a + b);
}

int addit(float a, float b) {
  // CHECK-LABEL: define {{.*}} @{{.*}}additff(float {{.*}}, float {{.*}}) #0 {
  int *p;
  float f = addF(a, b);
  int i = addF(1, 2);
  int *k = __fence(3 + p);

  // CHECK: [[ADDR_A:%.*]] = alloca float, align 4
  // CHECK: [[ADDR_B:%.*]] = alloca float, align 4

  // CHECKFAST: [[P_64:%.*]] = alloca ptr, align 8
  // CHECK32: [[P_32:%.*]] = alloca ptr, align 4

  // CHECK: [[F:%.*]] = alloca float, align 4
  // CHECK: [[I:%.*]] = alloca i32, align 4

  // CHECKFAST: [[K_64:%.*]] = alloca ptr, align 8
  // CHECK32: [[K_32:%.*]] = alloca ptr, align 4

  // CHECK-NEXT: store float {{.*}}, ptr [[ADDR_A]], align 4
  // CHECK-NEXT: store float {{.*}}, ptr [[ADDR_B]], align 4

  // CHECK-NEXT: [[TEMP_A:%.*]] = load float, ptr [[ADDR_A]], align 4
  // CHECK-NEXT: [[TEMP_B:%.*]] = load float, ptr [[ADDR_B]], align 4

  // CHECK:  [[CALL:%.*]] = call reassoc noundef float @_Z4addFIfET_S0_S0_(float noundef [[TEMP_A]], float noundef [[TEMP_B]])
  // CHECK-NEXT:  store float [[CALL]], ptr [[F]], align 4
  // CHECK-NEXT: [[CALL1:%.*]] = call noundef i32 @_Z4addFIiET_S0_S0_(i32 noundef 1, i32 noundef 2)
  // CHECK:  store i32 [[CALL1]], ptr [[I]], align 4

  // CHECK32: [[TMP2:%.*]] = load ptr, ptr [[P_32]], align 4
  // CHECK32-NEXT: [[ADD:%.*]] = getelementptr inbounds i32, ptr [[TMP2]], i32 3
  // CHECK32-NEXT:   store ptr [[ADD]], ptr [[K_32]], align 4

  // CHECKFAST-NEXT: [[TEMP2:%.*]] = load ptr, ptr [[P_64]], align 8
  // CHECKFAST-NEXT: [[GEP2:%.*]] = getelementptr inbounds i32, ptr [[TEMP2]], i64 3
  // CHECKFAST-NEXT: store ptr [[GEP2]], ptr [[K_64]], align 8

  return 0;

  // CHECK: ret i32 0
}

  // CHECK: define linkonce_odr noundef float @_Z4addFIfET_S0_S0_(float noundef {{.*}}, float noundef {{.*}})
  // CHECK: [[A:%.*]] = alloca float, align 4
  // CHECK-NEXT: [[B:%.*]] = alloca float, align 4
  // CHECK-NEXT: store float {{.*}}, ptr [[A]], align 4
  // CHECK-NEXT: store float {{.*}}, ptr [[B]], align 4
  // CHECK-NEXT: [[Z1:%.*]] = load float, ptr [[A]], align 4
  // CHECK-NEXT: [[Z2:%.*]] = load float, ptr [[B]], align 4
  // CHECK-NEXT: [[ADD:%.*]] = fadd reassoc float [[Z1]], [[Z2]]
  // CHECK-NEXT: [[RES:%.*]] = call reassoc float @llvm.arithmetic.fence.f32(float [[ADD]])
  // CHECK: ret float [[RES]]
