// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -fintel-compatibility -O0 -disable-llvm-passes -emit-llvm %s -o - | FileCheck %s

#define FEAT_1 1U << 7
#define FEAT_2 1U << 8

// CHECK: @__intel_cpu_feature_indicator_x = external global [2 x i64]
bool usage() {
  if (_may_i_use_cpu_feature(FEAT_1 | FEAT_2) == 7) {}
  // CHECK: call void @__intel_cpu_features_init_x()
  // CHECK: %[[INDICATOR:[A-Za-z0-9_-]+]] = load i64, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @__intel_cpu_feature_indicator_x, i64 0, i64 0), align 8
  // CHECK: %[[JOIN:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR]], 384
  // CHECK: %[[CHECK:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN]], 384
  // CHECK: %[[CONV:[A-Za-z0-9_-]+]] = zext i1 %[[CHECK]] to i32
  // CHECK: %[[IF_CMP:[A-Za-z0-9_-]+]] = icmp eq i32 %[[CONV]], 7
  // CHECK: br i1 %[[IF_CMP]]

  return _may_i_use_cpu_feature(FEAT_1 | FEAT_2);

  // CHECK: %[[INDICATOR:[A-Za-z0-9_-]+]] = load i64, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @__intel_cpu_feature_indicator_x, i64 0, i64 0), align 8
  // CHECK: %[[JOIN:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR]], 384
  // CHECK: %[[CHECK:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN]], 384
  // CHECK: ret i1 %[[CHECK]]
}

bool usage_ext_0() {
  if (_may_i_use_cpu_feature_ext(FEAT_1 | FEAT_2, 0) == 7) {
  }
  // CHECK: call void @__intel_cpu_features_init_x()
  // CHECK: %[[INDICATOR:[A-Za-z0-9_-]+]] = load i64, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @__intel_cpu_feature_indicator_x, i64 0, i64 0), align 8
  // CHECK: %[[JOIN:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR]], 384
  // CHECK: %[[CHECK:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN]], 384
  // CHECK: %[[CONV:[A-Za-z0-9_-]+]] = zext i1 %[[CHECK]] to i32
  // CHECK: %[[IF_CMP:[A-Za-z0-9_-]+]] = icmp eq i32 %[[CONV]], 7
  // CHECK: br i1 %[[IF_CMP]]

  return _may_i_use_cpu_feature_ext(FEAT_1 | FEAT_2, 0);

  // CHECK: %[[INDICATOR:[A-Za-z0-9_-]+]] = load i64, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @__intel_cpu_feature_indicator_x, i64 0, i64 0), align 8
  // CHECK: %[[JOIN:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR]], 384
  // CHECK: %[[CHECK:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN]], 384
  // CHECK: ret i1 %[[CHECK]]
}

bool usage_ext_1() {
  if (_may_i_use_cpu_feature_ext(FEAT_1 | FEAT_2, 1) == 7) {
  }
  // CHECK: call void @__intel_cpu_features_init_x()
  // CHECK: %[[INDICATOR:[A-Za-z0-9_-]+]] = load i64, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @__intel_cpu_feature_indicator_x, i64 0, i64 1), align 8
  // CHECK: %[[JOIN:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR]], 384
  // CHECK: %[[CHECK:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN]], 384
  // CHECK: %[[CONV:[A-Za-z0-9_-]+]] = zext i1 %[[CHECK]] to i32
  // CHECK: %[[IF_CMP:[A-Za-z0-9_-]+]] = icmp eq i32 %[[CONV]], 7
  // CHECK: br i1 %[[IF_CMP]]

  return _may_i_use_cpu_feature_ext(FEAT_1 | FEAT_2, 1);

  // CHECK: %[[INDICATOR:[A-Za-z0-9_-]+]] = load i64, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @__intel_cpu_feature_indicator_x, i64 0, i64 1), align 8
  // CHECK: %[[JOIN:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR]], 384
  // CHECK: %[[CHECK:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN]], 384
  // CHECK: ret i1 %[[CHECK]]
}

bool usage_str_0() {
  if (_may_i_use_cpu_feature_str("avx2", "bmi") == 7) {
  }
  // CHECK: call void @__intel_cpu_features_init_x()
  // CHECK: %[[INDICATOR:[A-Za-z0-9_-]+]] = load i64, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @__intel_cpu_feature_indicator_x, i64 0, i64 0), align 8
  // CHECK: %[[JOIN:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR]], 8912896
  // CHECK: %[[CHECK:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN]], 8912896
  // CHECK: %[[CONV:[A-Za-z0-9_-]+]] = zext i1 %[[CHECK]] to i32
  // CHECK: %[[IF_CMP:[A-Za-z0-9_-]+]] = icmp eq i32 %[[CONV]], 7
  // CHECK: br i1 %[[IF_CMP]]

  return _may_i_use_cpu_feature_str("bmi", "avx2");

  // CHECK: %[[INDICATOR:[A-Za-z0-9_-]+]] = load i64, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @__intel_cpu_feature_indicator_x, i64 0, i64 0), align 8
  // CHECK: %[[JOIN:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR]], 8912896
  // CHECK: %[[CHECK:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN]], 8912896
  // CHECK: ret i1 %[[CHECK]]
}

bool usage_str_1() {
  if (_may_i_use_cpu_feature_str("avx2", "bmi", "cldemote", "waitpkg") == 7) {
  }
  // CHECK: call void @__intel_cpu_features_init_x()
  // CHECK: %[[INDICATOR:[A-Za-z0-9_-]+]] = load i64, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @__intel_cpu_feature_indicator_x, i64 0, i64 0), align 8
  // CHECK: %[[JOIN:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR]], 8912896
  // CHECK: %[[CHECK:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN]], 8912896
  // CHECK: %[[INDICATOR2:[A-Za-z0-9_-]+]] = load i64, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @__intel_cpu_feature_indicator_x, i64 0, i64 1), align 8
  // CHECK: %[[JOIN2:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR2]], 9
  // CHECK: %[[CHECK2:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN2]], 9
  // CHECK: %[[AND_BOTH:[A-Za-z0-9_-]+]] = and i1 %[[CHECK]], %[[CHECK2]]
  // CHECK: %[[CONV:[A-Za-z0-9_-]+]] = zext i1 %[[AND_BOTH]] to i32
  // CHECK: %[[IF_CMP:[A-Za-z0-9_-]+]] = icmp eq i32 %[[CONV]], 7
  // CHECK: br i1 %[[IF_CMP]]

  return _may_i_use_cpu_feature_str("cldemote", "bmi", "avx2", "waitpkg");

  // CHECK: %[[INDICATOR:[A-Za-z0-9_-]+]] = load i64, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @__intel_cpu_feature_indicator_x, i64 0, i64 0), align 8
  // CHECK: %[[JOIN:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR]], 8912896
  // CHECK: %[[CHECK:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN]], 8912896
  // CHECK: %[[INDICATOR2:[A-Za-z0-9_-]+]] = load i64, i64* getelementptr inbounds ([2 x i64], [2 x i64]* @__intel_cpu_feature_indicator_x, i64 0, i64 1), align 8
  // CHECK: %[[JOIN2:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR2]], 9
  // CHECK: %[[CHECK2:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN2]], 9
  // CHECK: %[[AND_BOTH:[A-Za-z0-9_-]+]] = and i1 %[[CHECK]], %[[CHECK2]]
  // CHECK: ret i1 %[[AND_BOTH]]
}

// CMPLRLLVM-11744: Handle a case where a zero feature is given, it would
// previously cause a null-dereference, but should simply always return
// 1 for 'true'.
bool zero_features_0() {
  if (_may_i_use_cpu_feature(0)) {
  }
  // CHECK: call void @__intel_cpu_features_init_x()
  // CHECK: br i1 true

  return _may_i_use_cpu_feature(0);
  // CHECK: ret i1 true
}


bool zero_features_1() {
  if (_may_i_use_cpu_feature_ext(0, 0)) {
  }
  // CHECK: call void @__intel_cpu_features_init_x()
  // CHECK: br i1 true

  return _may_i_use_cpu_feature_ext(0, 0);
  // CHECK: ret i1 true
}

bool zero_features_2() {
  if (_may_i_use_cpu_feature_ext(0, 1)) {
  }
  // CHECK: call void @__intel_cpu_features_init_x()
  // CHECK: br i1 true

  return _may_i_use_cpu_feature_ext(0, 1);
  // CHECK: ret i1 true
}
