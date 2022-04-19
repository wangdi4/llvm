// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -fintel-compatibility -O0 -disable-llvm-passes -emit-llvm -opaque-pointers %s -o - | FileCheck %s

#define FEAT_1 1U << 7
#define FEAT_2 1U << 8

// CHECK: @__intel_cpu_feature_indicator_x = external global [2 x i64]

// CHECK: define{{.+}} @_Z5usagev()
bool usage() {
  if (_may_i_use_cpu_feature(FEAT_1 | FEAT_2) == 7) {}
  // CHECK: br label %[[INIT_CMP:.+]]

  // CHECK: [[INIT_CMP]]:
  // CHECK-NEXT: %[[INDICATOR:.+]] = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  // CHECK-NEXT: %[[CMP_EQ:.+]] = icmp eq i64 %[[INDICATOR]], 0
  // CHECK-NEXT: br i1 %[[CMP_EQ]], label %[[INIT_BODY:.+]], label %[[REST:.+]]

  // CHECK: [[INIT_BODY]]:
  // CHECK-NEXT: call intel_features_init_cc void @__intel_cpu_features_init_x()
  // CHECK-NEXT: br label %[[INIT_CMP]]

  // CHECK: [[REST]]:
  // CHECK-NEXT: %[[INDICATOR:[A-Za-z0-9_-]+]] = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  // CHECK-NEXT: %[[JOIN:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR]], 384
  // CHECK-NEXT: %[[CHECK:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN]], 384
  // CHECK-NEXT: %[[CONV:[A-Za-z0-9_-]+]] = zext i1 %[[CHECK]] to i32
  // CHECK-NEXT: %[[IF_CMP:[A-Za-z0-9_-]+]] = icmp eq i32 %[[CONV]], 7
  // CHECK-NEXT: br i1 %[[IF_CMP]], label %{{.+}}, label %[[IF_END:.+]]

  return _may_i_use_cpu_feature(FEAT_1 | FEAT_2);

  // CHECK: [[IF_END]]:
  // CHECK-NEXT: br label %[[INIT_CMP:.+]]

  // CHECK: [[INIT_CMP]]:
  // CHECK-NEXT: %[[INDICATOR:.+]] = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  // CHECK-NEXT: %[[CMP_EQ:.+]] = icmp eq i64 %[[INDICATOR]], 0
  // CHECK-NEXT: br i1 %[[CMP_EQ]], label %[[INIT_BODY:.+]], label %[[REST:.+]]

  // CHECK: [[INIT_BODY]]:
  // CHECK-NEXT: call intel_features_init_cc void @__intel_cpu_features_init_x()
  // CHECK-NEXT: br label %[[INIT_CMP]]

  // CHECK: [[REST]]:
  // CHECK-NEXT: %[[INDICATOR:[A-Za-z0-9_-]+]] = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  // CHECK-NEXT: %[[JOIN:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR]], 384
  // CHECK-NEXT: %[[CHECK:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN]], 384
  // CHECK-NEXT: ret i1 %[[CHECK]]
}

// CHECK: define{{.+}} @_Z11usage_ext_0v()
bool usage_ext_0() {
  if (_may_i_use_cpu_feature_ext(FEAT_1 | FEAT_2, 0) == 7) {
  }
  // CHECK: br label %[[INIT_CMP:.+]]

  // CHECK: [[INIT_CMP]]:
  // CHECK-NEXT: %[[INDICATOR:.+]] = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  // CHECK-NEXT: %[[CMP_EQ:.+]] = icmp eq i64 %[[INDICATOR]], 0
  // CHECK-NEXT: br i1 %[[CMP_EQ]], label %[[INIT_BODY:.+]], label %[[REST:.+]]

  // CHECK: [[INIT_BODY]]:
  // CHECK-NEXT: call intel_features_init_cc void @__intel_cpu_features_init_x()
  // CHECK-NEXT: br label %[[INIT_CMP]]

  // CHECK: [[REST]]:
  // CHECK-NEXT: %[[INDICATOR:[A-Za-z0-9_-]+]] = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  // CHECK-NEXT: %[[JOIN:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR]], 384
  // CHECK-NEXT: %[[CHECK:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN]], 384
  // CHECK-NEXT: %[[CONV:[A-Za-z0-9_-]+]] = zext i1 %[[CHECK]] to i32
  // CHECK-NEXT: %[[IF_CMP:[A-Za-z0-9_-]+]] = icmp eq i32 %[[CONV]], 7
  // CHECK-NEXT: br i1 %[[IF_CMP]]

  return _may_i_use_cpu_feature_ext(FEAT_1 | FEAT_2, 0);

  // CHECK: [[IF_END]]:
  // CHECK-NEXT: br label %[[INIT_CMP:.+]]

  // CHECK: [[INIT_CMP]]:
  // CHECK-NEXT: %[[INDICATOR:.+]] = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  // CHECK-NEXT: %[[CMP_EQ:.+]] = icmp eq i64 %[[INDICATOR]], 0
  // CHECK-NEXT: br i1 %[[CMP_EQ]], label %[[INIT_BODY:.+]], label %[[REST:.+]]

  // CHECK: [[INIT_BODY]]:
  // CHECK-NEXT: call intel_features_init_cc void @__intel_cpu_features_init_x()
  // CHECK-NEXT: br label %[[INIT_CMP]]

  // CHECK: [[REST]]:
  // CHECK-NEXT: %[[INDICATOR:[A-Za-z0-9_-]+]] = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  // CHECK-NEXT: %[[JOIN:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR]], 384
  // CHECK-NEXT: %[[CHECK:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN]], 384
  // CHECK-NEXT: ret i1 %[[CHECK]]
}

// CHECK: define{{.+}} @_Z11usage_ext_1v()

bool usage_ext_1() {
  if (_may_i_use_cpu_feature_ext(FEAT_1 | FEAT_2, 1) == 7) {
  }
  // CHECK: br label %[[INIT_CMP:.+]]

  // CHECK: [[INIT_CMP]]:
  // CHECK-NEXT: %[[INDICATOR:.+]] = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  // CHECK-NEXT: %[[CMP_EQ:.+]] = icmp eq i64 %[[INDICATOR]], 0
  // CHECK-NEXT: br i1 %[[CMP_EQ]], label %[[INIT_BODY:.+]], label %[[REST:.+]]

  // CHECK: [[INIT_BODY]]:
  // CHECK-NEXT: call intel_features_init_cc void @__intel_cpu_features_init_x()
  // CHECK-NEXT: br label %[[INIT_CMP]]

  // CHECK: [[REST]]:
  // CHECK-NEXT: %[[INDICATOR:[A-Za-z0-9_-]+]] = load i64, ptr getelementptr inbounds ([2 x i64], ptr @__intel_cpu_feature_indicator_x, i64 0, i64 1), align 8
  // CHECK-NEXT: %[[JOIN:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR]], 384
  // CHECK-NEXT: %[[CHECK:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN]], 384
  // CHECK-NEXT: %[[CONV:[A-Za-z0-9_-]+]] = zext i1 %[[CHECK]] to i32
  // CHECK-NEXT: %[[IF_CMP:[A-Za-z0-9_-]+]] = icmp eq i32 %[[CONV]], 7
  // CHECK-NEXT: br i1 %[[IF_CMP]]

  return _may_i_use_cpu_feature_ext(FEAT_1 | FEAT_2, 1);

  // CHECK: [[IF_END]]:
  // CHECK-NEXT: br label %[[INIT_CMP:.+]]

  // CHECK: [[INIT_CMP]]:
  // CHECK-NEXT: %[[INDICATOR:.+]] = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  // CHECK-NEXT: %[[CMP_EQ:.+]] = icmp eq i64 %[[INDICATOR]], 0
  // CHECK-NEXT: br i1 %[[CMP_EQ]], label %[[INIT_BODY:.+]], label %[[REST:.+]]

  // CHECK: [[INIT_BODY]]:
  // CHECK-NEXT: call intel_features_init_cc void @__intel_cpu_features_init_x()
  // CHECK-NEXT: br label %[[INIT_CMP]]

  // CHECK: [[REST]]:
  // CHECK-NEXT: %[[INDICATOR:[A-Za-z0-9_-]+]] = load i64, ptr getelementptr inbounds ([2 x i64], ptr @__intel_cpu_feature_indicator_x, i64 0, i64 1), align 8
  // CHECK-NEXT: %[[JOIN:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR]], 384
  // CHECK-NEXT: %[[CHECK:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN]], 384
  // CHECK-NEXT: ret i1 %[[CHECK]]
}

// CHECK: define{{.+}} @_Z11usage_str_0v()
bool usage_str_0() {
  if (_may_i_use_cpu_feature_str("avx2", "bmi") == 7) {
  }
  // CHECK: br label %[[INIT_CMP:.+]]

  // CHECK: [[INIT_CMP]]:
  // CHECK-NEXT: %[[INDICATOR:.+]] = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  // CHECK-NEXT: %[[CMP_EQ:.+]] = icmp eq i64 %[[INDICATOR]], 0
  // CHECK-NEXT: br i1 %[[CMP_EQ]], label %[[INIT_BODY:.+]], label %[[REST:.+]]

  // CHECK: [[INIT_BODY]]:
  // CHECK-NEXT: call intel_features_init_cc void @__intel_cpu_features_init_x()
  // CHECK-NEXT: br label %[[INIT_CMP]]

  // CHECK: [[REST]]:
  // CHECK-NEXT: %[[INDICATOR:[A-Za-z0-9_-]+]] = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  // CHECK-NEXT: %[[JOIN:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR]], 8912896
  // CHECK-NEXT: %[[CHECK:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN]], 8912896
  // CHECK-NEXT: %[[CONV:[A-Za-z0-9_-]+]] = zext i1 %[[CHECK]] to i32
  // CHECK-NEXT: %[[IF_CMP:[A-Za-z0-9_-]+]] = icmp eq i32 %[[CONV]], 7
  // CHECK-NEXT: br i1 %[[IF_CMP]]

  return _may_i_use_cpu_feature_str("bmi", "avx2");
  // CHECK: [[IF_END]]:
  // CHECK-NEXT: br label %[[INIT_CMP:.+]]

  // CHECK: [[INIT_CMP]]:
  // CHECK-NEXT: %[[INDICATOR:.+]] = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  // CHECK-NEXT: %[[CMP_EQ:.+]] = icmp eq i64 %[[INDICATOR]], 0
  // CHECK-NEXT: br i1 %[[CMP_EQ]], label %[[INIT_BODY:.+]], label %[[REST:.+]]

  // CHECK: [[INIT_BODY]]:
  // CHECK-NEXT: call intel_features_init_cc void @__intel_cpu_features_init_x()
  // CHECK-NEXT: br label %[[INIT_CMP]]

  // CHECK: [[REST]]:
  // CHECK-NEXT: %[[INDICATOR:[A-Za-z0-9_-]+]] = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  // CHECK-NEXT: %[[JOIN:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR]], 8912896
  // CHECK-NEXT: %[[CHECK:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN]], 8912896
  // CHECK-NEXT: ret i1 %[[CHECK]]
}

// CHECK: define{{.+}} @_Z11usage_str_1v()
bool usage_str_1() {
  if (_may_i_use_cpu_feature_str("avx2", "bmi", "cldemote", "waitpkg") == 7) {
  }
  // CHECK: br label %[[INIT_CMP:.+]]

  // CHECK: [[INIT_CMP]]:
  // CHECK-NEXT: %[[INDICATOR:.+]] = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  // CHECK-NEXT: %[[CMP_EQ:.+]] = icmp eq i64 %[[INDICATOR]], 0
  // CHECK-NEXT: br i1 %[[CMP_EQ]], label %[[INIT_BODY:.+]], label %[[REST:.+]]

  // CHECK: [[INIT_BODY]]:
  // CHECK-NEXT: call intel_features_init_cc void @__intel_cpu_features_init_x()
  // CHECK-NEXT: br label %[[INIT_CMP]]

  // CHECK: [[REST]]:
  // CHECK-NEXT: %[[INDICATOR:[A-Za-z0-9_-]+]] = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  // CHECK-NEXT: %[[JOIN:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR]], 8912896
  // CHECK-NEXT: %[[CHECK:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN]], 8912896
  // CHECK-NEXT: %[[INDICATOR2:[A-Za-z0-9_-]+]] = load i64, ptr getelementptr inbounds ([2 x i64], ptr @__intel_cpu_feature_indicator_x, i64 0, i64 1), align 8
  // CHECK-NEXT: %[[JOIN2:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR2]], 9
  // CHECK-NEXT: %[[CHECK2:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN2]], 9
  // CHECK-NEXT: %[[AND_BOTH:[A-Za-z0-9_-]+]] = and i1 %[[CHECK]], %[[CHECK2]]
  // CHECK-NEXT: %[[CONV:[A-Za-z0-9_-]+]] = zext i1 %[[AND_BOTH]] to i32
  // CHECK-NEXT: %[[IF_CMP:[A-Za-z0-9_-]+]] = icmp eq i32 %[[CONV]], 7
  // CHECK-NEXT: br i1 %[[IF_CMP]]

  return _may_i_use_cpu_feature_str("cldemote", "bmi", "avx2", "waitpkg");

  // CHECK: [[IF_END]]:
  // CHECK-NEXT: br label %[[INIT_CMP:.+]]

  // CHECK: [[INIT_CMP]]:
  // CHECK-NEXT: %[[INDICATOR:.+]] = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  // CHECK-NEXT: %[[CMP_EQ:.+]] = icmp eq i64 %[[INDICATOR]], 0
  // CHECK-NEXT: br i1 %[[CMP_EQ]], label %[[INIT_BODY:.+]], label %[[REST:.+]]

  // CHECK: [[INIT_BODY]]:
  // CHECK-NEXT: call intel_features_init_cc void @__intel_cpu_features_init_x()
  // CHECK-NEXT: br label %[[INIT_CMP]]

  // CHECK: [[REST]]:
  // CHECK-NEXT: %[[INDICATOR:[A-Za-z0-9_-]+]] = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  // CHECK-NEXT: %[[JOIN:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR]], 8912896
  // CHECK-NEXT: %[[CHECK:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN]], 8912896
  // CHECK-NEXT: %[[INDICATOR2:[A-Za-z0-9_-]+]] = load i64, ptr getelementptr inbounds ([2 x i64], ptr @__intel_cpu_feature_indicator_x, i64 0, i64 1), align 8
  // CHECK-NEXT: %[[JOIN2:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR2]], 9
  // CHECK-NEXT: %[[CHECK2:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN2]], 9
  // CHECK-NEXT: %[[AND_BOTH:[A-Za-z0-9_-]+]] = and i1 %[[CHECK]], %[[CHECK2]]
  // CHECK-NEXT: ret i1 %[[AND_BOTH]]
}

// CMPLRLLVM-11744: Handle a case where a zero feature is given, it would
// previously cause a null-dereference, but should simply always return
// 1 for 'true'.
// CHECK: define{{.+}} @_Z15zero_features_0v()
bool zero_features_0() {
  if (_may_i_use_cpu_feature(0)) {
  }
  // CHECK: br label %[[INIT_CMP:.+]]

  // CHECK: [[INIT_CMP]]:
  // CHECK-NEXT: %[[INDICATOR:.+]] = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  // CHECK-NEXT: %[[CMP_EQ:.+]] = icmp eq i64 %[[INDICATOR]], 0
  // CHECK-NEXT: br i1 %[[CMP_EQ]], label %[[INIT_BODY:.+]], label %[[REST:.+]]

  // CHECK: [[INIT_BODY]]:
  // CHECK-NEXT: call intel_features_init_cc void @__intel_cpu_features_init_x()
  // CHECK-NEXT: br label %[[INIT_CMP]]

  // CHECK: [[REST]]:
  // CHECK-NEXT: br i1 true

  return _may_i_use_cpu_feature(0);
  // CHECK: [[IF_END]]:
  // CHECK-NEXT: br label %[[INIT_CMP:.+]]

  // CHECK: [[INIT_CMP]]:
  // CHECK-NEXT: %[[INDICATOR:.+]] = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  // CHECK-NEXT: %[[CMP_EQ:.+]] = icmp eq i64 %[[INDICATOR]], 0
  // CHECK-NEXT: br i1 %[[CMP_EQ]], label %[[INIT_BODY:.+]], label %[[REST:.+]]

  // CHECK: [[INIT_BODY]]:
  // CHECK-NEXT: call intel_features_init_cc void @__intel_cpu_features_init_x()
  // CHECK-NEXT: br label %[[INIT_CMP]]

  // CHECK: [[REST]]:
  // CHECK-NEXT: ret i1 true
}

// CHECK: define{{.+}} @_Z15zero_features_1v()
bool zero_features_1() {
  if (_may_i_use_cpu_feature_ext(0, 0)) {
  }
  // CHECK: br label %[[INIT_CMP:.+]]

  // CHECK: [[INIT_CMP]]:
  // CHECK-NEXT: %[[INDICATOR:.+]] = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  // CHECK-NEXT: %[[CMP_EQ:.+]] = icmp eq i64 %[[INDICATOR]], 0
  // CHECK-NEXT: br i1 %[[CMP_EQ]], label %[[INIT_BODY:.+]], label %[[REST:.+]]

  // CHECK: [[INIT_BODY]]:
  // CHECK-NEXT: call intel_features_init_cc void @__intel_cpu_features_init_x()
  // CHECK-NEXT: br label %[[INIT_CMP]]

  // CHECK: [[REST]]:
  // CHECK-NEXT: br i1 true

  return _may_i_use_cpu_feature_ext(0, 0);
  // CHECK: [[IF_END]]:
  // CHECK-NEXT: br label %[[INIT_CMP:.+]]

  // CHECK: [[INIT_CMP]]:
  // CHECK-NEXT: %[[INDICATOR:.+]] = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  // CHECK-NEXT: %[[CMP_EQ:.+]] = icmp eq i64 %[[INDICATOR]], 0
  // CHECK-NEXT: br i1 %[[CMP_EQ]], label %[[INIT_BODY:.+]], label %[[REST:.+]]

  // CHECK: [[INIT_BODY]]:
  // CHECK-NEXT: call intel_features_init_cc void @__intel_cpu_features_init_x()
  // CHECK-NEXT: br label %[[INIT_CMP]]

  // CHECK: [[REST]]:
  // CHECK-NEXT: ret i1 true
}

// CHECK: define{{.+}} @_Z15zero_features_2v()
bool zero_features_2() {
  if (_may_i_use_cpu_feature_ext(0, 1)) {
  }
  // CHECK: br label %[[INIT_CMP:.+]]

  // CHECK: [[INIT_CMP]]:
  // CHECK-NEXT: %[[INDICATOR:.+]] = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  // CHECK-NEXT: %[[CMP_EQ:.+]] = icmp eq i64 %[[INDICATOR]], 0
  // CHECK-NEXT: br i1 %[[CMP_EQ]], label %[[INIT_BODY:.+]], label %[[REST:.+]]

  // CHECK: [[INIT_BODY]]:
  // CHECK-NEXT: call intel_features_init_cc void @__intel_cpu_features_init_x()
  // CHECK-NEXT: br label %[[INIT_CMP]]

  // CHECK: [[REST]]:
  // CHECK-NEXT: br i1 true


  return _may_i_use_cpu_feature_ext(0, 1);
  // CHECK: [[IF_END]]:
  // CHECK-NEXT: br label %[[INIT_CMP:.+]]

  // CHECK: [[INIT_CMP]]:
  // CHECK-NEXT: %[[INDICATOR:.+]] = load i64, ptr @__intel_cpu_feature_indicator_x, align 8
  // CHECK-NEXT: %[[CMP_EQ:.+]] = icmp eq i64 %[[INDICATOR]], 0
  // CHECK-NEXT: br i1 %[[CMP_EQ]], label %[[INIT_BODY:.+]], label %[[REST:.+]]

  // CHECK: [[INIT_BODY]]:
  // CHECK-NEXT: call intel_features_init_cc void @__intel_cpu_features_init_x()
  // CHECK-NEXT: br label %[[INIT_CMP]]

  // CHECK: [[REST]]:
  // CHECK-NEXT: ret i1 true
}
