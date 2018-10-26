// RUN: %clang_cc1 -triple=x86_64-unknown-linux-gnu -fintel-compatibility -O0 -emit-llvm %s -o - | FileCheck %s

// CHECK: @__intel_cpu_feature_indicator_x = external global i64
bool usage() {
#define FEAT_1 1U << 7
#define FEAT_2 1U << 8

  if (_may_i_use_cpu_feature(FEAT_1 | FEAT_2) == 7) {}
  // CHECK: call void @__cpu_indicator_init_x()
  // CHECK: %[[INDICATOR:[A-Za-z0-9_-]+]] = load i64, i64* @__intel_cpu_feature_indicator_x, align 8
  // CHECK: %[[JOIN:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR]], 384
  // CHECK: %[[CHECK:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN]], 384
  // CHECK: %[[CONV:[A-Za-z0-9_-]+]] = zext i1 %[[CHECK]] to i32
  // CHECK: %[[IF_CMP:[A-Za-z0-9_-]+]] = icmp eq i32 %[[CONV]], 7
  // CHECK: br i1 %[[IF_CMP]]

  return _may_i_use_cpu_feature(FEAT_1 | FEAT_2);

  // CHECK: %[[INDICATOR:[A-Za-z0-9_-]+]] = load i64, i64* @__intel_cpu_feature_indicator_x, align 8
  // CHECK: %[[JOIN:[A-Za-z0-9_-]+]] = and i64 %[[INDICATOR]], 384
  // CHECK: %[[CHECK:[A-Za-z0-9_-]+]] = icmp eq i64 %[[JOIN]], 384
  // CHECK: ret i1 %[[CHECK]]
}
